#pragma once
#define DECL_UNIT_LITERALS
#include "Strings.h"
#include "atomic_maps.h"
#include <concurrent_unordered_map.h>
#include <limits>
#include <sstream>

namespace GL {
    /* 
    Atomic floating-point value that keeps track of its own units. 
    Allowed to mix-and-match units dynamically, and will attempt to correctly select the most appropriate new unit type. 
    Supports addition, multiplication, powers, and other mathematical operations on unit types. All of these are performed atomicly on primitives with CAS operations. 
    */
    class value {
    public:
        class Categories {
        public:
            class length {
            public: static constexpr std::array<double, 6> unitType_m{ 1, 0, 0, 0, 0, 0 };
            };
            class mass {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 1, 0, 0, 0, 0 };
            };
            class time {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, 1, 0, 0, 0 };
            };
            class current {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, 0, 1, 0, 0 };
            };
            class dollar {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, 0, 0, 1, 0 };
            };
            class frequency {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, -1, 0, 0, 0 };
            };
            class velocity {
            public: static constexpr std::array<double, 6> unitType_m{ 1, 0, -1, 0, 0, 0 };
            };
            class acceleration {
            public: static constexpr std::array<double, 6> unitType_m{ 1, 0, -2, 0, 0, 0 };
            };
            class force {
            public: static constexpr std::array<double, 6> unitType_m{ 1, 1, -2, 0, 0, 0 };
            };
            class pressure {
            public: static constexpr std::array<double, 6> unitType_m{ -1, 1, -2, 0, 0, 0 };
            };
            class charge {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, 1, 1, 0, 0 };
            };
            class power {
            public: static constexpr std::array<double, 6> unitType_m{ 2, 1, -3, 0, 0, 0 };
            };
            class energy {
            public: static constexpr std::array<double, 6> unitType_m{ 2, 1, -2, 0, 0, 0 };
            };
            class voltage {
            public: static constexpr std::array<double, 6> unitType_m{ 2, 1, -3, -1, 0, 0 };
            };
            class impedance {
            public: static constexpr std::array<double, 6> unitType_m{ 2, 1, -3, -2, 0, 0 };
            };
            class conductance {
            public: static constexpr std::array<double, 6> unitType_m{ -2, -1, 3, 2, 0, 0 };
            };
            class area {
            public: static constexpr std::array<double, 6> unitType_m{ 2, 0, 0, 0, 0, 0 };
            };
            class volume {
            public: static constexpr std::array<double, 6> unitType_m{ 3, 0, 0, 0, 0, 0 };
            };
            class fillrate {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 1, -1, 0, 0, 0 };
            };
            class flowrate {
            public: static constexpr std::array<double, 6> unitType_m{ 3, 0, -1, 0, 0, 0 };
            };
            class density {
            public: static constexpr std::array<double, 6> unitType_m{ -3, 1, 0, 0, 0, 0 };
            };
            class temperature {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, 0, 0, 1, 0 };
            };
            class angle {
            public: static constexpr std::array<double, 6> unitType_m{ 0, 0, 0, 0, 0, 1 };
            };

        };
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
                name{ "scalar" };
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
                KELVIN{ 0 }; // si unit for temperature
            double
                RADIANS{ 0 }; // si unit for angle
            uint32_t
                hash{ std::numeric_limits< uint32_t>::max() };
            GL::deferred<concurrency::concurrent_unordered_map<uint16_t, impl_unit>>
                implimented_units{};
            GL::deferred<GL::atomic_map<double, impl_unit*>>
                sorted_units{};

            static uint16_t calc_si_hash(double meters, double kilograms, double seconds, double amperes, double kelvin, double radians) {
                if ((meters == 0) && (kilograms == 0) && (seconds == 0) && (amperes == 0) && (kelvin == 0) && (radians == 0)) {
                    return 0;
                }
                else {
                    size_t out = 0;
                    out ^= *(uint64_t*)(void*)(&meters) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&kilograms) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&seconds) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&amperes) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&kelvin) + 0x9e3779b9 + (out << 6) + (out >> 2);
                    out ^= *(uint64_t*)(void*)(&radians) + 0x9e3779b9 + (out << 6) + (out >> 2);
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

            impl_unit* try_get_impl_unit(double ratio) {
                uint16_t impl_hash = (((hash == 0) || (ratio == 0)) ? 0 : si_unit::calc_impl_hash(ratio));
                if (auto F = implimented_units->find(impl_hash); F != implimented_units->end()) {
                    return &F->second;
                }
                else {
                    return nullptr;
                }
            };
            impl_unit* try_get_nearest_impl_unit(double ratio) {
                if (auto F = sorted_units->find_less_or_equal(ratio); F != sorted_units->end()) {
                    return F->second;
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
                        sorted_units->insert_fast(ratio, &out);
                    }
                }
                return out;
            };
        };
        // get the cached si unit for this type (fast, assuming already have the unique hash value)
        static si_unit& get_si_unit(uint16_t hash);
        // get the cached si unit for this type (slow, assumes the unique hash is not known or needs to be initialized)
        static si_unit& get_si_unit(double meters, double kilograms, double seconds, double amperes, double kelvin, double radians) {
            uint16_t base_hash = si_unit::calc_si_hash(meters, kilograms, seconds, amperes, kelvin, radians);
            si_unit& out = get_si_unit(base_hash);
            if (out.hash == std::numeric_limits< uint32_t>::max()) {
                if (InterlockedCompareExchange(reinterpret_cast<volatile uint32_t*>(&out.hash), base_hash, std::numeric_limits< uint32_t>::max()) == std::numeric_limits< uint32_t>::max()) {
                    out.METERS = meters;
                    out.KILOGRAMS = kilograms;
                    out.SECONDS = seconds;
                    out.AMPERES = amperes;
                    out.KELVIN = kelvin;
                    out.RADIANS = radians;
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
            Num = std::to_string(v);
            Num = Num.remove_trailing('0');
            Num = Num.remove_trailing('.');            
            return Num;
        }
        static GL::string get_default_abbreviation(double meters, double kilograms, double seconds, double amperes, double kelvin, double radians) {
            std::array< GL::string, 6> unitBases{ "m", "kg", "s", "A", "K", "rad" };
            std::array< double, 6> data{ meters, kilograms, seconds, amperes, kelvin, radians };
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

    protected:
        package packed;

        // return the SI ratio of the current type. 
        static double const& ratio(package const& pkg) {
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).ratio;
        };
        // return the abbreviation for the current type. 
        static GL::string const& abbreviation(package const& pkg) {
            return get_si_unit(pkg.m_bits.si_unit).implimented_units->operator[](pkg.m_bits.impl_unit).abbreviation;
        };
        // return true if the type is a scalar.
        static bool is_scalar(package const& pkg) {
            return pkg.m_bits.si_unit == 0;
        };
        // Cast a unit value to a similar type. E.g. foot to meter, gallon to cubic foot, inch to scalar, or scalar to inch. Inch to gallon would throw an exception.
        static package cast(package from, package const& to) {
            if (is_scalar(to)) {
                from.m_bits2.unit_hash = 0;
            }
            else if (is_scalar(from)) {
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
                throw std::runtime_error(GL::string("Normal arithmetic failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
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
                if (compare_exchange_p(Old, New)) {
                    return Old;
                }
            }
        };
        template <typename F> package UpdatePackage(F const& update_func) {
            package Old;
            while (true) {
                Old = load();
                if (compare_exchange_p(Old, update_func(Old))) {
                    return Old;
                }
            }
        };

        template<typename Func> static bool do_comparison(Func const& toDo, value const& A, value const& V) noexcept {
            auto LHS = A.load();
            auto RHS = V.load();

            if (identical_units(LHS, RHS)) {
                return toDo(LHS.m_bits.val, RHS.m_bits.val);
            }
            else if (same_si_units(LHS, RHS)) {
                return toDo(LHS.m_bits.val, cast(RHS, LHS).m_bits.val);
            }
            else {
                return toDo(LHS.m_bits.val, RHS.m_bits.val);
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
        template <bool multiplication = true> static package compound_units(package lhs, package rhs) {
            bool lhs_is_scalar = is_scalar(lhs);
            bool rhs_is_scalar = is_scalar(rhs);

            // early-exit if the RHS is a scalar, which will not change the units of the LHS
            if (rhs_is_scalar) {
                if constexpr (multiplication) {
                    lhs.m_bits.val *= rhs.m_bits.val;
                    return lhs;
                }
                else {
                    lhs.m_bits.val /= rhs.m_bits.val;
                    return lhs;
                }
            }

            // RHS is not a scalar, so the result could become one.
            auto& lhs_si_units = get_si_unit(lhs.m_bits.si_unit);
            auto& rhs_si_units = get_si_unit(rhs.m_bits.si_unit);
            if constexpr (multiplication) {
                auto& new_si_units = get_si_unit(
                    lhs_si_units.METERS + rhs_si_units.METERS
                    , lhs_si_units.KILOGRAMS + rhs_si_units.KILOGRAMS
                    , lhs_si_units.SECONDS + rhs_si_units.SECONDS
                    , lhs_si_units.AMPERES + rhs_si_units.AMPERES
                    , lhs_si_units.KELVIN + rhs_si_units.KELVIN
                    , lhs_si_units.RADIANS + rhs_si_units.RADIANS
                );
                const double& lhs_ratio = lhs_si_units.implimented_units->operator[](lhs.m_bits.impl_unit).ratio;
                const double& rhs_ratio = rhs_si_units.implimented_units->operator[](rhs.m_bits.impl_unit).ratio;
                double desired_ratio = lhs_ratio * rhs_ratio;

                auto* _impl_unit = new_si_units.try_get_impl_unit(desired_ratio);
                if (!_impl_unit) _impl_unit = _impl_unit = new_si_units.try_get_nearest_impl_unit((static_cast<double>(lhs.m_bits.val) * lhs_ratio) * (static_cast<double>(rhs.m_bits.val) * rhs_ratio));
                if (_impl_unit) {
                    // found or found nearby
                    package out{ _impl_unit->default_bits };
                    out.m_bits.val = static_cast<float>((static_cast<double>(lhs.m_bits.val) * lhs_ratio) * (static_cast<double>(rhs.m_bits.val) * rhs_ratio) / _impl_unit->ratio);
                    return out;
                }
                else {
                    // create new
                    GL::string abbrev
                        = "(" + abbreviation(lhs) + ")*(" + abbreviation(rhs) + ")";
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
                    , lhs_si_units.KELVIN - rhs_si_units.KELVIN
                    , lhs_si_units.RADIANS - rhs_si_units.RADIANS
                );
                const double& lhs_ratio = lhs_si_units.implimented_units->operator[](lhs.m_bits.impl_unit).ratio;
                const double& rhs_ratio = rhs_si_units.implimented_units->operator[](rhs.m_bits.impl_unit).ratio;
                double desired_ratio = lhs_ratio / rhs_ratio;

                auto* _impl_unit = new_si_units.try_get_impl_unit(desired_ratio);
                if (!_impl_unit) _impl_unit = _impl_unit = new_si_units.try_get_nearest_impl_unit((static_cast<double>(lhs.m_bits.val) * lhs_ratio) / (static_cast<double>(rhs.m_bits.val) * rhs_ratio));

                if (_impl_unit) {
                    // found or found nearby
                    package out{ _impl_unit->default_bits };
                    out.m_bits.val = static_cast<float>((static_cast<double>(lhs.m_bits.val) * lhs_ratio) / (static_cast<double>(rhs.m_bits.val) * rhs_ratio) / _impl_unit->ratio);
                    return out;
                }
                else {
                    // create new
                    GL::string abbrev
                        = "(" + abbreviation(lhs) + ")/(" + abbreviation(rhs) + ")";
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
                bool lhs_is_scalar = is_scalar(lhs);
                // early-exit if the LHS is a scalar
                if (lhs_is_scalar) {
                    lhs.m_bits.val = static_cast<float>(std::pow(static_cast<double>(lhs.m_bits.val), rhs));
                    return lhs;
                }

                // RHS is not a scalar, so the result could become one.
                auto& lhs_si_units = get_si_unit(lhs.m_bits.si_unit);
                auto& new_si_units = get_si_unit(
                    lhs_si_units.METERS * rhs
                    , lhs_si_units.KILOGRAMS * rhs
                    , lhs_si_units.SECONDS * rhs
                    , lhs_si_units.AMPERES * rhs
                    , lhs_si_units.KELVIN * rhs
                    , lhs_si_units.RADIANS * rhs
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
                        = "(" + abbreviation(lhs) + ")^" + NumStr(rhs);
                    // NOTE: if using the default abbreviations, you must then set the ratio to '0' as this is now an SI unit. 
                    auto& new_impl_unit = new_si_units.get_impl_unit(desired_ratio, "", abbrev);
                    package out{ new_impl_unit.default_bits };
                    out.m_bits.val = static_cast<float>(std::pow(static_cast<double>(lhs.m_bits.val) * lhs_ratio, rhs) / new_impl_unit.ratio);
                    return out;
                }
            }
        };

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
        bool compare_exchange_p(package const& expected, package&& newValue) {
            return InterlockedCompareExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), std::move(newValue.m_n64), expected.m_n64) == expected.m_n64;
        };
        bool compare_exchange_p(package const& expected, package const& newValue) {
            return InterlockedCompareExchange(reinterpret_cast<volatile uint64_t*>(&packed.m_n64), newValue.m_n64, expected.m_n64) == expected.m_n64;
        };

    protected:
        explicit value(package&& from) : packed(std::move(from)) {};
        void TrySetTo(value const& RHS) {
            if (this == &RHS) return;

            auto rhs = RHS.load();
            UpdatePackage([&](package lhs) -> package {
                if (is_scalar(lhs) || identical_units(lhs, rhs)) {
                    return rhs;
                }
                else if (is_scalar(rhs)) {
                    lhs.m_bits.val = rhs.m_bits.val;
                    return lhs;
                }
                else if (same_si_units(lhs, rhs)) {
                    lhs.m_bits.val = static_cast<float>(static_cast<double>(rhs.m_bits.val) * ratio(rhs) / ratio(lhs));
                    return lhs;
                }
                else { // incoming unit AND this unit are different non-scalars of different categories. No exchange is reasonable. 
                    auto& A1 = abbreviation(lhs);
                    auto& A2 = abbreviation(rhs);
                    throw std::runtime_error(GL::string("Assignment(=) failed due to incompatible non-scalar value: '" + A1 + "' and '" + A2 + "'").to_string());
                }
            });
        };

    public:      
        static const concurrency::concurrent_unordered_map<uint16_t, value::si_unit>& all_known_unit_types();

        value() : value(package{ package::bitset2{ 0ull, 0.0f } }) {};       
        explicit value(impl_unit const& from) : packed(from.default_bits) {};
        value(float rhs) : value(package{ package::bitset2{ 0ull, rhs } }) {};
        value(value const& rhs) : packed(rhs.packed) {};
        value(value && rhs) noexcept : packed(rhs.packed) {};
        value& operator=(value const& RHS){
            TrySetTo(RHS);
            return *this;
        };
        value& operator=(value&& RHS) noexcept {
            TrySetTo(std::move(RHS));
            return *this;
        };
        ~value() = default;

        bool compare_exchange(value& expected, value&& newValue) {
            return compare_exchange_p(expected.packed, std::move(newValue.packed));
        };
        bool compare_exchange(value& expected, value const& newValue) {
            return compare_exchange_p(expected.packed, newValue.packed);
        };

        explicit operator float() const {
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
        bool is_scalar() const {
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
            return value(compound_units< true >(lhs.load(), rhs.load()));
        };
        friend value operator/(value const& lhs, value const& rhs) {
            return value(compound_units< false >(lhs.load(), rhs.load()));
        };
        value& operator*=(value const& RHS) {
            auto rhs = RHS.load();
            this->UpdatePackage([&](package Old) -> package {
                if ((rhs.m_bits.si_unit == 0) || (Old.m_bits.si_unit == 0)) {
                    return compound_units< true >(Old, rhs);
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
                    return compound_units< false >(Old, rhs);
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
            //auto LHS = A.packed;
            //auto RHS = V.packed;
            //if (LHS.m_bits2.unit_hash == RHS.m_bits2.unit_hash) {
            //    return A.packed.m_n64 == V.packed.m_n64;
            //}
            //else {
                return do_comparison([](float const& lhs, float const& rhs) -> bool { return std::abs(lhs - rhs) < 0.000001; }, A, V);
            //}
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
        // Calculates the absolute value
        value abs() const {
            value out{ *this };
            out.packed.m_bits.val = std::fabs(out.packed.m_bits.val);
            return out;
        };
        // clamp number to lower/upper bound
        value clamp(value const& min, value const& max) const {
            value V{ *this };
            auto lhs = min.cast(min.load(), V.packed);
            auto rhs = min.cast(max.load(), V.packed);
            if (V.packed.m_bits.val < lhs.m_bits.val) return value(package(lhs));
            if (V.packed.m_bits.val > rhs.m_bits.val) return value(package(rhs));
            return V;
        };
        // round to nearest number, as denoted by the magnitude. 
        value round(float magnitude = 1.0f) const {
            value V{ *this };
            V.packed.m_bits.val = std::floor((V.packed.m_bits.val / magnitude) + 0.5f) * magnitude;
            return V;
        };
        // return max(this, b);
        value max(const value& b) const {
            value V{ *this };
            auto lhs = b.cast(b.load(), V.packed);
            if (V.packed.m_bits.val < lhs.m_bits.val) return value(package(lhs));
            return V;
        };
        // return min(this, b);
        value min(const value& b) const {
            value V{ *this };
            auto lhs = b.cast(b.load(), V.packed);
            if (V.packed.m_bits.val > lhs.m_bits.val) return value(package(lhs));
            return V;
        };
        // directly access the current floating-point value, stored in the current unit expression. Should only modify in single-threaded fashion using this accessor. 
        float& unsafe_get() {
            return packed.m_bits.val;
        };
        // directly access the current floating-point value, stored in the current unit expression. Should only modify in single-threaded fashion using this accessor. 
        const float& unsafe_get() const {
            return packed.m_bits.val;
        };
        // return the log2 of this value. Returns unitless. 
        value log2() const {
            value out{ *this };
            out.packed.m_bits2.unit_hash = 0;
            out.packed.m_bits2.val = std::log2(out.packed.m_bits2.val);
            return out;
        };
        // return the log10 of this value. Returns unitless. 
        value log10() const {
            value out{ *this };
            out.packed.m_bits2.unit_hash = 0;
            out.packed.m_bits2.val = std::log10(out.packed.m_bits2.val);
            return out;
        };
        // return the log of this value. Returns unitless. 
        value log() const {
            value out{ *this };
            out.packed.m_bits2.unit_hash = 0;
            out.packed.m_bits2.val = std::log(out.packed.m_bits2.val);
            return out;
        };
        // return the scalar sign of this value (e.g. +1 or -1). Returns unitless. 
        value sign() const {
            value out{ *this };
            out.packed.m_bits2.unit_hash = 0;
            out.packed.m_bits2.val = (out.packed.m_bits2.val < 0.0f ? -1.0f : 1.0f);
            return out;
        };
#ifdef DECL_UNIT_LITERALS 
        value sin() const;
        value cos() const;
        value tan() const;

        value asin() const;
        value acos() const;
        value atan() const;

#endif
    };
    using scalar = value;    
};

// std::to_string and std::hash and std::numeric_limits
namespace std {
    _NODISCARD inline std::string to_string(GL::value const& _Val) { // convert number to string
        std::string out;
        std::ostringstream str(out);
        str << _Val;
        return out;
    };
    template <> struct hash<GL::value> {
        std::size_t operator()(const GL::value& k) const {
            return std::hash<double>{}(static_cast<double>((float)k));
        };
    };    
    template<> class numeric_limits<GL::value> {
    public: 
        static constexpr double min() { return std::numeric_limits<float>::min(); } 
        static constexpr double max() { return std::numeric_limits<float>::max(); } 
        static constexpr double lowest() { return std::numeric_limits<float>::lowest(); } 
        static constexpr bool is_integer = std::numeric_limits<float>::is_integer; 
        static constexpr bool is_signed = std::numeric_limits<float>::is_signed;
    }; 
};

// LITERALS AND CONSTANTS
#ifdef DECL_UNIT_LITERALS 
namespace GL {
    template <typename T> __forceinline static constexpr double Conversion(double ratio_of) { return ratio_of * T::conversion_ratio; };
    __forceinline static constexpr double SQUARED(double X) { return X * X; };
    __forceinline static constexpr double CUBED(double X) { return X * X * X; };

#define CalculateMetricPrefixV(metric) ((long double)std::metric::num / (long double)std::metric::den)
#define DerivedUnitList \
    DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0); \
    DerivedUnitType(foot, length, ft, Conversion<meter>(381.0 / 1250.0)); \
	DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0)); \
	DerivedUnitType(furlong, length, fur, Conversion<foot>(660)); \
	DerivedUnitType(mile, length, mi, Conversion<foot>(5280)); \
	DerivedUnitType(nauticalMile, length, nmi, Conversion<meter>(1852.0)); \
	DerivedUnitType(astronicalUnit, length, au, Conversion<meter>(149597870700.0)); \
	DerivedUnitType(yard, length, yd, Conversion<foot>(3.0)); \
	DerivedUnitTypeWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0); \
	DerivedUnitType(metric_ton, mass, t, Conversion<kilogram>(1000.0)); \
	DerivedUnitType(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0)); \
	DerivedUnitType(long_ton, mass, ln_t, Conversion<pound>(2240.0)); \
	DerivedUnitType(short_ton, mass, sh_t, Conversion<pound>(2000.0)); \
	DerivedUnitType(stone, mass, st, Conversion<pound>(14.0)); \
	DerivedUnitType(ounce, mass, oz, Conversion<pound>(1.0 / 16.0)); \
	DerivedUnitType(carat, mass, ct, Conversion<milligram>(200.0)); \
	DerivedUnitType(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(second, time, s, 1.0); \
	DerivedUnitType(minute, time, min, Conversion<second>(60.0)); \
	DerivedUnitType(hour, time, hr, Conversion<minute>(60.0)); \
	DerivedUnitType(day, time, d, Conversion<hour>(24.0)); \
	DerivedUnitType(week, time, wk, Conversion<day>(7.0)); \
	DerivedUnitType(year, time, yr, Conversion<day>(365.25)); /* includes additional day for every 4 years */ \
	DerivedUnitType(month, time, mnth, Conversion<year>(1.0 / 12.0)); \
	DerivedUnitType(julian_year, time, a_j, Conversion<second>(31557600.0)); \
	DerivedUnitType(gregorian_year, time, a_g, Conversion<second>(31556952.0)); \
	DerivedUnitTypeWithMetricPrefixes(ampere, current, A, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(hertz, frequency, Hz, 1.0); \
	DerivedUnitType(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0)); \
    DerivedUnitType(inches_per_day, velocity, ipd, Conversion<inch>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0))); \
	DerivedUnitType(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0))); \
	DerivedUnitType(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0)); \
	DerivedUnitTypeWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0)); \
	DerivedUnitType(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0)); \
	DerivedUnitType(dyne, force, dyn, Conversion<newton>(1.0 / 100000.0)); \
	DerivedUnitType(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0)); \
	DerivedUnitType(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(pascals, pressure, Pa, 1.0); \
	DerivedUnitType(bar, pressure, bar, Conversion<kilopascals>(100.0)); \
	DerivedUnitType(atmosphere, pressure, atm, Conversion<pascals>(101325.0)); \
	DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0))); \
	DerivedUnitType(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0))); \
	DerivedUnitType(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0)); \
	DerivedUnitType(coulomb, charge, C, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt, power, W, 1.0); \
	DerivedUnitType(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0)); \
	DerivedUnitType(joule, energy, J, 1.0); \
	DerivedUnitType(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0)); \
	DerivedUnitType(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0)); \
	DerivedUnitType(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0)); \
	DerivedUnitType(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0)); \
	DerivedUnitType(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0)); \
	DerivedUnitType(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0)); \
	DerivedUnitType(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0)); \
	DerivedUnitType(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(volt, voltage, V, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0); \
	DerivedUnitType(siemens, conductance, S, 1.0);  \
	DerivedUnitType(square_meter, area, sq_m, 1.0); \
	DerivedUnitType(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0)); \
	DerivedUnitType(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0)); \
	DerivedUnitType(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0)); \
	DerivedUnitType(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0)); \
	DerivedUnitType(hectare, area, ha, Conversion<square_meter>(1000.0)); \
	DerivedUnitType(acre, area, acre, Conversion<square_foot>(43560.0)); \
	DerivedUnitType(cubic_meter, volume, cu_m, 1.0); \
	DerivedUnitType(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0))); \
	DerivedUnitType(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0))); \
	DerivedUnitType(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0))); \
	DerivedUnitType(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0))); \
	DerivedUnitType(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0))); \
	DerivedUnitType(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0)); \
	DerivedUnitType(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0)); \
	DerivedUnitType(million_gallon, volume, MG, Conversion<gallon>(1.0) * CalculateMetricPrefixV(mega)); \
	DerivedUnitType(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0) * CalculateMetricPrefixV(mega)); \
	DerivedUnitType(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0)); \
	DerivedUnitType(quart, volume, qt, Conversion<gallon>(0.25)); \
	DerivedUnitType(pint, volume, pt, Conversion<quart>(0.5)); \
	DerivedUnitType(cup, volume, c, Conversion<pint>(0.5)); \
	DerivedUnitType(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125)); \
	DerivedUnitType(barrel, volume, bl, Conversion<gallon>(42.0)); \
	DerivedUnitType(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0)); \
	DerivedUnitType(cord, volume, cord, Conversion<cubic_foot>(128.0)); \
	DerivedUnitType(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5)); \
	DerivedUnitType(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0)); \
	DerivedUnitType(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0)); \
	DerivedUnitType(dash, volume, dash, Conversion<pinch>(1.0 / 2.0)); \
	DerivedUnitType(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0)); \
	DerivedUnitType(fifth, volume, fifth, Conversion<gallon>(0.2)); \
	DerivedUnitType(dram, volume, dr, Conversion<fluid_ounce>(0.125)); \
	DerivedUnitType(gill, volume, gi, Conversion<fluid_ounce>(4.0)); \
	DerivedUnitType(peck, volume, pk, Conversion<bushel>(0.25)); \
	DerivedUnitType(sack, volume, sacks, Conversion<bushel>(3.0)); \
	DerivedUnitType(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0)); \
	DerivedUnitType(strike, volume, strikes, Conversion<bushel>(2.0)); \
	DerivedUnitType(gram_per_second, fillrate, gs, 1.0 / 1000.0); \
	DerivedUnitType(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(cubic_meter_per_second, flowrate, cms, 1.0); \
	DerivedUnitType(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0); \
	DerivedUnitType(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0)); \
	DerivedUnitType(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0)); \
	DerivedUnitType(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0)); \
	DerivedUnitType(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0)); \
	DerivedUnitType(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0)); \
	DerivedUnitType(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0)); \
	DerivedUnitType(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0)); \
	DerivedUnitType(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0)); \
	DerivedUnitType(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0)); \
	DerivedUnitType(kelvin, temperature, K, 1.0); \
    DerivedUnitType(radian, angle, rad, 1.0); \
    DerivedUnitType(degree, angle, deg, Conversion<radian>(3.141592653589793238462643383279502884197169399375105820974944 / 180.0));

#define DerivedUnitType(type, category, abbreviation, Ratio) \
	class type final : public value { \
    private: \
        static package unique_pkg(); \
	public: \
		constexpr static double conversion_ratio{ Ratio }; \
        type() : value(unique_pkg()) {}; \
        type(float rhs) : value(unique_pkg()) { packed.m_bits2.val = rhs; }; \
        type(value const& rhs) : value(unique_pkg()) { this->TrySetTo(rhs); }; \
        type(value&& rhs) : value(unique_pkg()) { this->TrySetTo(std::move(rhs)); }; \
        ~type() = default; \
	};

#define DerivedUnitTypeWithMetricPrefix(type, prefix) \
    class prefix ## type final : public value { \
    private: \
        static package unique_pkg(); \
	public: \
		constexpr static double conversion_ratio{ type::conversion_ratio * ((double)std::prefix::num / (double)std::prefix::den) }; \
        prefix ## type() : value(unique_pkg()) {}; \
        prefix ## type(float rhs) : value(unique_pkg()) { packed.m_bits2.val = rhs; }; \
        prefix ## type(value const& rhs) : value(unique_pkg()) { this->TrySetTo(rhs); }; \
        prefix ## type(value&& rhs) : value(unique_pkg()) { this->TrySetTo(std::move(rhs)); }; \
        ~prefix ## type() = default; \
	};

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
	DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, femto); \
	DerivedUnitTypeWithMetricPrefix(type, pico); \
	DerivedUnitTypeWithMetricPrefix(type, nano); \
	DerivedUnitTypeWithMetricPrefix(type, micro); \
	DerivedUnitTypeWithMetricPrefix(type, milli); \
	DerivedUnitTypeWithMetricPrefix(type, centi); \
	DerivedUnitTypeWithMetricPrefix(type, deci); \
	DerivedUnitTypeWithMetricPrefix(type, deca); \
	DerivedUnitTypeWithMetricPrefix(type, hecto); \
	DerivedUnitTypeWithMetricPrefix(type, kilo); \
	DerivedUnitTypeWithMetricPrefix(type, mega); \
	DerivedUnitTypeWithMetricPrefix(type, giga); \
	DerivedUnitTypeWithMetricPrefix(type, tera); \
	DerivedUnitTypeWithMetricPrefix(type, peta)

    DerivedUnitList; // this loops through the definitions for DerivedUnitTypeWithMetricPrefixes() and DerivedUnitType() for all units. Change thosse macro definitions to change the implimentations. 

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitType
#undef CalculateMetricPrefixV

    /* Unit Literals (e.g. 1_ft, 1_gpm, etc.) */
    namespace literals {};
};

#define DerivedUnitType(type, category, abbreviation, ratio) \
    namespace std { template<> class numeric_limits<GL::type> { public: \
	    static constexpr double min() { return std::numeric_limits<float>::min(); } \
	    static constexpr double max() { return std::numeric_limits<float>::max(); } \
	    static constexpr double lowest() { return std::numeric_limits<float>::lowest(); } \
	    static constexpr bool is_integer = std::numeric_limits<float>::is_integer; \
	    static constexpr bool is_signed = std::numeric_limits<float>::is_signed; }; \
    }; namespace GL{ namespace literals { \
	        __forceinline static auto operator""_ ## abbreviation (long double d) { return GL::type(static_cast<float>(d)); } \
	        __forceinline static auto operator""_ ## abbreviation (unsigned long long d) { return GL::type(static_cast<float>(d)); } \
    } };

#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, p_abbreviation) \
    DerivedUnitType(prefix ## type, category, p_abbreviation ## abbreviation, ratio);

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
	DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, pico, p); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, nano, n); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, micro, u); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, milli, m); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, centi, c); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deci, d); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deca, da); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, hecto, h); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, kilo, k); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, mega, M); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, giga, G); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, tera, T); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, peta, P)

    DerivedUnitList;

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitType

    namespace GL {

        class constants {
        public:
            /* PI (equal to 180 degrees) */
            static radian					
                pi() {
                return 3.141592653589793238462643383279502884197169399375105820974944f;
            };

            /* speed of light in a vacuum (m/s) */
            static meters_per_second		    
                c() {
                return value(299792458.0f);
            };

            /* ( m^3 / (kg * s^2) ) */
            static value				        
                G() {
                using namespace literals;
                return 6.67408e-11_cu_m / (1_kg * 1_s * 1_s);
            };

            /* acceleration due to gravity ( m/s^2 ) */
            static meters_per_second_squared	
                g() {
                return value(9.8067f);
            };

            /* density of water ( kg/m^3 ) */
            static kilograms_per_cubic_meter 
                d() {
                return value(998.57f);
            };

            // Kinematic viscosity of water @ 20 deg C (sq ft/sec)
            static value
                viscosity() {
                using namespace literals;
                return 1.1E-5_sq_ft / 1_s; 
            };

        };

        __forceinline value value::sin() const {
            return std::sin(GL::radian(*this).operator float());                
        };
        __forceinline value value::cos() const {
            return std::cos(GL::radian(*this).operator float());
        };
        __forceinline value value::tan() const {
            return std::tan(GL::radian(*this).operator float());
        };
        __forceinline value value::asin() const {
            return std::asin(GL::radian(*this).operator float());
        };
        __forceinline value value::acos() const {
            return std::acos(GL::radian(*this).operator float());
        };
        __forceinline value value::atan() const {
            return std::atan(GL::radian(*this).operator float());
        };
    };

#endif



