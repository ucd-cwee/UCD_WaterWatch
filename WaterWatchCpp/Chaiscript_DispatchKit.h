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
    namespace parser {
        class ChaiScript_Parser_Base;
    }
    namespace dispatch {
        class Dynamic_Proxy_Function;
        class Proxy_Function_Base;
        struct Placeholder_Object;
    } // namespace dispatch
} // namespace chaiscript

namespace chaiscript {
    namespace exception {
        /// Exception thrown in the case that an object name is invalid because it is a reserved word
        class reserved_word_error : public std::runtime_error {
        public:
            explicit reserved_word_error(const std::string& t_word) noexcept
                : std::runtime_error("Reserved word not allowed in object name: " + t_word)
                , m_word(t_word) {
            }

            reserved_word_error(const reserved_word_error&) = default;

            ~reserved_word_error() noexcept override = default;

            std::string word() const { return m_word; }

        private:
            std::string m_word;
        };

        /// Exception thrown in the case that an object name is invalid because it contains illegal characters
        class illegal_name_error : public std::runtime_error {
        public:
            explicit illegal_name_error(const std::string& t_name) noexcept
                : std::runtime_error("Reserved name not allowed in object name: " + t_name)
                , m_name(t_name) {
            }

            illegal_name_error(const illegal_name_error&) = default;

            ~illegal_name_error() noexcept override = default;

            std::string name() const { return m_name; }

        private:
            std::string m_name;
        };

        /// Exception thrown in the case that an object name is invalid because it already exists in current context
        class name_conflict_error : public std::runtime_error {
        public:
            explicit name_conflict_error(const std::string& t_name) noexcept
                : std::runtime_error("Name already exists in current context " + t_name)
                , m_name(t_name) {
            }

            name_conflict_error(const name_conflict_error&) = default;

            ~name_conflict_error() noexcept override = default;

            std::string name() const { return m_name; }

        private:
            std::string m_name;
        };

        /// Exception thrown in the case that a non-const object was added as a shared object
        class global_non_const : public std::runtime_error {
        public:
            global_non_const() noexcept
                : std::runtime_error("a global object must be const") {
            }

            global_non_const(const global_non_const&) = default;
            ~global_non_const() noexcept override = default;
        };
    } // namespace exception

    /// \brief Holds a collection of ChaiScript settings which can be applied to the ChaiScript runtime.
    ///        Used to implement loadable module support.
    class Module {
    public:
        Module& add(Type_Info ti, std::string name) {
            m_typeinfos.emplace_back(ti, std::move(name));
            return *this;
        }

        Module& add(Type_Conversion d) {
            m_conversions.push_back(std::move(d));
            return *this;
        }

        Module& add(Proxy_Function f, std::string name) {
            m_funcs.emplace_back(std::move(f), std::move(name));
            return *this;
        }

        Module& add(Postfix_Definition f) {
            m_postfixes.push_back(std::move(f));
            return *this;
        }
        Module& add(ContainerValueType_Definition f) {
            m_containerValueTypes.push_back(std::move(f));
            return *this;
        }

        Module& add_global_const(Boxed_Value t_bv, std::string t_name) {
            if (!t_bv.is_const()) { throw chaiscript::exception::global_non_const(); }

            m_globals.emplace_back(std::move(t_bv), std::move(t_name));
            return *this;
        }

        // Add a bit of ChaiScript to eval during module implementation
        Module& eval(std::string str) {
            m_evals.push_back(std::move(str));
            return *this;
        }

        template<typename Eval, typename Engine>
        void apply(Eval& t_eval, Engine& t_engine) const {
            apply(m_typeinfos.begin(), m_typeinfos.end(), t_engine);
            apply(m_funcs.begin(), m_funcs.end(), t_engine);
            apply_postfix(m_postfixes.begin(), m_postfixes.end(), t_engine);
            apply_containerValueTypes(m_containerValueTypes.begin(), m_containerValueTypes.end(), t_engine);
            apply_eval(m_evals.begin(), m_evals.end(), t_eval);
            apply_single(m_conversions.begin(), m_conversions.end(), t_engine);
            apply_globals(m_globals.begin(), m_globals.end(), t_engine);
        }

        bool has_function(const Proxy_Function& new_f, std::string_view name) noexcept {
            return std::any_of(m_funcs.begin(), m_funcs.end(), [&](const std::pair<Proxy_Function, std::string>& existing_f) {
                return existing_f.second == name && *(existing_f.first) == *(new_f);
                });
        }

    private:
        chaiscript::small_vector<std::pair<Type_Info, std::string>> m_typeinfos;
        chaiscript::small_vector<std::pair<Proxy_Function, std::string>> m_funcs;
        chaiscript::small_vector<Postfix_Definition> m_postfixes;
        chaiscript::small_vector<ContainerValueType_Definition> m_containerValueTypes;
        chaiscript::small_vector<std::pair<Boxed_Value, std::string>> m_globals;
        chaiscript::small_vector<std::string> m_evals;
        chaiscript::small_vector<Type_Conversion> m_conversions;

        template<typename T, typename InItr>
        static void apply(InItr begin, const InItr end, T& t) {
            for_each(begin, end, [&t](const auto& obj) {
                try {
                    t.add(obj.first, obj.second);
                }
                catch (const chaiscript::exception::name_conflict_error&) {
                    /// \todo Should we throw an error if there's a name conflict
                    ///       while applying a module?
                }
                });
        }

        template<typename T, typename InItr>
        static void apply_postfix(InItr begin, InItr end, T& t) {
            while (begin != end) {
                t.add(*begin);
                ++begin;
            }
        }

        template<typename T, typename InItr>
        static void apply_containerValueTypes(InItr begin, InItr end, T& t) {
            while (begin != end) {
                t.add(*begin);
                ++begin;
            }
        }

        template<typename T, typename InItr>
        static void apply_globals(InItr begin, InItr end, T& t) {
            while (begin != end) {
                t.add_global_const(begin->first, begin->second);
                ++begin;
            }
        }

        template<typename T, typename InItr>
        static void apply_single(InItr begin, InItr end, T& t) {
            while (begin != end) {
                t.add(*begin);
                ++begin;
            }
        }

        template<typename T, typename InItr>
        static void apply_eval(InItr begin, InItr end, T& t) {
            while (begin != end) {
                t.eval(*begin);
                ++begin;
            }
        }
    };

    /// Convenience typedef for Module objects to be added to the ChaiScript runtime
    using ModulePtr = chaiscript::shared_ptr<Module>;

    namespace detail {
        /// A Proxy_Function implementation that is able to take
        /// a vector of Proxy_Functions and perform a dispatch on them. It is
        /// used specifically in the case of dealing with Function object variables
        class Dispatch_Function final : public dispatch::Proxy_Function_Base {
        public:
            explicit Dispatch_Function(chaiscript::small_vector<Proxy_Function> t_funcs)
                : Proxy_Function_Base(build_type_infos(t_funcs), calculate_arity(t_funcs))
                , m_funcs(std::move(t_funcs)) {
            }

            bool operator==(const dispatch::Proxy_Function_Base& rhs) const noexcept override {
                try {
                    const auto& dispatch_fun = dynamic_cast<const Dispatch_Function&>(rhs);
                    return m_funcs == dispatch_fun.m_funcs;
                }
                catch (const std::bad_cast&) {
                    return false;
                }
            }

            chaiscript::small_vector<Const_Proxy_Function> get_contained_functions() const override {
                return chaiscript::small_vector<Const_Proxy_Function>(m_funcs.begin(), m_funcs.end());
            }

            static int calculate_arity(const chaiscript::small_vector<Proxy_Function>& t_funcs) noexcept {
                if (t_funcs.empty()) {
                    return -1;
                }

                const auto arity = t_funcs.front()->get_arity();

                for (const auto& func : t_funcs) {
                    if (arity != func->get_arity()) {
                        // The arities in the list do not match, so it's unspecified
                        return -1;
                    }
                }

                return arity;
            }

            bool call_match(const Function_Params& vals, const Type_Conversions_State& t_conversions) const noexcept override {
                return std::any_of(std::begin(m_funcs), std::end(m_funcs), [&vals, &t_conversions](const Proxy_Function& f) {
                    return f->call_match(vals, t_conversions);
                    });
            }

        protected:
            Boxed_Value do_call(const Function_Params& params, const Type_Conversions_State& t_conversions) const override {
                return dispatch::dispatch(m_funcs, params, t_conversions);
            }

        private:
            chaiscript::small_vector<Proxy_Function> m_funcs;

            static chaiscript::small_vector<Type_Info> build_type_infos(const chaiscript::small_vector<Proxy_Function>& t_funcs) {
                auto begin = t_funcs.cbegin();
                const auto& end = t_funcs.cend();

                if (begin != end) {
                    chaiscript::small_vector<Type_Info> type_infos = (*begin)->get_param_types();

                    ++begin;

                    bool size_mismatch = false;

                    while (begin != end) {
                        chaiscript::small_vector<Type_Info> param_types = (*begin)->get_param_types();

                        if (param_types.size() != type_infos.size()) {
                            size_mismatch = true;
                        }

                        for (size_t i = 0; i < type_infos.size() && i < param_types.size(); ++i) {
                            if (!(type_infos[i] == param_types[i])) {
                                type_infos[i] = detail::Get_Type_Info<Boxed_Value>::get();
                            }
                        }

                        ++begin;
                    }

                    assert(!type_infos.empty() && " type_info vector size is < 0, this is only possible if something else is broken");

                    if (size_mismatch) {
                        type_infos.resize(1);
                    }

                    return type_infos;
                }

                return chaiscript::small_vector<Type_Info>();
            }
        };
    } // namespace detail

    namespace detail {
        class Stack_Holder_Impl {
        public:
            // template <class T, std::size_t BufSize = sizeof(T)*20000>
            //  using SmallVector = chaiscript::small_vector<T, short_alloc<T, BufSize>>;

            template<class T>
            using SmallVector = chaiscript::small_vector<T>;

            using Scope = utility::QuickFlatMap<std::string, Boxed_Value, str_equal>;
            using StackData = SmallVector<Scope>;
            using Stacks = SmallVector<StackData>;
            using Call_Param_List = SmallVector<Boxed_Value>;
            using Call_Params = SmallVector<Call_Param_List>;

            Stack_Holder_Impl() {
                push_stack();
                push_call_params();
            };

            void push_stack_data() {
                stacks.back().emplace_back();
            };

            void push_stack() { stacks.emplace_back(1); };

            void push_call_params() {
                call_params.emplace_back();
            };

            Stacks stacks;
            Call_Params call_params;
            int call_depth = 0;
        };

        using Stack_Holder = Stack_Holder_Impl;

        /// Main class for the dispatchkit. Handles management
        /// of the object stack, functions and registered types.
        class Dispatch_Engine {
        public:
#ifdef UseCweeThreadedMapAsGlobalsMap
            using Type_Name_Map = cweeThreadedMap<std::string, chaiscript::Type_Info>;
#else
            using Type_Name_Map = std::map<std::string, chaiscript::Type_Info, str_less>;
#endif
            using Scope = utility::QuickFlatMap<std::string, Boxed_Value, str_equal>;
            using StackData = Stack_Holder_Impl::StackData;

            class State {
            public:
                utility::QuickFlatMap<std::string, chaiscript::shared_ptr<chaiscript::small_vector<Proxy_Function>>, str_equal> m_functions;
                utility::QuickFlatMap<std::string, Proxy_Function, str_equal> m_function_objects;
                utility::QuickFlatMap<std::string, Boxed_Value, str_equal> m_boxed_functions;
                cweeThreadedMap<std::string, std::string> m_postfixes;
                cweeThreadedMap< std::string, ContainerValueType_Definition>  m_containerValueTypes;
                std::multimap<double, std::pair<std::string, std::string>> m_sortedPostfixes; // inverse length (assuems no zero-length postfixes are legal

#ifdef UseCweeThreadedMapAsGlobalsMap
                cweeThreadedMap<std::string, cweeAny> m_global_objects;
#else
                std::map<std::string, Boxed_Value, str_less> m_global_objects;
#endif

                Type_Name_Map m_types;
            };

            explicit Dispatch_Engine(chaiscript::parser::ChaiScript_Parser_Base& parser)
                : m_stack_holder()
                , earlyExitScript(make_cwee_shared<std::atomic<bool>>(false))
                , m_parser(parser) {
            }

            /// \brief casts an object while applying any Dynamic_Conversion available
            template<typename Type>
            decltype(auto) boxed_cast(const Boxed_Value& bv) const {
                Type_Conversions_State state(m_conversions, m_conversions.conversion_saves());
                return (chaiscript::boxed_cast<Type>(bv, &state));
            }

            /// Add a new conversion for upcasting to a base class
            void add(const Type_Conversion& d) { m_conversions.add_conversion(d); }

            /// Add a new named Proxy_Function to the system
#if false
            void add(const Proxy_Function& f, const std::string& name) { add_function(f, name); }
#endif
            void add(const Proxy_Function& f, const std::string& name, const std::string& descriptor = "") { add_function(f, name, descriptor); }

            /// Set the value of an object, by name. If the object
            /// is not available in the current scope it is created
            void add(Boxed_Value obj, const std::string& name) {
                auto& stack = get_stack_data();

                for (auto stack_elem = stack.rbegin(); stack_elem != stack.rend(); ++stack_elem) {
                    if (auto itr = stack_elem->find(name); itr != stack_elem->end()) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                        * itr->second = std::move(obj);
#else
                        itr->second = std::move(obj);
#endif
                        return;
                    }
                }

                add_object(name, std::move(obj));
            }

            /// Adds a named object to the current scope
            /// \warning This version does not check the validity of the name
            /// it is meant for internal use only
            Boxed_Value& add_get_nodef_object(std::string t_name, Boxed_Value obj, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();

#ifdef CHAISCRIPT_ALLOW_NAME_CONFLICTS

                if (AUTO itr = stack_elem.find(std::move(t_name)); itr != stack_elem.end()) {
                    // name already exists. 
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                    return *itr->second;
#else
                    return itr->second;
#endif

                }
                else {
                    // name does not exist yet
                    if (AUTO result = stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) }); result.second) {
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                        return *result.first->second;
#else
                        return result.first->second;
#endif                  
                    }
                    else {
                        // insert failed
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                        return *result.first->second;
#else
                        return result.first->second;
#endif     
                    }
                }

#else

                if (auto result = stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) }); result.second) {
                    return result.first->second;
                }
                else {
                    // insert failed
#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                    throw chaiscript::exception::name_conflict_error(result.first->first);
#else 
                    return stack_elem.find(std::move(t_name))->second;
#endif
                }


#endif

            }

            /// Adds a named object to the current scope
            /// \warning This version does not check the validity of the name
            /// it is meant for internal use only
            Boxed_Value& add_get_object(std::string t_name, Boxed_Value obj, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();

                if (AUTO result = stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) }); result.second) {
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                    return *result.first->second;
#else
                    return result.first->second;
#endif   
                }
                else {
                    // insert failed
#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                    throw chaiscript::exception::name_conflict_error(result.first->first);
#else 
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                    return *result.first->second;
#else
                    return result.first->second;
#endif   
#endif
                }
            }

            bool object_exists(const std::string& t_name, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();
                if (AUTO itr = stack_elem.find(t_name); itr != stack_elem.end()) {
                    // name already exists.
                    return true;
                }
                else {
                    return false;
                }
            }

            bool object_defined(const std::string& t_name, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();
                if (AUTO itr = stack_elem.find(t_name); itr != stack_elem.end()) {
                    // name already exists.
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                    return !itr->second->is_undef();
#else
                    return !itr->second.is_undef();
#endif   
                }
                else {
                    return false;
                }
            }

            /// Returns a named object to the current scope
            /// \warning This version does not check the validity of the name
            /// it is meant for internal use only
            Boxed_Value get_object(const std::string& t_name, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();

                if (AUTO itr = stack_elem.find(t_name); itr != stack_elem.end()) {
                    // name already exists.
#ifdef UseCweeThreadedMapAsQuickFlatMap  
                    return *itr->second;
#else
                    return itr->second;
#endif 
                }
                else {
                    return Boxed_Value();
                }
            }

            /// Adds a named object to the current scope
            /// \warning This version does not check the validity of the name
            /// it is meant for internal use only
            void add_nodef_object(std::string t_name, Boxed_Value obj, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();

#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                if (auto result = stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) }); !result.second) {
                    // insert failed
                    throw chaiscript::exception::name_conflict_error(result.first->first);
                }
#else
                if (const auto itr = stack_elem.find(std::move(t_name)); itr != stack_elem.end()) {
                    // name already exists. Since this is without assignment (nodef) then we can safely do nothing
                    return;
                }
                else {
                    stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) });
                }
#endif
            }

            /// Adds a named object to the current scope
            /// \warning This version does not check the validity of the name
            /// it is meant for internal use only
            void add_object(std::string t_name, Boxed_Value obj, Stack_Holder& t_holder) {
                auto& stack_elem = get_stack_data(t_holder).back();

#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                if (auto result = stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) }); !result.second) {
                    // insert failed
                    throw chaiscript::exception::name_conflict_error(result.first->first);
                }
#else
                stack_elem.insert(std::pair{ std::move(t_name), std::move(obj) });
#endif

            }

            /// Adds a named object to the current scope
            /// \warning This version does not check the validity of the name
            /// it is meant for internal use only
            void add_object(const std::string& name, Boxed_Value obj) { add_object(name, std::move(obj), get_stack_holder()); }

            /// Adds a new global shared object, between all the threads
            Boxed_Value add_global_const(const Boxed_Value& obj, const std::string& name) {
                if (!obj.is_const()) {
                    throw chaiscript::exception::global_non_const();
                }

                return set_global(obj, std::move(name));
            }

            /// Adds a new global (non-const) shared object, between all the threads
            Boxed_Value add_global_no_throw(Boxed_Value obj, std::string name) {
                return set_global(obj, std::move(name));
            }

            /// Adds a new global (non-const) shared object, between all the threads
            Boxed_Value add_global(Boxed_Value obj, std::string name) {
                return set_global(obj, std::move(name));
            }

            /// Updates an existing global shared object or adds a new global shared object if not found
            Boxed_Value set_global(Boxed_Value obj, std::string name) {
                AUTO shared_guard = GUARD();
#ifdef UseCweeThreadedMapAsGlobalsMap
                m_state.m_global_objects.insert_or_assign(name, Boxed_Value(obj));
                return m_state.m_global_objects[name]->cast();
#else
                if (obj.is_undef()) {
                    if (const auto itr = m_state.m_global_objects.find(name); itr != m_state.m_global_objects.end()) {
                        return m_state.m_global_objects[name]; // undefiend and already exists - simply return the currently valid object. 
                    }
                }
                return m_state.m_global_objects.insert_or_assign(std::move(name), std::move(obj)).first->second;
#endif
            }

            /// Adds a new scope to the stack
            void new_scope() { new_scope(*m_stack_holder); }

            /// Pops the current scope from the stack
            void pop_scope() { pop_scope(*m_stack_holder); }

            /// Adds a new scope to the stack
            static void new_scope(Stack_Holder& t_holder) {
                AUTO P = &t_holder;
                P->push_stack_data();
                P->push_call_params();
            };

            /// Pops the current scope from the stack
            static void pop_scope(Stack_Holder& t_holder) {
                AUTO P = &t_holder;
                P->call_params.pop_back();
                StackData& stack = get_stack_data(t_holder);
                assert(!stack.empty());
                stack.pop_back();
            };

            /// Pushes a new stack on to the list of stacks
            static void new_stack(Stack_Holder& t_holder) {
                AUTO P = &t_holder;
                P->push_stack(); // add a new Stack with 1 element
            };

            static void pop_stack(Stack_Holder& t_holder) {
                AUTO P = &t_holder;
                P->stacks.pop_back();
            }

            /// Searches the current stack for an object of the given name
            /// includes a special overload for the _ place holder object to
            /// ensure that it is always in scope.
            Boxed_Value get_object(std::string_view name, std::atomic_uint_fast32_t& t_loc, Stack_Holder& t_holder) const {
                enum class Loc : uint_fast32_t {
                    located = 0x80000000,
                    is_local = 0x40000000,
                    stack_mask = 0x0FFF0000,
                    loc_mask = 0x0000FFFF
                };

                uint_fast32_t loc = t_loc;

                if (loc == 0) {
                    auto& stack = get_stack_data(t_holder);

                    // Is it in the stack?
                    for (auto stack_elem = stack.rbegin(); stack_elem != stack.rend(); ++stack_elem) {
                        for (auto s = stack_elem->begin(); s != stack_elem->end(); ++s) {
                            if (s->first == name) {
                                t_loc = static_cast<uint_fast32_t>(std::distance(stack.rbegin(), stack_elem) << 16)
                                    | static_cast<uint_fast32_t>(std::distance(stack_elem->begin(), s)) | static_cast<uint_fast32_t>(Loc::located)
                                    | static_cast<uint_fast32_t>(Loc::is_local);
#ifdef UseCweeThreadedMapAsQuickFlatMap
                                return *s->second;
#else
                                return s->second;
#endif
                            }
                        }
                    }

                    t_loc = static_cast<uint_fast32_t>(Loc::located);
                }
                else if ((loc & static_cast<uint_fast32_t>(Loc::is_local)) != 0u) {
                    auto& stack = get_stack_data(t_holder);

#ifdef UseCweeThreadedMapAsQuickFlatMap
                    return *stack[stack.size() - 1 - ((loc & static_cast<uint_fast32_t>(Loc::stack_mask)) >> 16)].at_index(loc & static_cast<uint_fast32_t>(Loc::loc_mask));
#else
                    return stack[stack.size() - 1 - ((loc & static_cast<uint_fast32_t>(Loc::stack_mask)) >> 16)].at_index(
                        loc & static_cast<uint_fast32_t>(Loc::loc_mask));
#endif
                }

                // Is the value we are looking for a global or function?
                AUTO shared_guard = SHARED_GUARD();

#ifdef UseCweeThreadedMapAsGlobalsMap
                const auto itr = m_state.m_global_objects.find(std::string(name));
                if (itr != m_state.m_global_objects.end()) {
                    return itr->second->cast();
                }
#else
                const auto itr = m_state.m_global_objects.find(name);
                if (itr != m_state.m_global_objects.end()) {
                    return itr->second;
                }
#endif

                // no? is it a function object?
                auto obj = get_function_object_int(name, loc);
                if (obj.first != loc) {
                    t_loc = uint_fast32_t(obj.first);
                }

                return obj.second;
            }

            /// Registers a new named type
            void add(const Type_Info& ti, const std::string& name) {
                add_global_const(const_var(ti), name + "_type");

                AUTO shared_guard = GUARD();

                m_state.m_types.insert(std::make_pair(name, ti));
            }

            /// Returns the type info for a named type
            Type_Info get_type(std::string_view name, bool t_throw = true) const {
                AUTO shared_guard = SHARED_GUARD();
#ifdef UseCweeThreadedMapAsGlobalsMap
                auto itr = m_state.m_types.find(std::string(name));
#else
                const auto itr = m_state.m_types.find(name);
#endif
                if (itr != m_state.m_types.end()) {
                    // Type was registered by user and is explicitely known
#ifdef UseCweeThreadedMapAsGlobalsMap
                    return *itr->second;
#else
                    return itr->second;
#endif
                }
                else {
                    // type was not registered or is not a typename. Could be a function result? (i.e. foot() or gallon(), which are functions that return a different type than they seem)
                    const auto function_itr = m_state.m_functions.find(name);
                    if (function_itr != m_state.m_functions.end()) {
                        chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Proxy_Function>> functionList = function_itr->second;
                        if (functionList) {
                            for (const chaiscript::Proxy_Function& func : *functionList) {
                                if (func->get_arity() <= 0) {
                                    // we have a winner
                                    auto returnTypes = func->get_param_types();
                                    if (returnTypes.size() > 0) {
                                        return returnTypes[0];
                                    }
                                }
                            }
                        }
                    }
                }

                // it was not found
                if (t_throw) {
                    throw std::range_error("Type Not Known: " + std::string(name));
                }
                else {
                    return Type_Info();
                }
            }

            /// Returns the registered name of a known type_info object
            /// compares the "bare_type_info" for the broadest possible
            /// match
            std::string get_type_name(const Type_Info& ti) const {
                AUTO shared_guard = SHARED_GUARD();

                for (const auto& elem : m_state.m_types) {
#ifdef UseCweeThreadedMapAsGlobalsMap
                    if (elem.second->bare_equal(ti)) {
                        return elem.first;
                    }
#else
                    if (elem.second.bare_equal(ti)) {
                        return elem.first;
                    }
#endif
                }

                return ti.bare_name();
            }

            /// Return all registered types
            chaiscript::small_vector<std::pair<std::string, Type_Info>> get_types() const {
                AUTO shared_guard = SHARED_GUARD();
#ifdef UseCweeThreadedMapAsGlobalsMap
                chaiscript::small_vector<std::pair<std::string, Type_Info>> out;
                for (auto& x : m_state.m_types) {
                    out.push_back(std::pair<std::string, Type_Info>(x.first, *x.second));
                }
                return out;
#else
                return chaiscript::small_vector<std::pair<std::string, Type_Info>>(m_state.m_types.begin(), m_state.m_types.end());
#endif        
            }

            chaiscript::shared_ptr<chaiscript::small_vector<Proxy_Function>> get_method_missing_functions() const {
                uint_fast32_t method_missing_loc = m_method_missing_loc;
                auto method_missing_funs = get_function("method_missing", method_missing_loc);
                if (method_missing_funs.first != method_missing_loc) {
                    m_method_missing_loc = uint_fast32_t(method_missing_funs.first);
                }

                return std::move(method_missing_funs.second);
            }

            /// Return a function by name
            std::pair<size_t, chaiscript::shared_ptr<chaiscript::small_vector<Proxy_Function>>> get_function(std::string_view t_name, const size_t t_hint) const {
                AUTO shared_guard = SHARED_GUARD();

                const auto& funs = get_functions_int();

#ifdef UseCweeThreadedMapAsQuickFlatMap
                if (auto itr = funs.find(std::string(t_name), t_hint); itr != funs.end()) {
                    return std::make_pair(std::distance(funs.begin(), itr), *itr->second);
                }
                else {
                    return std::make_pair(size_t(0), chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>());
                }
#else
                if (const auto itr = funs.find(t_name, t_hint); itr != funs.end()) {
                    return std::make_pair(std::distance(funs.begin(), itr), itr->second);
                }
                else {
                    return std::make_pair(size_t(0), chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>());
                }
#endif
            }

            /// \returns a function object (Boxed_Value wrapper) if it exists
            /// \throws std::range_error if it does not
            Boxed_Value get_function_object(const std::string_view& t_name) const {
                AUTO shared_guard = SHARED_GUARD();

                return get_function_object_int(t_name, 0).second;
            }

            /// \returns a function object (Boxed_Value wrapper) if it exists
            /// \throws std::range_error if it does not
            /// \warn does not obtain a mutex lock. \sa get_function_object for public version
            std::pair<size_t, Boxed_Value> get_function_object_int(std::string_view t_name, const size_t t_hint) const {
                const auto& funs = get_boxed_functions_int();
#ifdef UseCweeThreadedMapAsQuickFlatMap
                if (const auto itr = funs.find(std::string(t_name), t_hint); itr != funs.end()) {
                    return std::make_pair(std::distance(funs.begin(), itr), *itr->second);
                }
                else {
                    throw std::range_error("Object not found: " + std::string(t_name));
                }
#else
                if (const auto itr = funs.find(t_name, t_hint); itr != funs.end()) {
                    return std::make_pair(std::distance(funs.begin(), itr), itr->second);
                }
                else {
                    throw std::range_error("Object not found: " + std::string(t_name));
                }
#endif
            }

            /// Return true if a function exists
            bool function_exists(std::string_view name) const {
                AUTO shared_guard = SHARED_GUARD();
#ifdef UseCweeThreadedMapAsQuickFlatMap
                return get_functions_int().count(std::string(name)) > 0;
#else
                return get_functions_int().count(name) > 0;
#endif

            }

            /// \returns All values in the local thread state in the parent scope, or if it doesn't exist,
            ///          the current scope.
            std::map<std::string, Boxed_Value> get_parent_locals() const {
                auto& stack = get_stack_data();
                if (stack.size() > 1) {
                    return std::map<std::string, Boxed_Value>(stack[1].begin(), stack[1].end());
                }
                else {
                    return std::map<std::string, Boxed_Value>(stack[0].begin(), stack[0].end());
                }
            }

            /// \returns All values in the local thread state, added through the add() function
            std::map<std::string, Boxed_Value> get_locals() const {
                auto& stack = get_stack_data();
                auto& scope = stack.front();
                return std::map<std::string, Boxed_Value>(scope.begin(), scope.end());
            }

            /// \brief Sets all of the locals for the current thread state.
            ///
            /// \param[in] t_locals The map<name, value> set of variables to replace the current state with
            ///
            /// Any existing locals are removed and the given set of variables is added
            void set_locals(const std::map<std::string, Boxed_Value>& t_locals) {
                auto& stack = get_stack_data();
                auto& scope = stack.front();
#ifdef UseCweeThreadedMapAsQuickFlatMap
                scope.srce.clear();
                for (auto& x : t_locals) {
                    scope.insert_or_assign(x.first, x.second);
                }
#else
                scope.assign(t_locals.begin(), t_locals.end());
#endif
            }

            /// \brief Finds any Boxed_Value with the matching content types and pointers, and replaces them with a blank Boxed_Value. 
            void Delete(Boxed_Value& which) {
                Stack_Holder& s = *m_stack_holder;
                AUTO P = &s;

                AUTO shared_guard = GUARD();

                if (P->stacks.size() >= 1) {
                    StackData& stack = P->stacks.back();

                    for (auto itr = stack.rbegin(); itr != stack.rend(); ++itr) {
                        for (int bv_ind = ((int)itr->size()) - 1; bv_ind >= 0; bv_ind--) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                            auto option = itr->pair_at_index(bv_ind);
                            if (Boxed_Value::type_match(which, *option.second)) {
                                if ((which.is_null() == option.second->is_null()) && (which.is_null() == false)) {
                                    if (which.get_const_ptr() == option.second->get_const_ptr()) {
                                        *option.second = Boxed_Value();
                                        itr->erase_at(bv_ind);
                                    }
                                }
                            }
#else
                            auto& option = itr->pair_at_index(bv_ind);
                            if (Boxed_Value::type_match(which, option.second)) {
                                if ((which.is_null() == option.second.is_null()) && (which.is_null() == false)) {
                                    if (which.get_const_ptr() == option.second.get_const_ptr()) {
                                        option.second = Boxed_Value();
                                        itr->erase_at(bv_ind);
                                    }
                                }
                            }
#endif
                        }

                        //for (auto& option : *itr) {
                        //    if (Boxed_Value::type_match(which, option.second)) {
                        //        if ((which.is_null() == option.second.is_null()) && (which.is_null() == false)) {
                        //            if (which.get_const_ptr() == option.second.get_const_ptr()) {
                        //                option.second = Boxed_Value();
                        //                
                        //            }
                        //        }
                        //    }
                        //}
                    }
                }

                //for (size_t bv_ind = m_state.m_global_objects.size() - 1; bv_ind >= 0; bv_ind--) {
                //    auto& option = m_state.m_global_objects.pair_at_index(bv_ind);
                //    if (Boxed_Value::type_match(which, option.second)) {
                //        if ((which.is_null() == option.second.is_null()) && (which.is_null() == false)) {
                //            if (which.get_const_ptr() == option.second.get_const_ptr()) {
                //                option.second = Boxed_Value();
                //                m_state.m_global_objects.erase_at(bv_ind);
                //            }
                //        }
                //    }
                //}

                while (true) {
                    bool breakout(true);
#ifdef UseCweeThreadedMapAsGlobalsMap
                    for (auto& option : m_state.m_global_objects) {
                        Boxed_Value& bv = option.second->cast();
                        if (Boxed_Value::type_match(which, bv)) {
                            if ((which.is_null() == bv.is_null()) && (which.is_null() == false)) {
                                if (which.get_const_ptr() == bv.get_const_ptr()) {
                                    bv = Boxed_Value();

                                    m_state.m_global_objects.erase(option.first);
                                    breakout = false;
                                    break;
                                }
                            }
                        }
                    }
#else
                    for (auto& option : m_state.m_global_objects) {
                        if (Boxed_Value::type_match(which, option.second)) {
                            if ((which.is_null() == option.second.is_null()) && (which.is_null() == false)) {
                                if (which.get_const_ptr() == option.second.get_const_ptr()) {
                                    option.second = Boxed_Value();

                                    m_state.m_global_objects.erase(option.first);
                                    breakout = false;
                                    break;
                                }
                            }
                        }
                    }
#endif
                    if (breakout) break;
                }

                auto empty = Boxed_Value();
                which.swap(empty);
            };

            void CancelCurrentScript() {
                if (earlyExitScript) earlyExitScript->store(true);
            };

            ///
            /// Get a map of all objects that can be seen from the current scope in a scripting context
            ///
            std::map<std::string, Boxed_Value> get_scripting_objects() const {
                const Stack_Holder& s = *m_stack_holder;
                AUTO P = &s;

                // We don't want the current context, but one up if it exists
                const StackData& stack = (P->stacks.size() == 1) ? (P->stacks.back()) : (P->stacks[P->stacks.size() - 2]);

                std::map<std::string, Boxed_Value> retval;

                // note: map insert doesn't overwrite existing values, which is why this works
                for (auto itr = stack.rbegin(); itr != stack.rend(); ++itr) {
                    retval.insert(itr->begin(), itr->end());
                }

                // add the global values
                AUTO shared_guard = SHARED_GUARD();
#ifdef UseCweeThreadedMapAsGlobalsMap
                for (auto& j : m_state.m_global_objects) {
                    retval[j.first] = j.second->cast();
                }
#else
                retval.insert(m_state.m_global_objects.begin(), m_state.m_global_objects.end());
#endif
                return retval;
            }

            ///
            /// Get a map of all objects that can be seen from any scope in a scripting context
            ///
            std::map<std::string, Boxed_Value> get_all_scripting_objects() const {
                const Stack_Holder& s = *m_stack_holder;
                AUTO P = &s;

                std::map<std::string, Boxed_Value> retval;
                // Add the Stacks
                int level = 0;
                for (auto& stack : P->stacks) {
                    auto stackMap = chaiscript::make_shared<std::map<std::string, Boxed_Value>>();
                    for (auto itr = stack.rbegin(); itr != stack.rend(); ++itr) {
                        stackMap->insert(itr->begin(), itr->end());
                    }
                    retval[std::to_string(level++)] = Boxed_Value(stackMap);
                }
                // Add the globals
                {
                    AUTO shared_guard = SHARED_GUARD();
                    auto stackMap = chaiscript::make_shared<std::map<std::string, Boxed_Value>>();
#ifdef UseCweeThreadedMapAsGlobalsMap
                    for (auto& j : m_state.m_global_objects) {
                        stackMap->operator[](j.first) = j.second->cast();
                    }
#else
                    stackMap->insert(m_state.m_global_objects.begin(), m_state.m_global_objects.end());
#endif                 
                    retval["Globals"] = Boxed_Value(stackMap);
                }
                return retval;
            }

            /* --> */
            decltype(auto) FunctionNames_CweeStr() const {
                AUTO shared_guard = GUARD();

                std::map<std::string, bool> objs; // list of strings

                const auto& funs = get_function_objects_int();

                for (const auto& fun : funs) {
                    objs[fun.first.c_str()] = true;
                }

                const auto& funs2 = get_functions_int();
                for (const auto& fun : funs2) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    auto* vec = fun.second->get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs.push_back(fun.first);
                    }
#else
                    auto* vec = fun.second.get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs[fun.first.c_str()] = true;
                    }
#endif
                }

                const auto& funs3 = get_boxed_functions_int();
                for (const auto& fun : funs3) {
                    objs[fun.first.c_str()] = true;
                }
                cweeList<cweeStr> reply;
                for (auto& x : objs) reply.Append(x.first.c_str());
                return reply;
            }
            decltype(auto) FunctionNames_exposed() const {
                AUTO shared_guard = GUARD();

                chaiscript::small_vector<Boxed_Value> objs; // list of strings

                const auto& funs = get_function_objects_int();

                for (const auto& fun : funs) {
                    objs.push_back(const_var(fun.first));
                }

                const auto& funs2 = get_functions_int();
                for (const auto& fun : funs2) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    auto* vec = fun.second->get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs.push_back(const_var(fun.first));
                    }
#else
                    auto* vec = fun.second.get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs.push_back(const_var(fun.first));
                    }
#endif
                }

                const auto& funs3 = get_boxed_functions_int();
                for (const auto& fun : funs3) {
                    objs.push_back(const_var(fun.first));
                }

                return objs;
            }
            decltype(auto) ObjectNames_exposed() const {
                chaiscript::small_vector<Boxed_Value> retval; // list of strings

                const Stack_Holder& s = *m_stack_holder;
                AUTO P = &s;

                // Add the globals
                {
                    AUTO shared_guard = GUARD();
                    for (const auto& x : m_state.m_global_objects) {
                        retval.push_back(const_var(x.first));
                    }
                }
                return retval;
            }
            /* <-- */





            ///
            /// Get a map of all global objects
            ///
            std::map<std::string, Boxed_Value> get_global_objects() const {
                const Stack_Holder& s = *m_stack_holder;

                std::map<std::string, Boxed_Value> retval;
                // Add the globals
                {
                    AUTO shared_guard = SHARED_GUARD();
#ifdef UseCweeThreadedMapAsGlobalsMap
                    for (auto& j : m_state.m_global_objects) {
                        retval[j.first] = j.second->cast();
                    }
#else
                    retval.insert(m_state.m_global_objects.begin(), m_state.m_global_objects.end());
#endif 

                }
                return retval;
            }

            ///
            /// Get a map of all functions that can be seen from a scripting context
            ///
            std::map<std::string, Boxed_Value> get_function_objects() const {
                AUTO shared_guard = SHARED_GUARD();

                const auto& funs = get_function_objects_int();

                std::map<std::string, Boxed_Value> objs;

                for (const auto& fun : funs) {
                    objs[fun.first] = const_var(fun.second);
                }

                const auto& funs2 = get_functions_int();
                for (const auto& fun : funs2) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    auto* vec = fun.second->get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs[fun.first] = const_var(func);
                    }
#else
                    auto* vec = fun.second.get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs[fun.first] = const_var(func);
                    }
#endif         
                }

                const auto& funs3 = get_boxed_functions_int();
                for (const auto& fun : funs3) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    objs[fun.first] = *fun.second;
#else
                    objs[fun.first] = fun.second;
#endif
                }

                return objs;
            }

            /// Get a vector of all registered functions
            chaiscript::small_vector<std::pair<std::string, Boxed_Value>> get_all_function_objects() const {
                AUTO shared_guard = SHARED_GUARD();

                chaiscript::small_vector<std::pair<std::string, Boxed_Value>> objs;

                const auto& funs = get_function_objects_int();

                for (const auto& fun : funs) {
                    objs.push_back(std::pair<std::string, Boxed_Value>(fun.first, const_var(fun.second)));
                }

                const auto& funs2 = get_functions_int();
                for (const auto& fun : funs2) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    auto* vec = fun.second->get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs.push_back(std::pair<std::string, Boxed_Value>(fun.first, const_var(func)));
                    }
#else
                    auto* vec = fun.second.get();
                    if (vec && vec->size() > 0) {
                        auto& func = vec->operator[](0);
                        objs.push_back(std::pair<std::string, Boxed_Value>(fun.first, const_var(func)));
                    }
#endif
                }

                const auto& funs3 = get_boxed_functions_int();
                for (const auto& fun : funs3) {
                    objs.push_back(std::pair<std::string, Boxed_Value>(fun.first, fun.second));
                }

                return objs;
            }

            /// Get a shared map of postfixes for numbers           
            const auto& get_sortedpostfixes() const {
                AUTO shared_guard = SHARED_GUARD();
                return m_state.m_sortedPostfixes;
            };
            const cweeThreadedMap<std::string, std::string>& get_postfixes() const {
                AUTO shared_guard = SHARED_GUARD();
                return m_state.m_postfixes;
            };
            const auto& get_containerValueTypes() const {
                AUTO shared_guard = SHARED_GUARD();
                return m_state.m_containerValueTypes;
            };
            void add(const Postfix_Definition& a) {
                if (a.postFix_m.size() > 0) {
                    AUTO shared_guard = SHARED_GUARD();
                    m_state.m_postfixes.Emplace(a.postFix_m, a.functionName_m);
                    m_state.m_sortedPostfixes.insert(std::pair<double,std::pair<std::string, std::string>>(1.0 / ((double)a.postFix_m.size()), std::pair<std::string, std::string>(a.postFix_m, a.functionName_m)));
                }
            };
            void add(const ContainerValueType_Definition& a) {
                //if (!a.containerType_m.is_undef() && !a.value_type_m.is_undef()) {
                    AUTO shared_guard = SHARED_GUARD();
                    m_state.m_containerValueTypes.Emplace(std::string(a.containerType_m.name()), a);
                //}
            };
#if 1
            /// Get a function with names similar to the requested one, with the given parameters
            std::pair<std::string, chaiscript::Proxy_Function>  get_function_with_similar_name(std::string name_f, chaiscript::small_vector<chaiscript::Boxed_Value> parameters, const chaiscript::Type_Conversions_State& t_conversions) const {
                std::pair<std::string, chaiscript::Proxy_Function> out = std::pair<std::string, chaiscript::Proxy_Function>("", nullptr);
                std::vector<cweeStr> option_names;
                std::vector<cweeStr> all_option_names;
                std::unordered_map< std::string, chaiscript::Proxy_Function > perfect_options;
                std::unordered_map< std::string, chaiscript::Proxy_Function > all_options;

                if (name_f.length() > 0) {  
                    AUTO shared_guard = SHARED_GUARD();
                    const auto& functions = get_functions_int();
                    bool match = true; int i;
                    for (const auto& function : functions) {
                        for (const auto& internal_func : *function.second) {
                            if (all_options.find(function.first) == all_options.end()) {
                                all_options[function.first] = internal_func;
                                all_option_names.push_back(function.first.c_str());
                            }

                            const auto& paramTypes = internal_func->get_param_types();
                            if ((paramTypes.size() - 1) == parameters.size()) {
                                match = true;
                                for (i = 0; i < parameters.size(); i++) {
                                    try {
                                        match = match && internal_func->compare_type_to_param(paramTypes[i + 1], parameters[i], t_conversions);
                                    }
                                    catch (...) {
                                        match = false;
                                    }
                                    if (!match) break;
                                }
                                if (match) {
                                    // option!
                                    if (perfect_options.find(function.first) == perfect_options.end()) {
                                        perfect_options[function.first] = internal_func;
                                        option_names.push_back(function.first.c_str());
                                    }
                                    break; // one option per name
                                }
                            }
                        }
                    }
                
                    cweeStr matched = cweeStr(name_f.c_str()).BestMatch(option_names);
                    int distance = matched.Levenshtein(name_f.c_str());
                    if (distance <= 2 || ((((double)distance) / ((double)name_f.length())) < 0.2) ) { // less than error of 2 corrections or 20% = perfect match OK
                        out.first = matched.c_str();
                        out.second = perfect_options[out.first];
                    }
                    else {
                        cweeStr matched = cweeStr(name_f.c_str()).BestMatch(all_option_names);
                        int distance = matched.Levenshtein(name_f.c_str());
                        if (distance <= 2 || ((((double)distance) / ((double)name_f.length())) < 0.2)) { // less than error of 2 corrections or 20% = generic match OK
                            out.first = matched.c_str();
                            out.second = perfect_options[out.first];
                        }
                        else {
                            // do nothing
                        }
                    }                
                }
                return out;
            };

#endif

            /// Get a vector of all registered functions
            chaiscript::small_vector<std::pair<std::string, Proxy_Function>> get_functions() const {
                AUTO shared_guard = SHARED_GUARD();

                chaiscript::small_vector<std::pair<std::string, Proxy_Function>> rets;

                const auto& functions = get_function_objects_int();
                for (const auto& function : functions) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    rets.push_back(std::pair<std::string, Proxy_Function>(function.first, *function.second));
#else
                    rets.push_back(function);
#endif          
                }

                const auto& funs2 = get_functions_int();
                for (const auto& fun : funs2) {

#ifdef UseCweeThreadedMapAsQuickFlatMap
                    auto* vec = fun.second->get();
                    for (auto func : *vec) {
                        rets.emplace_back(fun.first, func);
                    }
#else
                    auto* vec = fun.second.get();
                    for (auto func : *vec) {
                        rets.emplace_back(fun.first, func);
                    }
#endif

                }

                const auto& funs3 = get_boxed_functions_int();
                for (const auto& fun : funs3) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    if (fun.second->is_type(chaiscript::user_type<Proxy_Function>())) {
                        Proxy_Function* func = boxed_cast<Proxy_Function*>(*fun.second);
                        if (func) {
                            rets.emplace_back(fun.first, *func);
                        }
                    }
#else
                    if (fun.second.is_type(chaiscript::user_type<Proxy_Function>())) {
                        Proxy_Function* func = boxed_cast<Proxy_Function*>(fun.second);
                        if (func) {
                            rets.emplace_back(fun.first, *func);
                        }
                    }
#endif
                }

                return rets;
            }

            /// Get the name associated to the function
            std::string get_function_name(const dispatch::Proxy_Function_Base* fun) const {
                AUTO shared_guard = SHARED_GUARD();
                const decltype(State::m_functions)& functions = get_functions_int();
                for (const auto& function : functions) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    for (const auto& internal_func : **function.second) {
                        if (internal_func.get() == fun) {
                            return function.first;
                        }
                    }
#else
                    for (const auto& internal_func : *function.second) {
                        if (internal_func.get() == fun) {
                            return function.first;
                        }
                    }
#endif
                }
                return std::string("");
            }

            const Type_Conversions& conversions() const noexcept { return m_conversions; }

            static bool is_attribute_call(const chaiscript::small_vector<Proxy_Function>& t_funs,
                const Function_Params& t_params,
                bool t_has_params,
                const Type_Conversions_State& t_conversions) noexcept {
                if (!t_has_params || t_params.empty()) {
                    return false;
                }

                return std::any_of(std::begin(t_funs), std::end(t_funs), [&](const auto& fun) {
                    return fun->is_attribute_function() && fun->compare_first_type(t_params[0], t_conversions);
                    });
            }

#ifdef CHAISCRIPT_MSVC
            // MSVC is unable to recognize that "rethrow_exception" causes the function to return
            // so we must disable it here.
#pragma warning(push)
#pragma warning(disable : 4715)
#endif
            Boxed_Value call_member(const std::string& t_name,
                std::atomic_uint_fast32_t& t_loc,
                const Function_Params& params,
                bool t_has_params,
                const Type_Conversions_State& t_conversions) {
                uint_fast32_t loc = t_loc;
                const auto funs = get_function(t_name, loc);
                if (funs.first != loc) {
                    t_loc = uint_fast32_t(funs.first);
                }

                const auto do_attribute_call = [this](int l_num_params,
                    Function_Params l_params,
                    const chaiscript::small_vector<Proxy_Function>& l_funs,
                    const Type_Conversions_State& l_conversions) -> Boxed_Value {
                        Function_Params attr_params(l_params.begin(), l_params.begin() + l_num_params);
                        Boxed_Value bv = dispatch::dispatch(l_funs, attr_params, l_conversions);
                        if (l_num_params < int(l_params.size()) || bv.get_type_info().bare_equal(user_type<dispatch::Proxy_Function_Base>())) {
                            struct This_Foist {
                                This_Foist(Dispatch_Engine& e, const Boxed_Value& t_bv)
                                    : m_e(e) {
                                    m_e.get().new_scope();
                                    m_e.get().add_object("__this", t_bv);
                                }

                                ~This_Foist() { m_e.get().pop_scope(); }

                                std::reference_wrapper<Dispatch_Engine> m_e;
                            };

                            This_Foist fi(*this, l_params.front());

                            try {
                                auto func = boxed_cast<const dispatch::Proxy_Function_Base*>(bv);
                                try {
                                    return (*func)({ l_params.begin() + l_num_params, l_params.end() }, l_conversions);
                                }
                                catch (const chaiscript::exception::bad_boxed_cast&) {
                                }
                                catch (const chaiscript::exception::arity_error&) {
                                }
                                catch (const chaiscript::exception::guard_error&) {
                                }
                                throw chaiscript::exception::dispatch_error({ l_params.begin() + l_num_params, l_params.end() },
                                    chaiscript::small_vector<Const_Proxy_Function>{boxed_cast<Const_Proxy_Function>(bv)});
                            }
                            catch (const chaiscript::exception::bad_boxed_cast&) {
                                // unable to convert bv into a Proxy_Function_Base
                                throw chaiscript::exception::dispatch_error({ l_params.begin() + l_num_params, l_params.end() },
                                    chaiscript::small_vector<Const_Proxy_Function>(l_funs.begin(), l_funs.end()));
                            }
                        }
                        else {
                            return bv;
                        }
                };

                if (is_attribute_call(*funs.second, params, t_has_params, t_conversions)) {
                    return do_attribute_call(1, params, *funs.second, t_conversions);
                }
                else {
                    std::exception_ptr except;

                    if (!funs.second->empty()) {
                        try {
                            return dispatch::dispatch(*funs.second, params, t_conversions);
                        }
                        catch (chaiscript::exception::dispatch_error&) {
                            except = std::current_exception();
                        }
                    }

                    // If we get here we know that either there was no method with that name,
                    // or there was no matching method

                    const auto functions = [&]() -> chaiscript::small_vector<Proxy_Function> {
                        chaiscript::small_vector<Proxy_Function> fs;

                        const auto method_missing_funs = get_method_missing_functions();

                        for (const auto& f : *method_missing_funs) {
                            if (f->compare_first_type(params[0], t_conversions)) {
                                fs.push_back(f);
                            }
                        }

                        return fs;
                    }();

                    const bool is_no_param = [&]() -> bool {
                        for (const auto& f : functions) {
                            if (f->get_arity() != 2) {
                                return false;
                            }
                        }
                        return true;
                    }();

                    if (!functions.empty()) {
                        try {
                            if (is_no_param) {
                                auto tmp_params = params.to_vector();
                                tmp_params.insert(tmp_params.begin() + 1, var(t_name));
                                return do_attribute_call(2, Function_Params(tmp_params), functions, t_conversions);
                            }
                            else {
                                std::array<Boxed_Value, 3> p{ params[0], var(t_name), var(chaiscript::small_vector<Boxed_Value>(params.begin() + 1, params.end())) };
                                return dispatch::dispatch(functions, Function_Params{ p }, t_conversions);
                            }
                        }
                        catch (const dispatch::option_explicit_set& e) {
                            throw chaiscript::exception::dispatch_error(params,
                                chaiscript::small_vector<Const_Proxy_Function>(funs.second->begin(), funs.second->end()),
                                e.what());
                        }
                    }

                    // If we get all the way down here we know there was no "method_missing"
                    // method at all.
                    if (except) {
                        std::rethrow_exception(except);
                    }
                    else {
                        throw chaiscript::exception::dispatch_error(params,
                            chaiscript::small_vector<Const_Proxy_Function>(funs.second->begin(), funs.second->end()));
                    }
                }
            }
#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif

            Boxed_Value call_function(std::string_view t_name,
                std::atomic_uint_fast32_t& t_loc,
                const Function_Params& params,
                const Type_Conversions_State& t_conversions) const {
                uint_fast32_t loc = t_loc;
                const auto [func_loc, func] = get_function(t_name, loc);
                if (func_loc != loc) {
                    t_loc = uint_fast32_t(func_loc);
                }

                return dispatch::dispatch(*func, params, t_conversions);
            }

            /// Dump object info to stdout
            void dump_object(const Boxed_Value& o) const { std::cout << (o.is_const() ? "const " : "") << type_name(o) << '\n'; }

            /// Dump type info to stdout
            void dump_type(const Type_Info& type) const { std::cout << (type.is_const() ? "const " : "") << get_type_name(type); }

            /// Dump function to stdout
            void dump_function(const std::pair<const std::string, Proxy_Function>& f) const {
                const auto params = f.second->get_param_types();

                dump_type(params.front());
                std::cout << " " << f.first << "(";

                for (auto itr = params.begin() + 1; itr != params.end();) {
                    dump_type(*itr);
                    ++itr;

                    if (itr != params.end()) {
                        std::cout << ", ";
                    }
                }

                std::cout << ") \n";
            }

            /// Returns true if a call can be made that consists of the first parameter
            /// (the function) with the remaining parameters as its arguments.
            Boxed_Value call_exists(const Function_Params& params) const {
                if (params.empty()) {
                    throw chaiscript::exception::arity_error(static_cast<int>(params.size()), 1);
                }

                const auto& f = this->boxed_cast<Const_Proxy_Function>(params[0]);
                const Type_Conversions_State convs(m_conversions, m_conversions.conversion_saves());

                return const_var(f->call_match(Function_Params(params.begin() + 1, params.end()), convs));
            }

            /// Dump all system info to stdout
            void dump_system() const {
                std::cout << "Registered Types: \n";
                for (const auto& [type_name, type] : get_types()) {
                    std::cout << type_name << ": " << type.bare_name() << '\n';
                }

                std::cout << '\n';

                std::cout << "Functions: \n";
                for (const auto& func : get_functions()) {
                    dump_function(func);
                }
                std::cout << '\n';
            }

            /// return true if the Boxed_Value matches the registered type by name
            bool is_type(const Boxed_Value& r, std::string_view user_typename) const noexcept {
                try {
                    if (get_type(user_typename).bare_equal(r.get_type_info())) {
                        return true;
                    }
                }
                catch (const std::range_error&) {
                }

                try {
                    const dispatch::Dynamic_Object& d = boxed_cast<const dispatch::Dynamic_Object&>(r);
                    return d.get_type_name() == user_typename;
                }
                catch (const std::bad_cast&) {
                }

                try {
                    const Type_Conversions_State convs(m_conversions, m_conversions.conversion_saves());
                    if (convs->converts_polymorphic(get_type(user_typename) /*to*/, r.get_type_info() /*from*/)) {
                        return true;
                    }
                }
                catch (...) {
                }

                return false;
            };

            Boxed_Value polymorphic_cast(const Boxed_Value& bv, std::string_view user_typename) const {
                if (is_type(bv, user_typename)) {
                    const Type_Conversions_State convs(m_conversions, m_conversions.conversion_saves());
                    return convs->boxed_type_conversion_polymorphic(get_type(user_typename) /*to*/, m_conversions.conversion_saves() /* conversion saves */, bv /* from */);
                }
                throw std::bad_cast::__construct_from_string_literal(cweeStr::printf("Could not cast from '%s' to '%s'.", bv.get_type_info().name(), user_typename.data()));
            };

            std::string type_name(const Boxed_Value& obj) const { return get_type_name(obj.get_type_info()); }

            State get_state() const {
                AUTO shared_guard = SHARED_GUARD();

                return m_state;
            }

            void set_state(const State& t_state) {
                AUTO shared_guard = GUARD();

                m_state = t_state;
            }

            static void save_function_params(Stack_Holder& t_s, chaiscript::small_vector<Boxed_Value>&& t_params) {
                AUTO P = &t_s;
                for (auto&& param : t_params) {
                    AUTO alpha = P->call_params.back();
                    AUTO a = P->call_params.back();
                    AUTO a0 = a.begin();
                    alpha.insert(
                        a0,
                        std::move(param)
                    );
                }
            }

            static void save_function_params(Stack_Holder& t_s, const Function_Params& t_params) {
                AUTO P = &t_s;

                AUTO a = t_params.begin();
                AUTO b = t_params.end();
                AUTO alpha = &P->call_params;

                AUTO a1 = alpha->back();
                AUTO a2 = alpha->back();

                a1.insert(
                    a2.begin(),
                    a,
                    b
                );
            }

            void save_function_params(chaiscript::small_vector<Boxed_Value>&& t_params) { save_function_params(*m_stack_holder, std::move(t_params)); }

            void save_function_params(const Function_Params& t_params) { save_function_params(*m_stack_holder, t_params); }

            void new_function_call(Stack_Holder& t_s, Type_Conversions::Conversion_Saves& t_saves) {
                AUTO P = &t_s;
                if (P->call_depth == 0) {
                    m_conversions.enable_conversion_saves(t_saves, true);
                }

                P->call_depth++;

                save_function_params(m_conversions.take_saves(t_saves));
            }

            void pop_function_call(Stack_Holder& t_s, Type_Conversions::Conversion_Saves& t_saves) {
                AUTO P = &t_s;
                P->call_depth--;

                if (P->call_depth == 0) {
                    AUTO a = &P->call_params;
                    a->back().clear();
                    m_conversions.enable_conversion_saves(t_saves, false);
                }
            }

            void new_function_call() { new_function_call(*m_stack_holder, m_conversions.conversion_saves()); }

            void pop_function_call() { pop_function_call(*m_stack_holder, m_conversions.conversion_saves()); }

            Stack_Holder& get_stack_holder() noexcept { return *m_stack_holder; }

            /// Returns the current stack
            /// make const/non const versions
            const StackData& get_stack_data() const noexcept {
                return m_stack_holder->stacks.back();
            }

            static StackData& get_stack_data(Stack_Holder& t_holder) noexcept {
                AUTO P = &t_holder;
                return P->stacks.back();
            }

            StackData& get_stack_data() noexcept {
                return m_stack_holder->stacks.back();
            }

            parser::ChaiScript_Parser_Base& get_parser() noexcept { return m_parser.get(); }

            // --> RG
            chaiscript::detail::threading::recursive_mutex& get_mut() const {
                return m_recursive_threading_mutex; //  m_recursive_threading_mutex;
            };
            chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> GUARD() const {
                return chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex>(m_mutex);
            };
            chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> SHARED_GUARD() const {
                return chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex>(m_mutex);
            };
            // <-- RG

        private:
            const decltype(State::m_boxed_functions)& get_boxed_functions_int() const noexcept { return m_state.m_boxed_functions; }

            decltype(State::m_boxed_functions)& get_boxed_functions_int() noexcept { return m_state.m_boxed_functions; }

            const decltype(State::m_function_objects)& get_function_objects_int() const noexcept { return m_state.m_function_objects; }

            decltype(State::m_function_objects)& get_function_objects_int() noexcept { return m_state.m_function_objects; }

            const decltype(State::m_functions)& get_functions_int() const noexcept { return m_state.m_functions; }

            decltype(State::m_functions)& get_functions_int() noexcept { return m_state.m_functions; }

            static bool function_less_than(const Proxy_Function& lhs, const Proxy_Function& rhs) noexcept {
                auto dynamic_lhs(chaiscript::dynamic_shared_ptr_cast<const dispatch::Dynamic_Proxy_Function>(lhs));
                auto dynamic_rhs(chaiscript::dynamic_shared_ptr_cast<const dispatch::Dynamic_Proxy_Function>(rhs));

                if (dynamic_lhs && dynamic_rhs) {
                    if (dynamic_lhs->get_guard()) {
                        return dynamic_rhs->get_guard() ? false : true;
                    }
                    else {
                        return false;
                    }
                }

                if (dynamic_lhs && !dynamic_rhs) {
                    return false;
                }

                if (!dynamic_lhs && dynamic_rhs) {
                    return true;
                }

                const auto& lhsparamtypes = lhs->get_param_types();
                const auto& rhsparamtypes = rhs->get_param_types();

                const auto lhssize = lhsparamtypes.size();
                const auto rhssize = rhsparamtypes.size();

                const auto boxed_type = user_type<Boxed_Value>();
                const auto boxed_pod_type = user_type<Boxed_Number>();

                for (size_t i = 1; i < lhssize && i < rhssize; ++i) {
                    const Type_Info& lt = lhsparamtypes[i];
                    const Type_Info& rt = rhsparamtypes[i];

                    if (lt.bare_equal(rt) && lt.is_const() == rt.is_const()) {
                        continue; // The first two types are essentially the same, next iteration
                    }

                    // const is after non-const for the same type
                    if (lt.bare_equal(rt) && lt.is_const() && !rt.is_const()) {
                        return false;
                    }

                    if (lt.bare_equal(rt) && !lt.is_const()) {
                        return true;
                    }

                    // boxed_values are sorted last
                    if (lt.bare_equal(boxed_type)) {
                        return false;
                    }

                    if (rt.bare_equal(boxed_type)) {
                        return true;
                    }

                    if (lt.bare_equal(boxed_pod_type)) {
                        return false;
                    }

                    if (rt.bare_equal(boxed_pod_type)) {
                        return true;
                    }

                    // otherwise, we want to sort by typeid
                    return lt < rt;
                }

                return false;
            }

            /// Implementation detail for adding a function.
            /// \throws exception::name_conflict_error if there's a function matching the given one being added
#if false
            void add_function(const Proxy_Function& t_f, const std::string& t_name) {
                AUTO shared_guard = GUARD();

                Proxy_Function new_func = [&]() -> Proxy_Function {
                    auto& funcs = get_functions_int();
                    auto itr = funcs.find(t_name);

                    if (itr != funcs.end()) {
                        auto vec = *itr->second;
                        for (const auto& func : vec) {
                            if ((*t_f) == *(func)) {
#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                                throw chaiscript::exception::name_conflict_error(t_name);
#else
                                return t_f;
#endif
                            }
                        }

                        vec.reserve(vec.size() + 1); // tightly control vec growth
                        vec.push_back(t_f);
                        std::stable_sort(vec.begin(), vec.end(), &function_less_than);
                        itr->second = chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>(vec);
                        return chaiscript::make_shared<Dispatch_Function>(std::move(vec));
                    }
                    else if (t_f->has_arithmetic_param()) {
                        // if the function is the only function but it also contains
                        // arithmetic operators, we must wrap it in a dispatch function
                        // to allow for automatic arithmetic type conversions
                        chaiscript::small_vector<Proxy_Function> vec;
                        vec.push_back(t_f);
                        funcs.insert(std::pair{ t_name, chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>(vec) });
                        return chaiscript::make_shared<Dispatch_Function>(std::move(vec));
                    }
                    else {
                        auto vec = chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>();
                        vec->push_back(t_f);
                        funcs.insert(std::pair{ t_name, vec });
                        return t_f;
                    }
                }();

                get_boxed_functions_int().insert_or_assign(t_name, const_var(new_func));
                get_function_objects_int().insert_or_assign(t_name, std::move(new_func));
            }
#endif
            void add_function(const Proxy_Function& t_f, const std::string& t_name, const std::string& t_descriptor = "") {
                AUTO shared_guard = GUARD();

                if (t_descriptor.size() > 0) {
                    t_f->set_description(t_descriptor);
                }

                Proxy_Function new_func = [&]() -> Proxy_Function {
                    auto& funcs = get_functions_int();
                    auto itr = funcs.find(t_name);

                    if (itr != funcs.end()) {
                        auto vec = *itr->second;
#ifdef UseCweeThreadedMapAsQuickFlatMap
                        for (const auto& func : *vec) {
                            if ((*t_f) == *(func)) {
#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                                throw chaiscript::exception::name_conflict_error(t_name);
#else
                                return t_f;
#endif
                            }
                        }

                        vec->reserve(vec->size() + 1); // tightly control vec growth
                        vec->push_back(t_f);
                        std::stable_sort(vec->begin(), vec->end(), &function_less_than);
#ifdef UseCweeThreadedMapAsQuickFlatMap
                        * itr->second = chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>(*vec);
#else
                        * itr->second = chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>(vec);
#endif

#else
                        for (const auto& func : vec) {
                            if ((*t_f) == *(func)) {
#ifndef CHAISCRIPT_ALLOW_NAME_CONFLICTS
                                throw chaiscript::exception::name_conflict_error(t_name);
#else
                                return func;
                                // return t_f;
#endif
                            }
                        }

                        vec.reserve(vec.size() + 1); // tightly control vec growth
                        vec.push_back(t_f);
                        std::stable_sort(vec.begin(), vec.end(), &function_less_than);
                        itr->second = chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>(vec);
#endif

#ifdef UseCweeThreadedMapAsQuickFlatMap
                        chaiscript::shared_ptr< Dispatch_Function> sp = chaiscript::make_shared<Dispatch_Function>(*vec);
#else
                        chaiscript::shared_ptr< Dispatch_Function> sp = chaiscript::make_shared<Dispatch_Function>(std::move(vec));
#endif
                        return sp;
                    }
                    else if (t_f->has_arithmetic_param()) {
                        // if the function is the only function but it also contains
                        // arithmetic operators, we must wrap it in a dispatch function
                        // to allow for automatic arithmetic type conversions
                        chaiscript::small_vector<Proxy_Function> vec;
                        vec.push_back(t_f);
                        funcs.insert(std::pair{ t_name, chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>(vec) });

                        chaiscript::shared_ptr< Dispatch_Function> sp = chaiscript::make_shared<Dispatch_Function>(std::move(vec));
                        sp->set_parameterNames(t_f->get_parameterNames());

                        return sp;
                    }
                    else {
                        auto vec = chaiscript::make_shared<chaiscript::small_vector<Proxy_Function>>();
                        vec->push_back(t_f);
                        funcs.insert(std::pair{ t_name, vec });
                        return t_f;
                    }
                }();
                get_boxed_functions_int().insert_or_assign(t_name, const_var(new_func));
                get_function_objects_int().insert_or_assign(t_name, std::move(new_func));
            }

            //mutable chaiscript::detail::threading::shared_mutex m_mutex;
            mutable chaiscript::detail::threading::recursive_mutex m_recursive_threading_mutex;
            mutable chaiscript::detail::threading::shared_mutex m_mutex;

            Type_Conversions m_conversions;
            chaiscript::detail::threading::Thread_Storage<Stack_Holder> m_stack_holder;
            std::reference_wrapper<parser::ChaiScript_Parser_Base> m_parser;

            mutable std::atomic_uint_fast32_t m_method_missing_loc = { 0 };

            State m_state;
        public:
            mutable cweeSharedPtr<std::atomic<bool>> earlyExitScript;
        };

        class Dispatch_State {
        public:
            explicit Dispatch_State(Dispatch_Engine& t_engine)
                : m_engine(t_engine)
                , earlyExitScript(make_cwee_shared<std::atomic<bool>>(false))
                , m_stack_holder(t_engine.get_stack_holder())
                , m_conversions(t_engine.conversions(), t_engine.conversions().conversion_saves()) {
            }
            explicit Dispatch_State(Dispatch_Engine& t_engine, cweeSharedPtr<std::atomic<bool>> scriptCancelToken)
                : m_engine(t_engine)
                , earlyExitScript(scriptCancelToken)
                , m_stack_holder(t_engine.get_stack_holder())
                , m_conversions(t_engine.conversions(), t_engine.conversions().conversion_saves()) {
            }

            Dispatch_Engine* operator->() const noexcept { return &m_engine.get(); }

            Dispatch_Engine& operator*() const noexcept { return m_engine.get(); }

            Stack_Holder& stack_holder() const noexcept { return m_stack_holder.get(); }

            const Type_Conversions_State& conversions() const noexcept { return m_conversions; }

            Type_Conversions::Conversion_Saves& conversion_saves() const noexcept { return m_conversions.saves(); }

            Boxed_Value& add_get_nodef_object(const std::string& t_name, Boxed_Value obj) const {
                return m_engine.get().add_get_nodef_object(t_name, std::move(obj), m_stack_holder.get());
            }

            bool object_exists(const std::string& t_name) const {
                return m_engine.get().object_exists(t_name, m_stack_holder.get());
            }

            bool object_defined(const std::string& t_name) const {
                return m_engine.get().object_defined(t_name, m_stack_holder.get());
            }

            Boxed_Value get_object(const std::string& t_name) const {
                return m_engine.get().get_object(t_name, m_stack_holder.get());
            }

            Boxed_Value& add_get_object(const std::string& t_name, Boxed_Value obj) const {
                return m_engine.get().add_get_object(t_name, std::move(obj), m_stack_holder.get());
            }

            void add_nodef_object(const std::string& t_name, Boxed_Value obj) const {
                m_engine.get().add_nodef_object(t_name, std::move(obj), m_stack_holder.get());
            }

            void add_object(const std::string& t_name, Boxed_Value obj) const {
                m_engine.get().add_object(t_name, std::move(obj), m_stack_holder.get());
            }

            Boxed_Value get_object(std::string_view t_name, std::atomic_uint_fast32_t& t_loc) const {
                return m_engine.get().get_object(t_name, t_loc, m_stack_holder.get());
            }
        
        public:
            cweeSharedPtr<std::atomic<bool>> earlyExitScript;

        private:
            std::reference_wrapper<Dispatch_Engine> m_engine;
            std::reference_wrapper<Stack_Holder> m_stack_holder;
            Type_Conversions_State m_conversions;
        };
    } // namespace detail
} // namespace chaiscript
