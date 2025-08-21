#pragma once

#include <ShlDisp.h>
#include <winnt.h>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <math.h>
#include <complex>

// Atomic Numbers
namespace GL {
    // atomic variant of double. Keeps the final bit equal to 0 to support tags. 100x slower than using a normal double, so do not use this unless necessary. 
    class atomic_double {
    protected:
        static  double _abs(double val) {
            return val >= (double)0 ? val : -val;
        }
        static  double _floor(double val) {
            // casting to int truncates the value, which is floor(val) for positive values,
            // but we have to substract 1 for negative values (unless val is already floored == recasted int val)
            const auto val_int = (int64_t)val;
            const double fval_int = (double)val_int;
            return (val >= (double)0 ? fval_int : (val == fval_int ? val : fval_int - (double)1));
        };

        // floating_point to integral conversion: approximation full of magic numbers. 
        // assumes the structure of the double is MANTISSA, EXPONENT, SIGN. 
        // and assumes that the exponent can be reduced by one bit, the sign can be moved over, and the final bit can be cleared, reserved for CAS swaps.
        static  uint64_t pack_fast(double value) {
            if (value == 0) return 0;
            struct tempContainer { short value : 10; };
            uint64_t toReturn = (*(uint64_t*)(void*)&value << (64 - (DBL_MANT_DIG - 1))) >> (64 - (DBL_MANT_DIG - 1));
            uint64_t exponent_literal{ *(uint64_t*)(void*)&value >> (DBL_MANT_DIG - 1) };
            auto exponent_signed{ tempContainer{ static_cast<short>(static_cast<long long>(exponent_literal) - 1023ll) } };
            exponent_signed.value += 50;
            return (toReturn | ((*(uint64_t*)(void*)&exponent_signed) << (DBL_MANT_DIG - 1))) | (((*(uint64_t*)(void*)&value) >> 63) << 62);
        };
        static  double unpack_fast(uint64_t value) {
            if (value == 0) return 0;
            uint64_t toReturn{ (value << (64 - (DBL_MANT_DIG - 1))) >> (64 - (DBL_MANT_DIG - 1)) };
            uint64_t exponent_signed{ ((*(uint64_t*)(void*)&value) << 2) >> (DBL_MANT_DIG + 1) };
            uint64_t exponent_literal{ static_cast<uint64_t>(static_cast<long long>(exponent_signed) - 50ll + 1023ll) };
            toReturn |= ((exponent_literal << (DBL_MANT_DIG - 1)) | ((((*(uint64_t*)(void*)&value) >> 62) << 63)));
            return *(double*)(void*)&toReturn;
        };

    protected:
        uint64_t representation;

    protected:
        void pack(double a) {
            InterlockedExchange(static_cast<volatile unsigned long long*>(&representation), pack_fast(a));
        };
        double unpack() const {
            return unpack_fast(representation);
        };

    public:
        atomic_double() noexcept : representation{ pack_fast(0) } {};
        template <typename T, typename = std::enable_if_t<(!std::is_same<std::decay_t<T>, atomic_double>::value) && std::is_pod_v<std::decay_t<T>>>>  atomic_double(T const& a) : representation{ pack_fast(static_cast<double>(a)) } {};
        atomic_double(const atomic_double& a) = default;
        atomic_double& operator=(const atomic_double& a) noexcept {
            InterlockedExchange(static_cast<volatile unsigned long long*>(&representation), a.representation);
            return *this;
        };
        atomic_double(atomic_double&& a) = default;
        atomic_double& operator=(atomic_double&& a) noexcept {
            InterlockedExchange(static_cast<volatile unsigned long long*>(&representation), a.representation);
            return *this;
        };
        ~atomic_double() = default;

    public:
        operator double() const { return load(); };
        atomic_double operator+(atomic_double& b) {
            return atomic_double{ load() + b.load() };
        };
        atomic_double operator-(atomic_double& b) {
            return atomic_double{ load() - b.load() };
        };
        atomic_double operator/(atomic_double& b) {
            return atomic_double{ load() / b.load() };
        };
        atomic_double operator*(atomic_double& b) {
            return atomic_double{ load() * b.load() };
        };

        atomic_double& operator++() {
            (void)update([](double const& x) -> double { return x + 1; });
            return *this;
        };
        atomic_double& operator--() {
            (void)update([](double const& x) -> double { return x - 1; });
            return *this;
        };
        atomic_double operator++(int) { return operator++() - 1; };
        atomic_double operator--(int) { return operator--() + 1; };


        atomic_double& operator+=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x + i.load(); });
            return *this;
        };
        atomic_double& operator-=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x - i.load(); });
            return *this;
        };
        atomic_double& operator/=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x / i.load(); });
            return *this;
        };
        atomic_double& operator*=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x * i.load(); });
            return *this;
        };

        bool operator==(atomic_double const& b) const {
            return representation == atomic_double(b).representation;
            // return std::abs(load() - (double)b) <= 0.00005l;
        };
        bool operator!=(atomic_double const& b) const { return !operator==(b); };
        bool operator<=(atomic_double const& b) { return load() <= b.load(); };
        bool operator>=(atomic_double const& b) { return load() >= b.load(); };
        bool operator<(atomic_double const& b) { return !operator>=(b); };
        bool operator>(atomic_double const& b) { return !operator<=(b); };

        atomic_double pow(atomic_double const& V) const {
            return atomic_double{ std::pow(load(), V.load()) };
        };
        atomic_double sqrt() const {
            return atomic_double{ std::sqrt(load()) };
        };
        atomic_double abs() const {
            return atomic_double{ _abs(load()) };
        };
        atomic_double floor() const {
            return atomic_double{ _floor(load()) };
        };
        atomic_double ceil() const {
            return atomic_double{ _floor(load() + static_cast<double>(1)) };
        };

    private:
        template<typename Func>
        uint64_t update(Func const& updateFunction) {
            uint64_t prev;
            while (true) {
                prev = representation;
                if (InterlockedCompareExchange(static_cast<volatile unsigned long long*>(&representation), pack_fast(updateFunction(unpack_fast(prev))), prev) == prev) {
                    break;
                }
            }
            return prev;
        }; // returns the previous value while incrementing the actual counter

        template<bool returns = true>
        auto swap(double const& input) {
            if constexpr (!returns) {
                pack(input);
            }
            else {
                auto out{ load() };
                pack(input);
                return double(out);
            }
        }; // returns the previous value while changing the underlying value

    public: // std::atomic compatability
        double fetch_add(double const& v) {
            return unpack_fast(update([&v](double const& from) -> double {
                return from + v;
                }));
        }; // returns the previous value while incrementing the actual counter
        double fetch_sub(double const& v) {
            return unpack_fast(update([&v](double const& from) -> double {
                return from - v;
                }));
        }; // returns the previous value while decrementing the actual counter
        double exchange(double const& v) {
            return swap<true>(v);
        }; // returns the previous value while setting the value to the input
        double load() const {
            return unpack();
        }; // gets the value
        void store(double const& v) {
            swap<false>(v);
            return;
        }; // sets the value to the input

    };

    // atomic variant of float. Keeps the final bit equal to 0 to support tags. 100x slower than using a normal float, so do not use this unless necessary. 
    class atomic_float {
    protected:
        static  float _abs(float val) {
            return val >= (float)0 ? val : -val;
        }
        static  float _floor(float val) {
            // casting to int truncates the value, which is floor(val) for positive values,
            // but we have to substract 1 for negative values (unless val is already floored == recasted int val)
            const auto val_int = (int32_t)val;
            const float fval_int = (float)val_int;
            return (val >= (float)0 ? fval_int : (val == fval_int ? val : fval_int - (float)1));
        };

        // floating_point to integral conversion: approximation full of magic numbers. 
        // assumes the structure of the float is MANTISSA, EXPONENT, SIGN. 
        // and assumes that the exponent can be reduced by one bit, the sign can be moved over, and the final bit can be cleared, reserved for CAS swaps.        
        static uint32_t pack_fast(float value) {
            if (value == 0) return 0;
            struct tempContainer { short value : 7; };
            uint32_t toReturn = (*(uint32_t*)(void*)&value << (32 - (FLT_MANT_DIG - 1))) >> (32 - (FLT_MANT_DIG - 1));
            uint32_t exponent_literal{ *(uint32_t*)(void*)&value >> (FLT_MANT_DIG - 1) };
            tempContainer exponent_signed{ static_cast<short>(static_cast<long long>(exponent_literal) - 128ll) };
            exponent_signed.value += 50;
            return toReturn | (*(uint32_t*)(void*)&exponent_signed << (FLT_MANT_DIG - 1)) | (((*(uint32_t*)(void*)&value >> (32 - 1)) << (32 - 2)));
        };
        static float unpack_fast(uint32_t value) {
            if (value == 0) return 0;
            uint32_t toReturn{ (value << (32 - (FLT_MANT_DIG - 1))) >> (32 - (FLT_MANT_DIG - 1)) };
            uint32_t exponent_signed{ (*(uint32_t*)(void*)&value << 2) >> (FLT_MANT_DIG + 1) };
            uint32_t exponent_literal{ static_cast<uint32_t>(static_cast<long long>(exponent_signed) - 50ll + 128ll) };
            toReturn |= (exponent_literal << (FLT_MANT_DIG - 1)) | ((*(uint32_t*)(void*)&value >> (32 - 2)) << (32 - 1));
            return *(float*)(void*)&toReturn;
        };

    protected:
        uint32_t representation;

    protected:
        void pack(float a) {
            InterlockedExchange(static_cast<volatile uint32_t*>(&representation), pack_fast(a));
        };
        float unpack() const {
            return unpack_fast(representation);
        };

    public:
        atomic_float() noexcept : representation{ pack_fast(0) } {};
        template <typename T, typename = std::enable_if_t<(!std::is_same<std::decay_t<T>, atomic_float>::value) && std::is_pod_v<std::decay_t<T>>>>  atomic_float(T const& a) : representation{ pack_fast(static_cast<float>(a)) } {};
        atomic_float(const atomic_float& a) = default;
        atomic_float& operator=(const atomic_float& a) noexcept {
            InterlockedExchange(static_cast<volatile uint32_t*>(&representation), a.representation);
            return *this;
        };
        atomic_float(atomic_float&& a) = default;
        atomic_float& operator=(atomic_float&& a) noexcept {
            InterlockedExchange(static_cast<volatile uint32_t*>(&representation), a.representation);
            return *this;
        };
        ~atomic_float() = default;

    public:
        operator float() const { return load(); };
        atomic_float operator+(atomic_float& b) {
            return atomic_float{ load() + b.load() };
        };
        atomic_float operator-(atomic_float& b) {
            return atomic_float{ load() - b.load() };
        };
        atomic_float operator/(atomic_float& b) {
            return atomic_float{ load() / b.load() };
        };
        atomic_float operator*(atomic_float& b) {
            return atomic_float{ load() * b.load() };
        };

        atomic_float& operator++() {
            (void)update([](float const& x) -> float { return x + 1; });
            return *this;
        };
        atomic_float& operator--() {
            (void)update([](float const& x) -> float { return x - 1; });
            return *this;
        };
        atomic_float operator++(int) { return operator++() - 1; };
        atomic_float operator--(int) { return operator--() + 1; };


        atomic_float& operator+=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x + i.load(); });
            return *this;
        };
        atomic_float& operator-=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x - i.load(); });
            return *this;
        };
        atomic_float& operator/=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x / i.load(); });
            return *this;
        };
        atomic_float& operator*=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x * i.load(); });
            return *this;
        };


        bool operator==(atomic_float const& b) const {
            return representation == atomic_float(b).representation;
            // return std::abs(load() - (float)b) <= 0.00005l;
        };
        bool operator!=(atomic_float const& b) const { return !operator==(b); };
        bool operator<=(atomic_float const& b) { return load() <= b.load(); };
        bool operator>=(atomic_float const& b) { return load() >= b.load(); };
        bool operator<(atomic_float const& b) { return !operator>=(b); };
        bool operator>(atomic_float const& b) { return !operator<=(b); };

        atomic_float pow(atomic_float const& V) const {
            return atomic_float{ std::pow(load(), V.load()) };
        };
        atomic_float sqrt() const {
            return atomic_float{ std::sqrt(load()) };
        };
        atomic_float abs() const {
            return atomic_float{ _abs(load()) };
        };
        atomic_float floor() const {
            return atomic_float{ _floor(load()) };
        };
        atomic_float ceil() const {
            return atomic_float{ _floor(load() + static_cast<float>(1)) };
        };

    private:
        template<typename Func>
        uint32_t update(Func const& updateFunction) {
            uint32_t prev;
            while (true) {
                prev = representation;
                if (InterlockedCompareExchange(static_cast<volatile uint32_t*>(&representation), pack_fast(updateFunction(unpack_fast(prev))), prev) == prev) {
                    break;
                }
            }
            return prev;
        }; // returns the previous value while incrementing the actual counter

        template<bool returns = true>
        auto swap(float const& input) {
            if constexpr (!returns) {
                pack(input);
            }
            else {
                auto out{ load() };
                pack(input);
                return float(out);
            }
        }; // returns the previous value while changing the underlying value

    public: // std::atomic compatability
        float fetch_add(float const& v) {
            return unpack_fast(update([&v](float const& from) -> float {
                return from + v;
                }));
        }; // returns the previous value while incrementing the actual counter
        float fetch_sub(float const& v) {
            return unpack_fast(update([&v](float const& from) -> float {
                return from - v;
                }));
        }; // returns the previous value while decrementing the actual counter
        float exchange(float const& v) {
            return swap<true>(v);
        }; // returns the previous value while setting the value to the input
        float load() const {
            return unpack();
        }; // gets the value
        void store(float const& v) {
            swap<false>(v);
            return;
        }; // sets the value to the input

    };

};