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
#include <utility>

namespace chaiscript {
    namespace detail {
        namespace exception {
            /// \brief Thrown in the event that an Any cannot be cast to the desired type
            /// It is used internally during function dispatch.
            /// \sa chaiscript::detail::Any
            class bad_any_cast : public std::bad_cast {
            public:
                /// \brief Description of what error occurred
                const char* what() const noexcept override { return exc.c_str(); }

                bad_any_cast() {};
                bad_any_cast(const std::string& message) {
                    exc = message;
                };

            private:
                std::string exc = "bad any cast";
            };
        } // namespace exception
        class Any {
        private:
            struct Data {
                constexpr explicit Data(const std::type_info& t_type) noexcept : m_type(t_type) {}
                Data& operator=(const Data&) = delete;
                virtual ~Data() noexcept = default;
                virtual void* data() noexcept = 0;
                const std::type_info& type() const noexcept { return m_type; }
                virtual chaiscript::unique_ptr<Data> clone() const = 0;
                const std::type_info& m_type;
            };
            template<typename T> struct Data_Impl : Data {
                explicit Data_Impl(T t_type) : Data(typeid(T)) , m_data(std::move(t_type)) {}
                void* data() noexcept override { return &m_data; }
                chaiscript::unique_ptr<Data> clone() const override { return chaiscript::make_unique<Data_Impl<T>>(m_data); }
                Data_Impl& operator=(const Data_Impl&) = delete;
                T m_data;
            };
            chaiscript::unique_ptr<Data> m_data;

        public:
            // construct/copy/destruct
            constexpr Any() noexcept = default;
            Any(Any&&) noexcept = default;
            Any& operator=(Any&& t_any) = default;

            Any(const Any& t_any) : m_data(t_any.empty() ? nullptr : t_any.m_data->clone()) {};

            template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>>
            explicit Any(ValueType&& t_value) : m_data(chaiscript::make_unique<Data_Impl<std::decay_t<ValueType>>>(std::forward<ValueType>(t_value))) {};

            Any& operator=(const Any& t_any) { Any copy(t_any); swap(copy); return *this; };

            template<typename ToType>
            ToType& cast() const {
                if (m_data && typeid(ToType) == m_data->type()) {
                    return *static_cast<ToType*>(m_data->data());
                }
                else {
                    throw chaiscript::detail::exception::bad_any_cast(std::string("Bad Cast From \"") + m_data->type().name() + "\" To \"" + typeid(ToType).name() + "\".");
                }
            };

            template<typename ToType>
            ToType& force_cast() const {
                if (m_data) {
                    return *static_cast<ToType*>(m_data->data());
                }
                else {
                    throw chaiscript::detail::exception::bad_any_cast();
                }
            };

            template<typename ToType>
            ToType* force_cast_ptr() const {
                if (m_data) {
                    return static_cast<ToType*>(m_data->data());
                }
                else {
                    throw chaiscript::detail::exception::bad_any_cast();
                }
            };

            // modifiers
            Any& swap(Any& t_other) {
                std::swap(t_other.m_data, m_data);
                return *this;
            };

            // queries
            bool empty() const noexcept { return !static_cast<bool>(m_data); };
            const std::type_info& type() const noexcept { if (m_data) { return m_data->type(); } else { return typeid(void); } };
        };
    } // namespace detail
} // namespace chaiscript
