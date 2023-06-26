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
#include "InterlockedValues.h"
#include "SharedPtr.h"

template< typename type>
class cweeUnpooledInterlocked {
public:
	template< typename type>
	class ExclusiveObject {
	public:
		constexpr ExclusiveObject(const cweeUnpooledInterlocked<type>& mut) : owner(const_cast<cweeUnpooledInterlocked<type>&>(mut)) { this->owner.Lock(); };
		~ExclusiveObject() { this->owner.Unlock(); };

		ExclusiveObject() = delete;
		ExclusiveObject(const ExclusiveObject& other) = delete; // : owner(other.owner) {  };
		ExclusiveObject(ExclusiveObject&& other) = delete; // : owner(other.owner) {  };
		ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
		ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

		type& operator=(const type& a) { data() = a; return data(); };
		type& operator=(type&& a) { data() = a; return data(); };
		type& operator*() const { return data(); };
		type* operator->() const { return &data(); };

	protected:
		type& data() const { return *owner.UnsafeRead(); };
		cweeUnpooledInterlocked<type>& owner;
	};

public: // construction and destruction
	typedef type		Type;

	cweeUnpooledInterlocked() : data(new type()) {};
	cweeUnpooledInterlocked(const type& other) : data(new type()) { Swap(other); };
	cweeUnpooledInterlocked(type&& other) : data(new type()) { Swap(std::forward<type>(other)); };
	cweeUnpooledInterlocked(const cweeUnpooledInterlocked& other) : data(new type()) { this->Copy(other); };
	cweeUnpooledInterlocked(cweeUnpooledInterlocked&& other) : data(new type()) { this->Copy(std::forward<cweeUnpooledInterlocked>(other)); };
	~cweeUnpooledInterlocked() {};

public: // copy and clear
	cweeUnpooledInterlocked<type>& operator=(const cweeUnpooledInterlocked<type>& other) {
		this->Copy(other);
		return *this;
	};
	cweeUnpooledInterlocked<type>& operator=(cweeUnpooledInterlocked<type>&& other) {
		this->Copy(std::forward<cweeUnpooledInterlocked<type>>(other));
		return *this;
	};
	void Copy(const cweeUnpooledInterlocked<type>& copy) {
		if (&copy == this) return;
		*data = *copy.data;
	};
	void Copy(cweeUnpooledInterlocked<type>&& copy) {
		if (&copy == this) return;
		*data = *copy.data;
	};
	void Clear() {
		data = make_cwee_shared<type>(new type());
	};

public: // read and swap
	type Read() const {
		type out;
		AUTO g = Guard();
		auto ptr = UnsafeRead();
		if (ptr) {
			out = *ptr;
		}
		else {
			out = type();
		}
		return out;
	};
	void Swap(const type& replacement) {
		AUTO g = Guard();
		auto ptr = UnsafeRead();
		if (ptr) {
			*ptr = replacement;
		}
	};
	cweeUnpooledInterlocked<type>& operator=(const type& other) {
		Swap(other);
		return *this;
	};
	operator type() {
		return Read();
	};
	operator type() const {
		return Read();
	};

	cweeSharedPtr<type> operator->() {
		return data;
	};
	cweeSharedPtr<type> operator->() const {
		return data;
	};

public: // lock, unlock, and direct edit
	ExclusiveObject<type> GetExclusive() const {
		return ExclusiveObject<type>(*this);
	};

	NODISCARD AUTO Guard() const {
		return data.Guard();
	};
	void Lock() const {
		data.Lock();
	};
	void Unlock() const {
		data.Unlock();
	};
	type* UnsafeRead() const {
		return data.UnsafeGet();
	};

private:
	mutable cweeSharedPtr<type> data;

};
template <typename type> using cweeInterlocked = cweeUnpooledInterlocked<type>;