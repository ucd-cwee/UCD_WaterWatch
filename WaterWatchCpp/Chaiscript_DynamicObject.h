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
    namespace dispatch {
        struct option_explicit_set : std::runtime_error {
            explicit option_explicit_set(const std::string& t_param_name)
                : std::runtime_error("option explicit set and parameter '" + t_param_name + "' does not exist") {
            }

            option_explicit_set(const option_explicit_set&) = default;

            ~option_explicit_set() noexcept override = default;
        };
        class Dynamic_Object {
        public:
            explicit Dynamic_Object(std::string t_type_name)
                : m_type_name(std::move(t_type_name))
                , m_option_explicit(false) {
            }

            Dynamic_Object() = default;

            bool is_explicit() const noexcept { return m_option_explicit; }

            void set_explicit(const bool t_explicit) noexcept { m_option_explicit = t_explicit; }

            const std::string& get_type_name() const noexcept { return m_type_name; }

            const Boxed_Value& operator[](const std::string& t_attr_name) const { return get_attr(t_attr_name); }

            Boxed_Value& operator[](const std::string& t_attr_name) { return get_attr(t_attr_name); }

            const Boxed_Value& get_attr(const std::string& t_attr_name) const {
                auto a = m_attrs.find(t_attr_name);

                if (a != m_attrs.end()) {
                    return a->second;
                }
                else {
                    throw std::range_error("Attr not found '" + t_attr_name + "' and cannot be added to const obj");
                }
            }

            bool has_attr(const std::string& t_attr_name) const { return m_attrs.find(t_attr_name) != m_attrs.end(); }

            Boxed_Value& get_attr(const std::string& t_attr_name) { return m_attrs[t_attr_name]; }

            Boxed_Value& method_missing(const std::string& t_method_name) {
                if (m_option_explicit && m_attrs.find(t_method_name) == m_attrs.end()) {
                    throw option_explicit_set(t_method_name);
                }

                return get_attr(t_method_name);
            }

            const Boxed_Value& method_missing(const std::string& t_method_name) const {
                if (m_option_explicit && m_attrs.find(t_method_name) == m_attrs.end()) {
                    throw option_explicit_set(t_method_name);
                }

                return get_attr(t_method_name);
            }

            std::map<std::string, Boxed_Value> get_attrs() const { return m_attrs; }

            void remove_attr(const std::string& t_attr_name) {
                auto f = m_attrs.find(t_attr_name);
                if (f != m_attrs.end()) {
                    m_attrs.erase(f);
                }                
            };

        private:
            const std::string m_type_name = "";
            bool m_option_explicit = false;

            std::map<std::string, Boxed_Value> m_attrs;
        };
    } // namespace dispatch
} // namespace chaiscript