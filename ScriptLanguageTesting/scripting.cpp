#pragma once
#include "scripting.h"

namespace GL {
	namespace scope {
        std::vector<GL::type> impl::RootScope::all_convertable_types() const {
            std::unordered_set<GL::type> AllTypes;
            std::unordered_map<GL::type, std::unordered_map<GL::type, GL::Proxy_Function const*>> AllConversions; // all built-in conversions, e.g. int->float, float->double, etc.
            (void)this->constructors.for_each([&AllConversions, &AllTypes](GL::Proxy_Function const& func)->bool {
                AllTypes.insert(func->m_signature.returns_m);
                for (auto& x : func->m_signature.argument_types_m) AllTypes.insert(x);

                if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                    if (func->m_signature.argument_types_m.size() == 1) {
                        auto& from = func->m_signature.argument_types_m[0];
                        auto& to = func->m_signature.returns_m;
                        AllConversions[from][to] = &func;
                    }
                }
                return false;
            });

            std::vector<GL::type> out;
            for (auto& x : AllTypes) { 
                out.push_back(x);
            }
            return out;
        };
		void impl::RootScope::perform_builtins() {
            // basic numbers
            if (1) {
#define add_a(type) \
                this->make_class(GL::type_of< type >()).add_function(GL::make_callable(GL::type_of< type >().name(), []() -> type { return 0; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< type >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type &>() = rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type &>() }, { "rhs", GL::type_of<type const&>() }}, GL::type_of< type& >())); \
                this->make_class(GL::type_of< bool >()).add_function(GL::make_converter<type, bool>()); \
                this->make_class(GL::type_of< char >()).add_function(GL::make_converter<type, char>()); \
                this->make_class(GL::type_of< unsigned char >()).add_function(GL::make_converter<type, unsigned char>()); \
                this->make_class(GL::type_of< int >()).add_function(GL::make_converter<type, int>()); \
                this->make_class(GL::type_of< long >()).add_function(GL::make_converter<type, long>()); \
                this->make_class(GL::type_of< long long >()).add_function(GL::make_converter<type, long long>()); \
                this->make_class(GL::type_of< size_t >()).add_function(GL::make_converter<type, size_t>()); \
                this->make_class(GL::type_of< float >()).add_function(GL::make_converter<type, float>()); \
                this->make_class(GL::type_of< double >()).add_function(GL::make_converter<type, double>()); \
                this->make_class(GL::type_of< long double >()).add_function(GL::make_converter<type, long double>()); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("==", [](type const& lhs, type const& rhs) -> bool { return lhs == rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("!=", [](type const& lhs, type const& rhs) -> bool { return lhs != rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable(">", [](type const& lhs, type const& rhs) -> bool { return lhs > rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("<", [](type const& lhs, type const& rhs) -> bool { return lhs < rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable(">=", [](type const& lhs, type const& rhs) -> bool { return lhs >= rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("<=", [](type const& lhs, type const& rhs) -> bool { return lhs <= rhs; }, GL::function_signature::Constant))

#define add_c(type) \
                add_a(type); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("+=", [](GL::any::fast_any const& lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() += rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("-=", [](GL::any::fast_any const& lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() -= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("*=", [](GL::any::fast_any const& lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() *= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("/=", [](GL::any::fast_any const& lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() /= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("++", [](GL::any::fast_any const& lhs) -> GL::any::fast_any { ++lhs.cast< type& >(); return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("--", [](GL::any::fast_any const& lhs) -> GL::any::fast_any { --lhs.cast< type& >(); return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() } }, GL::type_of< type& >()))

#define add_d(type) \
                add_c(type); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("-", [](type const& lhs) -> type { \
                    if constexpr (std::is_unsigned_v< type > || std::is_same_v< type, size_t >) { return type{ 0 }; } \
                    else { return -lhs; } \
                }, GL::function_signature::Constant))

                add_a(bool);
                /*this->make_class(GL::type_of< bool >()).*/add_function(GL::make_callable("!", [](bool const& lhs) -> bool { return !lhs; }, GL::function_signature::Constant));
                add_d(char);
                add_c(unsigned char);
                add_d(int);
                add_d(long);
                add_d(long long);
                add_c(size_t);
                add_d(float);
                add_d(double);
                add_d(long double);
#undef add_a
#undef add_c
#undef add_d

                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("|=", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() |= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("|", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs | rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("&=", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() &= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("&", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs & rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("^=", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() ^= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("^", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs ^ rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("~", [](size_t const& lhs) -> size_t { return ~lhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("<<=", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() <<= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("<<", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs << rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable(">>=", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() >>= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable(">>", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs >> rhs; }, GL::function_signature::Constant));
            }

            // units 
            if (1) {
#define DerivedUnitType(type, category, abbreviation, Ratio) \
        this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable(GL::type_of< type >().name(), []() -> type { return type{ 0.0f }; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< type >())); \
        this->make_class(GL::type_of< GL::value >()).add_function(GL::make_converter<GL::type, GL::value>()); \
        this->make_class(GL::type_of< GL::type >()).add_function(GL::make_converter<GL::value, GL::type>())

#define DerivedUnitTypeWithMetricPrefix(type, prefix) \
        DerivedUnitType(prefix ## type, 0, 0, 0)

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

                DerivedUnitType(kelvin, 0, 0, 0);
                DerivedUnitType(fahrenheit, 0, 0, 0);

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitType
#undef CalculateMetricPrefixV

                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable(GL::type_of< GL::value >().name(), []() -> GL::value { return GL::value{ 0.0f }; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< GL::value >()));
                this->make_class(GL::type_of< float >()).add_function(GL::make_converter<GL::value, float>());
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_converter<float, GL::value>());
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() = rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("==", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs == rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("!=", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs != rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable(">", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs > rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("<", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs < rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable(">=", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs >= rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("<=", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs <= rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("+", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs + rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("-", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs - rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("*", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs * rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("/", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs / rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("+=", [](GL::any::fast_any const& lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() += rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("-=", [](GL::any::fast_any const& lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() -= rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("*=", [](GL::any::fast_any const& lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() *= rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("/=", [](GL::any::fast_any const& lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() /= rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));

                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("++", [](GL::any::fast_any const& lhs) -> GL::any::fast_any { ++lhs.cast<GL::value&>(); return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("--", [](GL::any::fast_any const& lhs) -> GL::any::fast_any { --lhs.cast<GL::value&>(); return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("-", [](GL::value const& lhs) -> GL::value { return -lhs; }, GL::function_signature::Async | GL::function_signature::Constant));

                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::pow));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::pow_value));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sqrt));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::rsqrt));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::rsqrt_fast));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::floor));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::ceiling));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::abs));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::clamp));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable("round", [](GL::value const& lhs, float magnitude) -> GL::value { return lhs.round(magnitude); },
                    GL::function_signature::Async | GL::function_signature::Constant, 
                    { GL::any{ 1.0f } }, { {"lhs", GL::type_of<GL::value const&>() }, {"magnitude", GL::type_of<float>() } }, GL::type_of<GL::value>() 
                ));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::max));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::min));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log2));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log10));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log1p));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::exp));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::exp2));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::expm1));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sign));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::mod));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::wrap));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::lerp));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sin));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sin_fast));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::cos));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::tan));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::asin));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::acos));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::atan));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable("to_string", [](GL::value const& lhs) -> GL::string {
                    GL::string Num = std::to_string((float)lhs);
                    return Num.remove_trailing('0').remove_trailing('.') + " " + lhs.abbreviation();
                }, GL::function_signature::Async | GL::function_signature::Constant));
            }

            // types
            if (1) {
                add_function(GL::make_callable("type_of", [](GL::any const& any_type) -> GL::type { return any_type.m_casted_type; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("size", [](GL::type const& any_type) -> size_t { return any_type.size(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                add_function(GL::make_callable("type_name", [](GL::any const& any_type) -> GL::string { return any_type.m_casted_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("type_name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                add_function(GL::make_callable("size_of", [](GL::any const& any_type) -> size_t { return any_type.m_casted_type.size(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_void", [](GL::type const& any_type) -> bool { return any_type.is_void(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_any", [](GL::type const& any_type) -> bool { return any_type.is_any(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_const", [](GL::type const& any_type) -> bool { return any_type.is_const(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_cpp_type", [](GL::type const& any_type) -> bool { return any_type.is_cpp_type(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_ref", [](GL::type const& any_type) -> bool { return any_type.is_ref(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_temp", [](GL::type const& any_type) -> bool { return any_type.is_temp(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_const_ref", [](GL::type const& any_type) -> bool { return any_type.is_const_ref(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_base", [](GL::type const& any_type) -> bool { return any_type.is_base(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_base_of", [](GL::type const& a, GL::type const& b) -> bool { return a.is_base_of(b); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("is_derived_from", [](GL::type const& a, GL::type const& b) -> bool { return a.is_derived_from(b); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("get_hash", [](GL::type const& a) -> size_t { return a.get_hash(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("get_base_hash", [](GL::type const& a) -> size_t { return a.get_base_hash(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
            }

            // var (generic "variable" container, for wrapping assignment of any type within the script language. Effectively the scripting language's version of 'any')
            if (1) {
                auto& var_class = this->make_class(GL::type_of< GL::var >());
                // default constructor
                var_class.add_function(GL::make_callable(GL::type_of< GL::var >().name(), []() -> GL::var { return {}; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< GL::var >()));
                // copy constructor
                var_class.add_function(GL::make_callable(GL::type_of< GL::var >().name(), [](GL::var const& rhs) -> GL::var { return rhs; }, GL::function_signature::Constructor | GL::function_signature::Async));
                // template constructor, create a var from anything
                var_class.add_function(GL::make_callable(GL::type_of< GL::var >().name(), [](GL::any::fast_any const& rhs) -> GL::var {
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        return rhs.cast<GL::var&>();
                    }
                    else {
                        return GL::var(GL::make_shared<GL::any>(rhs));
                    }
                    
                }, GL::function_signature::Constructor | GL::function_signature::Async, {}, { { "rhs", GL::type_of<GL::any>() } }, GL::type_of<GL::var>()));
                // assignment operator. Anything can be assigned to an empty var object.
                this->add_function(GL::make_callable("=", [](GL::any::fast_any& lhs, GL::any::fast_any const& rhs) -> GL::any::fast_any {
                    
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        lhs.cast<GL::var&>() = rhs.cast<GL::var&>();                        
                    }
                    else {
                        lhs.cast<GL::var&>() = GL::var(GL::make_shared<GL::any>(rhs));
                    }
                    GL::any::fast_any out = lhs;
                    out.m_casted_type = rhs.m_casted_type | GL::type::Reference;
                    return out;
                }, GL::function_signature::Async, {}, { { "lhs",GL::type_of<GL::var&>() }, { "rhs", GL::type_of<GL::any>() } }/*, GL::type_of<GL::var&>()*/));
                // forced assignment operator. Anything can be assigned to an var object using this operator. If something was already assigned, it is over-written. 
                this->add_function(GL::make_callable(":=", [](GL::any::fast_any& lhs, GL::any::fast_any const& rhs) -> GL::any::fast_any {
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        lhs.cast<GL::var&>() = rhs.cast<GL::var&>();
                    }
                    else {
                        lhs.cast<GL::var&>() = GL::var(GL::make_shared<GL::any>(rhs));
                    }
                    GL::any::fast_any out = lhs;
                    out.m_casted_type = rhs.m_casted_type | GL::type::Reference;
                    return out;
                }, GL::function_signature::Async, {}, { { "lhs",GL::type_of<GL::var&>() }, { "rhs", GL::type_of<GL::any>() } }/*, GL::type_of<GL::var&>()*/));                
                // boolean test for vars, to ensure they are "valid"
                this->make_class(GL::type_of< bool >()).add_function(GL::make_callable(GL::type_of< bool >().name(), [](GL::var const& rhs) -> bool {
                    return rhs.get_type().get_base_hash() != GL::type_of<GL::var>().get_base_hash();
                }));
                // boolean test for vars, to ensure they are "valid"
                var_class.add_function(GL::make_callable("valid", [](GL::var const& rhs) -> bool { return rhs.get_type().get_base_hash() != GL::type_of<GL::var>().get_base_hash(); }, GL::function_signature::Async));
            }






		};
        void impl::RootScope::preload_conversions() {
            for (auto& _type : all_convertable_types()) {
                (void)this->get_converters().try_get_converter(_type, _type);
            }
        };

	}
}