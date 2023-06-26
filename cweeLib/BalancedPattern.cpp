
#ifndef __BPATTERN_CPP__
#define __BPATTERN_CPP__

#pragma region "INCLUDES"
#pragma hdrstop
#include "precompiled.h"
#pragma endregion

template< class type >
INLINE cweeBalancedPattern<type>::cweeBalancedPattern() {
	Lock();
	container.Reserve(granularity);
	Unlock();

	IncrementLevel();
};

template< class type >
INLINE cweeBalancedPattern<type>::cweeBalancedPattern(const cweeBalancedPattern& source) {
	Lock();
	container.Reserve(granularity);
	Unlock();

	Copy(source);
	IncrementLevel();
};

template< class type >
INLINE cweeBalancedPattern<type>::cweeBalancedPattern(const cweeThreadedList<std::pair<u64, type>>& data) {
	Lock();
	container.Reserve(granularity);
	Unlock();

	AcceptNewData(data);
	IncrementLevel();
};

template< class type >
INLINE cweeBalancedPattern<type>::~cweeBalancedPattern() {
	Clear();
};

template< class type >
INLINE cweeStr	cweeBalancedPattern<type>::ToString() const {
	cweeStr out;

	for (auto& x : GetKnotSeries()) {
		out.AddToDelimiter(cweeStr::printf("<%s, %f>", cweeStr(x.first).c_str(), x.second), ", ");
	}

	out.Insert("[", 0);
	out.Append("]");

	return out;
};

template< class type >
INLINE void		cweeBalancedPattern<type>::Deserialize(cweeStr& inbound) {
	Clear();
#if 1

	cweeParser obj(inbound, ":PATTERN_in_DELIM:", true);
	inbound.Clear();
	if (obj.getNumVars() >= 19) {
		this->SetName(obj[0]);
		this->SetSpecifier((int)obj[1]);
		this->SetMAD(vec3((float)obj[2], (float)obj[3], (float)obj[4]));
		this->SetChanged((bool)(int)obj[6]);
		this->SetInterpolationType(static_cast<interpolation_t>((int)obj[7])); // (interpolation_t)(int)obj[7]
		this->SetBoundaryType((boundary_t)(int)obj[8]);
		{
			cweeStr& knots = obj[10];
			{
				const cweeStr delim2(":Knots_DELIM:");
				knots.Replace("|", delim2);

				if (!knots.IsEmpty()) {
					cweeParser obj(knots, delim2, true);
					knots.Clear();
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

				// deserialize the knots...
			}
			knots.Clear();
		}
		this->Lock(); {
			if (!obj[11].IsEmpty()) {
				if (learned) delete learned;
				learned = new cweeML_learned_parameters();
				if (learned) learned->Deserialize(obj[11]);
				obj[11].Clear();
			}
			machineLearn_MinMaxRound = vec3((float)obj[13], (float)obj[14], (float)obj[12]);
		} this->Unlock();
		this->SetCharacteristic((value_t)(int)obj[15]);
		this->SetMeasurement((measurement_t)(int)obj[16]);
		this->Lock(); {
			machineLearn_features.Clear(); if (!obj[17].IsEmpty()) {
				cweeParser temp(obj[17], ",", true); machineLearn_features.SetGranularity(temp.getNumVars() + 16); vec4 tempVec; for (auto& x : temp) {
					if (x.IsEmpty()) continue;
					tempVec.Deserialize(x);
					machineLearn_features.Append(tempVec);
				}
			}
			if (!obj[18].IsEmpty()) {
				cweeParser temp(obj[18], ",", true); machineLearn_MAD[0] = (float)temp[0]; machineLearn_MAD[1] = (float)temp[1];
			}
			exclusion_list.Clear(); if (!obj[19].IsEmpty()) {
				cweeParser temp(obj[19], ",", true); exclusion_list.SetGranularity(temp.getNumVars() + 16); for (auto& x : temp) {
					if (x.IsEmpty()) continue;
					exclusion_list.Append((bool)(int)x);
				}
			}
		} this->Unlock();
	}
	else if (obj.getNumVars() >= 12) {
		this->SetSpecifier((int)obj[0]);
		this->SetName(obj[1]);
		{
			vec3 t; t.FromString(obj[2]); this->SetMAD(t);
		}
		this->SetInterpolationType(static_cast<interpolation_t>((int)obj[3])); // (interpolation_t)(int)obj[7]
		this->SetBoundaryType((boundary_t)(int)obj[4]);
		{
			cweeStr& knots = obj[5];
			{
				const cweeStr delim2(":Knots_DELIM:");
				knots.Replace("|", delim2);

				if (!knots.IsEmpty()) {
					cweeParser obj(knots, delim2, true);
					knots.Clear();
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

				// deserialize the knots...
			}
			knots.Clear();
		}
		this->Lock(); {
			if (!obj[6].IsEmpty()) {
				if (learned) delete learned;
				learned = new cweeML_learned_parameters();
				if (learned) learned->Deserialize(obj[6]);
				obj[6].Clear();
			}
			{
				machineLearn_MinMaxRound.FromString(obj[7]);
			}
		} this->Unlock();
		this->SetCharacteristic((value_t)(int)obj[8]);
		this->SetMeasurement((measurement_t)(int)obj[9]);
		this->Lock(); {
			machineLearn_features.Clear(); if (!obj[10].IsEmpty()) {
				cweeParser temp(obj[10], ",", true); machineLearn_features.SetGranularity(temp.getNumVars() + 16); vec4 tempVec; for (auto& x : temp) {
					if (x.IsEmpty()) continue;
					tempVec.Deserialize(x);
					machineLearn_features.Append(tempVec);
				}
			}

			if (!obj[11].IsEmpty()) {
				cweeParser temp(obj[11], ",", true); machineLearn_MAD[0] = (float)temp[0]; machineLearn_MAD[1] = (float)temp[1];
			}

			exclusion_list.Clear(); if (!obj[12].IsEmpty()) {
				cweeParser temp(obj[12], ",", true); exclusion_list.SetGranularity(temp.getNumVars() + 16); for (auto& x : temp) {
					if (x.IsEmpty()) continue;
					exclusion_list.Append((bool)(int)x);
				}
			}
		} this->Unlock();
	}
	else if (obj.getNumVars() >= 10) {
		cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
		delim.ReplaceInline(" ", "");
		cweeParser x(inbound, delim, true);
		
		{
			vec5 V; V.Deserialize(x[0]);
			{			
				SetCharacteristic(static_cast<value_t>((int)V[0]));
				SetMeasurement(static_cast<measurement_t>((int)V[1]));
				SetBoundaryType(static_cast<boundary_t>((int)V[2]));
				SetInterpolationType(static_cast<interpolation_t>((int)V[3]));
				SetSpecifier((int)V[4]);
			}
			vec3 t; t.FromString(x[1]); SetMAD(t);
			SetChanged((bool)x[2]);
			SetName(x[3]);
			{
				Lock();
				cweeParser y(x[4], "|", true);
				int finder(-1); cweeStr left, right; u64 a; type b; bool t = false;
				u64 prevTime = 0;
				for (auto& knot : y) {
					finder = knot.Find(',');
					if (finder != -1) {
						knot.Mid(0, finder, left);
						knot.Mid(finder + 1, knot.Length(), right);

						a = DeserializeTime(left) + prevTime;

						if (right.IsEmpty()) b = 0.0f;
						else b = (type)right;
#ifndef  usePhmapBtree
						container.Add(b, a, false);
#else
						container[a] = b;
#endif
						prevTime = a;
					}
				}
				Unlock();
			}

			Lock();
			machineLearn_MinMaxRound.FromString(x[5]);
			machineLearn_MAD.FromString(x[6]);
			{
				vec4 a;
				cweeParser y(x[7], "|", true);
				for (auto& knot : y) {
					a.Deserialize(knot);
					machineLearn_features.Append(a);
				}
			}
			{
				cweeParser y(x[8], "|", true);
				for (auto& knot : y) {
					exclusion_list.Append((bool)knot);
				}
			}
			if (!x[12].IsEmpty() && x[9] != " ") {
				if (!learned) learned = new cweeML_learned_parameters();
				learned->Deserialize(x[9]);
			}
			Unlock();
		}
	}



#else
	cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
	delim.ReplaceInline(" ", "");
	cweeParser x(inbound, delim, true);
	
	if (x.getNumVars() >= 13) {
		SetCharacteristic(static_cast<value_t>((int)x[0]));
		SetMeasurement(static_cast<measurement_t>((int)x[1]));
		SetBoundaryType(static_cast<boundary_t>((int)x[2]));
		SetInterpolationType(static_cast<interpolation_t>((int)x[3]));
		vec3 t; t.FromString(x[4]); SetMAD(t);
		SetChanged((bool)x[5]);
		SetName(x[6]);
		{
			Lock();
			cweeParser y(x[7], "|", true);
			int finder(-1); cweeStr left, right; u64 a; type b; bool t = false;
			u64 prevTime = 0;
			for (auto& knot : y) {
				finder = knot.Find(',');
				if (finder != -1) {
					knot.Mid(0, finder, left);
					knot.Mid(finder + 1, knot.Length(), right);

					a = DeserializeTime(left) + prevTime;

					if (right.IsEmpty()) b = 0.0f;
					else b = (type)right;

#ifndef  usePhmapBtree
					container.Add(b, a, false);
#else
					container[a] = b;
#endif
					prevTime = a;
				}
			}
			Unlock();
		}

		Lock();
		machineLearn_MinMaxRound.FromString(x[8]);
		machineLearn_MAD.FromString(x[9]);
		{
			vec4 a;
			cweeParser y(x[10], "|", true);
			for (auto& knot : y) {
				a.Deserialize(knot);
				machineLearn_features.Append(a);
			}
		}
		{
			cweeParser y(x[11], "|", true);
			for (auto& knot : y) {
				exclusion_list.Append((bool)knot);
			}
		}
		if (!x[12].IsEmpty() && x[12] != " ") {
			if (!learned) learned = new cweeML_learned_parameters();
			learned->Deserialize(x[12]);
		}
		Unlock();

	}
#endif

};

template< class type >
INLINE cweeStr		cweeBalancedPattern<type>::Serialize(int percentToRemove) const {
	if (percentToRemove <= 0)
		RemoveUnnecessaryKnots();
	else
		ReduceMemory(percentToRemove);

#if 0

	cweeStr delim = ":PATTERN_in_DELIM:";
	cweeStr out;
	out.AddToDelimiter((int)GetSpecifier(), delim); // 0
	out.AddToDelimiter((cweeStr)this->GetName(), delim); // 1
	out.AddToDelimiter(this->GetMAD().ToString(), delim); // 2
	out.AddToDelimiter((int)enumClassToInt(this->GetInterpolationType()), delim); // 3
	out.AddToDelimiter((int)this->GetBoundaryType(), delim); // 4
	{
		cweeStr knots;
		{
			cweeStr saver;
			RemoveUnnecessaryKnots();
			for (auto& x : GetKnotSeries()) {
				if (x.second == 0.0f) {
					saver = cweeStr(x.first / serializationTimeConverter);
					saver.Append(",");
					knots.AddToDelimiter(saver, "|");
				}
				else {
					saver = (cweeStr)(x.first / serializationTimeConverter) + "," + (cweeStr)x.second;
					knots.AddToDelimiter(saver, "|");
				}
			}
		}
		out.AddToDelimiter(knots, delim); // 5
	}
	this->Lock(); {
		if (learned) out.AddToDelimiter((cweeStr)learned->Serialize(), delim); // 6			
		else out.AddToDelimiter("", delim); // 6	
		vec3 t = vec3((float)machineLearn_MinMaxRound.z, (float)machineLearn_MinMaxRound.x, (float)machineLearn_MinMaxRound.y); out.AddToDelimiter(t.ToString(), delim); // 7
	} this->Unlock();
	out.AddToDelimiter((int)this->GetCharacteristic(), delim); // 8
	out.AddToDelimiter((int)this->GetMeasurement(), delim); // 9
	this->Lock(); {
		cweeStr contain; for (auto& x : machineLearn_features) contain.AddToDelimiter(x.Serialize(), ","); out.AddToDelimiter(contain, delim); // 10
		out.AddToDelimiter(cweeStr(machineLearn_MAD.GetVec2().ToString()), delim); // 11
		contain = ""; for (auto& x : exclusion_list) contain.AddToDelimiter(x, ","); out.AddToDelimiter(contain, delim); // 12
	} this->Unlock();

	return out;

#else

	cweeStr out; cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
	delim.ReplaceInline(" ", "");
	{
		vec5 t;
		t[0] = static_cast<int>(GetCharacteristic());
		t[1] = static_cast<int>(GetMeasurement());
		t[2] = static_cast<int>(GetBoundaryType());
		t[3] = static_cast<int>(GetInterpolationType());
		t[4] = GetSpecifier();	
		out.AddToDelimiter(t.Serialize(), delim); // 0	
	}
	out.AddToDelimiter(GetMAD().ToString(), delim); // 1
	out.AddToDelimiter(GetChanged(), delim); // 2
	out.AddToDelimiter(GetName(), delim); // 3
	{
		cweeStr knots; cweeStr t;
		{
			u64 prevTime = 0;
			for (auto& x : GetKnotSeries()) {
				t = SerializeTime(x.first - prevTime);
				t += ",";
				if (x.second != 0.0f) t += cweeStr(x.second);
				knots.AddToDelimiter(t, "|");
				prevTime = x.first;
			}
		}
		out.AddToDelimiter(knots, delim); // 4
	}
	out.AddToDelimiter(machineLearn_MinMaxRound.ToString(), delim); // 5
	out.AddToDelimiter(machineLearn_MAD.ToString(), delim); // 6
	{
		cweeStr t;
		{
			for (auto& x : machineLearn_features) {
				t.AddToDelimiter(x.Serialize(), "|");
			}
		}
		out.AddToDelimiter(t, delim); // 7
	}
	{
		cweeStr t;
		{
			for (auto& x : exclusion_list) {
				t.AddToDelimiter(x, "|");
			}
		}
		out.AddToDelimiter(t, delim); // 8
	}
	if (learned) { out.AddToDelimiter(learned->Serialize(), delim); }
	else { out.AddToDelimiter(" ", delim); } // 9

	return out;

#endif
};

template< class type >
INLINE void		cweeBalancedPattern<type>::Update() {
	RemoveUnnecessaryKnots();
};

template< class type >
INLINE cweeStr		cweeBalancedPattern<type>::GetName(void) const {
	cweeStr out;

	Lock();
	out = _Name;
	Unlock();

	return out;
};

template< class type >
INLINE void		cweeBalancedPattern<type>::SetName(const cweeStr& newName) {
	Lock();
	_Name = newName;
	Unlock();
};

template< class type >
INLINE auto& cweeBalancedPattern<type>::UnsafeGetValues() const {
	return container; // assumes already locked
};

template< class type >
INLINE void		cweeBalancedPattern<type>::SetCharacteristic(const value_t& in) {
	Lock();
	valueType = in;
	Unlock();
};

template< class type >
INLINE value_t		cweeBalancedPattern<type>::GetCharacteristic() const {
	value_t out;
	Lock();
	out = valueType;
	Unlock();
	return out;
};

template< class type >
INLINE void		cweeBalancedPattern<type>::SetMeasurement(const measurement_t& in) {
	Lock();
	vec3 conv = cweeUnits::GetMadConversion(measurementType, in);
	Unlock();

	SetMAD(conv);

	Lock();
	measurementType = in;
	Unlock();
};

template< class type >
INLINE measurement_t cweeBalancedPattern<type>::GetMeasurement() const {
	measurement_t out;
	Lock();
	out = measurementType;
	Unlock();
	return out;
};

template< class type >
INLINE void		cweeBalancedPattern<type>::SetBoundaryType(const boundary_t& bt) {
	Lock(); {
		boundaryType = bt;
	} Unlock();
	SetChanged(true);
}

template< class type >
INLINE boundary_t	cweeBalancedPattern<type>::GetBoundaryType() const {
	boundary_t out;
	Lock();
	out = boundaryType;
	Unlock();
	return out;
}

template< class type >
INLINE void		cweeBalancedPattern<type>::SetInterpolationType(const interpolation_t& it) {
	Lock(); {
		interpolationType = it;
	} Unlock();
	SetChanged(true);
}

template< class type >
INLINE interpolation_t	cweeBalancedPattern<type>::GetInterpolationType() const {
	interpolation_t out;
	Lock();
	out = interpolationType;
	Unlock();
	return out;
}

template< class type >
INLINE void		cweeBalancedPattern<type>::SetMAD(float multiply_x, float then_add_to_x) {
	SetMAD(vec3(multiply_x, then_add_to_x, 0));
};

template< class type >
INLINE void		cweeBalancedPattern<type>::SetMAD(const vec3& in) {
	Lock();
	if (MAD != in) {
		// undo the current values;
		for (auto& x : container) {
#ifndef  usePhmapBtree
			*x.object = ((*x.object - MAD.y) / MAD.x);
#else
			x.second = ((x.second - MAD.y) / MAD.x);
#endif
		}

		MAD = in;

		// set the new values;
		for (auto& x : container) {
#ifndef  usePhmapBtree
			* x.object = *x.object * MAD.x + MAD.y;
#else
			x.second = x.second * MAD.x + MAD.y;
#endif			
		}
	}
	Unlock();
};

template< class type >
INLINE vec3		cweeBalancedPattern<type>::GetMAD() const {
	vec3 out;
	Lock();
	out = MAD;
	Unlock();
	return out;
};

template< class type >
INLINE void		cweeBalancedPattern<type>::AddValue(const u64& time, const type& valueIN, bool useMAD) {
	InsertPair(time, valueIN, useMAD, false);
};

template< class type >
INLINE void		cweeBalancedPattern<type>::AddUniqueValue(const u64& time, const type& valueIN, bool useMAD) { // slower with guarrantee of uniqueness 
	InsertPair(time, valueIN, useMAD, true);
};

template< class type >
INLINE void		cweeBalancedPattern<type>::RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo) {
	int lowerLimit, upperLimit, index; cweeThreadedList<int> indexesToDelete;
	Lock();
	{
#ifndef  usePhmapBtree
		auto iter = container.NodeFindLargestSmallerEqual(greaterThan);
		if (iter) {
			do {
				if (iter->key > greaterThan) {
					if (iter->key <= lessThenEqualTo) {
						container.Remove(iter);
					}
					else {
						break;
					}
				}

				iter = container.GetNextLeaf(iter);
			} while (iter);
		}
#else
		auto y = container.upper_bound(lessThenEqualTo);
		while (1) {
			y = container.upper_bound(greaterThan);
			if (y->first > lessThenEqualTo) break;
			container.erase(y);
		}

#endif

	}
	Unlock();
};

template< class type >
INLINE void		cweeBalancedPattern<type>::Clear() {
	Lock();
#ifndef  usePhmapBtree
	container.Clear();
#else
	container.clear();
#endif
	_Name.Clear();
	valueType = value_t::_END_;
	measurementType = measurement_t::_end_;
	MAD = vec3(1, 0, 0);
	changed = false;
	boundaryType = boundary_t::BT_FREE;
	interpolationType = interpolation_t::IT_LINEAR;
	machineLearn_MinMaxRound = vec3(-cweeMath::INF, cweeMath::INF, 0.001f);
	machineLearn_MAD = vec3(1, 0, 0);
	machineLearn_features.Clear();
	exclusion_list.Clear();
	if (learned) { delete learned; learned = nullptr; }

	Unlock();
}

template< class type >
INLINE float		cweeBalancedPattern<type>::GetMinimumDecimals() const {
	float decimal = 1.0f;
	int numbs = GetNumValues();
	if (numbs == 0) return 0.0001f;
	float F;
	int numSuccess = 0;

	Lock();
	for (auto& x : container) {
#ifndef  usePhmapBtree
		F = cweeMath::roundNearest((float)*x.object, decimal);
		if (cweeMath::Fabs((float)(F - *x.object)) > 0.00001) {
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
#else
		F = cweeMath::roundNearest((float)x.second, decimal);
		if (cweeMath::Fabs((float)(F - x.second)) > 0.00001) {
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
#endif
	}
	Unlock();

	return decimal;
};

template< class type >
INLINE u64		cweeBalancedPattern<type>::GetMinimumTimeStep() const {
	u64 out, prevTime, t;
	int numFailures, numbs;

	numbs = GetNumValues();
	if (numbs <= 1) return 1;

	out = GetMaxTime() - GetMinTime();
	prevTime = std::numeric_limits<u64>::max();
	numFailures = 100;

	Lock();
	for (auto& x : container) {
#ifndef  usePhmapBtree
		t = std::abs(prevTime - x.key);
		prevTime = x.key;
		if (t < out && t > 0) {
			out = t;
		}
		else {
			numFailures--;
			if (numFailures <= 0) break;
		}
#else
		t = std::abs(prevTime - x.first);
		prevTime = x.first;
		if (t < out && t > 0) {
			out = t;
		}
		else {
			numFailures--;
			if (numFailures <= 0) break;
		}
#endif
	}
	Unlock();

	if (out <= 1) out = 1;

	return out;
};

template< class type >
INLINE  type		cweeBalancedPattern<type>::GetMinValue() const {
	type out;
	out = 0;
	if (GetNumValues() == 0) return out;
	out = std::numeric_limits<type>::max();

	Lock();
	for (auto& x : container) {
#ifndef  usePhmapBtree
		if (*x.object < out) out = *x.object;
#else
		if (x.second < out) out = x.second;
#endif
	}
	Unlock();

	return out;
};

template< class type >
INLINE  type		cweeBalancedPattern<type>::GetMaxValue() const {
	type out;
	out = 0;
	if (GetNumValues() == 0) return out;
	out = -std::numeric_limits<type>::max();
	Lock();
	for (auto& x : container) {
#ifndef  usePhmapBtree
		if (*x.object > out) out = *x.object;
#else
		if (x.second > out) out = x.second;
#endif
	}
	Unlock();
	return out;
};

template< class type >
INLINE type		cweeBalancedPattern<type>::GetAvgValue() const {
	type out(0);
	int num(0);

	Lock();
	for (auto& x : container) {
		num++;
		out -= (type)(out / (float)num);
#ifndef  usePhmapBtree
		out += (type)(*x.object / (float)num);
#else
		out += (type)(x.second / (float)num);
#endif
	}
	Unlock();

	return out;
};

template< class type >
INLINE type		cweeBalancedPattern<type>::GetMinValue(const u64& start, const u64& end) const {
	type out;
	out = 0;
	int n = GetNumValues();
	if (n == 0) return out;
	out = std::numeric_limits<type>::max();

	Lock();
	{
#ifndef  usePhmapBtree
		auto iter = container.NodeFindLargestSmallerEqual(start);
		if (iter) {
			do {
				if (iter->key > start) {
					if (iter->key <= end) {
						if (*iter->object < out) out = *iter->object;
					}
					else {
						break;
					}					
				}
				iter = container.GetNextLeaf(iter);
			} while (iter);
		}
#else
		auto End = container.end();
		for (auto iter = container.lower_bound(start); iter != End && iter->first <= end; iter++) {
			if (iter->first < out) out = iter->first;
		}

#endif
	}
	Unlock();

	return out;
};

template< class type >
INLINE type		cweeBalancedPattern<type>::GetMaxValue(const u64& start, const u64& end) const {
	type out;
	out = 0;
	int n = GetNumValues();
	if (n == 0) return out;
	out = -std::numeric_limits<type>::max();

	Lock();
	{
#ifdef  usePhmapBtree
		auto End = container.end();
		for (auto iter = container.lower_bound(start); iter != End && iter->first <= end; iter++) {
			if (iter->first > out) out = iter->first;
		}
#else
		auto iter = container.NodeFindLargestSmallerEqual(start);
		if (iter) {
			do {
				if (iter->key > start) {
					if (iter->key <= end) {
						if (*iter->object > out) out = *iter->object;
					}
					else {
						break;
					}
				}
				iter = container.GetNextLeaf(iter);
			} while (iter);
		}
#endif
	}
	Unlock();

	return out;
};

template< class type >
INLINE type		cweeBalancedPattern<type>::GetAvgValue(const u64& start, const u64& end) const {
	type out;
	out = 0;
	int num(0);

	Lock();
	{
#ifdef  usePhmapBtree
		auto End = container.end();
		for (auto iter = container.lower_bound(start); iter != End && iter->first <= end; iter++) {
			cweeMath::rollingAverageRef(out, (float)iter->second, num);
		}
#else
		auto iter = container.NodeFindLargestSmallerEqual(start);
		if (iter) {
			do {
				if (iter->key > start) {
					if (iter->key <= end) {
						cweeMath::rollingAverageRef(out, (float)*iter->object, num);
					}
					else {
						break;
					}
				}
				iter = container.GetNextLeaf(iter);
			} while (iter);
		}
#endif
	}
	Unlock();

	return out;
};

template< class type >
INLINE u64		cweeBalancedPattern<type>::GetAvgTime() const {
	float out(0);
	int num(0);

	Lock();
	for (auto& x : container) {
		num++;
		out -= (type)(out / (float)num);
#ifndef  usePhmapBtree
		out += (type)(x.key / (float)num);
#else
		out += (type)(x.first / (float)num);
#endif
	}
	Unlock();

	return (u64)out;
};

template< class type >
INLINE u64		cweeBalancedPattern<type>::GetMaxTime(void) const {
	u64 out(0);

	if (GetNumValues() == 0)
		return out;
	else
	{
		Lock();
#ifndef  usePhmapBtree
		auto ptr = container.NodeFindLargestSmallerEqual(std::numeric_limits<u64>::max());
		if (ptr) {
			out = ptr->key;
		}
#else
		auto ptr = container.begin();
		if (ptr != container.end()) {
			out = ptr->first;
		}
#endif
		Unlock();

		return out;
	}
}

template< class type >
INLINE  u64		cweeBalancedPattern<type>::GetMinTime(void) const {
	u64 out(0);

	if (GetNumValues() == 0)
		return out;
	else
	{
		Lock();
#ifndef  usePhmapBtree
		auto ptr = container.NodeFindLargestSmallerEqual(0);
		if (ptr) {
			out = ptr->key;
		}
#else
		auto ptr = container.end();
		--ptr;
		if (ptr != container.end()) {
			out = ptr->first;
		}
#endif
		Unlock();

		return out;
	}

}

template< class type >
INLINE  int		cweeBalancedPattern<type>::GetNumValues() const {
	int out;
	Lock();
#ifndef  usePhmapBtree
	out = container.GetNodeCount();
#else
	out = container.size();
#endif
	Unlock();
	return out;
};

template< class type >
INLINE void		cweeBalancedPattern<type>::ShiftTime(const u64& deltaTime) {
	Lock();
	for (auto& x : container) {
#ifndef  usePhmapBtree
		x.key += deltaTime;
#else
		throw("phMap does not support key shifting");
		// x.first += deltaTime;
#endif
	}
	changed = true;
	Unlock();
};

template< class type >
INLINE void		cweeBalancedPattern<type>::Translate(const type& translation) {
	Lock();
	for (auto& x : container) {
#ifndef  usePhmapBtree
		*x.object += translation;
#else
		x.second += translation;
#endif
	}
	changed = true;
	Unlock();
};

template< class type >
INLINE void		cweeBalancedPattern<type>::Copy(const cweeBalancedPattern& copy, const u64& timeStart, const u64& timeEnd) {
	// Clear(); // cleared

	SetName(copy.GetName());
	SetMAD(copy.GetMAD());
	SetBoundaryType(copy.GetBoundaryType());
	SetInterpolationType(copy.GetInterpolationType());
	SetCharacteristic(copy.GetCharacteristic());
	SetMeasurement(copy.GetMeasurement());
	SetChanged(true);

	AcceptNewData(copy(timeStart, timeEnd));

	Lock(); copy.Lock();
	machineLearn_MinMaxRound = copy.machineLearn_MinMaxRound;
	machineLearn_MAD = copy.machineLearn_MAD;
	machineLearn_features = copy.machineLearn_features;
	exclusion_list = copy.exclusion_list;
	if (copy.learned) {
		if (!learned) {
			learned = new cweeML_learned_parameters();
		}
		if (learned) {
			*learned = *copy.learned;
		}
	}
	container = copy.container;
	copy.Unlock(); Unlock();
};

template< class type >
INLINE void		cweeBalancedPattern<type>::operator=(const cweeBalancedPattern<type>& source) {
	Copy(source);
};

template< class type >
INLINE void		cweeBalancedPattern<type>::operator=(const cweeThreadedList<std::pair<u64, type>>& data) {
	this->Clear();
	AcceptNewData(data);
};

template< class type >
INLINE void		cweeBalancedPattern<type>::operator=(float data) {
	this->Clear();
	this->AddValue(0, data);
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator*=(float a) {
	this->Lock();
	for (auto& x : this->container) {
#ifdef  usePhmapBtree
		x.second *= a;
#else
		*x.object *= a;	
#endif
	}
	this->Unlock();

	return *this;
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator/=(float a) {
	this->Lock();
	for (auto& x : this->container) {
#ifdef  usePhmapBtree
		x.second /= (a + cweeMath::EPSILON);
#else
		*x.object /= (a + cweeMath::EPSILON);
#endif
	}
	this->Unlock();
	return *this;
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator+=(float a) {
	this->Lock();
	for (auto& x : this->container) {
#ifdef  usePhmapBtree
		x.second += a;
#else
		*x.object += a;
#endif
	}
	this->Unlock();
	return *this;
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator-=(float a) {
	this->Lock();
	for (auto& x : this->container) {
#ifdef  usePhmapBtree
		x.second -= a;
#else
		*x.object -= a;
#endif
	}
	this->Unlock();
	return *this;
};


template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator*=(const cweeBalancedPattern<type>& a) {
	*this = *this * a;

	return *this;
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator/=(const cweeBalancedPattern<type>& a) {
	*this = *this / a;

	return *this;
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator+=(const cweeBalancedPattern<type>& a) {
	*this = *this + a;

	return *this;
};

template< class type >
INLINE cweeBalancedPattern<type>& cweeBalancedPattern<type>::operator-=(const cweeBalancedPattern<type>& a) {
	*this = *this - a;

	return *this;
};



template< class type >
INLINE cweeThreadedList<std::pair<u64, type>>		cweeBalancedPattern<type>::operator()(const u64& start, const u64& end, const u64& resolution) const {
	return GetTimeSeries(start, end, resolution);
};

template< class type >
INLINE cweeThreadedList<std::pair<u64, type>>		cweeBalancedPattern<type>::operator()(const u64& start, const u64& end) const {
	return GetKnotSeries(start, end);
};

template< class type >
INLINE float										cweeBalancedPattern<type>::operator()(const u64& time) const {
	return GetCurrentValue(time);
};

template< class type >
INLINE void										cweeBalancedPattern<type>::ClampValues(const type& min, const type& max) {
	this->Lock();
	for (auto& x : this->container) {
#ifdef  usePhmapBtree
		x.second = (type)cweeMath::Fmax((float)cweeMath::Fmin(x.second, (float)max), (float)min);
#else
		*x.object = (type)cweeMath::Fmax((float)cweeMath::Fmin(*x.object, (float)max), (float)min);
#endif
	}
	this->Unlock();
};

template< class type >
INLINE type								cweeBalancedPattern<type>::GetCurrentValue(const u64& time, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent, const measurement_t& outboundUnit) const {
	int i = 0, j = 0, k = 0;
	u64 bvals[4], clampedTime;
	type v; type* ptr;
	v = 0;
	j = this->GetNumValues();

	if (j < 1) {
		v = 0;
		return v;
	}
	if (j == 1) {
		Lock();
#ifdef  usePhmapBtree
		auto find = container.lower_bound(time);
		if (find != container.end())
			v = find->second;
#else
		ptr = container.FindLargestSmallerEqual(time);
		if (ptr) {
			v = *ptr;
		}
#endif
		Unlock();
		return v;
	}

	clampedTime = this->ClampedTime(time);
	clampedTime = this->LoopedTime(clampedTime);

	Lock();
	if (learned) {
		i = (int)learned->learned;
	}
	Unlock();

	if ((i > 0) && clampedTime != time) {
		// clamped time != time is indication that we are outside bounds of real dataset / knots.
		v = Forecast_Value_Automated(time, Parent);
	}
	else {
		Lock();
		switch (interpolationType) {
		case interpolation_t::IT_RIGHT_CLAMP: {

#ifdef  usePhmapBtree
			auto find = container.upper_bound(clampedTime);
			if (find != container.end())
				v = find->second;
#else
			ptr = container.FindSmallestLargerEqual(clampedTime);
			if (ptr) v = *ptr;

#endif
			break;
		}
		case interpolation_t::IT_SPLINE: {
			Unlock();
			Basis(clampedTime, bvals);
			Lock();

#ifdef  usePhmapBtree
			auto find = container.lower_bound(clampedTime);
			if (find != container.end()) {
				v = (find->second * (float)bvals[1]);
				--find;
				if (find != container.end()) {
					v += (find->second * (float)bvals[0]);
					find++;
					if (find != container.end()) {
						find++;
						if (find != container.end()) {
							v += (find->second * (float)bvals[2]);
							find++;
							if (find != container.end()) {
								v += (find->second * (float)bvals[3]);
							}
						}
					}
				}
			}
#else
			cweeBalancedTreeNode<type, u64>* x = container.NodeFindLargestSmallerEqual(clampedTime);
			if (x) {
				v = (*x->object * (float)bvals[1]);
				x = container.GetPrevLeaf(x);
				if (x) {
					v += (*x->object * (float)bvals[0]);
					x = container.GetNextLeaf(x);
					if (x) {
						x = container.GetNextLeaf(x);
						if (x) {
							v += (*x->object * (float)bvals[2]);
							x = container.GetNextLeaf(x);
							if (x) {
								v += (*x->object * (float)bvals[3]);
							}
						}
					}
				}
			}
#endif



			break;
		}
		case interpolation_t::IT_LINEAR: {
			Unlock();
			Basis(clampedTime, bvals);
			Lock();
#ifdef  usePhmapBtree
			auto find = container.lower_bound(clampedTime);
			if (find != container.end()) {
				v = (find->second * (float)bvals[1]);
				find++;
				if (find != container.end()) {
					v += (find->second * (float)bvals[2]);
				}
			}
#else
			cweeBalancedTreeNode<type, u64>* x = container.NodeFindLargestSmallerEqual(clampedTime);
			if (x) {
				v = (*x->object * (float)bvals[1]);

				x = container.GetNextLeaf(x);
				if (x) {
					v += (*x->object * (float)bvals[2]);
				}

			}
#endif
			break;
		}
		default:
		case interpolation_t::IT_LEFT_CLAMP: {
#ifdef  usePhmapBtree
			auto find = container.lower_bound(clampedTime);
			if (find != container.end())
				v = find->second;
#else
			ptr = container.FindLargestSmallerEqual(clampedTime);
			if (ptr) v = *ptr;
#endif
			break;
		}
		}
		Unlock();
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
INLINE  type								cweeBalancedPattern<type>::GetCurrentFirstDerivative(const u64& time) const {
	int i, j, k;
	u64 bvals[4], d = 1, clampedTime;
	type v;

	if (GetNumValues() <= 1) {
		v = 0;
		return v;
	}

	clampedTime = ClampedTime(time);
	BasisFirstDerivative(clampedTime, bvals);
	v = 0; //-V501
	Lock();
#ifdef  usePhmapBtree
	auto find = container.lower_bound(clampedTime);
	if (find != container.end()) {
		d = find->first;
		v = (find->second * (float)bvals[1]);
		--find;
		if (find != container.end()) {
			d -= find->first;
			v += (find->second * (float)bvals[0]);
			find++;
			if (find != container.end()) {
				find++;
				if (find != container.end()) {
					v += (find->second * (float)bvals[2]);
					find++;
					if (find != container.end()) {
						v += (find->second * (float)bvals[3]);
					}
				}
			}
		}
	}
#else
	cweeBalancedTreeNode<type, u64>* x = container.NodeFindLargestSmallerEqual(clampedTime);
	if (x) {
		d = x->key;
		v += (*x->object * (float)bvals[1]);
		x = container.GetPrevLeaf(x);
		if (x) {
			d -= x->key;
			v += (*x->object * (float)bvals[0]);
			x = container.GetNextLeaf(x);
			if (x) {
				x = container.GetNextLeaf(x);
				if (x) {
					v += (*x->object * (float)bvals[2]);
					x = container.GetNextLeaf(x);
					if (x) {
						v += (*x->object * (float)bvals[3]);
					}
				}
			}
		}
	}
#endif
	Unlock();
	v /= d;
	return v;
};

template< class type >
INLINE type								cweeBalancedPattern<type>::GetCurrentSecondDerivative(const u64& time) const {

	int i, j, k;
	u64 bvals[4], d = 0, clampedTime;
	type v;

	if (GetNumValues() <= 1) {
		v = 0;
		return v;
	}

	clampedTime = ClampedTime(time);
	BasisSecondDerivative(clampedTime, bvals);
	v = 0; //-V501
	Lock();
#ifdef  usePhmapBtree
	auto find = container.lower_bound(clampedTime);
	if (find != container.end()) {
		d = find->first;
		v = (find->second * (float)bvals[1]);
		--find;
		if (find != container.end()) {
			d -= find->first;
			v += (find->second * (float)bvals[0]);
			find++;
			if (find != container.end()) {
				find++;
				if (find != container.end()) {
					v += (find->second * (float)bvals[2]);
					find++;
					if (find != container.end()) {
						v += (find->second * (float)bvals[3]);
					}
				}
			}
		}
	}
#else
	cweeBalancedTreeNode<type, u64>* x = container.NodeFindLargestSmallerEqual(clampedTime);
	if (x) {
		d = x->key;
		v += (*x->object * (float)bvals[1]);
		x = container.GetPrevLeaf(x);
		if (x) {
			d -= x->key;
			v += (*x->object * (float)bvals[0]);
			x = container.GetNextLeaf(x);
			if (x) {
				x = container.GetNextLeaf(x);
				if (x) {
					v += (*x->object * (float)bvals[2]);
					x = container.GetNextLeaf(x);
					if (x) {
						v += (*x->object * (float)bvals[3]);
					}
				}
			}
		}
	}
#endif
	Unlock();

	v /= (float)(d * d);
	return v;
};

template< class type >
INLINE  cweeThreadedList<std::pair<u64, type>>		cweeBalancedPattern<type>::GetKnotSeries(const u64& timeStart, const u64& timeEnd, const measurement_t& outboundUnit) const {
	int numKnots = this->GetNumValues();
	cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);

	if (outboundUnit != measurement_t::_end_)
	{
		std::pair<u64, type> set;
		vec3 mad0 = cweeUnits::GetMadConversion(this->GetMeasurement(), outboundUnit);

		Lock();
#ifdef  usePhmapBtree

		auto End = container.end();
		for (auto f = container.lower_bound(timeStart); f != End && f->first <= timeEnd; f++) {
			if (f->first >= timeStart) {
				if (f->first < timeEnd) {
					set.first = f->first;
					set.second = f->second * mad0[0] + mad0[1];
					out.Append(set);
				}
				else {
					break;
				}
			}

		}

#else
		for (auto ptr = container.NodeFindLargestSmallerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
			if (ptr->object) {
				if (ptr->key >= timeStart) {
					if (ptr->key < timeEnd) {
						set.first = ptr->key;
						set.second = *ptr->object * mad0[0] + mad0[1];
						out.Append(set);
					}
					else {
						break;
					}
				}
			}
		}
#endif
		Unlock();
	}
	else {
		std::pair<u64, type> set;

		Lock();
#ifdef  usePhmapBtree

		auto End = container.end();
		for (auto f = container.lower_bound(timeStart); f != End && f->first <= timeEnd; f++) {
			if (f->first >= timeStart) {
				if (f->first < timeEnd) {
					set.first = f->first;
					set.second = f->second;
					out.Append(set);
				}
				else {
					break;
				}
			}

		}

#else

		for (auto ptr = container.NodeFindLargestSmallerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
			if (ptr->object) {
				if (ptr->key >= timeStart) {
					if (ptr->key < timeEnd) {
						set.first = ptr->key;
						set.second = *ptr->object;
						out.Append(set);
					}
					else {
						break;
					}
				}
			}
		}

#endif
		Unlock();
	}
	return out;
};

template< class type >
INLINE cweeThreadedList<std::pair<u64, type>>		cweeBalancedPattern<type>::GetReversedKnotSeries(const u64& timeStart, const u64& timeEnd, const measurement_t& outboundUnit) const {
	int numKnots = this->GetNumValues();
	cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);

	if (outboundUnit != measurement_t::_end_)
	{
		std::pair<u64, type> set;
		vec3 mad0 = cweeUnits::GetMadConversion(this->GetMeasurement(), outboundUnit);

		Lock();
#ifdef  usePhmapBtree

		auto End = container.end(); auto f = container.upper_bound(timeEnd); f--;
		for (; f != End && f->first >= timeStart; f--) {
			if (f->first >= timeStart) {
				if (f->first < timeEnd) {
					set.first = f->first;
					set.second = f->second * mad0[0] + mad0[1];
					out.Append(set);
				}
			}
		}

#else
		for (auto ptr = container.NodeFindLargestSmallerEqual(timeEnd); ptr; ptr = container.GetPrevLeaf(ptr)) {
			if (ptr->object) {
				if (ptr->key >= timeStart) {
					if (ptr->key < timeEnd) {
						set.first = ptr->key;
						set.second = *ptr->object * mad0[0] + mad0[1];
						out.Append(set);
					}
					else {
						break;
					}
				}
			}
		}
#endif
		Unlock();
	}
	else {
		std::pair<u64, type> set;

		Lock();
#ifdef  usePhmapBtree

		auto End = container.end(); auto f = container.upper_bound(timeEnd); f--;
		for (; f != End && f->first >= timeStart; f--) {
			if (f->first >= timeStart) {
				if (f->first < timeEnd) {
					set.first = f->first;
					set.second = f->second;
					out.Append(set);
				}
			}
		}

#else
		for (auto ptr = container.NodeFindLargestSmallerEqual(timeEnd); ptr; ptr = container.GetPrevLeaf(ptr)) {
			if (ptr->object) {
				if (ptr->key >= timeStart) {
					if (ptr->key < timeEnd) {
						set.first = ptr->key;
						set.second = *ptr->object;
						out.Append(set);
					}
					else {
						break;
					}
				}
			}
		}
#endif
		Unlock();
	}
	return out;
};

template< class type >
INLINE cweeThreadedList<std::pair<u64, type>>		cweeBalancedPattern<type>::GetTimeSeries(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent, const measurement_t& outboundUnit) const {
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

template< class type >
INLINE cweeThreadedList<type>				cweeBalancedPattern<type>::GetValueKnotSeries(const u64& timeStart, const u64& timeEnd, const measurement_t& outboundUnit) const {
	int numKnots = this->GetNumValues();
	cweeThreadedList<type> out(numKnots + 16);

	if (outboundUnit != measurement_t::_end_)
	{
		vec3 mad0 = cweeUnits::GetMadConversion(this->GetMeasurement(), outboundUnit);

		Lock();
#ifdef  usePhmapBtree

		auto End = container.end();
		for (auto f = container.lower_bound(timeStart); f != End && f->first <= timeEnd; f++) {
			if (f->first >= timeStart) {
				if (f->first < timeEnd) {
					out.Append(f->second * mad0[0] + mad0[1]);
				}
				else {
					break;
				}
			}

		}

#else
		for (auto ptr = container.NodeFindLargestSmallerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
			if (ptr->object) {
				if (ptr->key >= timeStart) {
					if (ptr->key < timeEnd) {
						out.Append(*ptr->object * mad0[0] + mad0[1]);
					}
					else {
						break;
					}
				}
			}
		}
#endif
		Unlock();
	}
	else {
		Lock();
#ifdef  usePhmapBtree

		auto End = container.end();
		for (auto f = container.lower_bound(timeStart); f != End && f->first <= timeEnd; f++) {
			if (f->first >= timeStart) {
				if (f->first < timeEnd) {
					out.Append(f->second);
				}
				else {
					break;
				}
			}

		}

#else
		for (auto ptr = container.NodeFindLargestSmallerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
			if (ptr->object) {
				if (ptr->key >= timeStart) {
					if (ptr->key < timeEnd) {
						out.Append(*ptr->object);
					}
					else {
						break;
					}
				}
			}
		}
#endif
		Unlock();
	}
	return out;
};

template< class type >
INLINE  cweeThreadedList<type>				cweeBalancedPattern<type>::GetValueTimeSeries(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent, const measurement_t& outboundUnit) const {
	u64 realTimestep = cweeMath::Fmax(timeStep, 1);
	cweeThreadedList<type> out(cweeMath::max(cweeMath::min((timeEnd - timeStart / (realTimestep)), 100000), 1000) + 16);

	out.Append(GetCurrentValue(timeStart, Parent, outboundUnit)); // ensure pattern always has a starter? 
	for (u64 t = timeStart + realTimestep; t < timeEnd; t += realTimestep) {
		out.Append(GetCurrentValue(t, Parent, outboundUnit));
	}
	out.Append(GetCurrentValue(timeEnd, Parent, outboundUnit)); // ensure pattern always has a closure? 

	return out;
};

template< class type >
INLINE  type								cweeBalancedPattern<type>::RombergIntegral(const u64& t0, const u64& t1, const float divisor, bool snapLeft) const {
	type sum;
	sum = 0.0f;

	if (this->GetNumValues() <= 1) { return sum; }

	u64 step = this->GetMinimumTimeStep();
	type v; v = 0.0f; 
	u64 minGot = t1, maxGot = t1;
	if (true) {
		AUTO Guard = this->lock.Guard();
		if (Guard) {
			cweeThreadedList<cweeBalancedTreeNode<type, u64>*> data = this->UnsafeGetKnotSeries(t0, t1);
			if (data.Num() > 1) {
				minGot = data[0]->key;
				maxGot = data[data.Num() - 1]->key;

				for (int i = 0; i < (data.Num() - 1); i++) {
					cweeBalancedTreeNode<type, u64>& left = *data.operator[](i), right = *data.operator[](i+1);
					v = (right.key - left.key) * ((*right.object + *left.object) / 2.0f);
					if (::isfinite(v))
						sum += v;					
				}
			}			
		}
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

template< class type >
INLINE  cweeThreadedList<float>				cweeBalancedPattern<type>::ValueQuantiles(const cweeThreadedList<float>& probs, const u64& timeStart, const u64& timeEnd, int numSamples) {
	cweeThreadedList<float> quantiles;
	float poi, datLeft, datRight, quantile;
	size_t left, right;

	cweeThreadedList<float> data;
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

	if (data.Num() == 0)
	{
		return data;
	}

	if (1 == data.Num())
	{
		return data;
	}

	data.Sort<float, TAG_LIST>(); // sort data

	for (size_t i = 0; i < probs.Num(); ++i)
	{
		poi = (1.0f - probs[i]) * -0.5f + probs[i] * (data.Num() - 0.5f);
		left = std::max(int64_t(std::floor(poi)), int64_t(0));
		right = std::min(int64_t(std::ceil(poi)), int64_t(data.Num() - 1.0f));
		datLeft = data[left];
		datRight = data[right];
		quantile = (1.0f - (poi - left)) * datLeft + (poi - left) * datRight;
		quantiles.Append(quantile);
	}
	return quantiles;
}

template< class type >
INLINE  void							cweeBalancedPattern<type>::RemoveUnnecessaryKnots(const u64& start, const u64& end) const {
	if (start >= end) return;
	int num, index; cweeThreadedList<u64> keysToDelete;
	constexpr float epsilon = 0.00001f;
	type* val1 = nullptr; type* val2 = nullptr; type* val3 = nullptr; type* val4 = nullptr; type* val5 = nullptr;
	u64 t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;

	Lock();
	{
		decltype(container)& _values = container;
#ifdef  usePhmapBtree
		num = _values.size();
#else
		num = _values.GetNodeCount();
#endif
		if (num > 5) {
			index = 0;

#ifdef  usePhmapBtree
			// TO-DO

#else
			for (cweeBalancedTreeNode<type, u64>& knot : _values) {
				if (index + 3 >= num) break;
				if (index >= 5) {
					if (cweeMath::Fabs((float)(*val1 - *val2)) > epsilon) { // !=
						++index;
						val1 = val2; t1 = t2;
						val2 = val3; t2 = t3;
						val3 = val4; t3 = t4;
						val4 = val5; t4 = t5;
						val5 = knot.object;
						t5 = knot.key;
						continue;
					}
					else if (cweeMath::Fabs((float)(*val5 - *val2)) < epsilon) { // ==
						if (cweeMath::Fabs((float)(*val4 - *val2)) < epsilon) { // ==
							if (cweeMath::Fabs((float)(*val3 - *val2)) < epsilon) { // ==
								keysToDelete.Append(t3);
								++index;
								val3 = val4; t3 = t4;
								val4 = val5; t4 = t5;
								val5 = knot.object;
								t5 = knot.key;
								continue; // don't move the currentPos forward. Repeat the analysis from this spot. 								
							}
						}
					}
					val1 = val2; t1 = t2;
					val2 = val3; t2 = t3;
					val3 = val4; t3 = t4;
					val4 = val5; t4 = t5;
					val5 = knot.object;
					t5 = knot.key;
				}
				else {
					switch (index) {
					case 0:
						val1 = knot.object;
						t1 = knot.key;
						break;
					case 1:
						val2 = knot.object;
						t2 = knot.key;
						break;
					case 2:
						val3 = knot.object;
						t3 = knot.key;
						break;
					case 3:
						val4 = knot.object;
						t4 = knot.key;
						break;
					case 4:
						val5 = knot.object;
						t5 = knot.key;
						break;
					}
				}
				index++;
			}
#endif

		}
#ifdef  usePhmapBtree
		// TO-DO
#else
		for (auto& key : keysToDelete) {
			_values.Remove(_values.NodeFind(key));
		}
#endif
	}
	Unlock();

	// return keysToDelete;
};

template< class type >
INLINE void								cweeBalancedPattern<type>::ReduceMemory(float percentToRemove, const u64& start, const u64& end) const {
	if (!this->GetChanged()) return;
	if (start >= end) return;
	RemoveUnnecessaryKnots(start, end);
	if (percentToRemove <= 0) return;

	type maxValue;
	type minValue;
	minValue = this->GetMinValue();
	maxValue = this->GetMaxValue();
	type epsilon = cweeMath::Fabs(maxValue - minValue) * (((float)percentToRemove) / 100.0f);
	int num, index; cweeThreadedList<u64> keysToDelete;

	type* val1 = nullptr; type* val2 = nullptr; type* val3 = nullptr; type* val4 = nullptr; type* val5 = nullptr;
	u64 t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;

	Lock();
	{
		decltype(container)& _values = container;
#ifdef  usePhmapBtree
		num = _values.size();
#else
		num = _values.GetNodeCount();
#endif

#ifdef  usePhmapBtree
		// TO-DO
#else
		if (num > 5) {
			index = 0;
			for (cweeBalancedTreeNode<type, u64>& knot : _values) {
				if (index + 3 >= num) break;
				if (index >= 5) {
					if (
						(cweeMath::Fabs((float)(*val2 - *val3)) <= epsilon)
						&& (cweeMath::Fabs((float)(*val4 - *val3)) <= epsilon)
						&& (cweeMath::Fabs((float)(*val2 - *val4)) <= epsilon)
						) { // vals 2 through 4 are within epsilon range
						if (((*val2 >= *val3) && (*val3 >= *val4)) || ((*val2 <= *val3) && (*val3 <= *val4))) { // i.e. the 'center' value is neither the max or min value in this relationship
							keysToDelete.Append(t3);
							++index;
							val3 = val4; t3 = t4;
							val4 = val5; t4 = t5;
							val5 = knot.object;
							continue; // don't move the currentPos forward. Repeat the analysis from this spot. 				
						}
					}
					val1 = val2; t1 = t2;
					val2 = val3; t2 = t3;
					val3 = val4; t3 = t4;
					val4 = val5; t4 = t5;
					val5 = knot.object;
					t5 = knot.key;
				}
				else {
					switch (index) {
					case 0:
						val1 = knot.object;
						t1 = knot.key;
						break;
					case 1:
						val2 = knot.object;
						t2 = knot.key;
						break;
					case 2:
						val3 = knot.object;
						t3 = knot.key;
						break;
					case 3:
						val4 = knot.object;
						t4 = knot.key;
						break;
					case 4:
						val5 = knot.object;
						t5 = knot.key;
						break;
					}
				}
				index++;
			}
		}
#endif

#ifdef  usePhmapBtree
		// TO-DO
#else
		for (auto& key : keysToDelete) {
			_values.Remove(_values.NodeFind(key));
		}
#endif
	}
	Unlock();

	this->SetChanged(false);
};

template< class type >
INLINE  vec2								cweeBalancedPattern<type>::GetLearnedPeriod() const {
	vec2 out(0, 0);

	this->Lock();
	if (learned) {
		out.x = learned->learnPeriod.x;
		out.y = learned->learnPeriod.y;
	}
	this->Unlock();

	return out;
};

template< class type >
INLINE  void								cweeBalancedPattern<type>::RemoveOlderThan(const u64& time) {
	cweeThreadedList<u64> keysToDelete;
	Lock();
	{
		decltype(container)& _values = container;
#ifdef  usePhmapBtree
		auto y = container.lower_bound(time);
		while (1) {
			y = container.lower_bound(time);
			if (y == container.end() || y->first > time) break;
			container.erase(y);
		}
#else
		for (auto& x : _values) {
			if (x.key > time) keysToDelete.Append(x.key);
		}

		for (auto& key : keysToDelete) {
			_values.Remove(_values.NodeFind(key));
		}
#endif
	}
	Unlock();
};

template< class type >
INLINE  void								cweeBalancedPattern<type>::AcceptNewData(const cweeThreadedList<std::pair<u64, type>>& list, bool useMAD) {
	for (auto& x : list) {
		this->AddUniqueValue(x.first, x.second, useMAD);
	}
}

template< class type >
INLINE  void								cweeBalancedPattern<type>::OverridePeriod(const cweeThreadedList<std::pair<u64, type>>& list, bool useMAD) {
	if (list.Num() <= 0) return;
	u64 start = list[0].first;
	u64 end = list[list.Num() - 1].first;
	cweeThreadedList<u64> keysToDelete;
	type value;
	value = 0;

	Lock();
	{
		decltype(container)& _values = container;
		for (auto& x : _values) {
#ifdef  usePhmapBtree
			auto y = container.upper_bound(start);
			while (1) {
				y = container.upper_bound(start);
				if (y == container.end() || y->first > end) break;
				container.erase(y);
			}
#else
			if (x.key >= start) {
				if (x.key <= end) {
					keysToDelete.Append(x.key);
				}
				else {
					break;
				}
			}
#endif
		}
	}
	Unlock();

	Lock();
	{
		decltype(container)& _values = container;
#ifdef  usePhmapBtree
		// Doesnt use this step
#else
		for (auto& key : keysToDelete) {
			_values.Remove(_values.NodeFind(key));
		}
#endif
		changed = true;
	}
	Unlock();

	{
		decltype(container)& _values = container;

		for (auto& x : list) {
			InsertPair(x.first, x.second, useMAD, true);
		}
		SetChanged(true);
	}
}

template< class type >
INLINE  void							cweeBalancedPattern<type>::AcceptChanges(const cweeBalancedPattern<type>& copy) {
	SetName(copy.GetName());
	SetMAD(copy.GetMAD());
	SetBoundaryType(copy.GetBoundaryType());
	SetInterpolationType(copy.GetInterpolationType());
	SetCharacteristic(copy.GetCharacteristic());
	SetMeasurement(copy.GetMeasurement());
	SetChanged(true);

	Lock(); copy.Lock();
	machineLearn_MinMaxRound = copy.machineLearn_MinMaxRound;
	machineLearn_MAD = copy.machineLearn_MAD;
	machineLearn_features = copy.machineLearn_features;
	exclusion_list = copy.exclusion_list;
	if (copy.learned) {
		if (!learned) {
			learned = new cweeML_learned_parameters();
		}
		if (learned) {
			*learned = *copy.learned;
		}
	}
	container = copy.container;
	copy.Unlock(); Unlock();

	AcceptNewData(copy.GetKnotSeries());
}

template< class type >
INLINE  void								cweeBalancedPattern<type>::SwapLearner(const cweeBalancedPattern<type>& copy) {
	Lock(); copy.Lock();
	machineLearn_MinMaxRound = copy.machineLearn_MinMaxRound;
	machineLearn_MAD = copy.machineLearn_MAD;
	machineLearn_features = copy.machineLearn_features;
	exclusion_list = copy.exclusion_list;
	if (copy.learned) {
		if (!learned) {
			learned = new cweeML_learned_parameters();
		}
		if (learned) {
			*learned = *copy.learned;
		}
	}
	container = copy.container;
	copy.Unlock(); Unlock();
};

template< class type >
INLINE  type								cweeBalancedPattern<type>::GetCurrentMovingAverage(const u64& time) const {
	type out; cweeBalancedTreeNode<type, u64>* ptr = nullptr;
	out = 0; int i = 0;
	Lock();
#ifdef  usePhmapBtree
	auto f = container.lower_bound(time);
	for (; i < 6; i++) {
		if (f != container.end())
			out += f->second;
		else
			break;

		f--;
	}


#else
	ptr = container.NodeFindLargestSmallerEqual(time);
	if (ptr) {
		out += *ptr->object;
		for (; i < 5; i++) {
			if (ptr) {
				ptr = container.GetPrevLeaf(ptr);
				if (ptr) {
					out += *ptr->object;
				}
			}
		}
	}
#endif
	Unlock();
	return out / 6.0f;
};

template< class type >
INLINE  cweeBalancedPattern<type>		cweeBalancedPattern<type>::GetTimePattern(bool returnHour) const {
	cweeBalancedPattern<type> out;
	cweeTime tmp;
	if (returnHour == true) {
		// return 0 - 23 value representing the hour (0 is midnight)
		Lock();
		for (auto& x : container) {
#ifdef  usePhmapBtree
			tmp = getLocalTime(x.first);
			out.AddValue(x.first, tmp.tm_hour);
#else
			tmp = getLocalTime(x.key);
			out.AddValue(x.key, tmp.tm_hour());
#endif
		}
		Unlock();
	}
	else {
		// return 0 - 6 value representing the day of the week (0 is Sunday)
		Lock();
		for (auto& x : container) {
#ifdef  usePhmapBtree
			tmp = getLocalTime(x.first);
			out.AddValue(x.first, tmp.tm_wday);
#else
			tmp = getLocalTime(x.key);
			out.AddValue(x.key, tmp.tm_wday());
#endif
		}
		Unlock();
	}
	return out;
};

template< class type >
INLINE int								cweeBalancedPattern<type>::GetMonthOfYear(const u64& time) {
	return getLocalTime(time).tm_mon();
};

template< class type >
INLINE int								cweeBalancedPattern<type>::GetDayOfMonth(const u64& time) {
	return getLocalTime(time).tm_mday();
};

template< class type >
INLINE int								cweeBalancedPattern<type>::GetDayOfWeek(const u64& time) {
	return getLocalTime(time).tm_wday();
};

template< class type >
INLINE float								cweeBalancedPattern<type>::GetHourOfDay(const u64& time) {
	cweeTime tmp = getLocalTime(time);
	return (((float)tmp.tm_hour()) + (((float)tmp.tm_min()) / 60.0f) + (((float)tmp.tm_sec()) / 3600.0f));
};

template< class type >
INLINE  int								cweeBalancedPattern<type>::GetSeason(const u64& time) {
	int month = GetMonthOfYear(time);
	switch (month) {
	case 0:
	case 1:
	case 2: { // mar
		return 0;
		break;
	}
	case 3:
	case 4:
	case 5: { // june
		return 1;
		break;
	}
	case 6:
	case 7:
	case 8: { // sep
		return 2;
		break;
	}
	case 9:
	case 10:
	case 11: { // dec
		return 3;
		break;
	}
	}
};

template< class type >
INLINE  type								cweeBalancedPattern<type>::GetNormalizedValue(const u64& time) const {
	auto min = this->GetMinValue();
	auto max = this->GetMaxValue();

	if (min == max) max = min + 1.0f;

	return (GetCurrentValue(time) - min) / (max - min);
};

template< class type >
INLINE  cweeBalancedPattern<type>			cweeBalancedPattern<type>::GetTransformedPattern(int choice) const {
	cweeBalancedPattern<type> out;
	cweeBalancedTreeNode<type, u64>* x = nullptr;
	switch (choice) {
	case 0: {
		{
			u64 time; u64 timeDif;
			type riseOverRun;

			Lock();
#ifdef  usePhmapBtree
			for (auto f = container.begin(); f != container.end(); f++) {
				time = f->first;
				riseOverRun = -1.0f * f->second;

				f++;
				if (f == container.end()) break;

				timeDif = f->first - time;
				time += f->first; time /= 2.0;
				riseOverRun += f->second;
				riseOverRun /= (float)(timeDif);

				out.AddUniqueValue(time, riseOverRun);
			}
#else
			for (x = container.NodeFindLargestSmallerEqual(0); x && x->object; x = container.GetNextLeaf(x)) {
				time = x->key;
				riseOverRun = -1.0f * (*x->object);

				x = container.GetNextLeaf(x);
				if (!x || !x->object) break;

				timeDif = x->key - time;
				time += x->key; time /= 2.0;
				riseOverRun += *x->object;
				riseOverRun /= (float)(timeDif);

				out.AddUniqueValue(time, riseOverRun);
			}
#endif
			Unlock();
		}

		break;
	}
	case 1: {
		out = GetTransformedPattern(0);
		out = out.GetTransformedPattern(0);

		break;
	}
	case 2: {
		float rollingAvg = 0; int numSamples = 0;
		Lock();
		for (auto& z : container) {
#ifdef  usePhmapBtree
			cweeMath::rollingAverageRef(rollingAvg, z.second, numSamples);
			out.AddUniqueValue(z.first, rollingAvg);
#else
			cweeMath::rollingAverageRef(rollingAvg, *z.object, numSamples);
			out.AddUniqueValue(z.key, rollingAvg);
#endif
		}
		Unlock();
		break;
	}
	case 3: {
		auto min = this->GetMinValue();
		auto max = this->GetMaxValue();
		auto maxMmin = (max - min);
		if (min == max) max = min + 1.0f;

		Lock();
		for (auto& z : container) {
#ifdef  usePhmapBtree
			out.AddUniqueValue(z.first, (z.second - min) / maxMmin);
#else
			out.AddUniqueValue(z.key, (*z.object - min) / maxMmin);
#endif
		}
		Unlock();

		break;
	}
	default: {
		break;
	}
	}
	return out;
}

template< class type >
INLINE  cweeBalancedPattern<type>			cweeBalancedPattern<type>::DuplicateWithNewData(const cweeThreadedList<std::pair<u64, type>>& in, bool useMAD) {
	cweeBalancedPattern<type> out;
	out.Copy(*this);
	out.Lock();
#ifdef  usePhmapBtree
	out.container.clear();
#else
	out.container.Clear();
#endif
	out.Unlock();

	for (auto& x : in) {
		out.AddUniqueValue(x.first, x.second, useMAD);
	}
	return out;
}

template< class type >
INLINE  bool								cweeBalancedPattern<type>::isLearned() const {
	bool out = false;
	this->Lock();
	if (learned) {
		out = learned->learned;
	}
	this->Unlock();
	return out;
};

template< class type >
INLINE 	cweeBalancedPattern<type>			cweeBalancedPattern<type>::GuassianBlur() {
	cweeBalancedPattern<type> out;
	int i = this->GetNumValues();

	if (i == 1) {
		out = *this;
	}
	else {
		{
			Lock();
#ifdef  usePhmapBtree
			auto x = container.begin();
			if (x != container.end()) {
				out.AddUniqueValue(x->first, x->second);
			}
#else
			auto x = container.NodeFindLargestSmallerEqual(0);
			if (x) {
				out.AddUniqueValue(x->key, *x->object);
			}
#endif
			Unlock();
		}
		{
			Lock();
#ifdef  usePhmapBtree
			auto x = container.end();
			x--;
			if (x != container.end()) {
				out.AddUniqueValue(x->first, x->second);
			}
#else
			auto x = container.NodeFindLargestSmallerEqual(std::numeric_limits<u64>::max());
			if (x) {
				out.AddUniqueValue(x->key, *x->object);
			}
#endif
			Unlock();
		}
		{
			cweeBalancedTreeNode<type, u64>* x = nullptr; u64 t; type v;
			Lock();
#ifdef  usePhmapBtree
			for (auto f = container.begin(); f != container.end(); f++) {
				t = f->first;
				v = f->second;

				f++;
				if (f == container.end()) break;

				out.AddUniqueValue(
					(t + f->first) / 2.0f,
					(v + f->second) / 2.0f
				);
			}
#else
			for (x = container.NodeFindLargestSmallerEqual(0); x && x->object; x = container.GetNextLeaf(x)) {
				t = x->key;
				v = *x->object;

				x = container.GetNextLeaf(x);
				if (!x || !x->object) break;

				out.AddUniqueValue(
					(t + x->key) / 2.0f,
					(v + *x->object) / 2.0f
				);
			}
#endif
			Unlock();
		}
	}
	return out;

};

template< class type >
INLINE 	cweeBalancedPattern<type>			cweeBalancedPattern<type>::LaplacianBlend(const cweeBalancedPattern<type>& highResFeatures) const {
	cweeBalancedPattern<type> out;

	cweeBalancedPattern<type> base = *this; base.RemoveUnnecessaryKnots(); base.SetBoundaryType(boundary_t::BT_LOOP);
	cweeBalancedPattern<type> features = highResFeatures; features.RemoveUnnecessaryKnots(); features.SetBoundaryType(boundary_t::BT_LOOP);

	u64 min = ::Min(base.GetMinTime(), features.GetMinTime());
	u64 max = ::Max(base.GetMaxTime(), features.GetMaxTime());
	u64 timeStep = ::Min(base.GetMinimumTimeStep(), features.GetMinimumTimeStep());

	cweeBalancedPattern<type> base2 = base.GetTimeSeries(min, max, timeStep);
	cweeBalancedPattern<type> features2 = features.GetTimeSeries(min, max, timeStep);

	// determine the "features" of the incoming pattern
	cweeThreadedList< cweeBalancedPattern<type> > a_blurs; a_blurs.Append(base2);
	cweeThreadedList< cweeBalancedPattern<type> > a_laplacians;

	cweeThreadedList< cweeBalancedPattern<type> > b_blurs; b_blurs.Append(features2 - features2.GetAvgValue());
	cweeThreadedList< cweeBalancedPattern<type> > b_laplacians;

	int j = 1;
	for (float k = cweeMath::max(a_blurs[0].GetNumValues(), b_blurs[0].GetNumValues()); k >= 1; k /= 2.0f) j++;

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

template< class type >
INLINE cweeBalancedPattern<type>			cweeBalancedPattern<type>::AdjustPatternTillIntegralsMatch(const cweeBalancedPattern<type>& toMatch_, cweeBalancedPattern<type>& toManipulate, const u64& start, const u64& end, const u64& toMatchTargetResolution) {
	// we can (in theory) manipulate "toManipulate" in many different ways until its integration(s) match the integration(s) of "toMatch"
	cweeBalancedPattern<type> toMatch;
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

	cweeBalancedPattern<type> out;

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
			float v1 = toMatch.GetAvgValue();
			float v2 = toManipulate.GetAvgValue();
			out *= v1 / (v2 == 0 ? (v2 + cweeMath::EPSILON) : v2);
			break;
		}
		default: {
			float v1, v2, mod; u64 t1, t2; int j = 0;


			{
				cweeBalancedTreeNode<type, u64>* x = nullptr;
				cweeBalancedTreeNode<type, u64>* y = nullptr;
				u64 t; type v; bool first = true;
				toMatch.Lock();


#ifdef  usePhmapBtree

				// TO-DO


#else
				for (x = toMatch.container.NodeFindLargestSmallerEqual(0); x && x->object; x = toMatch.container.GetNextLeaf(x)) {
					y = x;
					y = toMatch.container.GetNextLeaf(y);

					t1 = x->key; t2 = y->key;

					toMatch.Unlock();
					v1 = toMatch.RombergIntegral(t1, t2);
					toMatch.Lock();

					v2 = toManipulate.RombergIntegral(t1, t2);

					mod = v1 / (v2 == 0 ? (v2 + cweeMath::EPSILON) : v2);

					out.Lock();
					for (auto& z : out.container) {
						if (first) {
							if (z.key >= t1) {
								if (z.key <= t2)
									*z.object *= mod;
								else
									break;
							}
						}
						else {
							if (z.key > t1) {
								if (z.key <= t2)
									*z.object *= mod;
								else
									break;
							}
						}
					}
					out.Unlock();

					first = false;
				}
#endif



				toMatch.Unlock();
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

template< class type >
INLINE type							cweeBalancedPattern<type>::GetTransformedCurrentValue(const u64& time, patternModifier mod, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent) const {
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

		u64 timeDif;
		type riseOverRun;
		riseOverRun = 0;

		if (GetNumValues() >= 2) {

			Lock();
#ifdef  usePhmapBtree

			auto f = container.begin(); 
				timeDif = f->first;
				riseOverRun = -1.0f * f->second;

				f++;
				if (f == container.end()) break;

				timeDif = f->first - timeDif;
				riseOverRun += f->second;
				riseOverRun /= (float)(timeDif);
			
#else
			auto x = container.NodeFindLargestSmallerEqual(time);
			if (x && x->object) {
				timeDif = x->key;
				riseOverRun = -1.0f * (*x->object);

				x = container.GetNextLeaf(x);
				if (x && x->object) {

					timeDif = x->key - timeDif;
					riseOverRun += *x->object;
					riseOverRun /= (float)(timeDif);
				}
			}
#endif
			Unlock();
		}
		return riseOverRun;

		break;
	}
	case patternModifier::Acceleration: {
		return GetCurrentSecondDerivative(time);

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
};;

template< class type >
INLINE void								cweeBalancedPattern<type>::TemporarilyAdjustMachineLearnForecasts(const vec2& MAD_Adjustment) {
	this->Lock();
	machineLearn_MAD.x = MAD_Adjustment.x;
	machineLearn_MAD.y = MAD_Adjustment.y;
	this->Unlock();
};

template< class type >
INLINE  vec2								cweeBalancedPattern<type>::Learn_Automated(const cweeThreadedList<std::pair<cweeStr, vec2>> FeaturePatterns, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent, const u64& timeStart, const u64& timeEnd) {
	// find the source of the feature patterns. if they cannot be found, they cannot be used. 
	int parentIndex;
	for (auto& x : FeaturePatterns) {
		if (x.first != this->GetName()) {
			if (Parent != nullptr) {
				parentIndex = Parent->FindIndexWithName(x.first);
				if (parentIndex > 0) {
					patternSource source = patternSource::Parent;
					this->Lock();
					machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
					this->Unlock();
					continue;
				}
			}
			parentIndex = Globals->FindIndexWithName(x.first);
			if (parentIndex >= 0) {
				patternSource source = patternSource::Global;
				this->Lock();
				machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
				this->Unlock();
				continue;
			}
			parentIndex = Scada->FindIndexWithName(x.first);
			if (parentIndex >= 0) {
				patternSource source = patternSource::Scada;
				this->Lock();
				machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
				this->Unlock();
				continue;
			}
			parentIndex = Customers->FindIndexWithName(x.first);
			if (parentIndex >= 0) {
				patternSource source = patternSource::Customers;
				this->Lock();
				machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
				this->Unlock();
				continue;
			}
			// not found anywhere. Bad luck, it cannot be used in the future.
		}
	}
	return Relearn_Automated(Parent, timeStart, timeEnd);
};

template< class type >
INLINE vec2								cweeBalancedPattern<type>::Relearn_Automated(const cweeUnorderedList< cweeBalancedPattern<type> >* Parent, const u64& timeStart, const u64& timeEnd) {
	// instantiate the 'learned' object 
	this->Lock();
	if (!learned) {
		learned = new cweeML_learned_parameters();
	}
	this->Unlock();

	float splitSample = 20;

	if (this->isLearned()) {
		vec2 learnedPeriod = this->GetLearnedPeriod();
		this->RemoveUnnecessaryKnots(0, learnedPeriod.x);
		this->RemoveUnnecessaryKnots(learnedPeriod.y);
	}
	else {
		this->RemoveUnnecessaryKnots();
	}

	// override the current learned parameters based on the content of the machineLearn_features
	constexpr int MaxNumSamples = 80640; // 2688
	cweeThreadedList< std::pair<u64, type> >							labels;
	cweeThreadedList < cweeThreadedList< std::pair<u64, type> > >		features;

	bool learnWithExpansion = false;

	this->Lock();
	u64 start;
	if (learned && learned->learned && learned->learnPeriod.x != 0 && timeStart == 0) {
		start = learned->learnPeriod.x;
		this->Unlock();
		learnWithExpansion = false;
	}
	else {
		this->Unlock();
		start = ((timeStart == 0) ? this->GetMinTime() : timeStart);
		learnWithExpansion = true;
	}

	this->Lock();
	u64 end;
	if (learned && learned->learned && learned->learnPeriod.y != 0 && timeEnd == 0) {
		end = learned->learnPeriod.y;
		this->Unlock();
		learnWithExpansion = false;
	}
	else {
		this->Unlock();
		end = ((timeEnd == 0) ? this->GetMaxTime() : timeEnd);
		learnWithExpansion = true;
	}

	if (end <= start) {
		end = start + 7 * 24 * 60 * 60;
	}

	u64 minTimestep = this->GetMinimumTimeStep(); // seconds
	minTimestep /= 3600.0; // hours
	u64 _learnHourDelta = learnHourDelta; // 0.25 hours
	this->Lock();
	if (learned && learned->learned && learned->learnPeriod.z > 0) {
		_learnHourDelta = ::Max(_learnHourDelta, (u64)learned->learnPeriod.z);
		this->Unlock();
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Min(_learnHourDelta, (u64)2.0);
	}
	else if (learned && learned->learned && learned->learnPeriod.z <= 0) {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
	}
	else {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Min(_learnHourDelta, (u64)2.0);
	}


	float hr;
	u64 resolutionSeconds = _learnHourDelta * 60.0f * 60.0f; // x-seconds step to achieve the desired resolution
	resolutionSeconds = ::Max(resolutionSeconds, minTimestep);

	resolutionSeconds *= (1.0f - (splitSample / 100.0f)); // i.e. 20% greater number of samples to counter-act the samples removed by the learn/test operation.
	float numSamples = (end - start) / resolutionSeconds;

	if (numSamples > MaxNumSamples) {
		if ((numSamples * _learnHourDelta) < MaxNumSamples) {
			auto prevInterp = this->GetInterpolationType();
			this->SetInterpolationType(interpolation_t::IT_LINEAR);
			labels = GetTimeSeries(start, end, resolutionSeconds / _learnHourDelta);
			this->SetInterpolationType(prevInterp);
		}
		else if (((numSamples * _learnHourDelta) / 24.0f) < MaxNumSamples) {
			auto prevInterp = this->GetInterpolationType();
			this->SetInterpolationType(interpolation_t::IT_LINEAR);
			labels = GetTimeSeries(start, end, (resolutionSeconds / _learnHourDelta) * 24);
			this->SetInterpolationType(prevInterp);
		}
		else {
			auto prevInterp = this->GetInterpolationType();
			this->SetInterpolationType(interpolation_t::IT_LINEAR);
			labels = GetTimeSeries(start, end, (end - start) / MaxNumSamples);
			this->SetInterpolationType(prevInterp);
		}
	}
	else {
		auto prevInterp = this->GetInterpolationType();
		this->SetInterpolationType(interpolation_t::IT_LINEAR);
		labels = GetTimeSeries(start, end, resolutionSeconds);  // great!
		this->SetInterpolationType(prevInterp);
	}

	// develop exclusion list as we go.	
	cweeThreadedList<bool> exclusionList;

	this->Lock();
	cweeThreadedList<vec4> MachineLearnFeatures = machineLearn_features;
	this->Unlock();

	// user provided features
	for (auto& x : MachineLearnFeatures) {
		switch (static_cast<patternSource>((int)x.x)) {
		case patternSource::Parent: {
			if (Parent != nullptr) {
				cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
				std::pair<u64, type> tempHold;
				Parent->PreventDeletion(x.y);
				Parent->Lock();
				cweeBalancedPattern<type>* ptr = Parent->UnsafeRead(x.y);
				Parent->Unlock();
				if (ptr) {
					for (std::pair<u64, type>& y : labels) {
						tempHold.first = y.first; tempHold.second = (type)(ptr->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z), Parent));
						feature1.Append(tempHold);
					}
				}

				Parent->AllowDeletion(x.y);
				//bool ex = DetermineIfExcludeFeature(feature1);
				//if (!ex) 
				if (feature1.Num() > 0)
					features.Append(feature1);
			}
			break;
		}
		case patternSource::Global: {
			cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
			std::pair<u64, type> tempHold;
			Globals->PreventDeletion(x.y);
			Globals->Lock();
			auto ptr = Globals->UnsafeRead(x.y);
			Globals->Unlock();
			if (ptr) {
				for (std::pair<u64, type>& y : labels) {
					tempHold.first = y.first; tempHold.second = (type)(ptr->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z)
						//, Parent
					));
					feature1.Append(tempHold);
				}
			}
			Globals->AllowDeletion(x.y);
			//bool ex = DetermineIfExcludeFeature(feature1);
			//if (!ex) 
			if (feature1.Num() > 0)
				features.Append(feature1);
			break;
		}
		case patternSource::Scada: {
			cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
			std::pair<u64, type> tempHold;
			Scada->PreventDeletion(x.y);
			Scada->Lock();
			auto ptr = Scada->UnsafeRead(x.y);
			Scada->Unlock();
			if (ptr) {
				for (std::pair<u64, type>& y : labels) {
					tempHold.first = y.first; tempHold.second = (type)(ptr->GetMeasurement("value")->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z)
						//, Parent
					));
					feature1.Append(tempHold);
				}
			}
			Scada->AllowDeletion(x.y);
			//bool ex = DetermineIfExcludeFeature(feature1);
			//if (!ex) 
			if (feature1.Num() > 0)
				features.Append(feature1);
			break;
		}
		case patternSource::Customers: {
			cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
			std::pair<u64, type> tempHold;
			Customers->Lock();
			auto ptr = Customers->UnsafeRead(x.y);
			if (ptr) {
				for (std::pair<u64, type>& y : labels) {
					tempHold.first = y.first; tempHold.second = (type)(ptr->GetMeasurement("usage_gpm")->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z)
						// , Parent
					));
					feature1.Append(tempHold);
				}
			}
			Customers->Unlock();
			//bool ex = DetermineIfExcludeFeature(feature1);
			//if (!ex) 
			if (feature1.Num() > 0)
				features.Append(feature1);
			break;
		}
		}
	}

	// more complicated but corrected regression design based on M&V diff-diff concept
	if (MachineLearnFeatures.Num() <= 0 || features.Num() <= 0) {
		{
			// Hours
			for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
				std::vector < std::pair<u64, type>> feature1;
				for (auto& x : labels) {
					hr = this->GetHourOfDay(x.first);
					//cweeStr tempp;
					//tempp = cweeStr((time_t)x.first);
					int v = (int)(hr >= t_s && hr < (t_s + _learnHourDelta));
					std::pair<u64, type> t = std::make_pair(x.first, (type)(v));
					feature1.push_back(t);
				}
				features.Append(feature1);
			}
		}
		{
			// Day of Week
			bool excludeDays = false;
			{
				cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
				for (auto& x : labels) feature1.Append(std::make_pair(x.first, this->GetDayOfWeek(x.first)));
				excludeDays = DetermineIfExcludeFeature(feature1);
				exclusionList.Append(excludeDays);
			}

			// 0 automatically accounted for as the 'default'
			// 0
			if (!excludeDays) {
				for (int j = 0; j < 7; j++) {
					for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
						std::vector < std::pair<u64, type>> feature1;
						for (auto& x : labels) {
							hr = this->GetHourOfDay(x.first);
							//cweeStr tempp;
							//tempp = cweeStr((time_t)x.first);
							int v = (int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (this->GetDayOfWeek(x.first) == j));
							std::pair<u64, type> t = std::make_pair(x.first, (type)(v));
							feature1.push_back(t);
						}
						features.Append(feature1);
					}
				}
			}
		}
		{
			// Quarter of the Year
			bool excludeQuarters = false;
			{
				cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
				for (auto& x : labels) feature1.Append(std::make_pair(x.first,
					cweeMath::Floor(((float)this->GetMonthOfYear(x.first)) / 3.0f)
				));
				excludeQuarters = DetermineIfExcludeFeature(feature1);
				exclusionList.Append(excludeQuarters);
			}
			if (!excludeQuarters) {
				for (int j = 0; j < 4; j++) {
					cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
					for (auto& x : labels) {
						if (cweeMath::Floor(((float)this->GetMonthOfYear(x.first)) / 3.0f) == j)
							feature1.Append(std::make_pair(x.first, (type)1));
						else
							feature1.Append(std::make_pair(x.first, (type)0));
					}
					features.Append(feature1);
				}
			}
		}
	}

	// perform learning
	cweeML_learned_parameters attempt1; std::pair<vec2, vec2> fit1;
	attempt1 = cweeMachineLearning::Learn(labels, features, &fit1, splitSample); // actual learn activity - note that this is done async to actually accessing the pattern itself.
	attempt1.performance.x = fit1.first.x;
	attempt1.performance.y = fit1.first.y;
	attempt1.performance.z = fit1.second.x;
	attempt1.performance.w = fit1.second.y;
	attempt1.learnPeriod.x = start;
	attempt1.learnPeriod.y = end;
	attempt1.learnPeriod.z = _learnHourDelta;

	float mT = (float)this->GetMinimumDecimals();
	this->Lock(); {
		machineLearn_MinMaxRound.z = mT;
	} this->Unlock();

	mT = (float)this->GetMinValue();
	this->Lock(); {
		machineLearn_MinMaxRound.x = mT;
	} this->Unlock();

	mT = (float)this->GetMaxValue();
	this->Lock(); {
		machineLearn_MinMaxRound.y = mT;
	} this->Unlock();

	this->Lock();
	if (splitSample > 0) {
		if (learned) {
			if (attempt1.performance.w > learned->performance.y) { // Mean Squared Error is larger 		
				vec2 err = cweeMachineLearning::CalculateError(labels, cweeMachineLearning::Forecast(*learned, features, machineLearn_MinMaxRound.z)); // adjusted previous error
				if (attempt1.performance.w > err.y) { // the older learned object is better
					this->Unlock();
					return vec2(fit1.first.x, fit1.second.x); // return early - this was not approved for use. 
				}
			}
		}
	}
	else {
		if (learned) {
			if (attempt1.performance.y > learned->performance.y) { // Mean Squared Error is larger 		
				vec2 err = cweeMachineLearning::CalculateError(labels, cweeMachineLearning::Forecast(*learned, features, machineLearn_MinMaxRound.z)); // adjusted previous error
				if (attempt1.performance.y > err.y) { // the older learned object is better
					this->Unlock();
					return vec2(fit1.first.x, fit1.second.x); // return early - this was not approved for use. 
				}
			}
		}
	}
	this->Unlock();

	this->Lock(); {
		if (learned) *learned = attempt1; // save results	
		exclusion_list = exclusionList;
		machineLearn_MAD = vec3(1, 0, 0); // reset this! We just finished learning and this shouldn't be necessary anymore.
	} this->Unlock();
	return vec2(fit1.first.x, fit1.second.x); // return the fit
};;

template< class type >
INLINE type								cweeBalancedPattern<type>::Forecast_Value_Automated(const u64& time, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent) const {
	float hr; int exclusionCheck = 0;

	u64 _learnHourDelta = learnHourDelta;
	this->Lock();
	if (learned && learned->learned && learned->learnPeriod.z > 0) {
		_learnHourDelta = ::Max(_learnHourDelta, (u64)learned->learnPeriod.z);
		this->Unlock();
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}
	else if (learned && learned->learned && learned->learnPeriod.z <= 0) {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
	}
	else {
		this->Unlock();
		u64 minTimestep = this->GetMinimumTimeStep();
		_learnHourDelta = learnHourDelta;
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}

	cweeThreadedList < float > features(cweeMath::max(16, (int)(learnHourMaxTime / _learnHourDelta))); {
		this->Lock();
		cweeThreadedList<vec4> MachineLearnFeatures = machineLearn_features;
		this->Unlock();

		// more complicated but corrected regression design based on M&V diff-diff concept
		if (MachineLearnFeatures.Num() <= 0) {
			if (1) { // Hours
				hr = GetHourOfDay(time);
				for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					features.Append((int)(hr >= t_s && hr < (t_s + _learnHourDelta)));
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) {
				// Day of Week
				for (int j = 0; j < 7; j++) {
					for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
						features.Append((int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (GetDayOfWeek(time) == j)));
					}


					//features.Append((int)(GetDayOfWeek(time) == j));
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) {
				// Quarter of the Year
				for (int j = 0; j < 4; j++) {
					//for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					//	features.Append((int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (cweeMath::Floor(((float)this->GetMonthOfYear(time)) / 3.0f) == j)));
					//}


					features.Append((int)(cweeMath::Floor(((float)this->GetMonthOfYear(time)) / 3.0f) == j));
				}
			}
		}

		{
			float ts(0);
			for (auto& x : MachineLearnFeatures) {
				//if (!DetermineIfExcludeFeature(exclusionCheck)) {
				switch (static_cast<patternSource>((int)x.x)) {
				case patternSource::Parent: {
					if (Parent != nullptr) {
						Parent->PreventDeletion(x.y);
						Parent->Lock();
						auto ptr = Parent->UnsafeRead(x.y);
						Parent->Unlock();
						if (ptr) {
							ts = (float)ptr->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z), Parent);
						}
						Parent->AllowDeletion(x.y);
						features.Append(ts);
					}
					break;
				}
				case patternSource::Global: {
					Globals->PreventDeletion(x.y);
					Globals->Lock();
					auto ptr = Globals->UnsafeRead(x.y);
					Globals->Unlock();
					if (ptr) {
						ts = ((float)ptr->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z)
							// , Parent
						));
					}
					Globals->AllowDeletion(x.y);

					features.Append(ts);
					break;
				}
				case patternSource::Scada: {
					Scada->PreventDeletion(x.y);
					Scada->Lock();
					auto ptr = Scada->UnsafeRead(x.y);
					Scada->Unlock();
					if (ptr) {
						ts = ((float)ptr->GetMeasurement("value")->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z)
							//, Parent
						));
					}
					Scada->AllowDeletion(x.y);

					features.Append(ts);
					break;
				}
				case patternSource::Customers: {
					Customers->Lock();
					auto ptr = Customers->UnsafeRead(x.y);
					if (ptr) {
						ts = ((float)ptr->GetMeasurement("usage_gpm")->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z)
							//, Parent
						));
					}
					Customers->Unlock();
					features.Append(ts);
					break;
				}
				}
				//}
			}
		}

	}

	this->Lock();
	type out;
	out = 0.0f;
	if (learned) {
		out = (type)cweeMachineLearning::Forecast(*learned, features, machineLearn_MinMaxRound.z);
	}
	out = (type)cweeMath::Fmax(cweeMath::Fmin((float)out, machineLearn_MinMaxRound.y), machineLearn_MinMaxRound.x);
	if (machineLearn_MAD != vec3(1, 0, 0)) { out *= (type)machineLearn_MAD[0]; out += (type)machineLearn_MAD[1]; }
	this->Unlock();
	return out;
};;

template< class type >
INLINE cweeThreadedList< std::pair<u64, type> >	cweeBalancedPattern<type>::Forecast_Series_Automated(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent) const {
	float hr;  int exclusionCheck = 0;
	u64 realStep = cweeMath::Fmax(1, timeStep);

	u64 _learnHourDelta = learnHourDelta;
	this->Lock();
	if (learned && learned->learned && learned->learnPeriod.z > 0) {
		_learnHourDelta = ::Max(_learnHourDelta, (u64)learned->learnPeriod.z);
		this->Unlock();
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}
	else if (learned && learned->learned && learned->learnPeriod.z <= 0) {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
	}
	else {
		this->Unlock();
		u64 minTimestep = this->GetMinimumTimeStep();
		_learnHourDelta = learnHourDelta;
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}

	cweeThreadedList< std::pair<u64, type> > feature1(((timeEnd - timeStart) / realStep));
	cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > features; {
		this->Lock();
		cweeThreadedList<vec4> MachineLearnFeatures = machineLearn_features;
		this->Unlock();


		// more complicated but corrected regression design based on M&V diff-diff concept
		if (MachineLearnFeatures.Num() <= 0) {
			if (1) { // Hours
				for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					feature1.Clear();
					for (u64 t = timeStart; t < timeEnd; t += realStep) {
						hr = GetHourOfDay(t);
						feature1.Append(std::make_pair(t, (type)(
							(int)(hr >= t_s && hr < (t_s + _learnHourDelta))
							)));
					}
					features.Append(feature1);
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) { // Day of Week
				for (int j = 0; j < 7; j++) {
					for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
						feature1.Clear();
						for (u64 t = timeStart; t < timeEnd; t += realStep) {
							hr = GetHourOfDay(t);
							feature1.Append(std::make_pair(t, (type)(
								(int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (GetDayOfWeek(t) == j))
								)));
						}
						features.Append(feature1);
					}

					//feature1.Clear();
					//for (u64 t = timeStart; t < timeEnd; t += realStep)
					//	feature1.Append(std::make_pair(t, (type)(
					//		(int)(GetDayOfWeek(t) == j)
					//		)));
					//features.Append(feature1);
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) { // Quarter of the Year
				// 0 automatically accounted for as the 'default'
				// 0
				for (int j = 0; j < 4; j++) {
					//for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					//	feature1.Clear();
					//	for (u64 t = timeStart; t < timeEnd; t += realStep) {
					//		hr = GetHourOfDay(t);
					//		feature1.Append(std::make_pair(t, (type)(
					//			(int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (cweeMath::Floor(((float)this->GetMonthOfYear(t)) / 3.0f) == j))
					//			)));
					//	}
					//	features.Append(feature1);
					//}

					feature1.Clear();
					for (u64 t = timeStart; t < timeEnd; t += realStep)
						feature1.Append(std::make_pair(t, (type)(
							(int)(cweeMath::Floor(((float)this->GetMonthOfYear(t)) / 3.0f) == j)
							)));
					features.Append(feature1);
				}
			}
		}
		{
			for (auto& x : MachineLearnFeatures) {
				//if (!DetermineIfExcludeFeature(exclusionCheck)) {
				switch (static_cast<patternSource>((int)x.x)) {
				case patternSource::Parent: {
					if (Parent != nullptr) {
						Parent->PreventDeletion(x.y);
						Parent->Lock();
						cweeBalancedPattern<type>* ptr = Parent->UnsafeRead(x.y);
						Parent->Unlock();
						if (ptr) {
							switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
							case ::TrainedModel_Modifier::None: {
								feature1 = ptr->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::DayOfWeek: {
								feature1 = ptr->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::HourOfDay: {
								feature1 = ptr->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Velocity: {
								feature1 = ptr->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Acceleration: {
								feature1 = ptr->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::MovingAverage: {
								feature1 = ptr->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Normalize: {
								feature1 = ptr->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							}
						}
						Parent->AllowDeletion(x.y);
					}
					break;
				}
				case patternSource::Global: {
					Globals->PreventDeletion(x.y);
					Globals->Lock();
					auto ptr = Globals->UnsafeRead(x.y);
					Globals->Unlock();
					if (ptr) {
						switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
						case ::TrainedModel_Modifier::None: {
							feature1 = ptr->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::DayOfWeek: {
							feature1 = ptr->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::HourOfDay: {
							feature1 = ptr->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Velocity: {
							feature1 = ptr->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Acceleration: {
							feature1 = ptr->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::MovingAverage: {
							feature1 = ptr->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Normalize: {
							feature1 = ptr->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						}
					}
					Globals->AllowDeletion(x.y);

					break;
				}
				case patternSource::Scada: {
					Scada->PreventDeletion(x.y);
					Scada->Lock();
					auto ptr = Scada->UnsafeRead(x.y);
					Scada->Unlock();
					if (ptr) {
						switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
						case ::TrainedModel_Modifier::None: {
							feature1 = ptr->GetMeasurement("value")->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::DayOfWeek: {
							feature1 = ptr->GetMeasurement("value")->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::HourOfDay: {
							feature1 = ptr->GetMeasurement("value")->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Velocity: {
							feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Acceleration: {
							feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::MovingAverage: {
							feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Normalize: {
							feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						}
					}
					Scada->AllowDeletion(x.y);
					break;
				}
				case patternSource::Customers: {
					Customers->Lock();
					auto ptr = Customers->UnsafeRead(x.y);
					if (ptr) {
						switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
						case ::TrainedModel_Modifier::None: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::DayOfWeek: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::HourOfDay: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Velocity: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Acceleration: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::MovingAverage: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						case ::TrainedModel_Modifier::Normalize: {
							feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
							break; }
						}
					}
					Customers->Unlock();
					break;
				}
				}
				features.Append(feature1);
				//}
			}
		}
	}

	cweeBalancedPattern<type> out;
	out.SetInterpolationType(this->GetInterpolationType()); out.SetBoundaryType(this->GetBoundaryType());

	this->Lock(); {
		if (learned) {
			for (auto& x : cweeMachineLearning::Forecast(*learned, features, machineLearn_MinMaxRound.z)) {
				out.AddUniqueValue(x.first, (float)x.second, false);
			}
		}
		out.ClampValues((type)machineLearn_MinMaxRound.x, (type)machineLearn_MinMaxRound.y);
		if (machineLearn_MAD != vec2(1, 0)) { out *= (type)machineLearn_MAD[0]; out += (type)machineLearn_MAD[1]; }
	} this->Unlock();

	return out.GetTimeSeries(timeStart, timeEnd, realStep);
};;

template< class type >
INLINE  void								cweeBalancedPattern<type>::Lock() const {
	this->lock.Lock();
};

template< class type >
INLINE  void								cweeBalancedPattern<type>::Unlock() const {
	this->lock.Unlock();
};

template< class type >
INLINE bool								cweeBalancedPattern<type>::GetChanged() const {
	bool out;
	Lock();
	out = changed;
	Unlock();
	return out;
};

template< class type >
INLINE void								cweeBalancedPattern<type>::SetChanged(bool in) const {
	Lock();
	changed = in;
	Unlock();
};

template< class type >
INLINE void								cweeBalancedPattern<type>::InsertPair(const u64& time, const type& valueIN, bool useMAD, bool unique) {
	if (useMAD == true) {
		type value;
		value = 0;

		Lock();
		value = valueIN * MAD.x + MAD.y;
#ifdef  usePhmapBtree
		container[time] = value;
#else
		if (container.GetNodeCount() + 1 > granularity) {
			granularity = (cweeMath::max(granularity, container.GetNodeCount())) * GRANULARITY_SCALER;
			container.Reserve(granularity);
		}
		container.Add(value, time, unique);
#endif
		changed = true;
		Unlock();
	}
	else
	{
		Lock();
#ifdef  usePhmapBtree
		container[time] = valueIN;
#else
		if (container.GetNodeCount() + 1 > granularity) {
			granularity = (cweeMath::max(granularity, container.GetNodeCount())) * GRANULARITY_SCALER;
			container.Reserve(granularity);
		}
		container.Add(valueIN, time, unique);
#endif
		changed = true;
		Unlock();
	}
};

template< class type >
INLINE bool								cweeBalancedPattern<type>::IsLooped() const {
	bool out = false;
	Lock();
	out = (boundaryType == boundary_t::BT_LOOP);
	Unlock();
	return out;
};

template< class type >
INLINE bool								cweeBalancedPattern<type>::IsClamped() const {
	bool out = false;
	Lock();
	out = (boundaryType == boundary_t::BT_CLAMPED);
	Unlock();
	return out;
};

template< class type >
INLINE u64									cweeBalancedPattern<type>::LoopedTime(const u64& t, bool forceLoop) const {
	if (IsLooped() || forceLoop) {
		u64 minTime, maxTime, len, currentTime;
		minTime = GetMinTime();
		if (cweeMath::Fabs(t - minTime) < 1.01) return t;
		maxTime = GetMaxTime();
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

template< class type >
INLINE u64									cweeBalancedPattern<type>::ClampedTime(const u64& t) const {
	if (IsClamped()) {
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

template< class type >
INLINE cweeStr									cweeBalancedPattern<type>::SerializeTime(u64 t) {
	constexpr double div = 0.001;
	t /= div;
	return cweeStr(t);
};

template< class type >
INLINE u64										cweeBalancedPattern<type>::DeserializeTime(const cweeStr& t) {
	constexpr double mul = 1 / 0.001;
	u64 out = (u64)t;
	out *= mul;
	return out;
};

template< class type >
INLINE void										cweeBalancedPattern<type>::Basis(const u64& t, u64* bvals) const {
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

	u64 s; cweeBalancedTreeNode<type, u64>* index1 = nullptr;
	Lock();
	switch (interpolationType) {
	case interpolation_t::IT_RIGHT_CLAMP: {
		bvals[0] = 0;
		bvals[1] = 0;
		bvals[2] = 1;
		bvals[3] = 0;

		break;
	}
	case interpolation_t::IT_SPLINE: {
#ifdef  usePhmapBtree
		auto f = container.lower_bound(t);
		if (f != container.end()) {
			s = f->first;
			f++;
			if (f != container.end()) {
				if (s <= t && f->first >= t) {
					s = (u64)(t - s) / (f->first - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
					bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
					bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
					bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
				}
				else {
					// something went wrong - snap left. 
					s = (u64)(s - s) / (f->first - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
					bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
					bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
					bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
				}
			}
			else {
				bvals[0] = 0;
				bvals[1] = 1;
				bvals[2] = 0;
				bvals[3] = 0;
			}
		}
		else {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;
		}
#else
		index1 = container.NodeFindLargestSmallerEqual(t);
		if (index1) {
			s = index1->key;
			index1 = container.GetNextLeaf(index1);
			if (index1) {
				if (s <= t && index1->key >= t) {
					s = (u64)(t - s) / (index1->key - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
					bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
					bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
					bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
				}
				else {
					// something went wrong - snap left. 
					s = (u64)(s - s) / (index1->key - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
					bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
					bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
					bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
				}
			}
			else {
				bvals[0] = 0;
				bvals[1] = 1;
				bvals[2] = 0;
				bvals[3] = 0;
			}
		}
		else {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;
		}
#endif
		break;
	}
	case interpolation_t::IT_LINEAR: {

#ifdef  usePhmapBtree
		auto f = container.lower_bound(t);
		if (f != container.end()) {
			s = f->first;
			f++;
			if (f != container.end()) {
				if (s <= t && f->first >= t) {
					s = (u64)(t - s) / (f->first - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = 0;
					bvals[1] = (u64)(1.0f - s);
					bvals[2] = s;
					bvals[3] = 0;

				}
				else {
					// something went wrong - snap left. 
					s = (u64)(s - s) / (f->first - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = 0;
					bvals[1] = (u64)(1.0f - s);
					bvals[2] = s;
					bvals[3] = 0;
				}
			}
			else {
				bvals[0] = 0;
				bvals[1] = 1;
				bvals[2] = 0;
				bvals[3] = 0;
			}
		}
		else {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;
		}

#else
		index1 = container.NodeFindLargestSmallerEqual(t);
		if (index1) {
			s = index1->key;
			index1 = container.GetNextLeaf(index1);
			if (index1) {
				if (s <= t && index1->key >= t) {
					s = (u64)(t - s) / (index1->key - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = 0;
					bvals[1] = (u64)(1.0f - s);
					bvals[2] = s;
					bvals[3] = 0;

				}
				else {
					// something went wrong - snap left. 
					s = (u64)(s - s) / (index1->key - s);
					if (!::isfinite(s)) s = 0;

					bvals[0] = 0;
					bvals[1] = (u64)(1.0f - s);
					bvals[2] = s;
					bvals[3] = 0;
				}
			}
			else {
				bvals[0] = 0;
				bvals[1] = 1;
				bvals[2] = 0;
				bvals[3] = 0;
			}
		}
		else {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;
		}

#endif

		break;
	}
	default:
	case interpolation_t::IT_LEFT_CLAMP: {
		bvals[0] = 0;
		bvals[1] = 1;
		bvals[2] = 0;
		bvals[3] = 0;

		break;
	}
	}
	Unlock();
};

template< class type >
INLINE void									cweeBalancedPattern<type>::BasisFirstDerivative(const u64& t, u64* bvals) const {
	cweeBalancedTreeNode<type, u64>* index1 = nullptr;
	u64 s;

#ifdef  usePhmapBtree
	auto f = container.lower_bound(t);
	if (f != container.end()) {
		s = f->first;
		f++;
		if (f != container.end()) {
			if (s <= t && f->first >= t) {
				s = (u64)(t - s) / (f->first - s);
				if (!::isfinite(s)) s = 0;

				bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
				bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
				bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
				bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
			}
			else {
				s = (u64)(s - s) / (f->first - s);
				if (!::isfinite(s)) s = 0;

				bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
				bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
				bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
				bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s

			}
		}
	}

#else
	index1 = container.NodeFindLargestSmallerEqual(t);
	s = index1->key;
	index1 = container.GetNextLeaf(index1);
	if (s <= t && index1->key >= t) {
		s = (u64)(t - s) / (index1->key - s);
		if (!::isfinite(s)) s = 0;

		bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
		bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
		bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
		bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
	}
	else {
		s = (u64)(s - s) / (index1->key - s);
		if (!::isfinite(s)) s = 0;

		bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
		bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
		bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
		bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s

	}
#endif

};

template< class type >
INLINE void									cweeBalancedPattern<type>::BasisSecondDerivative(const u64& t, u64* bvals) const {
	cweeBalancedTreeNode<type, u64>* index1 = nullptr;
	u64 s;

#ifdef  usePhmapBtree
	auto f = container.lower_bound(t);
	if (f != container.end()) {
		s = f->first;
		f++;
		if (f != container.end()) {
			if (s <= t && f->first >= t) {
				s = (u64)(t - s) / (f->first - s);
				if (!::isfinite(s)) s = 0;

				bvals[0] = -3.0f * s + 2.0f;
				bvals[1] = 9.0f * s - 5.0f;
				bvals[2] = -9.0f * s + 4.0f;
				bvals[3] = 3.0f * s - 1.0f;
			}
			else {
				s = (u64)(s - s) / (f->first - s);
				if (!::isfinite(s)) s = 0;

				bvals[0] = -3.0f * s + 2.0f;
				bvals[1] = 9.0f * s - 5.0f;
				bvals[2] = -9.0f * s + 4.0f;
				bvals[3] = 3.0f * s - 1.0f;
			}
		}
	}

#else
	index1 = container.NodeFindLargestSmallerEqual(t);
	s = index1->key;
	index1 = container.GetNextLeaf(index1);
	if (s <= t && index1->key >= t) {
		s = (u64)(t - s) / (index1->key - s);
		if (!::isfinite(s)) s = 0;

		bvals[0] = -3.0f * s + 2.0f;
		bvals[1] = 9.0f * s - 5.0f;
		bvals[2] = -9.0f * s + 4.0f;
		bvals[3] = 3.0f * s - 1.0f;
	}
	else {
		s = (u64)(s - s) / (index1->key - s);
		if (!::isfinite(s)) s = 0;

		bvals[0] = -3.0f * s + 2.0f;
		bvals[1] = 9.0f * s - 5.0f;
		bvals[2] = -9.0f * s + 4.0f;
		bvals[3] = 3.0f * s - 1.0f;

	}
#endif

};

template< class type >
INLINE  bool									cweeBalancedPattern<type>::DetermineIfExcludeFeature(const cweeThreadedList< std::pair<u64, type> >& data) {
	cweeThreadedList<type> test(data.Num() + 2);
	for (auto& x : data)
	{
		test.AddUnique(x.second);
		if (test.Num() > 1) return false;
	}
	return true;
};

template< class type >
INLINE bool									cweeBalancedPattern<type>::DetermineIfExcludeFeature(int& check) const {
	bool toReturn = false;
	this->Lock();
	if (exclusion_list.Num() > check) toReturn = exclusion_list[check];
	this->Unlock();
	check++;
	return toReturn;
};

template< class type >
INLINE  cweeTime							cweeBalancedPattern<type>::getLocalTime(const u64& time) {
	return fileSystem->localtime(time);
};


#endif