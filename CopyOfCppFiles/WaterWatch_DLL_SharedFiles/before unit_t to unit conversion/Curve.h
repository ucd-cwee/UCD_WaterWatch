#pragma once
#include "Precompiled.h"

/*
===============================================================================
	Curve base template.
===============================================================================
*/
constexpr u64 serializationTimeConverter = 100.0f;

enum class boundary_t { BT_FREE, BT_CLAMPED, BT_CLOSED, BT_LOOP, BT_END };

const static std::map<boundary_t, const char*> StringMap_boundary_t = {
	{boundary_t::BT_FREE, "Free"},
	{boundary_t::BT_CLAMPED, "Clamped"},
	{boundary_t::BT_CLOSED, "Closed"},
	{boundary_t::BT_LOOP, "Loop"},
	{boundary_t::BT_END, "End"}
};
template<> const static std::map<boundary_t, const char*>& StaticStringMap< boundary_t >() {
	return StringMap_boundary_t;
};

template< class type = float >
class cweeCurve {
public:
	cweeCurve() {
		currentIndex = -1;
		knots.SetGranularity(64);
		setName("No Name");
		changed = false;
	};

	virtual				~cweeCurve() {};
	virtual int			AddUniqueValue(const u64& time, type&& value) { // slower with guarrantee of uniqueness 
		int i = IndexForTime(time);

		if (((i != 0) && (i < this->Num()) && (TimeAt(i) == time))) {
			ValueAt(i) = value;
			// finished;
			changed = true;
			return i;
		}
		u64 minTime; {
			if (this->Num() == 0) minTime = -std::numeric_limits < u64>::max();
			else minTime = TimeAt(0);
		}
		if (this->Num() == 0 || minTime != time) {
			InsertValue_Impl(i, time, std::forward<type>(value));
			if ((this->Num() + 1) >= (GetGranularity() * GRANULARITY_SCALER)) SetGranularity(GetGranularity() * GRANULARITY_SCALER);
		}
		else {
			ValueAt(0) = value;
			i = 0;
		}
		changed = true;
		return i;
	};
	virtual int			AddValue(const u64& time, type&& value) {
		int i = IndexForTime(time);
		InsertValue_Impl(i, time, std::forward<type>(value));
		if ((this->Num() + 1) >= (GetGranularity() * GRANULARITY_SCALER)) SetGranularity(GetGranularity() * GRANULARITY_SCALER);
		changed = true;
		return i;
	};
	virtual int			AddUniqueValue(const u64& time, const type& value) { // slower with guarrantee of uniqueness 
		int i = IndexForTime(time);

		if (((i != 0) && (i < this->Num()) && (TimeAt(i) == time))) {
			ValueAt(i) = value;
			// finished;
			changed = true;
			return i;
		}
		u64 minTime; {
			if (this->Num() == 0) minTime = -std::numeric_limits < u64>::max();
			else minTime = TimeAt(0);
		}
		if (this->Num() == 0 || minTime != time) {
			InsertValue_Impl(i, time, value);
			if ((this->Num() + 1) >= (GetGranularity() * GRANULARITY_SCALER)) SetGranularity(GetGranularity() * GRANULARITY_SCALER);
		}
		else {
			ValueAt(0) = value;
			i = 0;
		}
		changed = true;
		return i;
	};
	virtual int			AddValue(const u64& time, const type& value) {
		int i = IndexForTime(time);
		InsertValue_Impl(i, time, value);
		if ((this->Num() + 1) >= (GetGranularity() * GRANULARITY_SCALER)) SetGranularity(GetGranularity() * GRANULARITY_SCALER);
		changed = true;
		return i;
	};
	virtual void		RemoveIndex(const int index) { knots.RemoveIndex(index); changed = true; }
	virtual void		Clear() { knots.Clear(); currentIndex = -1; changed = true; }

	virtual	void		SetGranularity(int size) {
		knots.SetGranularity(size);
	};
	virtual	int			GetGranularity() { return knots.GetGranularity(); };
	virtual type		GetCurrentValue(const u64 time) const {
		int i;
		type v = type();
		int n = this->Num();
		if (n < 1) {
			return v;
		}
		if (n == 1) {
			return ValueAt(0);
		}

		i = IndexForTime(time);
		if (i >= n) {
			return ValueAt(n - 1);
		}
		else {
			return ValueAt(i);
		}
	};
	virtual type		GetCurrentFirstDerivative(const u64 time) const {
		return (ValueAt(0)); //-V501
	};
	virtual type		GetCurrentSecondDerivative(const u64 time) const {
		return (ValueAt(0)); //-V501
	};

	virtual bool		IsDone(const u64 time) const {
		return (time >= TimeAt(this->Num() - 1));
	};
	u64					GetMaxTime(void) const {
		if (GetNumValues() == 0)
			return 0;
		else
			return TimeAt(GetNumValues() - 1);
	}
	u64					GetMinTime(void) const {
		if (GetNumValues() == 0)
			return 0;
		else
			return TimeAt(0);
	}

	int					Num() const { return knots.Num(); };
	int					GetNumValues() const { return knots.Num(); }
	void				SetValue(const int index, const type& value) { ValueAt(index) = value; changed = true; }
	type				GetValue(int index) const {
		type out;
		if (index < this->Num() && index >= 0)
			out = ValueAt(index);
		else if (index >= this->Num() && this->Num() > 0)
			out = ValueAt(this->Num() - 1);
		else if (index < 0 && this->Num() > 0)
			out = ValueAt(0);
		else
			out = 0;
		return out;
	}
	type*				GetValueAddress(int index) {
		if (index < this->Num() && index >= 0)
			return &ValueAt(index);
		else if (index >= this->Num() && this->Num() > 0)
			return &ValueAt(this->Num() - 1);
		else if (index < 0 && this->Num() > 0)
			return &ValueAt(0);
		else
			return nullptr;
	}
	u64					GetTime(int index) const { return TimeAt(index); }

	u64					GetLengthForTime(const u64 time) const {
		u64 length = 0.0f;
		int index = IndexForTime(time);
		for (int i = 0; i < index; i++) {
			length += RombergIntegral(TimeAt(i), TimeAt(i + 1), 5);
		}
		length += RombergIntegral(TimeAt(index), time, 5);
		return length;
	};
	u64					GetTimeForLength(const u64 length, const u64 epsilon = 0.1f) const {
		int i, index;
		u64* accumLength, totalLength, len0, len1, t, diff;

		if (length <= 0.0f) {
			return TimeAt(0);
		}

		accumLength = (u64*)_alloca16(this->Num() * sizeof(u64));
		totalLength = 0.0f;
		for (index = 0; index < this->Num() - 1; index++) {
			totalLength += GetLengthBetweenKnots(index, index + 1);
			accumLength[index] = totalLength;
			if (length < accumLength[index]) {
				break;
			}
		}

		if (index >= this->Num() - 1) {
			return TimeAt(this->Num() - 1);
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
		t = (TimeAt(index + 1) - TimeAt(index)) * len0 / len1;
		for (i = 0; i < 32; i++) {
			diff = RombergIntegral(TimeAt(index), TimeAt(index) + t, 5) - len0;
			if (cweeMath::Fabs(diff) <= epsilon) {
				return TimeAt(index) + t;
			}
			t -= diff / GetSpeed(TimeAt(index) + t);
		}
		return TimeAt(index) + t;
	};
	u64					GetLengthBetweenKnots(const int i0, const int i1) const {
		u64 length = 0.0f;
		for (int i = i0; i < i1; i++) {
			length += RombergIntegral(TimeAt(i), TimeAt(i + 1), 5);
		}
		return length;
	};

	void				MakeUniform(const u64 totalTime) {
		int i, n;

		n = this->Num() - 1;
		for (i = 0; i <= n; i++) {
			TimeAt(i) = i * totalTime / n;
		}
		changed = true;
	};
	void				SetConstantSpeed(const u64 totalTime) {
		int i, j;
		u64* length, totalLength, scale, t;

		length = (u64*)_alloca16(this->Num() * sizeof(u64));
		totalLength = 0.0f;
		for (i = 0; i < this->Num() - 1; i++) {
			length[i] = GetLengthBetweenKnots(i, i + 1);
			totalLength += length[i];
		}
		scale = totalTime / totalLength;
		for (t = 0.0f, i = 0; i < this->Num() - 1; i++) {
			TimeAt(i) = t;
			t += scale * length[i];
		}
		TimeAt(this->Num() - 1) = totalTime;
		changed = true;
	};
	void				ShiftTime(const u64 deltaTime) {
		for (int i = 0; i < this->Num(); i++) {
			TimeAt(i) += deltaTime;
		}
		changed = true;
	};
	void				Translate(const type& translation) {
		for (int i = 0; i < this->Num(); i++) {
			ValueAt(i) += translation;
		}
		changed = true;
	};
	u64					RombergIntegral(const u64 t0, const u64 t1, const int order) const {
		int i, j, k, m, n;
		u64 sum, delta;
		u64* temp[2];

		temp[0] = (u64*)_alloca16(order * sizeof(u64));
		temp[1] = (u64*)_alloca16(order * sizeof(u64));

		delta = t1 - t0;
		temp[0][0] = 0.5f * delta * (GetSpeed(t0) + GetSpeed(t1));

		for (i = 2, m = 1; i <= order; i++, m *= 2, delta *= 0.5f) {

			// approximate using the trapezoid rule
			sum = 0.0f;
			for (j = 1; j <= m; j++) {
				sum += GetSpeed(t0 + delta * (j - 0.5f));
			}

			// Richardson extrapolation
			temp[1][0] = 0.5f * (temp[0][0] + delta * sum);
			for (k = 1, n = 4; k < i; k++, n *= 4) {
				temp[1][k] = (n * temp[1][k - 1] - temp[0][k - 1]) / (n - 1);
			}

			for (j = 0; j < i; j++) {
				temp[0][j] = temp[1][j];
			}
		}
		return temp[0][order - 1];
	};
	cweeStr				Name = "No Name";
	cweeStr				getName(void) const { return Name; };
	void				setName(const cweeStr& newName) { Name = newName; };
	size_t				MemoryUsed(void) const { size_t Bytes = 0; Bytes += knots.Size(); return Bytes; }
	u64					TimeForIndex(const int index) const {
		int n = this->Num() - 1;

		if (index < 0) {
			return TimeAt(0) + index * (TimeAt(1) - TimeAt(0));
		}
		else if (index > n) {
			return TimeAt(n) + (index - n) * (TimeAt(n) - TimeAt(n - 1));
		}
		return TimeAt(index);
	};
	type				ValueForIndex(int index) const {
		int n;
		if (index < 0) {
			return ValueAt(0);
		}
		n = this->Num() - 1;
		if (index > n) {
			return ValueAt(n);
		}
		return ValueAt(index);
	};
	int					IndexForTime(const u64& time) const {
		if (this->Num() <= 0) return 0; // there is no other data...

		int len, mid, offset;
		bool res;

		if (currentIndex >= 0 && currentIndex < Num()) {
			// use the cached index if it is still valid
			if (currentIndex == 0) {
				if (time <= TimeAt(currentIndex)) {
					return currentIndex;
				}
			}
			else if (currentIndex == Num()) {
				if (time > TimeAt(currentIndex - 1)) {
					return currentIndex;
				}
			}
			else if (time > TimeAt(currentIndex - 1) && time <= TimeAt(currentIndex)) {
				return currentIndex;
			}
			else if (time > TimeAt(currentIndex) && (currentIndex + 1 == Num() || time <= TimeAt(currentIndex + 1))) {
				// use the next index
				currentIndex++;
				return currentIndex;
			}
		}

		// use binary search to find the index for the given time
		len = this->Num();
		mid = len;
		offset = 0;
		res = false;
		u64* sample;

		while (mid > 0) {
			mid = len >> 1;
			// OPTIMIZED ORDERING
			sample = &TimeAt(offset + mid);
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
	int					FindExactX(const u64& time) const {
		int search;

		{
			search = IndexForTime(time);
			if (search >= this->Num() || search < 0) {
				search = -1; // out of bounds
			}
			else if (TimeAt(search) == time) {
				// no issues
			}
			else if ((search + 1) < this->Num() && (TimeAt(search + 1) == time)) {
				search++; // was up one value for some reason
			}
			else {
				search = -1;
			}
		}

		return search;
	};
	int					FindExactY(const type& val) const {
		int i = 0;
		for (auto& j : knots) {
			if (j.get<1>() == val) {
				return i;
			}
			i++;
		}
		return -1;
	};

	virtual void		RemoveUnnecessaryKnots() {
		int num = GetNumValues();
		if (num > 5) {
			cweeThreadedList<int> indexesToDelete(GetNumValues() + 16);

			// remove duplicate datapoints that would result in a straight line with the catmull-rom spline. 
			// reduce 5 identical data points to 4 by removing the middle and re-doing the check from the second point. 
			int currentPos = 1;
			type* val = GetValueAddress(currentPos - 1);
			type* val2 = GetValueAddress(currentPos + 0);
			type* val3 = GetValueAddress(currentPos + 1);
			type* val4 = GetValueAddress(currentPos + 2);
			type* val5 = GetValueAddress(currentPos + 3);
			do {
				if (num <= (currentPos + 3)) break;
				if (!(*val == *val2)) { // !=
					++currentPos;
					val = val2;
					val2 = val3;
					val3 = val4;
					val4 = val5;
					val5 = GetValueAddress(currentPos + 3);
					continue;
				}
				else if ((*val5 == *val2) && (*val4 == *val2) && (*val3 == *val2)) {
					indexesToDelete.Append(currentPos + 1);
					++currentPos;
					val3 = val4;
					val4 = val5;
					val5 = GetValueAddress(currentPos + 3);
					continue; // don't move the currentPos forward. Repeat the analysis from this spot. 																			
				}
				++currentPos;
				val = val2;
				val2 = val3;
				val3 = val4;
				val4 = val5;
				val5 = GetValueAddress(currentPos + 3);
			} while (1);

			knots.RemoveIndexes(indexesToDelete);
		}
	};



	/*!
	Get list of spline knots between the requested time
	*/
	virtual cweeThreadedList<std::pair<u64, type>>	GetKnotSeries(const u64 timeStart = -std::numeric_limits < u64>::max(), const u64 timeEnd = std::numeric_limits < u64>::max()) const {
		int numKnots = this->Num();
		cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);

		for (int i = 0; i < numKnots; i++) {
			auto time = this->TimeAt(i);
			if (time >= timeStart) {
				if (time < timeEnd) {
					out.Append(std::make_pair(time, this->ValueAt(i)));
				}
				else return out;
			}
		}
		return out;
	};

	template<class type> cweeStr						SerializeKnots() {
		cweeStr delim = ":Knots_DELIM:";
		cweeStr out;
		int numItems = this->Num();
		cweeStr recorder;
		for (int i = 0; i < numItems; i++) {
			recorder.Clear();
			recorder.Append(cweeStr((u64)(this->TimeAt(i)) / serializationTimeConverter));
			recorder.Append(',');
			recorder.Append(cweeStr((u64)this->ValueAt(i)));
			out.AddToDelimiter(recorder, delim);
		}
		return out;
	}

	template<> cweeStr									SerializeKnots<cweeStr>() {
		RemoveUnnecessaryKnots();

		cweeStr delim = ":Knots_DELIM:";
		cweeStr out;
		int numItems = this->Num();
		cweeStr recorder;
		for (int i = 0; i < numItems; i++) {
			recorder.Clear();
			recorder.Append(cweeStr((u64)(this->TimeAt(i)) / serializationTimeConverter));
			recorder.Append(',');
			recorder.Append(this->ValueAt(i));
			out.AddToDelimiter(recorder, delim);
		}
		return out;
	}

	template<class type> void							DeserializeKnots(cweeStr& in) {
		const cweeStr delim(":Knots_DELIM:");
		this->knots.Clear();
		if (!in.IsEmpty()) {
			cweeParser obj(in, delim, true);
			int finder(0); cweeStr left; cweeStr right;
			for (auto& x : obj) {
				finder = x.Find(',');
				if (finder != -1) {
					x.Mid(0, finder, left);
					x.Mid(finder + 1, x.Length(), right);
					this->AddValue((((u64)(left)) * serializationTimeConverter), (u64)right);
				}
			}
		}
	}

	template<> void										DeserializeKnots<cweeStr>(cweeStr& in) {
		const cweeStr delim(":Knots_DELIM:");
		this->knots.Clear();
		if (!in.IsEmpty()) {
			cweeParser obj(in, delim, true);
			int finder(0); cweeStr left; cweeStr right;
			for (auto& x : obj) {
				finder = x.Find(',');
				if (finder != -1) {
					x.Mid(0, finder, left);
					x.Mid(finder + 1, x.Length(), right);
					this->AddValue((((u64)(left)) * serializationTimeConverter), right);
				}
			}
		}
	}

	cweeStr				Serialize(int option = -1) {
		cweeStr delim = ":CURVE_in_DELIM:";
		cweeStr out;
		if (((cweeStr)this->Name).IsEmpty()) out = " "; else out.AddToDelimiter((cweeStr)this->Name, delim);
		out.AddToDelimiter((int)this->boundaryType, delim);
		out.AddToDelimiter((u64)this->closeTime, delim);
		out.AddToDelimiter(SerializeKnots<type>(), delim);
		return out;
	};
	void				Deserialize(cweeStr& in) {
		cweeParser obj(in, ":CURVE_in_DELIM:", true);
		this->Name = obj.getVar(0);
		this->boundaryType = (boundary_t)(int)(u64)obj.getVar(1);
		this->closeTime = (u64)obj.getVar(2);
		DeserializeKnots<type>(obj.getVar(3));
	};

	mutable cweeLinkedList<cweeUnion<u64, type>> knots;

protected:
	boundary_t			boundaryType = boundary_t::BT_FREE;
	u64					closeTime = 0;
	mutable int			currentIndex;	// cached index for fast lookup
	mutable bool		changed;		// set whenever the curve changes
	u64				GetSpeed(const u64 time) const {
		int i;
		u64 speed;
		type value;

		value = GetCurrentFirstDerivative(time);
		return (u64)value;
	};
	u64& TimeAt(int index) const {
		return knots[index].get<0>();
	};
	type& ValueAt(int index) const {
		return knots[index].get<1>();
	};
	void				InsertValue_Impl(int index, u64 const& t, type const& v) {
		knots.Insert(cweeUnion<u64, type>(t, v), index);
	};
	void				InsertValue_Impl(int index, u64 const& t, type&& v) {
		knots.Insert(cweeUnion<u64, type>(t, std::forward<type>(v)), index);
	};
};

/*
===============================================================================

	Spline base template.

===============================================================================
*/
template< class type >
class cweeCurve_Spline : public cweeCurve<type> {
public:
	cweeCurve_Spline() {
		this->boundaryType = boundary_t::BT_FREE;
		this->closeTime = 0.0f;
	};
	virtual bool		IsDone(const u64 time) const {
		return (this->boundaryType != boundary_t::BT_CLOSED && time >= this->TimeAt(this->Num() - 1));
	};
	virtual void		SetBoundaryType(const boundary_t bt) { this->boundaryType = bt; this->changed = true; }
	virtual boundary_t	GetBoundaryType() const { return this->boundaryType; }
	virtual void		SetCloseTime(const u64 t) { this->closeTime = t; this->changed = true; }
	virtual u64			GetCloseTime() { return this->boundaryType == boundary_t::BT_CLOSED ? this->closeTime : 0.0f; }
	type				ValueForIndex(const int index) const {
		int n = this->Num() - 1;

		if (index < 0) {
			if (this->boundaryType == boundary_t::BT_CLOSED) {
				return this->ValueAt(this->Num() + index % this->Num());
			}
			else {
				return this->ValueAt(0) + index * (this->ValueAt(1) - this->ValueAt(0));
			}
		}
		else if (index > n) {
			if (this->boundaryType == boundary_t::BT_CLOSED) {
				return this->ValueAt(index % this->Num());
			}
			else {
				return this->ValueAt(n) + (index - n) * (this->ValueAt(n) - this->ValueAt(n - 1));
			}
		}
		return this->ValueAt(index);
	};
	u64					TimeForIndex(const int index) const {
		int n = this->Num() - 1;

		if (index < 0) {
			if (this->boundaryType == boundary_t::BT_CLOSED) {
				return (index / this->Num()) * (this->TimeAt(n) + this->closeTime) - (this->TimeAt(n) + this->closeTime - this->TimeAt(this->Num() + index % this->Num()));
			}
			else {
				return this->TimeAt(0) + index * (this->TimeAt(1) - this->TimeAt(0));
			}
		}
		else if (index > n) {
			if (this->boundaryType == boundary_t::BT_CLOSED) {
				return (index / this->Num()) * (this->TimeAt(n) + this->closeTime) + this->TimeAt(index % this->Num());
			}
			else {
				return this->TimeAt(n) + (index - n) * (this->TimeAt(n) - this->TimeAt(n - 1));
			}
		}
		return this->TimeAt(index);
	};

protected:
	u64					ClampedTime(const u64 t) const {
		if (this->boundaryType == boundary_t::BT_CLAMPED) {
			if (t < this->TimeAt(0)) {
				return this->TimeAt(0);
			}
			else if (t >= this->TimeAt(this->Num() - 1)) {
				return this->TimeAt(this->Num() - 1);
			}
		}
		return t;
	};
};

/*
===============================================================================

	Uniform Cubic Interpolating Spline template.
	The curve goes through all the knots.

===============================================================================
*/
template< class type >
class cweeCurve_CatmullRomSpline final : public cweeCurve_Spline<type> {
public:
	cweeCurve_CatmullRomSpline() {
	};

	virtual type		GetCurrentValue(const u64 time) const {
		int i, j, k;
		u64 bvals[4], clampedTime;
		type v;
		v = 0; //-V501

		if (this->Num() < 1) {
			return v;
		}
		if (this->Num() == 1) {
			return this->ValueAt(0);
		}

		clampedTime = this->ClampedTime(time);
		i = this->IndexForTime(clampedTime);
		Basis(i - 1, clampedTime, bvals);
		for (j = 0; j < 4; j++) {
			k = i + j - 2;
			v += (this->ValueForIndex(k) * (float)bvals[j]);
		}

		return v;

		//int i, j, k;
		//u64 bvals[4], clampedTime;
		//type v;

		//if (this->Num() == 1) {
		//	return this->ValueAt(0);
		//}

		//clampedTime = this->ClampedTime(time);
		//i = this->IndexForTime(clampedTime);
		//Basis(i - 1, clampedTime, bvals);
		//v = 0; //-V501
		//for (j = 0; j < 4; j++) {
		//	k = i + j - 2;
		//	v += bvals[j] * this->ValueForIndex(k);
		//}
		//return v;
	};
	virtual type		GetCurrentFirstDerivative(const u64 time) const {
		int i, j, k;
		u64 bvals[4], d, clampedTime;
		type v;
		v = 0; //-V501

		if (this->Num() < 1) {
			return v;
		}
		if (this->Num() == 1) {
			return 0; //-V501
		}

		clampedTime = this->ClampedTime(time);
		i = this->IndexForTime(clampedTime);
		BasisFirstDerivative(i - 1, clampedTime, bvals);

		for (j = 0; j < 4; j++) {
			k = i + j - 2;
			v += bvals[j] * this->ValueForIndex(k);
		}
		d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
		return v / d;
	};
	virtual type		GetCurrentSecondDerivative(const u64 time) const {
		int i, j, k;
		u64 bvals[4], d, clampedTime;
		type v;
		v = 0; //-V501

		if (this->Num() < 1) {
			return v;
		}
		if (this->Num() == 1) {
			return (this->ValueAt(0) - this->ValueAt(0)); //-V501
		}

		clampedTime = this->ClampedTime(time);
		i = this->IndexForTime(clampedTime);
		BasisSecondDerivative(i - 1, clampedTime, bvals);
		v = 0; //-V501
		for (j = 0; j < 4; j++) {
			k = i + j - 2;
			v += bvals[j] * this->ValueForIndex(k);
		}
		d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
		return v / (d * d);
	};

protected:
	void				Basis(const int index, const u64 t, u64* bvals) const {
		u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
		bvals[0] = ((-s + 2.0f) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
		bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;	// 1.5f * s * s * s - 2.5f * s * s + 1.0f
		bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
		bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
	};
	void				BasisFirstDerivative(const int index, const u64 t, u64* bvals) const {
		u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
		bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
		bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
		bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
		bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
	};
	void				BasisSecondDerivative(const int index, const u64 t, u64* bvals) const {
		u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
		bvals[0] = -3.0f * s + 2.0f;
		bvals[1] = 9.0f * s - 5.0f;
		bvals[2] = -9.0f * s + 4.0f;
		bvals[3] = 3.0f * s - 1.0f;
	};

};