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
#include "cweeUnitedValue.h"
#include "BalancedPattern.h"
#include "Iterator.h"
#include "RTree.h"

//namespace cweeUnitValues {
class cweeUnitPatternContainer_t {
public:
	mutable cweeUnitValues::unit_value											internal_X_type;
	mutable cweeUnitValues::unit_value											internal_Y_type;

	class GenericIterator {
	public:
		class DataContainer {
		public:
			DataContainer() : X(), Y(nullptr) {};
			DataContainer(cweeUnitValues::unit_value x) : X(x), Y(nullptr) {};
			DataContainer(cweeUnitValues::unit_value x, cweeUnitValues::unit_value y) : X(x), Y(y) {};

			DataContainer(DataContainer const& other) : X(other.X), Y(other.Y) {};
			DataContainer(DataContainer&& other) : X(std::forward<decltype(X)>(other.X)), Y(std::forward<decltype(Y)>(other.Y)) {};

			DataContainer& operator=(DataContainer const& other) {
				this->X = other.X;
				this->Y = other.Y;
				return *this;
			};

			cweeUnitValues::unit_value X;
			cweeSharedPtr<cweeUnitValues::unit_value> Y;
		};

		cweeSharedPtr<DataContainer> thisContainer;

		GenericIterator() : thisContainer() {};
		virtual ~GenericIterator() {};
		virtual void begin(cweeUnitPatternContainer_t const* ref) = 0;
		virtual void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) = 0;
		virtual void next(cweeUnitPatternContainer_t const* ref) = 0;
		virtual void end(cweeUnitPatternContainer_t const* ref) = 0;
		virtual DataContainer& get(cweeUnitPatternContainer_t const* ref) = 0;
		virtual const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const = 0;
		virtual bool cmp(const cweeSharedPtr<GenericIterator>& s) const = 0;
		virtual long long distance(const cweeSharedPtr<GenericIterator>& s) const = 0;
		virtual void prev(const cweeUnitPatternContainer_t* ref) = 0;
		virtual const DataContainer& get(const cweeUnitPatternContainer_t* ref) const = 0;
	};
	class GenericConstIterator {
	public:
		class DataContainer {
		public:
			DataContainer() : X(), Y_actual(), Y(nullptr) {};
			DataContainer(cweeUnitValues::unit_value x) : X(x), Y_actual(), Y(nullptr) {};
			DataContainer(cweeUnitValues::unit_value x, cweeUnitValues::unit_value y) : X(x), Y_actual(y), Y(&Y_actual) {};

			DataContainer(DataContainer const& other) : X(other.X), Y_actual(other.Y_actual), Y(nullptr) {
				if (other.Y) { Y = &Y_actual; }
			};
			DataContainer(DataContainer&& other) : X(std::forward<decltype(X)>(other.X)), Y_actual(other.Y_actual), Y(nullptr) {
				if (other.Y) { Y = &Y_actual; }
			};

			DataContainer& operator=(DataContainer const& other) {
				this->X = other.X;
				this->Y_actual = other.Y_actual;

				if (other.Y) this->Y = &Y_actual;

				return *this;
			};

			cweeUnitValues::unit_value X;
			cweeUnitValues::unit_value* Y;
			cweeUnitValues::unit_value Y_actual;
		};

		cweeSharedPtr<DataContainer> thisContainer;

		GenericConstIterator() : thisContainer() {};
		virtual ~GenericConstIterator() {};
		virtual void begin(cweeUnitPatternContainer_t const* ref) = 0;
		virtual void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) = 0;
		virtual void next(cweeUnitPatternContainer_t const* ref) = 0;
		virtual void end(cweeUnitPatternContainer_t const* ref) = 0;
		virtual DataContainer& get(cweeUnitPatternContainer_t const* ref) = 0;
		virtual const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const = 0;
		virtual bool cmp(const cweeSharedPtr<GenericConstIterator>& s) const = 0;
		virtual long long distance(const cweeSharedPtr<GenericConstIterator>& s) const = 0;
		virtual void prev(const cweeUnitPatternContainer_t* ref) = 0;
		virtual const DataContainer& get(const cweeUnitPatternContainer_t* ref) const = 0;
	};

	virtual cweeSharedPtr<GenericIterator> CreateIterationState() const = 0;
	virtual cweeSharedPtr<GenericConstIterator> CreateConstIterationState() const = 0;

	using ParentClass = cweeUnitPatternContainer_t;
	using IterType = GenericIterator::DataContainer;
	using ConstIterType = GenericConstIterator::DataContainer;

	typedef std::ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef IterType value_type;
	typedef IterType* pointer;
	typedef const IterType* const_pointer;
	typedef IterType& reference;
	typedef const IterType& const_reference;

	class iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {
	public:
		const ParentClass* ref;
		cweeSharedPtr<GenericIterator> state;
		iterator() : ref(nullptr), state(nullptr) {};
		iterator(const ParentClass* parent, cweeSharedPtr<GenericIterator> State) : ref(parent), state(State) {};
		iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) if (state) state->next(ref); return *this; };
		iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) if (state) state->prev(ref); return *this; };
		difference_type operator-(iterator const& other) { if (state) return state->distance(other.state); return 0; };
		iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) if (state) state->prev(ref); return *this; };
		iterator& operator--() { if (state) state->prev(ref); return *this; };
		iterator operator--(int) { iterator retval = *this; --(*this); return retval; };
		iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) if (state) state->next(ref); return *this; };
		iterator& operator++() { if (state) state->next(ref); return *this; };
		iterator operator++(int) { iterator retval = *this; ++(*this); return retval; };
		bool operator==(iterator const& other) const { return !(operator!=(other)); };
		bool operator!=(iterator const& other) const { if (state) return (ref != other.ref || state->cmp(other.state)); return false; };
		reference operator*() { return const_cast<reference>(state->get(ref)); };
		pointer operator->() { return const_cast<pointer>(&state->get(ref)); };
		auto& operator*() const { return state->cget(ref); };
		auto* operator->() const { return &state->cget(ref); };
		iterator& begin() { if (state) state->begin(ref); return *this; };
		iterator& begin_at(cweeUnitValues::unit_value x) { if (state) state->begin_at(ref, x); return *this; };
		iterator& end() { if (state) state->end(ref); return *this; };
	};
	class const_iterator : public std::iterator<std::random_access_iterator_tag,
		GenericConstIterator::DataContainer,
		difference_type,
		const GenericConstIterator::DataContainer*,
		const GenericConstIterator::DataContainer&
	> {
	public:
		const ParentClass* ref;
		cweeSharedPtr<GenericConstIterator> state;
		const_iterator() : ref(nullptr), state(nullptr) {};
		const_iterator(const ParentClass* parent, cweeSharedPtr<GenericConstIterator> State) : ref(parent), state(State) {};
		const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) if (state) state->next(ref); return *this; };
		const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) if (state) state->prev(ref); return *this; };
		difference_type operator-(const_iterator const& other) { if (state) return state->distance(other.state); return 0; };
		const_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) if (state) state->prev(ref); return *this; };
		const_iterator& operator--() { if (state) state->prev(ref); return *this; };
		const_iterator operator--(int) { const_iterator retval = *this; --(retval); return retval; };
		const_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) if (state) state->next(ref); return *this; };
		const_iterator& operator++() { if (state) state->next(ref); return *this; };
		const_iterator operator++(int) { const_iterator retval = *this; ++(retval); return retval; };
		bool operator==(const_iterator const& other) const { return !(operator!=(other)); };
		bool operator!=(const_iterator const& other) const { if (state) return (ref != other.ref || state->cmp(other.state)); return false; };
		auto& operator*() const { return state->cget(ref); };
		auto* operator->() const { return &state->cget(ref); };
		const_iterator& begin() { if (state) state->begin(ref); return *this; };
		const_iterator& begin_at(cweeUnitValues::unit_value x) { if (state) state->begin_at(ref, x); return *this; };
		const_iterator& end() { if (state) state->end(ref); return *this; };
	};

	iterator begin() { return iterator(this, this->CreateIterationState()).begin(); };
	iterator begin_at(cweeUnitValues::unit_value x) { return iterator(this, this->CreateIterationState()).begin_at(x); };
	iterator end() { return iterator(this, this->CreateIterationState()).end(); };
	const_iterator begin() const { return const_iterator(this, this->CreateConstIterationState()).begin(); };
	const_iterator begin_at(cweeUnitValues::unit_value x) const { return const_iterator(this, this->CreateConstIterationState()).begin_at(x); };
	const_iterator end() const { return const_iterator(this, this->CreateConstIterationState()).end(); };
	const_iterator cbegin() const { return const_iterator(this, this->CreateConstIterationState()).begin(); };
	const_iterator cbegin_at(cweeUnitValues::unit_value x) const { return const_iterator(this, this->CreateConstIterationState()).begin_at(x); };
	const_iterator cend() const { return const_iterator(this, this->CreateConstIterationState()).end(); };

protected:
	virtual void AddValueActual(cweeUnitValues::unit_value X, cweeUnitValues::unit_value Y, bool isUnique) = 0;
	virtual void Convert_X(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) = 0;
	virtual void Convert_Y(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) = 0;
	virtual bool StaticUnits() { return false; };

public:
	cweeSharedPtr< cweeUnitPatternContainer_t> CopyAnotherContainer(cweeSharedPtr< cweeUnitPatternContainer_t> other) {
		bool canConvertX = CheckCanConvert(this->internal_X_type, other->internal_X_type);
		bool canConvertY = CheckCanConvert(this->internal_Y_type, other->internal_Y_type);

		if (!other->StaticUnits() && canConvertX && canConvertY) {
			// we can copy their container and change their units -- this is the best path as it prevents us from copying their data one-by-one
			auto out = other->Clone();

			out->Convert_X(other->internal_X_type, this->internal_X_type);
			out->Convert_Y(other->internal_Y_type, this->internal_Y_type);

			out->internal_X_type.Clear();
			out->internal_Y_type.Clear();

			out->internal_X_type = this->internal_X_type;
			out->internal_Y_type = this->internal_Y_type;

			return out;
		}
		else if (!this->StaticUnits()) {
			// we couldn't use their data container, so we will have to use a copy of our our own. 
			auto out = this->Clone();

			out->ClearData();

			if (canConvertX && canConvertY) {
				for (auto& x : *other) {
					if (x.Y) {
						out->AddValueActual(x.X, *x.Y, false);
					}
				}
				out->Convert_X(other->internal_X_type, this->internal_X_type);
				out->Convert_Y(other->internal_Y_type, this->internal_Y_type);
			}
			else if (!canConvertX && canConvertY) {
				for (auto& x : *other) {
					if (x.Y) {
						out->AddValueActual(x.X, *x.Y, false);
					}
				}
				out->Convert_Y(other->internal_Y_type, this->internal_Y_type);
			}
			else if (canConvertX && !canConvertY) {
				for (auto& x : *other) {
					if (x.Y) {
						out->AddValueActual(x.X, *x.Y, false);
					}
				}
				out->Convert_X(other->internal_X_type, this->internal_X_type);
			}
			else if (!canConvertX && !canConvertY) {
				for (auto& x : *other) {
					if (x.Y) {
						out->AddValueActual(x.X, *x.Y, false);
					}
				}
			}
			return out;
		}
		else {
			// everyone involved is statically typed, meaning there is no winning here.
			return other->Clone();
		}

	};
	virtual cweeSharedPtr< cweeUnitPatternContainer_t> Clone() = 0;
	cweeUnitPatternContainer_t() : internal_X_type(cweeUnitValues::second()), internal_Y_type(cweeUnitValues::scalar()) {};
	cweeUnitPatternContainer_t(cweeUnitValues::unit_value const& Y_type) : internal_X_type(cweeUnitValues::second()), internal_Y_type(Y_type) {};
	cweeUnitPatternContainer_t(cweeUnitValues::unit_value const& X_type, cweeUnitValues::unit_value const& Y_type) : internal_X_type(X_type), internal_Y_type(Y_type) {};
	cweeUnitPatternContainer_t(cweeUnitPatternContainer_t const& o) : internal_X_type(o.internal_X_type), internal_Y_type(o.internal_Y_type) {};
	cweeUnitPatternContainer_t& operator=(cweeUnitPatternContainer_t const& o) = delete;
	virtual ~cweeUnitPatternContainer_t() {};

	static bool CheckCanConvert(cweeUnitValues::unit_value const& a, cweeUnitValues::unit_value const& b) {
		return a.AreConvertableTypes(b);
	};
	static void RequireCanConvert(cweeUnitValues::unit_value const& a, cweeUnitValues::unit_value const& b) {
		if (!CheckCanConvert(a, b)) {
			throw(std::runtime_error(cweeStr::printf("Could not convert from '%s' to '%s' due to unit types.", a.ToString().c_str(), b.ToString().c_str()).c_str()));
		};
	};

	virtual cweeUnitValues::unit_value GetMinTime() const = 0;
	virtual cweeUnitValues::unit_value GetMaxTime() const = 0;
	virtual cweeUnitValues::unit_value GetAvgTime(void) const = 0;
	virtual cweeUnitValues::scalar											GetMinimumDecimals() const = 0;
	virtual cweeUnitValues::unit_value										GetMinimumTimeStep() const = 0;
	virtual void											                ScaleY(double ScaleY) = 0;
	virtual void											                Translate(const cweeUnitValues::unit_value& translation) = 0;
	virtual void											                ShiftTime(const cweeUnitValues::unit_value& deltaTime) = 0;
	virtual void											                UnsafeBasis(const cweeUnitValues::unit_value& t, u64* bvals, interpolation_t interpolationType) const = 0;
	virtual cweeUnitValues::unit_value										GetCurrentValue(cweeUnitValues::unit_value time, interpolation_t interpolationType) const = 0;
	virtual int GetNodeCount() const = 0;
	void AddValue(cweeUnitValues::unit_value const& X, cweeUnitValues::unit_value const& Y) {
		RequireCanConvert(X, internal_X_type); RequireCanConvert(Y, internal_Y_type);

		internal_Y_type = Y;
		internal_X_type = X;

		AddValueActual(internal_X_type, internal_Y_type, false);
	};
	void AddUniqueValue(cweeUnitValues::unit_value const& X, cweeUnitValues::unit_value const& Y) {
		RequireCanConvert(X, internal_X_type); RequireCanConvert(Y, internal_Y_type);

		internal_Y_type = Y;
		internal_X_type = X;

		AddValueActual(internal_X_type, internal_Y_type, true);
	};
	virtual void  ClearData() = 0;
	virtual void  RemoveUnnecessaryKnots(const cweeUnitValues::unit_value& timeStart, const cweeUnitValues::unit_value& timeEnd) = 0;
	virtual void  RemoveTimes(cweeUnitValues::unit_value greaterThanOrEqual, cweeUnitValues::unit_value LessThan) = 0;
	virtual void  RemoveWithMask(cweeSharedPtr< cweeUnitPatternContainer_t> other) = 0;
	void  Clear() {
		ClearData();
		internal_X_type.Clear();
		internal_Y_type.Clear();
	};
};

class cweeUnitPatternContainer final : public cweeUnitPatternContainer_t {
public:
	cweeBalancedTree<double, double, 10>						container;
	cweeSysInterlockedInteger									granularity;

	class SpecializedIterator final : public GenericIterator {
	public:
		SpecializedIterator() : GenericIterator() {};
		~SpecializedIterator() {};

		decltype(container)::const_iterator iter;
		decltype(container)::const_iterator iter_end;
		mutable cweeUnitValues::unit_value x_type;
		mutable cweeUnitValues::unit_value y_type;

		void begin(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				iter = p->container.begin();
				iter_end = p->container.end();

				x_type = p->internal_X_type;
				y_type = p->internal_Y_type;
			}
		};
		void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				iter = p->container.begin_at(x());
				iter_end = p->container.end();

				x_type = p->internal_X_type;
				y_type = p->internal_Y_type;
			}
		};
		void next(cweeUnitPatternContainer_t const* ref) {
			//auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			//if (p) {
			++iter;
			//while (iter != iter_end && !iter->object) {
			//	++iter;
			//}
		//}
		};
		void end(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				iter = p->container.end();
				iter_end = p->container.end();
			}
		};
		DataContainer& get(cweeUnitPatternContainer_t const* ref) {
			auto* p = const_cast<cweeUnitPatternContainer*>(dynamic_cast<cweeUnitPatternContainer const*>(ref));
			if (p) {
				cweeUnitValues::unit_value x_0 = (p->internal_X_type = iter->key);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (iter->object) {
					y_0 = (p->internal_Y_type = *iter->object);
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}

				thisContainer = cweeSharedPtr< DataContainer >(new_ptr, [x_0, y_0, p](DataContainer* ptr) {
					if (ptr->X != x_0) {
						if (ptr->Y) {
							if (*ptr->Y != y_0) {
								// x changed && y changed
								auto* node = p->container.NodeFind(x_0());
								p->container.Remove(node);
								p->container.Add(ptr->Y->operator()(), ptr->X(), true);
							}
							else {
								// x changed but y is the same
								auto* node = p->container.NodeFind(x_0());
								p->container.Remove(node);
								p->container.Add(y_0(), ptr->X(), true);
							}
						}
					}
					else {
						if (ptr->Y) {
							if (*ptr->Y != y_0) {
								// x is the same but y changed
								auto* node = p->container.NodeFind(x_0());
								if (node) {
									*node->object = ptr->Y->operator()();
								}
							}
							else {
								// x is the same && y is the same
								// DO NOTHING
							}
						}
					}
					delete ptr; });
			}
			return *thisContainer;
		};
		const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const {
			//auto* p = const_cast<cweeUnitPatternContainer*>(dynamic_cast<cweeUnitPatternContainer const*>(ref));
			//if (p) {
			if (thisContainer) {
				auto& x = thisContainer->X;
				auto& y = thisContainer->Y;

				if (cweeUnitValues::unit_value::IdenticalUnits(x, x_type)) {
					x.value_m = iter->key;
				}
				else {
					x = (x_type = iter->key);
				}

				if (iter->object) {
					y = make_cwee_shared<cweeUnitValues::unit_value>(y_type = *iter->object);

					//if (y) {	
					//	auto& z = *y;
					//	if (cweeUnitValues::unit_value::IdenticalUnits(z, y_type)) {
					//		z = *iter->object;
					//	}
					//	else {
					//		z = (y_type = *iter->object);
					//	}
					//}
					//else {
					//	y = make_cwee_shared<cweeUnitValues::unit_value>(y_type = *iter->object);
					//}
				}
				else {
					y = nullptr;
				}
			}
			else {
				DataContainer* new_ptr;
				if (iter->object) {
					new_ptr = new DataContainer((x_type = iter->key), (y_type = *iter->object));
				}
				else {
					new_ptr = new DataContainer((x_type = iter->key));
				}
				const_cast<cweeSharedPtr<DataContainer>&>(thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			//}
			return *thisContainer;
		};
		bool cmp(const cweeSharedPtr<GenericIterator>& s) const {
			if (!s) return 0;
			auto* p = dynamic_cast<SpecializedIterator*>(s.Get()); // auto p = s.CastReference<SpecializedIterator>();				
			if (p) {
				return iter.state.cmp(p->iter.state);
			}
			return true;
		};
		long long distance(const cweeSharedPtr<GenericIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedIterator>();
			if (p) {
				return iter.state.distance(p->iter.state);
			}
			return 0;
		};
		void prev(const cweeUnitPatternContainer_t* ref) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				--iter;
				while (iter != (p->container.begin()--) && !iter->object) --iter;
			}
		};
		const DataContainer& get(const cweeUnitPatternContainer_t* ref) const {
			auto* p = const_cast<cweeUnitPatternContainer*>(dynamic_cast<cweeUnitPatternContainer const*>(ref));
			if (p) {
				cweeUnitValues::unit_value x_0 = (p->internal_X_type = iter->key);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (iter->object) {
					y_0 = (p->internal_Y_type = *iter->object);
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}

				const_cast<cweeSharedPtr<DataContainer>&>(thisContainer) = cweeSharedPtr< DataContainer >(new_ptr, [x_0, y_0, p](DataContainer* ptr) {
					if (ptr->X != x_0) {
						if (ptr->Y) {
							if (*ptr->Y != y_0) {
								// x changed && y changed
								auto* node = p->container.NodeFind(x_0());
								p->container.Remove(node);
								p->container.Add(ptr->Y->operator()(), ptr->X(), true);
							}
							else {
								// x changed but y is the same
								auto* node = p->container.NodeFind(x_0());
								p->container.Remove(node);
								p->container.Add(y_0(), ptr->X(), true);
							}
						}
					}
					else {
						if (ptr->Y) {
							if (*ptr->Y != y_0) {
								// x is the same but y changed
								auto* node = p->container.NodeFind(x_0());
								if (node) {
									*node->object = ptr->Y->operator()();
								}
							}
							else {
								// x is the same && y is the same
								// DO NOTHING
							}
						}
					}
					delete ptr; });
			}
			return *thisContainer;
		};
	};
	class SpecializedConstIterator final : public GenericConstIterator {
	public:
		SpecializedConstIterator() : GenericConstIterator() {};
		~SpecializedConstIterator() {};

		decltype(container)::const_iterator iter;
		decltype(container)::const_iterator iter_end;
		mutable cweeUnitValues::unit_value x_type;
		mutable cweeUnitValues::unit_value y_type;

		void begin(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				iter = p->container.begin();
				iter_end = p->container.end();

				x_type = p->internal_X_type;
				y_type = p->internal_Y_type;
			}
		};
		void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				iter = p->container.begin_at(x());
				iter_end = p->container.end();

				x_type = p->internal_X_type;
				y_type = p->internal_Y_type;
			}
		};
		void next(cweeUnitPatternContainer_t const* ref) {
			++iter;
		};
		void end(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				iter = p->container.end();
				iter_end = p->container.end();
			}
		};
		DataContainer& get(cweeUnitPatternContainer_t const* ref) {
			auto* p = const_cast<cweeUnitPatternContainer*>(dynamic_cast<cweeUnitPatternContainer const*>(ref));
			if (p) {
				cweeUnitValues::unit_value x_0 = (p->internal_X_type = iter->key);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (iter->object) {
					y_0 = (p->internal_Y_type = *iter->object);
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}

				thisContainer = cweeSharedPtr< DataContainer >(new_ptr, [x_0, y_0, p](DataContainer* ptr) {	delete ptr; });
			}
			return *thisContainer;
		};
		const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const {
			if (thisContainer) {
				auto& x = thisContainer->X;
				auto& y = thisContainer->Y;

				if (cweeUnitValues::unit_value::IdenticalUnits(x, x_type)) {
					x.value_m = iter->key;
				}
				else {
					x = (x_type = iter->key);
				}

				if (iter->object) {
					thisContainer->Y_actual = (y_type = *iter->object);
					y = &thisContainer->Y_actual;
				}
				else {
					y = nullptr;
				}
			}
			else {
				DataContainer* new_ptr;
				if (iter->object) {
					new_ptr = new DataContainer((x_type = iter->key), (y_type = *iter->object));
				}
				else {
					new_ptr = new DataContainer((x_type = iter->key));
				}
				const_cast<cweeSharedPtr<DataContainer>&>(thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		bool cmp(const cweeSharedPtr<GenericConstIterator>& s) const {
			if (!s) return 0;
			auto* p = dynamic_cast<SpecializedConstIterator*>(s.Get()); // auto p = s.CastReference<SpecializedIterator>();				
			if (p) {
				return iter.state.cmp(p->iter.state);
			}
			return true;
		};
		long long distance(const cweeSharedPtr<GenericConstIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedConstIterator>();
			if (p) {
				return iter.state.distance(p->iter.state);
			}
			return 0;
		};
		void prev(const cweeUnitPatternContainer_t* ref) {
			auto* p = dynamic_cast<cweeUnitPatternContainer const*>(ref);
			if (p) {
				--iter;
				while (iter != (p->container.begin()--) && !iter->object) --iter;
			}
		};
		const DataContainer& get(const cweeUnitPatternContainer_t* ref) const {
			auto* p = const_cast<cweeUnitPatternContainer*>(dynamic_cast<cweeUnitPatternContainer const*>(ref));
			if (p) {
				cweeUnitValues::unit_value x_0 = (p->internal_X_type = iter->key);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (iter->object) {
					y_0 = (p->internal_Y_type = *iter->object);
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}

				const_cast<cweeSharedPtr<DataContainer>&>(thisContainer) = cweeSharedPtr< DataContainer >(new_ptr, [x_0, y_0, p](DataContainer* ptr) { delete ptr;  });
			}
			return *thisContainer;
		};
	};

	virtual cweeSharedPtr<GenericIterator> CreateIterationState() const {
		return make_cwee_shared<SpecializedIterator>().CastReference<GenericIterator>();
	};
	virtual cweeSharedPtr<GenericConstIterator> CreateConstIterationState() const {
		return make_cwee_shared<SpecializedConstIterator>().CastReference<GenericConstIterator>();
	};
public:
	cweeSharedPtr< cweeUnitPatternContainer_t> Clone() {
		cweeSharedPtr< cweeUnitPatternContainer> out = make_cwee_shared<cweeUnitPatternContainer>(*this);
		return out.CastReference< cweeUnitPatternContainer_t>();
	};
	cweeUnitPatternContainer() : cweeUnitPatternContainer_t(), container(), granularity(16) {};
	cweeUnitPatternContainer(cweeUnitValues::unit_value const& Y_type) : cweeUnitPatternContainer_t(Y_type), container(), granularity(16) {};
	cweeUnitPatternContainer(cweeUnitValues::unit_value const& X_type, cweeUnitValues::unit_value const& Y_type) : cweeUnitPatternContainer_t(X_type, Y_type), container(), granularity(16) {};
	cweeUnitPatternContainer(cweeUnitPatternContainer const& o) : cweeUnitPatternContainer_t(o.internal_X_type, o.internal_Y_type), container(), granularity(16) { container = o.container; };
	cweeUnitPatternContainer& operator=(cweeUnitPatternContainer const& o) = delete;
	~cweeUnitPatternContainer() {};

	cweeUnitValues::unit_value GetMinTime() const {
		cweeUnitValues::unit_value out(0);

		if (container.GetNodeCount() > 0) {
			auto ptr = container.GetFirst();
			if (ptr) {
				out = (internal_X_type = ptr->key);
			}
		}
		return out;
	};
	cweeUnitValues::unit_value GetMaxTime() const {
		cweeUnitValues::unit_value out(0);

		if (container.GetNodeCount() > 0) {
			auto ptr = container.GetLast();
			if (ptr) {
				out = (internal_X_type = ptr->key);
			}
		}
		return out;
	};
	cweeUnitValues::unit_value GetAvgTime(void) const {
		cweeUnitValues::unit_value out;

		int num(0);

		out = (internal_X_type = 0);
		for (auto& x : container) {
			if (x.object) {
				num++;
				out -= (out / num);
				out += ((internal_X_type = x.key) / num);
			}
		}
		return out;
	};

	cweeUnitValues::scalar											GetMinimumDecimals() const {
		cweeUnitValues::scalar decimal = 1.0f;
		int numbs = GetNodeCount();
		if (numbs == 0) return 0.0001f;
		cweeUnitValues::scalar F;
		int numSuccess = 0;

		for (auto& x : container) {
			if (x.object) {
				F = cweeMath::roundNearest((float)*x.object, (float)decimal);
				if (cweeMath::Fabs((float)(F - (float)*x.object)) > 0.00001) {
					// too great an error
					decimal = (float)decimal / 10.0f;
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
		}
		return decimal;
	};
	cweeUnitValues::unit_value										GetMinimumTimeStep() const {
		cweeUnitValues::unit_value out, prevTime, t;
		int numFailures, numbs;

		numbs = GetNodeCount();
		if (numbs <= 1) return 1;

		out = GetMaxTime() - GetMinTime();
		prevTime = out;
		prevTime = std::numeric_limits<cweeUnitValues::unit_value>::max();
		numFailures = 100;

		for (auto& x : container) {
			if (x.object) {
				t = cweeUnitValues::math::abs(prevTime - x.key);
				prevTime = x.key;
				if (t < out && t > 0) {
					out = t;
				}
				else {
					numFailures--;
					if (numFailures <= 0) break;
				}
			}
		}

		if (out <= 1) out = 1;

		return out;
	};

	void											ShiftTime(const cweeUnitValues::unit_value& deltaTime) {
		for (auto& x : container) {
			x.key = ((internal_X_type = deltaTime) + x.key)();
		}
	};
	void											ScaleY(double ScaleY) {
		for (auto& x : container) {
			if (x.object) {
				*x.object *= ScaleY;
			}
		}
	};
	void											Translate(const cweeUnitValues::unit_value& translation) {
		for (auto& x : container) {
			if (x.object) {
				*x.object = ((internal_Y_type = translation) + *x.object)();
			}
		}
	};

	void											UnsafeBasis(const cweeUnitValues::unit_value& t, u64* bvals, interpolation_t interpolationType) const {
		using node_type = cweeBalancedTree<double, double, 10>::cweeBalancedTreeNode;

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
		case interpolation_t::RIGHT: {
			bvals[0] = 0;
			bvals[1] = 0;
			bvals[2] = 1;
			bvals[3] = 0;
			break;
		}
		case interpolation_t::SPLINE: {
			index1 = container.NodeFindLargestSmallerEqual(t());
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
		case interpolation_t::LINEAR: {
			index1 = container.NodeFindLargestSmallerEqual(t());
			if (index1) {
				s = (u64)index1->key;
				index1 = container.GetNextLeaf(index1);
				if (index1) {
					if (s <= t && index1->key >= t) {
						if (index1->key == s) {
							bvals[0] = 0;
							bvals[1] = 1;
							bvals[2] = 0;
							bvals[3] = 0;
						}
						else {
							s = (u64)((t - s) / (index1->key - s));
							bvals[0] = 0;
							bvals[1] = 1.0f - s;
							bvals[2] = s;
							bvals[3] = 0;
						}
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
		case interpolation_t::LEFT: {
			bvals[0] = 0;
			bvals[1] = 1;
			bvals[2] = 0;
			bvals[3] = 0;
			break;
		}
		}
	};

	cweeUnitValues::unit_value										GetCurrentValue(cweeUnitValues::unit_value time, interpolation_t interpolationType) const {
		using node_type = cweeBalancedTree<double, double, 10>::cweeBalancedTreeNode;

		int j;
		u64 bvals[4];
		cweeUnitValues::unit_value v;
		v = (internal_Y_type = 0);
		internal_X_type = time; time.Clear(); time = internal_X_type;
		j = container.GetNodeCount();
		if (j < 1) {
			v = 0;
			return v;
		}
		else if (j == 1) {
			auto* ptr = container.FindLargestSmallerEqual(time());
			if (ptr) v = *ptr;
			return v;
		}
		else {
			{
				switch (interpolationType) {
				case interpolation_t::RIGHT: {
					auto* ptr = container.FindSmallestLargerEqual(time());
					if (ptr) v = *ptr;
					break;
				}
				case interpolation_t::SPLINE: {
					UnsafeBasis(time, bvals, interpolationType);
					node_type* x1 = container.NodeFindLargestSmallerEqual(time());
					node_type* x0 = x1 == nullptr ? (node_type*)nullptr : container.GetPrevLeaf(x1);
					node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
					node_type* x3 = x2 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x2);
					if (x0) v += *x0->object * bvals[0];	else if (x1) v += *x1->object * bvals[0];
					if (x1) v += *x1->object * bvals[1];
					if (x2) v += *x2->object * bvals[2];	else if (x1) v += *x1->object * bvals[2];
					if (x3) v += *x3->object * bvals[3]; else if (x1) v += *x1->object * bvals[3];
					break;
				}
				case interpolation_t::LINEAR: {
					UnsafeBasis(time, bvals, interpolationType);
					node_type* x1 = container.NodeFindLargestSmallerEqual(time());
					node_type* x2 = x1 == nullptr ? (node_type*)nullptr : container.GetNextLeaf(x1);
					if (x1) v += *x1->object * bvals[1];
					if (x2) v += *x2->object * bvals[2];	else if (x1) v += *x1->object * bvals[2];
					break;
				}
				default:
				case interpolation_t::LEFT: {
					auto* ptr = container.FindLargestSmallerEqual(time());
					if (ptr) v = *ptr;
					break;
				}
				}
			}
			return v;
		}
	};

	int GetNodeCount() const { return container.GetNodeCount(); };
	void Convert_X(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) {
		for (auto& x : container) {
			old_unit = x.key;
			new_unit = old_unit;
			x.key = new_unit();
		}
	};
	void Convert_Y(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) {
		for (auto& x : container) {
			if (x.object) {
				old_unit = *x.object;
				new_unit = old_unit;
				*x.object = new_unit();
			}
		}
	};
	void AddValueActual(cweeUnitValues::unit_value X, cweeUnitValues::unit_value Y, bool isUnique) {
		if (granularity <= 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}
		if ((GetNodeCount() + 1) >= (granularity.GetValue() * GRANULARITY_SCALER)) {
			granularity.SetValue(granularity.GetValue() * GRANULARITY_SCALER);
			int newsize = GetNodeCount() + granularity.GetValue();
			newsize -= newsize % granularity.GetValue();
			// if (container.GetReservedCount() < newsize) {
			container.Reserve(newsize);
			// }
		}
		container.Add(Y(), X(), isUnique);
	};
	void ClearData() {
		container.Clear();
	};
	int												GetNumValues() const {
		int out;
		out = GetNodeCount();
		return out;
	};
	cweeUnitValues::unit_value						GetMinValue() const {
		if (GetNumValues() == 0) return 0;
		bool skipFirst = true;
		auto out = internal_Y_type;

		for (auto& x : container) {
			if (x.object) {
				if (skipFirst) {
					skipFirst = false;
					out = *x.object;
				}
				else {
					if ((internal_Y_type = *x.object) < out) out = internal_Y_type;
				}
			}
		}
		return out;
	};
	cweeUnitValues::unit_value						GetMaxValue() const {
		if (GetNumValues() == 0) return 0;
		bool skipFirst = true;
		auto out = internal_Y_type;
		for (auto& x : container) {
			if (x.object) {
				if (skipFirst) {
					skipFirst = false;
					out = *x.object;
				}
				else {
					if ((internal_Y_type = *x.object) > out) out = internal_Y_type;
				}
			}
		}
		return out;
	};

	void RemoveUnnecessaryKnots(const cweeUnitValues::unit_value& start, const cweeUnitValues::unit_value& end) {

		if (start >= end) return;
		int num, index; cweeThreadedList<double> keysToDelete;

		double epsilon = (this->GetMaxValue() - this->GetMinValue())() / 10000.0;
		double* val1 = nullptr, * val2 = nullptr, * val3 = nullptr, * val4 = nullptr, * val5 = nullptr;
		double t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;

		{
			decltype(container)& _values = container;

			num = _values.GetNodeCount();

			if (num > 5) {
				index = 0;
				for (auto& knot : _values) {
					if (index + 3 >= num) break;
					if (index >= 5) {
						if (::fabs(*val1 - *val2) > epsilon) { // !=
							++index;
							val1 = val2; t1 = t2;
							val2 = val3; t2 = t3;
							val3 = val4; t3 = t4;
							val4 = val5; t4 = t5;
							val5 = knot.object;
							t5 = knot.key;
							continue;
						}
						else if (::fabs(*val5 - *val2) <= epsilon) { // ==
							if (::fabs(*val4 - *val2) <= epsilon) { // ==
								if (::fabs(*val3 - *val2) <= epsilon) { // ==
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
	void  RemoveTimes(cweeUnitValues::unit_value greaterThanOrEqual, cweeUnitValues::unit_value LessThan) {
		cweeThreadedList<int> indexesToDelete;
		{
			double greaterThan = (internal_X_type = greaterThanOrEqual)();

			auto iter = container.NodeFindSmallestLargerEqual(greaterThan);
			if (iter) {
				do {
					if (iter->key >= greaterThan) {
						if (iter->key < LessThan) {
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
	void  RemoveWithMask(cweeSharedPtr< cweeUnitPatternContainer_t> other) {
		auto iter = container.GetFirst();
		if (iter && other) {
			do {
				if (other->GetCurrentValue(iter->key, interpolation_t::LINEAR) > 0) {
					container.Remove(iter);
				}
				iter = container.GetNextLeaf(iter);
			} while (iter);
		}
	};
};

template<typename Y_Axis_Type, typename X_Axis_Type>
class cweeBalancedPatternReferenceContainer final : public cweeUnitPatternContainer_t {
public:
	cweeBalancedPattern<Y_Axis_Type, X_Axis_Type>* ref;

	class SpecializedIterator final : public GenericIterator {
	public:
		SpecializedIterator() : GenericIterator(), pos(0), numVars(0) {};
		~SpecializedIterator() {};

		int pos;
		int numVars;

		void begin(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void next(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				pos++;
			}
		};
		void end(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				numVars = p->ref->GetNumValues();
				pos = numVars;
			}
		};
		DataContainer& get(cweeUnitPatternContainer_t const* ref) {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = cweeUnitValues::unit_value::from_unit_t(t));
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				this->thisContainer = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = cweeUnitValues::unit_value::from_unit_t(t));
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		bool cmp(const cweeSharedPtr<GenericIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedIterator>();
			if (p) {
				return pos == p->pos;
			}
			return true;
		};
		long long distance(const cweeSharedPtr<GenericIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedIterator>();
			if (p) {
				return pos - p->pos;
			}
			return 0;
		};
		void prev(const cweeUnitPatternContainer_t* ref) {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				pos--;
			}
		};
		const DataContainer& get(const cweeUnitPatternContainer_t* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = cweeUnitValues::unit_value::from_unit_t(t));
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
	};
	class SpecializedConstIterator final : public GenericConstIterator {
	public:
		SpecializedConstIterator() : GenericConstIterator(), pos(0), numVars(0) {};
		~SpecializedConstIterator() {};

		int pos;
		int numVars;

		void begin(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void next(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				pos++;
			}
		};
		void end(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref);
			if (p) {
				numVars = p->ref->GetNumValues();
				pos = numVars;
			}
		};
		DataContainer& get(cweeUnitPatternContainer_t const* ref) {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = cweeUnitValues::unit_value::from_unit_t(t));
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				this->thisContainer = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = cweeUnitValues::unit_value::from_unit_t(t));
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		bool cmp(const cweeSharedPtr<GenericConstIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedConstIterator>();
			if (p) {
				return pos == p->pos;
			}
			return true;
		};
		long long distance(const cweeSharedPtr<GenericConstIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedConstIterator>();
			if (p) {
				return pos - p->pos;
			}
			return 0;
		};
		void prev(const cweeUnitPatternContainer_t* ref) {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				pos--;
			}
		};
		const DataContainer& get(const cweeUnitPatternContainer_t* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferenceContainer*>(dynamic_cast<cweeBalancedPatternReferenceContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = cweeUnitValues::unit_value::from_unit_t(t));
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
	};
	virtual cweeSharedPtr<GenericIterator> CreateIterationState() const {
		return make_cwee_shared<SpecializedIterator>().template CastReference<GenericIterator>();
	};
	virtual cweeSharedPtr<GenericConstIterator> CreateConstIterationState() const {
		return make_cwee_shared<SpecializedConstIterator>().CastReference<GenericConstIterator>();
	};

public:
	cweeSharedPtr< cweeUnitPatternContainer_t> Clone() {
		cweeSharedPtr< cweeBalancedPatternReferenceContainer> out = make_cwee_shared<cweeBalancedPatternReferenceContainer>(*this);
		return out.template CastReference< cweeUnitPatternContainer_t>();
	};
	cweeBalancedPatternReferenceContainer() : cweeUnitPatternContainer_t(), ref(nullptr) {};
	cweeBalancedPatternReferenceContainer(cweeBalancedPattern<Y_Axis_Type, X_Axis_Type>* Ref) : cweeUnitPatternContainer_t(cweeUnitValues::unit_value::from_unit_t(X_Axis_Type(0)), cweeUnitValues::unit_value::from_unit_t(Y_Axis_Type(0))), ref(Ref) {};
	cweeBalancedPatternReferenceContainer(cweeBalancedPatternReferenceContainer const& o) : cweeUnitPatternContainer_t(o.internal_X_type, o.internal_Y_type), ref(o.ref) {};
	cweeBalancedPatternReferenceContainer& operator=(cweeBalancedPatternReferenceContainer const& o) = delete; 
	~cweeBalancedPatternReferenceContainer() {};

	cweeUnitValues::unit_value GetMinTime() const {
		return internal_X_type = cweeUnitValues::unit_value::from_unit_t(ref->GetMinTime());
	};
	cweeUnitValues::unit_value GetMaxTime() const {
		return internal_X_type = cweeUnitValues::unit_value::from_unit_t(ref->GetMaxTime());
	};
	cweeUnitValues::unit_value GetAvgTime(void) const {
		return internal_X_type = cweeUnitValues::unit_value::from_unit_t(ref->GetAvgTime());
	};

	cweeUnitValues::scalar											GetMinimumDecimals() const {
		return cweeUnitValues::unit_value::from_unit_t(ref->GetMinimumDecimals());
	};
	cweeUnitValues::unit_value										GetMinimumTimeStep() const {
		return cweeUnitValues::unit_value::from_unit_t(ref->GetMinimumTimeStep());
	};

	void											ShiftTime(const cweeUnitValues::unit_value& deltaTime) {
		ref->ShiftTime((internal_X_type = deltaTime)());
	};
	void											ScaleY(double ScaleY) {
		ref->Lock();
		for (auto& x : ref->UnsafeGetValues()) {
			if (x.object) {
				*x.object *= ScaleY;
			}
		}
		ref->Unlock();
	};
	void											Translate(const cweeUnitValues::unit_value& translation) {
		ref->Translate((internal_Y_type = translation)());
	};

	void											UnsafeBasis(const cweeUnitValues::unit_value& t, u64* bvals, interpolation_t interpolationType) const {};
	cweeUnitValues::unit_value										GetCurrentValue(cweeUnitValues::unit_value time, interpolation_t interpolationType) const {
		AUTO prevIT = ref->GetInterpolationType();
		ref->SetInterpolationType(interpolationType);
		AUTO answer = ref->GetCurrentValue((internal_X_type = time)());
		ref->SetInterpolationType(prevIT);
		return cweeUnitValues::cweeUnitValues::unit_value::from_unit_t(answer);
	};

	int  GetNodeCount() const { return ref->GetNumValues(); };
	void Convert_X(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) {
		// we cannot do this -- cweeBalancedPattern is statically typed
	};
	void Convert_Y(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) {
		// we cannot do this -- cweeBalancedPattern is statically typed
	};
	bool StaticUnits() override { return true; };
	void AddValueActual(cweeUnitValues::unit_value X, cweeUnitValues::unit_value Y, bool isUnique) {
		if (isUnique) {
			ref->AddUniqueValue((internal_X_type = X)(), (internal_Y_type = Y)());
		}
		else {
			ref->AddValue((internal_X_type = X)(), (internal_Y_type = Y)());
		}
	};
	void ClearData() {
		ref->Clear();
	};
	void RemoveUnnecessaryKnots(const cweeUnitValues::unit_value& timeStart, const cweeUnitValues::unit_value& timeEnd) {
		ref->RemoveUnnecessaryKnots((internal_X_type = timeStart)(), (internal_X_type = timeEnd)());
	};
	void  RemoveTimes(cweeUnitValues::unit_value greaterThanOrEqual, cweeUnitValues::unit_value LessThan) {
		ref->RemoveTimes((internal_X_type = greaterThanOrEqual)(), (internal_X_type = (LessThan - 0.0001))());
	};
	void  RemoveWithMask(cweeSharedPtr< cweeUnitPatternContainer_t> other) {
		if (other) {
			ref->RemoveWithMask([other](X_Axis_Type T)->bool {
				return (other->GetCurrentValue(cweeUnitValues::cweeUnitValues::unit_value::from_unit_t(T), interpolation_t::LINEAR) > 0);
			});
		}
	};
};

template<typename Y_Axis_Type>
class cweeBalancedPatternReferencePartialContainer final : public cweeUnitPatternContainer_t {
public:
	using X_Axis_Type = typename cweeUnitValues::cweeUnitValues::second;
	cweeBalancedPattern<Y_Axis_Type, u64>* ref;

	class SpecializedIterator final : public GenericIterator {
	public:
		SpecializedIterator() : GenericIterator(), pos(0), numVars(0) {};
		~SpecializedIterator() {};

		int pos;
		int numVars;

		void begin(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void next(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				pos++;
			}
		};
		void end(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				numVars = p->ref->GetNumValues();
				pos = numVars;
			}
		};
		DataContainer& get(cweeUnitPatternContainer_t const* ref) {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = t);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				this->thisContainer = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = t);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		bool cmp(const cweeSharedPtr<GenericIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedIterator>();
			if (p) {
				return pos != p->pos;
			}
			return true;
		};
		long long distance(const cweeSharedPtr<GenericIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedIterator>();
			if (p) {
				return pos - p->pos;
			}
			return 0;
		};
		void prev(const cweeUnitPatternContainer_t* ref) {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				pos--;
			}
		};
		const DataContainer& get(const cweeUnitPatternContainer_t* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = t);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
	};
	class SpecializedConstIterator final : public GenericConstIterator {
	public:
		SpecializedConstIterator() : GenericConstIterator(), pos(0), numVars(0) {};
		~SpecializedConstIterator() {};

		int pos;
		int numVars;

		void begin(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void begin_at(cweeUnitPatternContainer_t const* ref, cweeUnitValues::unit_value x) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				pos = 0;
				numVars = p->ref->GetNumValues();
			}
		};
		void next(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				pos++;
			}
		};
		void end(cweeUnitPatternContainer_t const* ref) {
			auto* p = dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref);
			if (p) {
				numVars = p->ref->GetNumValues();
				pos = numVars;
			}
		};
		DataContainer& get(cweeUnitPatternContainer_t const* ref) {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = t);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				this->thisContainer = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		const DataContainer& cget(cweeUnitPatternContainer_t const* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = t);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
		bool cmp(const cweeSharedPtr<GenericConstIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedConstIterator>();
			if (p) {
				return pos != p->pos;
			}
			return true;
		};
		long long distance(const cweeSharedPtr<GenericConstIterator>& s) const {
			if (!s) return 0;
			auto p = s.CastReference<SpecializedConstIterator>();
			if (p) {
				return pos - p->pos;
			}
			return 0;
		};
		void prev(const cweeUnitPatternContainer_t* ref) {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				pos--;
			}
		};
		const DataContainer& get(const cweeUnitPatternContainer_t* ref) const {
			auto* p = const_cast<cweeBalancedPatternReferencePartialContainer*>(dynamic_cast<cweeBalancedPatternReferencePartialContainer const*>(ref));
			if (p) {
				AUTO t = p->ref->GetTimeAtIndex(pos);
				p->ref->Lock();
				auto* node = p->ref->UnsafeGetValue(t);

				cweeUnitValues::unit_value x_0 = (p->internal_X_type = t);
				cweeUnitValues::unit_value y_0;
				DataContainer* new_ptr;
				if (node && node->object) {
					y_0 = (p->internal_Y_type = cweeUnitValues::unit_value::from_unit_t(*node->object));
					new_ptr = new DataContainer(x_0, y_0);
				}
				else {
					new_ptr = new DataContainer(x_0);
				}
				p->ref->Unlock();
				const_cast<cweeSharedPtr<DataContainer>&>(this->thisContainer) = cweeSharedPtr< DataContainer >(new_ptr);
			}
			return *thisContainer;
		};
	};
	virtual cweeSharedPtr<GenericIterator> CreateIterationState() const {
		return make_cwee_shared<SpecializedIterator>().template CastReference<GenericIterator>();
	};
	virtual cweeSharedPtr<GenericConstIterator> CreateConstIterationState() const {
		return make_cwee_shared<SpecializedConstIterator>().CastReference<GenericConstIterator>();
	};
public:
	cweeSharedPtr< cweeUnitPatternContainer_t> Clone() {
		cweeSharedPtr< cweeBalancedPatternReferencePartialContainer> out = make_cwee_shared<cweeBalancedPatternReferencePartialContainer>(*this);
		return out.template CastReference< cweeUnitPatternContainer_t>();
	};
	cweeBalancedPatternReferencePartialContainer() : cweeUnitPatternContainer_t(), ref(nullptr) {};
	cweeBalancedPatternReferencePartialContainer(cweeBalancedPattern<Y_Axis_Type, u64>* Ref) : cweeUnitPatternContainer_t(cweeUnitValues::unit_value(X_Axis_Type(0)), cweeUnitValues::unit_value::from_unit_t(Y_Axis_Type(0))), ref(Ref) {};
	cweeBalancedPatternReferencePartialContainer(cweeBalancedPatternReferencePartialContainer const& o) : cweeUnitPatternContainer_t(o.internal_X_type, o.internal_Y_type), ref(o.ref) {};
	cweeBalancedPatternReferencePartialContainer& operator=(cweeBalancedPatternReferencePartialContainer const& o) = delete; /* {
		internal_X_type.Clear(); internal_Y_type.Clear();
		internal_X_type = o.internal_X_type;
		internal_Y_type = o.internal_Y_type;
		ref = o.ref;
		return *this;
	};*/
	~cweeBalancedPatternReferencePartialContainer() {};

	cweeUnitValues::unit_value GetMinTime() const { return internal_X_type = ref->GetMinTime(); };
	cweeUnitValues::unit_value GetMaxTime() const { return internal_X_type = ref->GetMaxTime(); };
	cweeUnitValues::unit_value GetAvgTime(void) const { return internal_X_type = ref->GetAvgTime(); };

	cweeUnitValues::scalar											GetMinimumDecimals() const {

		return cweeUnitValues::unit_value::from_unit_t(ref->GetMinimumDecimals());

	};
	cweeUnitValues::unit_value										GetMinimumTimeStep() const {

		return cweeUnitValues::unit_value::from_unit_t(ref->GetMinimumTimeStep());

	};

	void											ShiftTime(const cweeUnitValues::unit_value& deltaTime) {
		ref->ShiftTime((internal_X_type = deltaTime)());
	};
	void											ScaleY(double ScaleY) {
		ref->Lock();
		for (auto& x : ref->UnsafeGetValues()) {
			if (x.object) {
				*x.object *= ScaleY;
			}
		}
		ref->Unlock();
	};
	void											Translate(const cweeUnitValues::unit_value& translation) {
		ref->Translate((internal_Y_type = translation)());
	};

	void											UnsafeBasis(const cweeUnitValues::unit_value& t, u64* bvals, interpolation_t interpolationType) const {};
	cweeUnitValues::unit_value										GetCurrentValue(cweeUnitValues::unit_value time, interpolation_t interpolationType) const {
		AUTO prevIT = ref->GetInterpolationType();
		ref->SetInterpolationType(interpolationType);
		AUTO answer = ref->GetCurrentValue((internal_X_type = time)());
		ref->SetInterpolationType(prevIT);
		return cweeUnitValues::cweeUnitValues::unit_value::from_unit_t(answer);
	};

	int  GetNodeCount() const { return ref->GetNumValues(); };
	void Convert_X(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) {
		// we cannot do this -- cweeBalancedPattern is statically typed
	};
	void Convert_Y(cweeUnitValues::unit_value old_unit, cweeUnitValues::unit_value new_unit) {
		// we cannot do this -- cweeBalancedPattern is statically typed
	};
	bool StaticUnits() override { return true; };
	void AddValueActual(cweeUnitValues::unit_value X, cweeUnitValues::unit_value Y, bool isUnique) {
		if (isUnique) {
			ref->AddUniqueValue((internal_X_type = X)(), (internal_Y_type = Y)());
		}
		else {
			ref->AddValue((internal_X_type = X)(), (internal_Y_type = Y)());
		}
	};
	void ClearData() {
		ref->Clear();
	};
	void RemoveUnnecessaryKnots(const cweeUnitValues::unit_value& timeStart, const cweeUnitValues::unit_value& timeEnd) {
		ref->RemoveUnnecessaryKnots((internal_X_type = timeStart)(), (internal_X_type = timeEnd)());
	};
	void  RemoveTimes(cweeUnitValues::unit_value greaterThanOrEqual, cweeUnitValues::unit_value LessThan) {
		ref->RemoveTimes((internal_X_type = greaterThanOrEqual)(), (internal_X_type = (LessThan - 0.0001))());
	};
	void  RemoveWithMask(cweeSharedPtr< cweeUnitPatternContainer_t> other) {
		if (other) {
			ref->RemoveWithMask([other](u64 T)->bool {
				return (other->GetCurrentValue(T, interpolation_t::LINEAR) > 0);
			});
		}
	};

};

class cweeUnitPattern {
public:
	using pairT = cweePair< cweeUnitValues::unit_value, cweeUnitValues::unit_value >;

protected:
	cweeSharedPtr<cweeUnitPatternContainer_t>			container;
	boundary_t											boundaryType;
	interpolation_t										interpolationType;
	mutable cweeSysMutex								lock;

public:
	cweeUnitValues::unit_value X_Type() const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->internal_X_type;
		lock.Unlock();
		return out;
	};
	cweeUnitValues::unit_value Y_Type() const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->internal_Y_type; 
		lock.Unlock();
		return out;
	};

	cweeUnitPattern() :
		container(make_cwee_shared<cweeUnitPatternContainer>(cweeUnitPatternContainer(cweeUnitValues::second(), cweeUnitValues::scalar())).CastReference<cweeUnitPatternContainer_t>()),
		boundaryType(boundary_t::BT_FREE), interpolationType(interpolation_t::LINEAR), lock() {};
	cweeUnitPattern(cweeUnitValues::unit_value const& X_type, cweeUnitValues::unit_value const& Y_type) :
		container(make_cwee_shared<cweeUnitPatternContainer>(cweeUnitPatternContainer(X_type, Y_type)).CastReference<cweeUnitPatternContainer_t>()),
		boundaryType(boundary_t::BT_FREE), interpolationType(interpolation_t::LINEAR), lock() {};

	template<typename Y_Axis_Type, typename X_Axis_Type>
	cweeUnitPattern(cweeBalancedPattern<Y_Axis_Type, X_Axis_Type>& ref) :
		container(make_cwee_shared<cweeBalancedPatternReferenceContainer<Y_Axis_Type, X_Axis_Type>>(cweeBalancedPatternReferenceContainer<Y_Axis_Type, X_Axis_Type>(&ref)).template CastReference<cweeUnitPatternContainer_t>()),
		boundaryType(ref.GetBoundaryType()), interpolationType(ref.GetInterpolationType()), lock() {};

	template<typename Y_Axis_Type>
	cweeUnitPattern(cweeBalancedPattern<Y_Axis_Type, u64>& ref) :
		container(make_cwee_shared<cweeBalancedPatternReferencePartialContainer<Y_Axis_Type>>(cweeBalancedPatternReferencePartialContainer<Y_Axis_Type>(&ref)).template CastReference<cweeUnitPatternContainer_t>()),
		boundaryType(ref.GetBoundaryType()), interpolationType(ref.GetInterpolationType()), lock() {};

	~cweeUnitPattern() {};

	cweeUnitPattern(cweeUnitPattern const& o) :
		container(nullptr),
		boundaryType(boundary_t::BT_FREE), interpolationType(interpolation_t::LINEAR), lock() {
		AUTO g{ lock.Guard() };
		// AUTO g1{ o.lock.Guard() };
		cweeSharedPtr<cweeUnitPatternContainer_t> oContainer = o.container;
		if (oContainer) {
			container = oContainer->Clone();
		}		
		boundaryType = o.boundaryType;
		interpolationType = o.interpolationType;
	};
	cweeUnitPattern& operator=(cweeUnitPattern const& o) {
		if (this == &o) return *this;

		AUTO g{ lock.Guard() };
		AUTO g1{ o.lock.Guard() };

		container = container->CopyAnotherContainer(o.container); // gets the values of another, uses my units

		boundaryType = o.boundaryType;
		interpolationType = o.interpolationType;
		return *this;
	};

	void AddValue(cweeUnitValues::unit_value const& X, cweeUnitValues::unit_value const& Y) {
		container->AddValue(X, Y);
	};
	void AddUniqueValue(cweeUnitValues::unit_value const& X, cweeUnitValues::unit_value const& Y) {
		container->AddUniqueValue(X, Y);
	};

	void												SetBoundaryType(const boundary_t& bt) {
		lock.Lock();
		boundaryType = bt;
		lock.Unlock();
	};
	boundary_t											GetBoundaryType() const {
		boundary_t out;
		lock.Lock();
		out = boundaryType;
		lock.Unlock();
		return out;
	};

	void												SetInterpolationType(const interpolation_t& it) {
		lock.Lock();
		interpolationType = it;
		lock.Unlock();
	};
	interpolation_t										GetInterpolationType() const {
		interpolation_t out;
		lock.Lock();
		out = interpolationType;
		lock.Unlock();
		return out;
	};

	void Clear() {
		AUTO g{ lock.Guard() };
		container->Clear();
		boundaryType = boundary_t::BT_FREE;
		interpolationType = interpolation_t::LINEAR;
	};
	void ClearData() {
		AUTO g{ lock.Guard() };
		container->ClearData();
	};

	int												GetNumValues() const {
		int out;
		lock.Lock();
		out = container->GetNodeCount();
		lock.Unlock();
		return out;
	};

	cweeUnitValues::unit_value										GetMinValue() const {
		if (GetNumValues() == 0) return 0;
		bool skipFirst = true;
		AUTO g{ lock.Guard() };
		auto out = container->internal_Y_type;
		for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
			if (x.Y) {
				if (skipFirst) {
					skipFirst = false;
					out = *x.Y;
				}
				else {
					if ((container->internal_Y_type = *x.Y) < out) out = container->internal_Y_type;
				}
			}
		}
		return out;
	};
	cweeUnitValues::unit_value										GetMaxValue() const {
		if (GetNumValues() == 0) return 0;
		bool skipFirst = true;
		AUTO g{ lock.Guard() };
		auto out = container->internal_Y_type;
		for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
			if (x.Y) {
				if (skipFirst) {
					skipFirst = false;
					out = *x.Y;
				}
				else {
					if ((container->internal_Y_type = *x.Y) > out) out = container->internal_Y_type;
				}
			}
		}
		return out;
	};

	cweeUnitValues::unit_value										GetMinValue(cweeUnitValues::unit_value start, cweeUnitValues::unit_value end) const {
		cweeUnitValues::unit_value out;
		int n = GetNumValues();
		if (n == 0) return out;
		bool skipFirst = true;
		AUTO g{ lock.Guard() };
		out = (container->internal_Y_type = 0);
		container->internal_X_type = start; start.Clear(); start = container->internal_X_type;
		container->internal_X_type = end; end.Clear(); end = container->internal_X_type;

		AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
		for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(start); iter != endIter; ++iter) {
			auto& dataPair = *iter;
			if (dataPair.Y) {
				if (dataPair.X >= start) {
					if (dataPair.X <= end) {
						if (skipFirst) {
							skipFirst = false;
							out = *dataPair.Y;
						}
						else {
							if ((container->internal_Y_type = *dataPair.Y) < out) out = (container->internal_Y_type = *dataPair.Y);
						}
					}
					else {
						break;
					}
				}
			}
		}

		return out;
	};
	cweeUnitValues::unit_value										GetMaxValue(cweeUnitValues::unit_value start, cweeUnitValues::unit_value end) const {
		cweeUnitValues::unit_value out;

		int n = GetNumValues();
		if (n == 0) return out;
		bool skipFirst = true;
		AUTO g{ lock.Guard() };
		out = (container->internal_Y_type = 0);
		container->internal_X_type = start; start.Clear(); start = container->internal_X_type;
		container->internal_X_type = end; end.Clear(); end = container->internal_X_type;

		AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
		for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(start); iter != endIter; ++iter) {
			auto& dataPair = *iter;
			if (dataPair.Y) {
				if (dataPair.X >= start) {
					if (dataPair.X <= end) {
						if (skipFirst) {
							skipFirst = false;
							out = *dataPair.Y;
						}
						else {
							if ((container->internal_Y_type = *dataPair.Y) > out) out = (container->internal_Y_type = *dataPair.Y);
						}
					}
					else {
						break;
					}
				}
			}
		}

		return out;
	};

	cweeUnitValues::unit_value										GetMinTime(void) const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->GetMinTime();
		lock.Unlock();
		return out;
	};
	cweeUnitValues::unit_value										GetMaxTime(void) const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->GetMaxTime();
		lock.Unlock();
		return out;
	};
	cweeUnitValues::unit_value										GetAvgTime(void) const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->GetAvgTime();
		lock.Unlock();
		return out;
	};
	cweeUnitValues::scalar											GetMinimumDecimals() const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->GetMinimumDecimals();
		lock.Unlock();
		return out;
	};
	cweeUnitValues::unit_value										GetMinimumTimeStep() const {
		cweeUnitValues::unit_value out;
		lock.Lock();
		out = container->GetMinimumTimeStep();
		lock.Unlock();
		return out;
	};

	cweeUnitPattern& ShiftTime(const cweeUnitValues::unit_value& deltaTime) {
		AUTO g{ lock.Guard() };
		container->ShiftTime(deltaTime);
		return *this;
	};
	cweeUnitPattern& ScaleY(double ScaleY) {
		AUTO g{ lock.Guard() };
		container->ScaleY(ScaleY);
		return *this;
	};
	cweeUnitPattern& Translate(const cweeUnitValues::unit_value& translation) {
		AUTO g{ lock.Guard() };
		container->Translate(translation);
		return *this;
	};

	cweeUnitPattern& RemoveUnnecessaryKnots(const cweeUnitValues::unit_value& timeStart = -std::numeric_limits < cweeUnitValues::unit_value>::max(), const cweeUnitValues::unit_value& timeEnd = std::numeric_limits < cweeUnitValues::unit_value>::max()) {
		AUTO g{ lock.Guard() };
		container->RemoveUnnecessaryKnots(timeStart, timeEnd);
		return *this;
	};

	cweeUnitPattern& Copy(const cweeUnitPattern& o, const cweeUnitValues::unit_value& timeStart = -std::numeric_limits < cweeUnitValues::unit_value>::max(), const cweeUnitValues::unit_value& timeEnd = std::numeric_limits < cweeUnitValues::unit_value>::max()) {
		if (this == &o) return *this;

		AUTO g{ lock.Guard() };
		AUTO g1{ o.lock.Guard() };

		container = container->CopyAnotherContainer(o.container); // gets the values of another, uses my units			

		boundaryType = o.boundaryType;
		interpolationType = o.interpolationType;
		return *this;
	};

	cweeThreadedList<pairT>								GetKnotSeries(const cweeUnitValues::unit_value& timeStart = -std::numeric_limits < cweeUnitValues::unit_value>::max(), const cweeUnitValues::unit_value& timeEnd = std::numeric_limits <cweeUnitValues::unit_value>::max()) const {
		int numKnots = this->GetNumValues();
		cweeThreadedList<pairT> toReturn(numKnots + 16);

		auto t0 = (container->internal_X_type = timeStart);
		auto t1 = (container->internal_X_type = timeEnd);

		AUTO g{ lock.Guard() };
		for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
			if (x.Y) {
				if (x.X >= t0) {
					if (x.X < t1) {
						auto& set = toReturn.Alloc();
						set.first = (container->internal_X_type = x.X);
						set.second = (container->internal_Y_type = *x.Y);
					}
					else {
						break;
					}
				}
			}
		}

		return toReturn;
	};

	cweeUnitPattern&												ClampValues(const cweeUnitValues::unit_value& min, const cweeUnitValues::unit_value& max) {
		AUTO g{ lock.Guard() };
		for (auto& x : *container) {
			*x.Y = (container->internal_Y_type = cweeUnitValues::math::clamp((container->internal_Y_type = *x.Y), min, max))();
		}
		return *this;
	};

	cweeUnitValues::unit_value											GetValueAtIndex(size_t index) {
		AUTO g{ lock.Guard() };
		int n = 0;
		for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
			if (x.Y) {
				if (n >= index) {
					return (container->internal_Y_type = *x.Y);
				}
				n++;
			}
		}
		return cweeUnitValues::unit_value();
	};
	cweeUnitValues::unit_value											GetTimeAtIndex(size_t index) {
		AUTO g{ lock.Guard() };
		int n = 0;
		for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
			if (x.Y) {
				if (n >= index) {
					return (container->internal_X_type = x.X);
				}
				n++;
			}
		}
		return cweeUnitValues::unit_value();
	};

	size_t												GetLargestSmallerOrEqualTime(const cweeUnitValues::unit_value& time) const {
		AUTO g{ lock.Guard() };
		size_t n = -1;
		for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
			if (x.Y) {
				if (x.X > (container->internal_X_type = time)) {
					return n;
				}
				n++;
			}
		}
		return n;
	};

private:
	bool												IsLooped() const {
		bool out = false;
		lock.Lock();
		out = (boundaryType == boundary_t::BT_LOOP);
		lock.Unlock();
		return out;
	};
	bool												IsClamped() const {
		bool out = false;
		lock.Lock();
		out = (boundaryType == boundary_t::BT_CLAMPED);
		lock.Unlock();
		return out;
	};
	cweeUnitValues::unit_value											LoopedTime(cweeUnitValues::unit_value t, bool forceLoop = false) const {
		if (forceLoop || IsLooped()) {
			cweeUnitValues::unit_value minTime, maxTime, len, currentTime, avgStep;
			minTime = GetMinTime();
			maxTime = GetMaxTime();
			{
				AUTO g{ lock.Guard() };
				container->internal_X_type = t;
				t.Clear();
				t = container->internal_X_type;

			}

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
	cweeUnitValues::unit_value											ClampedTime(cweeUnitValues::unit_value t) const {
		if (IsClamped()) {
			cweeUnitValues::unit_value mT = this->GetMinTime();
			{
				AUTO g{ lock.Guard() };
				container->internal_X_type = t;
				t.Clear();
				t = container->internal_X_type;
			}
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
	void												Basis(const cweeUnitValues::unit_value& t, u64* bvals) const {
		lock.Lock();
		UnsafeBasis(t, bvals);
		lock.Unlock();
	};
	void												UnsafeBasis(const cweeUnitValues::unit_value& t, u64* bvals) const {
		container->UnsafeBasis(t, bvals, interpolationType);
	};

public:

	cweeUnitValues::unit_value											GetCurrentValue(cweeUnitValues::unit_value time) const {
		cweeUnitValues::unit_value out;
		time = LoopedTime(ClampedTime(time));
		lock.Lock();
		out = container->GetCurrentValue(time, interpolationType);
		lock.Unlock();
		return out;
	};
	/*! Return approximate derivative of spline at time */
	cweeUnitValues::unit_value											GetCurrentFirstDerivative(cweeUnitValues::unit_value time) const {
		cweeUnitValues::unit_value step; {
			AUTO g{ lock.Guard() };
			container->internal_X_type = time; time.Clear(); time = container->internal_X_type;
			step = (container->internal_X_type = 0.01);
		}
		return (GetCurrentValue(time + step) - GetCurrentValue(time - step)) / (step * 2.0);
	};

	/*! Return approximate derivative of spline */
	cweeUnitPattern											GetFirstDerivative() const {
		cweeUnitPattern result(GetMinTime(), GetCurrentValue(0) / GetMinTime()); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		result.AddValue(GetMinTime(), GetCurrentFirstDerivative(GetMinTime()));


		cweeUnitPatternContainer_t::ConstIterType prevValue;
		prevValue.Y = nullptr;

		for (AUTO dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
			if (dataPair.Y) {
				if (prevValue.Y) {
					if (dataPair.X > prevValue.X) {
						result.AddValue((prevValue.X + dataPair.X) / 2.0, (*dataPair.Y - *prevValue.Y) / (dataPair.X - prevValue.X));
					}
				}
				prevValue = dataPair;
			}
		}
		result.AddValue(GetMaxTime(), GetCurrentFirstDerivative(GetMaxTime()));
		return result;
	};

	/*! Return absolute-value of spline */
	cweeUnitPattern											Abs() const {
		cweeUnitPattern result(GetMinTime(), GetCurrentValue(0)); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		for (AUTO dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
			if (dataPair.Y) {
				result.AddValue(dataPair.X, cweeUnitValues::math::abs(*dataPair.Y));
			}
		}

		return result;
	};

	/*! Return the maximum of two patterns */
	cweeUnitPattern											Max(cweeUnitPattern const& other) const {
		cweeUnitPattern result(this->X_Type(), this->Y_Type()); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		auto temp = *this * other;
		for (auto& dataPair : const_cast<const cweeUnitPatternContainer_t&>(*temp.container)) {
			if (dataPair.Y) {				
				result.AddValue(dataPair.X, cweeUnitValues::math::max(other.GetCurrentValue(dataPair.X), GetCurrentValue(dataPair.X)));
			}
		}
		result.RemoveUnnecessaryKnots();
		return result;
	};
	
	/*! Return the minimum of two patterns */
	cweeUnitPattern											Min(cweeUnitPattern const& other) const {
		cweeUnitPattern result(this->X_Type(), this->Y_Type()); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		auto temp = *this * other;
		for (auto& dataPair : const_cast<const cweeUnitPatternContainer_t&>(*temp.container)) {
			if (dataPair.Y) {
				result.AddValue(dataPair.X, cweeUnitValues::math::min(other.GetCurrentValue(dataPair.X), GetCurrentValue(dataPair.X)));
			}
		}
		result.RemoveUnnecessaryKnots();
		return result;
	};

	/*! Return absolute-value of spline */
	cweeUnitPattern											RoundNearest(float roundToMagnitude) const {
		cweeUnitPattern result(GetMinTime(), GetCurrentValue(0)); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		for (AUTO dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
			if (dataPair.Y) {
				result.AddValue(dataPair.X, cweeUnitValues::math::round(*dataPair.Y, roundToMagnitude));
			}
		}

		return result;
	};

	/*! Return absolute-value of spline */
	cweeUnitPattern											Floor() const {
		cweeUnitPattern result(GetMinTime(), GetCurrentValue(0)); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		for (AUTO dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
			if (dataPair.Y) {
				result.AddValue(dataPair.X, cweeUnitValues::math::floor(*dataPair.Y));
			}
		}

		return result;
	};

	/*! Return absolute-value of spline */
	cweeUnitPattern											Ceiling() const {
		cweeUnitPattern result(GetMinTime(), GetCurrentValue(0)); {
			result.SetBoundaryType(GetBoundaryType());
			result.SetInterpolationType(GetInterpolationType());
		}

		for (AUTO dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
			if (dataPair.Y) {
				result.AddValue(dataPair.X, cweeUnitValues::math::ceiling(*dataPair.Y));
			}
		}

		return result;
	};

	/*! Return approximate cweeUnitValues::second derivative of spline at time */
	cweeUnitValues::unit_value											GetCurrentSecondDerivative(cweeUnitValues::unit_value time) const {
		cweeUnitValues::unit_value step; {
			AUTO g{ lock.Guard() };
			container->internal_X_type = time; time.Clear(); time = container->internal_X_type;
			step = (container->internal_X_type = 0.01);
		}
		return (GetCurrentFirstDerivative(time + step) - GetCurrentFirstDerivative(time - step)) / (step * 2.0);
	};

	/*! Get list of spline samples at a specified Timestep */
	cweeThreadedList<pairT>								GetTimeSeries(const cweeUnitValues::unit_value& timeStart, const cweeUnitValues::unit_value& timeEnd, const cweeUnitValues::unit_value& timeStep) const {
		pairT v;
		cweeUnitValues::unit_value realTimestep = (container->internal_X_type = timeStep); if (realTimestep < (container->internal_X_type = 1)) realTimestep = (container->internal_X_type = 1);

		cweeThreadedList<pairT> out(cweeMath::max(cweeMath::min(((u64)(timeEnd - timeStart) / (u64)(realTimestep)), 100000), 1000) + 16);

		v.first = (container->internal_X_type = timeStart); v.second = GetCurrentValue(timeStart);
		out.Append(v); // ensure pattern always has a starter? 

		for (v.first = (container->internal_X_type = (timeStart + realTimestep)); v.first < timeEnd; v.first += realTimestep) {
			v.second = GetCurrentValue(v.first);
			out.Append(v);
		}

		v.first = timeEnd; v.second = GetCurrentValue(timeEnd);
		out.Append(v); // ensure pattern always has a closure? 

		return out;
	};

	cweeThreadedList<cweeUnitValues::unit_value>						GetValueTimeSeries(const cweeUnitValues::unit_value& timeStart, const cweeUnitValues::unit_value& timeEnd, const cweeUnitValues::unit_value& timeStep) const {
		cweeUnitValues::unit_value realTimestep = timeStep; if (realTimestep < 1) realTimestep = 1;
		cweeThreadedList<cweeUnitValues::unit_value> out(cweeMath::max(cweeMath::min((u64)((u64)(timeEnd - timeStart) / (u64)(realTimestep)), 100000), 1000) + 16);

		out.Append(GetCurrentValue(timeStart)); // ensure pattern always has a starter? 
		for (cweeUnitValues::unit_value t = timeStart + realTimestep; t < timeEnd; t += realTimestep) {
			out.Append(GetCurrentValue(t));
		}
		out.Append(GetCurrentValue(timeEnd)); // ensure pattern always has a closure? 

		return out;
	};
	cweeThreadedList<cweeUnitValues::unit_value>						GetValueTimeSeries(const cweeUnitValues::unit_value& timeStart, const cweeUnitValues::unit_value& timeEnd, const cweeUnitValues::unit_value& timeStep, const cweeUnitPattern& mask) const {
		cweeUnitValues::unit_value realTimestep = timeStep; if (realTimestep < 1) realTimestep = 1;
		cweeThreadedList<cweeUnitValues::unit_value> out(cweeMath::max(cweeMath::min((u64)((u64)(timeEnd - timeStart) / (u64)(realTimestep)), 100000), 1000) + 16);

		if (mask.GetCurrentValue(timeStart) <= 0) out.Append(GetCurrentValue(timeStart)); // ensure pattern always has a starter? 
		for (cweeUnitValues::unit_value t = timeStart + realTimestep; t < timeEnd; t += realTimestep) {
			if (mask.GetCurrentValue(t) <= 0) out.Append(GetCurrentValue(t));
		}
		if (mask.GetCurrentValue(timeEnd) <= 0) out.Append(GetCurrentValue(timeEnd)); // ensure pattern always has a closure? 

		return out;
	};
	
	cweeUnitValues::unit_value StdDev() const {
		AUTO out = cweeUnitPattern(this->X_Type(), 1);
		out = *this;
		return (this->Y_Type() = ((((out - out.GetAvgValue()).pow(2.0)).GetAvgValue()).pow(0.5)));
	};
	cweeUnitValues::unit_value StdDev(const cweeUnitPattern& mask) const {
		AUTO out = cweeUnitPattern(this->X_Type(), 1);
		out = *this;
		auto minT = out.GetMinTime();
		auto maxT = out.GetMaxTime();
		return (this->Y_Type() = (out - out.GetAvgValue(minT, maxT, mask)).pow(2.0).GetAvgValue(minT, maxT, mask).pow(0.5));
	};
	cweeUnitPattern GetOutlierMask() const {
		AUTO GetOutlierMask = [](cweeUnitPattern const& data)->cweeUnitPattern {
			AUTO out{ cweeUnitPattern(data.X_Type(), 1) };
			cweeList<double> quantileSearch; quantileSearch.Append(0.25); quantileSearch.Append(0.75); quantileSearch.Append(0.95); 
			if (1) { // Conventional Outlier Removal (Y-Value)
				AUTO quantiles{ data.GetValueQuantiles(quantileSearch) };
				AUTO stddev = data.StdDev();
				AUTO low_bar = quantiles[0] - 1.5 * stddev;
				AUTO high_bar = quantiles[1] + 1.5 * stddev;

				AUTO findOutlierPeriods = [&low_bar, &high_bar](cweeUnitPattern data) {
					AUTO Data_TooHigh = (data - high_bar);
					Data_TooHigh.ClampValues(0, 1);

					AUTO Data_TooLow = (low_bar - data);
					Data_TooLow.ClampValues(0, 1);

					AUTO DataOutOfRange = (Data_TooHigh + Data_TooLow).Ceiling();
					DataOutOfRange.ClampValues(0, 1);

					DataOutOfRange.RemoveUnnecessaryKnots();

					return DataOutOfRange;
				};
				out += findOutlierPeriods(data);
			}
		    out = out.Ceiling().ClampValues(0,1);
			if (0) { // Distance-Based Outlier Removal (Odd-Man-Out)
				AUTO removeExceptionalPeriods = [&quantileSearch](cweeUnitPattern data) {
					AUTO distances = data.GetDistances(true);
					AUTO quantiles = distances.GetValueQuantiles(quantileSearch);
					AUTO stddev = distances.StdDev();
					AUTO low_bar = quantiles[0] - 2.5 * stddev;
					AUTO high_bar = quantiles[1] + 2.5 * stddev;

					AUTO Data_TooHigh = (distances - high_bar);
					Data_TooHigh.ClampValues(0, 1);

					AUTO Data_TooLow = (low_bar - distances);
					Data_TooLow.ClampValues(0, 1);

					AUTO DataOutOfRange = (Data_TooHigh + Data_TooLow).Ceiling();
					DataOutOfRange.ClampValues(0, 1);

					DataOutOfRange.RemoveUnnecessaryKnots();

					return DataOutOfRange;
				};
				out += removeExceptionalPeriods(data);
			}
			out = out.Ceiling().ClampValues(0, 1);
			if (1) { // Next-Nearest Distance-Based Outlier Removal (Big Gap Discovery)
				AUTO removeExceptionalPeriods = [&quantileSearch](cweeUnitPattern data) {
					AUTO p = data.GetApproximateDistances(false, false);
					cweeList<double> quantilesToSearch; quantilesToSearch.Append(0.5); quantilesToSearch.Append(0.99875); // 0.9999
					auto quantiles = p.GetValueQuantiles(quantilesToSearch);
					AUTO p2 = p - (quantiles[0] + 1.5 * quantiles[1]); 
					// AUTO p2 = (p - (quantiles[0] + 1.5 * cweeUnitValues::math::max(quantiles[1], p.StdDev())));
				    AUTO p3 = p2.ClampValues(0, 1).Ceiling();
					p3.RemoveUnnecessaryKnots();
					return p3;
				};
				out += removeExceptionalPeriods(data);
			}
			return out;
		};
		
		AUTO init = GetOutlierMask(*this).Ceiling();

#if 1
		init.ClampValues(0, 1);

		cweeUnitPattern out(this->X_Type(), 1);

		for (auto& x : init.GetTimeSeries(init.GetMinTime(), init.GetMaxTime(), init.GetMinimumTimeStep()))
			out.AddValue(x.first, x.second);

		cweeUnitPattern out2 = (out + init).Ceiling();
		out2.ClampValues(0, 1);
		out2.RemoveUnnecessaryKnots();
		return out2;
#else
		init.RemoveUnnecessaryKnots();

		cweeUnitPattern out(this->X_Type(), 1);
		
		cweeUnitValues::unit_value tempV{ cweeMath::INF };
		cweeUnitValues::unit_value tempV2{ cweeMath::INF };
		auto iter_maxT = init.GetMaxTime();
		auto iter_stepT = init.GetMinimumTimeStep();
		for (auto iter_t = init.GetMinTime(); iter_t <= iter_maxT; iter_t += iter_stepT) {
			tempV = init.GetCurrentValue(iter_t);
			if (tempV != tempV2) {
				tempV2 = tempV;
				out.AddValue(iter_t, tempV);
			}
		}
		out.RemoveUnnecessaryKnots();

		cweeUnitPattern out2 = (out + init).Ceiling();
		out2.ClampValues(0, 1);
		out2.RemoveUnnecessaryKnots();

		return out2;
#endif
	};










private:
	template<typename T>
	static AUTO R_Squared(const cweeList<T>& Real, const cweeList<T>& Estimate) {
		T avg_real; { avg_real = 0; int numSamples = 0;
		for (auto& x : Real) cweeMath::rollingAverageRef(avg_real, x, numSamples);
		}

		int N = cweeMath::min(Real.Num(), Estimate.Num());

		T real_error{ 0 };
		T estimate_error{ 0 };
		AUTO SumErrReal = real_error * real_error;
		AUTO SumErrEstimate = estimate_error * estimate_error;

		for (int i = 0; i < N; i++) {
			real_error = Real[i] - avg_real;
			estimate_error = Real[i] - Estimate[i];
			SumErrReal += real_error * real_error;
			SumErrEstimate += estimate_error * estimate_error;
		};

		if (avg_real != 0) {
			AUTO out = avg_real / avg_real;
			if (SumErrReal != 0) {
				out = 1.0 - (SumErrEstimate / SumErrReal);
			}
			else {
				if (SumErrEstimate == 0) {
					out = 1.0;
				}
				else {
					out = 0.0;
				}
			}
			return out;
		}
		else {
			AUTO out = avg_real / (avg_real + 1.0);
			if (SumErrReal != 0) {
				out = 1.0 - (SumErrEstimate / SumErrReal);
			}
			else {
				if (SumErrEstimate == 0) {
					out = 1.0;
				}
				else {
					out = 0.0;
				}
			}
			return out;
		}
	};

public:
	AUTO												R_Squared(const cweeUnitPattern& other) const {
		cweeUnitValues::scalar out, out2;
		AUTO x0 = cweeUnitValues::math::max(other.GetMinTime(), this->GetMinTime());
		AUTO x1 = cweeUnitValues::math::min(other.GetMaxTime(), this->GetMaxTime());
		if (x1 > x0) {
			double N_steps = cweeMath::max(10, cweeMath::max(this->GetNumValues(), other.GetNumValues())); // at least 10 samples
			AUTO real = this->GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps));
			AUTO estimate = other.GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps));
			out = cweeEng::R_Squared(real, estimate);
			out2 = cweeEng::R_Squared(estimate, real);

			if (out2 > 1 || out > 1) {
				if (out2 < out) {
					out = out2;
				}
			}
			else {
				if (out2 > out) {
					out = out2;
				}
			}

			// if (out > 1) out = 1;
			// if (out < 0) out = 0;
		}
		else {
			out = 0;
		}
		return out;
	};
	AUTO												R_Squared(const cweeUnitPattern& other, const cweeUnitPattern& mask) const {
		cweeUnitValues::scalar out, out2;
		AUTO x0 = cweeUnitValues::math::max(other.GetMinTime(), this->GetMinTime());
		AUTO x1 = cweeUnitValues::math::min(other.GetMaxTime(), this->GetMaxTime());
		if (x1 > x0) {
			double N_steps = cweeMath::max(10, cweeMath::max(this->GetNumValues(), other.GetNumValues())); // at least 10 samples

			AUTO real = this->GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps), mask);
			AUTO estimate = other.GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps), mask);
			
			out = cweeEng::R_Squared(real, estimate);
			out2 = cweeEng::R_Squared(estimate, real);

			if (out2 > 1 || out > 1) {
				if (out2 < out) {
					out = out2;
				}
			}
			else {
				if (out2 > out) {
					out = out2;
				}
			}
		}
		else {
			out = 0;
		}
		return out;
	};
	AUTO												R_Squared(double N_steps, cweeThreadedList<cweeUnitValues::unit_value> const& estimate, const cweeUnitValues::unit_value& from, const cweeUnitValues::unit_value& to) const {
		cweeUnitValues::scalar out;
		AUTO x0 = from;
		AUTO x1 = to;

		if (x1 > x0) {
			AUTO real = this->GetValueTimeSeries(x0, x1, (x1 - x0) * (1.0 / N_steps));
			out = cweeEng::R_Squared(real, estimate);
		}
		else {
			out = 0;
		}
		return out;
	};
	AUTO												R_Squared(const cweeUnitPattern& other, const cweeUnitValues::unit_value& from, const cweeUnitValues::unit_value& to) const {
		cweeUnitValues::scalar out;
		AUTO x0 = from;
		AUTO x1 = to;

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
	AUTO                                                StandardError() const {
		return StdDev() / std::pow(GetNumValues(), 0.5);
	};
	std::pair<double, double>                           PearsonCorrelation(const cweeUnitPattern& populationSet, const cweeUnitPattern& mask) const {
		std::pair<double, double> out; // Pearson's R, and the approximate p-value for the Pearson's R
		{
			auto patAvg = populationSet.GetAvgValue(populationSet.GetMinTime(), populationSet.GetMaxTime(), mask);
			auto finalAvg = this->GetAvgValue(GetMinTime(), GetMaxTime(), mask);
			double sum1 = 0;
			double sum2 = 0;
			double sum3 = 0;
			for (auto& x : populationSet.GetKnotSeries()) {
				sum1 += ((x.second - patAvg)() * (GetCurrentValue(x.first) - finalAvg)());
				sum2 += (x.second - patAvg)() * (x.second - patAvg)();
				sum3 += (GetCurrentValue(x.first) - finalAvg)() * (GetCurrentValue(x.first) - finalAvg)();
			}
			out.first = sum1 / std::pow(sum2 * sum3, 0.5);
		}
		{
			auto n = GetNumValues();
			auto df = n - 2;
			auto t = out.first / std::pow((1 - std::pow(out.first, 2.0)) / (n - 2.0), 0.5);

			AUTO zTest = cweeUnitPattern(1, 1);
			zTest.SetInterpolationType(interpolation_t::SPLINE);
			zTest.SetBoundaryType(boundary_t::BT_FREE);
			zTest.AddValue(0.0, 1.0);
			zTest.AddValue(0.674, 0.5);
			zTest.AddValue(0.842, 0.4);
			zTest.AddValue(1.036, 0.3);
			zTest.AddValue(1.282, 0.2);
			zTest.AddValue(1.645, 0.1);
			zTest.AddValue(1.960, 0.05);
			zTest.AddValue(2.326, 0.02);
			zTest.AddValue(2.576, 0.01);
			zTest.AddValue(3.090, 0.002);
			zTest.AddValue(3.291, 0.001);
			zTest.AddValue(6, 0.000);
			zTest.AddValue(100, 0.000);
			zTest.AddValue(1000, 0.000);
			
			out.second = std::min(1.0, std::max(0.0, zTest.RombergIntegral(t, 1000.0)() * 2.0));
			// out.second = zTest.GetCurrentValue(t)();
		}
		return out;
	};
	
	//double                                              StudentsT(const cweeUnitPattern& populationSet, const cweeUnitPattern& mask) const {
	//	return ((GetAvgValue(GetMinTime(), GetMaxTime(), mask) - populationSet.GetAvgValue(populationSet.GetMinTime(), populationSet.GetMaxTime(), mask)) / ((StdDev()) / std::pow(populationSet.GetNumValues(), 0.5)))();
	//	//auto r12 = 1.0 - PearsonCorrelation(populationSet, mask);
	//	//return r12 * (std::pow((this->GetNumValues() - 2.0), 0.5) / std::pow((1.0 - std::pow(r12, 2.0)), 0.5));
	//};
	///*Assumes the degrees of freedom is sufficiently large to assume Z table for look-up*/
	//double                                              PValue(const cweeUnitPattern& populationSet, const cweeUnitPattern& mask) const {
	//	auto students_t = std::abs(StudentsT(populationSet, mask));
	//	AUTO zTest = cweeUnitPattern(1, 1);
	//	zTest.SetInterpolationType(interpolation_t::SPLINE);
	//	zTest.SetBoundaryType(boundary_t::BT_FREE);
	//	zTest.AddValue(0.0, 1.0);
	//	zTest.AddValue(0.674, 0.5);
	//	zTest.AddValue(0.842, 0.4);
	//	zTest.AddValue(1.036, 0.3);
	//	zTest.AddValue(1.282, 0.2);
	//	zTest.AddValue(1.645, 0.1);
	//	zTest.AddValue(1.960, 0.05);
	//	zTest.AddValue(2.326, 0.02);
	//	zTest.AddValue(2.576, 0.01);
	//	zTest.AddValue(3.090, 0.002);
	//	zTest.AddValue(3.291, 0.001);
	//	return zTest.GetCurrentValue(students_t)();
	//};

	// VIF = 1/(1-R^2); VIF <= 2 means no collinearity. VIF > 2 && < 5 means mild collinearity. VIF >= 5 means strong collinearity.
	bool												Collinear(const cweeUnitPattern& other) const {
		AUTO rsqr = this->R_Squared(other);

		decltype(rsqr) one{ 1 };
		decltype(rsqr) five{ 5 };

		if (rsqr == one) {
			return true;
		}
		else {
			if (five <= (one / (one - rsqr))) {
				return true;
			}
			else {
				return false;
			}
		}
	};
	bool												Collinear(const cweeUnitPattern& other, const cweeUnitPattern& mask) const {
		AUTO rsqr = this->R_Squared(other, mask);

		static decltype(rsqr) one{ 1 };
		static decltype(rsqr) five{ 5 };

		if (rsqr == one) {
			return true;
		}
		else {
			if (five <= (one / (one - rsqr))) {
				return true;
			}
			else {
				return false;
			}
		}
	};

	/*! Request an integration of the time series. The timefactor determines the resulting time component. I.e. A pattern of kilowatt_t and a time factor of hour_t will return a kilowatt_hour_t. */
	cweeUnitValues::unit_value											RombergIntegral(cweeUnitValues::unit_value t0, cweeUnitValues::unit_value t1, cweeUnitPattern const& mask) const {
		return (cweeUnitPattern(*this) * (1.0 - mask)).RombergIntegral(t0, t1);
	};
	/*! Request an integration of the time series. The timefactor determines the resulting time component. I.e. A pattern of kilowatt_t and a time factor of hour_t will return a kilowatt_hour_t. */
	cweeUnitValues::unit_value											RombergIntegral(cweeUnitValues::unit_value t0, cweeUnitValues::unit_value t1) const {
		using node_type = cweeBalancedTree<double, double, 10>::cweeBalancedTreeNode;
		cweeUnitValues::unit_value step, t, stepDiv2, maxT;
		cweeUnitValues::unit_value sum;

		{
			AUTO g{ lock.Guard() };
			sum = ((container->internal_X_type * container->internal_Y_type) = 0);

			step = container->internal_X_type;

			container->internal_X_type = t0;
			t0.Clear();
			t0 = container->internal_X_type;

			container->internal_X_type = t1;
			t1.Clear();
			t1 = container->internal_X_type;
		}

		switch (this->GetInterpolationType()) {
		case interpolation_t::LEFT: {
			if (this->GetNumValues() > 1) {
				step = this->GetMinimumTimeStep();
				cweeUnitValues::unit_value minGot = t1, maxGot = t1;
				if (true) {

					cweeUnitPatternContainer_t::ConstIterType prevValue;
					prevValue.Y = nullptr;
					AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
					for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(t0); iter != endIter; ++iter) {
						auto& dataPair = *iter;
						if (dataPair.Y) {
							if (dataPair.X >= t0) {
								if (dataPair.X <= t1) {
									if (prevValue.Y) {
										// sum += (container->internal_Y_type = ((*prevValue.Y))) * (container->internal_X_type = (dataPair.X - prevValue.X));
										sum += ((*prevValue.Y) * (dataPair.X - prevValue.X));
									}
									else {
										minGot = dataPair.X;
									}
									prevValue = dataPair;
								}
								else {
									break;
								}
							}
						}
					}
					if (prevValue.Y) {
						maxGot = prevValue.X;
					}
				}

				{
					AUTO g{ lock.Guard() };
					t = (container->internal_X_type = 0);
					stepDiv2 = (container->internal_X_type = (step / 2.0));
					maxT = (container->internal_X_type = (t1 + stepDiv2));
				}

				
				t = t0; //for (t = t0; (t + step) < minGot; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				t = maxGot; //for (t = maxGot; (t + step) < t1; t += step) sum += step * GetCurrentValue(t + stepDiv2);				
				if (t1 > t)  sum += (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}

		}
								  break;
		case interpolation_t::RIGHT: {
			if (this->GetNumValues() > 1) {
				step = this->GetMinimumTimeStep();
				cweeUnitValues::unit_value minGot = t1, maxGot = t1;
				if (true) {
					cweeUnitPatternContainer_t::ConstIterType prevValue;
					prevValue.Y = nullptr;
					AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
					for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(t0); iter != endIter; ++iter) {
						auto& dataPair = *iter;
						if (dataPair.Y) {
							if (dataPair.X >= t0) {
								if (dataPair.X <= t1) {
									if (prevValue.Y) {
										// sum += (container->internal_Y_type = ((*dataPair.Y))) * (container->internal_X_type = (dataPair.X - prevValue.X));
										sum += ((*dataPair.Y) * (dataPair.X - prevValue.X));
									}
									else {
										minGot = dataPair.X;
									}
									prevValue = dataPair;
								}
								else {
									break;
								}
							}
						}
					}
					if (prevValue.Y) {
						maxGot = prevValue.X;
					}
				}

				{
					AUTO g{ lock.Guard() };
					t = (container->internal_X_type = 0);
					stepDiv2 = (container->internal_X_type = (step / 2.0));
					maxT = (container->internal_X_type = (t1 + stepDiv2));
				}

				t = t0;//for (t = t0; (t + step) < minGot; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				t = maxGot;// for (t = maxGot; (t + step) < t1; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (t1 > t)  sum += (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}

		}
								   break;
		default: {
			if (this->GetNumValues() > 1) {
				step = this->GetMinimumTimeStep();
				cweeUnitValues::unit_value minGot = t1, maxGot = t1;
				if (true) {
					cweeUnitPatternContainer_t::ConstIterType prevValue;
					prevValue.Y = nullptr;
					AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
					for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(t0); iter != endIter; ++iter) {
						auto& dataPair = *iter;
						if (dataPair.Y) {
							if (dataPair.X >= t0) {
								if (dataPair.X <= t1) {
									if (prevValue.Y) {
										// sum += (container->internal_Y_type = ((*prevValue.Y + *dataPair.Y)*0.5)) * (container->internal_X_type = (dataPair.X - prevValue.X));
										sum += (((*prevValue.Y + *dataPair.Y) * 0.5) * (dataPair.X - prevValue.X));
									}
									else {
										minGot = dataPair.X;
									}
									prevValue = dataPair;
								}
								else {
									break;
								}
							}
						}
					}
					if (prevValue.Y) {
						maxGot = prevValue.X;
					}
				}

				{
					AUTO g{ lock.Guard() };
					t = (container->internal_X_type = 0);
					stepDiv2 = (container->internal_X_type = (step / 2.0));
					maxT = (container->internal_X_type = (t1 + stepDiv2));
				}

				t = t0;//  for (t = t0; (t + step) < minGot; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				t = maxGot;// for (t = maxGot; (t + step) < t1; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (t1 > t)  sum += (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}

		}
			   break;
		}

		return sum;
	};
	/*! Request an integration of the time series. The timefactor determines the resulting time component. I.e. A pattern of kilowatt_t and a time factor of hour_t will return a kilowatt_hour_t. */
	cweeUnitValues::unit_value											RombergIntegral() const {
		using node_type = cweeBalancedTree<double, double, 10>::cweeBalancedTreeNode;
		cweeUnitValues::unit_value step, sum, t, stepDiv2, maxT;
		cweeUnitValues::unit_value t0 = GetMinTime(), t1 = GetMaxTime();
		{
			AUTO g{ lock.Guard() };
			sum = container->internal_X_type * container->internal_Y_type;
			sum = 0;

			step = container->internal_X_type;

			container->internal_X_type = t0;
			t0.Clear();
			t0 = container->internal_X_type;

			container->internal_X_type = t1;
			t1.Clear();
			t1 = container->internal_X_type;
		}

		switch (this->GetInterpolationType()) {
		case interpolation_t::LEFT: {
			if (this->GetNumValues() > 1) {
				step = this->GetMinimumTimeStep();
				cweeUnitValues::unit_value minGot = t1, maxGot = t1;
				if (true) {
					cweeUnitPatternContainer_t::ConstIterType prevValue;
					for (auto& dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
						if (dataPair.Y) {
							if (prevValue.Y) {
								sum += (container->internal_Y_type = ((*prevValue.Y))) * (container->internal_X_type = (dataPair.X - prevValue.X));
							}
							else {
								minGot = dataPair.X;
							}
							prevValue = dataPair;
						}
					}
					if (prevValue.Y) {
						maxGot = prevValue.X;
					}
				}

				{
					AUTO g{ lock.Guard() };
					t = (container->internal_X_type = 0);
					stepDiv2 = (container->internal_X_type = (step / 2.0));
					maxT = (container->internal_X_type = (t1 + stepDiv2));
				}

				for (t = t0; (t + step) < minGot; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				for (t = maxGot; (t + step) < t1; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (t1 > t)  sum += (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}

		}
								  break;
		case interpolation_t::RIGHT: {
			if (this->GetNumValues() > 1) {
				step = this->GetMinimumTimeStep();
				cweeUnitValues::unit_value minGot = t1, maxGot = t1;
				if (true) {
					cweeUnitPatternContainer_t::ConstIterType prevValue;
					for (auto& dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
						if (dataPair.Y) {
							if (prevValue.Y) {
								sum += (container->internal_Y_type = ((*dataPair.Y))) * (container->internal_X_type = (dataPair.X - prevValue.X));
							}
							else {
								minGot = dataPair.X;
							}
							prevValue = dataPair;
						}
					}
					if (prevValue.Y) {
						maxGot = prevValue.X;
					}
				}

				{
					AUTO g{ lock.Guard() };
					t = (container->internal_X_type = 0);
					stepDiv2 = (container->internal_X_type = (step / 2.0));
					maxT = (container->internal_X_type = (t1 + stepDiv2));
				}

				for (t = t0; (t + step) < minGot; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				for (t = maxGot; (t + step) < t1; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (t1 > t)  sum += (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}

		}
								   break;
		default: {
			if (this->GetNumValues() > 1) {
				step = this->GetMinimumTimeStep();
				cweeUnitValues::unit_value minGot = t1, maxGot = t1;
				if (true) {
					cweeUnitPatternContainer_t::ConstIterType prevValue;
					for (auto& dataPair : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
						if (dataPair.Y) {
							if (prevValue.Y) {
								sum += (container->internal_Y_type = ((*prevValue.Y + *dataPair.Y) * 0.5)) * (container->internal_X_type = (dataPair.X - prevValue.X));
							}
							else {
								minGot = dataPair.X;
							}
							prevValue = dataPair;
						}
					}
					if (prevValue.Y) {
						maxGot = prevValue.X;
					}
				}

				{
					AUTO g{ lock.Guard() };
					t = (container->internal_X_type = 0);
					stepDiv2 = (container->internal_X_type = (step / 2.0));
					maxT = (container->internal_X_type = (t1 + stepDiv2));
				}

				for (t = t0; (t + step) < minGot; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (minGot > t) sum += (minGot - t) * GetCurrentValue(t + ((minGot - t) / 2.0));
				for (t = maxGot; (t + step) < t1; t += step) sum += step * GetCurrentValue(t + stepDiv2);
				if (t1 > t)  sum += (t1 - t) * GetCurrentValue(t + ((t1 - t) / 2.0));
			}

		}
			   break;
		}

		return sum;
	};

	cweeList<cweeUnitValues::unit_value>								GetValueQuantiles(cweeList<double> const& probs, cweeUnitPattern const& mask) const {
		cweeList<cweeUnitValues::unit_value> quantiles;
		if (probs.Num() > 0) {
			quantiles.SetNum(probs.Num());

			cweeUnitValues::cweeUnitValues::scalar poi;
			size_t left, right;

			cweeList<cweeUnitValues::unit_value> data(this->GetNumValues() + 16);
			for (auto& iter : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
				if (iter.Y && !(mask.GetCurrentValue(iter.X) > 0)) {
					data.Append(*iter.Y);
				}
			}

			if (data.Num() < probs.Num()) {
				data.AssureSize(probs.Num());
			}
			if (data.Num() <= 1) return quantiles;

			data.Sort(); // sort data

			for (size_t i = 0; i < probs.Num(); ++i) {
				poi = (1.0f - probs[i]) * -0.5f + probs[i] * ((float)data.Num() - 0.5f);
				left = std::max(int64_t(std::floor((double)poi)), int64_t(0));
				right = std::min(int64_t(std::ceil((double)poi)), int64_t((float)data.Num() - 1.0f));
				quantiles[i] = (((cweeUnitValues::cweeUnitValues::scalar)1.0f - (poi - (cweeUnitValues::cweeUnitValues::scalar)left)) * data[left]) + ((poi - (cweeUnitValues::cweeUnitValues::scalar)left) * data[right]);
			}
		}
		return quantiles;
	};
	cweeList<cweeUnitValues::unit_value>								GetValueQuantiles(cweeList<double> const& probs) const {
		cweeList<cweeUnitValues::unit_value> quantiles;
		if (probs.Num() > 0) {
			quantiles.SetNum(probs.Num());

			cweeUnitValues::cweeUnitValues::scalar poi;
			size_t left, right;

			cweeList<cweeUnitValues::unit_value> data(this->GetNumValues() + 16);
			for (auto& iter : const_cast<const cweeUnitPatternContainer_t&>(*this->container)) {
				if (iter.Y) {
					data.Append(*iter.Y);
				}
			}

			if (data.Num() < probs.Num()) {
				data.AssureSize(probs.Num());
			}
			if (data.Num() <= 1) return quantiles;

			data.Sort(); // sort data

			for (size_t i = 0; i < probs.Num(); ++i) {
				poi = (1.0f - probs[i]) * -0.5f + probs[i] * ((float)data.Num() - 0.5f);
				left = std::max(int64_t(std::floor((double)poi)), int64_t(0));
				right = std::min(int64_t(std::ceil((double)poi)), int64_t((float)data.Num() - 1.0f));
				quantiles[i] = (((cweeUnitValues::cweeUnitValues::scalar)1.0f - (poi - (cweeUnitValues::cweeUnitValues::scalar)left)) * data[left]) + ((poi - (cweeUnitValues::cweeUnitValues::scalar)left) * data[right]);
			}
		}
		return quantiles;
	};
	cweeList<cweeUnitValues::unit_value>								GetValueQuantiles(cweeList<double> const& probs, cweeUnitValues::unit_value start, cweeUnitValues::unit_value end) const {
		cweeList<cweeUnitValues::unit_value> quantiles;
		if (probs.Num() > 0) {
			quantiles.SetNum(probs.Num());

			cweeUnitValues::cweeUnitValues::scalar poi;
			size_t left, right;

			cweeList<cweeUnitValues::unit_value> data(this->GetNumValues() + 16);
			AUTO EndIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
			for (AUTO iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(start); iter != EndIter; ++iter) {
				if (iter->X >= start && iter->Y) {
					if (iter->X > end) break;
					data.Append(*iter->Y);
				}
			}

			if (data.Num() < probs.Num()) {
				data.AssureSize(probs.Num());
			}
			if (data.Num() <= 1) return quantiles;

			data.Sort(); // sort data

			for (size_t i = 0; i < probs.Num(); ++i) {
				poi = (1.0f - probs[i]) * -0.5f + probs[i] * ((float)data.Num() - 0.5f);
				left = std::max(int64_t(std::floor((double)poi)), int64_t(0));
				right = std::min(int64_t(std::ceil((double)poi)), int64_t((float)data.Num() - 1.0f));
				quantiles[i] = (((cweeUnitValues::cweeUnitValues::scalar)1.0f - (poi - (cweeUnitValues::cweeUnitValues::scalar)left)) * data[left]) + ((poi - (cweeUnitValues::cweeUnitValues::scalar)left) * data[right]);
			}
		}
		return quantiles;
	};
	cweeUnitValues::unit_value								            GetValueQuantile(double prob, cweeUnitValues::unit_value start, cweeUnitValues::unit_value end) const {
		cweeUnitValues::unit_value quantiles;
		{
			cweeUnitValues::cweeUnitValues::scalar poi;
			size_t left, right;

			cweeList<cweeUnitValues::unit_value> data(this->GetNumValues() + 16);
			AUTO EndIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
			for (AUTO iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(start); iter != EndIter; ++iter) {
				if (iter->X >= start && iter->Y) {
					if (iter->X > end) break;
					data.Append(*iter->Y);
				}
			}

			if (data.Num() <= 1) return quantiles;

			data.Sort(); // sort data

			{
				poi = (1.0f - prob) * -0.5f + prob * ((float)data.Num() - 0.5f);
				left = std::max(int64_t(std::floor((double)poi)), int64_t(0));
				right = std::min(int64_t(std::ceil((double)poi)), int64_t((float)data.Num() - 1.0f));
				quantiles = (((cweeUnitValues::cweeUnitValues::scalar)1.0f - (poi - (cweeUnitValues::cweeUnitValues::scalar)left)) * data[left]) + ((poi - (cweeUnitValues::cweeUnitValues::scalar)left) * data[right]);
			}
		}
		return quantiles;
	};

	cweeUnitValues::unit_value											GetAvgTimestep() const {
		cweeUnitValues::unit_value out;

		int num(0);

		AUTO g{ lock.Guard() };
		out = (container->internal_X_type = 0);

		AUTO minT = this->GetMinTime(), maxT = this->GetMaxTime();
		if (minT >= maxT) return out;


		if (this->GetNumValues() > 1) {
			cweeUnitPatternContainer_t::ConstIterType prevValue;
			prevValue.Y = nullptr;
			int numSamples = 0;

			AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
			for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin_at(minT); iter != endIter; ++iter) {
				auto& dataPair = *iter;
				if (dataPair.Y) {
					prevValue = dataPair;

					if (dataPair.X >= minT) {
						if (dataPair.X <= maxT) {
							if (prevValue.Y) {
								cweeMath::rollingAverageRef(out, (dataPair.X - prevValue.X), numSamples);
							}
							prevValue = dataPair;
						}
					}
				}
			}
		}

		return out;
	};

	/* Calculates the average value from the observations, and is not volume- or time-weighted. */
	cweeUnitValues::unit_value											GetNumericalAvgValue() const {
		cweeUnitValues::unit_value currentAverage;

		AUTO g{ lock.Guard() };
		currentAverage = (container->internal_Y_type = 0);

		int numSamples = 0;

		AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
		for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin(); iter != endIter; ++iter) {
			auto& dataPair = *iter;
			if (dataPair.Y) {
				numSamples++;
				currentAverage -= (currentAverage / (double)numSamples);
				currentAverage += (*dataPair.Y / (double)numSamples);
			}
		}
		return currentAverage;
	};
	cweeUnitValues::unit_value											GetNumericalAvgValue(cweeUnitPattern const& mask) const {
		cweeUnitValues::unit_value currentAverage;

		AUTO g{ lock.Guard() };
		currentAverage = (container->internal_Y_type = 0);

		int numSamples = 0;

		AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
		for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin(); iter != endIter; ++iter) {
			auto& dataPair = *iter;
			if (mask.GetCurrentValue(dataPair.X) > 0) continue;
			if (dataPair.Y) {
				numSamples++;
				currentAverage -= (currentAverage / (double)numSamples);
				currentAverage += (*dataPair.Y / (double)numSamples);
			}
		}
		return currentAverage;
	};

	/* Calculates the volume- and time-weighted average of the timeseries. Closest to "ground truth" for inconsistent or variable measurements. */
	cweeUnitValues::unit_value											GetAvgValue() const {
		cweeUnitValues::unit_value out;

		int num(0);

		AUTO g{ lock.Guard() };
		out = (container->internal_Y_type = 0);

		AUTO minT = this->GetMinTime(), maxT = this->GetMaxTime();
		if (minT >= maxT) return out;
		out = this->RombergIntegral(minT, maxT) / (maxT - minT);

		return out;
	};
	cweeUnitValues::unit_value											GetAvgValue(cweeUnitValues::unit_value start, cweeUnitValues::unit_value end, cweeUnitPattern const& mask) const {
		cweeUnitValues::unit_value out;

		int num(0);
		int n = GetNumValues();
		if (n == 0) return out;
		AUTO g{ lock.Guard() };
		out = (container->internal_Y_type = 0);
		container->internal_X_type = start; start.Clear(); start = container->internal_X_type;
		container->internal_X_type = end; end.Clear(); end = container->internal_X_type;

		if (start >= end) return out;

		AUTO mask2{ cweeUnitPattern(mask).ClampValues(0,1).Ceiling() };
		cweeUnitValues::unit_value maskAvg = mask2.GetAvgValue(start, end);
		return this->RombergIntegral(start, end, mask2) / ((1.0 - maskAvg()) * (end - start));
	};
	cweeUnitValues::unit_value											GetAvgValue(cweeUnitValues::unit_value start, cweeUnitValues::unit_value end) const {
		cweeUnitValues::unit_value out;

		int num(0);
		int n = GetNumValues();
		if (n == 0) return out;
		AUTO g{ lock.Guard() };
		out = (container->internal_Y_type = 0);
		container->internal_X_type = start; start.Clear(); start = container->internal_X_type;
		container->internal_X_type = end; end.Clear(); end = container->internal_X_type;

		if (start >= end) return out;
		out = this->RombergIntegral(start, end) / (end - start);

		return out;
	};

	cweeUnitPattern                                                     Blur(double desiredNumValues, cweeUnitPattern const& mask) const {
#if 1
		cweeUnitPattern toReturn{ *this };
		toReturn.RemoveWithMask(mask);
		return toReturn.Blur(desiredNumValues);
#else
		AUTO width = this->GetMaxTime() - this->GetMinTime();
		if (width() <= 0 || desiredNumValues <= 0) {
			return cweeUnitPattern(*this);
		}
		AUTO out = cweeUnitPattern(this->X_Type(), this->Y_Type());

		AUTO scale = width / desiredNumValues;

		AUTO mask2{ cweeUnitPattern(mask).ClampValues(0,1).Ceiling() };
		mask2.SetInterpolationType(interpolation_t::LEFT);
		AUTO scaledPat = (*this * (1.0 - mask2));

		AUTO getScaledAvg = [&mask2, &scaledPat, this](cweeUnitValues::unit_value const& start, cweeUnitValues::unit_value const& end)->cweeUnitValues::unit_value {
			cweeUnitValues::unit_value scaledAvg = this->Y_Type();
			cweeUnitValues::unit_value maskAvg = mask2.GetAvgValue(start, end);
			if (maskAvg() != 1) {
				scaledAvg = scaledPat.RombergIntegral(start, end) / ((1.0 - maskAvg()) * (end - start));
			}			
			return scaledAvg;
		};

		cweeUnitValues::unit_value prevV = this->Y_Type();
		AUTO minT = this->GetMinTime();
		AUTO maxT = this->GetMaxTime();

		AUTO scale_half = scale / 2.0;
		for (auto time = minT; time < maxT; time += scale) {
			prevV = getScaledAvg(time, time + scale);
			if (out.GetNumValues() == 0)
				out.AddValue(minT, prevV);
			out.AddValue(time + scale_half, prevV);
		}
		out.AddValue(maxT, prevV);

		out.RemoveUnnecessaryKnots();

		{ // Correct the avg/integral
			AUTO t1 = scaledPat.RombergIntegral(minT, maxT); // this->RombergIntegral(minT, maxT, mask);
			AUTO t2 = out.RombergIntegral(minT, maxT);
			if (t2 != 0) {
				out *= t1 / t2;
			}
		}

		return out;
#endif
	};
	cweeUnitPattern                                                     Blur(double desiredNumValues) const {
		AUTO width = this->GetMaxTime() - this->GetMinTime();
		if (width() <= 0 || desiredNumValues <= 0) {
			return cweeUnitPattern(*this);
		}
		AUTO out = cweeUnitPattern(this->X_Type(), this->Y_Type());

		AUTO scale = width / desiredNumValues;


		cweeUnitValues::unit_value prevV = this->Y_Type();
		AUTO minT = this->GetMinTime();
		AUTO maxT = this->GetMaxTime();

		AUTO scale_half = scale / 2.0;
		for (auto time = minT; time < maxT; time += scale) {
			prevV = this->GetAvgValue(time, time + scale);
			if (out.GetNumValues() == 0)
				out.AddValue(minT, prevV);
			out.AddValue(time + scale_half, prevV);
		}
		out.AddValue(maxT, prevV);

		out.RemoveUnnecessaryKnots();

		{ // Correct the avg/integral
			AUTO t1 = this->RombergIntegral(minT, maxT);
			AUTO t2 = out.RombergIntegral(minT, maxT);
			if (t2 != 0) {
				out *= t1 / t2;
			}
		}
		return out;
	};
	static cweeUnitPattern                                              Lerp(cweeUnitPattern const& WhenOne, cweeUnitPattern const& WhenZero, cweeUnitPattern const& LerpBy) {
		cweeUnitPattern toReturn = WhenZero * (1.0 - LerpBy);
		toReturn += WhenOne * LerpBy;
		return toReturn;
	};
	void												                RemoveTimes(cweeUnitValues::unit_value greaterThanOrEqual, cweeUnitValues::unit_value LessThan) {
		container->RemoveTimes(greaterThanOrEqual, LessThan);
	};
	void                                                                RemoveWithMask(cweeUnitPattern const& other) {
		this->container->RemoveWithMask(other.container);
	};
	AUTO                                                                GetRTree(bool normalized = true) const {
		class RTreeContainer {
		public:
			static cweeBoundary const& GetCoordinates(RTreeContainer const& o) {
				return o.boundary;
			};
			static cwee_units::foot_t GetDistance(RTreeContainer const& o, cweeBoundary const& b) {
				return b.Distance(o.boundary);
			};

			cweeUnitValues::cweeUnitValues::unit_value data;
			cweeBoundary boundary;

			RTreeContainer() : data(), boundary() {};
			RTreeContainer(RTreeContainer const& o) : data(o.data), boundary(o.boundary) {};
			RTreeContainer& operator=(RTreeContainer const& o) {
				this->boundary = o.boundary;
				this->data = o.data;
				return *this;
			};
			bool operator==(RTreeContainer const& b) {
				return data == b.data;
			};
			bool operator!=(RTreeContainer const& b) { return !operator==(b); };
		};

		using RTreeType = RTree< RTreeContainer, RTreeContainer::GetCoordinates, RTreeContainer::GetDistance>;

		RTreeType out;

		{
			AUTO minTime = this->GetMinTime();
			AUTO maxTime = this->GetMaxTime();
			AUTO timeRange = maxTime - minTime;
			AUTO minValue = this->GetMinValue();
			AUTO maxValue = this->GetMaxValue();
			AUTO valueRange = maxValue - minValue;

			AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).end();
			RTreeContainer container;
			for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*this->container).begin(); iter != endIter; ++iter) {
				if (iter->Y) {
					AUTO container{ make_cwee_shared<RTreeContainer>() };
					container->data = iter->X;
					if (normalized) {
						container->boundary.bottomLeft.x = (double)((iter->X - minTime) / timeRange);
						container->boundary.bottomLeft.y = (double)((*iter->Y - minValue) / valueRange);
					}
					else {
						container->boundary.bottomLeft.x = (double)(iter->X);
						container->boundary.bottomLeft.y = (double)(*iter->Y);
					}
					container->boundary.topRight = container->boundary.bottomLeft;
					container->boundary.geographic = false;
					out.Add(container);
				}
			}
		}

		out.GetRoot();

		return out;
	};

	cweeUnitPattern Subdivide(cweeUnitValues::unit_value step) const {
		AUTO out = cweeUnitPattern(this->X_Type(), this->Y_Type());
		auto maxT{ this->GetMaxTime() };
		for (auto t = this->GetMinTime(); t <= maxT; t += step) {
			out.AddValue(t, this->GetCurrentValue(t));
		}
		return out;
	};

	/* returns the distance between each point and its neighbors. */
	cweeUnitPattern  GetDistances(bool normalized = true, int numNearest = 1) const {
		AUTO distances = cweeUnitPattern(this->X_Type(), cweeUnitValues::foot(1));
		if (this->GetNumValues() > 1) {
			AUTO out = GetRTree(normalized);

			AUTO n = out.GetNextLeaf(out.GetRoot());

			cweeList<decltype(out)::TreeNode*> samples;
			double distance;
			int numSamples;
			bool skip;
			while (n && n->object) {
				skip = true;
				numSamples = 0;
				distance = 0.0;
				samples = out.Near(n->bound, numNearest + 1);

				for (auto& x : samples) {
					if (skip) skip = false;
					else cweeMath::rollingAverageRef(distance, x->bound.Distance(n->bound)(), numSamples);
				}
				distances.AddValue(n->object->data, distance);

				n = out.GetNextLeaf(n);
			}


		}
		return distances;
	};
	cweeUnitPattern  GetApproximateDistances(bool normalized = true, bool xAxisOnly = true) const {
		AUTO distances = cweeUnitPattern(this->X_Type(), cweeUnitValues::foot(1));
		if (this->GetNumValues() > 1) {
			AUTO minTime = this->GetMinTime();
			AUTO maxTime = this->GetMaxTime();
			AUTO timeRange = maxTime - minTime;
			AUTO minValue = this->GetMinValue();
			AUTO maxValue = this->GetMaxValue();
			AUTO valueRange = maxValue - minValue;

			cweeUnitPatternContainer_t::ConstIterType prevValue1;
			prevValue1.Y = nullptr;

			cweeUnitPatternContainer_t::ConstIterType prevValue2;
			prevValue2.Y = nullptr;

			for (auto& iter : const_cast<const cweeUnitPatternContainer_t&>(*container)) {
				if (iter.Y) {
					if (prevValue1.Y && prevValue2.Y) {
						if (xAxisOnly) {
							if (normalized) {
								vec2d coord1(
									((prevValue1.X - minTime) / (timeRange))(),
									0
								);
								vec2d coord2(
									((prevValue2.X - minTime) / (timeRange))(),
									0
								);
								vec2d coord3(
									((iter.X - minTime) / (timeRange))(),
									0
								);
								distances.AddValue(prevValue2.X, cweeMath::Fmax(coord2.Distance(coord1), coord2.Distance(coord3)));
							}
							else {
								vec2d coord1(prevValue1.X.operator()(), 0);
								vec2d coord2(prevValue2.X.operator()(), 0);
								vec2d coord3(iter.X.operator()(), 0);

								distances.AddValue(prevValue2.X, cweeMath::Fmax(coord2.Distance(coord1), coord2.Distance(coord3)));
							}
						}
						else {
							if (normalized) {
								vec2d coord1(
									((prevValue1.X - minTime) / (timeRange))(),
									((*prevValue1.Y - minValue) / (valueRange))()
								);
								vec2d coord2(
									((prevValue2.X - minTime) / (timeRange))(),
									((*prevValue2.Y - minValue) / (valueRange))()
								);
								vec2d coord3(
									((iter.X - minTime) / (timeRange))(),
									((*iter.Y - minValue) / (valueRange))()
								);
								distances.AddValue(prevValue2.X, cweeMath::Fmax(coord2.Distance(coord1), coord2.Distance(coord3)));
							}
							else {
								vec2d coord1(prevValue1.X.operator()(), prevValue1.Y->operator()());
								vec2d coord2(prevValue2.X.operator()(), prevValue2.Y->operator()());
								vec2d coord3(iter.X.operator()(), iter.Y->operator()());

								distances.AddValue(prevValue2.X, cweeMath::Fmax(coord2.Distance(coord1), coord2.Distance(coord3)));
							}
						}


					}
					prevValue1 = prevValue2;
					prevValue2 = iter;
				}
			}
		}
		return distances;
	};
	cweeUnitPattern  LineOfBestFit() const {
		AUTO out{ cweeUnitPattern(this->X_Type(), this->Y_Type()) };

		cweeList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> knots;
		for (auto& x : GetKnotSeries()) {
			knots.Append({ x.first, x.second });
		}
		AUTO Intersept_Slope = cweeEng::LineOfBestFit(knots);

		out.AddUniqueValue(GetMinTime(), Intersept_Slope.first + GetMinTime() * Intersept_Slope.second);
		for (auto& knot : GetTimeSeries(GetMinTime(), GetMaxTime(), (GetMaxTime() - GetMinTime()) / 4)) {
			out.AddUniqueValue(knot.first, Intersept_Slope.first + knot.first * Intersept_Slope.second);
		}
		out.AddUniqueValue(GetMaxTime(), Intersept_Slope.first + GetMaxTime() * Intersept_Slope.second);


		return out;
	};
	cweeUnitPattern  LineOfBestFit(cweeUnitPattern const& mask) const {
		AUTO out{ cweeUnitPattern(this->X_Type(), this->Y_Type()) };

		cweeList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> knots;
		for (auto& x : GetKnotSeries()) {
			if (mask.GetCurrentValue(x.first) > 0) continue;
			knots.Append({x.first, x.second});
		}
		AUTO Intersept_Slope = cweeEng::LineOfBestFit(knots);

		out.AddUniqueValue(GetMinTime(), Intersept_Slope.first + GetMinTime() * Intersept_Slope.second);
		for (auto& knot : GetTimeSeries(GetMinTime(), GetMaxTime(), (GetMaxTime() - GetMinTime()) / 4)) {
			out.AddUniqueValue(knot.first, Intersept_Slope.first + knot.first * Intersept_Slope.second);
		}
		out.AddUniqueValue(GetMaxTime(), Intersept_Slope.first + GetMaxTime() * Intersept_Slope.second);

		return out;
	};

#if 1
	friend cweeUnitPattern operator+(const cweeUnitPattern& a, const cweeUnitPattern& b) {
		cweeUnitPattern result(a.GetMinTime(), a.GetCurrentValue(0)); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (true) {
			if (true) {
				a.lock.Lock();
				AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).end();
				for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).begin(); iter != endIter; ++iter) {
					if (iter->Y) {
						// result.AddUniqueValue((a.container->internal_X_type = iter->X), (a.container->internal_Y_type = *iter->Y) + b.GetCurrentValue((a.container->internal_X_type = iter->X)));
						result.AddValue(iter->X, *iter->Y + b.GetCurrentValue(iter->X));
					}
				}
				a.lock.Unlock();
			}
			if (true) {
				b.lock.Lock();
				AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*b.container).end();
				for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*b.container).begin(); iter != endIter; ++iter) {
					if (iter->Y) {
						// result.AddUniqueValue((b.container->internal_X_type = iter->X), (b.container->internal_Y_type = *iter->Y) + a.GetCurrentValue((b.container->internal_X_type = iter->X)));
						result.AddValue(iter->X, *iter->Y + a.GetCurrentValue(iter->X));
					}
				}
				b.lock.Unlock();
			}
		}
		return result;
	};
	friend cweeUnitPattern operator-(const cweeUnitPattern& a, const cweeUnitPattern& b) {
		cweeUnitPattern result(a.GetMinTime(), a.GetCurrentValue(0)); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (true) {
			AUTO knotsA = a.GetKnotSeries();
			a.lock.Lock();
			for (auto& x : knotsA) {
				result.AddUniqueValue((a.container->internal_X_type = x.first), (a.container->internal_Y_type = x.second) - b.GetCurrentValue((a.container->internal_X_type = x.first)));
			}
			a.lock.Unlock();
			AUTO knotsB = b.GetKnotSeries();
			b.lock.Lock();
			for (auto& x : knotsB) {
				result.AddUniqueValue((b.container->internal_X_type = x.first), a.GetCurrentValue((b.container->internal_X_type = x.first)) - (b.container->internal_Y_type = x.second));
			}
			b.lock.Unlock();
		}
		return result;
	};
	friend cweeUnitPattern operator*(const cweeUnitPattern& a, const cweeUnitPattern& b) {
		cweeUnitPattern result(a.X_Type(), a.Y_Type() * b.Y_Type()); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (true) {
			AUTO knotsA = a.GetKnotSeries();
			a.lock.Lock();
			for (auto& x : knotsA) {
				result.AddUniqueValue(x.first, x.second * b.GetCurrentValue(x.first));
			}
			a.lock.Unlock();
			AUTO knotsB = b.GetKnotSeries();
			b.lock.Lock();
			for (auto& x : knotsB) {
				result.AddUniqueValue(x.first, x.second * a.GetCurrentValue(x.first));
			}
			b.lock.Unlock();
		}
		return result;
	};
	friend cweeUnitPattern operator/(const cweeUnitPattern& a, const cweeUnitPattern& b) {
		cweeUnitPattern result(a.X_Type(), a.Y_Type() / b.GetCurrentValue(0)); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (true) {
			AUTO knotsA = a.GetKnotSeries();
			a.lock.Lock();
			for (auto& x : knotsA) {
				result.AddUniqueValue((a.container->internal_X_type = x.first), (a.container->internal_Y_type = x.second) / b.GetCurrentValue((a.container->internal_X_type = x.first)));
			}
			a.lock.Unlock();
			AUTO knotsB = b.GetKnotSeries();
			b.lock.Lock();
			for (auto& x : knotsB) {
				result.AddUniqueValue((b.container->internal_X_type = x.first), a.GetCurrentValue((b.container->internal_X_type = x.first)) / (b.container->internal_Y_type = x.second));
			}
			b.lock.Unlock();
		}
		return result;
	};

	friend cweeUnitPattern operator+(const cweeUnitPattern& a, const cweeUnitValues::unit_value& b) {
		cweeUnitPattern result(a.X_Type(), a.Y_Type() + b); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (a.GetNumValues() > 0) {
			a.lock.Lock();
			AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).end();
			for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).begin(); iter != endIter; ++iter) {
				if (iter->Y) {
					result.AddValue(iter->X, *iter->Y + b);
				}
			}
			a.lock.Unlock();
		}
		else {
			result.AddValue(0, b);
		}
		return result;
	};
	friend cweeUnitPattern operator-(const cweeUnitPattern& a, const cweeUnitValues::unit_value& b) {
		cweeUnitPattern result(a.GetMinTime(), a.GetCurrentValue(0) - b); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (a.GetNumValues() > 0) {
			a.lock.Lock();
			AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).end();
			for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).begin(); iter != endIter; ++iter) {
				if (iter->Y) {
					result.AddValue(iter->X, *iter->Y - b);
				}
			}
			a.lock.Unlock();
		}
		else {
			result.AddValue(0, -1 * b);
		}
		return result;
	};
	friend cweeUnitPattern operator*(const cweeUnitPattern& a, const cweeUnitValues::unit_value& b) {
		//if (b.is_scalar(b)) { // multiplying by a scalar won't change the units. 
		//	return cweeUnitPattern(a).ScaleY(b());
		//}
		//else 
		{ // units may change. 
			cweeUnitPattern result(a.X_Type(), a.Y_Type() * b); {
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (a.GetNumValues() > 0) {
				a.lock.Lock();
				for (auto& kn : const_cast<const cweeUnitPatternContainer_t&>(*a.container)) {
					if (kn.Y) {
						result.AddValue(kn.X, *kn.Y * b);
					}
				}
				a.lock.Unlock();
			}
			return result;
		}
	};
	friend cweeUnitPattern operator/(const cweeUnitPattern& a, const cweeUnitValues::unit_value& b) {
		//if (b.is_scalar(b)) { // multiplying by a scalar won't change the units. 
		//	return cweeUnitPattern(a).ScaleY(1.0 / b());
		//}
		//else 
		{ // units may change. 
			cweeUnitPattern result(a.X_Type(), a.Y_Type() / b); {
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (a.GetNumValues() > 0) {
				a.lock.Lock();
				for (auto& kn : const_cast<const cweeUnitPatternContainer_t&>(*a.container)) {
					if (kn.Y) {
						result.AddValue(kn.X, *kn.Y / b);
					}
				}
				a.lock.Unlock();
			}
			return result;
		}
	};

	friend cweeUnitPattern operator+(const cweeUnitValues::unit_value& b, const cweeUnitPattern& a) {
		cweeUnitPattern result(a.X_Type(), a.Y_Type() + b); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (a.GetNumValues() > 0) {
			a.lock.Lock();
			AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).end();
			for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).begin(); iter != endIter; ++iter) {
				if (iter->Y) {
					result.AddValue(iter->X, b + *iter->Y);
				}
			}
			a.lock.Unlock();
		}
		else {
			result.AddValue(0, b);
		}
		return result;
	};
	friend cweeUnitPattern operator-(const cweeUnitValues::unit_value& b, const cweeUnitPattern& a) {
		cweeUnitPattern result(a.X_Type(), b - a.Y_Type()); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (a.GetNumValues() > 0) {
			a.lock.Lock();
			AUTO endIter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).end();
			for (auto iter = const_cast<const cweeUnitPatternContainer_t&>(*a.container).begin(); iter != endIter; ++iter) {
				if (iter->Y) {
					result.AddValue(iter->X, b - *iter->Y);
				}
			}
			a.lock.Unlock();
		}
		else {
			result.AddValue(0, -1 * b);
		}
		return result;
	};
	friend cweeUnitPattern operator*(const cweeUnitValues::unit_value& b, const cweeUnitPattern& a) {
		//if (b.is_scalar(b)) { // multiplying by a scalar won't change the units. 
		//	return cweeUnitPattern(a).ScaleY(b());
		//}
		//else 
		{ // units may change. 
			cweeUnitPattern result(a.X_Type(), a.Y_Type() * b); {
				result.SetBoundaryType(a.GetBoundaryType());
				result.SetInterpolationType(a.GetInterpolationType());
			}
			if (a.GetNumValues() > 0) {
				a.lock.Lock();
				for (auto& kn : const_cast<const cweeUnitPatternContainer_t&>(*a.container)) {
					if (kn.Y) {
						result.AddValue(kn.X, *kn.Y * b);
					}
				}
				a.lock.Unlock();
			}
			return result;
		}
	};
	friend cweeUnitPattern operator/(const cweeUnitValues::unit_value& b, const cweeUnitPattern& a) {
		cweeUnitPattern result(a.X_Type(), b / a.Y_Type()); {
			result.SetBoundaryType(a.GetBoundaryType());
			result.SetInterpolationType(a.GetInterpolationType());
		}
		if (a.GetNumValues() > 0) {
			a.lock.Lock();
			for (auto& kn : const_cast<const cweeUnitPatternContainer_t&>(*a.container)) {
				if (kn.Y) {
					result.AddValue(kn.X, b / *kn.Y);
				}
			}
			a.lock.Unlock();
		}
		return result;
	};

	cweeUnitPattern& operator+=(const cweeUnitPattern& b) {
		cweeUnitPattern pat = *this;
		cweeUnitValues::unit_value val = this->Y_Type();
		{
			for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*pat.container)) {
				if (x.Y) {
					this->AddUniqueValue(x.X, val = *x.Y + b.GetCurrentValue(x.X));
				}
			}
		}
		{
			for (auto& x : const_cast<const cweeUnitPatternContainer_t&>(*b.container)) {
				if (x.Y) {
					this->AddUniqueValue(x.X, val = pat.GetCurrentValue(x.X) + *x.Y);
				}
			}
		}
		RemoveUnnecessaryKnots();
		return *this;

		// *this = (*this + b); return *this; 
	};
	cweeUnitPattern& operator-=(const cweeUnitPattern& b) { *this = (*this - b); return *this; };
	cweeUnitPattern& operator*=(const cweeUnitPattern& b) { *this = (*this * b); return *this; };
	cweeUnitPattern& operator/=(const cweeUnitPattern& b) { *this = (*this / b); return *this; };

	cweeUnitPattern& operator+=(const cweeUnitValues::unit_value& b) { *this = (*this + b); return *this; };
	cweeUnitPattern& operator-=(const cweeUnitValues::unit_value& b) { *this = (*this - b); return *this; };
	cweeUnitPattern& operator*=(const cweeUnitValues::unit_value& b) { *this = (*this * b); return *this; };
	cweeUnitPattern& operator/=(const cweeUnitValues::unit_value& b) { *this = (*this / b); return *this; };

	cweeUnitPattern pow(const cweeUnitPattern& b)const {
		auto x = cweeUnitPattern(GetMinTime(), GetCurrentValue(0).pow(b.GetCurrentValue(0)));
		AUTO knotSeries = GetKnotSeries();
		lock.Lock();
		for (auto& j : knotSeries) {
			auto newTypeAndValue = (container->internal_Y_type = j.second).pow(b.GetCurrentValue(container->internal_X_type = j.first));
			x.AddValue((container->internal_X_type = j.first), newTypeAndValue);
		}
		lock.Unlock();
		return x;
	};
	cweeUnitPattern pow(const cweeUnitValues::unit_value& b)const {
		auto x = cweeUnitPattern(GetMinTime(), GetCurrentValue(0).pow(b));
		AUTO knotSeries = GetKnotSeries();
		lock.Lock();
		for (auto& j : knotSeries) {
			auto newTypeAndValue = (container->internal_Y_type = j.second).pow(b);
			x.AddValue((container->internal_X_type = j.first), newTypeAndValue);
		}
		lock.Unlock();
		return x;
	};
#endif

};

INLINE void cweeBalancedPatternRefTest() {
	cweeBalancedPattern<units::length::meter_t, u64> t1;
	cweeUnitPattern pat1 = cweeUnitPattern(t1);

	pat1.GetCurrentFirstDerivative(0);

	cweeBalancedPattern<units::length::meter_t, units::time::second_t> t2;
	cweeUnitPattern pat2 = cweeUnitPattern(t2);

	pat2.GetCurrentFirstDerivative(0);


};

//};