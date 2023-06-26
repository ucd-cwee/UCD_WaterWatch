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
    class Type_Conversions_State;

    namespace detail {
        // Cast_Helper_Inner helper classes

        template<typename T>
        constexpr T* throw_if_null(T* t) {
            if (t) {
                return t;
            }
            throw std::runtime_error("Attempted to dereference null Boxed_Value");
        }

        template<typename T>
        static const T* verify_type_no_throw(const Boxed_Value& ob, const std::type_info& ti, const T* ptr) {
            if (ob.get_type_info() == ti) {
                return ptr;
            }
            else {
                throw chaiscript::detail::exception::bad_any_cast();
            }
        }

        template<typename T>
        static T* verify_type_no_throw(const Boxed_Value& ob, const std::type_info& ti, T* ptr) {
            if (!ob.is_const() && ob.get_type_info() == ti) {
                return ptr;
            }
            else {
                throw chaiscript::detail::exception::bad_any_cast();
            }
        }

        template<typename T>
        static const T* verify_type(const Boxed_Value& ob, const std::type_info& ti, const T* ptr) {
            if (ob.get_type_info().bare_equal_type_info(ti)) {
                return throw_if_null(ptr);
            }
            else {
                throw chaiscript::detail::exception::bad_any_cast();
            }
        }

        template<typename T>
        static T* verify_type(const Boxed_Value& ob, const std::type_info& ti, T* ptr) {
            if (!ob.is_const() && ob.get_type_info().bare_equal_type_info(ti)) {
                return throw_if_null(ptr);
            }
            else {
                throw chaiscript::detail::exception::bad_any_cast();
            }
        }

        /// Generic Cast_Helper_Inner, for casting to any type
        template<typename Result>
        struct Cast_Helper_Inner {
            static Result cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return *static_cast<const Result*>(verify_type(ob, typeid(Result), ob.get_const_ptr()));
            }
        };

        template<typename Result>
        struct Cast_Helper_Inner<const Result> : Cast_Helper_Inner<Result> {
        };

        /// Cast_Helper_Inner for casting to a const * type
        template<typename Result>
        struct Cast_Helper_Inner<const Result*> {
            static const Result* cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return static_cast<const Result*>(verify_type_no_throw(ob, typeid(Result), ob.get_const_ptr()));
            }
        };

        /// Cast_Helper_Inner for casting to a * type
        template<typename Result>
        struct Cast_Helper_Inner<Result*> {
            static Result* cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return static_cast<Result*>(verify_type_no_throw(ob, typeid(Result), ob.get_ptr()));
            }
        };

        template<typename Result>
        struct Cast_Helper_Inner<Result* const&> : public Cast_Helper_Inner<Result*> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const Result* const&> : public Cast_Helper_Inner<const Result*> {
        };

        /// Cast_Helper_Inner for casting to a & type
        template<typename Result>
        struct Cast_Helper_Inner<const Result&> {
            static const Result& cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return *static_cast<const Result*>(verify_type(ob, typeid(Result), ob.get_const_ptr()));
            }
        };

        /// Cast_Helper_Inner for casting to a & type
        template<typename Result>
        struct Cast_Helper_Inner<Result&> {
            static Result& cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return *static_cast<Result*>(verify_type(ob, typeid(Result), ob.get_ptr()));
            }
        };

        /// Cast_Helper_Inner for casting to a && type
        template<typename Result>
        struct Cast_Helper_Inner<Result&&> {
            static Result&& cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return std::move(*static_cast<Result*>(verify_type(ob, typeid(Result), ob.get_ptr())));
            }
        };

        /// Cast_Helper_Inner for casting to a chaiscript::unique_ptr<> && type
        /// \todo Fix the fact that this has to be in a shared_ptr for now
        template<typename Result>
        struct Cast_Helper_Inner<chaiscript::unique_ptr<Result>&&> {
            static chaiscript::unique_ptr<Result>&& cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return std::move(*(ob.get().cast<chaiscript::shared_ptr<chaiscript::unique_ptr<Result>>>()));
            }
        };

        /// Cast_Helper_Inner for casting to a chaiscript::unique_ptr<> & type
        /// \todo Fix the fact that this has to be in a shared_ptr for now
        template<typename Result>
        struct Cast_Helper_Inner<chaiscript::unique_ptr<Result>&> {
            static chaiscript::unique_ptr<Result>& cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return *(ob.get().cast<chaiscript::shared_ptr<chaiscript::unique_ptr<Result>>>());
            }
        };

        /// Cast_Helper_Inner for casting to a chaiscript::unique_ptr<> & type
        /// \todo Fix the fact that this has to be in a shared_ptr for now
        template<typename Result>
        struct Cast_Helper_Inner<const chaiscript::unique_ptr<Result>&> {
            static chaiscript::unique_ptr<Result>& cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return *(ob.get().cast<chaiscript::shared_ptr<chaiscript::unique_ptr<Result>>>());
            }
        };

        /// Cast_Helper_Inner for casting to a chaiscript::shared_ptr<> type
        template<typename Result>
        struct Cast_Helper_Inner<chaiscript::shared_ptr<Result>> {
            static auto cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) { return ob.get().cast<chaiscript::shared_ptr<Result>>(); }
        };

        /// Cast_Helper_Inner for casting to a chaiscript::shared_ptr<const> type
        template<typename Result>
        struct Cast_Helper_Inner<chaiscript::shared_ptr<const Result>> {
            static auto cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                if (!ob.get_type_info().is_const()) {
                    return chaiscript::const_shared_ptr_cast<const Result>(ob.get().cast<chaiscript::shared_ptr<Result>>());
                }
                else {
                    return ob.get().cast<chaiscript::shared_ptr<const Result>>();
                }
            }
        };

        /// Cast_Helper_Inner for casting to a const chaiscript::shared_ptr<> & type
        template<typename Result>
        struct Cast_Helper_Inner<const chaiscript::shared_ptr<Result>> : Cast_Helper_Inner<chaiscript::shared_ptr<Result>> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const chaiscript::shared_ptr<Result>&> : Cast_Helper_Inner<chaiscript::shared_ptr<Result>> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<chaiscript::shared_ptr<Result>&> {
            static_assert(!std::is_const<Result>::value, "Non-const reference to chaiscript::shared_ptr<const T> is not supported");
            static auto cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                chaiscript::shared_ptr<Result>& res = ob.get().cast<chaiscript::shared_ptr<Result>>();
                return ob.pointer_sentinel(res);
            }
        };

        /// Cast_Helper_Inner for casting to a const chaiscript::shared_ptr<const> & type
        template<typename Result>
        struct Cast_Helper_Inner<const chaiscript::shared_ptr<const Result>> : Cast_Helper_Inner<chaiscript::shared_ptr<const Result>> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const chaiscript::shared_ptr<const Result>&> : Cast_Helper_Inner<chaiscript::shared_ptr<const Result>> {
        };

        /// Cast_Helper_Inner for casting to a Boxed_Value type
        template<>
        struct Cast_Helper_Inner<Boxed_Value> {
            static Boxed_Value cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) { return ob; }
        };

        /// Cast_Helper_Inner for casting to a Boxed_Value & type
        template<>
        struct Cast_Helper_Inner<Boxed_Value&> {
            static std::reference_wrapper<Boxed_Value> cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return std::ref(const_cast<Boxed_Value&>(ob));
            }
        };

        /// Cast_Helper_Inner for casting to a const Boxed_Value & type
        template<>
        struct Cast_Helper_Inner<const Boxed_Value> : Cast_Helper_Inner<Boxed_Value> {
        };

        template<>
        struct Cast_Helper_Inner<const Boxed_Value&> : Cast_Helper_Inner<Boxed_Value> {
        };

        /// Cast_Helper_Inner for casting to a std::reference_wrapper type
        template<typename Result>
        struct Cast_Helper_Inner<std::reference_wrapper<Result>> : Cast_Helper_Inner<Result&> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const std::reference_wrapper<Result>> : Cast_Helper_Inner<Result&> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const std::reference_wrapper<Result>&> : Cast_Helper_Inner<Result&> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<std::reference_wrapper<const Result>> : Cast_Helper_Inner<const Result&> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const std::reference_wrapper<const Result>> : Cast_Helper_Inner<const Result&> {
        };

        template<typename Result>
        struct Cast_Helper_Inner<const std::reference_wrapper<const Result>&> : Cast_Helper_Inner<const Result&> {
        };

        /// The exposed Cast_Helper object that by default just calls the Cast_Helper_Inner
        template<typename T>
        struct Cast_Helper {
            static decltype(auto) cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                return (Cast_Helper_Inner<T>::cast(ob, t_conversions));
            }
        };
    } // namespace detail

    class BoxedValueCaster {
    public:
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
        static decltype(auto) cast(const Boxed_Value& bv, const Type_Conversions_State* t_conversions) { return detail::Cast_Helper::cast<Type>(bv, t_conversions); };
    };

}; // namespace chaiscript
