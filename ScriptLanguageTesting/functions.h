#pragma once
#include "types.h"
#include "Parallel.h"

namespace GL {

    // function name, function qualifiers (e.g. static), return type, argument types, and (optionally) argument default values. 
    class function_signature {
    public:
        enum function_state {
            Normal = 0, // default -- meaningless. 
            Static = 1, // whether the function is a static function or not. Non-static implies it is a member-function. 
            Constant = 2, // whether this function commits to making no changes to the underlying object. Often, const should also be async, but not always. 
            Async = 4, // whether this function can be safely called asynchronously or not
            Template = 8, // whether the function is a template 
            Explicit = 16, // whether the function is explicit and the input params must exactly match (does not allow conversion)
            Cached = 32 // whether the function is a cache from another function, for performance reasons. 
        };

    public:
        size_t
            state_m;
        std::vector<GL::any::fast_any>
            argument_defaults_m;
        std::vector<GL::string>
            argument_names_m;
        std::vector<GL::type>
            argument_types_m;
        GL::string
            name_m;
        GL::type
            returns_m;

    public:

        function_signature() = default;
        // The provided defaults are scooted to the end of the argument list, such as:
           //  argA, argB, argC = default1, argD = default2;        
        function_signature(GL::string const& name, GL::type returns, std::vector<std::pair<GL::string, GL::type>> const& args, std::vector<GL::any> const& defaults)
            : name_m{ name }
            , returns_m{ returns }
            , state_m(function_state::Normal)
        {
            for (auto& x : defaults)
                argument_defaults_m.push_back(x.fast());

            for (auto& arg : args) {
                argument_names_m.push_back(arg.first);
                argument_types_m.push_back(arg.second);
            }

            bool found_non_void = false;
            for (size_t index = 0; index < argument_defaults_m.size(); index++) {
                if (!argument_defaults_m[index].empty()) {
                    found_non_void = true;
                }
                else if (found_non_void) {
                    argument_defaults_m.resize(index);
                }
            }
            while (argument_defaults_m.size() < argument_types_m.size()) argument_defaults_m.insert(argument_defaults_m.begin(), GL::any::fast_any{});
            // all non-void defaults are guarranteed to be at the end. 

            evaluate_if_template_function();
        };
        function_signature(function_signature const&) = default;
        function_signature(function_signature&&) = default;
        function_signature& operator=(function_signature const&) = default;
        function_signature& operator=(function_signature&&) = default;
        ~function_signature() = default;

        // reviews the arguments to see if any are defined as "any". If so, sets the state of the function as "template". Otherwise, unsets the template state. 
        void evaluate_if_template_function() {
            for (auto& t : argument_types_m) {
                if (t.is_any()) {
                    state_m |= function_state::Template;
                    return;
                }
            }
            state_m &= ~function_state::Template; // unsets the template flag
        }
        template<typename iter_type> bool can_call_with_free_cast(iter_type iter, iter_type const& end) const {
            static_assert(std::is_same_v<iter_type::value_type, GL::type>, "iterator must be for a GL::type class");

            size_t i = 0;
            for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                if (!iter->can_free_cast(argument_types_m[i])) {
                    return false;
                }
            }
            for (; i < argument_defaults_m.size(); ++i) {
                if (argument_defaults_m[i].m_casted_type.is_void()) {
                    return false;
                }
            }
            return true;
        };
        template<typename iter_type> bool can_call_with_cast(iter_type iter, iter_type const& end) const {
            static_assert(std::is_same_v<iter_type::value_type, GL::type>, "iterator must be for a GL::type class");
            size_t i = 0;
            for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                if (!iter->can_cast(argument_types_m[i])) {
                    return false;
                }
            }
            for (; i < argument_defaults_m.size(); ++i) {
                if (argument_defaults_m[i].m_casted_type.is_void()) {
                    return false;
                }
            }
            return true;
        };
        bool can_call_with_free_cast(std::vector<GL::type> const& from) const {
            return can_call_with_free_cast(from.begin(), from.end());
        };
        bool can_call_with_cast(std::vector<GL::type> const& from) const {
            return can_call_with_cast(from.begin(), from.end());
        };
    };

    namespace details {
        /* Pure virtual base class for all Proxy_Function implementations
        Proxy_Functions are a type erasure of type-safe C++ function calls.
        At runtime parameter types are expected to be tested against passed in types.
        Dispatch_Engine only knows how to work with Proxy_Function, no other
        function classes */
        class Proxy_Function_Base {
        public:
            virtual ~Proxy_Function_Base() = default;

            function_signature 
                m_signature;

            // fast path
            template <typename iter_type> any operator()(iter_type begin, iter_type const& end) const {
                static_assert(std::is_same_v<iter_type::value_type, GL::any::fast_any>, "iterator must be for a GL::any::fast_any class");
                static thread_local std::array<any::fast_any*, 16> inputs;
                std::memset(&inputs[0], 0, sizeof(inputs));
                short pos{ 0 };
                for (; (begin != end) && (pos < 16); ++begin, ++pos) {
                    inputs[pos] = const_cast<any::fast_any*>(&*begin);
                }
                for (; (pos < 16) && (pos < m_signature.argument_defaults_m.size()); ++pos) {
                    inputs[pos] = const_cast<any::fast_any*>(&m_signature.argument_defaults_m[pos]);
                }
                return do_call(&inputs[0]);
            };
            // fast path
            any operator()(std::vector<any::fast_any>& params) const {
                return operator()(params.begin(), params.end());
            };
            // convenience path, requires casting to any::fast_any
            any operator()(const std::vector<any>& params) const {
                std::vector<any::fast_any> Params;
                Params.resize(params.size());
                std::transform(params.begin(), params.end(), Params.begin(), [](any const& from) { return from.fast(); });
                return operator()(Params.begin(), Params.end());
            };

            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const = 0;

        protected:
            virtual any do_call(any::fast_any** begin) const = 0;
            Proxy_Function_Base(function_signature&& p_signature) : m_signature(std::move(p_signature)) {}

        };
    };
    typedef GL::shared_ptr<details::Proxy_Function_Base> Proxy_Function;

    namespace details {        
        auto cast_any(any::fast_any* const& p) { return p->cast(); }
        template <typename F, std::size_t... Is> auto unpack_and_call_helper_returns(F const& func, any::fast_any** arr_ptr, std::index_sequence<Is...>) {
            return func(cast_any(arr_ptr[Is])...);
        };
        template <typename F, std::size_t... Is> auto unpack_and_call_helper(F const& func, any::fast_any** arr_ptr, std::index_sequence<Is...>) {
            func(cast_any(arr_ptr[Is])...);
        };
        template <typename F, bool returns, std::size_t N> auto unpack_and_call(F const& func, any::fast_any** arr_ptr) {
            if constexpr (returns) {
                return unpack_and_call_helper_returns(func, arr_ptr, std::make_index_sequence<N>{});
            }
            else {
                unpack_and_call_helper(func, arr_ptr, std::make_index_sequence<N>{});
            }
        }

        /* Use to call function objects */
        template <class Callable>
        class Explicit_Function_Impl final : public Proxy_Function_Base {
        public:
            using argType = typename parallel::impl::function_traits<decltype(std::function(std::declval<Callable>()))>::arguments;
            using returnType = typename parallel::impl::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>> 
                    args;
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature("", GL::type_of<returnType>(), args, std::move(defaults));
            };
            Explicit_Function_Impl(Explicit_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , F_m(from.F_m)
            {};

        public:
            Explicit_Function_Impl(Callable F_p, std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , F_m(std::move(F_p))
            {};
            virtual ~Explicit_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Explicit_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        protected:
            virtual any do_call(any::fast_any** begin) const override {
                if constexpr (std::is_same_v<returnType, void>) {
                    unpack_and_call<Callable, false, numArgs>(F_m, begin);
                    return any{};                    
                }
                else {
                    return unpack_and_call<Callable, true, numArgs>(F_m, begin);
                }
            };
            Callable F_m;
        };

        /* Use to call function objects */
        template <typename T, class Class>
        class Attribute_Access_Impl final : public Proxy_Function_Base {
        public:
            using actualT = typename std::decay_t<typename GL::type_erasure::get_type<T>::type>;

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
                args.push_back({ "parent", GL::type_of<const Class&>() });
                return function_signature("", GL::type_of<actualT&>(), args, std::move(defaults));
            };

        protected:
            Attribute_Access_Impl(Attribute_Access_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , m_attr(from.m_attr)
            {};

        public:
            Attribute_Access_Impl(T Class::* t_attr, std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , m_attr(t_attr)
            {};
            virtual ~Attribute_Access_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Attribute_Access_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        protected:
            __declspec(noinline) virtual any do_call(any::fast_any** begin) const override {
                if constexpr (std::is_same_v<T, void>) {
                    return any{};
                }
                else if constexpr (std::is_same_v<any, T>) {
                    return begin[0]->cast<Class*>()->*m_attr;
                }
                else if constexpr (std::is_pointer<T>::value) {
                    GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();
                    auto out = GL::shared_ptr<actualT>(GL::shared_ptr<void>(ptr));
                    out.set_pointer_without_modifying_control_block((*ptr).*m_attr);
                    return out;
                }
                else {
                    GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();         
                    auto out = GL::shared_ptr<actualT>(GL::shared_ptr<void>(ptr));
                    out.set_pointer_without_modifying_control_block(&(ptr.get()->*m_attr));
                    return out;
                }
            };
            T Class::* m_attr;
        };

        /**
         * Use to call member functions:
         * struct Test{ public: std::string attr(){ return "TEST"; }; }
         * var& func = details::Attribute_Access_Impl(&Test::attr);
         * assert(func(Test{}).cast<std::string>() == "TEST");
        */
        template <typename R, typename... T>
        class Static_Function_Impl final : public Proxy_Function_Base {
        public:
            using argType = std::tuple<T...>;
            using returnType = typename std::decay_t<typename GL::type_erasure::get_type<R>::type>;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature("", GL::type_of<returnType>(), args, std::move(defaults));
            };
            Static_Function_Impl(Static_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , F_m(from.F_m)
            {};

        public:
            Static_Function_Impl(R(*f)(T...), std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , F_m(std::move(f)) {};
            virtual ~Static_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Static_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        protected:
            virtual any do_call(any::fast_any** begin) const override {
                if constexpr (std::is_same_v<returnType, void>) {
                    unpack_and_call<R(*)(T...), false, numArgs>(F_m, begin);
                    return any{};
                }
                else {
                    return unpack_and_call<R(*)(T...), true, numArgs>(F_m, begin);
                }
            };
            R(*F_m)(T...);
        };

        template <typename R, typename Class, typename... T>
        class Default_Member_Function_Impl : public Proxy_Function_Base {
        public:
            using argType = std::tuple<T...>;
            using returnType = typename std::decay_t<typename GL::type_erasure::get_type<R>::type>;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
                args.push_back({ "parent", GL::type_of<const Class&>() });
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature("", GL::type_of<returnType>(), args, std::move(defaults));
            };
            Default_Member_Function_Impl(Default_Member_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , m_attr(from.m_attr)
            {};

        public:
            Default_Member_Function_Impl(R(Class::* f)(T...), std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , m_attr(std::move(f)) {};
            virtual ~Default_Member_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Default_Member_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        private:
            __declspec(noinline) virtual any do_call(any::fast_any** begin) const override {
                if (Class* parent = begin[0]->cast<Class*>()) {
                    if constexpr (numArgs == 15) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                        );
                    }
                    else if constexpr (numArgs == 14) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                        );
                    }
                    else if constexpr (numArgs == 13) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                        );
                    }
                    else if constexpr (numArgs == 12) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                        );
                    }
                    else if constexpr (numArgs == 11) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                        );
                    }
                    else if constexpr (numArgs == 10) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                        );
                    }
                    else if constexpr (numArgs == 9) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                        );
                    }
                    else if constexpr (numArgs == 8) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                        );
                    }
                    else if constexpr (numArgs == 7) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                        );
                    }
                    else if constexpr (numArgs == 6) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                        );
                    }
                    else if constexpr (numArgs == 5) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                        );
                    }
                    else if constexpr (numArgs == 4) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                        );
                    }
                    else if constexpr (numArgs == 3) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                        );
                    }
                    else if constexpr (numArgs == 2) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                        );
                    }
                    else if constexpr (numArgs == 1) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast()
                        );
                        else return (parent->*m_attr)(
                            begin[1]->cast()
                        );
                    }
                    else if constexpr (numArgs == 0) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)();
                        else return (parent->*m_attr)();
                    }
                }
                return any{};
            };

            R(Class::* m_attr)(T...);
        };

        template <typename R, typename Class, typename... T>
        class Const_Member_Function_Impl : public Proxy_Function_Base {
        public:
            using argType = std::tuple<T...>;
            using returnType = typename std::decay_t<typename GL::type_erasure::get_type<R>::type>;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
                args.push_back({ "parent", GL::type_of<const Class&>() });
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature("", GL::type_of<returnType>(), args, std::move(defaults));
            };
            Const_Member_Function_Impl(Const_Member_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , m_attr(from.m_attr)
            {};

        public:
            Const_Member_Function_Impl(R(Class::* f)(T...) const, std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , m_attr(std::move(f)) {};
            virtual ~Const_Member_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Const_Member_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        private:
            __declspec(noinline) virtual any do_call(any::fast_any** begin) const override {
                if (const Class* parent = begin[0]->cast<const Class*>()) {
                    if constexpr (numArgs == 15) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                            );
                    }
                    else if constexpr (numArgs == 14) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                            );
                    }
                    else if constexpr (numArgs == 13) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                            );
                    }
                    else if constexpr (numArgs == 12) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                            );
                    }
                    else if constexpr (numArgs == 11) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                            );
                    }
                    else if constexpr (numArgs == 10) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                            );
                    }
                    else if constexpr (numArgs == 9) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                            );
                    }
                    else if constexpr (numArgs == 8) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                            );
                    }
                    else if constexpr (numArgs == 7) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                            );
                    }
                    else if constexpr (numArgs == 6) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                            );
                    }
                    else if constexpr (numArgs == 5) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                            );
                    }
                    else if constexpr (numArgs == 4) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                            );
                    }
                    else if constexpr (numArgs == 3) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                            );
                    }
                    else if constexpr (numArgs == 2) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                            );
                    }
                    else if constexpr (numArgs == 1) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast()
                            );
                    }
                    else if constexpr (numArgs == 0) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)();
                        else return (parent->*m_attr)();
                    }
                }
                return any{};
            };

            R(Class::* m_attr)(T...) const;
        };

        namespace detail {
            template <typename T>
            struct is_static_member_function : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...)> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const volatile> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) volatile> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...)&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...)&&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const&&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) noexcept> : std::true_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const noexcept> : std::true_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) volatile noexcept> : std::true_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const volatile noexcept> : std::true_type {};


            template<typename... Param> struct Function_Params {
                typedef std::tuple<Param...> argType;
                static constexpr auto numArgs = std::tuple_size_v<argType>;
            };

            template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
            struct Function_Signature {
                typedef Params Param_Types;
                typedef Class Class_Type;
                typedef Ret Return_Type;

                constexpr static const bool is_object = IsObject; // e.g. lambda object
                constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
                constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
                constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
                constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

                template<typename T> constexpr Function_Signature(T&&) noexcept { };
                constexpr Function_Signature() noexcept = default;
            };

            // Free functions
            template<typename Ret, typename... Param>
            Function_Signature(Ret(*f)(Param...))
                ->Function_Signature<Ret, void, Function_Params<Param...>>; // static function

            // no reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...))
                ->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const)
                ->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

            // & reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile&)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const&)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...)&)
                ->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const&)
                ->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

            // && reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile&&)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...)&&)
                ->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const&&)
                ->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true>; // member function

            template<typename Ret, typename Class>
            Function_Signature(Ret Class::* f)
                ->Function_Signature<Ret, Class, Function_Params<Class&>, true, true>; // member object

            // primary template handles types that have no nested ::type member:
            template<class, class = std::void_t<>>
            struct has_call_operator : std::false_type {};

            // specialization recognizes types that do have a nested ::type member:
            template<class T>
            struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

            template<typename Func>
            auto function_signature(Func const& f) {
                if constexpr (has_call_operator<Func>::value) {
                    return Function_Signature<
                        typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
                        typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Class_Type,
                        typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Param_Types,
                        false,
                        false,
                        true
                    > {};
                }
                else {
                    return Function_Signature{ f };
                }
            };

            template<typename Obj, typename Param1, typename... Rest>
            Param1 get_first_param(Function_Params<Param1, Rest...>, Obj&& obj) {
                return static_cast<Param1>(std::forward<Obj>(obj));
            };

        } // namespace chaiscript::dispatch::detail

        template<typename Ret, typename Class, typename... Param>
        Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) const, std::vector<any>&& defaults) {
            auto* function_impl = new Const_Member_Function_Impl(f, std::move(defaults));
            function_impl->m_signature.state_m |= function_signature::Constant;
            // function_impl->m_signature.state_m |= function_signature::Async; // const member functions (e.g. std::string::length) are assumed to be async-friendly. 
            auto ptr{ GL::static_pointer_cast<Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
            return ptr;
        };
        
        template<typename Ret, typename Class, typename... Param>
        Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...), std::vector<any>&& defaults) {
            auto* function_impl = new Default_Member_Function_Impl(f, std::move(defaults));
            auto ptr{ GL::static_pointer_cast<Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
            return ptr;
        };
    };

    // Convert nearly any function or function pointer to a callable, generic proxy function. 
    template<typename Func> Proxy_Function make_callable(Func&& func, size_t stateModifier = 0, std::vector<any>&& defaults = {}) {
        typedef decltype(details::detail::function_signature(func)) function_header;
        if constexpr (function_header::is_object) { // function objects, e.g. auto x = [](){};            
            auto* function_impl = new details::Explicit_Function_Impl(std::move(func), std::move(defaults));
            function_impl->m_signature.state_m |= function_signature::Static;
            function_impl->m_signature.state_m |= function_signature::Constant;
            //function_impl->m_signature.state |= FunctionState::Async; // static functions are assumed to be async-friendly. 
            return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
        }
        else if constexpr (function_header::is_member_object) { // member objects, e.g. return object.member;            
            auto* function_impl = new details::Attribute_Access_Impl(std::move(func), std::move(defaults));
            function_impl->m_signature.state_m |= function_signature::Constant; // accessing a member object is assumed to be constant -- it does not necessarily change anything just to "look".
            function_impl->m_signature.state_m |= function_signature::Async; // accessing a member object is assumed to be async-friendly.
            return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
        }
        else if constexpr (function_header::is_member && !function_header::is_member_object) { // member functions, e.g. return object.member();
            return details::Member_Function_Impl(std::move(func), std::move(defaults));
        }
        else if constexpr (function_header::is_static_member_function) { // static function pointers, e.g. static foo(){};            
            auto* function_impl = new details::Static_Function_Impl(std::move(func), std::move(defaults));
            function_impl->m_signature.state_m |= function_signature::Static;
            function_impl->m_signature.state_m |= function_signature::Constant;
            //function_impl->m_signature.state_m |= function_signature::Async; // static functions are assumed to be async-friendly. 
            return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
        }
        else {
            throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
        }
    };

};