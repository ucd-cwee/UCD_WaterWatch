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
#include "Pattern.h"
#include "List.h"
#include "LinkedList.h"
#include "BasicUnits.h"
#include "vec.h"
#include "cwee_math.h"
#include "UnorderedList.h"
#include "BalancedTree.h"
#include "Strings.h"
#include "cweeTime.h"
#include "Engineering.h"

template<typename A, typename B>
class cweePair {
public:
	using first_type = typename A;
	using second_type = typename B;
	 
	cweePair<A, B>() {};
	cweePair<A, B>(const A& in1, const B& in2) {
		first = in1;
		second = in2;
	};

	cweePair<A, B>& operator=(const cweePair<A, B>& other) {
		first = other.first;
		second = other.second;

		return *this;
	};
	friend bool			operator==(const cweePair& a, const cweePair& b) {
		if ((b.first == a.first) && (b.second == a.second)) return true;
		return false;
	};
	friend bool			operator!=(const cweePair& a, const cweePair& b) {
		return !operator==(a);
	};

	template< typename _otherA_, typename _otherB_ >
	operator cweePair<_otherA_, _otherB_>() const {
		cweePair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};
	template< typename _otherA_, typename _otherB_ >
	operator cweePair<_otherA_, _otherB_>() {
		cweePair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};

	template< typename _otherA_, typename _otherB_ >
	operator std::pair<_otherA_, _otherB_>() const {
		std::pair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};
	template< typename _otherA_, typename _otherB_ >
	operator std::pair<_otherA_, _otherB_>() {
		std::pair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};

	mutable A first;
	mutable B second;
};

#if 1
template< class type >
class cweeBalancedCurve {
public:
	using node_type = typename cweeBalancedTree<type, u64>::cweeBalancedTreeNode;

	cweeBalancedCurve() {};
	cweeBalancedCurve(const cweeBalancedCurve& source) {
		Copy(source);
	};
	cweeBalancedCurve(const cweeThreadedList<std::pair<u64, type>>& data) {
		for (auto& x : data) {
			AddValue(x.first, x.second);
		}
	};
	~cweeBalancedCurve() {
		Clear();
	};
	cweeStr				Type() const { return cweeAny::TypeNameOf<decltype(*this)>(); };
	int					GetLevel() const { return 0; }
	 void		Deserialize(cweeStr& inbound) {
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
	 cweeStr		Serialize() const {
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

	auto& UnsafeGetValues() const { return container; };
	AUTO UnsafeGetValue(const u64& time) const { return UnsafeGetValues().NodeFindLargestSmallerEqual(time); };
	cweeThreadedList<node_type*> UnsafeGetKnotSeries() const {
		cweeThreadedList<node_type*> out;
		for (auto& x : UnsafeGetValues()) {
			out.Append(&x);
		}
		return out;
	};

	 void		AddValue(const u64& time, const type& valueIN) {
		InsertPair(time, valueIN, false);
	};;
	 void		AddUniqueValue(const u64& time, const type& valueIN) { // slower with guarrantee of uniqueness 
		InsertPair(time, valueIN, true);
	};

	void				RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo) {
		int lowerLimit, upperLimit, index; cweeThreadedList<int> indexesToDelete;
		Lock();
		{
#ifndef  usePhmapBtree
			auto iter = container.NodeFindSmallestLargerEqual(greaterThan);
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
	 void		Clear() {
		Lock();
#ifndef  usePhmapBtree
		container.Clear();
#else
		container.clear();
#endif
		Unlock();
	};

	 u64			GetAvgTime() const {
		u64 out(0);
		int num(0);

		Lock();
		for (auto& x : container) {
			if (x.object) {
				num++;
				out -= (decltype(out))(out / (u64)num);
#ifndef  usePhmapBtree
				out += (decltype(out))(x.key / (u64)num);
#else
				out += (decltype(out))(x.first / (u64)num);
#endif
			}

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
			auto ptr = container.NodeFindSmallestLargerEqual(-std::numeric_limits<u64>::max());
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
			//if (x.object) {
#ifndef  usePhmapBtree
				x.key += deltaTime;
#else
				throw("phMap does not support key shifting");
				// x.first += deltaTime;
#endif
			//}
		}
		Unlock();
	};

	 void		Copy(const cweeBalancedCurve& copy, const u64& timeStart = -std::numeric_limits <u64>::max(), const u64& timeEnd = std::numeric_limits < u64>::max()) {
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
		for (auto ptr = container.NodeFindSmallestLargerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
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
		for (auto ptr = container.NodeFindSmallestLargerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
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
	// mutable cweeConstexprLock					lock; 
	mutable cweeSysMutex						lock;

};
#endif

namespace cweeBalancedPatternDetail
{
	struct _cwee_balanced_pattern_t {};
	template<class T> struct is_balanced_pattern_t : std::is_base_of<_cwee_balanced_pattern_t, T>::type {};
}
template< class Y_Axis_Type = units::length::meter_t, class X_Axis_Type = u64 > 
class cweeBalancedPattern : public cweeBalancedPatternDetail::_cwee_balanced_pattern_t {
public:
	using node_type = typename cweeBalancedTree<Y_Axis_Type, X_Axis_Type>::cweeBalancedTreeNode;
	using scalarT = typename units::dimensionless::scalar_t;
	using pairT = typename cweePair< X_Axis_Type, Y_Axis_Type >;

private:
	mutable cweeBalancedTree< Y_Axis_Type, X_Axis_Type, 10>			container;
	int													granularity;
	boundary_t											boundaryType;
	interpolation_t										interpolationType;
	mutable cweeSysMutex								lock;

public:
	cweeBalancedPattern() : 
		granularity(16), 
		boundaryType(boundary_t::BT_FREE), 
		interpolationType(interpolation_t::IT_LINEAR), 
		lock(), 
		container(16) 
	{};
	cweeBalancedPattern(const cweeBalancedPattern& source) : granularity(16), boundaryType(boundary_t::BT_FREE), interpolationType(interpolation_t::IT_LINEAR), lock(), container(16) {
		Copy(source);
	};
	cweeBalancedPattern(const cweeThreadedList<pairT>& data) : granularity(16), boundaryType(boundary_t::BT_FREE), interpolationType(interpolation_t::IT_LINEAR), lock(), container(16) {
		AcceptNewData(data);
	};
	~cweeBalancedPattern() { Clear(); };

	int													GetGranularity(void) const {
		AUTO g = lock.Guard();
		int out = granularity;
		return out;
	};
	void												SetGranularity(int newNum) {
		AUTO g = lock.Guard();
		granularity = newNum;
	};

	auto&												UnsafeGetValues() const { return container; };
	node_type*											UnsafeGetValue(const X_Axis_Type& time) const { return UnsafeGetValues().NodeFindLargestSmallerEqual(time); };
	cweeThreadedList<node_type*>						UnsafeGetKnotSeries(const X_Axis_Type& t0 = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& t1 = std::numeric_limits < X_Axis_Type>::max()) const {
		cweeThreadedList<node_type*> out;
		for (auto& x : UnsafeGetValues()) {
			if (x.key >= t0 && x.key <= t1) {
				out.Append(&x);
			}
		}
		return out;
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

	 void												SetInterpolationType(const interpolation_t& it) {
		 AUTO g = lock.Guard();
		interpolationType = it;		
	};
	 interpolation_t									GetInterpolationType() const {
		interpolation_t out;
		AUTO g = lock.Guard();
		out = interpolationType;
		return out;
	};

	 void												AddValue(const X_Axis_Type& time, Y_Axis_Type valueIN) { InsertPair(time, valueIN, false); };
	 void												AddUniqueValue(const X_Axis_Type& time, Y_Axis_Type valueIN) { InsertPair(time, valueIN, true); };
	 void												CompressLastValueAdded() {
		 int num, index; cweeThreadedList<X_Axis_Type> keysToDelete;
		 constexpr Y_Axis_Type epsilon = 0.001f;
		 Y_Axis_Type* val1 = nullptr, * val2 = nullptr, * val3 = nullptr, * val4 = nullptr, * val5 = nullptr;
		 X_Axis_Type t3 = 0;

		 Lock();
		 {
			 num = container.GetNodeCount();
			 if (num >= 5) { // get the last 5 values if available	 
				 for (auto ptr = container.NodeFindLargestSmallerEqual(std::numeric_limits <X_Axis_Type>::max()); ptr; ptr = container.GetPrevLeaf(ptr)) {
					 if (ptr->object) {
						 if (!val5) {
							 val5 = ptr->object;
						 }
						 else if (!val4) {
							 val4 = ptr->object;
						 }
						 else if(!val3) {
							 val3 = ptr->object;
							 t3 = ptr->key;
						 }
						 else if (!val2) {
							 val2 = ptr->object;
						 }
						 else if (!val1) {
							 val1 = ptr->object;
							 break;
						 }
					 }
				 }
				 if (val5 && val4 && val3 && val2 && val1) {
					 if (units::math::fabs(*val1 - *val2) <= epsilon && units::math::fabs(*val2 - *val3) <= epsilon && units::math::fabs(*val3 - *val4) <= epsilon && units::math::fabs(*val4 - *val5) <= epsilon)
						 container.Remove(container.NodeFind(t3));
				 }				 
			 }
		 }
		 Unlock();
	 };

	 template <typename newY>
	 cweeBalancedPattern<newY, X_Axis_Type>				CastType(const X_Axis_Type& t0 = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& t1 = std::numeric_limits < X_Axis_Type>::max()) {
		 cweeBalancedPattern<newY, X_Axis_Type> out;

		 for (auto& x : GetKnotSeries(t0, t1)) {
			 out.AddValue(x.first, x.second);
		 }

		 return out;
	 };
	 template <typename newY, typename newX>
	 cweeBalancedPattern<newY, newX>					CastType(const X_Axis_Type& t0 = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& t1 = std::numeric_limits < X_Axis_Type>::max()) {
		 cweeBalancedPattern<newY, newX> out;

		 for (auto& x : GetKnotSeries(t0, t1)) {
			 out.AddValue(x.first, x.second);
		 }

		 return out;
	 };
	 template <typename newY>
	 cweeBalancedPattern<newY, X_Axis_Type>				ForceCastType(const X_Axis_Type& t0 = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& t1 = std::numeric_limits < X_Axis_Type>::max()) {
		 cweeBalancedPattern<newY, X_Axis_Type> out;

		 constexpr bool Y_convertible = units::traits::is_convertible_unit_t< newY, Y_Axis_Type>::value;

		 for (auto& x : GetKnotSeries(t0, t1)) {			 
			 if constexpr (Y_convertible) {
				 out.AddValue(x.first, x.second);
			 }
			 else {
				 out.AddValue((u64)x.first, (u64)x.second);
			 }			 
		 }

		 return out;
	 };
	 template <typename newY, typename newX>
	 cweeBalancedPattern<newY, newX>					ForceCastType(const X_Axis_Type& t0 = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& t1 = std::numeric_limits < X_Axis_Type>::max()) {
		 cweeBalancedPattern<newY, newX> out;

		 constexpr bool Y_convertible = units::traits::is_convertible_unit_t< newY, Y_Axis_Type>::value;
		 constexpr bool X_convertible = units::traits::is_convertible_unit_t< newX, X_Axis_Type>::value;

		 for (auto& x : GetKnotSeries(t0, t1)) {
			 if constexpr (Y_convertible) {
				 if constexpr (X_convertible) {
					 out.AddValue(x.first, x.second);
				 }
				 else {
					 out.AddValue((u64)x.first, x.second);
				 }
			 }
			 else {
				 if constexpr (X_convertible) {
					 out.AddValue(x.first, (u64)x.second);
				 }
				 else {
					 out.AddValue((u64)x.first, (u64)x.second);
				 }
			 }	
		 }

		 return out;
	 };

	 void												RemoveTimes(const X_Axis_Type& greaterThan, const X_Axis_Type& lessThenEqualTo) {
		int lowerLimit, upperLimit, index; cweeThreadedList<int> indexesToDelete;
		AUTO g = lock.Guard();
		{
			auto iter = container.NodeFindSmallestLargerEqual(greaterThan);
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
		}
	};
	 void												Clear() {
		 AUTO g = lock.Guard();
		container.Clear();
		boundaryType = boundary_t::BT_FREE;
		interpolationType = interpolation_t::IT_LINEAR;
	};
	 void												ClearData() {
		 AUTO g = lock.Guard();
		 container.Clear();
	 };
	 scalarT											GetMinimumDecimals() const {
		scalarT decimal = 1.0f;
		int numbs = GetNumValues();
		if (numbs == 0) return 0.0001f;
		scalarT F;
		int numSuccess = 0;

		AUTO g = lock.Guard();
		for (auto& x : container) {
			if (x.object) {
				F = cweeMath::roundNearest((float)*x.object, (float)decimal);
				if (cweeMath::Fabs((float)(F - (float)*x.object)) > 0.00001) {
					// too great an error
					decimal = (float)decimal / 10.0f;
					numSuccess = 0;
					if (decimal <= scalarT(0.0001f)) {
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
		}
		return decimal;
	 };
	 X_Axis_Type										GetMinimumTimeStep() const {
		X_Axis_Type out, prevTime, t;
		int numFailures, numbs;

		numbs = GetNumValues();
		if (numbs <= 1) return 1;

		out = GetMaxTime() - GetMinTime();
		prevTime = std::numeric_limits<X_Axis_Type>::max();
		numFailures = 100;

		AUTO g = lock.Guard();
		for (auto& x : container) {
			if (x.object) {
				t = (prevTime - x.key) >= (X_Axis_Type)0 ? (prevTime - x.key) : -(prevTime - x.key);
				prevTime = x.key;
				if (t < out && t >(X_Axis_Type)0) {
					out = t;
				}
				else {
					numFailures--;
					if (numFailures <= 0) break;
				}
			}
		}

		if (out <= (X_Axis_Type)1) out = (X_Axis_Type)1;

		return out;
	};

	 Y_Axis_Type										GetMinValue() const {
		Y_Axis_Type out;
		out = 0;
		if (GetNumValues() == 0) return out;
		bool skipFirst = true;
		AUTO g = lock.Guard(); 
		for (auto& x : container) {
			if (x.object) {
				if (skipFirst) {
					skipFirst = false;
					out = *x.object;
				}
				else {
					if (*x.object < out) out = *x.object;
				}
			}
		}
		return out;
	 };
	 Y_Axis_Type										GetMaxValue() const {
		 Y_Axis_Type out;
		 out = 0;
		 if (GetNumValues() == 0) return out;
		 bool skipFirst = true;
		 AUTO g = lock.Guard();
		 for (auto& x : container) {
			 if (x.object) {
				 if (skipFirst) {
					 skipFirst = false;
					 out = *x.object;
				 }
				 else {
					 if (*x.object > out) out = *x.object;
				 }
			 }
		 }
		 return out;
	 };
	 Y_Axis_Type										GetAvgValue() const {
		Y_Axis_Type out;
		out = 0;
		int num(0);

		AUTO g = lock.Guard();
		for (auto& x : container) {
			if (x.object) {
				num++;
				out -= (out / (scalarT)num);
				out += (*x.object / (scalarT)num);
			}
		}
		return out;
	};

	 Y_Axis_Type										GetMinValue(const X_Axis_Type& start, const X_Axis_Type& end) const {
		Y_Axis_Type out;
		out = 0;
		int n = GetNumValues();
		if (n == 0) return out;
		bool skipFirst = true;
		AUTO g = lock.Guard();
		
		auto iter = container.NodeFindSmallestLargerEqual(start);
		if (iter) {
			do {
				if (iter->key > start) {
					if (iter->key <= end) {
						if (skipFirst) {
							skipFirst = false;
							out = *iter->object;
						}
						else {
							if (*iter->object < out) out = *iter->object;
						}
					}
					else {
						break;
					}
				}
				iter = container.GetNextLeaf(iter);
			} while (iter);
		}		

		return out;
	};;
	 Y_Axis_Type										GetMaxValue(const X_Axis_Type& start, const X_Axis_Type& end) const {
		 Y_Axis_Type out;
		 out = 0;
		 int n = GetNumValues();
		 if (n == 0) return out;
		 bool skipFirst = true;
		 AUTO g = lock.Guard();

		 auto iter = container.NodeFindSmallestLargerEqual(start);
		 if (iter) {
			 do {
				 if (iter->key > start) {
					 if (iter->key <= end) {
						 if (skipFirst) {
							 skipFirst = false;
							 out = *iter->object;
						 }
						 else {
							 if (*iter->object > out) out = *iter->object;
						 }
					 }
					 else {
						 break;
					 }
				 }
				 iter = container.GetNextLeaf(iter);
			 } while (iter);
		 }

		 return out;
	};;
	 Y_Axis_Type										GetAvgValue(const X_Axis_Type& start, const X_Axis_Type& end) const {
		 Y_Axis_Type out;
		 out = 0;
		 int num(0);
		 int n = GetNumValues();
		 if (n == 0) return out;

		 AUTO g = lock.Guard();

		 auto iter = container.NodeFindSmallestLargerEqual(start);
		 if (iter) {
			 do {
				 if (iter->key > start) {
					 if (iter->key <= end) {
						 num++;
						 out -= (out / (scalarT)num);
						 out += (*iter->object / (scalarT)num);
					 }
					 else {
						 break;
					 }
				 }
				 iter = container.GetNextLeaf(iter);
			 } while (iter);
		 }

		 return out;
	};;

	 X_Axis_Type										GetAvgTime() const {
		X_Axis_Type out(0);
		int num(0);

		AUTO g = lock.Guard();
		for (auto& x : container) {
			if (x.object) {
				num++;
				out -= (out / num);
				out += (x.key / num);
			}
		}

		return (X_Axis_Type)out;
	};;
	 X_Axis_Type										GetMaxTime(void) const {
		X_Axis_Type out(0);

		Lock();
		if (container.GetNodeCount() > 0) {
			auto ptr = container.NodeFindLargestSmallerEqual(std::numeric_limits<X_Axis_Type>::max());
			if (ptr) {
				out = ptr->key;
			}
		}
		Unlock();
		return out;
	};
	 X_Axis_Type										GetMinTime(void) const {
		X_Axis_Type out(0);

		Lock();
		if (container.GetNodeCount() > 0) {
			auto ptr = container.GetFirst(); //  NodeFindSmallestLargerEqual(-std::numeric_limits<X_Axis_Type>::max());
			if (ptr) {
				out = ptr->key;
			}
		}
		Unlock();
		return out;
	};

	 int												GetNumValues() const {
		int out; 
		Lock();
		out = container.GetNodeCount();
		Unlock();
		return out;
	};
	 void												ShiftTime(const X_Axis_Type& deltaTime) {
		AUTO g = lock.Guard();
		for (auto& x : container) {
			//if (x.object) {
				x.key += deltaTime;
			//}
		}
	};
	 void												Translate(const Y_Axis_Type& translation) {
		AUTO g = lock.Guard();
		for (auto& x : container) {
			if (x.object) {
				*x.object += translation;
			}
		}
	};

	 void												Copy(const cweeBalancedPattern& copy, const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits < X_Axis_Type>::max()) {
		SetBoundaryType(copy.GetBoundaryType());
		SetInterpolationType(copy.GetInterpolationType());

		AcceptNewData(copy(timeStart, timeEnd));

		Lock(); copy.Lock();
		container = copy.container;
		copy.Unlock(); Unlock();
	};

	auto& operator=(const cweeBalancedPattern<Y_Axis_Type>& source) {
		if (this == &source) return *this;
		Copy(source); 
		return *this;
	};
	auto& operator=(const cweeThreadedList<pairT>& data) {
		this->Clear();
		AcceptNewData(data);
		return *this;
	};
	auto& operator=(Y_Axis_Type data) {
		this->Clear();
		this->AddValue(0, data);
		return *this;
	};

	AUTO												R_Squared(const cweeBalancedPattern& other) const {
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
	cweeThreadedList<pairT>	operator()(const X_Axis_Type& start, const X_Axis_Type& end, const X_Axis_Type& resolution) const { return GetTimeSeries(start, end, resolution); };
	/*!
	Get Knot Series of Pattern between date-Times
	*/
	cweeThreadedList<pairT>	operator()(const X_Axis_Type& start, const X_Axis_Type& end) const { return GetKnotSeries(start, end); };
	/*!
	Get Spline value of Pattern at date-time
	*/
	Y_Axis_Type												operator()(const X_Axis_Type& time) const { return GetCurrentValue(time); };

	/* Patterns */		
	auto& operator*=(cweeBalancedPattern<units::dimensionless::scalar_t> const& b) {  *this = (*this * b); return *this; };
	/* non-pattern */			template<class RHS, class = std::enable_if_t<!cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>> 
	auto& operator*=(RHS const& b) { *this = (*this * b); return *this; };

	/* Patterns */
	auto& operator/=(cweeBalancedPattern<units::dimensionless::scalar_t> const& b) { *this = (*this * b); return *this; };
	/* non-pattern */			template<class RHS, class = std::enable_if_t<!cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>>
	auto& operator/=(RHS const& b) { *this = (*this * b); return *this; };

	/* Patterns */ template<class RHS>
	auto& operator+=(cweeBalancedPattern<RHS> const& b) { *this = (*this * b); return *this; };
	/* non-pattern */			template<class RHS, class = std::enable_if_t<!cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>>
	auto& operator+=(RHS const& b) { *this = (*this * b); return *this; };

	/* Patterns */ template<class RHS>
	auto& operator-=(cweeBalancedPattern<RHS> const& b) { *this = (*this * b); return *this; };
	/* non-pattern */			template<class RHS, class = std::enable_if_t<!cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>>
	auto& operator-=(RHS const& b) { *this = (*this * b); return *this; };

	/*! Clamp the y-axis Values such that they do not exceed the maximum and minimum Values. */
	void												ClampValues(const Y_Axis_Type& min, const Y_Axis_Type& max) {
		AUTO g = lock.Guard();
		for (auto& x : this->container) {
			*x.object = units::math::clamp(*x.object, min, max);
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
		 if constexpr (std::is_same<X_Axis_Type, u64>::value) {
			 cweeBalancedPattern< units::dimensionless::scalar_t, Y_Axis_Type> inverted;
			 for (node_type& x1 : this->container) {
				 inverted.AddValue(*x1.object, (units::dimensionless::scalar_t)x1.key);
			 }
			 return inverted.GetCurrentValue(val)();
		 }
		 else {
			 cweeBalancedPattern< X_Axis_Type, Y_Axis_Type> inverted;
			 for (node_type& x1 : this->container) {
				 inverted.AddValue(*x1.object, x1.key);
			 }
			 return inverted.GetCurrentValue(val);
		 }
	 };
	 /*! Return approximate derivative of spline at time */
	 AUTO												GetCurrentFirstDerivative(const X_Axis_Type& time) const {
#if 1
		 if constexpr (std::is_same<X_Axis_Type, u64>::value) {
			 X_Axis_Type step = 0.01;
			 return (GetCurrentValue(time + step) - GetCurrentValue(time - step)) / units::time::second_t(step * 2.0);
		 }
		 else {
			 X_Axis_Type step = 0.01;
			 return (GetCurrentValue(time + step) - GetCurrentValue(time - step)) / (step * 2.0);
		 }
#else
		 if constexpr (std::is_same<X_Axis_Type, u64>::value) {
			 AUTO v = (Y_Axis_Type)0 / units::time::second_t(1);
			 int i, j, k;
			 u64 bvals[4], d = 0, clampedTime;
			 units::time::second_t t;
			 units::time::second_t z = 0_s;

			 j = GetNumValues();
			 if (j > 1) {
				 clampedTime = this->ClampedTime(time);
				 clampedTime = this->LoopedTime(clampedTime);
				 BasisFirstDerivative(clampedTime, bvals);
				 {
					 AUTO g = lock.Guard();
					 node_type* x1 = container.NodeFindLargestSmallerEqual(clampedTime);
					 node_type* x0 = x1 == nullptr ? (node_type*)nullptr : container.GetPrevLeaf(x1);
					 node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
					 node_type* x3 = x2 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x2);

					 if (x1 && x2) {
						 if (x0 && x1 && x2 && x3) {
							 // best case scenario

							 if (x3) { t = (units::time::second_t)(double)x3->key; }
							 else if (x2) { t = (units::time::second_t)(double)x2->key; }
							 else if (x1) { t = (units::time::second_t)(double)x1->key; }
							 else if (x0) { t = (units::time::second_t)(double)x0->key; }

							 if (x0) { t -= (units::time::second_t)(double)x0->key; }
							 else if (x1) { t -= (units::time::second_t)(double)x1->key; }
							 else if (x2) { t -= (units::time::second_t)(double)x2->key; }
							 else if (x3) { t -= (units::time::second_t)(double)x3->key; }

							 if (t > z) {
								 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / t);
								 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
								 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
								 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / t);
							 }
						 }
						 else if (!x0 && x1 && x2 && !x3) {
							 // report the slope between the two values
							 v = (*x2->object - *x1->object) / (units::time::second_t)(double)(x2->key - x1->key);
						 }
						 else if (!x0 && x1 && x2 && x3) {
							 // estimate the first value
							 X_Axis_Type x0_key; {
								 cweeThreadedList<double> list;
								 list.Append((double)x3->key);
								 list.Append((double)x2->key);
								 list.Append((double)x1->key);
								 x0_key = alglibwrapper::Interpolator::PredictNextInSequence(list, 1)[0];
							 }
							 Y_Axis_Type x0_object = GetCurrentValue(x0_key);

							 if (x3) { t = (units::time::second_t)(double)x3->key; }
							 else if (x2) { t = (units::time::second_t)(double)x2->key; }
							 else if (x1) { t = (units::time::second_t)(double)x1->key; }
							 else { t = (units::time::second_t)(double)x0_key; }
							 t -= (units::time::second_t)(double)x0_key;

							 if (t > z) {
								 if (x0) v += (x0_object) * ((units::dimensionless::scalar_t)bvals[0] / t);
								 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
								 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
								 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / t);
							 }
						 }
						 else if (x0 && x1 && x2 && !x3) {
							 // estimate the last value
							 X_Axis_Type x3_key; {
								 cweeThreadedList<double> list;
								 list.Append((double)x0->key);
								 list.Append((double)x1->key);
								 list.Append((double)x2->key);
								 x3_key = alglibwrapper::Interpolator::PredictNextInSequence(list, 1)[0];
							 }
							 Y_Axis_Type x3_object = GetCurrentValue(x3_key);

							 t = (units::time::second_t)(double)x3_key;

							 if (x0) { t -= (units::time::second_t)(double)x0->key; }
							 else if (x1) { t -= (units::time::second_t)(double)x1->key; }
							 else if (x2) { t -= (units::time::second_t)(double)x2->key; }
							 else { t -= (units::time::second_t)(double)x3_key; }

							 if (t > z) {
								 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / t);
								 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
								 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
								 if (x3) v += (x3_object) * ((units::dimensionless::scalar_t)bvals[3] / t);
							 }
						 }
					 }
					 else {
						 if (x1 && !x2) {
							 if (x0) {
								 AUTO step = x1->key - x0->key;

								 Y_Axis_Type x2_object = GetCurrentValue(x1->key + step);
								 Y_Axis_Type x3_object = GetCurrentValue(x1->key + step + step);

								 t = (units::time::second_t)(double)(x1->key + step + step - x0->key);

								 if (t > z) {
									 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / t);
									 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
									 if (x2) v += (x2_object) * ((units::dimensionless::scalar_t)bvals[2] / t);
									 if (x3) v += (x3_object) * ((units::dimensionless::scalar_t)bvals[3] / t);
								 }
							 }
							 else {
								 // x1 is on its own.
								 v = 0.0;
							 }
						 }
						 else if (!x1 && x2) {
							 if (x3) {
								 AUTO step = x3->key - x2->key;

								 Y_Axis_Type x0_object = GetCurrentValue((x2->key - step) - step);
								 Y_Axis_Type x1_object = GetCurrentValue(x2->key - step);

								 t = (units::time::second_t)(double)(x3->key - ((x2->key - step) - step));

								 if (t > z) {
									 if (x0) v += (x0_object) * ((units::dimensionless::scalar_t)bvals[0] / t);
									 if (x1) v += (x1_object) * ((units::dimensionless::scalar_t)bvals[1] / t);
									 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
									 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / t);
								 }
							 }
							 else {
								 // x2 is on its own.
								 v = 0.0;
							 }
						 }
						 else {
							 // no good values.
							 v = 0.0;
						 }
					 }
				 }
			 }
			 return v;
		 }
		 else {
			 AUTO v = (Y_Axis_Type)0 / X_Axis_Type(1);
			 int i, j, k;
			 u64 bvals[4];
			 X_Axis_Type d = 0, clampedTime;
			 X_Axis_Type z = 0;

			 j = GetNumValues();
			 if (j > 1) {
				 clampedTime = this->ClampedTime(time);
				 clampedTime = this->LoopedTime(clampedTime);
				 BasisFirstDerivative(clampedTime, bvals);
				 {
					 AUTO g = lock.Guard();
					 node_type* x1 = container.NodeFindLargestSmallerEqual(clampedTime);
					 node_type* x0 = x1 == nullptr ? (node_type*)nullptr : container.GetPrevLeaf(x1);
					 node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
					 node_type* x3 = x2 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x2);

					 if (x1 && x2) {					
						 if (x0 && x1 && x2 && x3) {
							 // best case scenario

							 X_Axis_Type t;
							 if (x3) { t = x3->key; }
							 else if (x2) { t = x2->key; }
							 else if (x1) { t = x1->key; }
							 else if (x0) { t = x0->key; }

							 if (x0) { t -= x0->key; }
							 else if (x1) { t -= x1->key; }
							 else if (x2) { t -= x2->key; }
							 else if (x3) { t -= x3->key; }

							 if (t > z) {
								 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / t);
								 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
								 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
								 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / t);
							 }
						 }
						 else if (!x0 && x1 && x2 && !x3) { 
							 // report the slope between the two values
							 v = (*x2->object - *x1->object) / (x2->key - x1->key);
						 }
						 else if (!x0 && x1 && x2 && x3) {
							 // estimate the first value
							 X_Axis_Type x0_key; {
								 cweeThreadedList<double> list;
								 list.Append((double)x3->key);
								 list.Append((double)x2->key);
								 list.Append((double)x1->key);
								 x0_key = alglibwrapper::Interpolator::PredictNextInSequence(list, 1)[0];
							 }
							 Y_Axis_Type x0_object = GetCurrentValue(x0_key);

							 X_Axis_Type t;
							 if (x3) { t = x3->key; }
							 else if (x2) { t = x2->key; }
							 else if (x1) { t = x1->key; }
							 else  { t = x0_key; }
							 t -= x0_key;

							 if (t > z) {
								 if (x0) v += (x0_object) * ((units::dimensionless::scalar_t)bvals[0] / t);
								 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
								 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
								 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / t);
							 }
						 } 
						 else if (x0 && x1 && x2 && !x3) {
							 // estimate the last value
							 X_Axis_Type x3_key; {
								 cweeThreadedList<double> list;
								 list.Append((double)x0->key);
								 list.Append((double)x1->key);
								 list.Append((double)x2->key);
								 x3_key = alglibwrapper::Interpolator::PredictNextInSequence(list, 1)[0];
							 }
							 Y_Axis_Type x3_object = GetCurrentValue(x3_key);

							 X_Axis_Type t;
							 t = x3_key;

							 if (x0) { t -= x0->key; }
							 else if (x1) { t -= x1->key; }
							 else if (x2) { t -= x2->key; }
							 else { t -= x3_key; }

							 if (t > z) {
								 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / t);
								 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
								 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
								 if (x3) v += (x3_object) * ((units::dimensionless::scalar_t)bvals[3] / t);
							 }
						 }
					 }
					 else {
						 if (x1 && !x2) {
							 if (x0) {
								 AUTO step = x1->key - x0->key;

								 Y_Axis_Type x2_object = GetCurrentValue(x1->key + step);
								 Y_Axis_Type x3_object = GetCurrentValue(x1->key + step + step);

								 X_Axis_Type t = x1->key + step + step - x0->key;

								 if (t > z) {
									 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / t);
									 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / t);
									 if (x2) v += (x2_object) * ((units::dimensionless::scalar_t)bvals[2] / t);
									 if (x3) v += (x3_object) * ((units::dimensionless::scalar_t)bvals[3] / t);
								 }
							 }
							 else {
								 // x1 is on its own.
								 v = 0.0;
							 }
						 }
						 else if (!x1 && x2) {
							 if (x3) {
								 AUTO step = x3->key - x2->key;

								 Y_Axis_Type x0_object = GetCurrentValue((x2->key - step) - step);
								 Y_Axis_Type x1_object = GetCurrentValue(x2->key - step);

								 X_Axis_Type t = x3->key - ((x2->key - step) - step);

								 if (t > z) {
									 if (x0) v += (x0_object) * ((units::dimensionless::scalar_t)bvals[0] / t);
									 if (x1) v += (x1_object) * ((units::dimensionless::scalar_t)bvals[1] / t);
									 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / t);
									 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / t);
								 }
							 }
							 else {
								 // x2 is on its own.
								 v = 0.0;
							 }
						 }
						 else {
							 // no good values.
							 v = 0.0;
						 }
					 }					
				 }				 
			 }
			 return v;
		 }
#endif
	 };

	 /*! Return approximate second derivative of spline at time */
	 AUTO												GetCurrentSecondDerivative(const X_Axis_Type& time) const {	
#if 1
		 if constexpr (std::is_same<X_Axis_Type, u64>::value) {
			 X_Axis_Type step = 0.01;
			 return (GetCurrentFirstDerivative(time + step) - GetCurrentFirstDerivative(time - step)) / units::time::second_t(step * 2.0);
		 }
		 else {
			 X_Axis_Type step = 0.01;
			 return (GetCurrentFirstDerivative(time + step) - GetCurrentFirstDerivative(time - step)) / (step * 2.0);
		 }
#else
		 if constexpr (std::is_same<X_Axis_Type, u64>::value) {
			 AUTO v = (Y_Axis_Type)0 / (units::time::second_t(1) * units::time::second_t(1));
			 int i, j, k;
			 u64 bvals[4], d = 0, clampedTime;

			 j = GetNumValues();
			 if (j <= 1) {
				 return v;
			 }
			 else {
				 clampedTime = this->ClampedTime(time);
				 clampedTime = this->LoopedTime(clampedTime);
				 BasisSecondDerivative(clampedTime, bvals);
				 {
					 AUTO g = lock.Guard();
					 node_type* x1 = container.NodeFindLargestSmallerEqual(clampedTime);
					 node_type* x0 = x1 == nullptr ? (node_type*)nullptr : container.GetPrevLeaf(x1);
					 node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
					 node_type* x3 = x2 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x2);

					 AUTO t = units::time::second_t(1) * units::time::second_t(1);
					 if (x0 && x3) {
						 t = units::time::second_t(x3->key - x0->key) * units::time::second_t(x3->key - x0->key);
					 }
					 if (x0 && x2) {
						 t = units::time::second_t(x2->key - x0->key) * units::time::second_t(x2->key - x0->key);
					 }
					 if (x0 && x1) {
						 t = units::time::second_t(x1->key - x0->key) * units::time::second_t(x1->key - x0->key);
					 }
					 if (x1 && x0) {
						 v += ((*x0->object * (scalarT)bvals[0]) / t);
						 v += ((*x1->object * (scalarT)bvals[1]) / t);
						 if (x2) v += ((*x2->object * (scalarT)(u64)bvals[2]) / t); else v += ((*x1->object * (scalarT)(u64)bvals[2]) / t);
						 if (x2) v += ((*x3->object * (scalarT)(u64)bvals[3]) / t); else v += ((*x1->object * (scalarT)(u64)bvals[3]) / t);
					 }
					 else {
						 return v;
					 }
				 }
				 return v;
			 }
			 return v;
		 }
		 else {
			 AUTO v = (Y_Axis_Type)0 / (X_Axis_Type(1) * X_Axis_Type(1));
			 int i, j, k;
			 u64 bvals[4];
			 X_Axis_Type d = 0, clampedTime;
			 X_Axis_Type z = 0;

			 j = GetNumValues();
			 if (j > 1) {
				 clampedTime = this->ClampedTime(time);
				 clampedTime = this->LoopedTime(clampedTime);
				 BasisSecondDerivative(clampedTime, bvals);
				 {
					 AUTO g = lock.Guard();
					 node_type* x1 = container.NodeFindLargestSmallerEqual(clampedTime);
					 node_type* x0 = x1 == nullptr ? (node_type*)nullptr : container.GetPrevLeaf(x1);
					 node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
					 node_type* x3 = x2 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x2);

					 X_Axis_Type t;
					 if (x3) { t = x3->key; }
					 else if (x2) { t = x2->key; }
					 else if (x1) { t = x1->key; }
					 else if (x0) { t = x0->key; }

					 if (x0) { t -= x0->key; }
					 else if (x1) { t -= x1->key; }
					 else if (x2) { t -= x2->key; }
					 else if (x3) { t -= x3->key; }

					 AUTO T = t * t;

					 if (T > z) {
						 if (x0) v += (*x0->object) * ((units::dimensionless::scalar_t)bvals[0] / T);
						 if (x1) v += (*x1->object) * ((units::dimensionless::scalar_t)bvals[1] / T);
						 if (x2) v += (*x2->object) * ((units::dimensionless::scalar_t)bvals[2] / T);
						 if (x3) v += (*x3->object) * ((units::dimensionless::scalar_t)bvals[3] / T);
					 }
				 }
			 }
			 return v;
		 }
#endif
	 };

	 cweeThreadedList<pairT>			GetKnotSeries(const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits <X_Axis_Type>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<pairT> out(numKnots + 16);

		AUTO g = lock.Guard();
		for (auto ptr = container.NodeFindSmallestLargerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
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

	 cweeThreadedList<pairT>			GetReversedKnotSeries(const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits <X_Axis_Type>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<pairT> out(numKnots + 16);

		AUTO g = lock.Guard();
		for (auto ptr = container.NodeFindLargestSmallerEqual(timeEnd); ptr; ptr = container.GetPrevLeaf(ptr)) {
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
	 cweeThreadedList<pairT>			GetTimeSeries(const X_Axis_Type& timeStart, const X_Axis_Type& timeEnd, const X_Axis_Type& timeStep) const {
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

	 cweeThreadedList<Y_Axis_Type>							GetValueKnotSeries(const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits < X_Axis_Type>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<Y_Axis_Type> out(numKnots + 16);

		AUTO g = lock.Guard();
		for (auto ptr = container.NodeFindSmallestLargerEqual(timeStart); ptr; ptr = container.GetNextLeaf(ptr)) {
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

	 cweeThreadedList<Y_Axis_Type>							GetValueTimeSeries(const X_Axis_Type& timeStart, const X_Axis_Type& timeEnd, const X_Axis_Type& timeStep) const {
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
	template <typename timeFactor = units::time::second_t>
	AUTO												RombergIntegral(const X_Axis_Type& t0, const X_Axis_Type& t1) const {
		auto sum = Y_Axis_Type(0) * timeFactor(0);
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

							sum += ((*right.object + *left.object) * scalarT(0.5)) * units::time::second_t(right.key - left.key);
						}
					}
				}
			}

			X_Axis_Type
				t = 0,
				stepDiv2 = step / 2.0,
				maxT = t1 + stepDiv2;

			for (t = t0; (t + step) < minGot; t += step) sum += units::time::second_t(step) * GetCurrentValue(t + stepDiv2);
			if (minGot > t) sum += units::time::second_t(minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
			for (t = maxGot; (t + step) < t1; t += step) sum += units::time::second_t(step) * GetCurrentValue(t + stepDiv2);
			if (t1 > t)  sum += units::time::second_t(t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
		}
		return sum;		
	};

	/*! <X,X,X> = pattern.ValueQuantiles({ 0.25, 0.5, 0.75 }); */
	 cweeThreadedList<Y_Axis_Type>							ValueQuantiles(const cweeThreadedList<float>& probs, const X_Axis_Type& timeStart = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& timeEnd = std::numeric_limits < X_Axis_Type>::max(), int numSamples = -1) {
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

		if (data.Num() < probs.Num()) {
			data.AssureSize(probs.Num());
		}		
		if (data.Num() <= 1) return data;
		
		data.Sort(); // sort data

		for (size_t i = 0; i < probs.Num(); ++i)
		{
			poi = (1.0f - probs[i]) * -0.5f + probs[i] * ((float)data.Num() - 0.5f);
			left = std::max(int64_t(std::floor((double)poi)), int64_t(0));
			right = std::min(int64_t(std::ceil((double)poi)), int64_t((float)data.Num() - 1.0f));
			quantiles.Append((((scalarT)1.0f - (poi - (scalarT)left)) * data[left]) + ((poi - (scalarT)left) * data[right]));
		}

		return quantiles;
	};

	 void												RemoveUnnecessaryKnots(const X_Axis_Type& start = -std::numeric_limits < X_Axis_Type>::max(), const X_Axis_Type& end = std::numeric_limits<X_Axis_Type>::max()) const {
		if (start >= end) return;
		int num, index; cweeThreadedList<X_Axis_Type> keysToDelete;
		constexpr Y_Axis_Type epsilon = 0.001f;
		Y_Axis_Type* val1 = nullptr, *val2 = nullptr, *val3 = nullptr, *val4 = nullptr, *val5 = nullptr;
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
						if (units::math::fabs(*val1 - *val2) > epsilon) { // !=
							++index;
							val1 = val2; t1 = t2;
							val2 = val3; t2 = t3;
							val3 = val4; t3 = t4;
							val4 = val5; t4 = t5;
							val5 = knot.object;
							t5 = knot.key;
							continue;
						}
						else if (units::math::fabs(*val5 - *val2) <= epsilon) { // ==
							if (units::math::fabs(*val4 - *val2) <= epsilon) { // ==
								if (units::math::fabs(*val3 - *val2) <= epsilon) { // ==
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
		Y_Axis_Type minValue  = this->GetMinValue();
		Y_Axis_Type epsilon = units::math::fabs(maxValue - minValue) * (units::dimensionless::scalar_t)(((float)percentToRemove) / 100.0f);
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
							(units::math::fabs(*val2 - *val3) <= epsilon)
							&& (units::math::fabs(*val4 - *val3) <= epsilon)
							&& (units::math::fabs(*val2 - *val4) <= epsilon)
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
	 void												AcceptChanges(const cweeBalancedPattern<Y_Axis_Type>& copy) {
		SetBoundaryType(copy.GetBoundaryType());
		SetInterpolationType(copy.GetInterpolationType());

		Lock(); copy.Lock();
		container = copy.container;
		copy.Unlock(); Unlock();

		AcceptNewData(copy.GetKnotSeries());
	};

	 Y_Axis_Type										GetCurrentMovingAverage(const X_Axis_Type& time) const {
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

	/*! Get a pattern (copy of this pattern) whose Y-Values are conversions of its X-Values */
	 cweeBalancedPattern<Y_Axis_Type>					GetTimePattern(bool returnHour = true) const {
		cweeBalancedPattern<Y_Axis_Type> out;
		cweeTime tmp;
		if (returnHour == true) {
			// return 0 - 23 value representing the hour (0 is midnight)
			AUTO g = lock.Guard();
			for (auto& x : container) {
				if (x.object) {
					tmp = cweeTime(x.key);
					out.AddValue(x.key, tmp.tm_hour());
				}
			}
		}
		else {
			// return 0 - 6 value representing the day of the week (0 is Sunday)
			AUTO g = lock.Guard();
			for (auto& x : container) {
				if (x.object) {
					tmp = cweeTime(x.key);
					out.AddValue(x.key, tmp.tm_wday());
				}
			}
		}
		return out;
	};

	static int											GetMonthOfYear(const X_Axis_Type& time) { return cweeTime(time).tm_mon(); };
	static int											GetDayOfMonth(const X_Axis_Type& time) { return cweeTime(time).tm_mday(); };
	static int											GetDayOfWeek(const X_Axis_Type& time) {	return cweeTime(time).tm_wday(); };
	static float										GetHourOfDay(const X_Axis_Type& time) { cweeTime tmp = cweeTime(time); return (((float)tmp.tm_hour()) + (((float)tmp.tm_min()) / 60.0f) + (((float)tmp.tm_sec()) / 3600.0f)); };
	static int											GetSeason(const X_Axis_Type& time) {
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
	};;
	scalarT												GetNormalizedValue(const X_Axis_Type& time) const {
		AUTO min = this->GetMinValue();
		AUTO max = this->GetMaxValue();
		if (min == max) max = min + (Y_Axis_Type)1.0f;
		return (GetCurrentValue(time) - min) / (max - min);
	};

	/*! Clear all knots and accept the new, incoming data Values from the list of knots. */
	cweeBalancedPattern<Y_Axis_Type>					DuplicateWithNewData(const cweeThreadedList<pairT>& in) {
		cweeBalancedPattern<Y_Axis_Type> out;
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
	 X_Axis_Type										LoopedTime(const X_Axis_Type& t, bool forceLoop = false) const {
#if 1
		 if (forceLoop || IsLooped()){
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
#else

		 if (IsLooped() || forceLoop) {
			 X_Axis_Type minTime, maxTime, len, currentTime;
			 minTime = GetMinTime();
			 if (cweeMath::Fabs((u64)(t - minTime)) < 1.01) return t;
			 maxTime = GetMaxTime();
			 if (cweeMath::Fabs((u64)(t - maxTime)) < 1.01) return t;
			 len = (maxTime - minTime);
			 if ((u64)len > 0) {
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
#endif
			//if (IsLooped() || forceLoop) {
			//	X_Axis_Type minTime, maxTime, len, currentTime, z;
			//	z = 0;
			//	minTime = GetMinTime();
			//	maxTime = GetMaxTime();
			//	len = maxTime - minTime;
			//	if (len > z) {
			//		if constexpr (std::is_same<X_Axis_Type, u64>::value) {
			//			return std::fmod((-minTime + std::fmod(t, len) + len), len) + minTime;
			//		}
			//		else {
			//			return units::math::fmod((-minTime + units::math::fmod(t, len) + len), len) + minTime;
			//		}
			//	}
			//}
			//return t;
		
		return t;
	};;
	 X_Axis_Type										ClampedTime(const X_Axis_Type& t) const {
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
	};;
	 static cweeStr										SerializeTime(X_Axis_Type t) {
		constexpr double div = 0.001;
		t /= div;
		return cweeStr(t);
	};;
	 static X_Axis_Type									DeserializeTime(const cweeStr& t) {
		constexpr double mul = 1 / 0.001;
		X_Axis_Type out = (X_Axis_Type)t;
		out *= mul;
		return out;
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

/* Patterns, shared types */		template<class LHS> static INLINE AUTO operator*(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<LHS> const& b) {
	// both are patterns!
	AUTO v = a.GetCurrentValue(0) * b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object * b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object * a.GetCurrentValue(x.key));
		}
		b.Unlock();
	}
	return result;
};
/* Pattern & non-pattern */			template<class LHS, class RHS, class = std::enable_if_t<cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value != cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>> static INLINE AUTO operator*(LHS const& a, RHS const& b) {
	// one of these are a pattern - but not both. 
	if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value) { // LHS is the pattern
		if constexpr (units::traits::is_unit_t<RHS>::value) {
			// RHS is a unit type
			AUTO v = a.GetCurrentValue(0) * b;
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object * b);
				}
				a.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<RHS>::value) {
			// RHS is a float or double or int
			AUTO v = a.GetCurrentValue(0) * units::dimensionless::scalar_t(b);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object * units::dimensionless::scalar_t(b));
				}
				a.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
	else if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value) { // RHS is the pattern
		if constexpr (units::traits::is_unit_t<LHS>::value) {
			// LHS is a unit type
			AUTO v = a * b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object * a);
				}
				b.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<LHS>::value) {
			// LHS is a float or double or int
			AUTO v = b.GetCurrentValue(0) * units::dimensionless::scalar_t(a);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object * units::dimensionless::scalar_t(a));
				}
				b.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
};
/* Patterns, non-shared types */	template<class LHS, class RHS, class = std::enable_if_t<!std::is_same_v<LHS, RHS>>> static INLINE AUTO operator*(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<RHS> const& b) {
	// both are patterns with different types 
	AUTO v = a.GetCurrentValue(0) * b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object * b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object * a.GetCurrentValue(x.key));
		}
		b.Unlock();
	}
	return result;
};

/* Patterns, shared types */		template<class LHS> static INLINE AUTO operator/(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<LHS> const& b) {
	// both are patterns!
	AUTO v = a.GetCurrentValue(0) / b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object / b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, a.GetCurrentValue(x.key) / *x.object);
		}
		b.Unlock();
	}
	return result;
};
/* Pattern & non-pattern */			template<class LHS, class RHS, class = std::enable_if_t<cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value != cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>> static INLINE AUTO operator/(LHS const& a, RHS const& b) {
	// one of these are a pattern - but not both. 
	if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value) { // LHS is the pattern
		if constexpr (units::traits::is_unit_t<RHS>::value) {
			// RHS is a unit type
			AUTO v = a.GetCurrentValue(0) / b;
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object / b);
				}
				a.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<RHS>::value) {
			// RHS is a float or double or int
			AUTO v = a.GetCurrentValue(0) / units::dimensionless::scalar_t(b);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object / units::dimensionless::scalar_t(b));
				}
				a.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
	else if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value) { // RHS is the pattern
		if constexpr (units::traits::is_unit_t<LHS>::value) {
			// LHS is a unit type
			AUTO v = a / b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, a / *x.object);
				}
				b.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<LHS>::value) {
			// LHS is a float or double or int
			AUTO v = units::dimensionless::scalar_t(a) / b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, units::dimensionless::scalar_t(a) / *x.object);
				}
				b.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
};
/* Patterns, non-shared types */	template<class LHS, class RHS, class = std::enable_if_t<!std::is_same_v<LHS, RHS>>> static INLINE AUTO operator/(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<RHS> const& b) {
	// both are patterns with different types 
	AUTO v = a.GetCurrentValue(0) / b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object / b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, a.GetCurrentValue(x.key) / *x.object);
		}
		b.Unlock();
	}
	return result;
};

/* Patterns, shared types */		template<class LHS> static INLINE AUTO operator+(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<LHS> const& b) {
	// both are patterns!
	AUTO v = a.GetCurrentValue(0) + b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object + b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object + a.GetCurrentValue(x.key));
		}
		b.Unlock();
	}
	return result;
};
/* Pattern & non-pattern */			template<class LHS, class RHS, class = std::enable_if_t<cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value != cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>> static INLINE AUTO operator+(LHS const& a, RHS const& b) {
	// one of these are a pattern - but not both. 
	if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value) { // LHS is the pattern
		if constexpr (units::traits::is_unit_t<RHS>::value) {
			// RHS is a unit type
			AUTO v = a.GetCurrentValue(0) + b;
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object + b);
				}
				a.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<RHS>::value) {
			// RHS is a float or double or int
			AUTO v = a.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object + (decltype(v))b);
				}
				a.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
	else if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value) { // RHS is the pattern
		if constexpr (units::traits::is_unit_t<LHS>::value) {
			// LHS is a unit type
			AUTO v = a + b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object + a);
				}
				b.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<LHS>::value) {
			// LHS is a float or double or int
			AUTO v = b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object + (decltype(v))a);
				}
				b.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
};
/* Patterns, non-shared types */	template<class LHS, class RHS, class = std::enable_if_t<!std::is_same_v<LHS, RHS>>> static INLINE AUTO operator+(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<RHS> const& b) {
	// both are patterns with different types 
	AUTO v = a.GetCurrentValue(0) + b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object + b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object + a.GetCurrentValue(x.key));
		}
		b.Unlock();
	}
	return result;
};

/* Patterns, shared types */		template<class LHS> static INLINE AUTO operator-(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<LHS> const& b) {
	// both are patterns!
	AUTO v = a.GetCurrentValue(0) - b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object - b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object - a.GetCurrentValue(x.key));
		}
		b.Unlock();
	}
	return result;
};
/* Pattern & non-pattern */			template<class LHS, class RHS, class = std::enable_if_t<cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value != cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value>> static INLINE AUTO operator-(LHS const& a, RHS const& b) {
	// one of these are a pattern - but not both. 
	if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<LHS>::value) { // LHS is the pattern
		if constexpr (units::traits::is_unit_t<RHS>::value) {
			// RHS is a unit type
			AUTO v = a.GetCurrentValue(0) - b;
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object - b);
				}
				a.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<RHS>::value) {
			// RHS is a float or double or int
			AUTO v = a.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(a.GetGranularity());
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (true) {
				a.Lock();
				for (auto& x : a.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object - (decltype(v))b);
				}
				a.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
	else if constexpr (cweeBalancedPatternDetail::is_balanced_pattern_t<RHS>::value) { // RHS is the pattern
		if constexpr (units::traits::is_unit_t<LHS>::value) {
			// LHS is a unit type
			AUTO v = a - b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object - a);
				}
				b.Unlock();
			}
			return result;
		}
		else if constexpr (std::is_arithmetic<LHS>::value) {
			// LHS is a float or double or int
			AUTO v = b.GetCurrentValue(0);
			cweeBalancedPattern<decltype(v)> result; {
				result.SetGranularity(b.GetGranularity());
				result.SetBoundaryType(b.GetBoundaryType());
				result.SetInterpolationType(b.GetInterpolationType());
			}
			if (true) {
				b.Lock();
				for (auto& x : b.UnsafeGetValues()) {
					result.AddUniqueValue(x.key, *x.object - (decltype(v))a);
				}
				b.Unlock();
			}
			return result;
		}
		else {
			static_assert(false, "Cannot perform the requested arithmetic with the given types.");
			return 0;
		}
	}
};
/* Patterns, non-shared types */	template<class LHS, class RHS, class = std::enable_if_t<!std::is_same_v<LHS, RHS>>> static INLINE AUTO operator-(cweeBalancedPattern<LHS> const& a, cweeBalancedPattern<RHS> const& b) {
	// both are patterns with different types 
	AUTO v = a.GetCurrentValue(0) - b.GetCurrentValue(0);
	cweeBalancedPattern<decltype(v)> result; {
		result.SetGranularity(a.GetGranularity());
		result.SetBoundaryType(a.GetBoundaryType());
		result.SetInterpolationType(a.GetInterpolationType());
	}
	if (true) {
		a.Lock();
		for (auto& x : a.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object - b.GetCurrentValue(x.key));
		}
		a.Unlock();
		b.Lock();
		for (auto& x : b.UnsafeGetValues()) {
			result.AddUniqueValue(x.key, *x.object - a.GetCurrentValue(x.key));
		}
		b.Unlock();
	}
	return result;
};