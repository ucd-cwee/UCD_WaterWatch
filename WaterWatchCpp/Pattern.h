/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "Curve.h"
#include "List.h"
#include "LinkedList.h"
#include "BasicUnits.h"
#include "vec.h"
#include "cwee_math.h"
#include "UnorderedList.h"
#include "Strings.h"
#include "cweeTime.h"
#include "enum.h"

BETTER_ENUM(interpolation_t, int, SPLINE, LEFT, RIGHT, LINEAR, END);
// enum class interpolation_t { IT_SPLINE, IT_LEFT_CLAMP, IT_RIGHT_CLAMP, IT_LINEAR, IT_END };

const static std::map<interpolation_t, const char*> StringMap_interpolation_t = {
	{interpolation_t::SPLINE, "Spline"},
	{interpolation_t::LEFT, "Left"},
	{interpolation_t::RIGHT, "Right"},
	{interpolation_t::LINEAR, "Linear"},
	{interpolation_t::END, "End"}
};
template<> const static std::map<interpolation_t, const char*>& StaticStringMap< interpolation_t >() { return StringMap_interpolation_t; };

enum class patternSource {
	Scada,
	Global,
	Customers,
	Parent
};
enum class patternModifier {
	None,
	DayOfWeek,
	HourOfDay,
	Velocity,
	Acceleration,
	MovingAverage,
	Normalize
};

static constexpr u64 learnHourDelta = 0.25; // 0.5 = 30 min
static constexpr u64 learnHourMaxTime = 24; // 24 hours

template< class type >
class cweePattern {
public:
#ifdef useLinkedList
	typedef cweeLinkedList<type>	valueList;
	typedef cweeLinkedList<u64>	keyList;
#else
	typedef cweeThreadedList<type>	valueList;
	typedef cweeThreadedList<u64>	keyList;
#endif

	cweePattern() {
		// lock = new cweeSysMutex();
		SetGranularity(512);
	};
	virtual				~cweePattern() {
		// if (lock) delete lock;
	};

	virtual	void		SetGranularity(int size) {
		Lock();
		Granularity = size;
		(times).SetGranularity(Granularity);
		(values).SetGranularity(Granularity);
		Unlock();
	};
	virtual	int			GetGranularity() {
		int out;
		Lock();
		out = Granularity;
		Unlock();
		return out;
	};

	virtual cweeStr		GetName(void) const {
		cweeStr out;

		Lock();
		out = Name;
		Unlock();

		return out;
	};
	virtual void		SetName(const cweeStr& newName) {
		Lock();
		Name = newName;
		Unlock();
	};

	virtual int			GetSpecifier(void) const {
		int out;

		Lock();
		out = identity;
		Unlock();

		return out;
	};
	virtual void		SetSpecifier(int newNum) {
		Lock();
		identity = newNum;
		Unlock();
	};

	virtual keyList& UnsafeGetTimes() const {
		return times; // assumes already locked
	};
	virtual valueList& UnsafeGetValues() const {
		return values; // assumes already locked
	};

	virtual void		SetCharacteristic(const value_t& in) {
		Lock();
		valueType = in;
		Unlock();
	};
	virtual value_t		GetCharacteristic() const {
		value_t out;
		Lock();
		out = valueType;
		Unlock();
		return out;
	};
	virtual void		SetMeasurement(const measurement_t& in) {
		Lock();
		vec3 conv = cweeUnits::GetMadConversion(measurementType, in);
		Unlock();

		SetMAD(conv);

		Lock();
		measurementType = in;
		Unlock();
	};
	virtual measurement_t GetMeasurement() const {
		measurement_t out;
		Lock();
		out = measurementType;
		Unlock();
		return out;
	};

	virtual void		SetMAD(float multiply_x, float then_add_to_x) {
		SetMAD(vec3(multiply_x, then_add_to_x, 0));
	};
	virtual void		SetMAD(const vec3& in) {
		Lock();
		if (MAD != in) {
			// undo the current values;
			for (int i = 0; i < values.NumRef(); i++)
				values[i] = ((values[i] - MAD.y) / MAD.x);

			MAD = in;

			// set the new values;
			for (int i = 0; i < values.NumRef(); i++)
				values[i] = values[i] * MAD.x + MAD.y;
		}
		Unlock();
	};
	virtual vec3		GetMAD() const {
		vec3 out;
		Lock();
		out = MAD;
		Unlock();
		return out;
	};

	virtual int			AddValue(const u64& time, const type& valueIN, bool useMAD = true) {
		return InsertPair(time, valueIN, useMAD, false);
	};
	virtual int			AddUniqueValue(const u64& time, const type& valueIN, bool useMAD = true) { // slower with guarrantee of uniqueness 
		return InsertPair(time, valueIN, useMAD, true);
	};

	virtual void		RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo);
	virtual void		RemoveIndex(int index) {
		Lock();
		(values).RemoveIndex(index); // safe if outside bounds
		(times).RemoveIndex(index); // safe if outside bounds	
		Unlock();

		SetChanged(true);
	}
	virtual void		Clear() {
		Lock();
		(values).Clear();
		(times).Clear();
		Unlock();

		SetGranularity(512);
		SetCurrentIndex(-1);
		SetChanged(true);
	}

	virtual type		GetCurrentValue(const u64& time) const {
		int i = IndexForTime(time);
		if (i >= GetNumValues()) {
			return ValueForIndex(GetNumValues() - 1);
		}
		else {
			return ValueForIndex(i);
		}
	};
	virtual type		GetCurrentFirstDerivative(const u64& time) const {
		return (ValueForIndex(0) - ValueForIndex(0)); //-V501
	};
	virtual type		GetCurrentSecondDerivative(const u64& time) const {
		return (ValueForIndex(0) - ValueForIndex(0)); //-V501
	};
	virtual float		GetMinimumDecimals() const {
		float decimal = 1.0f;
		int numbs = GetNumValues();
		if (numbs == 0) return 0.0001f;
		float F;
		int numSuccess = 0;

		for (int i = 0; i < numbs; i++) {
			Lock();
			const type& Y = values[i];
			Unlock();

			F = cweeMath::roundNearest((float)Y, decimal);
			if (cweeMath::Fabs((float)(F - Y)) > 0.00001) {
				// too great an error
				decimal /= 10.0f;
				numSuccess = 0;
				if (decimal <= 0.0001f) {
					decimal = 0.0001f;
					break;
				}
			}
			else {
				numSuccess++;
				if (numSuccess > 10) {
					break;
				}
			}
		}
		return decimal;
	};
	virtual u64			GetMinimumTimeStep() const {
		u64 out, prevTime, t;
		int numFailures, numbs;

		numbs = GetNumValues();
		if (numbs <= 1) return 1;

		out = GetMaxTime() - GetMinTime();
		prevTime = std::numeric_limits<u64>::max();
		numFailures = 100;

		Lock();
		for (auto& time : times) {
			t = std::abs(prevTime - time);
			prevTime = time;
			if (t < out && t > 0) {
				out = t;
			}
			else {
				numFailures--;
				if (numFailures <= 0) break;
			}
		}
		Unlock();

		if (out <= 1) out = 1;

		return out;
	};

	virtual type		GetMinValue() const {
		type out;
		out = 0;
		if (GetNumValues() == 0) return out;
		out = std::numeric_limits<type>::max();

		Lock();
		for (auto& value : values) {
			if (value < out) out = value;
		}
		Unlock();

		return out;
	};
	virtual type		GetMaxValue() const {
		type out;
		out = 0;
		if (GetNumValues() == 0) return out;
		out = -std::numeric_limits<type>::max();
		Lock();
		for (auto& value : values) {
			if (value > out) out = value;
		}
		Unlock();
		return out;
	};
	virtual type		GetAvgValue() const {
		type out(0);
		int num(0);

		Lock();
		for (const type& value : values) {
			num++;
			out -= (type)(out / (float)num);
			out += (type)(value / (float)num);
		}
		Unlock();

		return out;
	};

	virtual type		GetMinValue(const u64& start, const u64& end) const {
		type out;
		out = 0;
		int n = GetNumValues();
		if (n == 0) return out;
		out = std::numeric_limits<type>::max();
		for (int i = 0; i < n; i++) {
			if (TimeForIndex(i) >= start && TimeForIndex(i) <= end && ValueForIndex(i) < out)
				out = ValueForIndex(i);
		}
		return out;
	};
	virtual type		GetMaxValue(const u64& start, const u64& end) const {
		type out;
		out = 0;
		int n = GetNumValues();
		if (n == 0) return out;
		out = -std::numeric_limits<type>::max();
		for (int i = 0; i < n; i++) {
			if (TimeForIndex(i) >= start && TimeForIndex(i) <= end && ValueForIndex(i) > out)
				out = ValueForIndex(i);
		}
		return out;
	};
	virtual type		GetAvgValue(const u64& start, const u64& end) const {
		type out;
		out = 0;
		int num(0);
		for (int i = IndexForTime(start); i < GetNumValues(); i++) {
			if (TimeForIndex(i) >= start && TimeForIndex(i) <= end) {
				cweeMath::rollingAverageRef<type>(out, ValueForIndex(i), num);
			}
			else {
				if (TimeForIndex(i) > end) {
					break;
				}
			}
		}
		return out;
	};

	virtual u64			GetAvgTime() const {
		float out(0);
		int num(0);
		Lock();
		for (auto& time : times) {
			cweeMath::rollingAverageRef<type>(out, (float)time, num);
		}
		Unlock();
		return (u64)out;
	};
	virtual u64			GetMaxTime(void) const {
		if (GetNumValues() == 0)
			return 0;
		else
			return TimeForIndex(GetNumValues() - 1);
	}
	virtual u64			GetMinTime(void) const {
		if (GetNumValues() == 0)
			return 0;
		else
			return TimeForIndex(0);
	}

	virtual bool		IsDone(const u64& time) const {
		bool out;
		Lock();
		out = (time >= TimeForIndex(GetNumValues() - 1));
		Unlock();
		return out;
	};
	virtual int			GetNumValues() const {
		int out;
		Lock();
		out = (values).NumRef();
		Unlock();
		return out;
	};
	virtual cweeThreadedList<std::pair<const u64*, const type*>> UnsafeGetKnotSeries(const u64& t0 = -std::numeric_limits < u64>::max(), const u64& t1 = std::numeric_limits < u64>::max()) const {
		int n = UnsafeGetNumValues();
		cweeThreadedList<std::pair<const u64*, const type*>> out(n + 16);
		int minIndex = cweeMath::max(0, UnsafeIndexForTime(t0) - 1);
		int maxIndex = cweeMath::min(n, UnsafeIndexForTime(t1) + 1);

		std::pair<const u64*, const type*> t;
		for (int i = minIndex; i < maxIndex; i++) {
			t.first = UnsafeGetTime(i);
			t.second = UnsafeGetValue(i);

			out.Append(t);
		}

		return out;
	};

	virtual int			UnsafeGetNumValues() const {
		return (values).NumRef();
	};
	virtual void		SetValue(int index, const type& value) {
		Lock();
		if (index >= 0 && index < values.NumRef()) (values)[index] = value;
		Unlock();
	}
	virtual type		GetValue(int index) const {
		return ValueForIndex(index);
	}
	virtual const type* UnsafeGetValue(int index) const {
		return &(values)[index];
	}
	virtual const u64* UnsafeGetTime(int index) const {
		return &(times)[index];
	}
	virtual type* UnsafeGetValue(int index) {
		return &(values)[index];
	}
	virtual u64* UnsafeGetTime(int index) {
		return &(times)[index];
	}
	virtual u64			GetTime(int index) const {
		return TimeForIndex(index);
	}

	virtual u64			GetLengthForTime(const u64& time) const {
		u64 length = 0.0f;
		int index = IndexForTime(time);
		for (int i = 0; i < index; i++) {
			length += RombergIntegral(TimeForIndex(i), TimeForIndex(i + 1), 1);
		}
		length += RombergIntegral(TimeForIndex(index), time, 1);
		return length;
	};
	virtual u64			GetTimeForLength(const u64& length, const u64& epsilon = 0.1f) const {
		int i, index;
		u64* accumLength, totalLength, len0, len1, t, diff;

		if (length <= 0.0f) {
			return TimeForIndex(0);
		}

		int n = GetNumValues();
		accumLength = (u64*)_alloca16(n * sizeof(u64));
		totalLength = 0.0f;
		for (index = 0; index < n - 1; index++) {
			totalLength += GetLengthBetweenKnots(index, index + 1);
			accumLength[index] = totalLength;
			if (length < accumLength[index]) {
				break;
			}
		}


		if (index >= GetNumValues() - 1) {
			return TimeForIndex(GetNumValues() - 1);
		}

		if (index == 0) {
			len0 = length;
			len1 = accumLength[0];
		}
		else {
			len0 = length - accumLength[index - 1];
			len1 = accumLength[index] - accumLength[index - 1];
		}

		// invert the arc length integral using Newton's method
		t = (TimeForIndex(index + 1) - TimeForIndex(index)) * len0 / len1;
		for (i = 0; i < 32; i++) {
			diff = RombergIntegral(TimeForIndex(index), TimeForIndex(index) + t, 1) - len0;
			if (cweeMath::Fabs(diff) <= epsilon) {
				return TimeForIndex(index) + t;
			}
			t -= diff / GetSpeed(TimeForIndex(index) + t);
		}
		return TimeForIndex(index) + t;
	};
	virtual u64			GetLengthBetweenKnots(int i0, int i1) const {
		u64 length = 0.0f;
		for (int i = i0; i < i1; i++) {
			length += RombergIntegral(TimeForIndex(i), TimeForIndex(i + 1), 1);
		}
		return length;
	};
	virtual void		MakeUniform(const u64& totalTime) {
		int i, n;

		Lock();
		n = (times).NumRef() - 1;
		for (i = 0; i <= n; i++) {
			(times)[i] = i * totalTime / n;
		}
		Unlock();

		SetChanged(true);
	};
	virtual void		SetConstantSpeed(const u64& totalTime) {
		int i, j;
		u64* length, totalLength, scale, t;

		Lock();
		int n = (values).NumRef();
		length = (u64*)_alloca16(n * sizeof(u64));
		totalLength = 0.0f;
		for (i = 0; i < n - 1; i++) {
			Unlock();
			length[i] = GetLengthBetweenKnots(i, i + 1);
			Lock();
			totalLength += length[i];
		}
		scale = totalTime / totalLength;
		for (t = 0.0f, i = 0; i < (times).NumRef() - 1; i++) {
			(times)[i] = t;
			t += scale * length[i];
		}
		(times)[(times).NumRef() - 1] = totalTime;
		changed = true;
		Unlock();

	};
	virtual void		ShiftTime(const u64& deltaTime) {
		Lock();
		for (auto& x : times)  x += deltaTime;
		changed = true;
		Unlock();
	};
	virtual void		Translate(const type& translation) {
		Lock();
		for (auto& x : values) x += translation;
		changed = true;
		Unlock();
	};
	virtual type		RombergIntegral(const u64& t0, const u64& t1, const float divisor = 3600.0f, bool snapLeft = false) const {
		type sum;
		sum = 0.0f;
		return sum;
	};

	virtual size_t		MemoryUsed(void) const {
		size_t Bytes = 0;
		Lock();
		Bytes += (times).NumRef() * (sizeof(u64) + sizeof(float));
		Unlock();
		return Bytes;
	}
	virtual int			IndexForTime(const u64& time) const {
		int out;
		Lock();
		out = UnsafeIndexForTime(time);
		Unlock();
		return out;
	};
	virtual int			FindExactX(const u64& time) const {
		int search;

		Lock(); {
			search = UnsafeIndexForTime(time);
			if (search >= values.NumRef() || search < 0) {
				search = -1; // out of bounds
			}
			else if (times[search] == time) {
				// no issues
			}
			else if ((search + 1) < values.NumRef() && times[search + 1] == time) {
				search++; // was up one value for some reason
			}
			else {
				search = -1;
			}
		} Unlock();

		return search;
	};
	virtual int			FindExactY(const type& val) const {
		int out;
		Lock();
		out = values.FindIndex(val);
		Unlock();
		return out;
	};
	virtual int			UnsafeFindExactX(const u64& time) const {
		int search;

		{
			search = UnsafeIndexForTime(time);
			if (search >= values.NumRef() || search < 0) {
				search = -1; // out of bounds
			}
			else if (times[search] == time) {
				// no issues
			}
			else if ((search + 1) < values.NumRef() && times[search + 1] == time) {
				search++; // was up one value for some reason
			}
			else {
				search = -1;
			}
		}

		return search;
	};
	virtual int			UnsafeFindExactY(const type& val) const {
		int out;
		out = values.FindIndex(val);
		return out;
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<const type*> UnsafeSelectValues(std::function<bool(const type&)> predicate) const {
		cweeThreadedList<const type*> out;
		for (auto& x : UnsafeGetValues()) {
			if (predicate(x)) {
				out.Append(&x);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<type*> UnsafeSelectValues(std::function<bool(const type&)> predicate) {
		cweeThreadedList<type*> out;
		for (auto& x : UnsafeGetValues()) {
			if (predicate(x)) {
				out.Append(&x);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<int> SelectValueIndexes(std::function<bool(const type&)> predicate) const {
		cweeThreadedList<int> out;
		int i = 0;

		Lock();
		for (auto& x : UnsafeGetValues()) {
			if (predicate(x)) {
				out.Append(i);
			}
			++i;
		}
		Unlock();

		return out;
	};

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<const u64*> UnsafeSelectTimes(std::function<bool(const u64&)> predicate) const {
		cweeThreadedList<const u64*> out;
		for (auto& x : UnsafeGetTimes()) {
			if (predicate(x)) {
				out.Append(&x);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<u64*> UnsafeSelectTimes(std::function<bool(const u64&)> predicate) {
		cweeThreadedList<u64*> out;
		for (auto& x : UnsafeGetTimes()) {
			if (predicate(x)) {
				out.Append(&x);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<int> SelectTimeIndexes(std::function<bool(const u64&)> predicate) const {
		cweeThreadedList<int> out;
		int i = 0;

		Lock();
		for (auto& x : UnsafeGetTimes()) {
			if (predicate(x)) {
				out.Append(i);
			}
			++i;
		}
		Unlock();

		return out;
	};

	/*! Get Knot value of Pattern at index */
	virtual std::pair<u64, type>		operator[](int index) const {
		return At(index);
	};

	/*! Get Knot value of Pattern at index */
	virtual std::pair<u64, type>		At(int index) const {
		return std::make_pair(TimeForIndex(index), ValueForIndex(index));
	};

	/*! Get Knot value of Pattern at index */
	virtual std::pair<const u64*, const type*>		UnsafeAt(int index) const {
		return std::make_pair(UnsafeGetTime(index), UnsafeGetValue(index));
	};
	virtual std::pair<u64*, type*>		UnsafeAt(int index) {
		return std::make_pair(UnsafeGetTime(index), UnsafeGetValue(index));
	};

	virtual void		Lock() const
		//{lock.Lock();}
		;
	virtual void		Unlock() const
		//{ lock.Unlock();}
		;

	virtual u64			TimeForIndex(int index) const {
		u64 out = 0;

		Lock();
		int n = (times).NumRef();
		if (index >= 0 && index < n)
			out = (times)[index];
		else if (index < 0 && n > 1)
			out = (times)[0];// +index * ((times)[1] - (times)[0]);
		else if (index > (n - 1) && n > 0)
			out = (times)[n - 1];// + (index - (n - 1)) * ((times)[(n-1)] - (times)[(n - 2)]);		
		Unlock();

		return out;
	};
	virtual type		ValueForIndex(int index) const {
		type out;
		out = 0;

		Lock();
		int n = (values).NumRef();
		if (index >= 0 && index < n)
			out = (values)[index];
		else if (index < 0 && n > 1)
			out = (values)[0];// +index * ((times)[1] - (times)[0]);
		else if (index > (n - 1) && n > 0)
			out = (values)[n - 1];// + (index - (n - 1)) * ((times)[(n-1)] - (times)[(n - 2)]);		
		Unlock();

		return out;
	};
	/*! <X,X,X> = pattern.ValueQuantiles({ 0.25, 0.5, 0.75 }); */
	virtual cweeThreadedList<float>	ValueQuantiles(const cweeThreadedList<float>& probs) {
		cweeThreadedList<float> quantiles;
		float poi, datLeft, datRight, quantile;
		size_t left, right;
		int n = GetNumValues();

		if (n == 0)
		{
			return quantiles;
		}

		if (1 == n)
		{
			quantiles.Append(GetValue(0));
			return quantiles;
		}

		Lock();
		cweeThreadedList<float> data = values; // copy data
		Unlock();

		data.Sort(); // sort data

		for (size_t i = 0; i < probs.NumRef(); ++i)
		{
			poi = (1.0f - probs[i]) * -0.5f + probs[i] * (data.NumRef() - 0.5f);
			left = std::max(int64_t(std::floor(poi)), int64_t(0));
			right = std::min(int64_t(std::ceil(poi)), int64_t(data.NumRef() - 1.0f));
			datLeft = data[left];
			datRight = data[right];
			quantile = (1.0f - (poi - left)) * datLeft + (poi - left) * datRight;
			quantiles.Append(quantile);
		}
		return quantiles;
	}

protected:
	virtual u64			GetSpeed(const u64& time) const {
		int i;
		u64 speed = 0;
		type value;

		value = GetCurrentFirstDerivative(time);
		speed += value * value;
		return cweeMath::Sqrt(speed);
	};
	virtual bool		GetChanged() const {
		bool out;
		Lock();
		out = changed;
		Unlock();
		return out;
	};
	virtual void		SetChanged(bool in) const {
		Lock();
		changed = in;
		Unlock();
	};
	virtual int			GetCurrentIndex() const {
		int out;
		Lock();
		out = currentIndex;
		Unlock();
		return out;
	};
	virtual void		SetCurrentIndex(int in) {
		Lock();
		currentIndex = in;
		Unlock();
	};
	virtual int			InsertPair(const u64& time, const type& valueIN, bool useMAD = true, bool unique = false) {
		int i = 0;
		if (unique) {

			type value;
			value = 0;
			if (useMAD == true) {
				Lock();
				value = valueIN * MAD.x + MAD.y;
				Unlock();
			}
			else value = valueIN;

			Lock(); {
				i = UnsafeIndexForTime(time);
				if (((i != 0) && (i < values.NumRef()) && (times[i] == time))) {
					values[i] = value;
					// finished;
					Unlock();
					SetChanged(true);
					return i;
				}
				u64 minTime; {
					if (times.NumRef() == 0) minTime = 0;
					else minTime = times[0];
				}
				if (values.NumRef() == 0 || minTime != time) {
					(times).Insert(time, i);
					(values).Insert(value, i);
					if ((values.NumRef() + 1) >= (Granularity * GRANULARITY_SCALER)) Granularity = Granularity * GRANULARITY_SCALER;
				}
				else {
					values[0] = value;
					i = 0;
				}
			} Unlock();
			SetChanged(true);
			return i;
		}
		else {
			type value;
			value = 0;
			if (useMAD == true) {
				Lock();
				value = valueIN * MAD.x + MAD.y;
				Unlock();
			}
			else value = valueIN;

			Lock(); {
				i = UnsafeIndexForTime(time);
				(times).Insert(time, i);
				(values).Insert(value, i);
				if ((values.NumRef() + 1) >= (Granularity * GRANULARITY_SCALER)) Granularity = Granularity * GRANULARITY_SCALER;
			} Unlock();

			SetChanged(true);
			return i;
		}
	};
	virtual int			UnsafeIndexForTime(const u64& time) const {
		if (times.NumRef() <= 0) return 0; // there is no other data...

		int len, mid, offset;
		bool res;

		if (currentIndex >= 0 && currentIndex < (times).NumRef()) {
			// use the cached index if it is still valid
			if (currentIndex == 0) {
				if (time <= (times)[currentIndex]) {
					return currentIndex;
				}
			}
			else if (currentIndex == (times).NumRef()) {
				if (time > (times)[currentIndex - 1]) {
					return currentIndex;
				}
			}
			else if (time > (times)[currentIndex - 1] && time <= (times)[currentIndex]) {
				return currentIndex;
			}
			else if (time > (times)[currentIndex] && (currentIndex + 1 == (times).NumRef() || time <= (times)[currentIndex + 1])) {
				// use the next index
				currentIndex++;
				return currentIndex;
			}
		}

		// use binary search to find the index for the given time
		len = (times).NumRef();
		mid = len;
		offset = 0;
		res = false;
		u64* sample;
		while (mid > 0) {
			mid = len >> 1;
			// OPTIMIZED ORDERING
			sample = &times[offset + mid];
			if (time >= *sample)
			{
				offset += mid;
				len -= mid;
				res = true;
				if (time == *sample) {
					return offset;
				}
			}
			else
			{
				len -= mid;
				res = false;
			}
		}
		currentIndex = offset + (int)res;
		return currentIndex;
	};

private:
	int					Granularity = 16;
	cweeStr				Name; // = "No Name";
	int					identity = -1;
	mutable keyList		times;				// knot Times
	mutable valueList		values;				// knot Values	
	value_t				valueType = value_t::_END_;
	measurement_t		measurementType = measurement_t::_end_;
	vec3				MAD = vec3(1, 0, 0);
	mutable int			currentIndex = -1;		// cached index for fast lookup
	mutable bool		changed = false;		// set whenever the curve changes
	// mutable cweeSysMutex*	lock = nullptr;
	mutable cweeSysMutex lock;
};

/*!
===============================================================================
	Spline base template.
===============================================================================
*/
template< class type >
class cweePattern_Spline : public cweePattern<type> {

public:
	cweePattern_Spline() {
		SetBoundaryType(boundary_t::BT_LOOP);
		SetInterpolationType(interpolation_t::LINEAR);
		SetCloseTime(0.0f);
	};
	~cweePattern_Spline() {};

	virtual bool		IsDone(const u64& time) const {
		bool out;
		out = (GetBoundaryType() != boundary_t::BT_CLOSED && time >= this->TimeForIndex(this->GetNumValues() - 1));
		return out;
	};

	virtual void		SetBoundaryType(const boundary_t& bt) {
		this->Lock(); {
			boundaryType = bt;
		} this->Unlock();
		this->SetChanged(true);
	}
	virtual boundary_t	GetBoundaryType() const {
		boundary_t out;
		this->Lock();
		out = boundaryType;
		this->Unlock();
		return out;
	}

	virtual void		SetInterpolationType(const interpolation_t& it) {
		this->Lock(); {
			interpolationType = it;
		} this->Unlock();
		this->SetChanged(true);
	}
	virtual interpolation_t	GetInterpolationType() const {
		interpolation_t out;
		this->Lock();
		out = interpolationType;
		this->Unlock();
		return out;
	}

	virtual void		SetCloseTime(const u64& t) {
		this->Lock(); {
			closeTime = t;
		} this->Unlock();
		this->SetChanged(true);
	}
	virtual u64			GetCloseTime() const {
		u64 out;
		this->Lock();
		out = boundaryType == boundary_t::BT_CLOSED ? closeTime : 0.0f;
		this->Unlock();
		return out;
	}

	//virtual type		ValueForIndex(int index) const {
	//	type out;
	//	out = 0;
	//	this->Lock();
	//	int n = this->UnsafeGetValues().NumRef();
	//	int num = this->UnsafeGetValues().NumRef()-1;
	//	if (index >= 0 && index < n) {
	//		out = this->UnsafeGetValues()[index];
	//	}
	//	else if (index < 0 && n > 1) {
	//		if (boundaryType == BT_CLOSED) {
	//			out = this->UnsafeGetValues()[cweeMath::min(n-1, cweeMath::max(0, num + index % num))];
	//		}
	//		else {
	//			out = this->UnsafeGetValues()[0] + index * (this->UnsafeGetValues()[1] - this->UnsafeGetValues()[0]);
	//		}
	//	}
	//	else if ((index >= n) && n > 1) {
	//		if (boundaryType == BT_CLOSED) {
	//			out = this->UnsafeGetValues()[cweeMath::min(n - 1, cweeMath::max(0, index % num))];
	//		}
	//		else {
	//			out = this->UnsafeGetValues()[n-1] + (1 + index - n) * (this->UnsafeGetValues()[n - 1] - this->UnsafeGetValues()[n-2]);				
	//		}
	//	}
	//	this->Unlock();
	//	return out;
	//};

	//virtual u64			TimeForIndex(int index) const {
	//	u64 out;
	//	out = 0;
	//	this->Lock();
	//	int num = this->UnsafeGetTimes().NumRef();
	//	if (index >= 0 && index < num) {
	//		out = this->UnsafeGetTimes()[index];
	//	}
	//	else if (index < 0 && num > 1) {
	//		if (boundaryType == BT_CLOSED) {
	//			out = (index / num) * (this->UnsafeGetTimes()[num-1] + GetCloseTime()) - (this->UnsafeGetTimes()[num - 1] + GetCloseTime() - this->UnsafeGetTimes()[num + index % num]);
	//		}
	//		else {
	//			out = this->UnsafeGetTimes()[0] + index * (this->UnsafeGetTimes()[1] - this->UnsafeGetTimes()[0]);
	//		}
	//	}
	//	else if ((index >= num) && num > 1) {
	//		if (boundaryType == BT_CLOSED) {
	//			out = (index / num) * (this->UnsafeGetTimes()[num - 1] + GetCloseTime()) + this->UnsafeGetTimes()[index % num];
	//		}
	//		else {
	//			out = this->UnsafeGetTimes()[num - 1] + (1 + index - num) * (this->UnsafeGetTimes()[num - 1] - this->UnsafeGetTimes()[num - 2]);
	//		}
	//	}
	//	this->Unlock();
	//	return out;		
	//};

	virtual type		ValueForIndex(int index) const {
		type out;
		int numV;
		int n;

		this->Lock();
		numV = this->UnsafeGetTimes().NumRef();
		this->Unlock();

		n = numV - 1;

		if (n < 0) return 0;

		this->Lock();
		if (index < 0) {
			// if (boundaryType == BT_CLOSED) {
			out = this->UnsafeGetValues()[0];
			// out = this->UnsafeGetValues()[numV + index % numV];
		// }
		// else {
		// 	out = this->UnsafeGetValues()[0] + index * (this->UnsafeGetValues()[1] - this->UnsafeGetValues()[0]);
		// }
		}
		else if (index > n) {
			// if (boundaryType == BT_CLOSED) {
			out = this->UnsafeGetValues()[n];
			// out = this->UnsafeGetValues()[index % numV];
		// }
		// else {
		// 	out = this->UnsafeGetValues()[n] + (index - n) * (this->UnsafeGetValues()[n] - this->UnsafeGetValues()[n - 1]);
		// }
		}
		else {
			out = this->UnsafeGetValues()[index];
		}
		this->Unlock();

		return out;
	};
	virtual u64			TimeForIndex(int index) const {
		u64 out;
		int numV;
		int n;

		this->Lock();
		numV = this->UnsafeGetTimes().NumRef();
		n = numV - 1;
		if (n < 0) {
			this->Unlock();
			return 0;
		}

		if (index < 0) {
			if (boundaryType == boundary_t::BT_CLOSED) {
				out = this->UnsafeGetTimes()[0];
				//out = (index / numV) * (this->UnsafeGetTimes()[n] + closeTime) - (this->UnsafeGetTimes()[n] + closeTime - this->UnsafeGetTimes()[numV + index % numV]);
			}
			else {
				out = this->UnsafeGetTimes()[0] + index * (this->UnsafeGetTimes()[1] - this->UnsafeGetTimes()[0]);
			}
		}
		else if (index > n) {
			if (boundaryType == boundary_t::BT_CLOSED) {
				out = this->UnsafeGetTimes()[n];
				//out = (index / numV) * (this->UnsafeGetTimes()[n] + closeTime) + this->UnsafeGetTimes()[index % numV];
			}
			else {
				out = this->UnsafeGetTimes()[n] + (index - n) * (this->UnsafeGetTimes()[n] - this->UnsafeGetTimes()[n - 1]);
			}
		}
		else {
			out = this->UnsafeGetTimes()[index];
		}
		this->Unlock();

		return out;
	};

	virtual u64			ClampedTime(const u64& t) const {
		if (GetBoundaryType() == boundary_t::BT_CLAMPED) {
			u64 mT = this->GetMinTime();
			if (t <= mT) {
				return mT;
			}
			else {
				mT = this->GetMaxTime();
				if (t > mT)
					return mT;
			}
		}
		return t;
	};
	virtual u64			LoopedTime(const u64& t, bool forceLoop = false) const {
		if (IsLooped() || forceLoop) {
			u64 minTime, maxTime, len, currentTime;
			minTime = this->GetMinTime();
			if (cweeMath::Fabs(t - minTime) < 1.01) return t;
			maxTime = this->GetMaxTime();
			if (cweeMath::Fabs(t - maxTime) < 1.01) return t;
			len = (maxTime - minTime);
			if (len > 0) {
				if (t < minTime) {
					currentTime = t;
					while (currentTime < minTime) currentTime += len;
					return currentTime;
				}
				if (t > maxTime) {
					currentTime = t;
					while (currentTime > maxTime) currentTime -= len;
					return currentTime;
				}
			}
		}
		return t;
	};

	virtual void		Clear() {
		this->Lock();
		this->UnsafeGetValues().Clear();
		this->UnsafeGetTimes().Clear();
		this->Unlock();

		this->SetGranularity(512);
		this->SetCurrentIndex(-1);
		this->SetChanged(true);

		this->SetCloseTime(0);
		this->SetInterpolationType(interpolation_t::LINEAR);
		this->SetBoundaryType(boundary_t::BT_FREE);
	}

protected:
	virtual bool		IsLooped(void) const {
		return (GetBoundaryType() == boundary_t::BT_LOOP);
	};

private:
	boundary_t			boundaryType = boundary_t::BT_FREE;
	interpolation_t		interpolationType = interpolation_t::LINEAR;
	u64					closeTime = 0;
};

/*!
===============================================================================
	Uniform Cubic Interpolating Spline template.
	The curve goes through all the knots.
===============================================================================
*/
template< class type >
class cweePattern_CatmullRomSpline : public cweePattern_Spline<type> {
public:

	/*!
	Constructor
	*/
	cweePattern_CatmullRomSpline() {};
	cweePattern_CatmullRomSpline(const cweePattern_CatmullRomSpline<type>& source) {
		Copy(source);
	};
	cweePattern_CatmullRomSpline(const cweeThreadedList<std::pair<u64, type>>& data) {
		this->Clear();
		AcceptNewData(data);
	};

	/*!
	Destructor
	*/
	~cweePattern_CatmullRomSpline() {
		Clear();
	};

	void Reserve(int num) {
		this->SetGranularity(num);
	};

	virtual void		Clear() {
		this->Lock();
		this->UnsafeGetValues().Clear();
		this->UnsafeGetTimes().Clear();
		this->Unlock();

		this->SetGranularity(512);
		this->SetCurrentIndex(-1);
		this->SetChanged(true);

		this->SetCloseTime(0);
		this->SetInterpolationType(interpolation_t::LINEAR);
		this->SetBoundaryType(boundary_t::BT_FREE);
	}

	/*!
	Copy constructor
	*/
	void												operator=(const cweePattern_CatmullRomSpline<type>& source) {
		Copy(source);
	};

	/*!
	Copy constructor, data only
	*/
	void												operator=(const cweeThreadedList<std::pair<u64, type>>& data) {
		this->Clear();
		AcceptNewData(data);
	};

	/*!
	Replace pattern with value from a single float
	*/
	void												operator=(float data) {
		this->Clear();
		this->AddValue(0, data);
	};

	operator cweeThreadedList<std::pair<u64, type>>() const {
		return GetKnotSeries();
	};
	operator cweeThreadedList<std::pair<u64, type>>() {
		return GetKnotSeries();
	};

	friend cweePattern_CatmullRomSpline<type>			operator*(const cweePattern_CatmullRomSpline<type>& a, float b) {
		cweePattern_CatmullRomSpline<type> result(a);
		result *= b;
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator*(float b, const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> result(a);
		result *= b;
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator/(const cweePattern_CatmullRomSpline<type>& a, float b) {
		cweePattern_CatmullRomSpline<type> result(a);
		result /= b;
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator/(float b, const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> result(a);
		for (std::pair<u64, float>& x : result.GetKnotSeries())
			result.AddUniqueValue(x.first, b / (x.second + cweeMath::EPSILON));
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator+(const cweePattern_CatmullRomSpline<type>& a, float b) {
		cweePattern_CatmullRomSpline<type> result(a);
		result += b;
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator+(float b, const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> result(a);
		result += b;
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator-(const cweePattern_CatmullRomSpline<type>& a, float b) {
		cweePattern_CatmullRomSpline<type> result(a);
		result -= b;
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator-(float b, const cweePattern_CatmullRomSpline<type>& a) {
		// b - a = -1(a-b);
		cweePattern_CatmullRomSpline<type> result(a);
		result -= b;
		result *= -1;
		return result;
	};
	cweePattern_CatmullRomSpline<type>& operator*=(float a) {
		this->Lock();
		for (auto& x : this->UnsafeGetValues()) {
			x *= a;
		}
		this->Unlock();

		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator/=(float a) {
		this->Lock();
		for (auto& x : this->UnsafeGetValues()) {
			x /= (a + cweeMath::EPSILON);
		}
		this->Unlock();
		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator+=(float a) {
		this->Lock();
		for (auto& x : this->UnsafeGetValues()) {
			x += a;
		}
		this->Unlock();
		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator-=(float a) {
		this->Lock();
		for (auto& x : this->UnsafeGetValues()) {
			x -= a;
		}
		this->Unlock();
		return *this;
	};

	/*! adds the content of two patterns together without sub-sampling. Either the knots line-up perfectly (and therfore get added together) or they don't and become new values. */
	cweePattern_CatmullRomSpline<type>& Accumulate(const cweePattern_CatmullRomSpline<type>& a) {
		int n, i, j;
		bool success;

		a.Lock(); {
			n = a.UnsafeGetNumValues();
			for (i = 0; i < n; i++) {
				const type* p = a.UnsafeGetValue(i);
				const u64* t = a.UnsafeGetTime(i);
				if (p && t) {
					success = false;
					this->Lock();
					j = this->UnsafeFindExactX(*t);
					if (j >= 0) {
						this->UnsafeGetValues()[j] += *p;
						success = true;
					}
					this->Unlock();
					if (!success) this->AddUniqueValue(*t, *p);
				}
			}
		} a.Unlock();

		return *this;
	};

	cweePattern_CatmullRomSpline<type>& operator+=(const std::pair<u64, type>& a) {
		int i = this->FindExactX(a.first);
		bool success = false;
		if (i >= 0) {
			this->Lock();
			if (this->UnsafeGetValues().Num() < i) {
				this->UnsafeGetValues()[i] += a.second;
				success = true;
			}
			this->Unlock();
		}
		if (!success) this->AddUniqueValue(a.first, a.second);
		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator-=(const std::pair<u64, type>& a) {
		int i = this->FindExactX(a.first);
		bool success = false;
		if (i < 0) {
			this->Lock();
			if (this->UnsafeGetValues().Num() < i) {
				this->UnsafeGetValues()[i] -= a.second;
				success = true;
			}
			this->Unlock();
		}
		if (!success) this->AddUniqueValue(a.first, a.second);

		return *this;
	};

	friend cweePattern_CatmullRomSpline<type>			operator*(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		cweePattern_CatmullRomSpline<type> result(a); result.Clear();
		for (std::pair<u64, float>& x : a.GetKnotSeries()) result.AddUniqueValue(x.first, x.second + b.GetCurrentValue(x.first));
		for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) + x.second);

		result.RemoveUnnecessaryKnots();
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator/(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		cweePattern_CatmullRomSpline<type> result(a); result.Clear();
		for (std::pair<u64, float>& x : a.GetKnotSeries()) result.AddUniqueValue(x.first, x.second / (b.GetCurrentValue(x.first) + cweeMath::EPSILON));
		for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) / (x.second + cweeMath::EPSILON));
		result.RemoveUnnecessaryKnots();
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator+(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		cweePattern_CatmullRomSpline<type> result(a); result.Clear();
		for (std::pair<u64, float>& x : a.GetKnotSeries()) result.AddUniqueValue(x.first, x.second + b.GetCurrentValue(x.first));
		for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) + x.second);

		//u64 mint_a = a.GetMinTime();
		//u64 mint_b = b.GetMinTime();
		//u64 maxt_a = a.GetMaxTime();
		//u64 maxt_b = b.GetMaxTime();

		//auto overlap_min = mint_a > mint_b ? mint_a : mint_b;
		//auto overlap_max = maxt_a < maxt_b ? maxt_a : maxt_b;

		//cweePattern_CatmullRomSpline<type> result;
		//if (overlap_max > overlap_min) { // they do, in fact, overlap 
		//	for (std::pair<u64, float>& x : a.GetKnotSeries(overlap_min, overlap_max)) result.AddUniqueValue(x.first, x.second + b.GetCurrentValue(x.first));
		//	for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_min, overlap_max)) result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) + x.second);

		//	if (mint_b < mint_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(0, overlap_min)) result.AddUniqueValue(x.first, x.second); }
		//	else { for (std::pair<u64, float>& x : a.GetKnotSeries(0, overlap_min)) result.AddUniqueValue(x.first, x.second); }
		//	if (maxt_b > maxt_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_max)) result.AddUniqueValue(x.first, x.second); }
		//	else { for (std::pair<u64, float>& x : a.GetKnotSeries(overlap_max)) result.AddUniqueValue(x.first, x.second); }
		//}
		//else { // they do not overlap 
		//	for (std::pair<u64, float>& x : a.GetKnotSeries()) result.AddUniqueValue(x.first, x.second);
		//	for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, x.second);
		//}

		result.RemoveUnnecessaryKnots();
		return result;
	};
	friend cweePattern_CatmullRomSpline<type>			operator-(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		cweePattern_CatmullRomSpline<type> result(a); result.Clear();
		for (std::pair<u64, float>& x : a.GetKnotSeries()) result.AddUniqueValue(x.first, x.second - b.GetCurrentValue(x.first));
		for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) - x.second);

		//u64 mint_a = a.GetMinTime();
		//u64 mint_b = b.GetMinTime();
		//u64 maxt_a = a.GetMaxTime();
		//u64 maxt_b = b.GetMaxTime();

		//auto overlap_min = mint_a > mint_b ? mint_a : mint_b;
		//auto overlap_max = maxt_a < maxt_b ? maxt_a : maxt_b;

		//cweePattern_CatmullRomSpline<type> result;
		//if (overlap_max > overlap_min) { // they do, in fact, overlap 
		//	for (std::pair<u64, float>& x : a.GetKnotSeries(overlap_min, overlap_max)) result.AddUniqueValue(x.first, x.second - b.GetCurrentValue(x.first));
		//	for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_min, overlap_max)) result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) - x.second);

		//	if (mint_b < mint_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(0, overlap_min)) result.AddUniqueValue(x.first, -x.second); }
		//	else { for (std::pair<u64, float>& x : a.GetKnotSeries(0, overlap_min)) result.AddUniqueValue(x.first, x.second); }
		//	if (maxt_b > maxt_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_max)) result.AddUniqueValue(x.first, -x.second); }
		//	else { for (std::pair<u64, float>& x : a.GetKnotSeries(overlap_max)) result.AddUniqueValue(x.first, x.second); }
		//}
		//else { // they do not overlap 
		//	for (std::pair<u64, float>& x : a.GetKnotSeries()) result.AddUniqueValue(x.first, x.second);
		//	for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, x.second);
		//}

		result.RemoveUnnecessaryKnots();
		return result;
	};
	cweePattern_CatmullRomSpline<type>& operator*=(const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> pat = *this; float val;
		//pat.SetInterpolationType(interpolation_t::LINEAR);
		{
			auto thisKnotSeries = pat.GetKnotSeries();
			for (std::pair<u64, float>& x : thisKnotSeries) {
				val = x.second * a.GetCurrentValue(x.first);
				this->AddUniqueValue(x.first, val);
			}
		}
		{
			auto thatKnotSeries = a.GetKnotSeries();
			for (std::pair<u64, float>& x : thatKnotSeries) {
				val = pat.GetCurrentValue(x.first) * x.second;
				this->AddUniqueValue(x.first, val);
			}
		}
		RemoveUnnecessaryKnots();
		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator/=(const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> pat = *this; float val;
		//pat.SetInterpolationType(interpolation_t::LINEAR);
		{
			auto thisKnotSeries = pat.GetKnotSeries();
			for (std::pair<u64, float>& x : thisKnotSeries) {
				val = x.second / (a.GetCurrentValue(x.first) + cweeMath::EPSILON);
				this->AddUniqueValue(x.first, val);
			}
		}
		{
			auto thatKnotSeries = a.GetKnotSeries();
			for (std::pair<u64, float>& x : thatKnotSeries) {
				val = pat.GetCurrentValue(x.first) / (x.second + cweeMath::EPSILON);
				this->AddUniqueValue(x.first, val);
			}
		}
		RemoveUnnecessaryKnots();
		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator+=(const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> pat = *this; float val;
		{
			auto thisKnotSeries = pat.GetKnotSeries();
			for (std::pair<u64, float>& x : thisKnotSeries) {
				val = x.second + a.GetCurrentValue(x.first);
				this->AddUniqueValue(x.first, val);
			}
		}
		{
			auto thatKnotSeries = a.GetKnotSeries();
			for (std::pair<u64, float>& x : thatKnotSeries) {
				val = pat.GetCurrentValue(x.first) + x.second;
				this->AddUniqueValue(x.first, val);
			}
		}


		//u64 mint_a = a.GetMinTime();
		//u64 mint_b = b.GetMinTime();
		//u64 maxt_a = a.GetMaxTime();
		//u64 maxt_b = b.GetMaxTime();

		//auto overlap_min = mint_a > mint_b ? mint_a : mint_b;
		//auto overlap_max = maxt_a < maxt_b ? maxt_a : maxt_b;
		//if (overlap_max > overlap_min) { // they do, in fact, overlap 
		//	for (std::pair<u64, float>& x : a.GetKnotSeries(overlap_min, overlap_max)) AddUniqueValue(x.first, x.second + b.GetCurrentValue(x.first));
		//	for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_min, overlap_max)) AddUniqueValue(x.first, a.GetCurrentValue(x.first) + x.second);

		//	if (mint_b < mint_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(0, overlap_min)) AddUniqueValue(x.first, x.second); }
		//	if (maxt_b > maxt_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_max)) AddUniqueValue(x.first, x.second); }
		//}
		//else { // they do not overlap 
		//	for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, x.second);
		//}

		RemoveUnnecessaryKnots();
		return *this;
	};
	cweePattern_CatmullRomSpline<type>& operator-=(const cweePattern_CatmullRomSpline<type>& a) {
		cweePattern_CatmullRomSpline<type> pat = *this; float val;
		{
			auto thisKnotSeries = pat.GetKnotSeries();
			for (std::pair<u64, float>& x : thisKnotSeries) {
				val = x.second - a.GetCurrentValue(x.first);
				this->AddUniqueValue(x.first, val);
			}
		}
		{
			auto thatKnotSeries = a.GetKnotSeries();
			for (std::pair<u64, float>& x : thatKnotSeries) {
				val = pat.GetCurrentValue(x.first) - x.second;
				this->AddUniqueValue(x.first, val);
			}
		}

		//u64 mint_a = a.GetMinTime();
		//u64 mint_b = b.GetMinTime();
		//u64 maxt_a = a.GetMaxTime();
		//u64 maxt_b = b.GetMaxTime();

		//auto overlap_min = mint_a > mint_b ? mint_a : mint_b;
		//auto overlap_max = maxt_a < maxt_b ? maxt_a : maxt_b;
		//if (overlap_max > overlap_min) { // they do, in fact, overlap 
		//	for (std::pair<u64, float>& x : a.GetKnotSeries(overlap_min, overlap_max)) AddUniqueValue(x.first, x.second - b.GetCurrentValue(x.first));
		//	for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_min, overlap_max)) AddUniqueValue(x.first, a.GetCurrentValue(x.first) - x.second);

		//	if (mint_b < mint_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(0, overlap_min)) AddUniqueValue(x.first, -x.second); }
		//	if (maxt_b > maxt_a) { for (std::pair<u64, float>& x : b.GetKnotSeries(overlap_max)) AddUniqueValue(x.first, -x.second); }
		//}
		//else { // they do not overlap 
		//	for (std::pair<u64, float>& x : b.GetKnotSeries()) result.AddUniqueValue(x.first, -x.second);
		//}

		RemoveUnnecessaryKnots();
		return *this;
	};

	/*!
	Return true if the knot collections within the LHS and RHS are NOT equal
	*/
	friend bool											operator!=(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		return !operator==(a, b);
	};
	/*!
	Return true if the effective knot collections within the LHS and RHS are equal (can have fewer or more knots - but if the Timeseries is
	effectively the same, then it is allowed). Result *is* dependant on the interpolation type.
	*/
	friend bool											operator==(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		for (std::pair<u64, type>& pair : a.GetKnotSeries()) {
			if (pair.second != b.GetCurrentValue(pair.first)) return false;
		}
		for (std::pair<u64, type>& pair : b.GetKnotSeries()) {
			if (pair.second != a.GetCurrentValue(pair.first)) return false;
		}
		return true;
	};
	/*!
	Return true if the smallest value of the LHS is greater than the largest value of the RHS
	*/
	friend bool											operator>(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		return (a.GetMinValue() > b.GetMaxValue() ? true : false);
	};
	/*!
	Return true if the largest value of the LHS is less than the smallest value of the RHS
	*/
	friend bool											operator<(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		return (a.GetMaxValue() < b.GetMinValue() ? true : false);
	};
	/*!
	Return true if the smallest value of the LHS is greater than or equal to the largest value of the RHS
	*/
	friend bool											operator>=(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		return (a.GetMinValue() >= b.GetMaxValue() ? true : false);
	};
	/*!
	Return true if the largest value of the LHS is less than or equal to the smallest value of the RHS
	*/
	friend bool											operator<=(const cweePattern_CatmullRomSpline<type>& a, const cweePattern_CatmullRomSpline<type>& b) {
		return (a.GetMaxValue() <= b.GetMinValue() ? true : false);
	};

	/*!
	Get Time Series of Pattern between date-Times at specified resolution
	*/
	cweeThreadedList<std::pair<u64, type>>				operator()(const u64& start, const u64& end, const u64& resolution) const {
		return GetTimeSeries(start, end, resolution);
	};
	/*!
	Get Knot Series of Pattern between date-Times
	*/
	cweeThreadedList<std::pair<u64, type>>				operator()(const u64& start, const u64& end) const {
		return GetKnotSeries(start, end);
	};
	/*!
	Get Spline value of Pattern at date-time
	*/
	float												operator()(const u64& time) const {
		return GetCurrentValue(time);
	};

	/*!
	Clamp the y-axis Values such that they do not exceed the maximum and minimum Values.
	*/
	void												ClampValues(const type& min, const type& max) {
		this->Lock();
		for (auto& x : this->UnsafeGetValues()) {
			x = (type)cweeMath::Fmax((float)cweeMath::Fmin((float)x, (float)max), (float)min);
		}
		this->Unlock();
	};

	/*
	Serialize Knot Values
	*/
	cweeStr												SerializeKnots(int percentToRemove = -1) const {
		cweeStr delim = ":Knots_DELIM:";
		cweeStr out;
		cweeStr saver;

		if (percentToRemove <= 0) {
			RemoveUnnecessaryKnots();
			for (auto& x : GetKnotSeries()) {
				if (x.second == 0.0f) {
					saver = cweeStr(x.first / serializationTimeConverter);
					saver.Append(",");
					out.AddToDelimiter(saver, delim);
				}
				else {
					saver = (cweeStr)(x.first / serializationTimeConverter) + "," + (cweeStr)x.second;
					out.AddToDelimiter(saver, delim);
				}
			}
		}
		else {
			ReduceMemory((float)percentToRemove);
			for (auto& x : GetKnotSeries()) {
				if (x.second == 0.0f) {
					saver = cweeStr(x.first / serializationTimeConverter);
					saver.Append(",");
					out.AddToDelimiter(saver, delim);
				}
				else {
					saver = (cweeStr)(x.first / serializationTimeConverter) + "," + (cweeStr)x.second;
					out.AddToDelimiter(saver, delim);
				}
			}
		}

		if (1) {
			out.Replace(delim, "|");
		}

		return out;
	}

	/*
	Deserialize Knot Values
	*/
	void												DeserializeKnots(cweeStr& in) {
		const cweeStr delim(":Knots_DELIM:");
		this->Lock();
		this->UnsafeGetTimes().Clear();
		this->UnsafeGetValues().Clear();
		this->Unlock();

		if (1) {
			in.Replace("|", delim);
		}

		if (!in.IsEmpty()) {
			cweeParser obj(in, delim, true);
			in.Clear();
			this->SetGranularity(obj.getNumVars() + 16);
			int finder(-1); cweeStr left, right; u64 a; type b; bool t = false;
			for (auto& x : obj) {
				finder = x.Find(',');
				if (finder != -1) {
					x.Mid(0, finder, left);
					x.Mid(finder + 1, x.Length(), right);
					a = (((u64)left) * serializationTimeConverter);
					if (right.IsEmpty())
						b = 0.0f;
					else
						b = (type)right;
					this->AddValue(a, b, t);
				}
			}
		}
	}

	/*!
	Serialize Pattern
	*/
	cweeStr												Serialize(int percentToRemove = -1)  const {
		cweeStr delim = ":PATTERN_in_DELIM:";
		cweeStr out;

		out.AddToDelimiter((int)this->GetSpecifier(), delim); // 0
		out.AddToDelimiter((cweeStr)this->GetName(), delim); // 1
		out.AddToDelimiter(this->GetMAD().ToString(), delim); // 2
		out.AddToDelimiter((int)(this->GetInterpolationType()), delim); // 3
		out.AddToDelimiter((int)this->GetBoundaryType(), delim); // 4
		out.AddToDelimiter((cweeStr)SerializeKnots(percentToRemove), delim); // 5

		out.AddToDelimiter("", delim); // 6	
		vec3 t = vec3(0,0,0); out.AddToDelimiter(t.ToString(), delim); // 7

		out.AddToDelimiter((int)this->GetCharacteristic(), delim); // 8
		out.AddToDelimiter((int)this->GetMeasurement(), delim); // 9
			
		out.AddToDelimiter(" ", delim); // 10
		out.AddToDelimiter(" ", delim); // 11
		out.AddToDelimiter(" ", delim); // 12

		return out;
	};

	/*!
	Deserialize Pattern
	*/
	void												Deserialize(cweeStr& in) {
		cweeParser obj(in, ":PATTERN_in_DELIM:", true);
		in.Clear();

		this->Clear(); // cleared
		if (obj.getNumVars() >= 19) {
			this->SetName(obj[0]);
			this->SetSpecifier((int)obj[1]);
			this->SetMAD(vec3((float)obj[2], (float)obj[3], (float)obj[4]));
			this->SetCurrentIndex(-1); //  (int)obj[5];
			this->SetChanged((bool)(int)obj[6]);
			this->SetInterpolationType(interpolation_t::_from_integral((int)obj[7])); // (interpolation_t)(int)obj[7]
			this->SetBoundaryType(boundary_t::_from_integral((int)obj[8]));
			this->SetCloseTime((u64)(float)obj[9]);
			DeserializeKnots(obj[10]); obj[10].Clear();

			this->SetCharacteristic((value_t)(int)obj[15]);
			this->SetMeasurement((measurement_t)(int)obj[16]);
		}
		else if (obj.getNumVars() >= 12) {

			this->SetSpecifier((int)obj[0]);
			this->SetName(obj[1]);
			{
				vec3 t; t.FromString(obj[2]); this->SetMAD(t);
			}
			this->SetInterpolationType(interpolation_t::_from_integral((int)obj[3])); // (interpolation_t)(int)obj[7]
			this->SetBoundaryType(boundary_t::_from_integral((int)obj[4]));
			DeserializeKnots(obj[5]); obj[5].Clear();

			this->SetCharacteristic((value_t)(int)obj[8]);
			this->SetMeasurement((measurement_t)(int)obj[9]);

		}
	};

	/*!
	Sample spline at time
	*/
	virtual type										GetCurrentValue(const u64& time, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent = nullptr, const measurement_t& outboundUnit = measurement_t::_end_) const;

	/*!
	Return approximate derivative of spline at time
	*/
	virtual type										GetCurrentFirstDerivative(const u64& time) const {
		int i, j, k;
		u64 bvals[4], d, clampedTime;
		type v;

		if (this->GetNumValues() == 1) {
			return (this->ValueForIndex(0) - this->ValueForIndex(0)); //-V501
		}

		clampedTime = this->ClampedTime(time);
		i = this->IndexForTime(clampedTime);
		BasisFirstDerivative(i - 1, clampedTime, bvals);
		v = this->ValueForIndex(0) - this->ValueForIndex(0); //-V501
		for (j = 0; j < 4; j++) {
			k = i + j - 2;
			v += (this->ValueForIndex(k) * (float)bvals[j]);
		}
		d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
		v /= d;
		return v;


	};

	/*!
	Return approximate second derivative of spline at time
	*/
	virtual type										GetCurrentSecondDerivative(const u64& time) const {

		int i, j, k;
		u64 bvals[4], d, clampedTime;
		type v;

		if (this->GetNumValues() == 1) {
			return (this->ValueForIndex(0) - this->ValueForIndex(0)); //-V501
		}

		clampedTime = this->ClampedTime(time);
		i = this->IndexForTime(clampedTime);
		BasisSecondDerivative(i - 1, clampedTime, bvals);
		v = this->ValueForIndex(0) - this->ValueForIndex(0); //-V501
		for (j = 0; j < 4; j++) {
			k = i + j - 2;
			v += (this->ValueForIndex(k) * (float)bvals[j]);
		}
		d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
		v /= (float)(d * d);
		return v;
	};

	/*!
	Get list of spline samples at a specified Timestep
	*/
	virtual cweeThreadedList<std::pair<u64, type>>		GetTimeSeries(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent = nullptr, const measurement_t& outboundUnit = measurement_t::_end_) const {
		std::pair<u64, float> v;
		u64 realTimestep = cweeMath::Fmax(timeStep, 1);
		cweeThreadedList<std::pair<u64, type>> out(cweeMath::max(cweeMath::min((timeEnd - timeStart / (realTimestep)), 100000), 1000) + 16);

		v.first = timeStart; v.second = GetCurrentValue(timeStart, Parent, outboundUnit);
		out.Append(v); // ensure pattern always has a starter? 

		for (v.first = timeStart + realTimestep; v.first < timeEnd; v.first += realTimestep) {
			v.second = GetCurrentValue(v.first, Parent, outboundUnit);
			out.Append(v);
		}

		v.first = timeEnd; v.second = GetCurrentValue(timeEnd, Parent, outboundUnit);
		out.Append(v); // ensure pattern always has a closure? 

		return out;
	};

	/*!
	Get list of spline knots between the requested Times
	*/
	virtual cweeThreadedList<std::pair<u64, type>>		GetKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max(), const measurement_t& outboundUnit = measurement_t::_end_) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);

		if (outboundUnit != measurement_t::_end_)
		{
			vec3 mad0 = cweeUnits::GetMadConversion((measurement_t)this->GetMeasurement(), outboundUnit);
			for (int i = 0; i < numKnots; i++) {
				if (this->TimeForIndex(i) >= timeStart) {
					if (this->TimeForIndex(i) < timeEnd) {
						out.Append(std::make_pair(this->TimeForIndex(i), this->ValueForIndex(i) * mad0[0] + mad0[1]));
					}
					else break;
				}
			}
		}
		else {
			for (int i = 0; i < numKnots; i++) {
				if (this->TimeForIndex(i) >= timeStart) {
					if (this->TimeForIndex(i) < timeEnd) {
						out.Append(std::make_pair(this->TimeForIndex(i), this->ValueForIndex(i)));
					}
					else break;
				}
			}
		}
		return out;
	};

	virtual cweeThreadedList<type>						GetValueKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max(), const measurement_t& outboundUnit = measurement_t::_end_) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<type> out(numKnots + 16);

		if (outboundUnit != measurement_t::_end_)
		{
			vec3 mad0 = cweeUnits::GetMadConversion((measurement_t)this->GetMeasurement(), outboundUnit);
			for (int i = 0; i < numKnots; i++) {
				if (this->TimeForIndex(i) >= timeStart) {
					if (this->TimeForIndex(i) < timeEnd) {
						out.Append(this->ValueForIndex(i) * mad0[0] + mad0[1]);
					}
					else break;
				}
			}
		}
		else {
			for (int i = 0; i < numKnots; i++) {
				if (this->TimeForIndex(i) >= timeStart) {
					if (this->TimeForIndex(i) < timeEnd) {
						out.Append(this->ValueForIndex(i));
					}
					else break;
				}
			}
		}
		return out;
	};

	virtual cweeThreadedList<type>						GetValueTimeSeries(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent = nullptr, const measurement_t& outboundUnit = measurement_t::_end_) const {
		u64 realTimestep = cweeMath::Fmax(timeStep, 1);
		cweeThreadedList<type> out(cweeMath::max(cweeMath::min((timeEnd - timeStart / (realTimestep)), 100000), 1000) + 16);

		out.Append(GetCurrentValue(timeStart, Parent, outboundUnit)); // ensure pattern always has a starter? 
		for (u64 t = timeStart + realTimestep; t < timeEnd; t += realTimestep) {
			out.Append(GetCurrentValue(t, Parent, outboundUnit));
		}
		out.Append(GetCurrentValue(timeEnd, Parent, outboundUnit)); // ensure pattern always has a closure? 

		return out;
	};

	/*!
	Request an integration of the time series. Use divisor (ex. 3600) to convert (ex.) kW-sec to kW-hour.
	*/
	virtual type										RombergIntegral(const u64& t0, const u64& t1, const float divisor = 3600.0f, bool snapLeft = false) const {
		type sum;
		sum = 0.0f;

		if (this->GetNumValues() <= 1) { return sum; }

		u64 step = this->GetMinimumTimeStep();
		type v; v = 0.0f;
		u64 minGot = t1, maxGot = t1;
		if (true) {
			this->Lock();
			{
				cweeThreadedList<std::pair<const u64*, const type*>> data = this->UnsafeGetKnotSeries(t0, t1);
				if (data.Num() > 1) {
					minGot = *data[0].first;
					maxGot = *data[data.Num() - 1].first;

					for (int i = 0; i < (data.Num() - 1); i++) {
						std::pair<const u64*, const type*>& left = data[i], right = data[i + 1];
						if ((*left.first >= t0) && (*right.first <= t1)) {
							v = (*right.first - *left.first) * ((*right.second + *left.second) / 2.0f);
							if (::isfinite(v))
								sum += v;
						}
					}
				}
			}
			this->Unlock();
		}

		u64 t; u64 stepDiv2 = step / 2.0;  u64 maxT = t1 + stepDiv2;

		for (t = t0; (t + step) < minGot; t += step) {
			v = step * GetCurrentValue(t + stepDiv2); // assumes linear or snap behavior.
			if (::isfinite(v))
				sum += v;
		}
		if (minGot > t) {
			v = (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
			if (::isfinite(v))
				sum += v;
		}

		for (t = maxGot; (t + step) < t1; t += step) {
			v = step * GetCurrentValue(t + stepDiv2); // assumes linear or snap behavior.
			if (::isfinite(v))
				sum += v;
		}
		if (t1 > t) {
			v = (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			if (::isfinite(v)) sum += v;
		}

		return sum / (divisor != 0 ? divisor : 1);
	};

	/*! <X,X,X> = pattern.ValueQuantiles({ 0.25, 0.5, 0.75 }); */
	virtual cweeThreadedList<type>						ValueQuantiles(const cweeThreadedList<float>& probs, const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max(), int numSamples = -1) {
		cweeThreadedList<type> quantiles;
		float poi;
		size_t left, right;

		cweeThreadedList<type> data;
		if (numSamples >= 1) {
			u64 t0 = std::max(timeStart, this->GetMinTime());
			u64 t1 = std::min(timeEnd, this->GetMaxTime());
			u64 step = (t1 - t0) / numSamples;
			if (!::isfinite(step) || step == 0) step = (1 * 24 * 60 * 60);

			data = GetValueTimeSeries(t0, t1, step); // copy data
		}
		else
		{
			data = GetValueKnotSeries(timeStart, timeEnd); // copy data
		}

		if (data.NumRef() == 0)
		{
			return data;
		}

		if (1 == data.NumRef())
		{
			return data;
		}

		data.Sort(); // sort data // [](type const& a, type const& b)->bool {}

		for (size_t i = 0; i < probs.NumRef(); ++i) {
			poi = (1.0f - probs[i]) * -0.5f + probs[i] * (data.NumRef() - 0.5f);
			left = std::max(int64_t(std::floor(poi)), int64_t(0));
			right = std::min(int64_t(std::ceil(poi)), int64_t(data.NumRef() - 1.0f));
			quantiles.Append((1.0f - (poi - left)) * data[left] + (poi - left) * data[right]);
		}

		return quantiles;
	}

	/*!
	Copy another Pattern into this one.
	*/
	virtual void										Copy(const cweePattern_CatmullRomSpline<type>& copy, const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max()) {
		this->Clear(); // cleared

		this->SetName(copy.GetName());
		this->SetMAD(copy.GetMAD());
		this->SetSpecifier(copy.GetSpecifier());
		this->SetBoundaryType(copy.GetBoundaryType());
		this->SetCloseTime(copy.GetCloseTime());

		this->SetCurrentIndex(-1);
		this->SetChanged(true);
		this->SetInterpolationType(copy.GetInterpolationType());

		this->SetCharacteristic(copy.GetCharacteristic());
		this->SetMeasurement(copy.GetMeasurement());

		auto list = copy.GetKnotSeries(timeStart, timeEnd);
		this->SetGranularity(list.NumRef() + 16);

		for (auto& x : list) this->AddValue(x.first, x.second, false);
	}

	/*!
	Review the Pattern and remove Knots that are unecessary with Spline Sampling.
	*/
	virtual void						RemoveUnnecessaryKnots(const u64& start = -std::numeric_limits < u64>::max(), const u64& end = std::numeric_limits<u64>::max()) const {
		int num, currentPos; cweeThreadedList<int> indexesToDelete;
		if (start < end) {

			constexpr float epsilon = 0.00001f;
			type* val = nullptr;
			type* val2 = nullptr;
			type* val3 = nullptr;
			type* val4 = nullptr;
			type* val5 = nullptr;

			this->Lock();
			{
				auto& _values = this->UnsafeGetValues();
				auto& _times = this->UnsafeGetTimes();

				num = _values.NumRef();
				if (num > 5) {
					indexesToDelete.SetGranularity(num + 16);

					// remove duplicate datapoints that would result in a straight line with the catmull-rom spline. 
					// reduce 5 identical data points to 4 by removing the middle and re-doing the check from the second point. 
					currentPos = this->UnsafeIndexForTime(start) + 1; // should be "1" under most conditions					
					if (currentPos <= 0) currentPos++;
					if ((currentPos + 3) < num) {
						val = &_values[currentPos - 1];
						val2 = &_values[currentPos];
						val3 = &_values[currentPos + 1];
						val4 = &_values[currentPos + 2];

						do {
							if (num <= (currentPos + 3)) break;
							if (_times[currentPos + 3] > end) break;
							val5 = &_values[currentPos + 3];

							if (cweeMath::Fabs((float)(*val - *val2)) > epsilon) { // !=
								++currentPos;
								val = val2;
								val2 = val3;
								val3 = val4;
								val4 = val5;
								continue;
							}
							else if ((cweeMath::Fabs((float)(*val5 - *val2)) < epsilon) && (cweeMath::Fabs((float)(*val4 - *val2)) < epsilon) && (cweeMath::Fabs((float)(*val3 - *val2)) < epsilon)) { // ==
								indexesToDelete.Append(currentPos + 1);
								++currentPos;
								val3 = val4;
								val4 = val5;
								continue; // don't move the currentPos forward. Repeat the analysis from this spot. 																								
							}
							else {
								++currentPos;
								val = val2;
								val2 = val3;
								val3 = val4;
								val4 = val5;
							}
						} while (1);

						_times.RemoveIndexes(indexesToDelete);
						_values.RemoveIndexes(indexesToDelete);
					}
				}
			}
			this->Unlock();
		}
	}

	/*!
	Reduce memory usage where possible, with a desired sample per STEP value target.
	*/
	virtual void										ReduceMemory(float percentToRemove, const u64& start = -std::numeric_limits < u64>::max(), const u64& end = std::numeric_limits<u64>::max()) const;

	/*!
	Remove knots older than the specified time.
	*/
	virtual void										RemoveOlderThan(const u64& time) {

		cweeThreadedList<int> indexesToDelete(this->GetNumValues() + 16);
		this->Lock();
		int i = this->UnsafeIndexForTime(time);
		//#pragma loop(hint_parallel(8))
		for (int j = 0; j < i; j++) indexesToDelete.Append(j);
		this->UnsafeGetTimes().RemoveIndexes(indexesToDelete);
		this->UnsafeGetValues().RemoveIndexes(indexesToDelete);
		this->Unlock();
	};

	/*!
	Accept new Values from the Timeseries list of knots
	*/
	virtual void										AcceptNewData(const cweeThreadedList<std::pair<u64, type>>& list, bool useMAD = false) {
		this->SetGranularity(list.NumRef() + 16);
		for (auto& x : list) {
			this->AddUniqueValue(x.first, x.second, useMAD);
		}
	}

	virtual void										OverridePeriod(const cweeThreadedList<std::pair<u64, type>>& list, bool useMAD = false) {
#if 1 

		if (list.NumRef() <= 0) return;
		u64 start = list[0].first;
		u64 end = list[list.NumRef() - 1].first;

		cweeThreadedList<int> indexesToDelete(this->GetNumValues() + 16);

		this->Lock(); {
			int i = this->UnsafeIndexForTime(start);
			int j = this->UnsafeIndexForTime(end);
			//#pragma loop(hint_parallel(8))
			for (; i <= j; i++) {
				indexesToDelete.Append(i);
			}
			this->UnsafeGetTimes().RemoveIndexes(indexesToDelete);
			this->UnsafeGetValues().RemoveIndexes(indexesToDelete);
		} this->Unlock();

		AcceptNewData(list, useMAD);

#else		
		if (list.NumRef() <= 0) return;

		u64 start = list[0].first;
		u64 end = list[list.NumRef() - 1].first;

		auto part1 = GetKnotSeries(this->GetMinTime() - 1, start);
		auto part3 = GetKnotSeries(end, this->GetMaxTime() + 1);

		this->Clear();

		AcceptNewData(part1, false);
		AcceptNewData(list, useMAD);
		AcceptNewData(part3, false);
#endif
	}

	/*!
	Accept changes made to a similar pattern.
	*/
	virtual void										AcceptChanges(const cweePattern_CatmullRomSpline<type>& copy) {
		this->SetName(copy.GetName());
		this->SetMAD(copy.GetMAD());
		this->SetSpecifier(copy.GetSpecifier());
		this->SetBoundaryType(copy.GetBoundaryType());
		this->SetCloseTime(copy.GetCloseTime());

		this->SetCurrentIndex(-1);
		this->SetChanged(true);
		this->SetInterpolationType(copy.GetInterpolationType());

		this->SetCharacteristic(copy.GetCharacteristic());
		this->SetMeasurement(copy.GetMeasurement());

		AcceptNewData(copy.GetKnotSeries());
	}

	/*!
	Get a pattern (copy of this pattern) whose Y-Values are conversions of its X-Values
	*/
	virtual cweePattern_CatmullRomSpline<type>			GetTimePattern(bool returnHour = true) const;

	virtual type										GetCurrentMovingAverage(const u64& time) const {
		int index = this->IndexForTime(time);
		if (index < 5) return this->GetCurrentValue(time);

		return
			(this->GetCurrentValue(this->TimeForIndex(index)) +
				this->GetCurrentValue(this->TimeForIndex(index - 1)) +
				this->GetCurrentValue(this->TimeForIndex(index - 2)) +
				this->GetCurrentValue(this->TimeForIndex(index - 3)) +
				this->GetCurrentValue(this->TimeForIndex(index - 4)) +
				this->GetCurrentValue(this->TimeForIndex(index - 5))) / 6.0f;
	}

	/*!
	Get a pattern (copy of this pattern) whose Y-Values are conversions of its Y-Values
	0: Velocity
	1: Acceleration
	2: Moving Average
	3: Normalized ([0 - 1] range)
	*/
	virtual cweePattern_CatmullRomSpline<type>			GetTransformedPattern(int choice) const {
		cweePattern_CatmullRomSpline<type> out;
		out.Copy(*this);

		switch (choice) {
		case 0: {
			{
				out.Clear();
				for (int i = 0; i < this->GetNumValues(); i += 2) {
					if (i + 1 < this->GetNumValues()) {
						u64 time = (this->TimeForIndex(i) + this->TimeForIndex(i + 1)) / 2.0f;
						type riseOverRun = (type)(this->ValueForIndex(i + 1) - this->ValueForIndex(i));
						riseOverRun /= (float)(this->TimeForIndex(i + 1) - this->TimeForIndex(i));
						out.AddUniqueValue(time, riseOverRun);
					}
				}
			}

			break;
		}
		case 1: {
			out.Lock();
			for (int i = 0; i < out.UnsafeGetValues().NumRef(); i++) {
				out.UnsafeGetValues()[i] = this->GetCurrentSecondDerivative(out.UnsafeGetTimes()[i]);
			}
			out.Unlock();

			break;
		}
		case 2: {
			out.Lock();
			for (int i = 0; i < out.UnsafeGetValues().NumRef(); i++) {
				out.UnsafeGetValues()[i] = this->GetCurrentMovingAverage(out.UnsafeGetTimes()[i]);
			}
			out.Unlock();

			break;
		}
		case 3: {
			auto min = this->GetMinValue();
			auto max = this->GetMaxValue();
			if (min == max) max = min + 1.0f;

			out.Lock();
			for (int i = 0; i < out.UnsafeGetValues().NumRef(); i++) {
				out.UnsafeGetValues()[i] = (this->GetCurrentValue(out.UnsafeGetTimes()[i]) - min) / (max - min);
			}
			out.Unlock();

			break;
		}
		default: {
			break;
		}
		}
		return out;
	}

	static int											GetSeason(const u64& time);

	static int											GetMonthOfYear(const u64& Time);

	static int											GetDayOfMonth(const u64& time);

	static int											GetDayOfWeek(const u64& time);

	static float										GetHourOfDay(const u64& time);

	virtual type										GetNormalizedValue(const u64& time) const {
		auto min = this->GetMinValue();
		auto max = this->GetMaxValue();

		if (min == max) max = min + 1.0f;

		return (GetCurrentValue(time) - min) / (max - min);
	}

	/*!
	Clear all knots and accept the new, incoming data Values from the list of knots.
	*/
	virtual cweePattern_CatmullRomSpline<type>			DuplicateWithNewData(const cweeThreadedList<std::pair<u64, type>>& in, bool useMAD = false) {
		cweePattern_CatmullRomSpline<type> out;
		out.Copy(*this);
		out.Lock();
		out.UnsafeGetTimes().Clear();
		out.UnsafeGetValues().Clear();
		out.Unlock();

		for (auto& x : in) {
			out.AddUniqueValue(x.first, x.second, useMAD);
		}
		return out;
	}

	/*!
	Produces a new pattern that blends this pattern with the "features" of the incoming pattern. It will result in an "average" value from this pattern but with the higher-resolution features.
	Ideally the incoming patterns were set up with matching durations and Timesteps (i.e. already did the machine learning and time series sampling)
	*/
	virtual	cweePattern_CatmullRomSpline<type>			LaplacianBlend(const cweePattern_CatmullRomSpline<type>& highResFeatures) const {
		cweePattern_CatmullRomSpline<type> out;

		cweePattern_CatmullRomSpline<type> base = *this; base.RemoveUnnecessaryKnots(); base.SetBoundaryType(boundary_t::BT_LOOP);
		cweePattern_CatmullRomSpline<type> features = highResFeatures; features.RemoveUnnecessaryKnots(); features.SetBoundaryType(boundary_t::BT_LOOP);

		u64 min = ::Min(base.GetMinTime(), features.GetMinTime());
		u64 max = ::Max(base.GetMaxTime(), features.GetMaxTime());
		u64 timeStep = ::Min(base.GetMinimumTimeStep(), features.GetMinimumTimeStep());

		cweePattern_CatmullRomSpline<type> base2 = base.GetTimeSeries(min, max, timeStep);
		cweePattern_CatmullRomSpline<type> features2 = features.GetTimeSeries(min, max, timeStep);

		// determine the "features" of the incoming pattern
		cweeThreadedList< cweePattern_CatmullRomSpline<type> > a_blurs; a_blurs.Append(base2);
		cweeThreadedList< cweePattern_CatmullRomSpline<type> > a_laplacians;

		cweeThreadedList< cweePattern_CatmullRomSpline<type> > b_blurs; b_blurs.Append(features2 - features2.GetAvgValue());
		cweeThreadedList< cweePattern_CatmullRomSpline<type> > b_laplacians;

		int j = 1;
		for (float k = cweeMath::max(a_blurs[0].GetNumValues(), b_blurs[0].GetNumValues()); k >= 1; k /= 2.0f) j++;

		a_blurs.SetGranularity(j);
		a_laplacians.SetGranularity(j);
		b_blurs.SetGranularity(j);
		b_laplacians.SetGranularity(j);

		int i = 0;
		while (j >= 0) { // build pyramid forwards
			a_blurs.Append(a_blurs[i].GuassianBlur()); // size i + 2 
			a_laplacians.Append(a_blurs[i] - a_blurs[i + 1]); // size i + 1

			b_blurs.Append(b_blurs[i].GuassianBlur()); // size i + 2
			b_laplacians.Append(b_blurs[i] - b_blurs[i + 1]); // size i + 1

			i++; j--;
		}

		// reconstruct backwards
		out = a_blurs[i]; // initialize the pyramid
		for (; i >= 0; i--) {
			out += (a_laplacians[i] + b_laplacians[i]);
		}

		return out;
	};

	/*! Produces a new pattern that is half the resolution of the previous pattern */
	virtual	cweePattern_CatmullRomSpline<type>			GuassianBlur() {
		cweePattern_CatmullRomSpline<type> out;
		int i = this->GetNumValues();
		out.SetGranularity(i + 16);

		if (i == 1) {
			out = *this;
		}
		else {
			out.AddUniqueValue(this->TimeForIndex(0), this->ValueForIndex(0));
			out.AddUniqueValue(this->TimeForIndex(this->GetNumValues() - 1), this->ValueForIndex(this->GetNumValues() - 1));

			//#pragma loop(hint_parallel(8))
			for (int j = 0; (j + 1) < i; j += 2) {
				out.AddUniqueValue(
					(this->TimeForIndex(j) + this->TimeForIndex(j + 1)) / 2.0f,
					(this->ValueForIndex(j) + this->ValueForIndex(j + 1)) / 2.0f
				);
			}
		}
		return out;

	};

	/*!
	Seeks to manipulate "toManipulate" until the integration(s) match "toMatch"'s integration(s)
	Works best when "toManipulate" has significantly more knots than "toMatch"
	*/
	static cweePattern_CatmullRomSpline<type>			AdjustPatternTillIntegralsMatch(const cweePattern_CatmullRomSpline<type>& toMatch_, cweePattern_CatmullRomSpline<type>& toManipulate, u64 start = 0, u64 end = 0, u64 toMatchTargetResolution = 0) {
		// we can (in theory) manipulate "toManipulate" in many different ways until its integration(s) match the integration(s) of "toMatch"
		cweePattern_CatmullRomSpline<type> toMatch;
		if (end == 0 && start == 0 && toMatchTargetResolution == 0) {
			toMatch = toMatch_;
		}
		else if (toMatchTargetResolution == 0 && end == 0) {
			toMatch = toMatch_(start - 0.01f, toMatch_.GetMaxTime() + 0.01f);
		}
		else if (toMatchTargetResolution == 0) {
			toMatch = toMatch_(start - 0.01f, end + 0.01f);
		}
		else {
			toMatch = toMatch_(start, end, toMatchTargetResolution);
		}

		cweePattern_CatmullRomSpline<type> out;

		{
			auto bt_prev = toManipulate.GetBoundaryType();
			toManipulate.SetBoundaryType(boundary_t::BT_LOOP);
			out = toManipulate(toMatch.GetMinTime(), toMatch.GetMaxTime(), toManipulate.GetMinimumTimeStep());
			toManipulate.SetBoundaryType(bt_prev);

			switch (toMatch.GetNumValues()) {
			case 0: {
				// return out;
				break;
			}
			case 1: {
				float v1 = toMatch.ValueForIndex(0);
				float v2 = toManipulate.GetAvgValue();
				out *= v1 / (v2 == 0 ? (v2 + cweeMath::EPSILON) : v2);
				break;
			}
			default: {
				float v1, v2, mod; u64 t1, t2; int j;
				{ // adjust the first 2 points manually					
					t1 = toMatch.TimeForIndex(0); t2 = toMatch.TimeForIndex(1);
					v1 = toMatch.RombergIntegral(t1, t2);
					v2 = toManipulate.RombergIntegral(t1, t2);
					mod = v1 / (v2 == 0 ? (v2 + cweeMath::EPSILON) : v2);
					out.Lock();
					for (j = 0; j < out.UnsafeGetValues().NumRef(); j++) {
						if (out.UnsafeGetTimes()[j] >= t1) {
							if (out.UnsafeGetTimes()[j] <= t2)
								out.UnsafeGetValues()[j] *= mod;
							else
								break;
						}
					}
					out.Unlock();
				}
				{ // adjust the remaining points
					for (int i = 1; (i + 1) < toMatch.GetNumValues(); i++) {
						t1 = toMatch.TimeForIndex(i); t2 = toMatch.TimeForIndex(i + 1);
						v1 = toMatch.RombergIntegral(t1, t2);
						v2 = toManipulate.RombergIntegral(t1, t2);
						mod = v1 / (v2 == 0 ? (v2 + cweeMath::EPSILON) : v2);
						out.Lock();
						for (; j < out.UnsafeGetValues().NumRef(); j++) {
							if (out.UnsafeGetTimes()[j] > t1) {
								if (out.UnsafeGetTimes()[j] <= t2)
									out.UnsafeGetValues()[j] *= mod;
								else
									break;
							}
						}
						out.Unlock();
					}
				}
				break;
			}
			}

			// perform an over-all adjustment to ensure the total sum matches this sum
			float v1, v2, mod; u64 t1, t2;
			{
				t1 = toMatch.GetMinTime(); t2 = toMatch.GetMaxTime();
				v1 = toMatch.RombergIntegral(t1, t2);
				v2 = out.RombergIntegral(t1, t2);
				mod = v1 / (v2 == 0 ? (v2 + cweeMath::EPSILON) : v2);
			}
			out *= mod;
		}

		return out;
	};

	virtual type										GetTransformedCurrentValue(const u64& time, patternModifier mod, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent = nullptr) const;

protected:
	void												Basis(int index, const u64& t, u64* bvals) const {
		//const float x = cweeMath::min(cweeMath::max((float)((t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index))), 0), 1);
		//const float s = 0; // where s=0 means linear, s=1 means fully smooth, s=0.5 = traditional catmull-rom. 
		//const float sx = s * x;
		//const float sx2 = sx * x;
		//const float sx3 = sx2 * x;
		//const float x2 = x * x;
		//const float x3 = x2 * x;

		//bvals[0] = 2.0f*sx2-sx-sx3;
		//bvals[1] = 1.0f-3.0f*x2+sx2+2.0f*x3-sx3;
		//bvals[2] = sx+3.0f*x2-2.0f*sx2-2.0f*x3+sx3;
		//bvals[3] = sx3 - sx2;

		u64 s;
		switch (this->GetInterpolationType()) {
		case interpolation_t::SPLINE: {
			s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
			if (!::isfinite(s)) s = 0;

			bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
			bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
			bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
			bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s

			break;
		}
		case interpolation_t::LEFT: {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;

			break;
		}
		case interpolation_t::RIGHT: {
			bvals[0] = 0;
			bvals[1] = 0;
			bvals[2] = 1;
			bvals[3] = 0;

			break;
		}
		case interpolation_t::LINEAR: {
			s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
			if (!::isfinite(s)) s = 0;

			bvals[0] = 0; // 0
			bvals[1] = (u64)(1.0f - s); // 1
			bvals[2] = s; // 0
			bvals[3] = 0; // 0

			break;
		}
		default: {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;

			break;
		}
		}
	};
	void												BasisFirstDerivative(int index, const u64& t, u64* bvals) const {

		u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
		bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
		bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
		bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
		bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
	};
	void												BasisSecondDerivative(int index, const u64& t, u64* bvals) const {

		u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
		bvals[0] = -3.0f * s + 2.0f;
		bvals[1] = 9.0f * s - 5.0f;
		bvals[2] = -9.0f * s + 4.0f;
		bvals[3] = 3.0f * s - 1.0f;
	};

private:
	static cweeTime getLocalTime(const u64& time);

};

template< class type >
INLINE cweeTime cweePattern_CatmullRomSpline<type>::getLocalTime(const u64& time) { return cweeTime(time); };

template< class type >
INLINE cweePattern_CatmullRomSpline<type>			cweePattern_CatmullRomSpline<type>::GetTimePattern(bool returnHour) const {
	cweePattern_CatmullRomSpline<type> out;
	out.Copy(*this);
	if (returnHour == true) {
		// return 0 - 23 value representing the hour (0 is midnight)
		out.Lock();
		for (int i = 0; i < out.UnsafeGetValues().Num(); i++) {
			time_t time = out.UnsafeGetTimes()[i];
			cweeTime tmp = getLocalTime(time);
			out.UnsafeGetValues()[i] = (int)tmp.tm_hour();
		}
		out.Unlock();
	}
	else {
		// return 0 - 6 value representing the day of the week (0 is Sunday)
		out.Lock();
		for (int i = 0; i < out.UnsafeGetValues().Num(); i++) {
			time_t time = out.UnsafeGetTimes()[i];
			cweeTime tmp = getLocalTime(time);
			out.UnsafeGetValues()[i] = (int)tmp.tm_wday();
		}
		out.Unlock();
	}
	return out;
};

template< class type >
INLINE int								cweePattern_CatmullRomSpline<type>::GetMonthOfYear(const u64& Time) {
	time_t localTime = (u64)Time;
	cweeTime tmp = getLocalTime(localTime);
	return tmp.tm_mon();
};

template< class type >
INLINE int								cweePattern_CatmullRomSpline<type>::GetDayOfMonth(const u64& time) {
	time_t temp = time;
	cweeTime tmp = getLocalTime(temp);
	return tmp.tm_mday();
};

template< class type >
INLINE int								cweePattern_CatmullRomSpline<type>::GetDayOfWeek(const u64& time) {
	time_t temp = time;
	cweeTime tmp = getLocalTime(temp);
	return tmp.tm_wday();
};

template< class type >
INLINE float							cweePattern_CatmullRomSpline<type>::GetHourOfDay(const u64& time) {
	time_t temp = time;
	cweeTime tmp = getLocalTime(temp);
	return (((float)tmp.tm_hour()) + (((float)tmp.tm_min()) / 60.0f) + (((float)tmp.tm_sec()) / 3600.0f));
};

template< class type >
INLINE int								cweePattern_CatmullRomSpline<type>::GetSeason(const u64& time) {
	int month = GetMonthOfYear(time);
	switch (month) {
	case 0: { // jan
		return 0;
		break;
	}
	case 1: { // feb
		return 0;
		break;
	}
	case 2: { // mar
		return 0;
		break;
	}
	case 3: { // apr
		return 1;
		break;
	}
	case 4: { // may
		return 1;
		break;
	}
	case 5: { // june
		return 1;
		break;
	}
	case 6: { // july
		return 2;
		break;
	}
	case 7: { // aug
		return 2;
		break;
	}
	case 8: { // sep
		return 2;
		break;
	}
	case 9: { // oct
		return 3;
		break;
	}
	case 10: { // nov
		return 3;
		break;
	}
	case 11: { // dec
		return 3;
		break;
	}
	}
};

template< class type >
INLINE type								cweePattern_CatmullRomSpline<type>::GetTransformedCurrentValue(const u64& time, patternModifier mod, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent) const {
	switch (mod) {
	case patternModifier::None: {
		return this->GetCurrentValue(time, Parent);
		break;
	}
	case patternModifier::DayOfWeek: {
		return (type)this->GetDayOfWeek(time);
		break;
	}
	case patternModifier::HourOfDay: {
		return (type)this->GetHourOfDay(time);
		break;
	}
	case patternModifier::Velocity: {
		int i = this->IndexForTime(time);
		if (i + 1 < this->GetNumValues()) {
			u64 tP = this->TimeForIndex(i + 1) - this->TimeForIndex(i);
			if (tP == 0.0f) return 0.0f;
			return (this->ValueForIndex(i + 1) - this->ValueForIndex(i)) / (tP);
		}
		else {
			type toReturn;
			toReturn = 0;
			return toReturn;
		}
		break;
	}
	case patternModifier::Acceleration: {
		int i = this->IndexForTime(time);
		if (i + 1 < this->GetNumValues()) {
			u64 tP = this->TimeForIndex(i + 1) - this->TimeForIndex(i);
			if (tP == 0.0f) return 0.0f;
			return
				(
					GetTransformedCurrentValue(this->TimeForIndex(i + 1), patternModifier::Velocity, Parent)
					-
					GetTransformedCurrentValue(this->TimeForIndex(i), patternModifier::Velocity, Parent)
					) / (
						tP
						);
		}
		else {
			type toReturn;
			toReturn = 0;
			return toReturn;
		}
		break;
	}
	case patternModifier::MovingAverage: {
		return this->GetCurrentMovingAverage(time);
		break;
	}
	case patternModifier::Normalize: {
		return this->GetNormalizedValue(time);
		break;
	}
	}
};

template< class type >
INLINE void								cweePattern<type>::RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo) {
	int lowerLimit, upperLimit, index; cweeThreadedList<int> indexesToDelete;
	Lock();
	{
		lowerLimit = UnsafeIndexForTime(greaterThan) - 1;
		upperLimit = UnsafeIndexForTime(lessThenEqualTo) + 1;
		indexesToDelete.SetGranularity(cweeMath::max(16, (upperLimit - lowerLimit) + 16));
		for (index = upperLimit; index >= lowerLimit; index--) {
			if (index < times.Num() && index >= 0) {
				auto& pos = times[index];
				if (pos > greaterThan && pos <= lessThenEqualTo) {
					indexesToDelete.Append(index);
				}
			}
		}

		times.RemoveIndexes(indexesToDelete); // safe if outside bounds
		values.RemoveIndexes(indexesToDelete); // safe if outside bounds
	}
	Unlock();
};

template< class type >
INLINE void								cweePattern<type>::Lock() const {
	this->lock.Lock();
};

template< class type >
INLINE void								cweePattern<type>::Unlock() const {
	this->lock.Unlock();
};

template< class type >
INLINE type								cweePattern_CatmullRomSpline<type>::GetCurrentValue(const u64& time, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent, const measurement_t& outboundUnit) const {

	int i = 0, j = 0, k = 0;
	u64 bvals[4], clampedTime;
	type v;

	j = this->GetNumValues();

	if (j < 1) {
		v = 0;
		return v;
	}
	if (j == 1) {
		return this->ValueForIndex(0);
	}

	clampedTime = this->ClampedTime(time);
	clampedTime = this->LoopedTime(clampedTime);


	i = this->IndexForTime(clampedTime);
	Basis(i - 1, clampedTime, bvals);
	v = 0;
	for (j = 0; j < 4; j++) {
		k = i + j - 2;
		v += (this->ValueForIndex(k) * (float)bvals[j]);
	}
	
	if (outboundUnit != measurement_t::_end_)
	{
		vec3 mad0 = cweeUnits::GetMadConversion(this->GetMeasurement(), outboundUnit);
		v *= mad0[0];
		v += mad0[1];
	}
	return v;

};

template< class type >
INLINE void								cweePattern_CatmullRomSpline<type>::ReduceMemory(float percentToRemove, const u64& start, const u64& end) const {
	if (!this->GetChanged()) return;

	if (start >= end) return;

	RemoveUnnecessaryKnots(start, end);

	if (percentToRemove <= 0) return;

	type maxValue;
	type minValue;
	minValue = this->GetMinValue();
	maxValue = this->GetMaxValue();
	type epsilon = cweeMath::Fabs(maxValue - minValue) * (((float)percentToRemove) / 100.0f);
	int num, currentPos;
	cweeThreadedList<int> indexesToDelete;
	type* val = nullptr;
	type* val2 = nullptr;
	type* val3 = nullptr;

	this->Lock();
	{
		auto& _values = this->UnsafeGetValues();
		auto& _times = this->UnsafeGetTimes();

		num = _values.Num();

		// reduction based on (lack of) change in y-axis.
		if (num > 3) {
			// where step = 1 -> 1% removal. step = 2 -> 2% removal, etc. 		
			indexesToDelete.SetGranularity(num + 16);
			{
				currentPos = this->UnsafeIndexForTime(start) + 1; // should be "1" under most conditions
				if (currentPos <= 0) currentPos++;
				if ((currentPos + 3) < num) {
					val = &_values[currentPos - 1];
					val2 = &_values[currentPos];
					do {
						if (num <= (currentPos + 2)) break;
						if (_times[currentPos + 2] > end) break;
						val3 = &_values[currentPos + 1];

						if (
							(cweeMath::Fabs((float)(*val - *val2)) <= epsilon) && (cweeMath::Fabs((float)(*val3 - *val2)) <= epsilon) && (cweeMath::Fabs((float)(*val - *val3)) <= epsilon)
							) { // ==				
							if (
								((*val >= *val2) && (*val2 >= *val3)) || ((*val <= *val2) && (*val2 <= *val3))
								) { // i.e. the 'middle' value is neither the max or min value in this relationship
								indexesToDelete.Append(currentPos);
								++currentPos;
								val2 = val3;
								continue; // don't move the currentPos forward. Repeat the analysis from this spot. 		
							}
						}
						++currentPos;
						val = val2;
						val2 = val3;
					} while (1);
				}
			}
			_times.RemoveIndexes(indexesToDelete); // safe if outside bounds
			_values.RemoveIndexes(indexesToDelete); // safe if outside bounds

		}
	}
	this->Unlock();
	this->SetChanged(false);
}

using Pattern = cweePattern_CatmullRomSpline<float>;