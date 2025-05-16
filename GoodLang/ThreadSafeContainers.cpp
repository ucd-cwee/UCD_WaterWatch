#pragma once
#include "Any.h"
#include "ThreadSafeContainers.h"
#include <ShlDisp.h> // InterlockedExchangePointer
#include <iostream>

namespace GoodLang {	
	// InterlockedLong
	long InterlockedLong::Increment() { return InterlockedIncrementAcquire(&value); } // atomically increments the integer and returns the new value
	long InterlockedLong::Decrement() { return InterlockedDecrementRelease(&value); } // atomically decrements the integer and returns the new value
	long InterlockedLong::Add(long v) { return InterlockedExchangeAdd(&value, v) + v; } // atomically adds a value to the integer and returns the new value
	long InterlockedLong::Sub(long v) { return InterlockedExchangeAdd(&value, -v) - v; } // atomically subtracts a value from the integer and returns the new value
	long InterlockedLong::GetValue() const { return value; } // returns the current value of the integer
	void InterlockedLong::SetValue(long v) { InterlockedExchange(&value, v); };
	bool InterlockedLong::SetValueIfEqual(long desired, long compare) { return InterlockedCompareExchange(&value, desired, compare) == compare; };
	bool InterlockedLong::TryIncrementTo(long n) {
		if (Increment() == n) {
			return true;
		}
		Decrement();
		return false;
	};
	void InterlockedLong::lock() {
		while (!TryIncrementTo(1)) {};
	};
	void InterlockedLong::unlock() {
		Decrement();
	};
	long InterlockedLong::load() const { return GetValue(); };
	bool InterlockedLong::CompareExchange(long oldVersion, long RecordVersion) {
		return SetValueIfEqual(RecordVersion, oldVersion);
	};



#if 0
	// DoubleWrapper
	DoubleWrapper::Arg DoubleWrapper::abs(Arg val) {
		return val >= (Arg)0 ? val : -val;
	}
	DoubleWrapper::Arg DoubleWrapper::floor(Arg val) {
		// casting to int truncates the value, which is floor(val) for positive values,
		// but we have to substract 1 for negative values (unless val is already floored == recasted int val)
		const auto val_int = (int64_t)val;
		const Arg fval_int = (Arg)val_int;
		return (val >= (Arg)0 ? fval_int : (val == fval_int ? val : fval_int - (Arg)1));
	};
	uint64_t DoubleWrapper::pack_fast(double value) {
		if ((value + 0.0) == 0) return 0;
		struct tempContainer { short value : 10; };
		uint64_t toReturn = ((*(uint64_t*)(void*)(double*)(&value)) << (64 - (DBL_MANT_DIG - 1))) >> (64 - (DBL_MANT_DIG - 1));
		uint64_t exponent_literal{ *(uint64_t*)(void*)(double*)(&value) >> (DBL_MANT_DIG - 1) };
		tempContainer exponent_signed{ static_cast<short>(static_cast<long long>(exponent_literal) - 1023ll) };
		exponent_signed.value += 50;
		return (toReturn | ((*(uint64_t*)(void*)(tempContainer*)(&exponent_signed)) << (DBL_MANT_DIG - 1))) | (((*(uint64_t*)(void*)(double*)(&value)) >> 63) << 62);
	};
	double DoubleWrapper::unpack_fast(uint64_t value) {
		if ((value + 0.0) == 0) return 0;
		uint64_t toReturn{ (value << (64 - (DBL_MANT_DIG - 1))) >> (64 - (DBL_MANT_DIG - 1)) };
		uint64_t exponent_signed{ ((*(uint64_t*)(void*)&value) << 2) >> (DBL_MANT_DIG + 1) };
		uint64_t exponent_literal{ static_cast<uint64_t>(static_cast<long long>(exponent_signed) - 50ll + 1023ll) };
		toReturn |= ((exponent_literal << (DBL_MANT_DIG - 1)) | ((((*(uint64_t*)(void*)&value) >> 62) << 63)));
		return *(double*)(void*)&toReturn;
	};
	double DoubleWrapper::unpack_impl(uint64_t source) {
		return unpack_fast(source);
	};
	uint64_t DoubleWrapper::pack_impl(double value) {
		return pack_fast(std::move(value));
	};
	void DoubleWrapper::pack(double a) {
		representation = pack_impl(std::move(a));
	};
	DoubleWrapper::Arg DoubleWrapper::unpack() const {
		return unpack_impl(representation);
	};
	DoubleWrapper::operator Arg() { return load(); };
	DoubleWrapper::operator const Arg() const { return load(); };
	DoubleWrapper DoubleWrapper::operator+(DoubleWrapper& b) {
		return DoubleWrapper{ load() + b.load() };
	};
	DoubleWrapper DoubleWrapper::operator-(DoubleWrapper& b) {
		return DoubleWrapper{ load() - b.load() };
	};
	DoubleWrapper DoubleWrapper::operator/(DoubleWrapper& b) {
		return DoubleWrapper{ load() / b.load() };
	};
	DoubleWrapper DoubleWrapper::operator*(DoubleWrapper& b) {
		return DoubleWrapper{ load() * b.load() };
	};
	DoubleWrapper& DoubleWrapper::operator--() {
		Add(-1);
		return *this;
	};
	DoubleWrapper& DoubleWrapper::operator++() {
		Add(1);
		return *this;
	};
	DoubleWrapper DoubleWrapper::operator--(int) { return operator--() + 1; };
	DoubleWrapper DoubleWrapper::operator++(int) { return operator++() - 1; };
	DoubleWrapper& DoubleWrapper::operator+=(const DoubleWrapper& i) {
		pack(this->load() + i.load());
		return *this;
	};
	DoubleWrapper& DoubleWrapper::operator-=(const DoubleWrapper& i) {
		pack(this->load() - i.load());
		return *this;
	};
	DoubleWrapper& DoubleWrapper::operator/=(const DoubleWrapper& i) {
		pack(this->load() / i.load());
		return *this;
	};
	DoubleWrapper& DoubleWrapper::operator*=(const DoubleWrapper& i) {
		pack(this->load() * i.load());
		return *this;
	};
	bool DoubleWrapper::operator<=(DoubleWrapper& b) { auto x{ load() }; auto y{ b.load() }; return x <= y; };
	bool DoubleWrapper::operator>=(DoubleWrapper& b) { auto x{ load() }; auto y{ b.load() }; return x >= y; };
	bool DoubleWrapper::operator<(DoubleWrapper& b) { return !operator>=(b); };
	bool DoubleWrapper::operator>(DoubleWrapper& b) { return !operator<=(b); };
	DoubleWrapper DoubleWrapper::Pow(DoubleWrapper const& V) const {
		return DoubleWrapper{ std::pow(load(), V.load()) };
	};
	DoubleWrapper DoubleWrapper::Sqrt() const {
		return DoubleWrapper{ std::sqrt(load()) };
	};
	DoubleWrapper DoubleWrapper::Abs() const {
		return DoubleWrapper{ abs(load()) };
	};
	DoubleWrapper DoubleWrapper::Floor() const {
		return DoubleWrapper{ floor(load()) };
	};
	DoubleWrapper DoubleWrapper::Ceiling() const {
		return DoubleWrapper{ floor(load() + static_cast<Arg>(1)) };
	};
	DoubleWrapper::Arg DoubleWrapper::Swap(Arg const& input) {
		auto out{ load() };
		pack(input);
		return out;
	}; // returns the previous value while changing the underlying value
	DoubleWrapper::Arg DoubleWrapper::Add(Arg const& input) {
		auto out{ load() };
		pack(input + out);
		return out;
	}; // returns the previous value while incrementing the actual counter
	DoubleWrapper::Arg DoubleWrapper::fetch_add(Arg const& v) {
		return Add(v);
	}; // returns the previous value while incrementing the actual counter
	DoubleWrapper::Arg DoubleWrapper::fetch_sub(Arg const& v) {
		return Add(-v);
	}; // returns the previous value while decrementing the actual counter
	DoubleWrapper::Arg DoubleWrapper::exchange(Arg const& v) {
		return Swap(v);
	}; // returns the previous value while setting the value to the input
	DoubleWrapper::Arg DoubleWrapper::load() const {
		return unpack();
	}; // gets the value
	void DoubleWrapper::store(Arg const& v) {
		Swap(v);
		return;
	}; // sets the value to the input

	// FloatWrapper
	FloatWrapper::Arg FloatWrapper::abs(Arg val) {
		return val >= (Arg)0 ? val : -val;
	};
	FloatWrapper::Arg FloatWrapper::floor(Arg val) {
		// casting to int truncates the value, which is floor(val) for positive values,
		// but we have to substract 1 for negative values (unless val is already floored == recasted int val)
		const auto val_int = (int64_t)val;
		const Arg fval_int = (Arg)val_int;
		return (val >= (Arg)0 ? fval_int : (val == fval_int ? val : fval_int - (Arg)1));
	};
	uint32_t FloatWrapper::pack_fast(float value) {
		if (value == 0) return 0;
		struct tempContainer { short value : 7; };
		uint32_t toReturn = (*(uint32_t*)(void*)&value << (32 - (FLT_MANT_DIG - 1))) >> (32 - (FLT_MANT_DIG - 1));
		uint32_t exponent_literal{ *(uint32_t*)(void*)&value >> (FLT_MANT_DIG - 1) };
		tempContainer exponent_signed{ static_cast<short>(static_cast<long long>(exponent_literal) - 128ll) };
		exponent_signed.value += 50;
		return toReturn | (*(uint32_t*)(void*)&exponent_signed << (FLT_MANT_DIG - 1)) | (((*(uint32_t*)(void*)&value >> (32 - 1)) << (32 - 2)));
	};
	float FloatWrapper::unpack_fast(uint32_t value) {
		if (value == 0) return 0;
		uint32_t toReturn{ (value << (32 - (FLT_MANT_DIG - 1))) >> (32 - (FLT_MANT_DIG - 1)) };
		uint32_t exponent_signed{ (*(uint32_t*)(void*)&value << 2) >> (FLT_MANT_DIG + 1) };
		uint32_t exponent_literal{ static_cast<uint32_t>(static_cast<long long>(exponent_signed) - 50ll + 128ll) };
		toReturn |= (exponent_literal << (FLT_MANT_DIG - 1)) | ((*(uint32_t*)(void*)&value >> (32 - 2)) << (32 - 1));
		return *(float*)(void*)&toReturn;
	};
	float FloatWrapper::unpack_impl(uint32_t source) {
		return unpack_fast(source);
	};
	uint32_t FloatWrapper::pack_impl(float value) {
		return pack_fast(value);
	};
	void FloatWrapper::pack(float a) {
		representation = pack_impl(a);
	};
	FloatWrapper::Arg FloatWrapper::unpack() const {
		return unpack_impl(representation);
	};
	FloatWrapper::operator Arg() { return load(); };
	FloatWrapper::operator const Arg() const { return load(); };
	FloatWrapper FloatWrapper::operator+(FloatWrapper& b) {
		return FloatWrapper{ load() + b.load() };
	}
	FloatWrapper FloatWrapper::operator-(FloatWrapper& b) {
		return FloatWrapper{ load() - b.load() };
	}
	FloatWrapper FloatWrapper::operator/(FloatWrapper& b) {
		return FloatWrapper{ load() / b.load() };
	}
	FloatWrapper FloatWrapper::operator*(FloatWrapper& b) {
		return FloatWrapper{ load() * b.load() };
	}
	FloatWrapper& FloatWrapper::operator--() {
		Add(-1);
		return *this;
	};
	FloatWrapper& FloatWrapper::operator++() {
		Add(1);
		return *this;
	};
	FloatWrapper FloatWrapper::operator--(int) { return operator--() + 1; };
	FloatWrapper FloatWrapper::operator++(int) { return operator++() - 1; };
	FloatWrapper& FloatWrapper::operator+=(const FloatWrapper& i) {
		Update([&i](Arg x)->Arg { return x + i.load(); });
		return *this;
	};
	FloatWrapper& FloatWrapper::operator-=(const FloatWrapper& i) {
		Update([&i](Arg x)->Arg { return x - i.load(); });
		return *this;
	};
	FloatWrapper& FloatWrapper::operator/=(const FloatWrapper& i) {
		Update([&i](Arg x)->Arg { return x / i.load(); });
		return *this;
	};
	FloatWrapper& FloatWrapper::operator*=(const FloatWrapper& i) {
		Update([&i](Arg x)->Arg { return x * i.load(); });
		return *this;
	};
	bool FloatWrapper::operator<=(FloatWrapper& b) { auto x{ load() }; auto y{ b.load() }; return x <= y; };
	bool FloatWrapper::operator>=(FloatWrapper& b) { auto x{ load() }; auto y{ b.load() }; return x >= y; };
	bool FloatWrapper::operator<(FloatWrapper& b) { return !operator>=(b); };
	bool FloatWrapper::operator>(FloatWrapper& b) { return !operator<=(b); };
	FloatWrapper FloatWrapper::Pow(FloatWrapper const& V) const {
		return FloatWrapper{ std::pow(load(), V.load()) };
	};
	FloatWrapper FloatWrapper::Sqrt() const {
		return FloatWrapper{ std::sqrt(load()) };
	};
	FloatWrapper FloatWrapper::Abs() const {
		return FloatWrapper{ abs(load()) };
	};
	FloatWrapper FloatWrapper::Floor() const {
		return FloatWrapper{ floor(load()) };
	};
	FloatWrapper FloatWrapper::Ceiling() const {
		return FloatWrapper{ floor(load() + static_cast<Arg>(1)) };
	};
	FloatWrapper::Arg FloatWrapper::Swap(Arg const& input) {
		auto out{ load() };
		pack(input);
		return out;
	}; // returns the previous value while changing the underlying value
	FloatWrapper::Arg FloatWrapper::Add(Arg const& input) {
		auto out{ load() };
		pack(input + out);
		return out;
	}; // returns the previous value while incrementing the actual counter
	FloatWrapper::Arg FloatWrapper::fetch_add(Arg const& v) {
		return Add(v);
	}; // returns the previous value while incrementing the actual counter
	FloatWrapper::Arg FloatWrapper::fetch_sub(Arg const& v) {
		return Add(-v);
	}; // returns the previous value while decrementing the actual counter
	FloatWrapper::Arg FloatWrapper::exchange(Arg const& v) {
		return Swap(v);
	}; // returns the previous value while setting the value to the input
	FloatWrapper::Arg FloatWrapper::load() const {
		return unpack();
	}; // gets the value
	void FloatWrapper::store(Arg const& v) {
		Swap(v);
		return;
	}; // sets the value to the input

	// LongLongWrapper
	LongLongWrapper::Arg LongLongWrapper::abs(Arg val) {
		return val >= (Arg)0 ? val : -val;
	}
	LongLongWrapper::Arg LongLongWrapper::floor(Arg val) {
		// casting to int truncates the value, which is floor(val) for positive values,
		// but we have to substract 1 for negative values (unless val is already floored == recasted int val)
		const auto val_int = (int64_t)val;
		const Arg fval_int = (Arg)val_int;
		return (val >= (Arg)0 ? fval_int : (val == fval_int ? val : fval_int - (Arg)1));
	};
	uint64_t LongLongWrapper::pack_fast(Arg value) {
		return static_cast<uint64_t>(value + (std::numeric_limits<Arg>::max() / 100000000ll));
	};
	LongLongWrapper::Arg LongLongWrapper::unpack_fast(uint64_t value) {
		return static_cast<Arg>(value) - (std::numeric_limits<Arg>::max() / 100000000ll);
	};
	LongLongWrapper::Arg LongLongWrapper::unpack_impl(uint64_t source) {
		return unpack_fast(source);
	};
	uint64_t LongLongWrapper::pack_impl(Arg value) {
		return pack_fast(value);
	};
	void LongLongWrapper::pack(Arg a) {
		representation = pack_impl(a);
	};
	LongLongWrapper::Arg LongLongWrapper::unpack() const {
		return unpack_impl(representation);
	};
	LongLongWrapper::operator Arg() { return load(); };
	LongLongWrapper::operator const Arg() const { return load(); };
	LongLongWrapper LongLongWrapper::operator+(LongLongWrapper& b) {
		return LongLongWrapper{ load() + b.load() };
	};
	LongLongWrapper LongLongWrapper::operator-(LongLongWrapper& b) {
		return LongLongWrapper{ load() - b.load() };
	};
	LongLongWrapper LongLongWrapper::operator/(LongLongWrapper& b) {
		return LongLongWrapper{ load() / b.load() };
	};
	LongLongWrapper LongLongWrapper::operator*(LongLongWrapper& b) {
		return LongLongWrapper{ load() * b.load() };
	};
	LongLongWrapper& LongLongWrapper::operator--() {
		Add(-1);
		return *this;
	};
	LongLongWrapper& LongLongWrapper::operator++() {
		Add(1);
		return *this;
	};
	LongLongWrapper LongLongWrapper::operator--(int) { return operator--() + 1; };
	LongLongWrapper LongLongWrapper::operator++(int) { return operator++() - 1; };
	LongLongWrapper& LongLongWrapper::operator+=(const LongLongWrapper& i) {
		Update([&i](Arg x)->Arg { return x + i.load(); });
		return *this;
	};
	LongLongWrapper& LongLongWrapper::operator-=(const LongLongWrapper& i) {
		Update([&i](Arg x)->Arg { return x - i.load(); });
		return *this;
	};
	LongLongWrapper& LongLongWrapper::operator/=(const LongLongWrapper& i) {
		Update([&i](Arg x)->Arg { return x / i.load(); });
		return *this;
	};
	LongLongWrapper& LongLongWrapper::operator*=(const LongLongWrapper& i) {
		Update([&i](Arg x)->Arg { return x * i.load(); });
		return *this;
	};
	bool LongLongWrapper::operator<=(LongLongWrapper& b) { auto x{ load() }; auto y{ b.load() }; return x <= y; };
	bool LongLongWrapper::operator>=(LongLongWrapper& b) { auto x{ load() }; auto y{ b.load() }; return x >= y; };
	bool LongLongWrapper::operator<(LongLongWrapper& b) { return !operator>=(b); };
	bool LongLongWrapper::operator>(LongLongWrapper& b) { return !operator<=(b); };
	LongLongWrapper LongLongWrapper::Pow(LongLongWrapper const& V) const {
		return LongLongWrapper{ std::pow(load(), V.load()) };
	};
	LongLongWrapper LongLongWrapper::Sqrt() const {
		return LongLongWrapper{ std::sqrt(load()) };
	};
	LongLongWrapper LongLongWrapper::Abs() const {
		return LongLongWrapper{ abs(load()) };
	};
	LongLongWrapper LongLongWrapper::Floor() const {
		return LongLongWrapper{ floor(load()) };
	};
	LongLongWrapper LongLongWrapper::Ceiling() const {
		return LongLongWrapper{ floor(load() + static_cast<Arg>(1)) };
	};
	LongLongWrapper::Arg LongLongWrapper::Swap(Arg const& input) {
		auto out{ load() };
		pack(input);
		return out;
	}; // returns the previous value while changing the underlying value
	LongLongWrapper::Arg LongLongWrapper::Add(Arg const& input) {
		auto out{ load() };
		pack(input + out);
		return out;
	}; // returns the previous value while incrementing the actual counter
	LongLongWrapper::Arg LongLongWrapper::fetch_add(Arg const& v) {
		return Add(v);
	}; // returns the previous value while incrementing the actual counter
	LongLongWrapper::Arg LongLongWrapper::fetch_sub(Arg const& v) {
		return Add(-v);
	}; // returns the previous value while decrementing the actual counter
	LongLongWrapper::Arg LongLongWrapper::exchange(Arg const& v) {
		return Swap(v);
	}; // returns the previous value while setting the value to the input
	LongLongWrapper::Arg LongLongWrapper::load() const {
		return unpack();
	}; // gets the value
	void LongLongWrapper::store(Arg const& v) {
		Swap(v);
		return;
	}; // sets the value to the input
#endif

};