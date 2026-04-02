#pragma once

#include <map>
#include <unordered_set>
#include "functions.h"

namespace GL {
    namespace scope {
        // "" becomes "::", "::UI" becomes "::UI::", "std::string" becomes "::std::string::"
        static __forceinline GL::string make_scope_name(GL::string const& x) {
            return (GL::string::namespace_colons() + x.remove_leading_and_trailing(':') + GL::string::namespace_colons()).replace("::::", GL::string::empty_string());
        };
        template <typename iter_type> static __forceinline GL::type const& get_type_of(iter_type const& rhs) {
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                return *rhs;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                return rhs->m_casted_type;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                return rhs->m_casted_type;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                return *rhs;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                return rhs->m_casted_type;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                return rhs->m_casted_type;
            }
        };
        template <typename iter_type> static __forceinline bool can_free_cast(iter_type const& lhs, GL::type rhs) {
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                return lhs->can_free_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                return lhs->can_free_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                return lhs->can_free_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                return lhs->can_free_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                return lhs->can_free_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                return lhs->can_free_cast(rhs);
            }
        };        
        template <typename iter_type> static __forceinline bool can_cast(iter_type const& lhs, GL::type rhs) {
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                return lhs->can_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                return lhs->can_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                return lhs->can_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                return lhs->can_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                return lhs->can_cast(rhs);
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                return lhs->can_cast(rhs);
            }
        };
        template <typename iter_type> static __forceinline GL::type const& get_actual_type_of(iter_type const& rhs) {
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                return *rhs;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                return rhs->get_actual_type();
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                return rhs->get_actual_type();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                return *rhs;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                return rhs->get_actual_type();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                return rhs->get_actual_type();
            }
        };

        enum ScopeType {
            Basic = 1,
            Namespace = 2,
            Class = 4,
            Root = 8
        };

        class impl {
        public:
            class Breadcrumb;
            class BasicScope;
            class NamespaceScope;
            class ClassScope;
            class RootScope;

            // Identity of an individual scope
            class ScopeID {
                friend class Breadcrumb;
            public:
                GL::string
                    scope_name; // e.g. "Color"
                BasicScope*
                    scope;
            private:
                GL::string
                    current_namespace; // e.g. "::" or "::UI::Color::"
                int
                    scope_type; // may be a compound of multiple types, e.g. a root is also a namespace

            public:
                ScopeID(GL::string&& scope_name_p = {}, int scope_type_p = ScopeType::Basic)
                    : scope_name{ std::move(scope_name_p) }
                    , scope{ nullptr }
                    , current_namespace{ GL::string::empty_string() }
                    , scope_type{ scope_type_p }
                {}
                ScopeID(ScopeID&& rhs) noexcept
                    : scope_name{ std::move(rhs.scope_name) }
                    , scope{ std::move(rhs.scope) }
                    , scope_type{ rhs.scope_type }
                    , current_namespace{ std::move(rhs.current_namespace) }
                {};
                ScopeID(ScopeID const&) = delete;
                ScopeID& operator=(ScopeID&&) = delete;
                ScopeID& operator=(ScopeID const&) = delete;
                ~ScopeID() = default;

                bool is_namespace() const;
                bool is_class() const;
                bool is_root() const;
            };

            // Used to track and hash the current scope position. 
            class Breadcrumb {
            public:
                ScopeID
                    this_m; // will always point to the owner node's scope ID
                Breadcrumb*
                    parent_m; // may be nullptr for root nodes, otherwise will point to the parent breadcrumb node
                Breadcrumb*
                    root_m; // may point to this
                Breadcrumb*
                    namespace_m; // may point to this

            private:
                GL::ticket_dispensor<false>::ScopedTicket
                    scope_index; // unique index of this scope for check_flags

            public:
                size_t GetScopeIndex();
                GL::string const& GetCurrentNamespace() const;

                Breadcrumb(GL::string&& name = {}, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
                    : this_m(std::move(name), scope_type_p)
                    , parent_m(std::move(parent))
                    , root_m{ nullptr }
                    , namespace_m{ nullptr }
                    , scope_index()
                {
                    // ROOT
                    if (parent_m) root_m = parent_m->root_m;
                    else root_m = this;

                    // NAMESPACE
                    if (this_m.is_namespace()) namespace_m = this;
                    else if (parent_m) namespace_m = parent_m->namespace_m;
                    else namespace_m = this->root_m;

                    // current_namespace
                    if (parent_m) {
                        if (this_m.scope_name.length() > 0) {
                            this_m.current_namespace = make_scope_name(parent_m->this_m.current_namespace + this_m.scope_name);
                        }
                    }
                    else {
                        this_m.current_namespace = GL::string::namespace_colons();
                    }
                };
                Breadcrumb(Breadcrumb const&) = delete;
                Breadcrumb(Breadcrumb&&) = delete;
                Breadcrumb& operator=(Breadcrumb const&) = delete;
                Breadcrumb& operator=(Breadcrumb&&) = delete;
                ~Breadcrumb() {
                    if (this_m.is_namespace() && (scope_index._index > 0) && root_m) if (auto* root_ptr = dynamic_cast<RootScope*>(root_m->this_m.scope))
                        root_ptr->scopes[scope_index._index] = nullptr;
                }
            };

            // Thread-safe access to a cache of data. While new caches are made, old caches may be deleted safely, protected by Epoch-controlled allocators. 
            template <typename T, int numCategories = 4> class TypedCache {
            private: // CacheVersion -> CacheCategory -> Inputs -> Result
                using ResultForInputType = GL::shared_ptr<T>; // only emplaces, never deletes, so concurrent_unordered_map should be OK. 
                GL::epoch_search_tree<std::array<ResultForInputType, numCategories>, size_t>
                    _current_cache;
            public:
                TypedCache() = default;
                TypedCache(TypedCache const&) = delete;
                TypedCache(TypedCache&&) = delete;
                TypedCache& operator=(TypedCache const&) = delete;
                TypedCache& operator=(TypedCache&&) = delete;
                ~TypedCache() = default;

                void unsafe_unload() {
                    _current_cache.clear();
                };

                // Insert an item into the cache.
                template<int category> __declspec(noinline) void EmplaceCache(size_t cache_version, GL::shared_ptr<T> result) {
                    auto g{ _current_cache.ProtectCurrentEpoch() };
                    _current_cache.get_or_make(cache_version, [&]()->std::array<ResultForInputType, numCategories> {
                        std::array<ResultForInputType, numCategories> out;
                        return out;
                    })->object()->operator[](category).compare_exchange(nullptr, result.release_control_block()); // .compare_exchange(nullptr, std::move(result)); //
                    _current_cache.pop_front_if([&](size_t curr_version, std::array<ResultForInputType, numCategories>& cache) -> bool {
                        return curr_version < cache_version;
                        });
                };

                // Try to copy an item from the cache.
                template<int category> __declspec(noinline) GL::shared_ptr<T>& TryGetCache(size_t cache_version) {
                    GL::shared_ptr<T>* out{ nullptr };
                    _current_cache.do_at_end([&](size_t curr_version, std::array<ResultForInputType, numCategories>& cache) {
                        if (curr_version >= cache_version) {
                            out = &cache[category];
                        }
                        });
                    if (out) return *out;
                    else {
                        static GL::shared_ptr<T> temp{ nullptr };
                        return temp;
                    }
                };

                template<int category> __declspec(noinline) GL::shared_ptr<T>& at(size_t cache_version) {
                    while (true) {
                        if (GL::shared_ptr<T>& out = TryGetCache<category>(cache_version); out) {
                            return out;
                        }
                        else {
                            EmplaceCache<category>(cache_version, GL::make_shared<T>());
                        }
                    }
                };

            };

            class Converter;
            class Functions {
            private:
                GL::epoch_map< GL::shared_lockable<std::map<size_t, GL::Proxy_Function>>, GL::string>
                    functions;
            public:
                Functions() = default;
                Functions(Functions const&) = delete;
                Functions(Functions &&) = delete;
                Functions& operator=(Functions const&) = delete;
                Functions& operator=(Functions&&) = delete;
                ~Functions() = default;
            public:
                // insert a function into the storage
                GL::Proxy_Function const& add_function(GL::Proxy_Function&& func);
                // insert a function into the storage
                GL::Proxy_Function const& add_function(GL::string name_m, GL::Proxy_Function&& func);
                // insert a function into the storage
                GL::Proxy_Function const& add_function(GL::Proxy_Function&& func, std::remove_pointer_t<typename decltype(functions)::Iterator::value_type::second_type>::shared_locked& locked);
                // to_do should be of the form: [](GL::Proxy_Function const&)->bool{}. Return true to early-exit the for-each loop. 
                template<typename Func> GL::Proxy_Function const& for_each(Func const& to_do) const {
                    typedef decltype(GL::details::detail::function_signature(to_do)) function_header;
                    static_assert(std::is_same_v<bool, function_header::Return_Type>);
                    static_assert(std::is_same_v< GL::Proxy_Function const&, std::tuple_element_t<0, function_header::Param_Types::argType>>);
                    static_assert(function_header::Param_Types::numArgs <= 1);

                    static GL::Proxy_Function temp{ nullptr };
                    for (auto& funcs_by_name : functions) {
                        if (*funcs_by_name.second) {
                            GL::string const& name = *funcs_by_name.first;                                
                            for (auto& funcs : *funcs_by_name.second->lock_shared()) {
                                GL::Proxy_Function const& func = funcs.second;
                                if (to_do(func)) {
                                    return func;
                                }
                            }
                        }
                    }                    
                    return temp;
                };

                // to_do should be of the form: [](GL::Proxy_Function const&)->bool{}. Return true to early-exit the for-each loop. 
                template<typename Func> GL::Proxy_Function const& for_each(GL::string const& name, Func const& to_do) const {
                    typedef decltype(GL::details::detail::function_signature(to_do)) function_header;
                    static_assert(std::is_same_v<bool, function_header::Return_Type>);
                    static_assert(std::is_same_v< GL::Proxy_Function const&, std::tuple_element_t<0, function_header::Param_Types::argType>>);
                    static_assert(function_header::Param_Types::numArgs <= 1);

                    static GL::Proxy_Function temp{ nullptr };
                    if (auto* funcs_by_name = functions.try_at(name); funcs_by_name && *funcs_by_name) {
                        for (auto& funcs : *funcs_by_name->lock_shared()) {
                            GL::Proxy_Function const& func = funcs.second;
                            if (to_do(func)) {
                                return func;
                            }
                        }
                    }                    
                    return temp;
                };

                // to_do should be of the form: [](GL::Proxy_Function const&)->bool{}. Return true to early-exit the for-each loop. 
                template<typename Func> GL::Proxy_Function const& for_each_constructor(GL::string const& name, Func const& to_do) const {
                    typedef decltype(GL::details::detail::function_signature(to_do)) function_header;
                    static_assert(std::is_same_v<bool, function_header::Return_Type>);
                    static_assert(std::is_same_v< GL::Proxy_Function const&, std::tuple_element_t<0, function_header::Param_Types::argType>>);
                    static_assert(function_header::Param_Types::numArgs <= 1);
                    static GL::Proxy_Function temp{ nullptr };
                    if (auto* funcs_by_name = functions.try_at(name); funcs_by_name && *funcs_by_name) {
                        for (auto& funcs : *funcs_by_name->lock_shared()) {
                            GL::Proxy_Function const& func = funcs.second;
                            if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                                if (to_do(func)) {
                                    return func;
                                }
                            }
                        }
                    }
                    return temp;
                };

                // attempts to find a suitable function from this set that is callable with the given parameters. 
                enum search_conditions {
                    ignore_templates = 1,
                    only_templates = 2,
                    free_cast_only = 4,
                    no_polymorphism = 8,
                };
                template<typename iter> GL::Proxy_Function const& try_find_callable(GL::string const& name, iter const& from_iter, iter const& from_end, int search_mode = 0) const {
                    return for_each(name, [&](GL::Proxy_Function const& f)->bool {
                        if ((search_mode & only_templates) > 0) {
                            if ((f->m_signature.state_m & GL::function_signature::Template) == 0)
                                return false;
                        }
                        else if ((search_mode & ignore_templates) > 0) {
                            if ((f->m_signature.state_m & GL::function_signature::Template) > 0)
                                return false;
                        }

                        if ((search_mode & free_cast_only) > 0)
                            return f->m_signature.can_call_with_free_cast(from_iter, from_end);
                        else
                            return f->m_signature.can_call_with_cast(from_iter, from_end);
                        });
                };
                template<typename iter_type> GL::Proxy_Function const& try_find_callable(GL::string const& name, iter_type const& from_iter, iter_type const& end, int search_mode, Converter converters) const {
                    std::multimap<double, GL::Proxy_Function const*> options;
                    /*return*/ (void)for_each(name, [&](GL::Proxy_Function const& f)->bool {
                        iter_type iter = from_iter;
                        if ((search_mode & Functions::only_templates) > 0) {
                            if ((f->m_signature.state_m & GL::function_signature::Template) == 0)
                                return false;
                        }
                        else if ((search_mode & ignore_templates) > 0) {
                            if ((f->m_signature.state_m & GL::function_signature::Template) > 0)
                                return false;
                        }

                        if ((search_mode & free_cast_only) > 0) {
                            auto& sig = f->m_signature;
                            size_t i = 0;
                            double cost = 0;
                            for (; (iter != end) && (i < sig.argument_types_m.size()); ++i, ++iter) {
                                if (!can_free_cast(iter, sig.argument_types_m[i])) {
                                    return false;
                                }
                                else {
                                    cost -= (sig.argument_types_m[i].is_temp() == get_type_of(iter).is_temp()) ? 0.01 : 0.0;
                                    cost -= (sig.argument_types_m[i].is_const() == get_type_of(iter).is_const()) ? 0.01 : 0.0;
                                    cost -= (sig.argument_types_m[i].is_ref() == get_type_of(iter).is_ref()) ? 0.01 : 0.0;
                                }
                            }
                            for (; i < sig.argument_defaults_m.size(); ++i) {
                                if (sig.argument_defaults_m[i].m_casted_type.is_void()) {
                                    return false;
                                }
                            }
                            if (iter == end) {
                                options.insert({ cost, &f });
                                // return true;
                            }
                            return false;
                        }
                        else {
                            auto& sig = f->m_signature;

                            size_t i = 0;
                            double cost = 0;
                            for (; (iter != end) && (i < sig.argument_types_m.size()); ++i, ++iter) {
                                if (i == 0) {
                                    if (can_free_cast(iter, sig.argument_types_m[i])) {
                                        cost -= 0.48;
                                    }
                                }
                                if (!can_cast(iter, sig.argument_types_m[i])) {
                                    if (auto f = converters.try_get_converter(get_type_of(iter), sig.argument_types_m[i], 0, true); f) {
                                        cost += f->m_signature.numConversions;
                                    }
                                    else {
                                        return false;
                                    }
                                }
                                // should this option be preferred over others?
                                cost -= (sig.argument_types_m[i].is_temp() == get_type_of(iter).is_temp()) ? 0.01 : 0.0;
                                cost -= (sig.argument_types_m[i].is_const() == get_type_of(iter).is_const()) ? 0.01 : 0.0;
                                cost -= (sig.argument_types_m[i].is_ref() == get_type_of(iter).is_ref()) ? 0.01 : 0.0;
                            }
                            for (; i < sig.argument_defaults_m.size(); ++i) {
                                if (sig.argument_defaults_m[i].m_casted_type.is_void()) {
                                    return false;
                                }
                            }
                            if (iter == end) {
                                options.insert({ cost, &f });
                                // return cost <= 0.0; // stops looking if true
                            }
                            return false;
                        }
                    });
                    if (options.size() > 0) {
                        return *options.begin()->second;
                    }
                    else {
                        static GL::Proxy_Function temp{ nullptr };
                        return temp;
                    }
                };

                template<typename iter> GL::Proxy_Function const& try_find_callable(GL::type const& return_type, iter const& from_iter, iter const& from_end, int search_mode = 0) const {
                    return for_each([&](GL::Proxy_Function const& f)->bool {
                        if (!f->m_signature.returns_m.can_free_cast(return_type, (search_mode & no_polymorphism) == 0)) {
                            return false;
                        }

                        if ((search_mode & only_templates) > 0) {
                            if ((f->m_signature.state_m & GL::function_signature::Template) == 0)
                                return false;
                        }
                        else if ((search_mode & ignore_templates) > 0) {
                            if ((f->m_signature.state_m & GL::function_signature::Template) > 0)
                                return false;
                        }

                        if ((search_mode & free_cast_only) > 0)
                            return f->m_signature.can_call_with_free_cast(from_iter, from_end);
                        else
                            return f->m_signature.can_call_with_cast(from_iter, from_end);
                        });
                };

            public:
                class UniformCostSearchNodeBestPath {
                public:
                    UniformCostSearchNodeBestPath() = default;
                    UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath* previous, GL::type const& nextNodePath)
                        : previousBestPath(previous)
                        , thisNodePath(nextNodePath)
                    {};
                    UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath const&) = default;
                    UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath&&) noexcept = default;
                    UniformCostSearchNodeBestPath& operator=(UniformCostSearchNodeBestPath const&) = default;
                    UniformCostSearchNodeBestPath& operator=(UniformCostSearchNodeBestPath&&) noexcept = default;
                    ~UniformCostSearchNodeBestPath() = default;

                    UniformCostSearchNodeBestPath* previousBestPath{ nullptr };
                    GL::type thisNodePath;
                    size_t cached_size{ 0 };

                private:
                    void get_impl(std::vector<GL::type>& out) const;

                public:
                    GL::Proxy_Function make_converter(GL::type const& from, Functions const& srce) const;
                    void get(std::vector<GL::type>& out) const;
                    size_t size() const;
                };
                class UniformCostSearchNode {
                public:
                    UniformCostSearchNode() = default;
                    UniformCostSearchNode(GL::type const& a, double b, UniformCostSearchNodeBestPath* c)
                        : thisVertexType(a)
                        , distanceFromTarget(std::move(b))
                        , bestPath(std::move(c))
                    {};
                    UniformCostSearchNode(UniformCostSearchNode&&) noexcept = default;
                    UniformCostSearchNode(UniformCostSearchNode const&) = default;
                    UniformCostSearchNode& operator=(UniformCostSearchNode&&) noexcept = default;
                    UniformCostSearchNode& operator=(UniformCostSearchNode const&) = default;
                    ~UniformCostSearchNode() = default;
                public:
                    GL::type thisVertexType;
                    double distanceFromTarget{ 0 }; // if not known, then we can simply guess. 
                    UniformCostSearchNodeBestPath* bestPath{ nullptr };

                public:
                    size_t size() const;
                    bool operator()(const UniformCostSearchNode* a, const UniformCostSearchNode* b) const;
                    bool operator()(const std::shared_ptr<UniformCostSearchNode>& a, const std::shared_ptr<UniformCostSearchNode>& b) const;
                };
            public:
                // Solves the Uniform Cost Search Algorithm to determine the shortest path for "From" to all available, castable types.
                // If a type that was desired to be casted to is not in the collection, that means no available path was found to accomplish the requested cast.
                std::unordered_map<GL::type, UniformCostSearchNode*> CreateConversionPaths(
                    GL::atomic_allocator<std::variant<UniformCostSearchNode, UniformCostSearchNodeBestPath>, 1024>& alloc,
                    GL::type const& From
                );
                std::unordered_map<GL::type, GL::Proxy_Function> CreateConversions(GL::type const& From);
            };
            class Converter {
            private:
                Functions&
                    constructors;
                TypedCache<concurrency::concurrent_unordered_map<GL::type, concurrency::concurrent_unordered_map<GL::type, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>>>, 1>&
                    converters;
                std::mutex&
                    converter_lock;
                std::atomic<long long>&
                    constructors_version;
            public:
                Converter() = delete;
                Converter(decltype(constructors) _constructors, decltype(converters) _converters, decltype(converter_lock) _converter_lock, decltype(constructors_version) _constructors_version)
                    : constructors{ _constructors }, converters{ _converters }, converter_lock{ _converter_lock }, constructors_version{ _constructors_version }
                {};
                Converter(Converter const&) = default;
                Converter(Converter&&) = default;
                Converter& operator=(Converter const&) = default;
                Converter& operator=(Converter&&) = default;
                ~Converter() = default;

                GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_get_converter(GL::type const& from, GL::type const& to, int depth = 0, bool in_function = false);
                bool can_convert(GL::type const& from, GL::type const& to, bool in_function = false);
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func);
                template <typename iter_type> __declspec(noinline) GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, iter_type begin, iter_type const& end) {
                    if constexpr (!std::is_same_v< iter_type, GL::any::fast_any*>) {
                        static_assert(std::is_same_v<iter_type::value_type, GL::any::fast_any>, "iterator must be for a GL::any::fast_any class");
                    }

                    if (!func) return {};

                    thread_local std::array<GL::any::fast_any, 16> raw_params;
                    thread_local std::array<GL::any::fast_any*, 16> params;
                    short pos = 0;
                    short raw_pos = 0;
                    bool did_conversions = false;
                    for (; (begin != end) && (pos < 16); ++begin, ++pos) {
                        if (!can_free_cast(begin, func->m_signature.argument_types_m[pos])) {
                            if (GL::fast_shared_ptr<GL::details::Proxy_Function_Base> conversion_func{ try_get_converter(const_cast<any::fast_any*>(&*begin)->m_casted_type, func->m_signature.argument_types_m[pos], 0, true) }; conversion_func) {
                                did_conversions = true;
                                raw_params[raw_pos] = conversion_func->operator()(const_cast<any::fast_any&>(*begin));
                                params[pos] = &raw_params[raw_pos];
                                ++raw_pos;
                            }
                            else {
                                if (can_cast(begin, func->m_signature.argument_types_m[pos])) {
                                    did_conversions = true;
                                    raw_params[raw_pos] = *begin;
                                    params[pos] = &raw_params[raw_pos];
                                    ++raw_pos;
                                }
                                else {
                                    if (get_type_of(begin).get_base_hash() == GL::type_of<var>().get_base_hash()) {
                                        if (begin->cast<GL::var&>().get_type().can_cast(func->m_signature.argument_types_m[pos])) {
                                            did_conversions = true;
                                            raw_params[raw_pos] = *begin;
                                            params[pos] = &raw_params[raw_pos];
                                            ++raw_pos;
                                            continue;
                                        }
                                    }
                                    GL::string err = GL::string("Could not cast from ") + get_type_of(begin).name() + " to " + func->m_signature.argument_types_m[pos].name();
                                    throw std::runtime_error(err.to_string());
                                }
                            }
                        }
                        else {
                            params[pos] = const_cast<any::fast_any*>(&*begin);
                        }
                    }
                    GL::any::fast_any out{ func->operator()(&params[0], pos) };
                    if (did_conversions) for (pos = 0; pos < raw_pos; ++pos) raw_params[pos] = nullptr;
                    return out;
                };
                // fast path
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, std::vector<any::fast_any>& params);
                // convenience path, requires casting to any::fast_any
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, const std::vector<any>& params);
                // convenience path, requires casting to any::fast_any
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, any& param);
                // fast path
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, any::fast_any& param);


                bool can_call_with_conversions(const GL::details::Proxy_Function_Base* func);
                template <typename iter_type> bool can_call_with_conversions(const GL::details::Proxy_Function_Base* func, iter_type _begin, iter_type const& end) {
                    if (!func) return false;

                    short pos = 0;
                    for (iter_type begin = _begin; (begin != end) && (pos < 16); ++begin, ++pos) {
                        if (!can_free_cast(begin, func->m_signature.argument_types_m[pos])) {
                            if (GL::fast_shared_ptr<GL::details::Proxy_Function_Base> conversion_func{ try_get_converter(get_type_of(begin), func->m_signature.argument_types_m[pos], 0, true) }; conversion_func) {

                            }
                            else {
                                return false;
                            }
                        }
                    }
                    return true;
                };
                // fast path
                bool can_call_with_conversions(const GL::details::Proxy_Function_Base* func, std::vector<any::fast_any>& params);
                // convenience path, requires casting to any::fast_any
                bool can_call_with_conversions(const GL::details::Proxy_Function_Base* func, const std::vector<any>& params);
                // convenience path, requires casting to any::fast_any
                bool can_call_with_conversions(const GL::details::Proxy_Function_Base* func, any& param);
                // fast path
                bool can_call_with_conversions(const GL::details::Proxy_Function_Base* func, any::fast_any& param);

            };

            /// <summary>
            /// Foundational element of a scope. Should not be created on its own, and instead should be issued by a parent.
            /// </summary>
            class BasicScope {
                friend class NamespaceScope;
                friend class ClassScope;
                friend class RootScope;
                friend class Breadcrumb;
            protected:
                Breadcrumb
                    breadcrumb_m;
                GL::deferred<concurrency::concurrent_unordered_map<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>>
                    using_m; // NOTE: calling "using" should split a normal, BasicScope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
                GL::shared_lockable<std::map<GL::string, GL::any>> // concurrency::concurrent_unordered_map
                    objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 

                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) {};
                template <bool overwriteIfExists> bool EmplaceObject_Impl(GL::string const& sv, GL::any&& Obj) {
                    if constexpr (overwriteIfExists) {
                        auto locked = objects_m.lock();
                        locked->operator[](sv) = std::move(Obj);
                    }
                    else {
                        if (1) {
                            auto locked = objects_m.lock_shared();
                            if (auto f = locked->find(sv), e = locked->end(); f != e) return false;
                        }
                        if (1) {
                            auto locked = objects_m.lock();
                            locked->operator[](sv) = std::move(Obj);
                        }
                    }
                    return true;
                };
                GL::any* GetObject_Impl(GL::string const& sv);
                virtual bool AddUsing_Impl(Breadcrumb* scope);

            private:
                static auto& scope_stack() {
                    thread_local std::deque< const BasicScope* > out;
                    return out;
                };
                [[nodiscard]] static auto push_back_caller(const BasicScope* p) {
                    class handler {
                    public:
                        handler(const BasicScope* P) {
                            scope_stack().push_back(P);
                        };
                        handler(handler const&) = delete;
                        handler(handler&&) = delete;
                        handler& operator=(handler const&) = delete;
                        handler& operator=(handler&&) = delete;
                        ~handler() {
                            scope_stack().pop_back();
                        };
                    };
                    return handler(p);
                };
            public:
                static BasicScope* GetCurrentCaller();

            protected:
                BasicScope(GL::string&& name, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
                    : breadcrumb_m(std::move(name), scope_type_p, parent)
                    , using_m()
                    , objects_m()
                {
                    breadcrumb_m.this_m.scope = this;
                };

            public:
                operator bool() const;
                // Returns true if this scope is a namespace scope
                bool is_namespace() const;
                // Returns true if this scope is a class scope
                bool is_class() const;
                // Returns true if this scope is a root scope
                bool is_root() const;
                // Get the immediate parent, if one exists.
                BasicScope* GetParent() const;
                // Get the current namespace (for inserting functions, etc)
                NamespaceScope* GetNamespace() const;
                // Get the root of the entire scope tree
                RootScope* GetRoot() const;

            protected:
                enum CheckFlagState {
                    none = 0,
                    self = 1,
                    all = 2
                };
                enum SearchState {
                    SearchingParents = 1,
                    SearchingUsings = 2,
                    SearchingChildren = 4,
                    SearchUpHitNamespace = 8,
                    SkipChildren = 16,
                    SkipParent = 32
                };
                enum SearchResult {
                    Failure = 1,
                    Success = 2,
                    StaticFailure = 4
                };
                using check_cache = std::vector<short>;
                static check_cache& GetCheckMap();
                virtual Breadcrumb* FindNearestScopeWhere(
                    std::function<int(Breadcrumb*, int)> const& func,
                    Breadcrumb* SecondaryPriortyScope = nullptr,
                    int searchState = 0,
                    check_cache& check_flags = GetCheckMap(),
                    int depth = 0
                ) const;

                static Breadcrumb* FindNamespace(GL::string const& Name, Breadcrumb* start);

            public:
                /* Attempts to determine the type from a given string. This may include instancing a template-defined class if the string
                 includes the appropriate type information, such as `map<int, ::value>` or `vector<std::string>`. 
                 Will respect the use of type states, such as: `const int&`, `int&&`, `vector<std::string const>&`, etc. */
                GL::type DetermineType(GL::string from);

                /* 
                returns ["final name following final `::`", nearest_found_scope*]
                This function may initialize template classes. For example, the following call: 
                ParsePossiblyScopedName("std::vector<int>::max_length") may initialize the template class "std::vector<int>". 
                */
                std::pair<GL::string, const Breadcrumb*> ParsePossiblyScopedName(GL::string const& PossiblyScopedName) const;

            public:
                BasicScope() = delete;
                virtual ~BasicScope() {
                    //if (!is_namespace()) {
                    if (using_m) {
                        if (using_m->size() > 0) {
                            GetNamespace()->invalidate_cache();
                        }
                    }
                    if (objects_m) {
                        objects_m.lock()->clear();
                    }
                    //}
                };

                /// <summary>
                /// Get the index that is unique to this scope, which will remain unique for the life of the scope. May be re-used after the scope ends. 
                /// </summary>
                /// <returns>size_t</returns>
                size_t get_unique_index() const;

                /// <summary>
                /// Make a child scope from this scope. 
                /// Thread-safe, and allowed to make many children of this scope in parallel safely. 
                /// The created scope (and its children) are destroyed at the end of this C++ scope. 
                /// </summary>
                BasicScope make_scope() const;

                /// <summary>
                /// Instruct this scope to "use" the provided namespace when searching for objects, functions, or other scopes by name. 
                /// If this scope is a namespace, this will reset the search cache.
                /// </summary>
                /// <param name="ptr"></param>
                /// <returns></returns>
                bool add_using_here(NamespaceScope const& ptr);

                /// <summary>
                /// Insert an object into this scope only if it does not yet exist. Does not search neighbors or review the object name.
                /// </summary>
                /// <param name="sv"></param>
                /// <param name="Obj"></param>
                /// <returns>bool</returns>
                bool insert_object_here(GL::string const& sv, GL::any&& Obj);

                /// <summary>
                /// Emplace an object into this scope whether or not it exists. Does not search neighbors or review the object name.
                /// </summary>
                /// <param name="sv"></param>
                /// <param name="Obj"></param>
                /// <returns>bool</returns>
                bool emplace_object_here(GL::string const& sv, GL::any&& Obj);

                /// <summary>
                /// Try to find an object in this scope. Does not search neighbors or review the object name. Since objects cannot be removed, it safely returns a pointer. 
                /// </summary>
                /// <returns>ObjectWrapper</returns>
                GL::any* find_object_here(GL::string const& sv) const;

                // Searches for a namespace that best fits the provided information, starting from this scope's namespace or position.
                Breadcrumb* find_namespace(GL::string const& Name) const;

            private:
                Breadcrumb* FindNamespaceImpl(GL::string const& Name, Breadcrumb*& nearest_scope) const;

            public:
                // Searches for a namespace while also specifying the "closest" it was able to get to the requested namespace. Useful for debugging where the search last ended. 
                Breadcrumb* find_namespace(GL::string const& Name, Breadcrumb*& nearest_scope) const;

            public:
                // User is allowed to request a scoped object, e.g. "x" or "::x" or "::std::string::npos"
                std::pair<GL::any::fast_any, bool> try_find_object(GL::string const& PossiblyScopedName, Breadcrumb* search_from = nullptr) const;

                // User is allowed to request a scoped object, e.g. "x" or "::x" or "::std::string::npos"
                GL::any::fast_any find_object(GL::string const& PossiblyScopedName, Breadcrumb* search_from = nullptr) const;

                // attempts to find a suitable function from this set that is callable with the given parameters. 
                // searches for object-lambdas (lambda or not), non-template-functions, and template-functions, in that order of preference. 
                template<typename iter> const GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_find_callable(GL::string const& PossiblyScopedName, iter const& from_iter, iter const& from_end, int search_state = 0, Breadcrumb* search_from = nullptr) const {
                    if (search_from) {
                        // first: search this and its parents for an object that matches these conditions.
                        if (Breadcrumb* obj_search = search_from; obj_search) {
                            while (obj_search && !obj_search->this_m.is_namespace()) {
                                if (auto* o = obj_search->this_m.scope->find_object_here(PossiblyScopedName)) {
                                    if (o->can_free_cast(GL::type_of<GL::details::Proxy_Function_Base const&>())) {
                                        if ((search_state & Functions::search_conditions::free_cast_only) > 0) {
                                            if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_free_cast(from_iter, from_end)) {
                                                return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                            }
                                        }
                                        else {
                                            if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_cast(from_iter, from_end)) {
                                                return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                            }

                                            if (search_from->this_m.scope->GetRoot()->get_converters().can_call_with_conversions(&o->cast<GL::details::Proxy_Function_Base const&>(), from_iter, from_end)) {
                                                return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                            }
                                        }
                                    }
                                    if (from_iter == from_end) {
                                        // allowed to return a static object, perhaps?
                                        return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(GL::make_callable(PossiblyScopedName, [o]() -> GL::any::fast_any { return o->fast(); }, GL::function_signature::Static, {}, {}, o->m_casted_type + GL::type::Reference));
                                    }
                                }
                                obj_search = obj_search->parent_m;
                            }
                        }

                        auto total_hash = GL::util::inline_hash(search_from->this_m.scope->GetNamespace()->cache_version, search_from->this_m.scope->GetRoot()->constructors_version.load());

                        size_t search_hash = PossiblyScopedName.hash();
                        if (from_iter != from_end) {
                            for (iter _iter = from_iter; (_iter != from_end); ++_iter) {
                                GL::util::hash(search_hash, get_type_of(_iter).get_hash());
                            }
                        }

                        if (auto& cache = search_from->this_m.scope->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                            // cache exists for this moment - no new constructor or function or object has been added recently. 
                            if (auto* node = cache->try_at(search_hash); node) return node->load_fast();
                        }

                        // search for exact match?
                        if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int)-> int {
                            if (auto* o = namespacePtr->this_m.scope->find_object_here(PossiblyScopedName)) {
                                if (o->can_free_cast(GL::type_of<GL::details::Proxy_Function_Base const&>())) {
                                    if ((search_state & Functions::search_conditions::free_cast_only) > 0) {
                                        if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_free_cast(from_iter, from_end)) {
                                            return SearchResult::Success;
                                        }
                                    }
                                    else {
                                        if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_cast(from_iter, from_end)) {
                                            return SearchResult::Success;
                                        }

                                        if (search_from->this_m.scope->GetRoot()->get_converters().can_call_with_conversions(&o->cast<GL::details::Proxy_Function_Base const&>(), from_iter, from_end)) {
                                            return SearchResult::Success;
                                        }
                                    }
                                }
                                if (from_iter == from_end) {
                                    // allowed to return a static object, perhaps?
                                    return SearchResult::Success;
                                }
                            }

                            if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                            if (auto const& f = namespacePtr->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::ignore_templates | search_state); f) {
                                return SearchResult::Success;
                            }
                            else {
                                return SearchResult::Failure;
                            }
                            }, nullptr, SkipChildren)) {
                            if (auto* o = BC->this_m.scope->find_object_here(PossiblyScopedName)) {
                                if (o->can_free_cast(GL::type_of<GL::details::Proxy_Function_Base const&>())) {
                                    if ((search_state & Functions::search_conditions::free_cast_only) > 0) {
                                        if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_free_cast(from_iter, from_end)) {
                                            search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, o->cast<GL::Proxy_Function>());
                                            return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                        }
                                    }
                                    else {
                                        if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_cast(from_iter, from_end)) {
                                            search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, o->cast<GL::Proxy_Function>());
                                            return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                        }

                                        if (search_from->this_m.scope->GetRoot()->get_converters().can_call_with_conversions(&o->cast<GL::details::Proxy_Function_Base const&>(), from_iter, from_end)) {
                                            search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, o->cast<GL::Proxy_Function>());
                                            return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                        }
                                    }
                                }
                                if (from_iter == from_end) {
                                    // allowed to return a static object, perhaps?
                                    return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(GL::make_callable(PossiblyScopedName, [o]() -> GL::any::fast_any { return o->fast(); }, GL::function_signature::Static, {}, {}, o->m_casted_type + GL::type::Reference));
                                }
                            }

                            // re-do the search
                            if (auto const& f = BC->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, (iter)from_iter, from_end, Functions::search_conditions::ignore_templates | search_state); f) {
                                search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)f);
                                return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>((Proxy_Function)f);
                            }
                            else {
                                search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)nullptr);
                                return nullptr;
                            }
                        }

                        // search for match that leverages the type-cast system on normally-accessable function calls
                        if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int)-> int {
                            // skip the object search -- that was already done. 
                            if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                            if (auto const& f = namespacePtr->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::ignore_templates | search_state, this->GetRoot()->get_converters()); f) {
                                return SearchResult::Success;
                            }
                            else {
                                return SearchResult::Failure;
                            }
                            }, nullptr, SkipChildren)) {
                            // re-do the search
                            if (auto const& f = BC->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::ignore_templates | search_state, this->GetRoot()->get_converters()); f) {
                                search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)f);
                                return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>((Proxy_Function)f);
                            }
                            else {
                                search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)nullptr);
                                return nullptr;
                            }
                        }

                        // search for template match with casting support?
                        if ((search_state & Functions::search_conditions::ignore_templates) == 0) {
                            if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int)-> int {
                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                                if (auto const& f = namespacePtr->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::only_templates | search_state, this->GetRoot()->get_converters()); f) {
                                    return SearchResult::Success;
                                }
                                else {
                                    return SearchResult::Failure;
                                }
                            }, nullptr, SkipChildren)) {
                                // re-do the search
                                if (auto const& f = BC->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::only_templates | search_state, this->GetRoot()->get_converters()); f) {
                                    search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)f);
                                    return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>((Proxy_Function)f);
                                }
                                else {
                                    search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)nullptr);
                                    return nullptr;
                                }
                            }
                        }

                        // search for a match that leverages the type-cast system on the first argument to search alternative classes or namespaces
                        if (from_iter != from_end) {
                            auto* this_root = this->GetRoot();
                            GL::type this_t = get_type_of(from_iter);

                            (void)this_root->get_converters().try_get_converter(this_t, this_t + GL::type::Const + GL::type::Reference);
                            if (auto conversions = this_root->converters.TryGetCache<0>(this->GetRoot()->constructors_version.load()); conversions) {
                                if (auto f = conversions->find(this_t), e = conversions->end(); f != e) {
                                    std::multimap<int, GL::type> cast_options;
                                    for (auto& conversion_option : f->second) {
                                        if (conversion_option.first.get_base_hash() != this_t.get_base_hash()) {
                                            if (auto converter = conversion_option.second.load_fast(); converter) {
                                                cast_options.insert({ converter->m_signature.numConversions, conversion_option.first });
                                            }
                                        }
                                    }
                                    if (GL::type actual_t = get_actual_type_of(from_iter); actual_t != this_t) {
                                        cast_options.insert({ -2, actual_t });
                                    }
                                    for (auto& conversion_option : this_t.all_base_types(false)) {
                                        if (conversion_option.get_base_hash() == this_t.get_base_hash()) {
                                            cast_options.insert({ -1, conversion_option });
                                        }
                                        else {
                                            cast_options.insert({ 0, conversion_option });
                                        }                                        
                                    }

                                    for (auto& conversion_option : cast_options) {
                                        if (auto potential_class = this_root->classes.find(conversion_option.second.get_base_hash()), e2 = this_root->classes.end(); potential_class != e2) {
                                            if (Breadcrumb* BC = potential_class->second->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int)-> int {
                                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                                                if (auto const& f = namespacePtr->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::ignore_templates | search_state, this_root->get_converters()); f) {
                                                    return SearchResult::Success;
                                                }
                                                else {
                                                    return SearchResult::Failure;
                                                }
                                                }, nullptr, SkipChildren); BC) {
                                                // re-do the search
                                                if (auto const& f = BC->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, (iter)from_iter, from_end, Functions::search_conditions::ignore_templates | search_state, this_root->get_converters()); f) {
                                                    search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)f);
                                                    return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>((Proxy_Function)f);
                                                }
                                                else {
                                                    search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)nullptr);
                                                    return nullptr;
                                                }
                                            };
                                        }
                                    }
                                }
                            }
                        }

                        // failure
                        search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)nullptr);

                        return nullptr;
                    }
                    else {
                         auto [remainder, ThisBC] = this->ParsePossiblyScopedName(PossiblyScopedName);
                         if (!remainder.empty() && ThisBC) {
                             // We only have an object name -- just do the normal search from here.
                             if (auto f = ThisBC->this_m.scope->try_find_callable(remainder, from_iter, from_end, search_state, const_cast<Breadcrumb*>(ThisBC)); f) {
                                 return f;
                             }
                             else {
                                 //auto total_hash = GL::util::inline_hash(this->GetNamespace()->cache_version, this->GetRoot()->constructors_version.load());
                                 //size_t search_hash = remainder.hash();
                                 //if (from_iter != from_end) {
                                 //      for (iter _iter = from_iter; (_iter != from_end); ++_iter) {
                                 //          GL::util::hash(search_hash, get_type_of(_iter).get_hash());
                                 //      }
                                 //}                                
                                 //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                 //    // cache exists for this moment - no new constructor or function or object has been added recently. 
                                 //    if (auto* node = cache->try_at(search_hash); node) return node->load_fast();
                                 //}
                                 //else {
                                 //    this->GetNamespace()->search_cache.EmplaceCache<0>(total_hash, GL::make_shared< GL::epoch_map<GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>, size_t>>());
                                 //}

                                 if (from_iter != from_end) {
                                     auto incremented_once = from_iter;
                                     ++incremented_once;
                                     if (auto actual_t = get_actual_type_of(from_iter), casted_t = get_type_of(from_iter); casted_t != actual_t) {
                                         if (auto* BC = this->GetRoot()->try_find_class(actual_t); BC) {
                                             if (auto ff = ThisBC->this_m.scope->try_find_callable(remainder, from_iter, from_end, search_state, BC); ff) {
                                                 //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                                 //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                                 //}
                                                 return ff;
                                             }
                                             //if (auto ff = try_find_callable(remainder, incremented_once, from_end, search_state, BC); (ff && ((ff->m_signature.state_m & GL::function_signature::Static) > 0))) {
                                             //    return std::move(ff);
                                             //}
                                         }
                                     }
                                     for (auto& this_t : get_type_of(from_iter).all_base_types(false)) {
                                         if (auto* BC = this->GetRoot()->try_find_class(this_t); BC) {
                                             if (auto ff = ThisBC->this_m.scope->try_find_callable(remainder, from_iter, from_end, search_state, BC); ff) {
                                                 //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                                 //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                                 //}
                                                 return ff;
                                             }
                                             //if (auto ff = try_find_callable(remainder, incremented_once, from_end, search_state, BC); (ff && ((ff->m_signature.state_m & GL::function_signature::Static) > 0))) {
                                             //    return std::move(ff);
                                             //}
                                         }
                                     }
                                 }
                                 // is the "remainder" name an exact match for a class name? They may be trying to invoke a class... 
                                 if (auto search = this->GetRoot()->classes_by_name.find(remainder), end = this->GetRoot()->classes_by_name.end(), search2 = search; search != end) {
                                     ++search2;
                                     if ((search2 != end) && (search2->first == search->first)) {
                                         // handle the case where multiple classes share the same name
                                         if (Breadcrumb* BC = ThisBC->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int) -> int {
                                             if (!namespacePtr->this_m.is_class()) return SearchResult::Failure;
                                             if (namespacePtr->this_m.scope_name == remainder) return SearchResult::Success;
                                             return SearchResult::Failure;
                                             })) {
                                             if (auto ff = ThisBC->this_m.scope->try_find_callable(remainder, from_iter, from_end, search_state, BC); ff) {
                                                 //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                                 //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                                 //}
                                                 return ff;
                                             }
                                         };
                                     }
                                     else {
                                         // special case when there is only one class that shares this unique name.
                                         if (auto ff = ThisBC->this_m.scope->try_find_callable(remainder, from_iter, from_end, search_state, search->second); ff) {
                                             //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                             //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                             //}
                                             return ff;
                                         }
                                     }
                                 }

                                 // perhaps it needs to be template initialized?
                                 if (auto remainder_t = const_cast<BasicScope*>(this)->DetermineType(remainder); remainder_t != GL::type_of<GL::undefined>()) {
                                     if (auto* BC = this->GetRoot()->try_find_class(remainder_t); BC) {
                                         if (auto ff = BC->this_m.scope->try_find_callable(remainder, from_iter, from_end, search_state, BC); ff) {
                                             //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                             //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                             //}
                                             return ff;
                                         }
                                     }
                                 }


                                 //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                 //    cache->insert_fast(search_hash, (GL::Proxy_Function)nullptr);
                                 //}
                                 return nullptr;
                             }
                         }
                    }
                
                    return nullptr;
                };
                // attempts to find a suitable function from this set that is callable with the given parameters. 
// searches for object-lambdas (lambda or not), non-template-functions, and template-functions, in that order of preference. 
                const GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_find_callable(GL::string const& PossiblyScopedName, std::vector<GL::type> const& params = {}) const {
                    return try_find_callable(PossiblyScopedName, params.begin(), params.end());
                };

            protected:
                template<typename iter_type> GL::any::fast_any call_impl(GL::string const& PossiblyScopedName, iter_type const& from_iter, iter_type const& from_end) const {
                    auto handler = push_back_caller(this);

                    //std::vector<GL::type> types;
                    //for (iter_type iter = from_iter; iter != from_end; ++iter) {
                    //    types.push_back(get_type_of(iter));
                    //}
                    //if (auto f = try_find_callable(PossiblyScopedName, types.begin(), types.end()); f)
                    if (auto f = try_find_callable(PossiblyScopedName, from_iter, from_end); f) 
                    {
                        return this->GetRoot()->get_converters().call_with_conversions(&*f, from_iter, from_end);
                    }
                    else {
                        GL::string params;
                        for (iter_type i = from_iter; i != from_end; ++i) {
                            params = params.add_to_delim(get_type_of(i).name(), ", ");
                        }
                        GL::string err = GL::string("Could not find callable matching `") + PossiblyScopedName + "`(" + params + ") from " + this->breadcrumb_m.this_m.scope->GetNamespace()->path() + ".";
                        throw std::runtime_error(err.to_string());
                    }
                };

            public:
                // calls the requested function or finds the requested object
                GL::any::fast_any call(GL::string const& PossiblyScopedName, std::vector<GL::any::fast_any> const& params = {}) const;
                // casts to the requested c++ type.
                template <typename T> T cast(GL::any::fast_any const& got) const {
                    if (!got.can_cast(GL::type_of<T>())) {
                        if (auto converter = this->GetRoot()->get_converters().try_get_converter(got.m_casted_type, GL::type_of<std::decay_t<T>>())) {
                            return converter->operator()(const_cast<GL::any::fast_any&>(got)).cast<T>();
                        }
                    }
                    return got.cast<T>();
                };
                // casts to the requested c++ or scripting type.
                GL::any::fast_any cast(GL::any::fast_any const& from, GL::type const& to) const {
                    if (!from.can_free_cast(to)) {
                        if (auto converter = this->GetRoot()->get_converters().try_get_converter(from.m_casted_type, to)) {
                            return converter->operator()(const_cast<GL::any::fast_any&>(from));
                        }
                    }
                    return from;
                };
                // calls the requested function or finds the requested object, and casts to the requested c++ type.
                template <typename T> T call(GL::string const& PossiblyScopedName, std::vector<GL::any::fast_any> const& params = {}) const {
                    return cast<T>(call_impl(PossiblyScopedName, params.begin(), params.end()));                    
                };
                // estimates the return type of a call, if that call was to have been performed. Does not actually perform the call. In the case of failure to find anything, will return a 'GL::any' type, since that does not 100% guarrantee the function does not exist (e.g. var's, templates, etc.)
                GL::type return_type_of_potential_call(GL::string const& PossiblyScopedName, std::vector<GL::type> const& types = {}) {
                    if (auto f = try_find_callable(PossiblyScopedName, types.begin(), types.end()); f) {
                        return f->m_signature.returns_m;
                    }
                    return GL::type_of<GL::any>();
                };
            };

            /// <summary>
            /// A special type of scope that serves as the "nodes" in the script tree, hosting functions and the caches. 
            /// Should not be created on its own, and instead should be issued by a parent.
            /// </summary>
            class NamespaceScope : public BasicScope {
                friend class BasicScope;
                friend class ClassScope;
                friend class RootScope;
                friend class Breadcrumb;
            protected:
                // explicit children namespaces, with strongly-held protections to their memory.
                concurrency::concurrent_unordered_map<size_t, std::shared_ptr<NamespaceScope>>
                    children; // children cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option. 
                Functions
                    functions;

            protected:
                mutable TypedCache<
                    GL::epoch_map<Breadcrumb*, size_t
                    // concurrency::concurrent_unordered_map<size_t, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>
                    >, 1> namespace_search_cache; // while thread-safe, it does seem to singificantly decrease the performance of creating new BasicScope's, hence moving it here. 

                mutable TypedCache<
                    GL::epoch_map<GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>, size_t
                    // concurrency::concurrent_unordered_map<size_t, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>
                    >, 1> search_cache; // while thread-safe, it does seem to singificantly decrease the performance of creating new BasicScope's, hence moving it here. 

            protected:
                GL::callback<NamespaceScope> sockets_for_cache_versions; // socket(s) for others to connect to for listening to changes to THIS scope. Thread-safe. 
                GL::callback<NamespaceScope>::ScopedListener connection_for_cache_version; // socket connection for this scope to its parent, to listen to changes to THEIR scope. Not thread-safe.
                size_t cache_version; // the current cache version of this scope. Thread-safe to read. Will be updated during every call to "invalidate_cache()"

            public:
                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override;

            protected:
                virtual bool AddUsing_Impl(Breadcrumb* scope) override;

            public:
                // instancing a child namespace should only be done from an existing namespace
                NamespaceScope(GL::string&& name, int scope_type_p = ScopeType::Basic | ScopeType::Namespace, Breadcrumb* parent = nullptr)
                    : children()
                    , functions()
                    , namespace_search_cache()
                    , search_cache()
                    , sockets_for_cache_versions(&NamespaceScope::invalidate_cache)
                    , connection_for_cache_version{}
                    , cache_version{ 0 }
                    , BasicScope(std::move(name), scope_type_p, parent)
                {
                    if (this->breadcrumb_m.parent_m) {
                        if (auto* p = dynamic_cast<NamespaceScope*>(this->breadcrumb_m.parent_m->namespace_m->this_m.scope)) {
                            connection_for_cache_version = p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this);
                        }
                    }
                };
            protected:
                // unloads the connections to other namespaces before deletion, which can prevent a memory-access crash. 
                void unload() {
                    this->connection_for_cache_version = {};
                    if (using_m) {
                        for (auto& x : *using_m) x.second = {};
                    }
                    for (auto& child : this->children) child.second->unload();
                    this->children.clear();
                };

            protected:
                __declspec(noinline) virtual Breadcrumb* FindNearestScopeWhere(
                    std::function<int(Breadcrumb*, int)> const& func,
                    Breadcrumb* SecondaryPriortyScope = nullptr,
                    int searchState = 0,
                    check_cache& check_flags = GetCheckMap(),
                    int depth = 0
                ) const {
                    auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);
                    Breadcrumb* finalResult = nullptr;
                    if (depth == 0) {
                        if (auto numTickets = GetRoot()->scope_indexs.num_tickets(); check_flags.size() < numTickets) {
                            check_flags.resize(numTickets + 1);
                        }
                        for (auto& x : check_flags) x = CheckFlagState::none;
                    }

                    // Prevent Duplication
                    if ((check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all) > 0) {
                        finalResult = nullptr;
                        return finalResult;
                    }
                    if (searchState & SkipChildren) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
                    }

                    // test myself directly	
                    if ((check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::self) == 0) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::self;

                        auto res = func(&selfPtr, searchState);
                        if ((res & SearchResult::Success) > 0) {
                            finalResult = &selfPtr;
                            return finalResult;
                        }
                        else if ((res & SearchResult::StaticFailure) > 0) {
                            finalResult = nullptr;
                            return finalResult;
                        }
                    }

                    bool RequestedSkipChildren = check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all;
                    if (!(searchState & SkipChildren)) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
                    }

                    // test my personal "using" namespaces completely
                    if (using_m) {
                        if (using_m->size() > 0ull) {
                            for (auto& childNamespace : *using_m) {
                                if ((check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) > 0) { continue; }
                                if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                    return finalResult;
                                }
                            }
                        }
                    }

                    // test all of my parents directly -- hoping to quickly find "it"
                    if (!(searchState & SkipParent)) {
                        Breadcrumb* thisParent = &selfPtr;
                        while (thisParent = thisParent->parent_m) {
                            auto& flag = check_flags[thisParent->GetScopeIndex()];
                            if ((flag & CheckFlagState::self) > 0) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                                if ((res & SearchResult::Success) > 0) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if ((res & SearchResult::StaticFailure) > 0) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                                if ((res & SearchResult::Success) > 0) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if ((res & SearchResult::StaticFailure) > 0) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            // check the using statements of the parent.
                            if (thisParent->this_m.scope->using_m) {
                                if (thisParent->this_m.scope->using_m->size() > 0) {
                                    for (auto& childNamespace : *thisParent->this_m.scope->using_m) {
                                        auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                        if ((flag2 & CheckFlagState::all) > 0) continue;
                                        if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                            return finalResult;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // test the SecondaryPriortyScope, often the class of the first param provided in a function call
                    if ((depth == 0) && SecondaryPriortyScope) {
                        Breadcrumb* thisParent = SecondaryPriortyScope;
                        if (thisParent) {
                            auto& flag1 = check_flags[thisParent->GetScopeIndex()];
                            flag1 = CheckFlagState::none;

                            // test myself directly
                            if ((flag1 & CheckFlagState::self) == 0) {
                                flag1 |= CheckFlagState::self;

                                auto res = func(thisParent, searchState);
                                if ((res & SearchResult::Success) > 0) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if ((res & SearchResult::StaticFailure) > 0) {
                                    finalResult = nullptr;
                                    return finalResult;
                                }
                            }

                            // test my personal "using" namespaces completely
                            if (thisParent->this_m.scope->using_m) {
                                if (thisParent->this_m.scope->using_m->size() > 0) {
                                    for (auto& childNamespace : *thisParent->this_m.scope->using_m) {
                                        auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                                        if ((flag & CheckFlagState::all) > 0) { continue; }
                                        if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                            return finalResult;
                                        }
                                    }
                                }
                            }
                        }
                        while (thisParent = thisParent->parent_m) {
                            auto& flag = check_flags[thisParent->GetScopeIndex()];
                            if ((flag & CheckFlagState::self) > 0) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                                if ((res & SearchResult::Success) > 0) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if ((res & SearchResult::StaticFailure) > 0) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                                if ((res & SearchResult::Success) > 0) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if ((res & SearchResult::StaticFailure) > 0) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            // check the using statements of the parent.
                            if (thisParent->this_m.scope->using_m) {
                                if (thisParent->this_m.scope->using_m->size() > 0) {
                                    for (auto& childNamespace : *thisParent->this_m.scope->using_m) {
                                        auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                        if ((flag2 & CheckFlagState::all) > 0) continue;
                                        if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                            return finalResult;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Test my children themselves. 
                    if (!RequestedSkipChildren && ((searchState & SkipChildren) == 0) && (this->children.size() > 0ull)) {
                        for (auto& child : this->children) {
                            auto* child_bc = &child.second->breadcrumb_m;
                            auto& flag = check_flags[child_bc->GetScopeIndex()];

                            if ((flag & CheckFlagState::self) > 0) continue;

                            flag |= CheckFlagState::self;

                            auto res = func(child_bc, searchState | SearchingChildren | SkipChildren | SkipParent);
                            if ((res & SearchResult::Success) > 0) {
                                finalResult = child_bc;
                                return finalResult;
                            }
                            else if ((res & SearchResult::StaticFailure) > 0) {
                                flag |= CheckFlagState::all;
                            }
                        }
                    }

                    // Test my parent completely.
                    if ((searchState & SkipParent) == 0) {
                        if (selfPtr.parent_m) {
                            auto& flag = check_flags[selfPtr.parent_m->GetScopeIndex()];
                            if ((flag & CheckFlagState::all) == 0) {
                                if (selfPtr.parent_m->this_m.is_namespace()) {
                                    if (finalResult = selfPtr.parent_m->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingParents | SearchUpHitNamespace, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }
                                else {
                                    if (finalResult = selfPtr.parent_m->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingParents, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }

                            }
                        }
                    }

                    // Test my children completely. 
                    if (!RequestedSkipChildren && ((searchState & SkipChildren) == 0) && this->children.size() > 0ull) {
                        for (auto& child : this->children) {
                            auto* child_bc = &child.second->breadcrumb_m;
                            auto& flag = check_flags[child_bc->GetScopeIndex()];

                            if ((flag & CheckFlagState::all) > 0) continue;

                            if (finalResult = child_bc->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingChildren | SkipParent, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                    }

                    return finalResult;
                };

            public:
                NamespaceScope() = delete;
                virtual ~NamespaceScope() {
                    unload();
                };

                GL::string path() const;

                /// <summary>
                /// Finds or creates a new namespace scope as a child of this one, and keeps it in memory. 
                /// The created scope will survive for the life of this parent scope. 
                /// If a namespace already exists with the provided name, it will return the existing namespace without creating a new one or overwritting the existing one.
                /// </summary>
                /// <returns>NamespaceScope</returns>
                NamespaceScope& make_namespace(GL::string const& name);

                /// <summary>
                /// Finds or creates a new class scope as a child of this one, and keeps it in memory. 
                /// The created class scope will survive for the life of this parent scope. 
                /// If a namespace already exists with the provided name or type, it will return the existing namespace without creating a new one or overwritting the existing one.
                /// </summary>
                /// <returns>NamespaceScope</returns>
                ClassScope& make_class(GL::type class_type);

                /// <summary>
                /// Finds or creates a new class scope as a child of this one, and keeps it in memory. 
                /// The created class scope will survive for the life of this parent scope. 
                /// If a namespace already exists with the provided name or type, it will return the existing namespace without creating a new one or overwritting the existing one.
                /// </summary>
                /// <returns>NamespaceScope</returns>
                ClassScope& make_class(GL::string const& class_type);

                // insert a function into the storage
                void add_function(GL::Proxy_Function&& func);

                // to_do should be of the form: [](GL::Proxy_Function const&)->bool{}. Return true to early-exit the for-each loop. 
                template <typename Func> bool for_each_function(Func const& to_do) const {
                    return this->functions.for_each(to_do);
                };
            };

            /// <summary>
            /// A special type of scope that represents C++ or script-only classes, and can include the definitions for member-objects. 
            /// </summary>
            class ClassScope : public NamespaceScope {
                friend class BasicScope;
                friend class NamespaceScope;
                friend class RootScope;
                friend class Breadcrumb;
            public:
                // instancing a child namespace should only be done from an existing namespace
                ClassScope(GL::type class_type, int scope_type_p = ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, Breadcrumb* parent = nullptr)
                    : NamespaceScope(class_type.name(), scope_type_p, parent)
                    , this_type{ class_type }
                    , template_types()
                {};
                // instancing a child namespace should only be done from an existing namespace
                ClassScope(GL::string const& class_type, int scope_type_p = ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, Breadcrumb* parent = nullptr)
                    : NamespaceScope((GL::string)class_type, scope_type_p, parent)
                    , type_ownership(std::make_unique<GL::script_type>(class_type))
                    , template_types()
                {
                    const_cast<GL::type&>(this_type) = type_ownership->load();
                };
                ClassScope() = delete;
                virtual ~ClassScope() {};
                
            private:
                std::unique_ptr<GL::script_type> 
                    type_ownership;

            protected:
                concurrency::concurrent_unordered_map<GL::string, std::pair<GL::type, GL::any::fast_any>> 
                    member_objects;            
                void default_construct(GL::dynamic_object& destination);

            public:
                const GL::type this_type;
                std::vector< std::pair< GL::string, GL::type > > template_types;

                void add_member_object(GL::string const& member_name, GL::type const& member_type, GL::any::fast_any const& default_value = {});                
                void initialize_basic_member_functions();
                ClassScope& make_inherited_template_class(std::vector< std::pair<GL::string, GL::type> > const& templates);
                virtual Breadcrumb* FindNearestScopeWhere(
                    std::function<int(Breadcrumb*, int)> const& func,
                    Breadcrumb* SecondaryPriortyScope = nullptr,
                    int searchState = 0,
                    check_cache& check_flags = GetCheckMap(),
                    int depth = 0
                ) const;
            };

            /// <summary>
            /// The only scope that should be instanced on its own. 
            /// </summary>
            class RootScope : public NamespaceScope {
                friend class BasicScope;
                friend class NamespaceScope;
                friend class ClassScope;
                friend class Breadcrumb;
            protected:
                // When a scope is born it will get the smallest-possible unique index for itself. 
                // This "ticket" or unique index will be unique to the scope for its life, after which it returns the ticket to here.
                GL::ticket_dispensor<false>
                    scope_indexs;
                GL::atomic_vector<Breadcrumb*>
                    scopes; // namespaces and classes may add themselves to this list (order not guarranteed) to help with debugging or other activities. 

            public:
                Functions
                    constructors;
                TypedCache<concurrency::concurrent_unordered_map<GL::type, concurrency::concurrent_unordered_map<GL::type, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>>>, 1>
                    converters;
                std::mutex // GL::fast_shared_mutex
                    converter_lock;
                std::atomic<long long>
                    constructors_version;
                concurrency::concurrent_unordered_map<size_t, Breadcrumb*>
                    classes; // list of all unique classes, which cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option.  
                concurrency::concurrent_unordered_multimap<GL::string, Breadcrumb*>
                    classes_by_name; // list of all unique classes, which cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option.  

            public:
                RootScope()
                    : scope_indexs()
                    , scopes()
                    , constructors()
                    , converter_lock()
                    , constructors_version{0}
                    , classes()
                    , classes_by_name()
                    , NamespaceScope((GL::string)GL::string::namespace_colons(), ScopeType::Basic | ScopeType::Namespace | ScopeType::Root, nullptr)
                {};
                virtual ~RootScope() {
                    this->unload(); // must call the namespace's unload function BEFORE this destroys itself, otherwise connections are unable to resolve themselves. 
                    converters.unsafe_unload();
                };
                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override;
                /* Explicitely adds a constructor function to this script, to support the type-conversion tools. */
                void add_constructor(Breadcrumb* BC, GL::Proxy_Function&& func);
                /* returns a fast wrapper for the type-conversion tools for this script. */
                Converter get_converters();
                /* attempts to return the function to perform the conversion for `from` into `to`. Clarify if the conversion is being requested inside of a function, to allow automatic `&&`->`base` conversion support. */
                GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_get_converter(GL::type const& from, GL::type const& to, bool in_function = false);
                /* returns true if it is possible to convert `from` into `to`. Clarify if the conversion is being requested inside of a function, to allow automatic `&&`->`base` conversion support. */
                bool can_convert(GL::type const& from, GL::type const& to, bool in_function = false);
                /* returns a list of all convertable types */
                std::vector<GL::type> all_convertable_types() const;
                /* Compile the built-in types and support functions to the language, such as basic numbers, units, var's, etc. */
                void perform_builtins();
                /* force the pre-loading of all conversions for known types, to help reduce run-time stuttering. Not necessary for normal use. */
                void preload_conversions();
                /* attempts to find the scripting class for the provided type. */
                Breadcrumb* try_find_class(GL::type this_t) const;
            };

        };
        // Get the nearest calling scope for the current thread.
        impl::BasicScope* GetCurrentCaller();
        // attempts to find the scripting class for the provided type from the nearest script scope for the current thread.
        impl::ClassScope* GetClass(GL::type const& rhs);
        //// attempts to find the scripting class for the provided type from the nearest script scope for the current thread.
        //GL::any::fast_any Call(GL::string const& name, std::vector<GL::any::fast_any> const& inputs = {});
        //// attempts to find the scripting class for the provided type from the nearest script scope for the current thread.
        //template <typename T>
        //decltype(auto) Call(GL::string const& name, std::vector<GL::any::fast_any> const& inputs = {}) {
        //    if (auto* caller = GetCurrentCaller(); caller) {
        //        auto scope = caller->make_scope();
        //        return scope.call<T>(name, inputs);
        //    }
        //    else {
        //        GL::scope::impl::RootScope
        //            root;
        //        root.perform_builtins();
        //        auto scope = root.make_scope();
        //        return scope.call<T>(name, inputs);
        //    }
        //};
    }        
}