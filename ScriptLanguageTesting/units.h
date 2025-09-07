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

            bitset2
                m_bits2;
            bitset
                m_bits;
            uint64_t
                m_n64; // for CAS
        };
        class impl_unit {
        public:
            GL::string
                name{ "scaler" };
            GL::string 
                abbreviation{ "" };
            double
                ratio{ 1 }; // si unit ratio. 1 == si unit. negative or 0 is impossible. 
            uint32_t
                hash{ std::numeric_limits< uint32_t>::max() };
            package
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
                hash{ std::numeric_limits< uint32_t>::max() };
            GL::deferred<GL::atomic_map<uint16_t, impl_unit>>
                implimented_units{};

            static uint16_t calc_si_hash(double meters, double kilograms, double seconds, double amperes, double dollars) {
                if ((meters == 0) && (kilograms == 0) && (seconds == 0) && (amperes == 0) && (dollars == 0)) {
                    return 0;
                }
                else {
                    size_t out = 0;
                    out ^= *(uint64_t*)(void*)(&meters) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&kilograms) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&seconds) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&amperes) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&dollars) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    return static_cast<uint16_t>(out % std::numeric_limits<uint16_t>::max());
                }
            };
            static uint16_t calc_impl_hash(double ratio) {
                if ((ratio == 0) || (ratio == 1)) {
                    return 0;
                }
                else {
                    return static_cast<uint16_t>((*(uint64_t*)(void*)(&ratio)) % std::numeric_limits<uint16_t>::max());
                }
            };

            impl_unit* try_get_nearest_impl_unit(double ratio) {
                uint16_t impl_hash = (((hash == 0) || (ratio == 0)) ? 0 : si_unit::calc_impl_hash(ratio));
                if (auto F = implimented_units->find_less_or_equal(impl_hash); F != implimented_units->end()) {
                    return &F->second;
                }
                else if (auto F = implimented_units->find_larger_or_equal(impl_hash); F != implimented_units->end()) {
                    return &F->second;
                } 
                else {
                    return nullptr;
                }
            };
            impl_unit& get_impl_unit(double ratio, GL::string const& name, GL::string const& abbreviation) {
                uint16_t impl_hash = (((hash == 0) || (ratio == 0)) ? 0 : si_unit::calc_impl_hash(ratio));
                auto& out = implimented_units->operator[](impl_hash);
                if (out.hash == std::numeric_limits< uint32_t>::max()) {
                    package default_bits;
                    default_bits.m_n64 = 0;
                    default_bits.m_bits.si_unit = hash;
                    default_bits.m_bits.impl_unit = impl_hash;
                    default_bits.m_bits.val = 0;

                    if (InterlockedCompareExchange(reinterpret_cast<volatile uint32_t*>(&out.hash), impl_hash, std::numeric_limits< uint32_t>::max()) == std::numeric_limits< uint32_t>::max()) {
                        out.ratio = ratio;
                        out.name = name;
                        out.abbreviation = abbreviation;
                        out.default_bits = default_bits;
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
            if (out.hash == std::numeric_limits< uint32_t>::max()) {
                if (InterlockedCompareExchange(reinterpret_cast<volatile uint32_t*>(&out.hash), base_hash, std::numeric_limits< uint32_t>::max()) == std::numeric_limits< uint32_t>::max()) {
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
        static bool IsInteger(double value) {
            double intpart;
            return modf(value, &intpart) == 0.0;
        };
        static GL::string NumStr(double v) {
            GL::string Num;
            if (IsInteger(v)) {
                Num = std::to_string((int)v);
            }
            else {
                Num = std::to_string(v);
                Num.remove_trailing('0');
                Num.remove_trailing('.');
            }
            return Num;
        }
        static GL::string get_default_abbreviation(double meters, double kilograms, double seconds, double amperes, double dollars) {
            std::array< GL::string, 5> unitBases{ "m", "kg", "s", "A", "$" };
            std::array< double, 5> data{ meters, kilograms, seconds, amperes, dollars };
            GL::string out;
            bool anyNegatives = false;

            for (int i = 0; i < 5; ++i) {
                GL::string& unitBase = unitBases[i];
                double& v = data[i];

                if (v > 0) {
                    if (v == 1)
                        out = out.add_to_delim(unitBase, " ");
                    else {
                        GL::string Num;
                        if (IsInteger(v)) {
                            Num = std::to_string((int)v);
                        }
                        else {
                            Num = std::to_string(v);
                            Num.remove_trailing('0');
                            Num.remove_trailing('.');
                        }
                        out = out.add_to_delim(unitBase + "^" + Num, " ");
                    }
                }
                else if (v < 0) {
                    anyNegatives = true;
                }
            }
            if (anyNegatives) {
                out = out + " /";
                for (int i = 0; i < 5; ++i) {
                    GL::string& unitBase = unitBases[i];
                    double& v = data[i];

                    if (v < 0) {
                        if (v == -1)
                            out = out.add_to_delim(unitBase, " ");
                        else {
                            GL::string Num;
                            if (IsInteger(v)) {
                                Num = std::to_string((int)(-1.0 * v));
                            }
                            else {
                                Num = std::to_string((-1.0 * v));
                                Num.remove_trailing('0');
                                Num.remove_trailing('.');
                            }
                            out = out.add_to_delim(unitBase + "^" + Num, " ");
                        }
                    }
                }
            }
            return out;
        };
    
    private:
        package packed;

        // return the SI ratio of the current type. 
        static double const& ratio(package const& pkg) {
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).ratio;
        };
        // return the abbreviation for the current type. 
        static GL::string const& abbreviation(package const& pkg) {
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).abbreviation;
        };
        // return true if the type is a scaler.
        static bool is_scaler(package const& pkg) {
            return pkg.m_bits.si_unit == 0;
        };
        // Cast a unit value to a similar type. E.g. foot to meter, gallon to cubic foot, inch to scaler, or scaler to inch. Inch to gallon would throw an exception.
        static package cast(package from, package const& to) {
            if (is_scaler(to)) {
                from.m_bits2.unit_hash = 0;
            }
            else if (is_scaler(from)) {
                from.m_bits2.unit_hash = to.m_bits2.unit_hash;
            }
            else if (from.m_bits2.unit_hash == to.m_bits2.unit_hash) {
                // do nothing
            }
            else if (from.m_bits.si_unit == to.m_bits.si_unit) {
                from.m_bits.val = static_cast<float>(static_cast<double>(from.m_bits.val) * ratio(from) / ratio(to));
                from.m_bits.impl_unit = to.m_bits.impl_unit;
            }
            else {
                auto& A1 = abbreviation(from);
                auto& A2 = abbreviation(to);
                throw std::runtime_error(GL::string("Normal arithmetic failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
            }
            return from;
        };

        static bool same_si_units(package const& LHS, package const& RHS) noexcept {
            return LHS.m_bits.si_unit == RHS.m_bits.si_unit;
        };
        static bool identical_units(package const& LHS, package const& RHS) noexcept { 
            return LHS.m_bits2.unit_hash == RHS.m_bits2.unit_hash;
        };
        static bool normal_arithmetic_okay(package const& LHS, package const& RHS) noexcept {
            if ((LHS.m_bits.si_unit == 0) || (RHS.m_bits.si_unit == 0)) return true;
            if (same_si_units(LHS, RHS)) return true;
            return false;
        };
        static bool unary_arithmetic_okay(package const& LHS, package const& RHS) noexcept {
            if (RHS.m_bits.si_unit == 0) return true;
            if (same_si_units(LHS, RHS)) return true;
            return false;
        };
        static void confirm_normal_arithmetic(package const& LHS, package const& RHS) {
            if (!normal_arithmetic_okay(LHS, RHS)) {
                auto& A1 = abbreviation(LHS);
                auto& A2 = abbreviation(RHS);
                throw std::runtime_error(GL::string("Normal arithmetic failed due to incompatible non-scalar value: '" + A1 +"' and '" + A2 + "'").to_string());
            }
        };
        static void confirm_unary_arithmetic(package const& LHS, package const& RHS) {
            if (!unary_arithmetic_okay(LHS, RHS)) {
                auto& A1 = abbreviation(LHS);
                auto& A2 = abbreviation(RHS);
                throw std::runtime_error(GL::string("Unary (in-place or self-modifying) arithmetic failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
            }
        };        
        static void confirm_is_scalar(package const& LHS) {
            if (LHS.m_bits.si_unit != 0) {
                auto& A1 = abbreviation(LHS);
                throw std::runtime_error(GL::string("Type must be scalar (was '" + A1 + "'").to_string());
            }
        };

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
        
        template<typename Func> static bool do_comparison(Func const& toDo, value const& A, value const& V) noexcept {
            auto LHS = A.load();
            auto RHS = V.load();
            if (!normal_arithmetic_okay(LHS, RHS)) return false; // we aren't the same category -- just early-exit

            // we are the same category
            if (is_scaler(LHS) == is_scaler(RHS)) {
                return toDo(LHS.m_bits.val, RHS.m_bits.val);
            }
            else if (is_scaler(RHS)) {
                return toDo(LHS.m_bits.val, cast(RHS, LHS).m_bits.val);
            }
            else { // LHS is a scalar.  
                return toDo(cast(LHS, RHS).m_bits.val, RHS.m_bits.val);
            }
        };
        template<typename Func> static void atomic_add_or_sub(Func const& toDo, value& lhs, value const& rhs) {
            auto RHS = rhs.load();
            lhs.UpdatePackage([&](package Old) -> package {
                confirm_unary_arithmetic(Old, RHS);

                if (Old.m_bits2.unit_hash == RHS.m_bits2.unit_hash) {
                    toDo(Old.m_bits.val, RHS.m_bits.val);
                    // Old.m_bits.val += RHS.m_bits.val;
                }
                else if (Old.m_bits.si_unit == RHS.m_bits.si_unit) {
                    const double& RHS_ratio = ratio(RHS);
                    const double& LHS_ratio = ratio(Old);
                    toDo(Old.m_bits.val, static_cast<float>(static_cast<double>(RHS.m_bits.val) * RHS_ratio / LHS_ratio));
                }
                else if (RHS.m_bits.si_unit == 0) {
                    toDo(Old.m_bits.val, RHS.m_bits.val);
                }
                else /*if (LHS.m_bits.si_unit == 0)*/ {
                    Old = cast(Old, RHS);
                    toDo(Old.m_bits.val, RHS.m_bits.val);
                }
                return Old;
            });
        };
        template<typename Func> static void ST_add_or_sub(Func const& toDo, value& lhs, value const& rhs) {
            auto RHS = rhs.load();
            auto& LHS = lhs.packed;

            confirm_unary_arithmetic(LHS, RHS);

            if (LHS.m_bits2.unit_hash == RHS.m_bits2.unit_hash) {
                toDo(LHS.m_bits.val, RHS.m_bits.val);
            }
            else if (LHS.m_bits.si_unit == RHS.m_bits.si_unit) {
                const double& RHS_ratio = ratio(RHS);
                const double& LHS_ratio = ratio(LHS);
                toDo(LHS.m_bits.val, static_cast<float>(static_cast<double>(RHS.m_bits.val) * RHS_ratio / LHS_ratio));
            }
            else if (RHS.m_bits.si_unit == 0) {
                toDo(LHS.m_bits.val, RHS.m_bits.val);
            }
            else /*if (LHS.m_bits.si_unit == 0)*/ {
                LHS = cast(LHS, RHS);
                toDo(LHS.m_bits.val, RHS.m_bits.val);
            }
        };

        static package compound_units(package lhs, package rhs, bool multiplication = true) {
            bool lhs_is_scalar = is_scaler(lhs);
            bool rhs_is_scalar = is_scaler(rhs);

            // early-exit if the RHS is a scalar, which will not change the units of the LHS
            if (rhs_is_scalar) {
                if (multiplication) {
                    lhs.m_bits.val *= rhs.m_bits.val;
                    return lhs;
                }
                else {
                    lhs.m_bits.val /= rhs.m_bits.val;
                    return lhs;
                }
            }

            // RHS is not a scaler, so the result could become one.
            auto& lhs_si_units = get_si_unit(lhs.m_bits.si_unit);
            auto& rhs_si_units = get_si_unit(rhs.m_bits.si_unit);
            if (multiplication) {
                auto& new_si_units = get_si_unit(
                    lhs_si_units.METERS + rhs_si_units.METERS
                    , lhs_si_units.KILOGRAMS + rhs_si_units.KILOGRAMS
                    , lhs_si_units.SECONDS + rhs_si_units.SECONDS
                    , lhs_si_units.AMPERES + rhs_si_units.AMPERES
                    , lhs_si_units.DOLLARS + rhs_si_units.DOLLARS
                );
                const double& lhs_ratio = lhs_si_units.implimented_units->operator[](lhs.m_bits.impl_unit).ratio;
                const double& rhs_ratio = rhs_si_units.implimented_units->operator[](rhs.m_bits.impl_unit).ratio;
                double desired_ratio = lhs_ratio * rhs_ratio;
                if (auto* _impl_unit = new_si_units.try_get_nearest_impl_unit(desired_ratio)) {
                    // found or found nearby
                    package out{ _impl_unit->default_bits };
                    out.m_bits.val = static_cast<float>((static_cast<double>(lhs.m_bits.val) * lhs_ratio) * (static_cast<double>(rhs.m_bits.val) * rhs_ratio) / _impl_unit->ratio);
                    return out;
                }
                else {
                    // create new
                    GL::string abbrev
                        // = get_default_abbreviation(new_si_units.METERS, new_si_units.KILOGRAMS, new_si_units.SECONDS, new_si_units.AMPERES, new_si_units.DOLLARS);
                        = abbreviation(lhs) + "*" + abbreviation(rhs);
                    // NOTE: if using the default abbreviations, you must then set the ratio to '0' as this is now an SI unit. 
                    auto& new_impl_unit = new_si_units.get_impl_unit(desired_ratio, "", abbrev);
                    package out{ new_impl_unit.default_bits };
                    out.m_bits.val = static_cast<float>((static_cast<double>(lhs.m_bits.val) * lhs_ratio) * (static_cast<double>(rhs.m_bits.val) * rhs_ratio) / new_impl_unit.ratio);
                    return out;
                }               
            }
            else {
                auto& new_si_units = get_si_unit(
                    lhs_si_units.METERS - rhs_si_units.METERS
                    , lhs_si_units.KILOGRAMS - rhs_si_units.KILOGRAMS
                    , lhs_si_units.SECONDS - rhs_si_units.SECONDS
                    , lhs_si_units.AMPERES - rhs_si_units.AMPERES
                    , lhs_si_units.DOLLARS - rhs_si_units.DOLLARS
                );
                const double& lhs_ratio = lhs_si_units.implimented_units->operator[](lhs.m_bits.impl_unit).ratio;
                const double& rhs_ratio = rhs_si_units.implimented_units->operator[](rhs.m_bits.impl_unit).ratio;
                double desired_ratio = lhs_ratio / rhs_ratio;
                if (auto* _impl_unit = new_si_units.try_get_nearest_impl_unit(desired_ratio)) {
                    // found or found nearby
                    package out{ _impl_unit->default_bits };
                    out.m_bits.val = static_cast<float>((static_cast<double>(lhs.m_bits.val) * lhs_ratio) / (static_cast<double>(rhs.m_bits.val) * rhs_ratio) / _impl_unit->ratio);
                    return out;
                }
                else {
                    // create new
                    GL::string abbrev
                        // = get_default_abbreviation(new_si_units.METERS, new_si_units.KILOGRAMS, new_si_units.SECONDS, new_si_units.AMPERES, new_si_units.DOLLARS);
                        = abbreviation(lhs) + "/" + abbreviation(rhs);
                    // NOTE: if using the default abbreviations, you must then set the ratio to '0' as this is now an SI unit. 
                    auto& new_impl_unit = new_si_units.get_impl_unit(desired_ratio, "", abbrev);
                    package out{ new_impl_unit.default_bits };
                    out.m_bits.val = static_cast<float>((static_cast<double>(lhs.m_bits.val) * lhs_ratio) / (static_cast<double>(rhs.m_bits.val) * rhs_ratio) / new_impl_unit.ratio);
                    return out;
                }
            }
        };
        static package multiply_units(package lhs, double rhs) {
            if (rhs == 1.0) {
                return lhs;
            }
            else {
                bool lhs_is_scalar = is_scaler(lhs);
                // early-exit if the LHS is a scalar
                if (lhs_is_scalar) {
                    lhs.m_bits.val = static_cast<float>(std::pow(static_cast<double>(lhs.m_bits.val), rhs));
                    return lhs;
                }

                // RHS is not a scaler, so the result could become one.
                auto& lhs_si_units = get_si_unit(lhs.m_bits.si_unit);
                auto& new_si_units = get_si_unit(
                    lhs_si_units.METERS * rhs
                    , lhs_si_units.KILOGRAMS * rhs
                    , lhs_si_units.SECONDS * rhs
                    , lhs_si_units.AMPERES * rhs
                    , lhs_si_units.DOLLARS * rhs
                );
                const double& lhs_ratio = lhs_si_units.implimented_units->operator[](lhs.m_bits.impl_unit).ratio;
                double desired_ratio = std::pow(lhs_ratio, rhs);
                if (auto* _impl_unit = new_si_units.try_get_nearest_impl_unit(desired_ratio)) {
                    // found or found nearby
                    package out{ _impl_unit->default_bits };
                    out.m_bits.val = static_cast<float>(std::pow(static_cast<double>(lhs.m_bits.val) * lhs_ratio, rhs) / _impl_unit->ratio);
                    return out;
                }
                else {
                    // create new
                    GL::string abbrev
                        // = get_default_abbreviation(new_si_units.METERS, new_si_units.KILOGRAMS, new_si_units.SECONDS, new_si_units.AMPERES, new_si_units.DOLLARS);
                        = abbreviation(lhs) + "^" + std::to_string(rhs);
                    // NOTE: if using the default abbreviations, you must then set the ratio to '0' as this is now an SI unit. 
                    auto& new_impl_unit = new_si_units.get_impl_unit(desired_ratio, "", abbrev);
                    package out{ new_impl_unit.default_bits };
                    out.m_bits.val = static_cast<float>(std::pow(static_cast<double>(lhs.m_bits.val) * lhs_ratio, rhs) / new_impl_unit.ratio);
                    return out;
                }
            }
        };

        explicit value(package&& from) : packed(std::move(from)) {};

        package load() const {
            return packed;
        };
        void store(package const& data) {
            InterlockedExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), data.m_n64);
        };
        void store(package&& data) {
            InterlockedExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), std::move(data.m_n64));
        };
        package exchange(package&& data) {
            package out;
            out.m_n64 = InterlockedExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), std::move(data.m_n64));
            return out;
        };
        package exchange(package const& data) {
            package out;
            out.m_n64 = InterlockedExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), data.m_n64);
            return out;
        };
        bool compare_exchange(package const& expected, package&& newValue) {
            return InterlockedCompareExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), std::move(newValue.m_n64), expected.m_n64) == expected.m_n64;
        };
        bool compare_exchange(package const& expected, package const& newValue) {
            return InterlockedCompareExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), newValue.m_n64, expected.m_n64) == expected.m_n64;
        };

    public:        
        value() : value(package{ package::bitset2{ 0ull, 0.0f } }) {};       
        explicit value(impl_unit const& from) : packed(from.default_bits) {};
        value(float rhs) : value(package{ package::bitset2{ 0ull, rhs } }) {};
        value(value const& rhs) : packed(rhs.packed) {};
        value(value && rhs) noexcept : packed(rhs.packed) {};
        value& operator=(value const& RHS){
            if (this == &RHS) return *this;

            auto rhs = RHS.load();
            UpdatePackage([&](package lhs) -> package {
                if (is_scaler(lhs) || identical_units(lhs, rhs)) {
                    return rhs;
                }
                else if (is_scaler(rhs)) {
                    lhs.m_bits.val = rhs.m_bits.val;
                    return lhs;
                }
                else if (same_si_units(lhs, rhs)) {
                    lhs.m_bits.val = static_cast<float>(static_cast<double>(rhs.m_bits.val) * ratio(rhs) / ratio(lhs));
                    return lhs;
                }
                else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
                    auto& A1 = abbreviation(lhs);
                    auto& A2 = abbreviation(rhs);
                    throw std::runtime_error(GL::string("Assignment(=) failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
                }
            });
            return *this;

            //InterlockedExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), rhs.packed.m_n64);
            //return *this;
        };
        value& operator=(value&& RHS) noexcept {
            auto rhs = std::move(RHS.packed);
            UpdatePackage([&](package lhs) -> package {
                if (is_scaler(lhs) || identical_units(lhs, rhs)) {
                    return rhs;
                }
                else if (is_scaler(rhs)) {
                    lhs.m_bits.val = rhs.m_bits.val;
                    return lhs;
                }
                else if (same_si_units(lhs, rhs)) {
                    lhs.m_bits.val = static_cast<float>(static_cast<double>(rhs.m_bits.val) * ratio(rhs) / ratio(lhs));
                    return lhs;
                }
                else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
                    auto& A1 = abbreviation(lhs);
                    auto& A2 = abbreviation(rhs);
                    throw std::runtime_error(GL::string("Assignment(=) failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
                }
            });
            return *this;

            //InterlockedExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), rhs.packed.m_n64);
            //return *this;
        };
        ~value() = default;

        operator float() const {
            return this->packed.m_bits.val;
        };
        GL::string const& name() const {
            auto pkg = load();
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).name;
        };
        GL::string const& abbreviation() const {
            auto pkg = load();
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).abbreviation;
        };
        const double& ratio() const {
            auto pkg = load();
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).ratio;
        };
        bool is_scaler() const {
            auto pkg = load();
            return pkg.m_bits.si_unit == 0;
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
            return value(std::move(t));
        };
        value& operator+=(value const& rhs) {
            atomic_add_or_sub([](float& lhs, float const& rhs) {
                lhs += rhs;
            }, *this, rhs);
            return *this;
        };
        value& operator-=(value const& rhs) {
            atomic_add_or_sub([](float& lhs, float const& rhs) {
                lhs -= rhs;
            }, *this, rhs);
            return *this;
        };
        friend value operator+(value const& lhs, value const& rhs) {
            value LHS(lhs);
            ST_add_or_sub([](float& lhs, float const& rhs) {
                lhs += rhs;
            }, LHS, rhs);
            return LHS;
        };
        friend value operator-(value const& lhs, value const& rhs) {
            value LHS(lhs);
            ST_add_or_sub([](float& lhs, float const& rhs) {
                lhs -= rhs;
            }, LHS, rhs);
            return LHS;
        };
        friend value operator*(value const& lhs, value const& rhs) {
            return value(compound_units(lhs.load(), rhs.load(), true));
        };
        friend value operator/(value const& lhs, value const& rhs) {
            return value(compound_units(lhs.load(), rhs.load(), false));
        };
        value& operator*=(value const& RHS) {
            auto rhs = RHS.load();
            this->UpdatePackage([&](package Old) -> package {
                if ((rhs.m_bits.si_unit == 0) || (Old.m_bits.si_unit == 0)) {
                    return compound_units(Old, rhs, true);
                }
                else {
                    auto& A1 = abbreviation(Old);
                    auto& A2 = abbreviation(rhs);
                    throw std::runtime_error(GL::string("Unary (in-place or self-modifying) arithmetic failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
                }
            });
            return *this;
        };
        value& operator/=(value const& RHS) {
            auto rhs = RHS.load();
            this->UpdatePackage([&](package Old) -> package {
                if ((rhs.m_bits.si_unit == 0) || (Old.m_bits.si_unit == 0)) {
                    return compound_units(Old, rhs, false);
                }
                else {
                    auto& A1 = abbreviation(Old);
                    auto& A2 = abbreviation(rhs);
                    throw std::runtime_error(GL::string("Unary (in-place or self-modifying) arithmetic failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
                }
            });
            return *this;
        };

        friend bool operator==(value const& A, value const& V) noexcept {
            auto LHS = A.packed;
            auto RHS = V.packed;
            if (LHS.m_bits2.unit_hash == RHS.m_bits2.unit_hash) {
                return A.packed.m_n64 == V.packed.m_n64;
            }
            else {
                return do_comparison([](float const& lhs, float const& rhs) -> bool { return std::abs(lhs - rhs) < 0.000001; }, A, V);
            }
        };
        friend bool operator<(value const& A, value const& V) {
            return do_comparison([](float const& lhs, float const& rhs) -> bool { return lhs < rhs; }, A, V);
        };
        friend bool operator<=(value const& A, value const& V) {
            return do_comparison([](float const& lhs, float const& rhs) -> bool { return lhs <= rhs; }, A, V);
        };
        friend bool operator>(value const& A, value const& V) {
            return !operator<=(A, V);
        };
        friend bool operator>=(value const& A, value const& V) {
            return !operator<(A, V);
        };
        friend bool operator!=(value const& A, value const& V) noexcept {
            return !operator==(A, V);
        };
        friend std::ostream& operator<<(std::ostream& os, value const& obj) {
            auto pkg = obj.packed;
            GL::string out = NumStr(static_cast<double>(pkg.m_bits.val)) + " " + abbreviation(pkg);
            os << out;
            return os;
        };

        // Returns a new value multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
        value pow(value const& V) const {
            auto other = V.load();
            confirm_is_scalar(other);
            return value(multiply_units(this->load(), other.m_bits.val));
        };
        // updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
        value pow_value(value const& V) const {
            auto other = V.load();
            confirm_is_scalar(other);

            auto pkg = this->load();
            pkg.m_bits.val = std::pow(pkg.m_bits.val, other.m_bits.val);

            return value(std::move(pkg));
        };
        // pow(0.5)
        value sqrt() const {
            return pow(0.5);
        };
        // Creates a copy of the value and floors (rounds to lower whole integer) the underlying value
        value floor() const {
            value out{ *this };
            out.packed.m_bits.val = std::floor(out.packed.m_bits.val);
            return out;
        };
        // Creates a copy of the value and ceilings (rounds to upper whole integer) the underlying value
        value ceiling() const {
            value out{ *this };
            out.packed.m_bits.val = std::ceil(out.packed.m_bits.val);
            return out;
        };

        // faster call to update the value without atomic ensurances. 
        template <typename F> value& single_threaded_update(F const& func) {
            func(this->packed.m_bits.val);
            return *this;
        };
    };




};