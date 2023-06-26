#pragma once
#include "Precompiled.h"

namespace chaiscript::dispatch::detail {
    template<typename Class, typename... Params>
    Proxy_Function build_constructor_(Class(*)(Params...)) {
        if constexpr (!std::is_copy_constructible_v<Class>) {
            auto call = [](auto &&...param) { return chaiscript::make_shared<Class>(std::forward<decltype(param)>(param)...); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base,
                dispatch::Proxy_Function_Callable_Impl<chaiscript::shared_ptr<Class>(Params...), decltype(call)>>(call));
        }
        else if constexpr (true) {
            auto call = [](auto &&...param) { return Class(std::forward<decltype(param)>(param)...); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Class(Params...), decltype(call)>>(
                    call));
        }
    }
} // namespace chaiscript::dispatch::detail

namespace chaiscript::dispatch::detail {
    template<typename Class, typename Class2>
    Proxy_Function build_base_class_with_derived() {
        if constexpr (!std::is_copy_constructible_v<Class>) {
            auto call = []() { return chaiscript::make_shared<Class>(Class2()); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base,
                dispatch::Proxy_Function_Callable_Impl<chaiscript::shared_ptr<Class>(), decltype(call)>>(call));
        }
        else if constexpr (true) {
            auto call = [](auto &&...param) { return Class(Class2()); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Class(), decltype(call)>>(
                    call));
        }
    }
} // namespace chaiscript::dispatch::detail

namespace chaiscript {
    /// \brief Generates a constructor function for use with ChaiScript
    ///
    /// \tparam T The signature of the constructor to generate. In the form of: ClassType (ParamType1, ParamType2, ...)
    ///
    /// Example:
    /// \code
    ///    chaiscript::ChaiScript chai;
    ///    // Create a new function that creates a MyClass object using the (int, float) constructor
    ///    // and call that function "MyClass" so that it appears as a normal constructor to the user.
    ///    chai.add(constructor<MyClass (int, float)>(), "MyClass");
    /// \endcode
    template<typename T>
    Proxy_Function constructor() {
        T* f = nullptr;
        return (dispatch::detail::build_constructor_(f));
    }

    template<typename T, typename T2>
    Proxy_Function constructor_from_derived() {
        return (dispatch::detail::build_base_class_with_derived<T,T2>());
    }
} // namespace chaiscript