
#ifndef __BPATTERN_H__
#define __BPATTERN_H__

static constexpr u64 learnHourDelta = 0.25; // 0.5 = 30 min
static constexpr u64 learnHourMaxTime = 24; // 24 hours

template< class type >
class cweeBalancedCurve : public cweeBaseObj {
public:
	cweeBalancedCurve() {
		IncrementLevel();
	};
	cweeBalancedCurve(const cweeBalancedCurve& source) {
		Copy(source);
		IncrementLevel();
	};
	cweeBalancedCurve(const cweeThreadedList<std::pair<u64, type>>& data) {
		for (auto& x : data) {
			AddValue(x.first, x.second);
		}
		IncrementLevel();
	};
	~cweeBalancedCurve() {
		Clear();
	};

	virtual cweeStr		ToString() const {
		cweeStr out;

		for (auto& x : GetKnotSeries()) {
			out.AddToDelimiter(cweeStr::printf("<%s, %s>", cweeStr::ToString(x.first).c_str(), cweeStr::ToString(x.second).c_str()), ", ");
		}

		out.Insert("[", 0);
		out.Append("]");

		return out;
	};

	virtual void		Deserialize(cweeStr& inbound) {
		Clear();

		cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
		delim.ReplaceInline(" ", "");
		cweeParser x(inbound, delim, true);
		inbound.Clear();
		{
			Lock();
			cweeParser y(x[0], "|", true);
			int finder(-1); cweeStr left, right; u64 a; type b; bool t = false;
			u64 prevTime = 0;
			for (auto& knot : y) {
				finder = knot.Find(',');
				if (finder != -1) {
					knot.Mid(0, finder, left);
					knot.Mid(finder + 1, knot.Length(), right);

					a = DeserializeTime(left) + prevTime;

					if (right.IsEmpty()) b = type();
					else b = type(right);

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
	};
	virtual cweeStr		Serialize() const {
		cweeStr out; cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
		delim.ReplaceInline(" ", "");
		{
			cweeStr knots; cweeStr t;
			{
				u64 prevTime = 0;
				for (auto& x : GetKnotSeries()) {
					t = SerializeTime(x.first - prevTime);
					t += ",";
					t += cweeStr::ToString(x.second);
					knots.AddToDelimiter(t, "|");
					prevTime = x.first;
				}
			}
			out.AddToDelimiter(knots, delim); // 0
		}
		return out;
	};
	virtual void		Update() {

	};

	auto&				UnsafeGetValues() const {
		return container; // assumes already locked
	};
	cweeBalancedTreeNode<type, u64>* UnsafeGetValue(const u64& time) const {
		return UnsafeGetValues().NodeFindLargestSmallerEqual(time);
	};
	cweeThreadedList<cweeBalancedTreeNode<type, u64>*> UnsafeGetKnotSeries() const {
		cweeThreadedList<cweeBalancedTreeNode<type, u64>*> out;
		for (cweeBalancedTreeNode<type, u64>& x : UnsafeGetValues()) {
			out.Append(&x);
		}
		return out;
	};

	virtual void		AddValue(const u64& time, const type& valueIN) {
		InsertPair(time, valueIN, false);
	};;
	virtual void		AddUniqueValue(const u64& time, const type& valueIN) { // slower with guarrantee of uniqueness 
		InsertPair(time, valueIN, true);
	};

	void				RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo) {
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
	};;
	virtual void		Clear() {
		Lock();
#ifndef  usePhmapBtree
		container.Clear();
#else
		container.clear();
#endif
		Unlock();
	};

	virtual u64			GetAvgTime() const {
		float out(0);
		int num(0);

		Lock();
		for (auto& x : container) {
			num++;
			out -= (decltype(out))(out / (float)num);
#ifndef  usePhmapBtree
			out += (decltype(out))(x.key / (float)num);
#else
			out += (decltype(out))(x.first / (float)num);
#endif
		}
		Unlock();

		return (u64)out;
	};;
	u64					GetMaxTime(void) const {
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
	};
	u64					GetMinTime(void) const {
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

	};

	int					GetNumValues() const {
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

	void				ShiftTime(const u64& deltaTime) {
		Lock();
		for (auto& x : container) {
#ifndef  usePhmapBtree
			x.key += deltaTime;
#else
			throw("phMap does not support key shifting");
			// x.first += deltaTime;
#endif
		}
		Unlock();
	};

	virtual void		Copy(const cweeBalancedCurve& copy, const u64& timeStart = -std::numeric_limits <u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max()) {
		Lock(); copy.Lock();
		container = copy.container;
		copy.Unlock(); Unlock();
	};

	/*! Copy constructor */
	void												operator=(const cweeBalancedCurve<type>& source) {
		Copy(source);
	};

	/*!	Copy constructor, data only	*/
	void												operator=(const cweeThreadedList<std::pair<u64, type>>& data) {
		this->Clear();
		for (auto& x : data) {
			AddValue(x.first, x.second);
		}
	};

	operator cweeThreadedList<std::pair<u64, type>>() const {
		return GetKnotSeries();
	};
	operator cweeThreadedList<std::pair<u64, type>>() {
		return GetKnotSeries();
	};

	/*!
	Return true if the knot collections within the LHS and RHS are NOT equal
	*/
	friend bool									operator!=(const cweeBalancedCurve<type>& a, const cweeBalancedCurve<type>& b) {
		return !operator==(a, b);
	};;
	/*!
	Return true if the effective knot collections within the LHS and RHS are equal (can have fewer or more knots - but if the Timeseries is
	effectively the same, then it is allowed). Result *is* dependant on the interpolation type.
	*/
	friend bool									operator==(const cweeBalancedCurve<type>& a, const cweeBalancedCurve<type>& b) {
		bool out = true;
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			if (x.second != b.GetCurrentValue(x.first)) {
				out = false;
				break;
			}
#else
			if (*x.object != b.GetCurrentValue(x.key)) {
				out = false;
				break;
			}
#endif
		}
		a.Unlock();

		if (!out) return out;

		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			if (x.second != a.GetCurrentValue(x.first)) {
				out = false;
				break;
			}
#else
			if (*x.object != a.GetCurrentValue(x.key)) {
				out = false;
				break;
			}
#endif
		}
		b.Unlock();

		return out;
	};;

	cweeThreadedList<std::pair<u64, type>>		GetKnotSeries(const u64& timeStart = -std::numeric_limits <u64>::max(), const u64& timeEnd = std::numeric_limits <u64>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);


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
		
		return out;
	};
	cweeThreadedList<std::pair<u64, type>>		GetReversedKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits <u64>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<std::pair<u64, type>> out(numKnots + 16);
		
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
		
		return out;
	};

	cweeThreadedList<type>						GetValueKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<type> out(numKnots + 16);

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
		
		return out;
	};

	void										Lock() const {
		this->lock.Lock();
	};
	void										Unlock() const {
		this->lock.Unlock();
	};

protected:
	void										InsertPair(const u64& time, const type& valueIN, bool unique = true) {
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
		Unlock();		
	};
	static cweeStr								SerializeTime(u64 t) {
		constexpr double div = 0.001;
		t /= div;
		return cweeStr::ToString(t);
	};
	static u64									DeserializeTime(const cweeStr& t) {
		constexpr double mul = 1 / 0.001;
		u64 out = (u64)t;
		out *= mul;
		return out;
	};

private:
#ifndef  usePhmapBtree
	mutable cweeBalancedTree< type, u64, 10>	container;
#else
	mutable phmap::btree_map< u64, type >		container;
#endif
	int											granularity = 16;
	mutable cweeSysMutex						lock;
};

template< class type >
class cweeBalancedPattern : public cweeBaseObj {
public:
	cweeBalancedPattern();
	cweeBalancedPattern(const cweeBalancedPattern& source);
	cweeBalancedPattern(const cweeThreadedList<std::pair<u64, type>>& data);
	~cweeBalancedPattern();

	virtual cweeStr		ToString() const;
	virtual void		Deserialize(cweeStr& inbound);
	virtual cweeStr		Serialize(int percentToRemove = -1) const;
	virtual void		Update();

	virtual cweeStr		GetName(void) const;
	virtual void		SetName(const cweeStr& newName);

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

	virtual int			GetGranularity(void) const {
		int out;

		Lock();
		out = granularity;
		Unlock();

		return out;
	};
	virtual void		SetGranularity(int newNum) {
		Lock();
		granularity = newNum;
		Unlock();
	};

	auto&				UnsafeGetValues() const;
	cweeBalancedTreeNode<type, u64>* UnsafeGetValue(const u64& time) const {
		return UnsafeGetValues().NodeFindLargestSmallerEqual(time);
	};
	cweeThreadedList<cweeBalancedTreeNode<type, u64>*> UnsafeGetKnotSeries(const u64& t0 = -std::numeric_limits < u64>::max(), const u64& t1 = std::numeric_limits < u64>::max()) const {
		cweeThreadedList<cweeBalancedTreeNode<type, u64>*> out;
		for (cweeBalancedTreeNode<type, u64>& x : UnsafeGetValues()) {
			if (x.key >= t0 && x.key <= t1) {
				out.Append(&x);
			}
		}
		return out;
	};


	virtual void		SetCharacteristic(const value_t& in);
	virtual value_t		GetCharacteristic() const;
	virtual void		SetMeasurement(const measurement_t& in);
	virtual measurement_t GetMeasurement() const;
	virtual void		SetBoundaryType(const boundary_t& bt);
	virtual boundary_t	GetBoundaryType() const;

	virtual void		SetInterpolationType(const interpolation_t& it);
	virtual interpolation_t	GetInterpolationType() const;

	virtual void		SetMAD(float multiply_x, float then_add_to_x);
	virtual void		SetMAD(const vec3& in);
	virtual vec3		GetMAD() const;

	virtual void		AddValue(const u64& time, const type& valueIN, bool useMAD = true);
	virtual void		AddUniqueValue(const u64& time, const type& valueIN, bool useMAD = true);

	virtual void		RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo);
	virtual void		Clear();

	virtual float		GetMinimumDecimals() const;
	virtual u64			GetMinimumTimeStep() const;

	virtual type		GetMinValue() const;
	virtual type		GetMaxValue() const;
	virtual type		GetAvgValue() const;

	virtual type		GetMinValue(const u64& start, const u64& end) const;
	virtual type		GetMaxValue(const u64& start, const u64& end) const;
	virtual type		GetAvgValue(const u64& start, const u64& end) const;

	virtual u64			GetAvgTime() const;
	virtual u64			GetMaxTime(void) const;
	virtual u64			GetMinTime(void) const;

	virtual int			GetNumValues() const;

	virtual void		ShiftTime(const u64& deltaTime);
	virtual void		Translate(const type& translation);

	virtual void		Copy(const cweeBalancedPattern& copy, const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max());

	/*! Copy constructor */
	void												operator=(const cweeBalancedPattern<type>& source);

	/*!	Copy constructor, data only	*/
	void												operator=(const cweeThreadedList<std::pair<u64, type>>& data);

	/*!	Replace pattern with value from a single float	*/
	void												operator=(float data);

	operator cweeThreadedList<std::pair<u64, type>>() const {
		return GetKnotSeries();
	};;
	operator cweeThreadedList<std::pair<u64, type>>() {
		return GetKnotSeries();
	};;

	friend cweeBalancedPattern<type>			operator*(const cweeBalancedPattern<type>& a, float b) {
		cweeBalancedPattern<type> result(a);
		result *= b;
		return result;
	};;
	friend cweeBalancedPattern<type>			operator*(float b, const cweeBalancedPattern<type>& a) {
		cweeBalancedPattern<type> result(a);
		result *= b;
		return result;
	};;
	friend cweeBalancedPattern<type>			operator/(const cweeBalancedPattern<type>& a, float b) {
		cweeBalancedPattern<type> result(a);
		result /= b;
		return result;
	};;
	friend cweeBalancedPattern<type>			operator/(float b, const cweeBalancedPattern<type>& a) {
		cweeBalancedPattern<type> result(a);
		result.Lock();
		for (auto& x : result.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			x.second = b / (x.second + cweeMath::EPSILON);
#else
			*x.object = b / (*x.object + cweeMath::EPSILON);
#endif
		}
		result.Unlock();
		return result;
	};;
	friend cweeBalancedPattern<type>			operator+(const cweeBalancedPattern<type>& a, float b) {
		cweeBalancedPattern<type> result(a);
		result += b;
		return result;
	};;
	friend cweeBalancedPattern<type>			operator+(float b, const cweeBalancedPattern<type>& a) {
		cweeBalancedPattern<type> result(a);
		result += b;
		return result;
	};;
	friend cweeBalancedPattern<type>			operator-(const cweeBalancedPattern<type>& a, float b) {
		cweeBalancedPattern<type> result(a);
		result -= b;
		return result;
	};;
	friend cweeBalancedPattern<type>			operator-(float b, const cweeBalancedPattern<type>& a) {
		// b - a = -1(a-b);
		cweeBalancedPattern<type> result(a);
		result -= b;
		result *= -1;
		return result;
	};;
	cweeBalancedPattern<type>& operator*=(float a);
	cweeBalancedPattern<type>& operator/=(float a);
	cweeBalancedPattern<type>& operator+=(float a);
	cweeBalancedPattern<type>& operator-=(float a);

	friend cweeBalancedPattern<type>			operator*(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		cweeBalancedPattern<type> result;

		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, x.second * b.GetCurrentValue(x.first));
#else
			result.AddUniqueValue(x.key, *x.object * b.GetCurrentValue(x.key));
#endif
		}
		a.Unlock();

		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, x.second * a.GetCurrentValue(x.first));
#else
			result.AddUniqueValue(x.key, *x.object * a.GetCurrentValue(x.key));
#endif
		}
		b.Unlock();

		result.RemoveUnnecessaryKnots();
		return result;
	};
	friend cweeBalancedPattern<type>			operator/(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		cweeBalancedPattern<type> result;

		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, x.second / (b.GetCurrentValue(x.first) + cweeMath::EPSILON));
#else
			result.AddUniqueValue(x.key, (*x.object) / (b.GetCurrentValue(x.key) + cweeMath::EPSILON));
#endif
		}
		a.Unlock();

		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) / (x.second + cweeMath::EPSILON));
#else
			result.AddUniqueValue(x.key, a.GetCurrentValue(x.key) / (*x.object + cweeMath::EPSILON));
#endif
		}
		b.Unlock();

		result.RemoveUnnecessaryKnots();
		return result;
	};;
	friend cweeBalancedPattern<type>			operator+(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		cweeBalancedPattern<type> result;

		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, x.second + b.GetCurrentValue(x.first));
#else
			result.AddUniqueValue(x.key, (*x.object) + b.GetCurrentValue(x.key));
#endif
		}
		a.Unlock();

		b.Lock();
		for (auto& x : b.UnsafeGetValues())
		{
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) + x.second);
#else
			result.AddUniqueValue(x.key, a.GetCurrentValue(x.key) + (*x.object));
#endif
		}
		b.Unlock();

		result.RemoveUnnecessaryKnots();
		return result;
	};;
	friend cweeBalancedPattern<type>			operator-(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		cweeBalancedPattern<type> result;

		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, x.second - b.GetCurrentValue(x.first));
#else
			result.AddUniqueValue(x.key, (*x.object) - b.GetCurrentValue(x.key));
#endif
		}
		a.Unlock();

		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			result.AddUniqueValue(x.first, a.GetCurrentValue(x.first) - x.second);
#else
			result.AddUniqueValue(x.key, a.GetCurrentValue(x.key) - (*x.object));
#endif
		}
		b.Unlock();
		result.RemoveUnnecessaryKnots();
		return result;
	};
	cweeBalancedPattern<type>& operator*=(const cweeBalancedPattern<type>& a);
	cweeBalancedPattern<type>& operator/=(const cweeBalancedPattern<type>& a);
	cweeBalancedPattern<type>& operator+=(const cweeBalancedPattern<type>& a);
	cweeBalancedPattern<type>& operator-=(const cweeBalancedPattern<type>& a);


	/*!
	Return true if the knot collections within the LHS and RHS are NOT equal
	*/
	friend bool									operator!=(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		return !operator==(a, b);
	};;
	/*!
	Return true if the effective knot collections within the LHS and RHS are equal (can have fewer or more knots - but if the Timeseries is
	effectively the same, then it is allowed). Result *is* dependant on the interpolation type.
	*/
	friend bool									operator==(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		bool out = true;
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			if (x.second != b.GetCurrentValue(x.first)) {
				out = false;
				break;
			}
#else
			if (*x.object != b.GetCurrentValue(x.key)) {
				out = false;
				break;
			}
#endif
		}
		a.Unlock();

		if (!out) return out;

		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
#ifdef  usePhmapBtree
			if (x.second != a.GetCurrentValue(x.first)) {
				out = false;
				break;
			}
#else
			if (*x.object != a.GetCurrentValue(x.key)) {
				out = false;
				break;
			}
#endif
		}
		b.Unlock();

		return out;
	};;
	/*!
	Return true if the smallest value of the LHS is greater than the largest value of the RHS
	*/
	friend bool									operator>(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		return (a.GetMinValue() > b.GetMaxValue() ? true : false);
	};;
	/*!
	Return true if the largest value of the LHS is less than the smallest value of the RHS
	*/
	friend bool									operator<(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		return (a.GetMaxValue() < b.GetMinValue() ? true : false);
	};;
	/*!
	Return true if the smallest value of the LHS is greater than or equal to the largest value of the RHS
	*/
	friend bool									operator>=(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		return (a.GetMinValue() >= b.GetMaxValue() ? true : false);
	};;
	/*!
	Return true if the largest value of the LHS is less than or equal to the smallest value of the RHS
	*/
	friend bool									operator<=(const cweeBalancedPattern<type>& a, const cweeBalancedPattern<type>& b) {
		return (a.GetMaxValue() <= b.GetMinValue() ? true : false);
	};;

	/*!
	Get Time Series of Pattern between date-Times at specified resolution
	*/
	cweeThreadedList<std::pair<u64, type>>		operator()(const u64& start, const u64& end, const u64& resolution) const;
	/*!
	Get Knot Series of Pattern between date-Times
	*/
	cweeThreadedList<std::pair<u64, type>>		operator()(const u64& start, const u64& end) const;
	/*!
	Get Spline value of Pattern at date-time
	*/
	float										operator()(const u64& time) const;

	/*! Clamp the y-axis Values such that they do not exceed the maximum and minimum Values. */
	void										ClampValues(const type& min, const type& max);

	virtual type								GetCurrentValue(const u64& time, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr, const measurement_t& outboundUnit = measurement_t::_end_) const;

	/*!
	Return approximate derivative of spline at time
	*/
	virtual type								GetCurrentFirstDerivative(const u64& time) const;

	/*!
	Return approximate second derivative of spline at time
	*/
	virtual type								GetCurrentSecondDerivative(const u64& time) const;

	virtual cweeThreadedList<std::pair<u64, type>>		GetKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits <u64>::max(), const measurement_t& outboundUnit = measurement_t::_end_) const;
	virtual cweeThreadedList<std::pair<u64, type>>		GetReversedKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits <u64>::max(), const measurement_t& outboundUnit = measurement_t::_end_) const;

	/*! Get list of spline samples at a specified Timestep */
	virtual cweeThreadedList<std::pair<u64, type>>		GetTimeSeries(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr, const measurement_t& outboundUnit = measurement_t::_end_) const;

	virtual cweeThreadedList<type>				GetValueKnotSeries(const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max(), const measurement_t& outboundUnit = measurement_t::_end_) const;

	virtual cweeThreadedList<type>				GetValueTimeSeries(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr, const measurement_t& outboundUnit = measurement_t::_end_) const;

	/*! Request an integration of the time series. Use divisor (ex. 3600) to convert (ex.) kW-sec to kW-hour. */
	virtual type								RombergIntegral(const u64& t0, const u64& t1, const float divisor = 3600.0f, bool snapLeft = false) const;

	/*! <X,X,X> = pattern.ValueQuantiles({ 0.25, 0.5, 0.75 }); */
	virtual cweeThreadedList<float>				ValueQuantiles(const cweeThreadedList<float>& probs, const u64& timeStart = -std::numeric_limits < u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max(), int numSamples = -1);

	virtual void								RemoveUnnecessaryKnots(const u64& start = -std::numeric_limits < u64>::max(), const u64& end = std::numeric_limits<u64>::max()) const;

	/*! Reduce memory usage where possible, with a desired percent removal. */
	virtual void								ReduceMemory(float percentToRemove, const u64& start = -std::numeric_limits < u64>::max(), const u64& end = std::numeric_limits<u64>::max()) const;

	virtual vec2								GetLearnedPeriod() const;

	/*! Remove knots older than the specified time. */
	virtual void								RemoveOlderThan(const u64& time);

	/*! Accept new Values from the Timeseries list of knots */
	virtual void								AcceptNewData(const cweeThreadedList<std::pair<u64, type>>& list, bool useMAD = false);

	virtual void								OverridePeriod(const cweeThreadedList<std::pair<u64, type>>& list, bool useMAD = false);

	/*! Accept changes made to a similar pattern. */
	virtual void								AcceptChanges(const cweeBalancedPattern<type>& copy);

	virtual void								SwapLearner(const cweeBalancedPattern<type>& copy);

	virtual type								GetCurrentMovingAverage(const u64& time) const;

	/*! Get a pattern (copy of this pattern) whose Y-Values are conversions of its X-Values */
	virtual cweeBalancedPattern<type>			GetTimePattern(bool returnHour = true) const;

	static int									GetMonthOfYear(const u64& time);

	static int									GetDayOfMonth(const u64& time);

	static int									GetDayOfWeek(const u64& time);

	static float								GetHourOfDay(const u64& time);

	static int									GetSeason(const u64& time);

	virtual type								GetNormalizedValue(const u64& time) const;

	/*!
	Get a pattern (copy of this pattern) whose Y-Values are conversions of its Y-Values
	0: Velocity
	1: Acceleration
	2: Moving Average
	3: Normalized ([0 - 1] range)
	*/
	virtual cweeBalancedPattern<type>			GetTransformedPattern(int choice) const;

	/*! Clear all knots and accept the new, incoming data Values from the list of knots. */
	virtual cweeBalancedPattern<type>			DuplicateWithNewData(const cweeThreadedList<std::pair<u64, type>>& in, bool useMAD = false);

	virtual bool								isLearned() const;

	/*! Produces a new pattern that is half the resolution of the previous pattern */
	virtual	cweeBalancedPattern<type>			GuassianBlur();

	/*!
	Produces a new pattern that blends this pattern with the "features" of the incoming pattern. It will result in an "average" value from this pattern but with the higher-resolution features.
	Ideally the incoming patterns were set up with matching durations and Timesteps (i.e. already did the machine learning and time series sampling)
	*/
	virtual	cweeBalancedPattern<type>			LaplacianBlend(const cweeBalancedPattern<type>& highResFeatures) const;

	/*!
	Seeks to manipulate "toManipulate" until the integration(s) match "toMatch"'s integration(s)
	Works best when "toManipulate" has significantly more knots than "toMatch"
	*/
	static cweeBalancedPattern<type>			AdjustPatternTillIntegralsMatch(const cweeBalancedPattern<type>& toMatch_, cweeBalancedPattern<type>& toManipulate, const u64& start = 0, const u64& end = 0, const u64& toMatchTargetResolution = 0);

	virtual type								GetTransformedCurrentValue(const u64& time, patternModifier mod, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr) const;

	virtual void								TemporarilyAdjustMachineLearnForecasts(const vec2& MAD_Adjustment);

	virtual vec2								Learn_Automated(const cweeThreadedList<std::pair<cweeStr, vec2>> FeaturePatterns = cweeThreadedList<std::pair<cweeStr, vec2>>(), const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr, const u64& timeStart = 0, const u64& timeEnd = 0);
	virtual vec2								Relearn_Automated(const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr, const u64& timeStart = 0, const u64& timeEnd = 0);
	virtual type								Forecast_Value_Automated(const u64& time, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr) const;
	virtual cweeThreadedList< std::pair<u64, type> >	Forecast_Series_Automated(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweeBalancedPattern<type> >* Parent = nullptr) const;

	virtual void								Lock() const;
	virtual void								Unlock() const;

protected:
	virtual bool								GetChanged() const;
	virtual void								SetChanged(bool in) const;
	virtual void								InsertPair(const u64& time, const type& valueIN, bool useMAD = true, bool unique = true);
	virtual bool								IsLooped() const;
	virtual bool								IsClamped() const;
	virtual u64									LoopedTime(const u64& t, bool forceLoop = false) const;
	virtual u64									ClampedTime(const u64& t) const;
	static cweeStr								SerializeTime(u64 t);
	u64											DeserializeTime(const cweeStr& t);
	void										Basis(const u64& t, u64* bvals) const;
	void										BasisFirstDerivative(const u64& t, u64* bvals) const;
	void										BasisSecondDerivative(const u64& t, u64* bvals) const;
	static bool									DetermineIfExcludeFeature(const cweeThreadedList< std::pair<u64, type> >& data);
	bool										DetermineIfExcludeFeature(int& check) const;
	static cweeTime								getLocalTime(const u64& time);

private:
#ifndef  usePhmapBtree
	mutable cweeBalancedTree< type, u64, 10>	container;
#else
	mutable phmap::btree_map< u64, type >		container;
#endif

	int											granularity = 16;
	int											identity = -1;
	vec3										MAD = vec3(1, 0, 0);
	mutable bool								changed = false;		// set whenever the curve changes
	cweeStr										_Name;
	value_t										valueType = value_t::_END_;
	measurement_t								measurementType = measurement_t::_end_;
	boundary_t									boundaryType = boundary_t::BT_FREE;
	interpolation_t								interpolationType = interpolation_t::IT_LINEAR;
	mutable cweeSysMutex						lock;

protected:
	mutable vec3								machineLearn_MinMaxRound = vec3(-cweeMath::INF, cweeMath::INF, 0.001f);
	mutable vec3								machineLearn_MAD = vec3(1, 0, 0);
	mutable cweeThreadedList<vec4>				machineLearn_features; // x:patternSource, y:patternIndex, z:patternModifier, w:timeOffset
	mutable cweeThreadedList<bool>				exclusion_list; // 0 = hours, 1 = dayOfWeek, 2 = weekOfMonth, 3 = quarterOfYear, 4... = machineLearn_features[0] ... machineLearn_features[n]
	mutable cweeML_learned_parameters*			learned = nullptr;

};

#endif