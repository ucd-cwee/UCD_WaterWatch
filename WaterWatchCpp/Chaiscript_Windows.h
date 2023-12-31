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
        struct Loadable_Module {
            template<typename T>
            static std::wstring to_wstring(const T& t_str) {
                return std::wstring(t_str.begin(), t_str.end());
            }

            template<typename T>
            static std::string to_string(const T& t_str) {
                return std::string(t_str.begin(), t_str.end());
            }

#if defined(_UNICODE) || defined(UNICODE)
            template<typename T>
            static std::wstring to_proper_string(const T& t_str) {
                return to_wstring(t_str);
            }
#else
            template<typename T>
            static std::string to_proper_string(const T& t_str) {
                return to_string(t_str);
            }
#endif

            static std::string get_error_message(DWORD t_err) {
                using StringType = LPTSTR;

#if defined(_UNICODE) || defined(UNICODE)
                std::wstring retval = L"Unknown Error";
#else
                std::string retval = "Unknown Error";
#endif
                StringType lpMsgBuf = nullptr;

                if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    t_err,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<StringType>(&lpMsgBuf),
                    0,
                    nullptr)
                    != 0
                    && lpMsgBuf) {
                    retval = lpMsgBuf;
                    LocalFree(lpMsgBuf);
                }

                return to_string(retval);
            }

            struct DLModule {
                explicit DLModule(const std::string& t_filename)
                    : m_data(
#ifdef LoadLibrary            
                        LoadLibrary(to_proper_string(t_filename).c_str())
#else

#endif
                    ) {
                    if (!m_data) {
                        throw chaiscript::exception::load_module_error(get_error_message(GetLastError()));
                    }
                }

                DLModule(DLModule&&) = default;
                DLModule& operator=(DLModule&&) = default;
                DLModule(const DLModule&) = delete;
                DLModule& operator=(const DLModule&) = delete;

                ~DLModule() { FreeLibrary(m_data); }

                HMODULE m_data;
            };

            template<typename T>
            struct DLSym {
                DLSym(DLModule& t_mod, const std::string& t_symbol)
                    : m_symbol(reinterpret_cast<T>(GetProcAddress(t_mod.m_data, t_symbol.c_str()))) {
                    if (!m_symbol) {
                        throw chaiscript::exception::load_module_error(get_error_message(GetLastError()));
                    }
                }

                T m_symbol;
            };

            Loadable_Module(const std::string& t_module_name, const std::string& t_filename)
                : m_dlmodule(t_filename)
                , m_func(m_dlmodule, "create_chaiscript_module_" + t_module_name)
                , m_moduleptr(m_func.m_symbol()) {
            }

            DLModule m_dlmodule;
            DLSym<Create_Module_Func> m_func;
            ModulePtr m_moduleptr;
        };
    } // namespace detail
} // namespace chaiscript