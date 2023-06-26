#pragma once
#include "Precompiled.h"

namespace data {
	using namespace cweeUnitValues;

	class TimeSeriesData {
	public:
		TimeSeriesData() : granularity(16), boundaryType(boundary_t::BT_FREE), interpolationType(interpolation_t::IT_LINEAR), lock(), X_Type(), Y_Type(), container() {};
		~TimeSeriesData() {};

	private:
		mutable cweeSysMutex								lock; 
		int													granularity;
		boundary_t											boundaryType;
		interpolation_t										interpolationType;		
		mutable unit_value									X_Type;
		mutable unit_value									Y_Type;
		mutable cweeBalancedTree< double, double, 10>		container;

	private:
		template<bool isX = true>
		void ConvertAllValues(unit_value oldT, unit_value newT) const {
			oldT = 1.0; // example (say) 1 foot
			newT = oldT; // became (say) 12 inches. New type is inches, and therefore all values must be multiplied by 12.
			double conversion = newT();
			if (conversion == 1.0) return;

			for (auto& x : container) {
				if constexpr (isX) {
					x.key *= conversion;
				}
				else {
					if (x.object) {
						*x.object *= conversion;
					}
				}
			}
		};
		unit_value& Unsafe_X(unit_value const& t) const {
			if (!cweeUnitValues::unit_value::is_scalar(t) && !cweeUnitValues::unit_value::IdenticalUnits(X_Type, t)) {
				ConvertAllValues<true>(X_Type, t);
			}
			X_Type = t; return X_Type;
		};
		unit_value& Unsafe_Y(unit_value const& t) const {
			if (!cweeUnitValues::unit_value::is_scalar(t) && !cweeUnitValues::unit_value::IdenticalUnits(Y_Type, t)) {
				ConvertAllValues<false>(Y_Type, t);
			}
			Y_Type = t; return Y_Type;
		};

	public:
		void												InsertPair(const unit_value& time, const unit_value& value, bool unique = true) {
			Lock();
			if (container.GetNodeCount() == 0 || container.GetNodeCount() + 1 > granularity) {
				granularity = (cweeMath::max(granularity, container.GetNodeCount())) * GRANULARITY_SCALER;
				container.Reserve(granularity);
			}
			container.Add(Unsafe_Y(value)(), Unsafe_X(time)(), unique);
			Unlock();
		};
		void												Clear() {
			AUTO g = lock.Guard();

			granularity = 16;
			boundaryType = boundary_t::BT_FREE;
			interpolationType = interpolation_t::IT_LINEAR;
			X_Type.Clear();
			Y_Type.Clear();
			container.Clear();			
		};
		void												ClearData() {
			AUTO g = lock.Guard();
			container.Clear();
		};

	public:
		int													GetGranularity(void) const {
			Lock();
			int out = granularity;
			Unlock();
			return out;
		};
		void												SetGranularity(int newNum) {
			Lock();
			granularity = newNum;
			Unlock();
		};
		void												SetBoundaryType(const boundary_t& bt) {
			AUTO g = lock.Guard();
			boundaryType = bt;
		};
		boundary_t											GetBoundaryType() const {
			boundary_t out;
			AUTO g = lock.Guard();
			out = boundaryType;
			return out;
		};
		interpolation_t										GetInterpolationType() const {
			interpolation_t out;
			AUTO g = lock.Guard();
			out = interpolationType;
			return out;
		};
		void												SetInterpolationType(const interpolation_t& it) {
			AUTO g = lock.Guard();
			interpolationType = it;
		};

	private:
		AUTO												Guard() const { return lock.Guard(); };
		void												Lock() const { lock.Lock(); };
		void												Unlock() const { lock.Unlock(); };	
	};

#if 0
	class TimeSeries { 
	private:
		TimeSeriesData data;

	public:
		TimeSeries() {};
		TimeSeries(const TimeSeries& source) { Copy(source); };
		~TimeSeries() { Clear(); };

		int													GetGranularity(void) const { return data.GetGranularity(); };
		void												SetGranularity(int newNum) { data.SetGranularity(newNum); };

		boundary_t											GetBoundaryType() const { return data.GetBoundaryType(); };
		void												SetBoundaryType(const boundary_t& bt) { data.SetBoundaryType(bt); };
		
		interpolation_t										GetInterpolationType() const { return data.GetInterpolationType(); };
		void												SetInterpolationType(const interpolation_t& it) { data.SetInterpolationType(it); };
		
		void												AddValue(const unit_value& time, unit_value const& valueIN) { data.InsertPair(time, valueIN, false); };
		void												AddUniqueValue(const unit_value& time, unit_value const& valueIN) { data.InsertPair(time, valueIN, true); };

		void												Clear() { data.Clear(); };
		void												ClearData() { data.ClearData(); };

		void												Copy(const TimeSeries& copy, const unit_value& timeStart = -std::numeric_limits < unit_value>::max(), const unit_value& timeEnd = std::numeric_limits < unit_value>::max()) {
			SetBoundaryType(copy.GetBoundaryType());
			SetInterpolationType(copy.GetInterpolationType());

			AcceptNewData(copy(timeStart, timeEnd));

			Lock(); copy.Lock();
			container = copy.container;
			copy.Unlock(); Unlock();
		};

		auto&												operator=(const TimeSeries& source) {
			if (this == &source) return *this;
			Copy(source);
			return *this;
		};
		auto&												operator=(const cweeThreadedList<pairT>& data) {
			this->Clear();
			AcceptNewData(data);
			return *this;
		};
		auto&												operator=(Y_Axis_Type data) {
			this->Clear();
			this->AddValue(0, data);
			return *this;
		};

		AUTO												R_Squared(const TimeSeries& other) const {
			AUTO out = this->GetCurrentValue(0) / this->GetCurrentValue(0);
			AUTO x0 = this->GetMinTime() < other.GetMinTime() ? other.GetMinTime() : this->GetMinTime();
			AUTO x1 = this->GetMaxTime() < other.GetMaxTime() ? this->GetMaxTime() : other.GetMaxTime();
			if (x1 > x0) {
				double N_steps = cweeMath::max(10, cweeMath::max(this->GetNumValues(), other.GetNumValues())); // at least 10 samples
				AUTO real = this->GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps));
				AUTO estimate = other.GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps));
				out = cweeEng::R_Squared(real, estimate);
			}
			else {
				out = 0;
			}
			return out;
		};



		operator cweeThreadedList<pairT>() const { return GetKnotSeries(); };
		operator cweeThreadedList<pairT>() { return GetKnotSeries(); };

		/*!
		Get Time Series of Pattern between date-Times at specified resolution
		*/
		cweeThreadedList<pairT>								operator()(const X_Axis_Type& start, const X_Axis_Type& end, const X_Axis_Type& resolution) const { return GetTimeSeries(start, end, resolution); };
		/*!
		Get Knot Series of Pattern between date-Times
		*/
		cweeThreadedList<pairT>								operator()(const X_Axis_Type& start, const X_Axis_Type& end) const { return GetKnotSeries(start, end); };
		/*!
		Get Spline value of Pattern at date-time
		*/
		Y_Axis_Type											operator()(const X_Axis_Type& time) const { return GetCurrentValue(time); };



		/*! Clamp the y-axis Values such that they do not exceed the maximum and minimum Values. */
		void												ClampValues(const Y_Axis_Type& min, const Y_Axis_Type& max) {
			AUTO g = lock.Guard();
			for (auto& x : this->container) {
				*x.object = cweeUnitValues::math::clamp(*x.object, min, max);
			}
		};

		Y_Axis_Type											GetValueAtIndex(size_t index) {
			AUTO g = lock.Guard();
			int n = 0;
			for (auto& x : container) {
				if (x.object) {
					if (n >= index) {
						return *x.object;
					}
					n++;
				}
			}
			return Y_Axis_Type();
		};
		X_Axis_Type											GetTimeAtIndex(size_t index) {
			AUTO g = lock.Guard();
			int n = 0;
			for (auto& x : container) {
				if (x.object) {
					if (n >= index) {
						return x.key;
					}
					n++;
				}
			}
			return X_Axis_Type();
		};

		size_t												GetLargestSmallerOrEqualTime(const X_Axis_Type& time) const {
			AUTO g = lock.Guard();
			size_t n = -1;
			for (auto& x : container) {
				if (x.object) {
					if (x.key > time) {
						return n;
					}
					n++;
				}
			}
			return n;
		};

		Y_Axis_Type											GetCurrentValue(const X_Axis_Type& time) const {
			int i = 0, j, k = 0;
			u64 bvals[4];
			X_Axis_Type clampedTime;
			Y_Axis_Type v; Y_Axis_Type* ptr;
			v = 0;
			Lock();
			j = container.GetNodeCount();
			if (j < 1) {
				v = 0;
				Unlock();
				return v;
			}
			else if (j == 1) {
				ptr = container.FindLargestSmallerEqual(time);
				if (ptr) v = *ptr;
				Unlock();
				return v;
			}
			else {
				Unlock();
				clampedTime = this->ClampedTime(time);
				clampedTime = this->LoopedTime(clampedTime);
				{
					Lock();
					switch (interpolationType) {
					case interpolation_t::IT_RIGHT_CLAMP: {
						ptr = container.FindSmallestLargerEqual(clampedTime);
						if (ptr) v = *ptr;
						break;
					}
					case interpolation_t::IT_SPLINE: {
						Unlock();
						Basis(clampedTime, bvals);
						Lock();
						node_type* x1 = container.NodeFindLargestSmallerEqual(clampedTime);
						node_type* x0 = x1 == nullptr ? (node_type*)nullptr : container.GetPrevLeaf(x1);
						node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
						node_type* x3 = x2 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x2);
						if (x0) v += (*x0->object * (scalarT)(u64)bvals[0]);	else if (x1) v += (*x1->object * (scalarT)(u64)bvals[0]);
						if (x1) v += (*x1->object * (scalarT)(u64)bvals[1]);
						if (x2) v += (*x2->object * (scalarT)(u64)bvals[2]);	else if (x1) v += (*x1->object * (scalarT)(u64)bvals[2]);
						if (x3) v += (*x3->object * (scalarT)(u64)bvals[3]); else if (x1) v += (*x1->object * (scalarT)(u64)bvals[3]);
						break;
					}
					case interpolation_t::IT_LINEAR: {
						Unlock();
						Basis(clampedTime, bvals);
						Lock();
						node_type* x1 = container.NodeFindLargestSmallerEqual(clampedTime);
						node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
						if (x1) v += (*x1->object * (scalarT)(u64)bvals[1]);
						if (x2) v += (*x2->object * (scalarT)(u64)bvals[2]);	else if (x1) v += (*x1->object * (scalarT)(u64)bvals[2]);
						break;
					}
					default:
					case interpolation_t::IT_LEFT_CLAMP: {
						ptr = container.FindLargestSmallerEqual(clampedTime);
						if (ptr) v = *ptr;
						break;
					}
					}
					Unlock();
				}
				return v;
			}
		};

		X_Axis_Type										GetTimeForValue(const Y_Axis_Type& val) const {
			// try and determine the first appropriate X-axis value that could generate the given Y-axis value;
			AUTO g = lock.Guard();
			cweeSharedPtr<bool> startedAbove;
			for (node_type& x1 : this->container) {
				if (*x1.object == val) return x1.key; // get out if found
				if (!startedAbove) startedAbove = *x1.object > val;
				if (*startedAbove) {
					// waiting to see when we shrink smaller than the val
					if (*x1.object < val) {
						node_type* x0p = container.GetPrevLeaf(&x1);
						if (x0p) {
							AUTO x0 = *x0p;
							// x0 > val > x1
							AUTO S = (val - *x1.object) / (*x0.object - *x1.object);
							return (x0.key - x1.key) * S;
						}
						else {
							// something went wrong. Return nearest time value. 
							return x1.key;
						}
					}
				}
				else {
					// waiting to see when we grow larger than the val
					if (*x1.object > val) {
						node_type* x0p = container.GetPrevLeaf(&x1);
						if (x0p) {
							AUTO x0 = *x0p;
							// x0 < val < x1
							AUTO S = (val - *x0.object) / (*x1.object - *x0.object);
							return (x1.key - x0.key) * S;
						}
						else {
							// something went wrong. Return nearest time value. 
							return x1.key;
						}
					}
				}
			}
			// never crossed above the requested value
			// ... invert the entire pattern and attempt an inverted interpolation. 
			TimeSeries inverted;
			for (node_type& x1 : this->container) {
				inverted.AddValue(*x1.object, x1.key);
			}
			return inverted.GetCurrentValue(val);			
		};
		/*! Return approximate derivative of spline at time */
		AUTO												GetCurrentFirstDerivative(const X_Axis_Type& time) const {
			X_Axis_Type step = 0.01;
			return (GetCurrentValue(time + step) - GetCurrentValue(time - step)) / (step * 2.0);
		};

		/*! Return approximate second derivative of spline at time */
		AUTO												GetCurrentSecondDerivative(const X_Axis_Type& time) const {
			X_Axis_Type step = 0.01;
			return (GetCurrentFirstDerivative(time + step) - GetCurrentFirstDerivative(time - step)) / (step * 2.0);		
		};

		cweeThreadedList<pairT>								GetKnotSeries(const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits <X_Axis_Type>::max()) const {
			int numKnots = this->GetNumValues();
			cweeThreadedList<pairT> out(numKnots + 16);

			AUTO g = lock.Guard();
			for (auto ptr = container.NodeFindLargestSmallerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
				if (ptr->object) {
					if (ptr->key >= timeStart) {
						if (ptr->key < timeEnd) {
							auto& set = out.Alloc();
							set.first = ptr->key;
							set.second = *ptr->object;
						}
						else {
							break;
						}
					}
				}
			}
			return out;
		};

		cweeThreadedList<pairT>								GetReversedKnotSeries(const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits <X_Axis_Type>::max()) const {
			int numKnots = this->GetNumValues();
			cweeThreadedList<pairT> out(numKnots + 16);

			AUTO g = lock.Guard();
			for (auto ptr = container.NodeFindSmallestLargerEqual(timeEnd); ptr; ptr = container.GetPrevLeaf(ptr)) {
				if (ptr->object) {
					if (ptr->key >= timeStart) {
						if (ptr->key < timeEnd) {
							auto& set = out.Alloc();
							set.first = ptr->key;
							set.second = *ptr->object;
						}
						else {
							break;
						}
					}
				}
			}

			return out;
		};

		/*! Get list of spline samples at a specified Timestep */
		cweeThreadedList<pairT>								GetTimeSeries(const X_Axis_Type& timeStart, const X_Axis_Type& timeEnd, const X_Axis_Type& timeStep) const {
			pairT v;
			X_Axis_Type realTimestep = cweeMath::Fmax((u64)timeStep, 1);
			cweeThreadedList<pairT> out(cweeMath::max(cweeMath::min(((u64)(timeEnd - timeStart) / (u64)(realTimestep)), 100000), 1000) + 16);

			v.first = timeStart; v.second = GetCurrentValue(timeStart);
			out.Append(v); // ensure pattern always has a starter? 

			for (v.first = timeStart + realTimestep; v.first < timeEnd; v.first += realTimestep) {
				v.second = GetCurrentValue(v.first);
				out.Append(v);
			}

			v.first = timeEnd; v.second = GetCurrentValue(timeEnd);
			out.Append(v); // ensure pattern always has a closure? 

			return out;
		};

		cweeThreadedList<Y_Axis_Type>						GetValueKnotSeries(const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits < X_Axis_Type>::max()) const {
			int numKnots = this->GetNumValues();
			cweeThreadedList<Y_Axis_Type> out(numKnots + 16);

			AUTO g = lock.Guard();
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

			return out;
		};

		cweeThreadedList<Y_Axis_Type>						GetValueTimeSeries(const X_Axis_Type& timeStart, const X_Axis_Type& timeEnd, const X_Axis_Type& timeStep) const {
			X_Axis_Type realTimestep = cweeMath::Fmax((u64)timeStep, 1);
			cweeThreadedList<Y_Axis_Type> out(cweeMath::max(cweeMath::min((u64)((u64)(timeEnd - timeStart) / (u64)(realTimestep)), 100000), 1000) + 16);

			out.Append(GetCurrentValue(timeStart)); // ensure pattern always has a starter? 
			for (X_Axis_Type t = timeStart + realTimestep; t < timeEnd; t += realTimestep) {
				out.Append(GetCurrentValue(t));
			}
			out.Append(GetCurrentValue(timeEnd)); // ensure pattern always has a closure? 

			return out;
		};

		/*! Request an integration of the time series. The timefactor determines the resulting time component. I.e. A pattern of kilowatt_t and a time factor of hour_t will return a kilowatt_hour_t. */
		AUTO												RombergIntegral(const X_Axis_Type& t0, const X_Axis_Type& t1) const {
			auto sum = Y_Axis_Type(0) * X_Axis_Type(0);
			if (this->GetNumValues() > 1) {
				X_Axis_Type step = this->GetMinimumTimeStep();
				X_Axis_Type minGot = t1, maxGot = t1;
				if (true) {
					AUTO Guard = this->lock.Guard();
					if (Guard) {
						cweeThreadedList<node_type*> data = this->UnsafeGetKnotSeries(t0, t1);
						if (data.Num() > 1) {
							minGot = data[0]->key;
							maxGot = data[data.Num() - 1]->key;

							for (int i = 0; i < (data.Num() - 1); i++) {
								node_type&
									left = *data.operator[](i),
									right = *data.operator[](i + 1);

								sum += ((*right.object + *left.object) * scalarT(0.5)) * X_Axis_Type(right.key - left.key);
							}
						}
					}
				}

				X_Axis_Type
					t = 0,
					stepDiv2 = step / 2.0,
					maxT = t1 + stepDiv2;

				for (t = t0; (t + step) < minGot; t += step) sum += X_Axis_Type(step) * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += X_Axis_Type(minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				for (t = maxGot; (t + step) < t1; t += step) sum += X_Axis_Type(step) * GetCurrentValue(t + stepDiv2);
				if (t1 > t)  sum += X_Axis_Type(t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}
			return sum;
		};

		/*! <X,X,X> = pattern.ValueQuantiles({ 0.25, 0.5, 0.75 }); */
		cweeThreadedList<Y_Axis_Type>						ValueQuantiles(const cweeThreadedList<float>& probs, const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits < X_Axis_Type>::max(), int numSamples = -1) {
			cweeThreadedList<Y_Axis_Type> quantiles;
			scalarT poi;
			size_t left, right;

			cweeThreadedList<Y_Axis_Type> data;
			if (numSamples >= 1) {
				X_Axis_Type t0 = std::max(timeStart, this->GetMinTime());
				X_Axis_Type t1 = std::min(timeEnd, this->GetMaxTime());
				X_Axis_Type step = (t1 - t0) / numSamples;
				if (!::isfinite(step) || step == 0) step = (1 * 24 * 60 * 60);

				data = GetValueTimeSeries(t0, t1, step); // copy data
			}
			else
			{
				data = GetValueKnotSeries(timeStart, timeEnd); // copy data
			}

			data.AssureSize(probs.Num());
			if (data.Num() <= 1) return data;

			data.Sort(); // sort data

			for (size_t i = 0; i < probs.Num(); ++i)
			{
				poi = (1.0f - probs[i]) * -0.5f + probs[i] * (data.Num() - 0.5f);
				left = std::max(int64_t(std::floor((double)poi)), int64_t(0));
				right = std::min(int64_t(std::ceil((double)poi)), int64_t(data.Num() - 1.0f));
				quantiles.Append((((scalarT)1.0f - (poi - (scalarT)left)) * data[left]) + ((poi - (scalarT)left) * data[right]));
			}

			return quantiles;
		};

		void												RemoveUnnecessaryKnots(const X_Axis_Type& start = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& end = std::numeric_limits<X_Axis_Type>::max()) const {
			if (start >= end) return;
			int num, index; cweeThreadedList<X_Axis_Type> keysToDelete;
			Y_Axis_Type epsilon = 0.001f;
			Y_Axis_Type* val1 = nullptr, * val2 = nullptr, * val3 = nullptr, * val4 = nullptr, * val5 = nullptr;
			X_Axis_Type t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;

			AUTO g = lock.Guard();
			{
				decltype(container)& _values = container;

				num = _values.GetNodeCount();

				if (num > 5) {
					index = 0;
					for (auto& knot : _values) {
						if (index + 3 >= num) break;
						if (index >= 5) {
							if (cweeUnitValues::math::fabs(*val1 - *val2) > epsilon) { // !=
								++index;
								val1 = val2; t1 = t2;
								val2 = val3; t2 = t3;
								val3 = val4; t3 = t4;
								val4 = val5; t4 = t5;
								val5 = knot.object;
								t5 = knot.key;
								continue;
							}
							else if (cweeUnitValues::math::fabs(*val5 - *val2) <= epsilon) { // ==
								if (cweeUnitValues::math::fabs(*val4 - *val2) <= epsilon) { // ==
									if (cweeUnitValues::math::fabs(*val3 - *val2) <= epsilon) { // ==
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
				}

				for (auto& key : keysToDelete) { _values.Remove(_values.NodeFind(key)); }
			}
		};

		/*! Reduce memory usage where possible, with a desired percent removal. */
		void												ReduceMemory(float percentToRemove, const X_Axis_Type& start = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& end = std::numeric_limits<X_Axis_Type>::max()) const {
			if (start >= end) return;
			RemoveUnnecessaryKnots(start, end);
			if (percentToRemove <= 0) return;

			Y_Axis_Type maxValue = this->GetMaxValue();
			Y_Axis_Type minValue = this->GetMinValue();
			Y_Axis_Type epsilon = cweeUnitValues::math::fabs(maxValue - minValue) * (scalarT)(((float)percentToRemove) / 100.0f);
			int num, index; cweeThreadedList<X_Axis_Type> keysToDelete;

			Y_Axis_Type* val1 = nullptr, * val2 = nullptr, * val3 = nullptr, * val4 = nullptr, * val5 = nullptr;
			X_Axis_Type t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;

			AUTO g = lock.Guard();
			{
				decltype(container)& _values = container;

				num = _values.GetNodeCount();

				if (num > 5) {
					index = 0;
					for (auto& knot : _values) {
						if (index + 3 >= num) break;
						if (index >= 5) {
							if (
								(cweeUnitValues::math::fabs(*val2 - *val3) <= epsilon)
								&& (cweeUnitValues::math::fabs(*val4 - *val3) <= epsilon)
								&& (cweeUnitValues::math::fabs(*val2 - *val4) <= epsilon)
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

				for (auto& key : keysToDelete) {
					_values.Remove(_values.NodeFind(key));
				}
			}
		};;

		/*! Remove knots older than the specified time. */
		void												RemoveOlderThan(const X_Axis_Type& time) {
			cweeThreadedList<X_Axis_Type> keysToDelete;
			AUTO g = lock.Guard();
			{
				decltype(container)& _values = container;
				for (auto& x : _values) {
					if (x.key > time) keysToDelete.Append(x.key);
				}

				for (auto& key : keysToDelete) {
					_values.Remove(_values.NodeFind(key));
				}
			}
		};

		/*! Accept new Values from the Timeseries list of knots */
		void												AcceptNewData(const cweeThreadedList<pairT>& list) {
			for (auto& x : list) {
				this->AddUniqueValue(x.first, x.second);
			}
		};

		void												OverridePeriod(const cweeThreadedList<pairT>& list) {
			if (list.Num() <= 0) return;
			X_Axis_Type start = list[0].first;
			X_Axis_Type end = list[list.Num() - 1].first;
			cweeThreadedList<X_Axis_Type> keysToDelete;

			{
				AUTO g = lock.Guard();
				{
					decltype(container)& _values = container;
					for (auto& x : _values) {

						if (x.key >= start) {
							if (x.key <= end) {
								keysToDelete.Append(x.key);
							}
							else {
								break;
							}
						}
					}
				}
				{
					decltype(container)& _values = container;

					for (auto& key : keysToDelete) {
						_values.Remove(_values.NodeFind(key));
					}
				}
			}
			{
				decltype(container)& _values = container;

				for (auto& x : list) {
					InsertPair(x.first, x.second, true);
				}
			}
		};

		/*! Accept changes made to a similar pattern. */
		void												AcceptChanges(const TimeSeries& copy) {
			SetBoundaryType(copy.GetBoundaryType());
			SetInterpolationType(copy.GetInterpolationType());

			Lock(); copy.Lock();
			container = copy.container;
			copy.Unlock(); Unlock();

			AcceptNewData(copy.GetKnotSeries());
		};

		Y_Axis_Type											GetCurrentMovingAverage(const X_Axis_Type& time) const {
			Y_Axis_Type out; node_type* ptr = nullptr;
			out = 0; int i = 0;
			AUTO g = lock.Guard();
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
			return out / ((scalarT)6.0f);
		};

		scalarT												GetNormalizedValue(const X_Axis_Type& time) const {
			AUTO min = this->GetMinValue();
			AUTO max = this->GetMaxValue();
			if (min == max) max = min + (Y_Axis_Type)1.0f;
			return (GetCurrentValue(time) - min) / (max - min);
		};

		/*! Clear all knots and accept the new, incoming data Values from the list of knots. */
		TimeSeries					DuplicateWithNewData(const cweeThreadedList<pairT>& in) {
			TimeSeries out;
			out.Copy(*this);
			out.Lock();
			out.container.Clear();
			out.Unlock();

			for (auto& x : in) {
				out.AddUniqueValue(x.first, x.second);
			}
			return out;
		};

		AUTO												Guard() const { return this->lock.Guard(); };
		void												Lock() const { this->lock.Lock(); };
		void												Unlock() const { this->lock.Unlock(); };

protected:
		void												InsertPair(const X_Axis_Type& time, const Y_Axis_Type& valueIN, bool unique = true) {
			Lock();
			if (container.GetNodeCount() + 1 > granularity) {
				granularity = (cweeMath::max(granularity, container.GetNodeCount())) * GRANULARITY_SCALER;
				container.Reserve(granularity);
			}
			container.Add(valueIN, time, unique);
			Unlock();
		};
		bool												IsLooped() const {
			bool out = false;
			Lock();
			out = (boundaryType == boundary_t::BT_LOOP);
			Unlock();
			return out;
		};
		bool												IsClamped() const {
			bool out = false;
			Lock();
			out = (boundaryType == boundary_t::BT_CLAMPED);
			Unlock();
			return out;
		};
		X_Axis_Type											LoopedTime(const X_Axis_Type& t, bool forceLoop = false) const {
			if (forceLoop || IsLooped()) {
				X_Axis_Type minTime, maxTime, len, currentTime, avgStep;
				minTime = GetMinTime();
				maxTime = GetMaxTime();
				if (maxTime < t || minTime > t) {
					if (maxTime > minTime) {
						len = maxTime - minTime;
						avgStep = len / (GetNumValues() - 1);
						len += avgStep;
						maxTime = len + minTime;

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
					else {
						return minTime;
					}
				}
			}
			return t;
		};
		X_Axis_Type											ClampedTime(const X_Axis_Type& t) const {
			if (IsClamped()) {
				X_Axis_Type mT = this->GetMinTime();
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
		void												Basis(const X_Axis_Type& t, u64* bvals) const {
			AUTO g = lock.Guard();
			UnsafeBasis(t, bvals);
		};
		void												UnsafeBasis(const X_Axis_Type& t, u64* bvals) const {
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
			u64 s; node_type* index1 = nullptr;
			switch (interpolationType) {
			case interpolation_t::IT_RIGHT_CLAMP: {
				bvals[0] = 0;
				bvals[1] = 0;
				bvals[2] = 1;
				bvals[3] = 0;
				break;
			}
			case interpolation_t::IT_SPLINE: {
				index1 = container.NodeFindLargestSmallerEqual(t);
				if (index1) {
					s = (u64)index1->key;
					index1 = container.GetNextLeaf(index1);
					if (index1) {
						if (s <= (u64)t && index1->key >= t) {
							s = (u64)((u64)t - s) / ((u64)index1->key - s);
							if (!::isfinite(s)) s = 0;

							bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;				// -0.5f s * s * s + s * s - 0.5f * s
							bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
							bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
							bvals[3] = ((s - 1.0f) * s * s) * 0.5f;						// 0.5f * s * s * s - 0.5f * s * s
						}
						else {
							// something went wrong - snap left. 
							s = (u64)(s - s) / ((u64)index1->key - s);
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
				break;
			}
			case interpolation_t::IT_LINEAR: {
				index1 = container.NodeFindLargestSmallerEqual(t);
				if (index1) {
					s = (u64)index1->key;
					index1 = container.GetNextLeaf(index1);
					if (index1) {
						if (s <= (u64)t && index1->key >= t) {
							s = (u64)((u64)t - s) / ((u64)index1->key - s);
							if (!::isfinite(s)) s = 0;

							bvals[0] = 0;
							bvals[1] = (u64)(1.0f - s);
							bvals[2] = s;
							bvals[3] = 0;

						}
						else {
							// something went wrong - snap left. 
							s = (u64)(s - s) / ((u64)index1->key - s);
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
		};
		void												BasisFirstDerivative(const X_Axis_Type& t, u64* bvals) const {
			node_type* index1 = nullptr;
			u64 s;
			AUTO g = lock.Guard();
			index1 = container.NodeFindLargestSmallerEqual(t);
			if (index1) {
				s = (u64)index1->key;
				index1 = container.GetNextLeaf(index1);
				if (index1) {
					if (s <= (u64)t && index1->key >= t) {
						s = (u64)((u64)t - s) / ((u64)index1->key - s);
						if (!::isfinite(s)) s = 0;

						bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
						bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
						bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
						bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
					}
					else {
						s = (u64)(s - s) / ((u64)index1->key - s);
						if (!::isfinite(s)) s = 0;

						bvals[0] = (-1.5f * s + 2.0f) * s - 0.5f;						// -1.5f * s * s + 2.0f * s - 0.5f
						bvals[1] = (4.5f * s - 5.0f) * s;								// 4.5f * s * s - 5.0f * s
						bvals[2] = (-4.5 * s + 4.0f) * s + 0.5f;						// -4.5 * s * s + 4.0f * s + 0.5f
						bvals[3] = 1.5f * s * s - s;									// 1.5f * s * s - s
					}
				}
				else {
					bvals[0] = -0.5;
					bvals[1] = 0;
					bvals[2] = 0.5;
					bvals[3] = 0;
				}
			}
			else {
				bvals[0] = -0.5;
				bvals[1] = 0;
				bvals[2] = 0.5;
				bvals[3] = 0;
			}
		};
		void												BasisSecondDerivative(const X_Axis_Type& t, u64* bvals) const {
			node_type* index1 = nullptr;
			u64 s;
			AUTO g = lock.Guard();
			index1 = container.NodeFindLargestSmallerEqual(t);
			if (index1) {
				s = (u64)index1->key;
				index1 = container.GetNextLeaf(index1);
				if (index1) {
					if (s <= (u64)t && index1->key >= t) {
						s = (u64)((u64)t - s) / ((u64)index1->key - s);
						if (!::isfinite(s)) s = 0;

						bvals[0] = -3.0f * s + 2.0f;
						bvals[1] = 9.0f * s - 5.0f;
						bvals[2] = -9.0f * s + 4.0f;
						bvals[3] = 3.0f * s - 1.0f;
					}
					else {
						s = (u64)(s - s) / ((u64)index1->key - s);
						if (!::isfinite(s)) s = 0;

						bvals[0] = -3.0f * s + 2.0f;
						bvals[1] = 9.0f * s - 5.0f;
						bvals[2] = -9.0f * s + 4.0f;
						bvals[3] = 3.0f * s - 1.0f;

					}
				}
				else {
					bvals[0] = 2.0;
					bvals[1] = -5.0;
					bvals[2] = 4.0;
					bvals[3] = -1.0;
				}
			}
			else {
				bvals[0] = 2.0;
				bvals[1] = -5.0;
				bvals[2] = 4.0;
				bvals[3] = -1.0;
			}
		};


	};
#endif

};