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
    /// \brief Function for extracting a value stored in a Boxed_Value object
    /// \tparam Type The type to extract from the Boxed_Value
    /// \param[in] bv The Boxed_Value to extract a typed value from
    /// \returns Type equivalent to the requested type
    /// \throws exception::bad_boxed_cast If the requested conversion is not possible
    ///
    /// boxed_cast will attempt to make conversions between value, &, *, chaiscript::shared_ptr, std::reference_wrapper,
    /// and std::function (const and non-const) where possible. boxed_cast is used internally during function
    /// dispatch. This means that all of these conversions will be attempted automatically for you during
    /// ChaiScript function calls.
    ///
    /// \li non-const values can be extracted as const or non-const
    /// \li const values can be extracted only as const
    /// \li Boxed_Value constructed from pointer or std::reference_wrapper can be extracted as reference,
    ///     pointer or value types
    /// \li Boxed_Value constructed from chaiscript::shared_ptr or value types can be extracted as reference,
    ///     pointer, value, or chaiscript::shared_ptr types
    ///
    /// Conversions to std::function objects are attempted as well
    ///
    /// Example:
    /// \code
    /// // All of the following should succeed
    /// chaiscript::Boxed_Value bv(1);
    /// chaiscript::shared_ptr<int> spi = chaiscript::boxed_cast<chaiscript::shared_ptr<int> >(bv);
    /// int i = chaiscript::boxed_cast<int>(bv);
    /// int *ip = chaiscript::boxed_cast<int *>(bv);
    /// int &ir = chaiscript::boxed_cast<int &>(bv);
    /// chaiscript::shared_ptr<const int> cspi = chaiscript::boxed_cast<chaiscript::shared_ptr<const int> >(bv);
    /// const int ci = chaiscript::boxed_cast<const int>(bv);
    /// const int *cip = chaiscript::boxed_cast<const int *>(bv);
    /// const int &cir = chaiscript::boxed_cast<const int &>(bv);
    /// \endcode
    ///
    /// std::function conversion example
    /// \code
    /// chaiscript::ChaiScript chai;
    /// Boxed_Value bv = chai.eval("`+`"); // Get the functor for the + operator which is built in
    /// std::function<int (int, int)> f = chaiscript::boxed_cast<std::function<int (int, int)> >(bv);
    /// int i = f(2,3);
    /// assert(i == 5);
    /// \endcode
    template<typename Type>
    decltype(auto) boxed_cast(const Boxed_Value& bv, const Type_Conversions_State* t_conversions) {         
        bool exactTypeMatch = bv.get_type_info().bare_equal(user_type<Type>());            
        if (t_conversions && !exactTypeMatch) {
            bool convertable = (*t_conversions)->converts(chaiscript::user_type<Type>(), bv.get_type_info());
            // bool convertable = (*t_conversions)->convertable_type<Type>();
            if (convertable) {
                try {
                    // We will not catch any bad_boxed_dynamic_cast that is thrown, let the user get it either way, we are not responsible if it doesn't work
                    return (detail::Cast_Helper<Type>::cast((*t_conversions)->boxed_type_conversion<Type>(t_conversions->saves(), bv), t_conversions));
                }
                catch (...) {
                    try {
                        // try going the other way
                        return (detail::Cast_Helper<Type>::cast((*t_conversions)->boxed_type_down_conversion<Type>(t_conversions->saves(), bv), t_conversions));
                    }
                    catch (const chaiscript::detail::exception::bad_any_cast&) {
                        throw exception::bad_boxed_cast(bv.get_type_info(), typeid(Type));
                    }
                }
            }
        }
        try {
            return detail::Cast_Helper<Type>::cast(bv, t_conversions);
        }
        catch (const chaiscript::detail::exception::bad_any_cast&) {
            throw exception::bad_boxed_cast(bv.get_type_info(), typeid(Type));
        }        
    }
} // namespace chaiscript
