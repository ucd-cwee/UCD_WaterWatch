#pragma once

#include "Strings.h"
#include "atomic_maps.h"
#include <concurrent_unordered_map.h>

namespace GL {
  
    class value {
    public:
        union package {
        public:
            struct bitset {
                uint16_t si_unit; // hash for unique SI unit combination. (e.g. meter, cubic meter, meter per second)
                uint16_t impl_unit; // hash for unique ratio or unit implimentation within SI unit. (e.g. foot, meter, inch)
                float val; // as float
            };
            struct bitset2 {
                uint32_t unit_hash; // hash for unique unit. 
                float val; // as float
            };

            uint64_t
                m_n64; // for CAS
            bitset
                m_bits;
            bitset2
                m_bits2;
        };
        class impl_unit {
        public:
            GL::string
                name{ "scaler" };
            double
                ratio{ 0 }; // si unit for length
            uint32_t
                hash{ 0 };
            uint64_t
                default_bits;
        };
        // does not use the 'ratio' values yet. 
        class si_unit {
        public:
            double
                METERS{ 0 }; // si unit for length
            double
                KILOGRAMS{ 0 }; // si unit for mass
            double
                SECONDS{ 0 }; // si unit for time
            double
                AMPERES{ 0 }; // si unit for current
            double
                DOLLARS{ 0 }; // si unit for worth
            uint32_t
                hash{ 0 };
            GL::deferred<GL::atomic_map<uint16_t, impl_unit>>
                implimented_units{};

            static uint16_t calc_si_hash(double meters, double kilograms, double seconds, double amperes, double dollars) {
                size_t out = 0;
                out ^= *(uint64_t*)(void*)(&meters) + 0x9e3779b9 + (out << 6) + (out >> 2);
                out ^= *(uint64_t*)(void*)(&kilograms) + 0x9e3779b9 + (out << 6) + (out >> 2);
                out ^= *(uint64_t*)(void*)(&seconds) + 0x9e3779b9 + (out << 6) + (out >> 2);
                out ^= *(uint64_t*)(void*)(&amperes) + 0x9e3779b9 + (out << 6) + (out >> 2);
                out ^= *(uint64_t*)(void*)(&dollars) + 0x9e3779b9 + (out << 6) + (out >> 2);
                return static_cast<uint16_t>(out % std::numeric_limits<uint16_t>::max());
            };
            static uint16_t calc_impl_hash(/*uint32_t si_hash, */double ratio) {
                return static_cast<uint16_t>((*(uint64_t*)(void*)(&ratio)) % std::numeric_limits<uint16_t>::max());

                //size_t out = si_hash;
                //out ^= *(uint64_t*)(void*)(&ratio) + 0x9e3779b9 + (out << 6) + (out >> 2);
                //return static_cast<uint16_t>(out % std::numeric_limits<uint16_t>::max());
            };

            impl_unit& get_impl_unit(double ratio, GL::string const& name = "") {
                uint16_t impl_hash = si_unit::calc_impl_hash(ratio);
                auto& out = implimented_units->operator[](impl_hash);
                if (out.hash == 0) {
                    package default_bits;
                    default_bits.m_n64 = 0;
                    default_bits.m_bits.si_unit = hash;
                    default_bits.m_bits.impl_unit = impl_hash;
                    default_bits.m_bits.val = 0;

                    if (InterlockedCompareExchange(reinterpret_cast<volatile uint32_t*>(&out.hash), impl_hash, 0) == 0) {
                        out.ratio = ratio;
                        out.name = name;
                        out.default_bits = default_bits.m_n64;
                    }
                }
                return out;
            };
        };

    
        // get the cached si unit for this type (fast, assuming already have the unique hash value)
        static si_unit& get_si_unit(uint16_t hash);
        // get the cached si unit for this type (slow, assumes the unique hash is not known or needs to be initialized)
        static si_unit& get_si_unit(double meters, double kilograms, double seconds, double amperes, double dollars) {
            uint16_t base_hash = si_unit::calc_si_hash(meters, kilograms, seconds, amperes, dollars);
            si_unit& out = get_si_unit(base_hash);
            if (out.hash == 0) {
                if (InterlockedCompareExchange(reinterpret_cast<volatile uint32_t*>(&out.hash), base_hash, 0) == 0) {
                    out.METERS = meters;
                    out.KILOGRAMS = kilograms;
                    out.SECONDS = seconds;
                    out.AMPERES = amperes;
                    out.DOLLARS = dollars;
                }
            }
            return out;
        };
    
    private:
        std::atomic<package> packed;
        template <typename F> package Update(F const& update_func) {
            package Old, New;
            while (true) {
                Old = load();
                New.m_n64 = Old.m_n64;
                New.m_bits.val = update_func(Old.m_bits.val);
                if (compare_exchange(Old, New)) { 
                    return Old;
                }
            }
        };
        template <typename F> package UpdatePackage(F const& update_func) {
            package Old;
            while (true) {
                Old = load();
                if (compare_exchange(Old, update_func(Old))) {
                    return Old;
                }
            }
        };
        static double const& ratio(package const& pkg) {
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).ratio;
        };
        value& add_inline(value const& rhs) {
            auto RHS = rhs.load();
            UpdatePackage([&](package Old) -> package {
                if (Old.m_bits2.unit_hash == RHS.m_bits2.unit_hash) {
                    Old.m_bits.val += RHS.m_bits.val;
                }
                else if (Old.m_bits.si_unit == RHS.m_bits.si_unit) {
                    const double& RHS_ratio = ratio(RHS);
                    const double& LHS_ratio = ratio(Old);
                    Old.m_bits.val += static_cast<float>((static_cast<double>(RHS.m_bits.val) * RHS_ratio) / LHS_ratio);
                }
                else {
                    throw std::runtime_error("Incompatable SI units");
                }
                return Old;
            });
            return *this;
        };


    public:
        value() : packed(package{ 0ull }) {};
        explicit value(package const& from) : packed(from) {};
        explicit value(impl_unit const& from) : packed(package{ from.default_bits }) {};
        value(value const& rhs) : packed(rhs.load()) {};
        value(value && rhs) noexcept : packed(rhs.load()) {};
        value& operator=(value const& rhs){
            store(rhs.load());
            return *this;
        };
        value& operator=(value&& rhs) noexcept {
            store(rhs.load());
            return *this;
        };
        ~value() = default;

        package load() const {
            return packed.load();
        };
        void store(package const& data) {
            packed.store(data);
        };
        void store(package&& data) {
            packed.store(std::move(data));
        };
        package exchange(package&& data) {
            return packed.exchange(std::move(data));
        };
        package exchange(package const& data) {
            return packed.exchange(data);
        };
        bool compare_exchange(package const& expected, package&& newValue) {
            return packed.compare_exchange_strong(const_cast<package&>(expected), std::move(newValue));
        };
        bool compare_exchange(package const& expected, package const& newValue) {
            return packed.compare_exchange_strong(const_cast<package&>(expected), newValue);
        };

        GL::string const& name() const {
            auto pkg = load();
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).name;
        };
        double ratio() const {
            auto pkg = load();
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).ratio;
        };
        value& operator++() {
            Update([](float init) -> float { return init + 1; });
            return *this;
        };
        value& operator--() {
            Update([](float init) -> float { return init - 1; });
            return *this;
        };
        value operator++(int) {
            return value{ Update([](float init) -> float { return init + 1; }) };
        };
        value operator--(int) {
            return value{ Update([](float init) -> float { return init - 1; }) };
        };
        value& operator+=(float rhs) {
            Update([&](float init) -> float { return init + rhs; });
            return *this;
        };
        value& operator-=(float rhs) {
            Update([&](float init) -> float { return init - rhs; });
            return *this;
        };
        value operator-() const {
            auto t = load();
            t.m_bits.val *= -1;
            return value(t);
        };

        value& operator+=(value const& rhs) {
            return add_inline(rhs);
        };
        value& operator-=(value const& rhs) {
            return add_inline(-rhs);
        };

    };


};