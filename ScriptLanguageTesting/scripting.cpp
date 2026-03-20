#pragma once
#include "scripting.h"
#include "../GpuProgramming/matrix.h"

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
            // type-casting or language support
            if (1) {
                // this->add_function(GL::make_callable("reference_cast", [](GL::any::fast_any const& any_type) -> GL::any::fast_any { return any_type | GL::type::Reference; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                // this->add_function(GL::make_callable("reinterpret_cast", [](GL::any::fast_any from, GL::type const& to_type) -> GL::any::fast_any { from.m_casted_type = to_type; return from; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                // this->add_function(GL::make_callable("const_cast", [](GL::any::fast_any const& any_type) -> GL::any::fast_any { return any_type - GL::type::Const; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
            }

            // basic numbers
            if (1) {
#define add_a(type) \
                this->make_class(GL::type_of< type >()).add_function(GL::make_callable(GL::type_of< type >().name(), []() -> type { return 0; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< type >())); \
                add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type &>() = rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type &>() }, { "rhs", GL::type_of<type const&>() }}, GL::type_of< type& >())); \
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
                add_c(unsigned int);
                add_d(long);
                add_c(unsigned long);
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
                using class_t = GL::type;
                auto& Class = this->make_class(GL::type_of<class_t>());
                /* default constructor */ Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return {}; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< class_t >()));
                /* copy constructor */    Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor | GL::function_signature::Async));
                /* assignment operator */ this->add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));

                this->add_function(GL::make_callable("type_of", [](GL::any::fast_any const& any_type) -> GL::type { return any_type.m_casted_type; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("size", [](GL::type const& any_type) -> size_t { return any_type.size(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->add_function(GL::make_callable("type_name", [](GL::any::fast_any const& any_type) -> GL::string { 
                    return any_type.m_casted_type.name(); 
                }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("type_name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->add_function(GL::make_callable("size_of", [](GL::any::fast_any const& any_type) -> size_t { return any_type.m_casted_type.size(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_void", [](GL::type const& any_type) -> bool { return any_type.is_void(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_any", [](GL::type const& any_type) -> bool { return any_type.is_any(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_const", [](GL::type const& any_type) -> bool { return any_type.is_const(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_cpp_type", [](GL::type const& any_type) -> bool { return any_type.is_cpp_type(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_ref", [](GL::type const& any_type) -> bool { return any_type.is_ref(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_temp", [](GL::type const& any_type) -> bool { return any_type.is_temp(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_const_ref", [](GL::type const& any_type) -> bool { return any_type.is_const_ref(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_base", [](GL::type const& any_type) -> bool { return any_type.is_base(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_base_of", [](GL::type const& a, GL::type const& b) -> bool { return a.is_base_of(b); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_derived_from", [](GL::type const& a, GL::type const& b) -> bool { return a.is_derived_from(b); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("get_hash", [](GL::type const& a) -> size_t { return a.get_hash(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("get_base_hash", [](GL::type const& a) -> size_t { return a.get_base_hash(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
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
                    GL::any::fast_any& out = lhs;
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        lhs.cast<GL::var&>() = rhs.cast<GL::var&>();   
                        out.m_casted_type = rhs.m_casted_type | GL::type::Reference;
                    }
                    else {
                        lhs.cast<GL::var&>() = GL::var(GL::make_shared<GL::any>(rhs));
                        
                    }
                    return out;
                }, GL::function_signature::Async, {}, { { "lhs",GL::type_of<GL::var&>() }, { "rhs", GL::type_of<GL::any>() } }/*, GL::type_of<GL::var&>()*/));
                // forced assignment operator. Anything can be assigned to an var object using this operator. If something was already assigned, it is over-written. 
                this->add_function(GL::make_callable(":=", [](GL::any::fast_any& lhs, GL::any::fast_any const& rhs) -> GL::any::fast_any {
                    GL::any::fast_any& out = lhs; 
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        lhs.cast<GL::var&>() = rhs.cast<GL::var&>();
                        out.m_casted_type = rhs.m_casted_type | GL::type::Reference;
                    }
                    else {
                        lhs.cast<GL::var&>() = GL::var(GL::make_shared<GL::any>(rhs));
                        out.m_casted_type = rhs.m_casted_type;
                    }
                    return out;
                }, GL::function_signature::Async, {}, { { "lhs",GL::type_of<GL::var&>() }, { "rhs", GL::type_of<GL::any>() } }, GL::type_of<GL::var&>()));                
                // boolean test for vars, to ensure they are "valid". Note that this may actually call the conversion on the stored object, so this has been cut (for now)
                //this->make_class(GL::type_of< bool >()).add_function(GL::make_callable(GL::type_of< bool >().name(), [](GL::var const& rhs) -> bool {
                //    return rhs.get_type().get_base_hash() != GL::type_of<GL::var>().get_base_hash();
                //}, GL::function_signature::Explicit | GL::function_signature::Constructor, {}, {}, GL::type_of<bool>()));
                // boolean test for vars, to ensure they are "valid"
                var_class.add_function(GL::make_callable("valid", [](GL::var const& rhs) -> bool { return rhs.get_type().get_base_hash() != GL::type_of<GL::var>().get_base_hash(); }, GL::function_signature::Async));
            }

            // std::string support
            if (1) {
                using class_t = std::string;
                auto& std_ns = this->make_namespace("std");
                auto& Class = std_ns.make_class(GL::type_of<class_t>());
                // default constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type));
                // copy constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor));
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GL::string const& rhs) -> class_t { return rhs.to_string(); }, GL::function_signature::Constructor));
                // assignment operator
                this->add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));                               
            }

            // string functions
            if (1) {
                using class_t = GL::string;
                auto& Class = this->make_class(GL::type_of<class_t>());
                // default constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type));
                // copy constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor));
                Class.add_function(GL::make_callable(Class.this_type.name(), [](std::string const& rhs) -> class_t { return class_t((std::string)rhs); }, GL::function_signature::Constructor));
                // assignment operator
                this->add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                this->add_function(GL::make_callable("==", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs == rhs; }));
                this->add_function(GL::make_callable("!=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs != rhs; }));
                this->add_function(GL::make_callable(">", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs > rhs; }));
                this->add_function(GL::make_callable(">=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs >= rhs; }));
                this->add_function(GL::make_callable("<", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs < rhs; }));
                this->add_function(GL::make_callable("<=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs <= rhs; }));
                this->add_function(GL::make_callable("+", [](class_t const& lhs, class_t const& rhs) -> class_t { return lhs + rhs; }));

                Class.add_function(GL::decl_func(&class_t::add_to_delim));
                Class.add_function(GL::decl_func(&class_t::at));
                Class.add_function(GL::decl_func(&class_t::back));
                Class.add_function(GL::decl_func(&class_t::begins_with));
                Class.add_function(GL::make_callable("distance", [](class_t const& lhs, class_t const& rhs, bool case_sensitive) -> size_t { return lhs.distance(rhs, case_sensitive); }, GL::function_signature::Async | GL::function_signature::Constant, { true }, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const}, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const }, { "case_sensitive", GL::type_of<bool const&>() } }, GL::type_of<size_t>()));
                Class.add_function(GL::decl_func(&class_t::empty));
                Class.add_function(GL::decl_func(&class_t::empty_string));
                Class.add_function(GL::decl_func(&class_t::ends_with));
                Class.add_function(GL::make_callable("find", [](class_t const& lhs, class_t const& rhs, bool case_sensitive, long long start, long long end) -> size_t { return lhs.find(rhs, case_sensitive, start, end); }, { true, 0ll, -1ll }));
                Class.add_function(GL::decl_func(&class_t::front));
                Class.add_function(GL::decl_func(&class_t::hash));
                Class.add_function(GL::decl_func(&class_t::has_lower));
                Class.add_function(GL::decl_func(&class_t::has_upper));
                Class.add_function(GL::decl_func(&class_t::left));
                // Class.add_function(GL::decl_func(&class_t::left_and_right_of)); // needs conversion to std::pair<var, var> or equivalent...
                // Class.add_function(GL::decl_func(&class_t::left_and_right_of_last)); // needs conversion to std::pair<var, var> or equivalent...
                Class.add_function(GL::decl_func(&class_t::left_of));
                Class.add_function(GL::decl_func(&class_t::left_of_last));
                Class.add_function(GL::decl_func(&class_t::length));
                Class.add_function(GL::decl_func(&class_t::namespace_colons));
                Class.insert_object_here("npos", GL::any::ref(class_t::npos));
                Class.add_function(GL::decl_func(&class_t::remove_leading));
                Class.add_function(GL::decl_func(&class_t::remove_leading_and_trailing));                
                Class.add_function(GL::make_callable("remove_prefix", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_prefix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::make_callable("remove_prefix", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_prefix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<size_t const&>() } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::make_callable("remove_suffix", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_suffix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::make_callable("remove_suffix", [](GL::any::fast_any const& lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_suffix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<size_t const&>() } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::decl_func(&class_t::remove_trailing));
                Class.add_function(GL::decl_func(&class_t::replace));
                Class.add_function(GL::decl_func(&class_t::rfind));
                Class.add_function(GL::decl_func(&class_t::right));
                Class.add_function(GL::decl_func(&class_t::right_of));
                Class.add_function(GL::decl_func(&class_t::right_of_last));
                Class.add_function(GL::decl_func(&class_t::size));
                // Class.add_function(GL::decl_func(&class_t::split)); // needs conversion to std::vector<var> or equivalent...
                Class.add_function(GL::make_callable("substr", [](class_t const& lhs, size_t _off, size_t _count) -> class_t { return lhs.substr(_off, _count); }, GL::function_signature::Async | GL::function_signature::Constant, { 0ull, class_t::npos }, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const}, { "_Off", GL::type_of<size_t const&>() }, { "_Count", GL::type_of<size_t const&>() } }, Class.this_type));
                Class.add_function(GL::decl_func(&class_t::to_lower));                
                Class.add_function(GL::decl_func(&class_t::to_number));
                Class.add_function(GL::decl_func(&class_t::to_upper));
            }

            // GPU-accelerated arrays
            if (1) {
                // initialize the classes and their names
#define add_matrix(TypeT) if (1) { \
                GL::type_of<GPU::matrix<TypeT>>().try_update_name(GL::type_of<TypeT>().name() + "_matrix"); \
                using class_t = GPU::matrix<TypeT>; \
                auto& Class = this->make_class(GL::type_of<class_t>()); \
                }

                add_matrix(float);
                add_matrix(unsigned long);
                add_matrix(long);
                add_matrix(int);
                add_matrix(unsigned int);
                add_matrix(char);
                add_matrix(unsigned char);
#undef add_matrix
                GL::type_of<GPU::matrix_kernel<float>>().try_update_name("matrix_kernel"); 
                this->make_class(GL::type_of<GPU::matrix_kernel<float>>());

#define add_matrix(TypeT) if (1) { \
                using class_t = GPU::matrix<TypeT>; \
                auto& Class = this->make_class(GL::type_of<class_t>()); \
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type)); \
                this->add_function(GL::make_callable("=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<float> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<unsigned long> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<long> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<int> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<unsigned int> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<char> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<unsigned char> const& rhs) -> class_t { return rhs.cast<TypeT>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::decl_func(&class_t::abs)); \
                Class.add_function(GL::decl_func(&class_t::acos)); \
                Class.add_function(GL::decl_func(&class_t::acosh)); \
                Class.add_function(GL::decl_func(&class_t::adjoint)); \
                Class.add_function(GL::decl_func(&class_t::ASCII)); \
                Class.add_function(GL::decl_func(&class_t::asin)); \
                Class.add_function(GL::decl_func(&class_t::asinh)); \
                Class.add_function(GL::decl_func(&class_t::atan)); \
                Class.add_function(GL::decl_func(&class_t::atanh)); \
                Class.add_function(GL::decl_func(&class_t::avg)); \
                Class.add_function(GL::decl_func(&class_t::binomial_search_smallest_gre)); \
                Class.add_function(GL::decl_func(&class_t::ceil)); \
                Class.add_function(GL::decl_func(&class_t::cofactor)); \
                Class.add_function(GL::decl_func(&class_t::constant)); \
                Class.add_function(GL::make_callable("convolve", [](class_t const& parent, GPU::matrix_kernel<float> const& rhs) -> class_t { return parent.convolve(rhs); })); \
                Class.add_function(GL::decl_func(&class_t::cos)); \
                Class.add_function(GL::decl_func(&class_t::cosh)); \
                Class.add_function(GL::decl_func(&class_t::determinant)); \
                Class.add_function(GL::decl_func(&class_t::diagonal)); \
                Class.add_function(GL::decl_func(&class_t::doublesize)); \
                Class.add_function(GL::decl_func(&class_t::exp)); \
                Class.add_function(GL::decl_func(&class_t::exp10)); \
                Class.add_function(GL::decl_func(&class_t::exp2)); \
                Class.add_function(GL::decl_func(&class_t::expm1)); \
                Class.add_function(GL::decl_func(&class_t::floor)); \
                Class.add_function(GL::decl_func(&class_t::fma)); \
                Class.add_function(GL::decl_func(&class_t::grow_by_wrapping)); \
                Class.add_function(GL::make_callable("guassian_kernel", [](unsigned int X, unsigned int Y) { return GPU::matrix<float>::guassian_kernel(X, Y); })); \
                Class.add_function(GL::decl_func(&class_t::halfsize)); \
                Class.add_function(GL::decl_func(&class_t::identity)); \
                Class.add_function(GL::decl_func(&class_t::inverse)); \
                Class.add_function(GL::decl_func(&class_t::is_colinear)); \
                Class.add_function(GL::decl_func(&class_t::join)); \
                Class.add_function(GL::decl_func(&class_t::lgamma)); \
                Class.add_function(GL::decl_func(&class_t::linear)); \
                Class.add_function(GL::decl_func(&class_t::log)); \
                Class.add_function(GL::decl_func(&class_t::log10)); \
                Class.add_function(GL::decl_func(&class_t::log1p)); \
                Class.add_function(GL::decl_func(&class_t::log2)); \
                Class.add_function(GL::decl_func(&class_t::make_square)); \
                Class.add_function(GL::decl_func(&class_t::matrix_multiply)); \
                Class.add_function(GL::make_callable("max", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.max(rhs); })); \
                Class.add_function(GL::make_callable("max", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.max(rhs); })); \
                Class.add_function(GL::make_callable("min", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.min(rhs); })); \
                Class.add_function(GL::make_callable("min", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.min(rhs); })); \
                Class.add_function(GL::make_callable("mod", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.mod(rhs); })); \
                Class.add_function(GL::make_callable("mod", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.mod(rhs); })); \
                Class.add_function(GL::make_callable("pow", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.pow(rhs); })); \
                Class.add_function(GL::make_callable("pow", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.pow(rhs); })); \
                Class.add_function(GL::make_callable("pown", [](class_t const& parent, GPU::matrix<int> const& rhs) -> class_t { return parent.pown(rhs); })); \
                Class.add_function(GL::make_callable("pown", [](class_t const& parent, int const& rhs) -> class_t { return parent.pown(rhs); })); \
                Class.add_function(GL::decl_func(&class_t::quadruplesize)); \
                Class.add_function(GL::decl_func(&class_t::quartersize)); \
                Class.add_function(GL::decl_func(&class_t::random)); \
                Class.add_function(GL::decl_func(&class_t::random_between)); \
                Class.add_function(GL::decl_func(&class_t::resample)); \
                Class.add_function(GL::make_callable("resize", [](class_t const& lhs, unsigned int x, unsigned int y, unsigned int z) { return lhs.resize(x, y, z); })); \
                Class.add_function(GL::decl_func(&class_t::resize_stretch)); \
                Class.add_function(GL::decl_func(&class_t::round)); \
                Class.add_function(GL::decl_func(&class_t::row)); \
                Class.add_function(GL::decl_func(&class_t::sin)); \
                Class.add_function(GL::decl_func(&class_t::sinh)); \
                Class.add_function(GL::make_callable("size", [](class_t const& lhs) { return lhs.size(); })); \
                Class.add_function(GL::make_callable("size", [](class_t const& lhs, unsigned int d) { return lhs.size(d); })); \
                Class.add_function(GL::decl_func(&class_t::sqrt)); \
                Class.add_function(GL::decl_func(&class_t::subsample_1D)); \
                Class.add_function(GL::decl_func(&class_t::subsample_pat)); \
                Class.add_function(GL::decl_func(&class_t::sum)); \
                Class.add_function(GL::decl_func(&class_t::tan)); \
                Class.add_function(GL::decl_func(&class_t::tanh)); \
                Class.add_function(GL::decl_func(&class_t::transpose)); \
                Class.add_function(GL::make_callable("to_string", [](class_t const& lhs) { return lhs.to_string(); })); \
                this->add_function(GL::make_callable("[]", [](class_t const& lhs, unsigned int x, unsigned int y, unsigned int z) -> typename class_t::type { return lhs.operator()(x,y,z); })); \
                this->add_function(GL::make_callable("[]", [](class_t const& lhs, unsigned int x) -> typename class_t::type { return lhs.operator[](x); })); \
                this->add_function(GL::make_callable("+", [](class_t const& lhs, class_t const& rhs) { return lhs + rhs; })); \
                this->add_function(GL::make_callable("-", [](class_t const& lhs, class_t const& rhs) { return lhs - rhs; })); \
                this->add_function(GL::make_callable("*", [](class_t const& lhs, class_t const& rhs) { return lhs * rhs; })); \
                this->add_function(GL::make_callable("/", [](class_t const& lhs, class_t const& rhs) { return lhs / rhs; })); \
                this->add_function(GL::make_callable("+", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs + rhs; })); \
                this->add_function(GL::make_callable("-", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs - rhs; })); \
                this->add_function(GL::make_callable("*", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs * rhs; })); \
                this->add_function(GL::make_callable("/", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs / rhs; })); \
                this->add_function(GL::make_callable("+=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() += rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("-=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() -= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("*=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() *= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("/=", [](GL::any::fast_any const& lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() /= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("+=", [](GL::any::fast_any const& lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() += rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("-=", [](GL::any::fast_any const& lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() -= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("*=", [](GL::any::fast_any const& lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() *= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("/=", [](GL::any::fast_any const& lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() /= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("%", [](class_t const& lhs, class_t const& rhs) { return lhs % rhs; })); \
                this->add_function(GL::make_callable("%", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs % rhs; })); \
                this->add_function(GL::make_callable("&&", [](class_t const& lhs, class_t const& rhs) { return lhs && rhs; })); \
                this->add_function(GL::make_callable("&&", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs && rhs; })); \
                this->add_function(GL::make_callable("||", [](class_t const& lhs, class_t const& rhs) { return lhs || rhs; })); \
                this->add_function(GL::make_callable("||", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs || rhs; })); \
                this->add_function(GL::make_callable("!", [](class_t const& lhs) { return !lhs; })); \
                this->add_function(GL::make_callable("==", [](class_t const& lhs, class_t const& rhs) { return lhs == rhs; })); \
                this->add_function(GL::make_callable("!=", [](class_t const& lhs, class_t const& rhs) { return lhs != rhs; })); \
                this->add_function(GL::make_callable(">", [](class_t const& lhs, class_t const& rhs) { return lhs > rhs; })); \
                this->add_function(GL::make_callable(">=", [](class_t const& lhs, class_t const& rhs) { return lhs >= rhs; })); \
                this->add_function(GL::make_callable("<", [](class_t const& lhs, class_t const& rhs) { return lhs < rhs; })); \
                this->add_function(GL::make_callable("<=", [](class_t const& lhs, class_t const& rhs) { return lhs <= rhs; })); \
                }

                add_matrix(float);
                add_matrix(unsigned long);
                add_matrix(long);
                add_matrix(int);
                add_matrix(unsigned int);
                add_matrix(char);
                add_matrix(unsigned char);
#undef add_matrix

                this->make_class(GL::type_of<GPU::matrix<float>>()).add_function(GL::make_callable("random", &GPU::matrix<float>::random));
            }
		};
        void impl::RootScope::preload_conversions() {
            for (auto& _type : all_convertable_types()) {
                (void)this->get_converters().try_get_converter(_type, _type);
            }
        };

	}
}