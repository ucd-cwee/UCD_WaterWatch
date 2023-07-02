/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

This file is distributed under the BSD License.
Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
http://www.chaiscript.com

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and for better real-time support, including object-typing from parsed code, pre-parsing code without running, real multithreaded code analysis, and more.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "Strings.h"
#include "Parser.h"

#ifdef _MSC_VER
#define CHAISCRIPT_STRINGIZE(x) "" #x
#define CHAISCRIPT_STRINGIZE_EXPANDED(x) CHAISCRIPT_STRINGIZE(x)
#define CHAISCRIPT_COMPILER_VERSION CHAISCRIPT_STRINGIZE_EXPANDED(_MSC_FULL_VER)
#define CHAISCRIPT_MSVC _MSC_VER
#define CHAISCRIPT_HAS_DECLSPEC

#define AddFunction(captures, name, returns, todo, ...) add(fun([captures](__VA_ARGS__) returns { todo; }, cweeStr(#__VA_ARGS__).RemoveBetween("<", ">").ReplaceInline("*", " ").ReplaceInline(" const", " ").ReplaceInline("const ", " ").ReplaceInline("&", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").Split(",").Trim(' ').ReplaceInline("  ", " ").SplitAgain(" ").Trim(' ').GetEveryOtherVar()), #name)
#define SINGLE_ARG(...) __VA_ARGS__

static_assert(_MSC_FULL_VER >= 190024210, "Visual C++ 2015 Update 3 or later required");

#else
#define CHAISCRIPT_COMPILER_VERSION __VERSION__
#endif

#include <string_view>
#include <vector>

#if defined(_LIBCPP_VERSION)
#define CHAISCRIPT_LIBCPP
#endif

// RG
#define _WIN32

#if defined(_WIN32) || defined(__CYGWIN__)
#define CHAISCRIPT_WINDOWS
#endif

#if defined(_WIN32)
#if defined(__llvm__)
#define CHAISCRIPT_COMPILER_NAME "clang(windows)"
#elif defined(__GNUC__)
#define CHAISCRIPT_COMPILER_NAME "gcc(mingw)"
#else
#define CHAISCRIPT_COMPILER_NAME "msvc"
#endif
#else
#if defined(__llvm__)
#define CHAISCRIPT_COMPILER_NAME "clang"
#elif defined(__GNUC__)
#define CHAISCRIPT_COMPILER_NAME "gcc"
#else
#define CHAISCRIPT_COMPILER_NAME "unknown"
#endif
#endif

#if defined(__llvm__)
#define CHAISCRIPT_CLANG
#endif

#ifdef CHAISCRIPT_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C"
#endif

#if defined(CHAISCRIPT_MSVC) || (defined(__GNUC__) && __GNUC__ >= 5) || defined(CHAISCRIPT_CLANG)
#define CHAISCRIPT_UTF16_UTF32
#endif

#ifdef _DEBUG
#define CHAISCRIPT_DEBUG true
#else
#define CHAISCRIPT_DEBUG false
#endif

#include <cmath>
#include <memory>
#include <string>

// #define USE_CWEE_SHARED_PTR
namespace chaiscript {
    constexpr static const int version_major = 7;
    constexpr static const int version_minor = 0;
    constexpr static const int version_patch = 0;

    constexpr static const char* compiler_version = CHAISCRIPT_COMPILER_VERSION;
    constexpr static const char* compiler_name = CHAISCRIPT_COMPILER_NAME;
    constexpr static const bool debug_build = CHAISCRIPT_DEBUG;

    template <class, template <class> class> struct is_instance_of : public std::false_type {};
    template <class T, template <class> class U> struct is_instance_of<U<T>, U> : public std::true_type {};

    template<typename T> using shared_ptr =
#ifdef USE_CWEE_SHARED_PTR
        cweeSharedPtr<T>;
#else
        std::shared_ptr<T>;
#endif
    template<typename T> using unique_ptr =
#ifdef USE_CWEE_SHARED_PTR
        cweeSharedPtr<T>;
#else
        std::unique_ptr<T>;
#endif      

    template<typename T> using small_vector =
        std::vector<T>;

    template<typename B>
    inline chaiscript::shared_ptr<B> make_shared() {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
        return std::make_shared<B>();
#else
        return chaiscript::shared_ptr<B>(new B());
#endif
    };

    template<typename B>  inline chaiscript::unique_ptr<B> make_unique() {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
        return std::make_unique<B>();
#else
        return chaiscript::unique_ptr<B>(new B());
#endif
    };

    template<typename B, typename... Arg>  inline chaiscript::shared_ptr<B> make_shared(Arg &&...arg) {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
        return std::make_shared<D>(std::forward<Arg>(arg)...);
#else
        return chaiscript::shared_ptr<B>(new B(std::forward<Arg>(arg)...));
#endif
    };

    template<typename B, typename... Arg>  inline chaiscript::unique_ptr<B> make_unique(Arg &&...arg) {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
        return std::make_unique<D>(std::forward<Arg>(arg)...);
#else
        return chaiscript::unique_ptr<B>(new B(std::forward<Arg>(arg)...));
#endif
    };

    template<typename B, typename D, typename... Arg>  inline chaiscript::shared_ptr<B> make_shared(Arg &&...arg) {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
        return std::make_shared<D>(std::forward<Arg>(arg)...);
#else
        return chaiscript::shared_ptr<B>(static_cast<B*>(new D(std::forward<Arg>(arg)...)));
#endif
    };

    template<typename B, typename D, typename... Arg>  inline chaiscript::unique_ptr<B> make_unique(Arg &&...arg) {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
        return std::make_unique<D>(std::forward<Arg>(arg)...);
#else
        return chaiscript::unique_ptr<B>(static_cast<B*>(new D(std::forward<Arg>(arg)...)));
#endif
    };

    template< typename to, typename from >  inline decltype(auto) dynamic_shared_ptr_cast(chaiscript::shared_ptr<from> const& t_func, chaiscript::shared_ptr<to>& t_out) {
#ifdef USE_CWEE_SHARED_PTR
        t_out = t_func.CastReference<to>();
        return t_out;
#else
        t_out = std::dynamic_pointer_cast<to>(t_func);
        return t_out;
#endif
    };

    template< typename to, typename from >  inline decltype(auto) dynamic_shared_ptr_cast(chaiscript::shared_ptr<from> const& t_func) {
#ifdef USE_CWEE_SHARED_PTR
        return t_func.CastReference<to>();
#else
        return std::dynamic_pointer_cast<to>(t_func);
#endif
    };

    template< typename to, typename from >  inline decltype(auto) dynamic_unique_ptr_cast(chaiscript::unique_ptr<from>&& t_func) {
#ifdef USE_CWEE_SHARED_PTR
        return t_func.CastReference<to>();
#else
        return chaiscript::unique_ptr<to>(std::move(t_func));
        // return chaiscript::make_unique<to>(dynamic_cast<to>(t_func));
#endif
    };

    template< typename to, typename from >  inline decltype(auto) const_shared_ptr_cast(chaiscript::shared_ptr<from> const& t_func) {
#ifdef USE_CWEE_SHARED_PTR
        return t_func.CastReference<to>();
#else
        return std::const_pointer_cast<to>(t_func);
#endif
    };

    template< typename to, typename from >  inline decltype(auto) static_pointer_cast(chaiscript::shared_ptr<from> const& t_func) {
#ifdef USE_CWEE_SHARED_PTR
        return t_func.CastReference<to>();
#else
        return std::static_pointer_cast<to>(t_func);
#endif
    };

    struct Build_Info {
        [[nodiscard]] constexpr static int version_major() noexcept { return chaiscript::version_major; }

        [[nodiscard]] constexpr static int version_minor() noexcept { return chaiscript::version_minor; }

        [[nodiscard]] constexpr static int version_patch() noexcept { return chaiscript::version_patch; }

        [[nodiscard]] static std::string version() {
            return std::to_string(version_major()) + '.' + std::to_string(version_minor()) + '.' + std::to_string(version_patch());
        }

        [[nodiscard]] static std::string compiler_id() { return compiler_name() + '-' + compiler_version(); }

        [[nodiscard]] static std::string build_id() { return compiler_id() + (debug_build() ? "-Debug" : "-Release"); }

        [[nodiscard]] static std::string compiler_version() { return chaiscript::compiler_version; }

        [[nodiscard]] static std::string compiler_name() { return chaiscript::compiler_name; }

        [[nodiscard]] constexpr static bool debug_build() noexcept { return chaiscript::debug_build; }
    };

    template<typename T>
    [[nodiscard]] constexpr auto parse_num(const std::string_view t_str) noexcept -> typename std::enable_if<std::is_integral<T>::value, T>::type {
        T t = 0;
        for (const auto c : t_str) {
            if (c < '0' || c > '9') {
                return t;
            }
            t *= 10;
            t += c - '0';
        }
        return t;
    }

    template<typename T>
    [[nodiscard]] auto parse_num(const std::string_view t_str) -> typename std::enable_if<!std::is_integral<T>::value, T>::type {
        T t = 0;
        T base{};
        T decimal_place = 0;
        int exponent = 0;

        for (const auto c : t_str) {
            switch (c) {
            case '.':
                decimal_place = 10;
                break;
            case 'e':
            case 'E':
                exponent = 1;
                decimal_place = 0;
                base = t;
                t = 0;
                break;
            case '-':
                exponent = -1;
                break;
            case '+':
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (decimal_place < 10) {
                    t *= 10;
                    t += static_cast<T>(c - '0');
                }
                else {
                    t += static_cast<T>(c - '0') / decimal_place;
                    decimal_place *= 10;
                }
                break;
            default:
                break;
            }
        }
        return exponent ? base * std::pow(T(10), t * static_cast<T>(exponent)) : t;
    }

    struct str_equal {
        [[nodiscard]] bool operator()(const std::string& t_lhs, const std::string& t_rhs) const noexcept { return t_lhs == t_rhs; }
        template<typename LHS, typename RHS>
        [[nodiscard]] constexpr bool operator()(const LHS& t_lhs, const RHS& t_rhs) const noexcept {
            return std::equal(t_lhs.begin(), t_lhs.end(), t_rhs.begin(), t_rhs.end());
        }
        struct is_transparent {
        };
    };

    struct str_less {
        [[nodiscard]] bool operator()(const std::string& t_lhs, const std::string& t_rhs) const noexcept { return t_lhs < t_rhs; }
        template<typename LHS, typename RHS>
        [[nodiscard]] constexpr bool operator()(const LHS& t_lhs, const RHS& t_rhs) const noexcept {
            return std::lexicographical_compare(t_lhs.begin(), t_lhs.end(), t_rhs.begin(), t_rhs.end());
        }
        struct is_transparent {
        };
    };

    enum class Options {
        No_Load_Modules,
        Load_Modules,
        No_External_Scripts,
        External_Scripts
    };

    template<typename From, typename To>
    struct is_nothrow_forward_constructible : std::bool_constant < noexcept(To{ std::declval<From>() }) > {
    };

    template<class From, class To>
    static inline constexpr bool is_nothrow_forward_constructible_v = is_nothrow_forward_constructible<From, To>::value;

    template<typename Container, typename... T>
    [[nodiscard]] constexpr auto make_container(T &&...t) {
        Container c;
        c.reserve(sizeof...(t));
        (c.push_back(std::forward<T>(t)), ...);
        return c;
    }

    template<typename... T>
    [[nodiscard]] auto make_vector(T &&...t) -> chaiscript::small_vector<std::common_type_t<std::decay_t<T>...>> {
        using container_type = chaiscript::small_vector<std::common_type_t<std::decay_t<T>...>>;

        return make_container<container_type>(std::forward<T>(t)...);
    }

    [[nodiscard]] inline chaiscript::small_vector<Options> default_options() {
#ifdef CHAISCRIPT_NO_DYNLOAD
        return { Options::No_Load_Modules, Options::External_Scripts };
#else
        return { Options::Load_Modules, Options::External_Scripts };
#endif
    }
} // namespace chaiscript