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

namespace chaiscript::dispatch::detail {
    /// used internally for unwrapping a function call's types
    template<typename Ret, typename... Param>
    struct Build_Function_Caller_Helper {
        Build_Function_Caller_Helper(std::vector<Const_Proxy_Function> t_funcs, const Type_Conversions* t_conversions)
            : m_funcs(std::move(t_funcs))
            , m_conversions(t_conversions) {
        }

        Ret call(const chaiscript::Function_Params& params, const Type_Conversions_State& t_state) {
            if constexpr (std::is_arithmetic_v<Ret> && !std::is_same_v<std::remove_cv_t<std::remove_reference_t<Ret>>, bool>) {
                return Boxed_Number(dispatch::dispatch(m_funcs, params, t_state)).get_as<Ret>();
            }
            else if constexpr (std::is_same_v<void, Ret>) {
                dispatch::dispatch(m_funcs, params, t_state);
            }
            else {
                return boxed_cast<Ret>(dispatch::dispatch(m_funcs, params, t_state), &t_state);
            }
        }

        template<typename... P>
        Ret operator()(P &&...param) {
            std::array<Boxed_Value, sizeof...(P)> params{ box<P>(std::forward<P>(param))... };

            if (m_conversions) {
                Type_Conversions_State state(*m_conversions, m_conversions->conversion_saves());
                return call(chaiscript::Function_Params{ params }, state);
            }
            else {
                Type_Conversions conv;
                Type_Conversions_State state(conv, conv.conversion_saves());
                return call(chaiscript::Function_Params{ params }, state);
            }
        }

        template<typename P, typename Q>
        static Boxed_Value box(Q&& q) {
            if constexpr (std::is_same_v<chaiscript::Boxed_Value, std::decay_t<Q>>) {
                return std::forward<Q>(q);
            }
            else if constexpr (std::is_reference_v<P>) {
                return Boxed_Value(std::ref(std::forward<Q>(q)));
            }
            else {
                return Boxed_Value(std::forward<Q>(q));
            }
        }

        std::vector<Const_Proxy_Function> m_funcs;
        const Type_Conversions* m_conversions;
    };

    /// \todo what happens if t_conversions is deleted out from under us?!
    template<typename Ret, typename... Params>
    std::function<Ret(Params...)>
        build_function_caller_helper(Ret(Params...), const std::vector<Const_Proxy_Function>& funcs, const Type_Conversions_State* t_conversions) {
        /*
        if (funcs.size() == 1)
        {
          std::shared_ptr<const Proxy_Function_Impl<Ret (Params...)>> pfi =
            std::dynamic_pointer_cast<const Proxy_Function_Impl<Ret (Params...)> >
              (funcs[0]);

          if (pfi)
          {
            return pfi->internal_function();
          }
          // looks like this either wasn't a Proxy_Function_Impl or the types didn't match
          // we cannot make any other guesses or assumptions really, so continuing
        }
      */

        return std::function<Ret(Params...)>(Build_Function_Caller_Helper<Ret, Params...>(funcs, t_conversions ? t_conversions->get() : nullptr));
    }
} // namespace chaiscript::dispatch::detail
namespace chaiscript {
    namespace dispatch {
        namespace detail {
            template<typename Ret, typename... Param>
            constexpr auto arity(Ret(*)(Param...)) noexcept {
                return sizeof...(Param);
            }
        } // namespace detail

        /// Build a function caller that knows how to dispatch on a set of functions
        /// example:
        /// std::function<void (int)> f =
        ///      build_function_caller(dispatchkit.get_function("print"));
        /// \returns A std::function object for dispatching
        /// \param[in] funcs the set of functions to dispatch on.
        template<typename FunctionType>
        std::function<FunctionType> functor(const std::vector<Const_Proxy_Function>& funcs, const Type_Conversions_State* t_conversions) {
            const bool has_arity_match = std::any_of(funcs.begin(), funcs.end(), [](const Const_Proxy_Function& f) {
                return f->get_arity() == -1 || size_t(f->get_arity()) == detail::arity(static_cast<FunctionType*>(nullptr));
                });

            if (!has_arity_match) {
                throw exception::bad_boxed_cast(user_type<Const_Proxy_Function>(), typeid(std::function<FunctionType>));
            }

            FunctionType* p = nullptr;
            return detail::build_function_caller_helper(p, funcs, t_conversions);
        }

        /// Build a function caller for a particular Proxy_Function object
        /// useful in the case that a function is being pass out from scripting back
        /// into code
        /// example:
        /// void my_function(Proxy_Function f)
        /// {
        ///   std::function<void (int)> local_f =
        ///      build_function_caller(f);
        /// }
        /// \returns A std::function object for dispatching
        /// \param[in] func A function to execute.
        template<typename FunctionType>
        std::function<FunctionType> functor(Const_Proxy_Function func, const Type_Conversions_State* t_conversions) {
            return functor<FunctionType>(std::vector<Const_Proxy_Function>({ std::move(func) }), t_conversions);
        }

        /// Helper for automatically unboxing a Boxed_Value that contains a function object
        /// and creating a typesafe C++ function caller from it.
        template<typename FunctionType>
        std::function<FunctionType> functor(const Boxed_Value& bv, const Type_Conversions_State* t_conversions) {
            return functor<FunctionType>(boxed_cast<Const_Proxy_Function>(bv, t_conversions), t_conversions);
        }
    } // namespace dispatch

    namespace detail {
        /// Cast helper to handle automatic casting to const std::function &
        template<typename Signature>
        struct Cast_Helper<const std::function<Signature>&> {
            static std::function<Signature> cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                if (ob.get_type_info().bare_equal(user_type<Const_Proxy_Function>())) {
                    return dispatch::functor<Signature>(ob, t_conversions);
                }
                else {
                    return Cast_Helper_Inner<const std::function<Signature>&>::cast(ob, t_conversions);
                }
            }
        };

        /// Cast helper to handle automatic casting to std::function
        template<typename Signature>
        struct Cast_Helper<std::function<Signature>> {
            static std::function<Signature> cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                if (ob.get_type_info().bare_equal(user_type<Const_Proxy_Function>())) {
                    return dispatch::functor<Signature>(ob, t_conversions);
                }
                else {
                    return Cast_Helper_Inner<std::function<Signature>>::cast(ob, t_conversions);
                }
            }
        };

        /// Cast helper to handle automatic casting to const std::function
        template<typename Signature>
        struct Cast_Helper<const std::function<Signature>> {
            static std::function<Signature> cast(const Boxed_Value& ob, const Type_Conversions_State* t_conversions) {
                if (ob.get_type_info().bare_equal(user_type<Const_Proxy_Function>())) {
                    return dispatch::functor<Signature>(ob, t_conversions);
                }
                else {
                    return Cast_Helper_Inner<const std::function<Signature>>::cast(ob, t_conversions);
                }
            }
        };
    } // namespace detail
} // namespace chaiscript
namespace chaiscript {
    namespace detail {
        template<typename T>
        constexpr T* get_pointer(T* t) noexcept {
            return t;
        }

        template<typename T>
        T* get_pointer(const std::reference_wrapper<T>& t) noexcept {
            return &t.get();
        }

        template<typename O, typename Ret, typename P1, typename... Param>
        constexpr auto bind_first(Ret(*f)(P1, Param...), O&& o) {
            return[f, o = std::forward<O>(o)](Param... param)->Ret { return f(o, std::forward<Param>(param)...); };
        }

        template<typename O, typename Ret, typename Class, typename... Param>
        constexpr auto bind_first(Ret(Class::* f)(Param...), O&& o) {
            return[f, o = std::forward<O>(o)](Param... param)->Ret { return (get_pointer(o)->*f)(std::forward<Param>(param)...); };
        }

        template<typename O, typename Ret, typename Class, typename... Param>
        constexpr auto bind_first(Ret(Class::* f)(Param...) const, O&& o) {
            return[f, o = std::forward<O>(o)](Param... param)->Ret { return (get_pointer(o)->*f)(std::forward<Param>(param)...); };
        }

        template<typename O, typename Ret, typename P1, typename... Param>
        auto bind_first(const std::function<Ret(P1, Param...)>& f, O&& o) {
            return[f, o = std::forward<O>(o)](Param... param)->Ret { return f(o, std::forward<Param>(param)...); };
        }

        template<typename F, typename O, typename Ret, typename Class, typename P1, typename... Param>
        constexpr auto bind_first(const F& fo, O&& o, Ret(Class::* f)(P1, Param...) const) {
            return[fo, o = std::forward<O>(o), f](Param... param)->Ret { return (fo.*f)(o, std::forward<Param>(param)...); };
        }

        template<typename F, typename O>
        constexpr auto bind_first(const F& f, O&& o) {
            return bind_first(f, std::forward<O>(o), &F::operator());
        }

    } // namespace detail
} // namespace chaiscript
namespace chaiscript::dispatch::detail {
    template<typename... Param>
    struct Function_Params {
    };

    template<typename Ret, typename Params, bool IsNoExcept = false, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
    struct Function_Signature {
        using Param_Types = Params;
        using Return_Type = Ret;
        constexpr static const bool is_object = IsObject;
        constexpr static const bool is_member_object = IsMemberObject;
        constexpr static const bool is_noexcept = IsNoExcept;
        template<typename T>
        constexpr Function_Signature(T&&) noexcept {
        }
        constexpr Function_Signature() noexcept = default;
    };

    // Free functions

    template<typename Ret, typename... Param>
    Function_Signature(Ret(*f)(Param...))->Function_Signature<Ret, Function_Params<Param...>>;

    template<typename Ret, typename... Param>
    Function_Signature(Ret(*f)(Param...) noexcept)->Function_Signature<Ret, Function_Params<Param...>, true>;

    // no reference specifier

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile)->Function_Signature<Ret, Function_Params<volatile Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile noexcept)
        ->Function_Signature<Ret, Function_Params<volatile Class&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile const)
        ->Function_Signature<Ret, Function_Params<volatile const Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile const noexcept)
        ->Function_Signature<Ret, Function_Params<volatile const Class&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...))->Function_Signature<Ret, Function_Params<Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) noexcept)->Function_Signature<Ret, Function_Params<Class&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) const)->Function_Signature<Ret, Function_Params<const Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) const noexcept)->Function_Signature<Ret, Function_Params<const Class&, Param...>, true, true>;

    // & reference specifier

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile&)->Function_Signature<Ret, Function_Params<volatile Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile& noexcept)
        ->Function_Signature<Ret, Function_Params<volatile Class&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile const&)
        ->Function_Signature<Ret, Function_Params<volatile const Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile const& noexcept)
        ->Function_Signature<Ret, Function_Params<volatile const Class&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...)&)->Function_Signature<Ret, Function_Params<Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) & noexcept)->Function_Signature<Ret, Function_Params<Class&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) const&)->Function_Signature<Ret, Function_Params<const Class&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) const& noexcept)->Function_Signature<Ret, Function_Params<const Class&, Param...>, true, true>;

    // && reference specifier

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile&&)->Function_Signature<Ret, Function_Params<volatile Class&&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile&& noexcept)
        ->Function_Signature<Ret, Function_Params<volatile Class&&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
        ->Function_Signature<Ret, Function_Params<volatile const Class&&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) volatile const&& noexcept)
        ->Function_Signature<Ret, Function_Params<volatile const Class&&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...)&&)->Function_Signature<Ret, Function_Params<Class&&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) && noexcept)->Function_Signature<Ret, Function_Params<Class&&, Param...>, true, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) const&&)->Function_Signature<Ret, Function_Params<const Class&&, Param...>, false, true>;

    template<typename Ret, typename Class, typename... Param>
    Function_Signature(Ret(Class::* f)(Param...) const&& noexcept)
        ->Function_Signature<Ret, Function_Params<const Class&&, Param...>, true, true>;

    template<typename Ret, typename Class>
    Function_Signature(Ret Class::* f)->Function_Signature<Ret, Function_Params<Class&>, true, true, true>;

    // primary template handles types that have no nested ::type member:
    template<class, class = std::void_t<>>
    struct has_call_operator : std::false_type {
    };

    // specialization recognizes types that do have a nested ::type member:
    template<class T>
    struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {
    };

    template<typename Func>
    auto function_signature(const Func& f) {
        if constexpr (has_call_operator<Func>::value) {
            return Function_Signature<typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
                typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Param_Types,
                decltype(Function_Signature{ &std::decay_t<Func>::operator() })::is_noexcept,
                false,
                false,
                true > {};
        }
        else {
            return Function_Signature{ f };
        }
    }

} // namespace chaiscript::dispatch::detail
namespace chaiscript {
    namespace dispatch::detail {
        template<typename Obj, typename Param1, typename... Rest>
        Param1 get_first_param(Function_Params<Param1, Rest...>, Obj&& obj) {
            return static_cast<Param1>(std::forward<Obj>(obj));
        }

        template<typename Func, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject, bool Is_Object, typename Ret, typename... Param>
        auto make_callable_impl(Func&& func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, Is_Object>) {
            if constexpr (Is_MemberObject) {
                // we now that the Param pack will have only one element, so we are safe expanding it here
                return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Attribute_Access<Ret, std::decay_t<Param>...>>(
                    std::forward<Func>(func)));
            }
            else if constexpr (Is_Member) {
                // TODO some kind of bug is preventing forwarding of this noexcept for the lambda
                auto call = [func = std::forward<Func>(func)](auto&& obj, auto &&...param) /* noexcept(Is_Noexcept) */ -> decltype(auto) {
                    return ((get_first_param(Function_Params<Param...>{}, obj).*func)(std::forward<decltype(param)>(param)...));
                };
                return Proxy_Function(
                    chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret(Param...), decltype(call)>>(
                        std::move(call)));
            }
            else {
                return Proxy_Function(
                    chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret(Param...), std::decay_t<Func>>>(
                        std::forward<Func>(func)));
            }
        }

        // this version peels off the function object itself from the function signature, when used
        // on a callable object
        template<typename Func, typename Ret, typename Object, typename... Param, bool Is_Noexcept>
        auto make_callable(Func&& func, Function_Signature<Ret, Function_Params<Object, Param...>, Is_Noexcept, false, false, true>) {
            return make_callable_impl(std::forward<Func>(func), Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, false, false, true>{});
        }

        // this version peels off the function object itself from the function signature, when used
        // on a callable object
        //template<typename Func, typename Ret, typename Object, typename... Param, bool Is_Noexcept>
        //auto make_callable_attr(Func&& func, Function_Signature<Ret, Function_Params<Object, Param...>, Is_Noexcept, false, false, true>) {
        //    return make_callable_impl(std::forward<Func>(func), Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, true, true, true>{});
        //}

        template<typename Func, typename Ret, typename... Param, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject>
        auto make_callable(Func&& func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, false> fs) {
            return make_callable_impl(std::forward<Func>(func), fs);
        }

        //template<typename Func, typename Ret, typename... Param, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject>
        //auto make_callable_attr(Func&& func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, false> fs) {
        //    return make_callable_impl(std::forward<Func>(func), fs);
        //}
    } // namespace dispatch::detail

    /// \brief Creates a new Proxy_Function object from a free function, member function or data member
    /// \param[in] t Function / member to expose
    ///
    /// \b Example:
    /// \code
    /// int myfunction(const std::string &);
    /// class MyClass
    /// {
    ///   public:
    ///     void memberfunction();
    ///     int memberdata;
    /// };
    ///
    /// chaiscript::ChaiScript chai;
    /// chai.add(fun(&myfunction), "myfunction");
    /// chai.add(fun(&MyClass::memberfunction), "memberfunction");
    /// chai.add(fun(&MyClass::memberdata), "memberdata");
    /// \endcode
    ///
    /// \sa \ref adding_functions
    template<typename T>
    Proxy_Function fun(T&& t) {
        return dispatch::detail::make_callable(std::forward<T>(t), dispatch::detail::function_signature(t));
    }

    //template<typename T>
    //Proxy_Function attr_fun(T&& t) {
    //    return dispatch::detail::make_callable_attr(std::forward<T>(t), dispatch::detail::function_signature(t));
    //}

#if 0
    /// \brief Creates a new Proxy_Function object from a free function, member function or data member and binds the first parameter of it
    /// \param[in] t Function / member to expose
    /// \param[in] q Value to bind to first parameter
    ///
    /// \b Example:
    /// \code
    /// struct MyClass
    /// {
    ///   void memberfunction(int);
    /// };
    ///
    /// MyClass obj;
    /// chaiscript::ChaiScript chai;
    /// // Add function taking only one argument, an int, and permanently bound to "obj"
    /// chai.add(fun(&MyClass::memberfunction, std::ref(obj)), "memberfunction");
    /// \endcode
    ///
    /// \sa \ref adding_functions
    template<typename T, typename Q>
    Proxy_Function fun(T&& t, const Q& q) {
        return fun(detail::bind_first(std::forward<T>(t), q));
    }
#endif

} // namespace chaiscript
