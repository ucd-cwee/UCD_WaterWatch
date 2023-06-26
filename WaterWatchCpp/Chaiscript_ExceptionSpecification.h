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
        struct Exception_Handler_Base {
            virtual void handle(const Boxed_Value& bv, const Dispatch_Engine& t_engine) = 0;

            virtual ~Exception_Handler_Base() = default;

        protected:
            template<typename T>
            static void throw_type(const Boxed_Value& bv, const Dispatch_Engine& t_engine) {
                try {
                    T t = t_engine.boxed_cast<T>(bv);
                    throw t;
                }
                catch (const chaiscript::exception::bad_boxed_cast&) {
                }
            }
        };

        template<typename... T>
        struct Exception_Handler_Impl : Exception_Handler_Base {
            void handle(const Boxed_Value& bv, const Dispatch_Engine& t_engine) override { (throw_type<T>(bv, t_engine), ...); }
        };
    } // namespace detail

    /// \brief Used in the automatic unboxing of exceptions thrown during script evaluation
    ///
    /// Exception specifications allow the user to tell ChaiScript what possible exceptions are expected from the script
    /// being executed. Exception_Handler objects are created with the chaiscript::exception_specification() function.
    ///
    /// Example:
    /// \code
    /// chaiscript::ChaiScript chai;
    ///
    /// try {
    ///   chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<int, double, float, const std::string &, const
    ///   std::exception &>());
    /// } catch (const double e) {
    /// } catch (int) {
    /// } catch (float) {
    /// } catch (const std::string &) {
    /// } catch (const std::exception &e) {
    ///   // This is the one what will be called in the specific throw() above
    /// }
    /// \endcode
    ///
    /// It is recommended that if catching the generic \c std::exception& type that you specifically catch
    /// the chaiscript::exception::eval_error type, so that there is no confusion.
    ///
    /// \code
    /// try {
    ///   chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<const std::exception &>());
    /// } catch (const chaiscript::exception::eval_error &) {
    ///   // Error in script parsing / execution
    /// } catch (const std::exception &e) {
    ///   // Error explicitly thrown from script
    /// }
    /// \endcode
    ///
    /// Similarly, if you are using the ChaiScript::eval form that unboxes the return value, then chaiscript::exception::bad_boxed_cast
    /// should be handled as well.
    ///
    /// \code
    /// try {
    ///   chai.eval<int>("1.0", chaiscript::exception_specification<const std::exception &>());
    /// } catch (const chaiscript::exception::eval_error &) {
    ///   // Error in script parsing / execution
    /// } catch (const chaiscript::exception::bad_boxed_cast &) {
    ///   // Error unboxing return value
    /// } catch (const std::exception &e) {
    ///   // Error explicitly thrown from script
    /// }
    /// \endcode
    ///
    /// \sa chaiscript::exception_specification for creation of chaiscript::Exception_Handler objects
    /// \sa \ref exceptions
    using Exception_Handler = chaiscript::shared_ptr<detail::Exception_Handler_Base>;

    /// \brief creates a chaiscript::Exception_Handler which handles one type of exception unboxing
    /// \sa \ref exceptions
    template<typename... T>
    Exception_Handler exception_specification() {
        return chaiscript::make_shared<detail::Exception_Handler_Impl<T...>>();
    }
} // namespace chaiscript
