
#ifndef __CURVE_H__
#define __CURVE_H__

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


// #define useLinkedList
template< class type >
class cweeCurve {
public:
#ifdef useLinkedList
	typedef cweeLinkedList<type>	valueList;
	typedef cweeLinkedList<u64>		keyList;
#else
	typedef cweeThreadedList<type>	valueList;
	typedef cweeThreadedList<u64>	keyList;
#endif


	cweeCurve();

	virtual				~cweeCurve();
	virtual int			AddUniqueValue(const u64& time, const type& value);
	virtual int			AddValue(const u64& time, const type& value);
	virtual void		RemoveIndex(const int index) { values.RemoveIndex(index); times.RemoveIndex(index); changed = true; }
	virtual void		Clear() { values.Clear(); times.Clear(); currentIndex = -1; changed = true; }

	virtual	void		SetGranularity(int size) {
		(times).SetGranularity(size);
		(values).SetGranularity(size);
	};
	virtual	int			GetGranularity() { return (times).GetGranularity(); };
	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

	virtual bool		IsDone(const u64 time) const;
	u64					GetMaxTime(void) const {
		if (GetNumValues() == 0)
			return 0;
		else
			return times[GetNumValues() - 1];
	}
	u64					GetMinTime(void) const {
		if (GetNumValues() == 0)
			return 0;
		else
			return times[0];
	}

	int					GetNumValues() const { return values.NumRef(); }
	void				SetValue(const int index, const type& value) { values[index] = value; changed = true; }
	type				GetValue(int index) const { 
		type out;
		if (index < values.NumRef() && index >= 0)
			out = values[index];
		else if (index >= values.NumRef() && values.NumRef() > 0)
			out = values[values.NumRef() - 1];
		else if (index < 0 && values.NumRef() > 0)
			out = values[0];
		else
			out = 0;
		return out; 
	}
	type*				GetValueAddress(int index) { 
		if (index < values.NumRef() && index >= 0)
			return &values[index];
		else if (index >= values.NumRef() && values.NumRef() > 0)
			return &values[values.NumRef() - 1];
		else if (index < 0 && values.NumRef() > 0)
			return &values[0];
		else
			return nullptr;	
	}
	u64				GetTime(int index) const { return times[index]; }

	u64				GetLengthForTime(const u64 time) const;
	u64				GetTimeForLength(const u64 length, const u64 epsilon = 0.1f) const;
	u64				GetLengthBetweenKnots(const int i0, const int i1) const;

	void				MakeUniform(const u64 totalTime);
	void				SetConstantSpeed(const u64 totalTime);
	void				ShiftTime(const u64 deltaTime);
	void				Translate(const type& translation);
	u64					RombergIntegral(const u64 t0, const u64 t1, const int order) const;
	cweeStr				Name = "No Name";
	cweeStr				getName(void) const { return Name; };
	void				setName(const cweeStr& newName) { Name = newName; };
	size_t				MemoryUsed(void) const { size_t Bytes = 0; Bytes += times.Size(); Bytes += values.Size(); return Bytes; }
	u64					TimeForIndex(const int index) const;
	type				ValueForIndex(int index) const; 
	int					IndexForTime(const u64& time) const {
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
		len = times.NumRef();
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
	int					FindExactX(const u64& time) const {
		int search;

		//Lock(); 
		{
			search = IndexForTime(time);
			if (search >= values.NumRef() || search < 0) {
				search = -1; // out of bounds
			}
			else if (times[search] == time) {
				// no issues
			}
			else if ((search + 1) < values.NumRef() && (times[search + 1] == time)) {
				search++; // was up one value for some reason
			}
			else {
				search = -1;
			}
		} 
		//Unlock();

		return search;



		// return times.FindIndex(time);
	};
	int					FindExactY(const type& val) const {
		return values.FindIndex(val);
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

			times.RemoveIndexes(indexesToDelete);
			values.RemoveIndexes(indexesToDelete);
		}
	};

	/*!
	Get list of spline knots between the requested times
	*/
	virtual cweeThreadedList<std::pair<u64, type>>	GetKnotSeries(const u64 timeStart = -std::numeric_limits < u64>::max(), const u64 timeEnd = std::numeric_limits < u64>::max()) const {
		int numKnots = (this->values).NumRef();
		cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);

		for (int i = 0; i < numKnots; i++) {
			auto time = (this->times)[i];
			if (time >= timeStart) {
				if (time < timeEnd) {
					out.Append(std::make_pair(time, (this->values)[i]));
				}
				else return out;
			}
		}
		return out;
	};

	template<class type> cweeStr						SerializeKnots() {
		cweeStr delim = ":Knots_DELIM:";
		cweeStr out;
		int numItems = (this->times).NumRef();
		cweeStr recorder;
		for (int i = 0; i < numItems; i++) {
			recorder.Clear();
			recorder.Append(cweeStr((u64)((this->times)[i]) / serializationTimeConverter));
			recorder.Append(',');
			recorder.Append(cweeStr((u64)(this->values)[i]));
			out.AddToDelimiter(recorder, delim);
		}
		return out;
	}

	template<> cweeStr									SerializeKnots<cweeStr>() {
		RemoveUnnecessaryKnots();
					
		cweeStr delim = ":Knots_DELIM:";
		cweeStr out;
		int numItems = (this->times).NumRef();
		cweeStr recorder;
		for (int i = 0; i < numItems; i++) {
			recorder.Clear();
			recorder.Append(cweeStr((u64)((this->times)[i]) / serializationTimeConverter));
			recorder.Append(',');
			recorder.Append((this->values)[i]);
			out.AddToDelimiter(recorder, delim);
		}
		return out;
	}

	template<class type> void							DeserializeKnots(cweeStr& in) {
		const cweeStr delim(":Knots_DELIM:");
		(this->values).Clear();
		(this->times).Clear();
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
		(this->values).Clear();
		(this->times).Clear();
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

	mutable keyList		times;			// knots
	mutable valueList	values;			// knot values
protected:

	boundary_t			boundaryType = boundary_t::BT_FREE;
	u64					closeTime = 0;
	mutable int			currentIndex;	// cached index for fast lookup
	mutable bool		changed;		// set whenever the curve changes

	u64				GetSpeed(const u64 time) const;
};

/*
====================
cweeCurve::cweeCurve
====================
*/
template< class type >
INLINE cweeCurve<type>::cweeCurve() {
	currentIndex = -1;
	times.SetGranularity(64);
	values.SetGranularity(64);
	setName("No Name");
	changed = false;
}

/*
====================
cweeCurve::~cweeCurve
====================
*/
template< class type >
INLINE cweeCurve<type>::~cweeCurve() {
}

/*
====================
cweeCurve::AddValue

  add a timed/value pair to the spline
  returns the index to the inserted pair
====================
*/
template< class type >
INLINE int cweeCurve<type>::AddValue(const u64& time, const type& value) {
	int i = IndexForTime(time);
	(times).Insert(time, i);
	(values).Insert(value, i);
	if ((values.NumRef() + 1) >= (GetGranularity() * GRANULARITY_SCALER)) SetGranularity(GetGranularity() * GRANULARITY_SCALER);
	changed = true;
	return i;
}

template< class type >
INLINE int cweeCurve<type>::AddUniqueValue(const u64& time, const type& value) { // slower with guarrantee of uniqueness 
	int i = IndexForTime(time);

#if 0

	if (i != 0) {
		// if the time already exists, simply override its value 
		int index1, index2, index3;
		index1 = i - 1; if (index1 < 0)	index1 = i;
		index2 = i;
		index3 = i + 1; if (index3 >= (times).NumRef()) index3 = i;

		if ((times)[index1] == time) {
			(values)[index1] = value;
			changed = true;
			return (index1);
		}
		if ((times)[index2] == time) {
			(values)[index2] = value;
			changed = true;
			return (index2);
		}
		if ((times)[index3] == time) {
			(values)[index3] = value;
			changed = true;
			return (index3);
		}
	}
	if (GetNumValues() == 0 || GetMinTime() != time) {
		(times).Insert(time, i);
		(values).Insert(value, i);

		if ((GetNumValues() + 1) >= (GetGranularity() * 10)) SetGranularity(GetGranularity() * 10);
	}
	else {
		(values)[0] = value;
	}
#else

	if (((i != 0) && (i < values.NumRef()) && (times[i] == time))) {
		values[i] = value;
		// finished;
		changed = true;
		return i;
	}
	u64 minTime; {
		if (times.NumRef() == 0) minTime = -std::numeric_limits < u64>::max();
		else minTime = times[0];
	}
	if (values.NumRef() == 0 || minTime != time) {
		(times).Insert(time, i);
		(values).Insert(value, i);
		if ((values.NumRef() + 1) >= (GetGranularity() * GRANULARITY_SCALER)) SetGranularity(GetGranularity() * GRANULARITY_SCALER);
	}
	else {
		values[0] = value;
		i = 0;
	}

	//if (i != 0 && i < (times).NumRef()) {	
	//	if ((times)[i] == time)
	//	{
	//		(values)[i] = value;
	//		changed = true;
	//		return (i);
	//	}
	//}
	//if (GetNumValues() == 0 || GetMinTime() != time) {
	//	(times).Insert(time, i);
	//	(values).Insert(value, i);

	//	if ((GetNumValues() + 1) >= (GetGranularity() * 10)) SetGranularity(GetGranularity() * 10);
	//}
	//else {
	//	(values)[0] = value;
	//}

#endif

	changed = true;
	return i;
}

/*
====================
cweeCurve::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve<type>::GetCurrentValue(const u64 time) const {
	int i;
	type v = type();
	int n = values.NumRef();
	if (n < 1) {
		return v;
	}
	if (n == 1) {
		return values[0];
	}

	i = IndexForTime(time);
	if (i >= n) {
		return values[n - 1];
	}
	else {
		return values[i];
	}
}

/*
====================
cweeCurve::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve<type>::GetCurrentFirstDerivative(const u64 time) const {
	return (values[0]); //-V501
}

/*
====================
cweeCurve::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve<type>::GetCurrentSecondDerivative(const u64 time) const {
	return (values[0]); //-V501
}

/*
====================
cweeCurve::IsDone
====================
*/
template< class type >
INLINE bool cweeCurve<type>::IsDone(const u64 time) const {
	return (time >= times[times.NumRef() - 1]);
}

/*
====================
cweeCurve::GetSpeed
====================
*/
template< class type >
INLINE u64 cweeCurve<type>::GetSpeed(const u64 time) const {
	int i;
	u64 speed;
	type value;

	value = GetCurrentFirstDerivative(time);
	return (u64)value;
}

/*
====================
cweeCurve::RombergIntegral
====================
*/
template< class type >
INLINE u64 cweeCurve<type>::RombergIntegral(const u64 t0, const u64 t1, const int order) const {
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
}

/*
====================
cweeCurve::GetLengthBetweenKnots
====================
*/
template< class type >
INLINE u64 cweeCurve<type>::GetLengthBetweenKnots(const int i0, const int i1) const {
	u64 length = 0.0f;
	for (int i = i0; i < i1; i++) {
		length += RombergIntegral(times[i], times[i + 1], 5);
	}
	return length;
}

/*
====================
cweeCurve::GetLengthForTime
====================
*/
template< class type >
INLINE u64 cweeCurve<type>::GetLengthForTime(const u64 time) const {
	u64 length = 0.0f;
	int index = IndexForTime(time);
	for (int i = 0; i < index; i++) {
		length += RombergIntegral(times[i], times[i + 1], 5);
	}
	length += RombergIntegral(times[index], time, 5);
	return length;
}

/*
====================
cweeCurve::GetTimeForLength
====================
*/
template< class type >
INLINE u64 cweeCurve<type>::GetTimeForLength(const u64 length, const u64 epsilon) const {
	int i, index;
	u64* accumLength, totalLength, len0, len1, t, diff;

	if (length <= 0.0f) {
		return times[0];
	}

	accumLength = (u64*)_alloca16(values.NumRef() * sizeof(u64));
	totalLength = 0.0f;
	for (index = 0; index < values.NumRef() - 1; index++) {
		totalLength += GetLengthBetweenKnots(index, index + 1);
		accumLength[index] = totalLength;
		if (length < accumLength[index]) {
			break;
		}
	}

	if (index >= values.NumRef() - 1) {
		return times[times.NumRef() - 1];
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
	t = (times[index + 1] - times[index]) * len0 / len1;
	for (i = 0; i < 32; i++) {
		diff = RombergIntegral(times[index], times[index] + t, 5) - len0;
		if (cweeMath::Fabs(diff) <= epsilon) {
			return times[index] + t;
		}
		t -= diff / GetSpeed(times[index] + t);
	}
	return times[index] + t;
}

/*
====================
cweeCurve::MakeUniform
====================
*/
template< class type >
INLINE void cweeCurve<type>::MakeUniform(const u64 totalTime) {
	int i, n;

	n = times.NumRef() - 1;
	for (i = 0; i <= n; i++) {
		times[i] = i * totalTime / n;
	}
	changed = true;
}

/*
====================
cweeCurve::SetConstantSpeed
====================
*/
template< class type >
INLINE void cweeCurve<type>::SetConstantSpeed(const u64 totalTime) {
	int i, j;
	u64* length, totalLength, scale, t;

	length = (u64*)_alloca16(values.NumRef() * sizeof(u64));
	totalLength = 0.0f;
	for (i = 0; i < values.NumRef() - 1; i++) {
		length[i] = GetLengthBetweenKnots(i, i + 1);
		totalLength += length[i];
	}
	scale = totalTime / totalLength;
	for (t = 0.0f, i = 0; i < times.NumRef() - 1; i++) {
		times[i] = t;
		t += scale * length[i];
	}
	times[times.NumRef() - 1] = totalTime;
	changed = true;
}

/*
====================
cweeCurve::ShiftTime
====================
*/
template< class type >
INLINE void cweeCurve<type>::ShiftTime(const u64 deltaTime) {
	for (int i = 0; i < times.NumRef(); i++) {
		times[i] += deltaTime;
	}
	changed = true;
}

/*
====================
cweeCurve::Translate
====================
*/
template< class type >
INLINE void cweeCurve<type>::Translate(const type& translation) {
	for (int i = 0; i < values.NumRef(); i++) {
		values[i] += translation;
	}
	changed = true;
}

///*
//====================
//cweeCurve::IndexForTime
//  find the index for the first time greater than or equal to the given time
//====================
//*/
//template< class type >
//INLINE int cweeCurve<type>::IndexForTime(const u64& time) const

/*
====================
cweeCurve::ValueForIndex
  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve<type>::ValueForIndex(int index) const {
	int n;
	if (index < 0) {
		return values[0];
	}
	n = values.NumRef() - 1;
	if (index > n) {
		return values[n];
	}
	return values[index];
}

/*
====================
cweeCurve::TimeForIndex
  get the value for the given time
====================
*/
template< class type >
INLINE u64 cweeCurve<type>::TimeForIndex(const int index) const {
	int n = times.NumRef() - 1;

	if (index < 0) {
		return times[0] + index * (times[1] - times[0]);
	}
	else if (index > n) {
		return times[n] + (index - n) * (times[n] - times[n - 1]);
	}
	return times[index];
}


/*
===============================================================================
	Bezier Curve template.
	The degree of the polynomial equals the number of knots minus one.
===============================================================================
*/

template< class type >
class cweeCurve_Bezier : public cweeCurve<type> {
public:
	cweeCurve_Bezier();

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	void				Basis(const int order, const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const int order, const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const int order, const u64 t, u64* bvals) const;
};

/*
====================
cweeCurve_Bezier::cweeCurve_Bezier
====================
*/
template< class type >
INLINE cweeCurve_Bezier<type>::cweeCurve_Bezier() {
}

/*
====================
cweeCurve_Bezier::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_Bezier<type>::GetCurrentValue(const u64 time) const {
	int i;
	u64* bvals;
	type v;

	bvals = (u64*)_alloca16(this->values.NumRef() * sizeof(u64));

	Basis(this->values.NumRef(), time, bvals);
	v = bvals[0] * this->values[0];
	for (i = 1; i < this->values.NumRef(); i++) {
		v += bvals[i] * this->values[i];
	}
	return v;
}

/*
====================
cweeCurve_Bezier::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_Bezier<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i;
	u64* bvals, d;
	type v;

	bvals = (u64*)_alloca16(this->values.NumRef() * sizeof(u64));

	BasisFirstDerivative(this->values.NumRef(), time, bvals);
	v = bvals[0] * this->values[0];
	for (i = 1; i < this->values.NumRef(); i++) {
		v += bvals[i] * this->values[i];
	}
	d = (this->times[this->times.NumRef() - 1] - this->times[0]);
	return ((u64)(this->values.NumRef() - 1) / d) * v;
}

/*
====================
cweeCurve_Bezier::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_Bezier<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i;
	u64* bvals, d;
	type v;

	bvals = (u64*)_alloca16(this->values.NumRef() * sizeof(u64));

	BasisSecondDerivative(this->values.NumRef(), time, bvals);
	v = bvals[0] * this->values[0];
	for (i = 1; i < this->values.NumRef(); i++) {
		v += bvals[i] * this->values[i];
	}
	d = (this->times[this->times.NumRef() - 1] - this->times[0]);
	return ((u64)(this->values.NumRef() - 2) * (this->values.NumRef() - 1) / (d * d)) * v;
}

/*
====================
cweeCurve_Bezier::Basis

  bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_Bezier<type>::Basis(const int order, const u64 t, u64* bvals) const {
	int i, j, d;
	u64* c, c1, c2, s, o, ps, po;

	bvals[0] = 1.0f;
	d = order - 1;
	if (d <= 0) {
		return;
	}

	c = (u64*)_alloca16((d + 1) * sizeof(u64));
	s = (u64)(t - this->times[0]) / (this->times[this->times.NumRef() - 1] - this->times[0]);
	o = 1.0f - s;
	ps = s;
	po = o;

	for (i = 1; i < d; i++) {
		c[i] = 1.0f;
	}
	for (i = 1; i < d; i++) {
		c[i - 1] = 0.0f;
		c1 = c[i];
		c[i] = 1.0f;
		for (j = i + 1; j <= d; j++) {
			c2 = c[j];
			c[j] = c1 + c[j - 1];
			c1 = c2;
		}
		bvals[i] = c[d] * ps;
		ps *= s;
	}
	for (i = d - 1; i >= 0; i--) {
		bvals[i] *= po;
		po *= o;
	}
	bvals[d] = ps;
}

/*
====================
cweeCurve_Bezier::BasisFirstDerivative

  first derivative of bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_Bezier<type>::BasisFirstDerivative(const int order, const u64 t, u64* bvals) const {
	int i;

	Basis(order - 1, t, bvals + 1);
	bvals[0] = 0.0f;
	for (i = 0; i < order - 1; i++) {
		bvals[i] -= bvals[i + 1];
	}
}

/*
====================
cweeCurve_Bezier::BasisSecondDerivative

  second derivative of bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_Bezier<type>::BasisSecondDerivative(const int order, const u64 t, u64* bvals) const {
	int i;

	BasisFirstDerivative(order - 1, t, bvals + 1);
	bvals[0] = 0.0f;
	for (i = 0; i < order - 1; i++) {
		bvals[i] -= bvals[i + 1];
	}
}


/*
===============================================================================

	Quadratic Bezier Curve template.
	Should always have exactly three knots.

===============================================================================
*/

template< class type >
class cweeCurve_QuadraticBezier : public cweeCurve<type> {

public:
	cweeCurve_QuadraticBezier();

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	void				Basis(const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const u64 t, u64* bvals) const;
};

/*
====================
cweeCurve_QuadraticBezier::cweeCurve_QuadraticBezier
====================
*/
template< class type >
INLINE cweeCurve_QuadraticBezier<type>::cweeCurve_QuadraticBezier() {
}


/*
====================
cweeCurve_QuadraticBezier::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_QuadraticBezier<type>::GetCurrentValue(const u64 time) const {
	u64 bvals[3];
	assert(this->values.NumRef() == 3);
	Basis(time, bvals);
	return (bvals[0] * this->values[0] + bvals[1] * this->values[1] + bvals[2] * this->values[2]);
}

/*
====================
cweeCurve_QuadraticBezier::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_QuadraticBezier<type>::GetCurrentFirstDerivative(const u64 time) const {
	u64 bvals[3], d;
	assert(this->values.NumRef() == 3);
	BasisFirstDerivative(time, bvals);
	d = (this->times[2] - this->times[0]);
	return (bvals[0] * this->values[0] + bvals[1] * this->values[1] + bvals[2] * this->values[2]) / d;
}

/*
====================
cweeCurve_QuadraticBezier::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_QuadraticBezier<type>::GetCurrentSecondDerivative(const u64 time) const {
	u64 bvals[3], d;
	assert(this->values.NumRef() == 3);
	BasisSecondDerivative(time, bvals);
	d = (this->times[2] - this->times[0]);
	return (bvals[0] * this->values[0] + bvals[1] * this->values[1] + bvals[2] * this->values[2]) / (d * d);
}

/*
====================
cweeCurve_QuadraticBezier::Basis

  quadratic bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_QuadraticBezier<type>::Basis(const u64 t, u64* bvals) const {
	u64 s1 = (u64)(t - this->times[0]) / (this->times[2] - this->times[0]);
	u64 s2 = s1 * s1;
	bvals[0] = s2 - 2.0f * s1 + 1.0f;
	bvals[1] = -2.0f * s2 + 2.0f * s1;
	bvals[2] = s2;
}

/*
====================
cweeCurve_QuadraticBezier::BasisFirstDerivative

  first derivative of quadratic bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_QuadraticBezier<type>::BasisFirstDerivative(const u64 t, u64* bvals) const {
	u64 s1 = (u64)(t - this->times[0]) / (this->times[2] - this->times[0]);
	bvals[0] = 2.0f * s1 - 2.0f;
	bvals[1] = -4.0f * s1 + 2.0f;
	bvals[2] = 2.0f * s1;
}

/*
====================
cweeCurve_QuadraticBezier::BasisSecondDerivative

  second derivative of quadratic bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_QuadraticBezier<type>::BasisSecondDerivative(const u64 t, u64* bvals) const {
	u64 s1 = (u64)(t - this->times[0]) / (this->times[2] - this->times[0]);
	bvals[0] = 2.0f;
	bvals[1] = -4.0f;
	bvals[2] = 2.0f;
}


/*
===============================================================================

	Cubic Bezier Curve template.
	Should always have exactly four knots.

===============================================================================
*/

template< class type >
class cweeCurve_CubicBezier : public cweeCurve<type> {

public:
	cweeCurve_CubicBezier();

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	void				Basis(const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const u64 t, u64* bvals) const;
};

/*
====================
cweeCurve_CubicBezier::cweeCurve_CubicBezier
====================
*/
template< class type >
INLINE cweeCurve_CubicBezier<type>::cweeCurve_CubicBezier() {
}


/*
====================
cweeCurve_CubicBezier::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_CubicBezier<type>::GetCurrentValue(const u64 time) const {
	u64 bvals[4];
	assert(this->values.NumRef() == 4);
	Basis(time, bvals);
	return (bvals[0] * this->values[0] + bvals[1] * this->values[1] + bvals[2] * this->values[2] + bvals[3] * this->values[3]);
}

/*
====================
cweeCurve_CubicBezier::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_CubicBezier<type>::GetCurrentFirstDerivative(const u64 time) const {
	u64 bvals[4], d;
	assert(this->values.NumRef() == 4);
	BasisFirstDerivative(time, bvals);
	d = (this->times[3] - this->times[0]);
	return (bvals[0] * this->values[0] + bvals[1] * this->values[1] + bvals[2] * this->values[2] + bvals[3] * this->values[3]) / d;
}

/*
====================
cweeCurve_CubicBezier::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_CubicBezier<type>::GetCurrentSecondDerivative(const u64 time) const {
	u64 bvals[4], d;
	assert(this->values.NumRef() == 4);
	BasisSecondDerivative(time, bvals);
	d = (this->times[3] - this->times[0]);
	return (bvals[0] * this->values[0] + bvals[1] * this->values[1] + bvals[2] * this->values[2] + bvals[3] * this->values[3]) / (d * d);
}

/*
====================
cweeCurve_CubicBezier::Basis

  cubic bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_CubicBezier<type>::Basis(const u64 t, u64* bvals) const {
	u64 s1 = (u64)(t - this->times[0]) / (this->times[3] - this->times[0]);
	u64 s2 = s1 * s1;
	u64 s3 = s2 * s1;
	bvals[0] = -s3 + 3.0f * s2 - 3.0f * s1 + 1.0f;
	bvals[1] = 3.0f * s3 - 6.0f * s2 + 3.0f * s1;
	bvals[2] = -3.0f * s3 + 3.0f * s2;
	bvals[3] = s3;
}

/*
====================
cweeCurve_CubicBezier::BasisFirstDerivative

  first derivative of cubic bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_CubicBezier<type>::BasisFirstDerivative(const u64 t, u64* bvals) const {
	u64 s1 = (u64)(t - this->times[0]) / (this->times[3] - this->times[0]);
	u64 s2 = s1 * s1;
	bvals[0] = -3.0f * s2 + 6.0f * s1 - 3.0f;
	bvals[1] = 9.0f * s2 - 12.0f * s1 + 3.0f;
	bvals[2] = -9.0f * s2 + 6.0f * s1;
	bvals[3] = 3.0f * s2;
}

/*
====================
cweeCurve_CubicBezier::BasisSecondDerivative

  second derivative of cubic bezier basis functions
====================
*/
template< class type >
INLINE void cweeCurve_CubicBezier<type>::BasisSecondDerivative(const u64 t, u64* bvals) const {
	u64 s1 = (u64)(t - this->times[0]) / (this->times[3] - this->times[0]);
	bvals[0] = -6.0f * s1 + 6.0f;
	bvals[1] = 18.0f * s1 - 12.0f;
	bvals[2] = -18.0f * s1 + 6.0f;
	bvals[3] = 6.0f * s1;
}


/*
===============================================================================

	Spline base template.

===============================================================================
*/

template< class type >
class cweeCurve_Spline : public cweeCurve<type> {

public:
	//enum				boundary_t { BT_FREE, BT_CLAMPED, BT_CLOSED, BT_LOOP };

	cweeCurve_Spline();

	virtual bool		IsDone(const u64 time) const;

	virtual void		SetBoundaryType(const boundary_t bt) { this->boundaryType = bt; this->changed = true; }
	virtual boundary_t	GetBoundaryType() const { return this->boundaryType; }

	virtual void		SetCloseTime(const u64 t) { this->closeTime = t; this->changed = true; }
	virtual u64		GetCloseTime() { return this->boundaryType == boundary_t::BT_CLOSED ? this->closeTime : 0.0f; }


	type				ValueForIndex(const int index) const;
	u64				TimeForIndex(const int index) const;

protected:

	u64				ClampedTime(const u64 t) const;
};

/*
====================
cweeCurve_Spline::cweeCurve_Spline
====================
*/
template< class type >
INLINE cweeCurve_Spline<type>::cweeCurve_Spline() {
	this->boundaryType = boundary_t::BT_FREE;
	this->closeTime = 0.0f;
}

/*
====================
cweeCurve_Spline::ValueForIndex

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_Spline<type>::ValueForIndex(const int index) const {
	int n = this->values.NumRef() - 1;

	if (index < 0) {
		if (this->boundaryType == boundary_t::BT_CLOSED) {
			return this->values[this->values.NumRef() + index % this->values.NumRef()];
		}
		else {
			return this->values[0] + index * (this->values[1] - this->values[0]);
		}
	}
	else if (index > n) {
		if (this->boundaryType == boundary_t::BT_CLOSED) {
			return this->values[index % this->values.NumRef()];
		}
		else {
			return this->values[n] + (index - n) * (this->values[n] - this->values[n - 1]);
		}
	}
	return this->values[index];
}

/*
====================
cweeCurve_Spline::TimeForIndex

  get the value for the given time
====================
*/
template< class type >
INLINE u64 cweeCurve_Spline<type>::TimeForIndex(const int index) const {
	int n = this->times.NumRef() - 1;

	if (index < 0) {
		if (this->boundaryType == boundary_t::BT_CLOSED) {
			return (index / this->times.NumRef()) * (this->times[n] + this->closeTime) - (this->times[n] + this->closeTime - this->times[this->times.NumRef() + index % this->times.NumRef()]);
		}
		else {
			return this->times[0] + index * (this->times[1] - this->times[0]);
		}
	}
	else if (index > n) {
		if (this->boundaryType == boundary_t::BT_CLOSED) {
			return (index / this->times.NumRef()) * (this->times[n] + this->closeTime) + this->times[index % this->times.NumRef()];
		}
		else {
			return this->times[n] + (index - n) * (this->times[n] - this->times[n - 1]);
		}
	}
	return this->times[index];
}

/*
====================
cweeCurve_Spline::ClampedTime

  return the clamped time based on the boundary type
====================
*/
template< class type >
INLINE u64 cweeCurve_Spline<type>::ClampedTime(const u64 t) const {
	if (this->boundaryType == boundary_t::BT_CLAMPED) {
		if (t < this->times[0]) {
			return this->times[0];
		}
		else if (t >= this->times[this->times.NumRef() - 1]) {
			return this->times[this->times.NumRef() - 1];
		}
	}
	return t;
}

/*
====================
cweeCurve_Spline::IsDone
====================
*/
template< class type >
INLINE bool cweeCurve_Spline<type>::IsDone(const u64 time) const {
	return (this->boundaryType != boundary_t::BT_CLOSED && time >= this->times[this->times.NumRef() - 1]);
}


/*
===============================================================================

	Cubic Interpolating Spline template.
	The curve goes through all the knots.

===============================================================================
*/

template< class type >
class cweeCurve_NaturalCubicSpline : public cweeCurve_Spline<type> {
public:
	cweeCurve_NaturalCubicSpline();

	virtual void		Clear() { cweeCurve_Spline<type>::Clear(); this->values.Clear(); b.Clear(); c.Clear(); d.Clear(); }

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	mutable cweeList<type>b;
	mutable cweeList<type>c;
	mutable cweeList<type>d;

	void				Setup() const;
	void				SetupFree() const;
	void				SetupClamped() const;
	void				SetupClosed() const;
};

/*
====================
cweeCurve_NaturalCubicSpline::cweeCurve_NaturalCubicSpline
====================
*/
template< class type >
INLINE cweeCurve_NaturalCubicSpline<type>::cweeCurve_NaturalCubicSpline() {
}

/*
====================
cweeCurve_NaturalCubicSpline::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NaturalCubicSpline<type>::GetCurrentValue(const u64 time) const {
	u64 clampedTime = this->ClampedTime(time);
	int i = this->IndexForTime(clampedTime);
	u64 s = time - this->TimeForIndex(i);
	Setup();
	return (this->values[i] + s * (b[i] + s * (c[i] + s * d[i])));
}

/*
====================
cweeCurve_NaturalCubicSpline::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NaturalCubicSpline<type>::GetCurrentFirstDerivative(const u64 time) const {
	u64 clampedTime = this->ClampedTime(time);
	int i = this->IndexForTime(clampedTime);
	u64 s = time - this->TimeForIndex(i);
	Setup();
	return (b[i] + s * (2.0f * c[i] + 3.0f * s * d[i]));
}

/*
====================
cweeCurve_NaturalCubicSpline::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NaturalCubicSpline<type>::GetCurrentSecondDerivative(const u64 time) const {
	u64 clampedTime = this->ClampedTime(time);
	int i = this->IndexForTime(clampedTime);
	u64 s = time - this->TimeForIndex(i);
	Setup();
	return (2.0f * c[i] + 6.0f * s * d[i]);
}

/*
====================
cweeCurve_NaturalCubicSpline::Setup
====================
*/
template< class type >
INLINE void cweeCurve_NaturalCubicSpline<type>::Setup() const {
	if (this->changed) {
		switch (this->boundaryType) {
			case cweeCurve_Spline<type>::BT_FREE:			SetupFree(); break;
			case cweeCurve_Spline<type>::BT_CLAMPED:		SetupClamped(); break;
			case cweeCurve_Spline<type>::BT_CLOSED:			SetupClosed(); break;
			case cweeCurve_Spline<type>::BT_LOOP:			SetupFree(); break;
		}
		this->changed = false;
	}
}

/*
====================
cweeCurve_NaturalCubicSpline::SetupFree
====================
*/
template< class type >
INLINE void cweeCurve_NaturalCubicSpline<type>::SetupFree() const {
	int i;
	u64 inv;
	u64* d0, * d1, * beta, * gamma;
	type* alpha, * delta;

	d0 = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	d1 = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	alpha = (type*)_alloca16((this->values.NumRef() - 1) * sizeof(type));
	beta = (u64*)_alloca16(this->values.NumRef() * sizeof(u64));
	gamma = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	delta = (type*)_alloca16(this->values.NumRef() * sizeof(type));

	for (i = 0; i < this->values.NumRef() - 1; i++) {
		d0[i] = this->times[i + 1] - this->times[i];
	}

	for (i = 1; i < this->values.NumRef() - 1; i++) {
		d1[i] = this->times[i + 1] - this->times[i - 1];
	}

	for (i = 1; i < this->values.NumRef() - 1; i++) {
		type sum = 3.0f * (d0[i - 1] * this->values[i + 1] - d1[i] * this->values[i] + d0[i] * this->values[i - 1]);
		inv = 1.0f / (d0[i - 1] * d0[i]);
		alpha[i] = inv * sum;
	}

	beta[0] = 1.0f;
	gamma[0] = 0.0f;
	delta[0] = this->values[0] - this->values[0]; //-V501

	for (i = 1; i < this->values.NumRef() - 1; i++) {
		beta[i] = 2.0f * d1[i] - d0[i - 1] * gamma[i - 1];
		inv = 1.0f / beta[i];
		gamma[i] = inv * d0[i];
		delta[i] = inv * (alpha[i] - d0[i - 1] * delta[i - 1]);
	}
	beta[this->values.NumRef() - 1] = 1.0f;
	delta[this->values.NumRef() - 1] = this->values[0] - this->values[0]; //-V501

	b.AssureSize(this->values.NumRef());
	c.AssureSize(this->values.NumRef());
	d.AssureSize(this->values.NumRef());

	c[this->values.NumRef() - 1] = this->values[0] - this->values[0]; //-V501

	for (i = this->values.NumRef() - 2; i >= 0; i--) {
		c[i] = delta[i] - gamma[i] * c[i + 1];
		inv = 1.0f / d0[i];
		b[i] = inv * (this->values[i + 1] - this->values[i]) - (1.0f / 3.0f) * d0[i] * (c[i + 1] + 2.0f * c[i]);
		d[i] = (1.0f / 3.0f) * inv * (c[i + 1] - c[i]);
	}
}

/*
====================
cweeCurve_NaturalCubicSpline::SetupClamped
====================
*/
template< class type >
INLINE void cweeCurve_NaturalCubicSpline<type>::SetupClamped() const {
	int i;
	u64 inv;
	u64* d0, * d1, * beta, * gamma;
	type* alpha, * delta;

	d0 = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	d1 = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	alpha = (type*)_alloca16((this->values.NumRef() - 1) * sizeof(type));
	beta = (u64*)_alloca16(this->values.NumRef() * sizeof(u64));
	gamma = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	delta = (type*)_alloca16(this->values.NumRef() * sizeof(type));

	for (i = 0; i < this->values.NumRef() - 1; i++) {
		d0[i] = this->times[i + 1] - this->times[i];
	}

	for (i = 1; i < this->values.NumRef() - 1; i++) {
		d1[i] = this->times[i + 1] - this->times[i - 1];
	}

	inv = 1.0f / d0[0];
	alpha[0] = 3.0f * (inv - 1.0f) * (this->values[1] - this->values[0]);
	inv = 1.0f / d0[this->values.NumRef() - 2];
	alpha[this->values.NumRef() - 1] = 3.0f * (1.0f - inv) * (this->values[this->values.NumRef() - 1] - this->values[this->values.NumRef() - 2]);

	for (i = 1; i < this->values.NumRef() - 1; i++) {
		type sum = 3.0f * (d0[i - 1] * this->values[i + 1] - d1[i] * this->values[i] + d0[i] * this->values[i - 1]);
		inv = 1.0f / (d0[i - 1] * d0[i]);
		alpha[i] = inv * sum;
	}

	beta[0] = 2.0f * d0[0];
	gamma[0] = 0.5f;
	inv = 1.0f / beta[0];
	delta[0] = inv * alpha[0];

	for (i = 1; i < this->values.NumRef() - 1; i++) {
		beta[i] = 2.0f * d1[i] - d0[i - 1] * gamma[i - 1];
		inv = 1.0f / beta[i];
		gamma[i] = inv * d0[i];
		delta[i] = inv * (alpha[i] - d0[i - 1] * delta[i - 1]);
	}

	beta[this->values.NumRef() - 1] = d0[this->values.NumRef() - 2] * (2.0f - gamma[this->values.NumRef() - 2]);
	inv = 1.0f / beta[this->values.NumRef() - 1];
	delta[this->values.NumRef() - 1] = inv * (alpha[this->values.NumRef() - 1] - d0[this->values.NumRef() - 2] * delta[this->values.NumRef() - 2]);

	b.AssureSize(this->values.NumRef());
	c.AssureSize(this->values.NumRef());
	d.AssureSize(this->values.NumRef());

	c[this->values.NumRef() - 1] = delta[this->values.NumRef() - 1];

	for (i = this->values.NumRef() - 2; i >= 0; i--) {
		c[i] = delta[i] - gamma[i] * c[i + 1];
		inv = 1.0f / d0[i];
		b[i] = inv * (this->values[i + 1] - this->values[i]) - (1.0f / 3.0f) * d0[i] * (c[i + 1] + 2.0f * c[i]);
		d[i] = (1.0f / 3.0f) * inv * (c[i + 1] - c[i]);
	}
}

/*
====================
cweeCurve_NaturalCubicSpline::SetupClosed
====================
*/
template< class type >
INLINE void cweeCurve_NaturalCubicSpline<type>::SetupClosed() const {
	int i, j;
	u64 c0, c1;
	u64* d0;
	cweeMatX mat;
	cweeVecX x;

	d0 = (u64*)_alloca16((this->values.NumRef() - 1) * sizeof(u64));
	x.SetData(this->values.NumRef(), VECX_ALLOCA(this->values.NumRef()));
	mat.SetData(this->values.NumRef(), this->values.NumRef(), MATX_ALLOCA(this->values.NumRef() * this->values.NumRef()));

	b.AssureSize(this->values.NumRef());
	c.AssureSize(this->values.NumRef());
	d.AssureSize(this->values.NumRef());

	for (i = 0; i < this->values.NumRef() - 1; i++) {
		d0[i] = this->times[i + 1] - this->times[i];
	}

	// matrix of system
	mat[0][0] = 1.0f;
	mat[0][this->values.NumRef() - 1] = -1.0f;
	for (i = 1; i <= this->values.NumRef() - 2; i++) {
		mat[i][i - 1] = d0[i - 1];
		mat[i][i] = 2.0f * (d0[i - 1] + d0[i]);
		mat[i][i + 1] = d0[i];
	}
	mat[this->values.NumRef() - 1][this->values.NumRef() - 2] = d0[this->values.NumRef() - 2];
	mat[this->values.NumRef() - 1][0] = 2.0f * (d0[this->values.NumRef() - 2] + d0[0]);
	mat[this->values.NumRef() - 1][1] = d0[0];

	// right-hand side
	c[0].Zero();
	for (i = 1; i <= this->values.NumRef() - 2; i++) {
		c0 = 1.0f / d0[i];
		c1 = 1.0f / d0[i - 1];
		c[i] = 3.0f * (c0 * (this->values[i + 1] - this->values[i]) - c1 * (this->values[i] - this->values[i - 1]));
	}
	c0 = 1.0f / d0[0];
	c1 = 1.0f / d0[this->values.NumRef() - 2];
	c[this->values.NumRef() - 1] = 3.0f * (c0 * (this->values[1] - this->values[0]) - c1 * (this->values[0] - this->values[this->values.NumRef() - 2]));

	// solve system for each dimension
	mat.LU_Factor(NULL);
	for (i = 0; i < this->values[0].GetDimension(); i++) {
		for (j = 0; j < this->values.NumRef(); j++) {
			x[j] = c[j][i];
		}
		mat.LU_Solve(x, x, NULL);
		for (j = 0; j < this->values.NumRef(); j++) {
			c[j][i] = x[j];
		}
	}

	for (i = 0; i < this->values.NumRef() - 1; i++) {
		c0 = 1.0f / d0[i];
		b[i] = c0 * (this->values[i + 1] - this->values[i]) - (1.0f / 3.0f) * (c[i + 1] + 2.0f * c[i]) * d0[i];
		d[i] = (1.0f / 3.0f) * c0 * (c[i + 1] - c[i]);
	}
}


/*
===============================================================================

	Uniform Cubic Interpolating Spline template.
	The curve goes through all the knots.

===============================================================================
*/

template< class type >
class cweeCurve_CatmullRomSpline : public cweeCurve_Spline<type> {

public:
	cweeCurve_CatmullRomSpline();

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	void				Basis(const int index, const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const int index, const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const int index, const u64 t, u64* bvals) const;
};

template< class type >
INLINE cweeCurve_CatmullRomSpline<type>::cweeCurve_CatmullRomSpline() {
}

template< class type >
INLINE type cweeCurve_CatmullRomSpline<type>::GetCurrentValue(const u64 time) const {
	int i, j, k;
	u64 bvals[4], clampedTime;
	type v;
	v = 0; //-V501

	if ((this->times).NumRef() < 1) {
		return v;
	}
	if ((this->times).NumRef() == 1) {
		return (this->values)[0];
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

	//if (this->times.NumRef() == 1) {
	//	return this->values[0];
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
}

template< class type >
INLINE type cweeCurve_CatmullRomSpline<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i, j, k;
	u64 bvals[4], d, clampedTime;
	type v;
	v = 0; //-V501

	if ((this->times).NumRef() < 1) {
		return v;
	}
	if (this->times.NumRef() == 1) {
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
}

template< class type >
INLINE type cweeCurve_CatmullRomSpline<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i, j, k;
	u64 bvals[4], d, clampedTime;
	type v;
	v = 0; //-V501

	if ((this->times).NumRef() < 1) {
		return v;
	}
	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
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
}

template< class type >
INLINE void cweeCurve_CatmullRomSpline<type>::Basis(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = ((-s + 2.0f) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
	bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;	// 1.5f * s * s * s - 2.5f * s * s + 1.0f
	bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
	bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
}

template< class type >
INLINE void cweeCurve_CatmullRomSpline<type>::BasisFirstDerivative(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
	bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
	bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
	bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
}

template< class type >
INLINE void cweeCurve_CatmullRomSpline<type>::BasisSecondDerivative(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = -3.0f * s + 2.0f;
	bvals[1] = 9.0f * s - 5.0f;
	bvals[2] = -9.0f * s + 4.0f;
	bvals[3] = 3.0f * s - 1.0f;
}






/*
===============================================================================

	Cubic Interpolating Spline template.
	The curve goes through all the knots.
	The curve becomes the Catmull-Rom spline if the tension,
	continuity and bias are all set to zero.

===============================================================================
*/

template< class type >
class cweeCurve_KochanekBartelsSpline : public cweeCurve_Spline<type> {

public:
	cweeCurve_KochanekBartelsSpline();

	virtual int			AddValue(const u64 time, const type& value);
	virtual int			AddValue(const u64 time, const type& value, const u64 tension, const u64 continuity, const u64 bias);
	virtual void		RemoveIndex(const int index) { this->values.RemoveIndex(index); this->times.RemoveIndex(index); tension.RemoveIndex(index); continuity.RemoveIndex(index); bias.RemoveIndex(index); }
	virtual void		Clear() { this->values.Clear(); this->times.Clear(); tension.Clear(); continuity.Clear(); bias.Clear(); this->currentIndex = -1; }

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	cweeList<u64>		tension;
	cweeList<u64>		continuity;
	cweeList<u64>		bias;

	void				TangentsForIndex(const int index, type& t0, type& t1) const;

	void				Basis(const int index, const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const int index, const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const int index, const u64 t, u64* bvals) const;
};

/*
====================
cweeCurve_KochanekBartelsSpline::cweeCurve_KochanekBartelsSpline
====================
*/
template< class type >
INLINE cweeCurve_KochanekBartelsSpline<type>::cweeCurve_KochanekBartelsSpline() {
}

/*
====================
cweeCurve_KochanekBartelsSpline::AddValue

  add a timed/value pair to the spline
  returns the index to the inserted pair
====================
*/
template< class type >
INLINE int cweeCurve_KochanekBartelsSpline<type>::AddValue(const u64 time, const type& value) {
	int i;

	i = this->IndexForTime(time);
	this->times.Insert(time, i);
	this->values.Insert(value, i);
	tension.Insert(0.0f, i);
	continuity.Insert(0.0f, i);
	bias.Insert(0.0f, i);
	return i;
}

/*
====================
cweeCurve_KochanekBartelsSpline::AddValue

  add a timed/value pair to the spline
  returns the index to the inserted pair
====================
*/
template< class type >
INLINE int cweeCurve_KochanekBartelsSpline<type>::AddValue(const u64 time, const type& value, const u64 tension, const u64 continuity, const u64 bias) {
	int i;

	i = this->IndexForTime(time);
	this->times.Insert(time, i);
	this->values.Insert(value, i);
	this->tension.Insert(tension, i);
	this->continuity.Insert(continuity, i);
	this->bias.Insert(bias, i);
	return i;
}

/*
====================
cweeCurve_KochanekBartelsSpline::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_KochanekBartelsSpline<type>::GetCurrentValue(const u64 time) const {
	int i;
	u64 bvals[4], clampedTime;
	type v, t0, t1;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	TangentsForIndex(i - 1, t0, t1);
	Basis(i - 1, clampedTime, bvals);
	v = bvals[0] * this->ValueForIndex(i - 1);
	v += bvals[1] * this->ValueForIndex(i);
	v += bvals[2] * t0;
	v += bvals[3] * t1;
	return v;
}

/*
====================
cweeCurve_KochanekBartelsSpline::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_KochanekBartelsSpline<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i;
	u64 bvals[4], d, clampedTime;
	type v, t0, t1;

	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	TangentsForIndex(i - 1, t0, t1);
	BasisFirstDerivative(i - 1, clampedTime, bvals);
	v = bvals[0] * this->ValueForIndex(i - 1);
	v += bvals[1] * this->ValueForIndex(i);
	v += bvals[2] * t0;
	v += bvals[3] * t1;
	d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
	return v / d;
}

/*
====================
cweeCurve_KochanekBartelsSpline::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_KochanekBartelsSpline<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i;
	u64 bvals[4], d, clampedTime;
	type v, t0, t1;

	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	TangentsForIndex(i - 1, t0, t1);
	BasisSecondDerivative(i - 1, clampedTime, bvals);
	v = bvals[0] * this->ValueForIndex(i - 1);
	v += bvals[1] * this->ValueForIndex(i);
	v += bvals[2] * t0;
	v += bvals[3] * t1;
	d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
	return v / (d * d);
}

/*
====================
cweeCurve_KochanekBartelsSpline::TangentsForIndex
====================
*/
template< class type >
INLINE void cweeCurve_KochanekBartelsSpline<type>::TangentsForIndex(const int index, type& t0, type& t1) const {
	u64 dt, omt, omc, opc, omb, opb, adj, s0, s1;
	type delta;

	delta = this->ValueForIndex(index + 1) - this->ValueForIndex(index);
	dt = this->TimeForIndex(index + 1) - this->TimeForIndex(index);

	omt = 1.0f - tension[index];
	omc = 1.0f - continuity[index];
	opc = 1.0f + continuity[index];
	omb = 1.0f - bias[index];
	opb = 1.0f + bias[index];
	adj = 2.0f * dt / (this->TimeForIndex(index + 1) - this->TimeForIndex(index - 1));
	s0 = 0.5f * adj * omt * opc * opb;
	s1 = 0.5f * adj * omt * omc * omb;

	// outgoing tangent at first point
	t0 = s1 * delta + s0 * (this->ValueForIndex(index) - this->ValueForIndex(index - 1));

	omt = 1.0f - tension[index + 1];
	omc = 1.0f - continuity[index + 1];
	opc = 1.0f + continuity[index + 1];
	omb = 1.0f - bias[index + 1];
	opb = 1.0f + bias[index + 1];
	adj = 2.0f * dt / (this->TimeForIndex(index + 2) - this->TimeForIndex(index));
	s0 = 0.5f * adj * omt * omc * opb;
	s1 = 0.5f * adj * omt * opc * omb;

	// incoming tangent at second point
	t1 = s1 * (this->ValueForIndex(index + 2) - this->ValueForIndex(index + 1)) + s0 * delta;
}

/*
====================
cweeCurve_KochanekBartelsSpline::Basis

  spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_KochanekBartelsSpline<type>::Basis(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = ((2.0f * s - 3.0f) * s) * s + 1.0f;				// 2.0f * s * s * s - 3.0f * s * s + 1.0f
	bvals[1] = ((-2.0f * s + 3.0f) * s) * s;					// -2.0f * s * s * s + 3.0f * s * s
	bvals[2] = ((s - 2.0f) * s) * s + s;						// s * s * s - 2.0f * s * s + s
	bvals[3] = ((s - 1.0f) * s) * s;							// s * s * s - s * s
}

/*
====================
cweeCurve_KochanekBartelsSpline::BasisFirstDerivative

  first derivative of spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_KochanekBartelsSpline<type>::BasisFirstDerivative(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = (6.0f * s - 6.0f) * s;								// 6.0f * s * s - 6.0f * s
	bvals[1] = (-6.0f * s + 6.0f) * s;							// -6.0f * s * s + 6.0f * s
	bvals[2] = (3.0f * s - 4.0f) * s + 1.0f;						// 3.0f * s * s - 4.0f * s + 1.0f
	bvals[3] = (3.0f * s - 2.0f) * s;								// 3.0f * s * s - 2.0f * s
}

/*
====================
cweeCurve_KochanekBartelsSpline::BasisSecondDerivative

  second derivative of spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_KochanekBartelsSpline<type>::BasisSecondDerivative(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = 12.0f * s - 6.0f;
	bvals[1] = -12.0f * s + 6.0f;
	bvals[2] = 6.0f * s - 4.0f;
	bvals[3] = 6.0f * s - 2.0f;
}


/*
===============================================================================

	B-Spline base template. Uses recursive definition and is slow.
	Use cweeCurve_UniformCubicBSpline or cweeCurve_NonUniformBSpline instead.

===============================================================================
*/

template< class type >
class cweeCurve_BSpline : public cweeCurve_Spline<type> {

public:
	cweeCurve_BSpline();

	virtual int			GetOrder() const { return order; }
	virtual void		SetOrder(const int i) { assert(i > 0 && i < 10); order = i; }

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	int					order;

	u64				Basis(const int index, const int order, const u64 t) const;
	u64				BasisFirstDerivative(const int index, const int order, const u64 t) const;
	u64				BasisSecondDerivative(const int index, const int order, const u64 t) const;
};

/*
====================
cweeCurve_BSpline::cweeCurve_NaturalCubicSpline
====================
*/
template< class type >
INLINE cweeCurve_BSpline<type>::cweeCurve_BSpline() {
	order = 4;	// default to cubic
}

/*
====================
cweeCurve_BSpline::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_BSpline<type>::GetCurrentValue(const u64 time) const {
	int i, j, k;
	u64 clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < order; j++) {
		k = i + j - (order >> 1);
		v += Basis(k - 2, order, clampedTime) * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_BSpline::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_BSpline<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i, j, k;
	u64 clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < order; j++) {
		k = i + j - (order >> 1);
		v += BasisFirstDerivative(k - 2, order, clampedTime) * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_BSpline::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_BSpline<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i, j, k;
	u64 clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < order; j++) {
		k = i + j - (order >> 1);
		v += BasisSecondDerivative(k - 2, order, clampedTime) * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_BSpline::Basis

  spline basis function
====================
*/
template< class type >
INLINE u64 cweeCurve_BSpline<type>::Basis(const int index, const int order, const u64 t) const {
	if (order <= 1) {
		if (this->TimeForIndex(index) < t && t <= this->TimeForIndex(index + 1)) {
			return 1.0f;
		}
		else {
			return 0.0f;
		}
	}
	else {
		u64 sum = 0.0f;
		u64 d1 = this->TimeForIndex(index + order - 1) - this->TimeForIndex(index);
		if (d1 != 0.0f) {
			sum += (u64)(t - this->TimeForIndex(index)) * Basis(index, order - 1, t) / d1;
		}

		u64 d2 = this->TimeForIndex(index + order) - this->TimeForIndex(index + 1);
		if (d2 != 0.0f) {
			sum += (u64)(this->TimeForIndex(index + order) - t) * Basis(index + 1, order - 1, t) / d2;
		}
		return sum;
	}
}

/*
====================
cweeCurve_BSpline::BasisFirstDerivative

  first derivative of spline basis function
====================
*/
template< class type >
INLINE u64 cweeCurve_BSpline<type>::BasisFirstDerivative(const int index, const int order, const u64 t) const {
	return (Basis(index, order - 1, t) - Basis(index + 1, order - 1, t)) *
		(u64)(order - 1) / (this->TimeForIndex(index + (order - 1) - 2) - this->TimeForIndex(index - 2));
}

/*
====================
cweeCurve_BSpline::BasisSecondDerivative

  second derivative of spline basis function
====================
*/
template< class type >
INLINE u64 cweeCurve_BSpline<type>::BasisSecondDerivative(const int index, const int order, const u64 t) const {
	return (BasisFirstDerivative(index, order - 1, t) - BasisFirstDerivative(index + 1, order - 1, t)) *
		(u64)(order - 1) / (this->TimeForIndex(index + (order - 1) - 2) - this->TimeForIndex(index - 2));
}


/*
===============================================================================

	Uniform Non-Rational Cubic B-Spline template.

===============================================================================
*/

template< class type >
class cweeCurve_UniformCubicBSpline : public cweeCurve_BSpline<type> {

public:
	cweeCurve_UniformCubicBSpline();

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	void				Basis(const int index, const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const int index, const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const int index, const u64 t, u64* bvals) const;
};

/*
====================
cweeCurve_UniformCubicBSpline::cweeCurve_UniformCubicBSpline
====================
*/
template< class type >
INLINE cweeCurve_UniformCubicBSpline<type>::cweeCurve_UniformCubicBSpline() {
	this->order = 4;	// always cubic
}

/*
====================
cweeCurve_UniformCubicBSpline::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_UniformCubicBSpline<type>::GetCurrentValue(const u64 time) const {
	int i, j, k;
	u64 bvals[4], clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	Basis(i - 1, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < 4; j++) {
		k = i + j - 2;
		v += bvals[j] * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_UniformCubicBSpline::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_UniformCubicBSpline<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i, j, k;
	u64 bvals[4], d, clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	BasisFirstDerivative(i - 1, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < 4; j++) {
		k = i + j - 2;
		v += bvals[j] * this->ValueForIndex(k);
	}
	d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
	return v / d;
}

/*
====================
cweeCurve_UniformCubicBSpline::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_UniformCubicBSpline<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i, j, k;
	u64 bvals[4], d, clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	BasisSecondDerivative(i - 1, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < 4; j++) {
		k = i + j - 2;
		v += bvals[j] * this->ValueForIndex(k);
	}
	d = (this->TimeForIndex(i) - this->TimeForIndex(i - 1));
	return v / (d * d);
}

/*
====================
cweeCurve_UniformCubicBSpline::Basis

  spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_UniformCubicBSpline<type>::Basis(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = (((-s + 3.0f) * s - 3.0f) * s + 1.0f) * (1.0f / 6.0f);
	bvals[1] = (((3.0f * s - 6.0f) * s) * s + 4.0f) * (1.0f / 6.0f);
	bvals[2] = (((-3.0f * s + 3.0f) * s + 3.0f) * s + 1.0f) * (1.0f / 6.0f);
	bvals[3] = (s * s * s) * (1.0f / 6.0f);
}

/*
====================
cweeCurve_UniformCubicBSpline::BasisFirstDerivative

  first derivative of spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_UniformCubicBSpline<type>::BasisFirstDerivative(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = -0.5f * s * s + s - 0.5f;
	bvals[1] = 1.5f * s * s - 2.0f * s;
	bvals[2] = -1.5f * s * s + s + 0.5f;
	bvals[3] = 0.5f * s * s;
}

/*
====================
cweeCurve_UniformCubicBSpline::BasisSecondDerivative

  second derivative of spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_UniformCubicBSpline<type>::BasisSecondDerivative(const int index, const u64 t, u64* bvals) const {
	u64 s = (u64)(t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index));
	bvals[0] = -s + 1.0f;
	bvals[1] = 3.0f * s - 2.0f;
	bvals[2] = -3.0f * s + 1.0f;
	bvals[3] = s;
}


/*
===============================================================================

	Non-Uniform Non-Rational B-Spline (NUBS) template.

===============================================================================
*/

template< class type >
class cweeCurve_NonUniformBSpline : public cweeCurve_BSpline<type> {

public:
	cweeCurve_NonUniformBSpline();

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	void				Basis(const int index, const int order, const u64 t, u64* bvals) const;
	void				BasisFirstDerivative(const int index, const int order, const u64 t, u64* bvals) const;
	void				BasisSecondDerivative(const int index, const int order, const u64 t, u64* bvals) const;
};

/*
====================
cweeCurve_NonUniformBSpline::cweeCurve_NonUniformBSpline
====================
*/
template< class type >
INLINE cweeCurve_NonUniformBSpline<type>::cweeCurve_NonUniformBSpline() {
}

/*
====================
cweeCurve_NonUniformBSpline::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NonUniformBSpline<type>::GetCurrentValue(const u64 time) const {
	int i, j, k;
	u64 clampedTime;
	type v;
	u64* bvals = (u64*)_alloca16(this->order * sizeof(u64));

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	Basis(i - 1, this->order, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < this->order; j++) {
		k = i + j - (this->order >> 1);
		v += bvals[j] * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_NonUniformBSpline::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NonUniformBSpline<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i, j, k;
	u64 clampedTime;
	type v;
	u64* bvals = (u64*)_alloca16(this->order * sizeof(u64));

	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	BasisFirstDerivative(i - 1, this->order, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < this->order; j++) {
		k = i + j - (this->order >> 1);
		v += bvals[j] * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_NonUniformBSpline::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NonUniformBSpline<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i, j, k;
	u64 clampedTime;
	type v;
	u64* bvals = (u64*)_alloca16(this->order * sizeof(u64));

	if (this->times.NumRef() == 1) {
		return (this->values[0] - this->values[0]); //-V501
	}

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	BasisSecondDerivative(i - 1, this->order, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	for (j = 0; j < this->order; j++) {
		k = i + j - (this->order >> 1);
		v += bvals[j] * this->ValueForIndex(k);
	}
	return v;
}

/*
====================
cweeCurve_NonUniformBSpline::Basis

  spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_NonUniformBSpline<type>::Basis(const int index, const int order, const u64 t, u64* bvals) const {
	int r, s, i;
	u64 omega;

	bvals[order - 1] = 1.0f;
	for (r = 2; r <= order; r++) {
		i = index - r + 1;
		bvals[order - r] = 0.0f;
		for (s = order - r + 1; s < order; s++) {
			i++;
			omega = (u64)(t - this->TimeForIndex(i)) / (this->TimeForIndex(i + r - 1) - this->TimeForIndex(i));
			bvals[s - 1] += (1.0f - omega) * bvals[s];
			bvals[s] *= omega;
		}
	}
}

/*
====================
cweeCurve_NonUniformBSpline::BasisFirstDerivative

  first derivative of spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_NonUniformBSpline<type>::BasisFirstDerivative(const int index, const int order, const u64 t, u64* bvals) const {
	int i;

	Basis(index, order - 1, t, bvals + 1);
	bvals[0] = 0.0f;
	for (i = 0; i < order - 1; i++) {
		bvals[i] -= bvals[i + 1];
		bvals[i] *= (u64)(order - 1) / (this->TimeForIndex(index + i + (order - 1) - 2) - this->TimeForIndex(index + i - 2));
	}
	bvals[i] *= (u64)(order - 1) / (this->TimeForIndex(index + i + (order - 1) - 2) - this->TimeForIndex(index + i - 2));
}

/*
====================
cweeCurve_NonUniformBSpline::BasisSecondDerivative

  second derivative of spline basis functions
====================
*/
template< class type >
INLINE void cweeCurve_NonUniformBSpline<type>::BasisSecondDerivative(const int index, const int order, const u64 t, u64* bvals) const {
	int i;

	BasisFirstDerivative(index, order - 1, t, bvals + 1);
	bvals[0] = 0.0f;
	for (i = 0; i < order - 1; i++) {
		bvals[i] -= bvals[i + 1];
		bvals[i] *= (u64)(order - 1) / (this->TimeForIndex(index + i + (order - 1) - 2) - this->TimeForIndex(index + i - 2));
	}
	bvals[i] *= (u64)(order - 1) / (this->TimeForIndex(index + i + (order - 1) - 2) - this->TimeForIndex(index + i - 2));
}


/*
===============================================================================

	Non-Uniform Rational B-Spline (NURBS) template.

===============================================================================
*/

template< class type >
class cweeCurve_NURBS : public cweeCurve_NonUniformBSpline<type> {

public:
	cweeCurve_NURBS();

	virtual int			AddValue(const u64 time, const type& value);
	virtual int			AddValue(const u64 time, const type& value, const u64 weight);
	virtual void		RemoveIndex(const int index) { this->values.RemoveIndex(index); this->times.RemoveIndex(index); weights.RemoveIndex(index); }
	virtual void		Clear() { this->values.Clear(); this->times.Clear(); weights.Clear(); this->currentIndex = -1; }

	virtual type		GetCurrentValue(const u64 time) const;
	virtual type		GetCurrentFirstDerivative(const u64 time) const;
	virtual type		GetCurrentSecondDerivative(const u64 time) const;

protected:
	cweeList<u64>		weights;

	u64				WeightForIndex(const int index) const;
};

/*
====================
cweeCurve_NURBS::cweeCurve_NURBS
====================
*/
template< class type >
INLINE cweeCurve_NURBS<type>::cweeCurve_NURBS() {
}

/*
====================
cweeCurve_NURBS::AddValue

  add a timed/value pair to the spline
  returns the index to the inserted pair
====================
*/
template< class type >
INLINE int cweeCurve_NURBS<type>::AddValue(const u64 time, const type& value) {
	int i;

	i = this->IndexForTime(time);
	this->times.Insert(time, i);
	this->values.Insert(value, i);
	weights.Insert(1.0f, i);
	return i;
}

/*
====================
cweeCurve_NURBS::AddValue

  add a timed/value pair to the spline
  returns the index to the inserted pair
====================
*/
template< class type >
INLINE int cweeCurve_NURBS<type>::AddValue(const u64 time, const type& value, const u64 weight) {
	int i;

	i = this->IndexForTime(time);
	this->times.Insert(time, i);
	this->values.Insert(value, i);
	weights.Insert(weight, i);
	return i;
}

/*
====================
cweeCurve_NURBS::GetCurrentValue

  get the value for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NURBS<type>::GetCurrentValue(const u64 time) const {
	int i, j, k;
	u64 w, b, * bvals, clampedTime;
	type v;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	bvals = (u64*)_alloca16(this->order * sizeof(u64));

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	this->Basis(i - 1, this->order, clampedTime, bvals);
	v = this->values[0] - this->values[0]; //-V501
	w = 0.0f;
	for (j = 0; j < this->order; j++) {
		k = i + j - (this->order >> 1);
		b = bvals[j] * WeightForIndex(k);
		w += b;
		v += b * this->ValueForIndex(k);
	}
	return v / w;
}

/*
====================
cweeCurve_NURBS::GetCurrentFirstDerivative

  get the first derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NURBS<type>::GetCurrentFirstDerivative(const u64 time) const {
	int i, j, k;
	u64 w, wb, wd1, b, d1, * bvals, * d1vals, clampedTime;
	type v, vb, vd1;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	bvals = (u64*)_alloca16(this->order * sizeof(u64));
	d1vals = (u64*)_alloca16(this->order * sizeof(u64));

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	this->Basis(i - 1, this->order, clampedTime, bvals);
	this->BasisFirstDerivative(i - 1, this->order, clampedTime, d1vals);
	vb = vd1 = this->values[0] - this->values[0]; //-V501
	wb = wd1 = 0.0f;
	for (j = 0; j < this->order; j++) {
		k = i + j - (this->order >> 1);
		w = WeightForIndex(k);
		b = bvals[j] * w;
		d1 = d1vals[j] * w;
		wb += b;
		wd1 += d1;
		v = this->ValueForIndex(k);
		vb += b * v;
		vd1 += d1 * v;
	}
	return (wb * vd1 - vb * wd1) / (wb * wb);
}

/*
====================
cweeCurve_NURBS::GetCurrentSecondDerivative

  get the second derivative for the given time
====================
*/
template< class type >
INLINE type cweeCurve_NURBS<type>::GetCurrentSecondDerivative(const u64 time) const {
	int i, j, k;
	u64 w, wb, wd1, wd2, b, d1, d2, * bvals, * d1vals, * d2vals, clampedTime;
	type v, vb, vd1, vd2;

	if (this->times.NumRef() == 1) {
		return this->values[0];
	}

	bvals = (u64*)_alloca16(this->order * sizeof(u64));
	d1vals = (u64*)_alloca16(this->order * sizeof(u64));
	d2vals = (u64*)_alloca16(this->order * sizeof(u64));

	clampedTime = this->ClampedTime(time);
	i = this->IndexForTime(clampedTime);
	this->Basis(i - 1, this->order, clampedTime, bvals);
	this->BasisFirstDerivative(i - 1, this->order, clampedTime, d1vals);
	this->BasisSecondDerivative(i - 1, this->order, clampedTime, d2vals);
	vb = vd1 = vd2 = this->values[0] - this->values[0]; //-V501
	wb = wd1 = wd2 = 0.0f;
	for (j = 0; j < this->order; j++) {
		k = i + j - (this->order >> 1);
		w = WeightForIndex(k);
		b = bvals[j] * w;
		d1 = d1vals[j] * w;
		d2 = d2vals[j] * w;
		wb += b;
		wd1 += d1;
		wd2 += d2;
		v = this->ValueForIndex(k);
		vb += b * v;
		vd1 += d1 * v;
		vd2 += d2 * v;
	}
	return ((wb * wb) * (wb * vd2 - vb * wd2) - (wb * vd1 - vb * wd1) * 2.0f * wb * wd1) / (wb * wb * wb * wb);
}

/*
====================
cweeCurve_NURBS::WeightForIndex

  get the weight for the given index
====================
*/
template< class type >
INLINE u64 cweeCurve_NURBS<type>::WeightForIndex(const int index) const {
	int n = weights.NumRef() - 1;

	if (index < 0) {
		if (this->boundaryType == cweeCurve_Spline<type>::BT_CLOSED) {
			return weights[weights.NumRef() + index % weights.NumRef()];
		}
		else {
			return weights[0] + index * (weights[1] - weights[0]);
		}
	}
	else if (index > n) {
		if (this->boundaryType == cweeCurve_Spline<type>::BT_CLOSED) {
			return weights[index % weights.NumRef()];
		}
		else {
			return weights[n] + (index - n) * (weights[n] - weights[n - 1]);
		}
	}
	return weights[index];
}

#endif /* !__MATH_CURVE_H__ */
