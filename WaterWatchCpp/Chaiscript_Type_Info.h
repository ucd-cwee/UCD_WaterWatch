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
#include "chaiscript_wrapper.h"

namespace chaiscript {
    namespace detail {

        struct Unknown_Type {
        };

        template<typename T>
        struct Bare_Type {
            using type = typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type;
        };

        template<typename T, typename _ = void>
        struct is_container {
            static bool const value = false; 
            using type = Unknown_Type; 
        };

        template<typename... Ts>
        struct is_container_helper {};

        template<typename T>
        struct is_container<
            T,
            std::conditional_t<
            false,
            is_container_helper<
            typename T::value_type,
            decltype(std::declval<T>().size()),
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end())
            >,
            void
            >
        > { 
            static bool const value = true; 
            using type = typename Bare_Type<T>::type::value_type;
        };

        template<typename T>
        struct Contained_Type {
            using bare_type = typename chaiscript::detail::Bare_Type<T>::type;
            using container_inner_type = typename chaiscript::detail::is_container<bare_type>::type;
            using type = container_inner_type;

            // using type = typename is_container<typename Bare_Type<T>::type>::type;
        };
    } // namespace detail

    /// \brief Compile time deduced information about a type
    class Type_Info {
    public:
        static bool SameTypeInfo(const std::type_info& a, const std::type_info& b) noexcept {
            return &a == &b; // a.hash_code() == b.hash_code();
        };

        constexpr Type_Info(const bool t_is_const,
            const bool t_is_reference,
            const bool t_is_pointer,
            const bool t_is_void,
            const bool t_is_arithmetic,
            const std::type_info* t_ti,
            const std::type_info* t_bare_ti, 
            const std::type_info* t_contained_ti
            ) noexcept
            : m_type_info(t_ti)
            , m_bare_type_info(t_bare_ti)
            , m_contained_type_info(t_contained_ti)
            , m_flags((static_cast<unsigned int>(t_is_const) << is_const_flag) + (static_cast<unsigned int>(t_is_reference) << is_reference_flag)
                + (static_cast<unsigned int>(t_is_pointer) << is_pointer_flag) + (static_cast<unsigned int>(t_is_void) << is_void_flag)
                + (static_cast<unsigned int>(t_is_arithmetic) << is_arithmetic_flag)) {
        }

        constexpr Type_Info() noexcept = default;

        bool operator<(const Type_Info& ti) const noexcept { return m_type_info->before(*ti.m_type_info); }

        bool operator!=(const Type_Info& ti) const noexcept { return !(operator==(ti)); }

        bool operator!=(const std::type_info& ti) const noexcept { return !(operator==(ti)); }

        bool operator==(const Type_Info& ti) const noexcept {
            return ti.m_type_info == m_type_info || (m_type_info && ti.m_type_info && SameTypeInfo(*ti.m_type_info, *m_type_info));
        }

        bool operator==(const std::type_info& ti) const noexcept { return !is_undef() && m_type_info && SameTypeInfo(ti, *m_type_info); }

        bool bare_equal(const Type_Info& ti) const noexcept {
            return ti.m_bare_type_info == m_bare_type_info || (ti.m_bare_type_info && m_bare_type_info && SameTypeInfo(*ti.m_bare_type_info, *m_bare_type_info));
        }

        bool bare_equal_type_info(const std::type_info& ti) const noexcept { return !is_undef() && m_bare_type_info && SameTypeInfo(ti, *m_bare_type_info); }

        constexpr bool is_const() const noexcept { return (m_flags & (1 << is_const_flag)) != 0; }
        constexpr bool is_reference() const noexcept { return (m_flags & (1 << is_reference_flag)) != 0; }
        constexpr bool is_void() const noexcept { return (m_flags & (1 << is_void_flag)) != 0; }
        constexpr bool is_arithmetic() const noexcept { return (m_flags & (1 << is_arithmetic_flag)) != 0; }
        constexpr bool is_undef() const noexcept { return (m_flags & (1 << is_undef_flag)) != 0; }
        constexpr bool is_pointer() const noexcept { return (m_flags & (1 << is_pointer_flag)) != 0; }

        const char* name() const noexcept {
            if (!is_undef()) {
                return m_type_info->name();
            }
            else {
                return "";
            }
        }

        const char* bare_name() const noexcept {
            if (!is_undef()) {
                return m_bare_type_info->name();
            }
            else {
                return "";
            }
        }

        const char* contained_name() const noexcept {
            if (!is_undef()) {
                return m_contained_type_info->name();
            }
            else {
                return "";
            }
        }

        constexpr const std::type_info* bare_type_info() const noexcept { return m_bare_type_info; }
        constexpr const std::type_info* contained_type_info() const noexcept { return m_contained_type_info; }
        constexpr bool is_container_type() const noexcept { return m_contained_type_info != &typeid(detail::Unknown_Type); }

    private:
        const std::type_info* m_type_info = &typeid(detail::Unknown_Type);
        const std::type_info* m_bare_type_info = &typeid(detail::Unknown_Type);
        const std::type_info* m_contained_type_info = &typeid(detail::Unknown_Type);
        static const int is_const_flag = 0;
        static const int is_reference_flag = 1;
        static const int is_pointer_flag = 2;
        static const int is_void_flag = 3;
        static const int is_arithmetic_flag = 4;
        static const int is_undef_flag = 5;
        unsigned int m_flags = (1 << is_undef_flag);
    };

    namespace detail {
        /// Helper used to create a Type_Info object
        template<typename T>
        struct Get_Type_Info {
            constexpr static Type_Info get() noexcept {
                return Type_Info(std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value,
                    std::is_reference<T>::value,
                    std::is_pointer<T>::value,
                    std::is_void<T>::value,
                    (std::is_arithmetic<T>::value || std::is_arithmetic<typename std::remove_reference<T>::type>::value)
                    && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
                    &typeid(T),
                    &typeid(typename Bare_Type<T>::type), 
                    &typeid(typename Contained_Type<T>::type)
                );
            }
        };

        template<typename T>
        struct Get_Type_Info<chaiscript::shared_ptr<T>> {
            constexpr static Type_Info get() noexcept {
                return Type_Info(std::is_const<T>::value,
                    std::is_reference<T>::value,
                    std::is_pointer<T>::value,
                    std::is_void<T>::value,
                    std::is_arithmetic<T>::value
                    && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
                    &typeid(chaiscript::shared_ptr<T>),
                    &typeid(typename Bare_Type<T>::type), 
                    &typeid(typename Contained_Type<T>::type)
                );
            }
        };

        template<typename T>
        struct Get_Type_Info<chaiscript::shared_ptr<T>&> : Get_Type_Info<chaiscript::shared_ptr<T>> {
        };

        template<typename T>
        struct Get_Type_Info<const chaiscript::shared_ptr<T>&> {
            constexpr static Type_Info get() noexcept {
                return Type_Info(std::is_const<T>::value,
                    std::is_reference<T>::value,
                    std::is_pointer<T>::value,
                    std::is_void<T>::value,
                    std::is_arithmetic<T>::value
                    && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
                    &typeid(const chaiscript::shared_ptr<T> &),
                    &typeid(typename Bare_Type<T>::type), 
                    &typeid(typename Contained_Type<T>::type)
                );
            }
        };

        template<typename T>
        struct Get_Type_Info<std::reference_wrapper<T>> {
            constexpr static Type_Info get() noexcept {
                return Type_Info(std::is_const<T>::value,
                    std::is_reference<T>::value,
                    std::is_pointer<T>::value,
                    std::is_void<T>::value,
                    std::is_arithmetic<T>::value
                    && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
                    &typeid(std::reference_wrapper<T>),
                    &typeid(typename Bare_Type<T>::type), 
                    &typeid(typename Contained_Type<T>::type)
                );
            }
        };

        template<typename T>
        struct Get_Type_Info<const std::reference_wrapper<T>&> {
            constexpr static Type_Info get() noexcept {
                return Type_Info(std::is_const<T>::value,
                    std::is_reference<T>::value,
                    std::is_pointer<T>::value,
                    std::is_void<T>::value,
                    std::is_arithmetic<T>::value
                    && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
                    &typeid(const std::reference_wrapper<T> &),
                    &typeid(typename Bare_Type<T>::type), 
                    &typeid(typename Contained_Type<T>::type)
                );
            }
        };

    } // namespace detail

    /// \brief Creates a Type_Info object representing the type passed in
    /// \tparam T Type of object to get a Type_Info for, derived from the passed in parameter
    /// \return Type_Info for T
    ///
    /// \b Example:
    /// \code
    /// int i;
    /// chaiscript::Type_Info ti = chaiscript::user_type(i);
    /// \endcode
    template<typename T>
    constexpr chaiscript::Type_Info user_type(const T& /*t*/) noexcept {
        return detail::Get_Type_Info<T>::get();
    }

    /// \brief Creates a Type_Info object representing the templated type
    /// \tparam T Type of object to get a Type_Info for
    /// \return Type_Info for T
    ///
    /// \b Example:
    /// \code
    /// chaiscript::Type_Info ti = chaiscript::user_type<int>();
    /// \endcode
    template<typename T>
    constexpr chaiscript::Type_Info user_type() noexcept {
        return detail::Get_Type_Info<T>::get();
    }

} // namespace chaiscript
