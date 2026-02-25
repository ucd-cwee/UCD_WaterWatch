#pragma once

#include <map>
#include <unordered_set>
#include "functions.h"
#include "../GpuProgramming/matrix.h"


namespace GL {
    namespace scope {
        // "" becomes "::", "::UI" becomes "::UI::", "std::string" becomes "::std::string::"
        static __forceinline GL::string make_scope_name(GL::string const& x) {
            return (GL::string::namespace_colons() + x.remove_leading_and_trailing(':') + GL::string::namespace_colons()).replace("::::", GL::string::empty_string());
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
                ScopeID(ScopeID&& rhs)
                    : scope_name{ std::move(rhs.scope_name) }
                    , scope{ std::move(rhs.scope) }
                    , scope_type{ rhs.scope_type }
                    , current_namespace{ std::move(rhs.current_namespace) }
                {};
                ScopeID(ScopeID const&) = delete;
                ScopeID& operator=(ScopeID&&) = delete;
                ScopeID& operator=(ScopeID const&) = delete;
                ~ScopeID() = default;

                bool is_namespace() const {
                    return scope_type & ScopeType::Namespace;
                };
                bool is_class() const {
                    return scope_type & ScopeType::Class;
                };
                bool is_root() const {
                    return scope_type & ScopeType::Root;
                };
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
                size_t GetScopeIndex() {
                    if (scope_index._index == 0) {
                        if (parent_m) {
                            if (auto* root_ptr = dynamic_cast<RootScope*>(root_m->this_m.scope)) {
                                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&scope_index._parent), static_cast<PVOID>(&root_ptr->scope_indexs));
                                auto new_index = root_ptr->scope_indexs.get_ticket();
                                if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&scope_index._index), new_index, 0) > 0) {
                                    root_ptr->scope_indexs.return_ticket(new_index);
                                }
                                else {
                                    if (this_m.is_namespace()) {
                                        // Since basic_scopes can be created and deleted without much notice,
                                        // we limit the caching to namespaces to help guarrantee that looping over the list
                                        // will likely be protected from the lifetime perspective.                                 
                                        root_ptr->scopes.grow_to_at_least(new_index + 1);
                                        root_ptr->scopes[new_index] = this;
                                    }
                                }
                            }
                        }
                    }
                    return scope_index._index;
                };
                GL::string const& GetCurrentNamespace() const {
                    if (this->this_m.is_namespace()) {
                        return this->this_m.current_namespace;
                    }
                    else {
                        return this->namespace_m->this_m.current_namespace;
                    }
                };

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
            template <int numCategories = 4> class Cache {
            private: // CacheVersion -> CacheCategory -> Inputs -> Result
                using ResultType = Breadcrumb*;
                using InputType = size_t;
                using ResultForInputType = concurrency::concurrent_unordered_map<InputType, ResultType>; // only emplaces, never deletes, so concurrent_unordered_map should be OK. 
                GL::atomic_map<size_t, std::array<GL::deferred<ResultForInputType>, numCategories>>
                    _current_cache; // cache uses atomic_map since it may delete items as well as append items. Needs to be sorted since we "pop" the first item frequently. 
                std::atomic<long>
                    _working{ 0 };
            public:
                Cache() = default;
                Cache(Cache const&) = delete;
                Cache(Cache&&) = delete;
                Cache& operator=(Cache const&) = delete;
                Cache& operator=(Cache&&) = delete;
                ~Cache() = default;

                void unsafe_unload() {
                    _current_cache.unsafe_unload();
                };

                // Insert an item into the cache.
                template<int category> __declspec(noinline) void EmplaceCache(size_t cache_version, size_t input_hash, Breadcrumb* result) {
                    auto g{ _current_cache.ProtectCurrentEpoch() };
                    bool success = false;
                    ++_working;
                    while (!success) {
                        if (!_current_cache.do_at_end([&](size_t curr_version, std::array<GL::deferred<ResultForInputType>, numCategories>& cache) {
                            if (curr_version >= cache_version) {
                                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&cache[category]->operator[](input_hash)), reinterpret_cast<PVOID>(result));
                                success = true;
                            }
                            })) {
                        };
                        if (!success) {
                            (void)_current_cache.get_or_make(cache_version, [&]()->std::array<GL::deferred<ResultForInputType>, numCategories> {
                                std::array<GL::deferred<ResultForInputType>, numCategories> out;
                                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&out[category]->operator[](input_hash)), reinterpret_cast<PVOID>(result));
                                return out;
                            });
                            // (void)_current_cache.operator[](cache_version); // default-initializes the item at the specified index if it does not already exist. 
                            _current_cache.pop_front_if([&](size_t curr_version, std::array<GL::deferred<ResultForInputType>, numCategories>& cache) -> bool {
                                return curr_version < cache_version;
                                });
                        }
                    }
                    --_working;
                };

                // Try to copy an item from the cache.
                template<int category> __declspec(noinline) Breadcrumb* TryGetCache(size_t cache_version, size_t input_hash) {
                    auto g{ _current_cache.ProtectCurrentEpoch() };
                    Breadcrumb* out{ nullptr };
                    _current_cache.do_at_end([&](size_t curr_version, std::array<GL::deferred<ResultForInputType>, numCategories>& cache) {
                        if (curr_version >= cache_version)
                            if (cache[category].valid())
                                if (auto f = cache[category]->find(input_hash); f != cache[category]->end())
                                    out = f->second;
                        });
                    return out;
                };

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
            public:
                GL::deferred<GL::epoch_map< GL::deferred<GL::epoch_map<GL::Proxy_Function, size_t>>, GL::string>>
                    functions;

            public:
                // insert a function into the storage
                GL::Proxy_Function const& add_function(GL::Proxy_Function const& func) {
                    return functions->operator[](func->m_signature.name_m)->insert(func->m_signature.get_hash(), (GL::Proxy_Function)func).second;
                };

                // to_do should be of the form: [](GL::Proxy_Function const&)->bool{}. Return true to early-exit the for-each loop. 
                template<typename Func> GL::Proxy_Function const& for_each(Func const& to_do) const {
                    typedef decltype(GL::details::detail::function_signature(to_do)) function_header;
                    static_assert(std::is_same_v<bool, function_header::Return_Type>);
                    static_assert(std::is_same_v< GL::Proxy_Function const&, std::tuple_element_t<0, function_header::Param_Types::argType>>);
                    static_assert(function_header::Param_Types::numArgs <= 1);

                    static GL::Proxy_Function temp{ nullptr };
                    if (functions.valid()) {
                        for (auto& funcs_by_name : *functions) {
                            if (*funcs_by_name.second) {
                                GL::string const& name = *funcs_by_name.first;
                                for (auto& funcs : funcs_by_name.second->operator*()) {
                                    GL::Proxy_Function const& func = *funcs.second;
                                    if (to_do(func)) {
                                        return func;
                                    }
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
                    if (functions.valid()) {
                        if (auto* funcs_by_name = functions->try_at(name); funcs_by_name && funcs_by_name->valid()) {
                            for (auto& funcs : funcs_by_name->operator*()) {
                                GL::Proxy_Function const& func = *funcs.second;
                                if (to_do(func)) {
                                    return func;
                                }
                            }
                        }
                    }
                    return temp;
                };

                // to_do should be of the form: [](GL::Proxy_Function const&)->bool{}. Return true to early-exit the for-each loop. 
                template<typename Func> void for_each_constructor(GL::string const& name, Func const& to_do) const {
                    typedef decltype(GL::details::detail::function_signature(to_do)) function_header;
                    static_assert(std::is_same_v<bool, function_header::Return_Type>);
                    static_assert(std::is_same_v< GL::Proxy_Function const&, std::tuple_element_t<0, function_header::Param_Types::argType>>);
                    static_assert(function_header::Param_Types::numArgs <= 1);

                    if (functions.valid()) {
                        if (auto* funcs_by_name = functions->try_at(name); funcs_by_name && funcs_by_name->valid()) {
                            for (auto& funcs : funcs_by_name->operator*()) {
                                GL::Proxy_Function const& func = *funcs.second;
                                if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                                    if (to_do(func)) {
                                        return func;
                                    }
                                }
                            }
                        }
                    }
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
                    std::map<int, GL::Proxy_Function const*> options;
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
                            if constexpr (std::is_same_v<iter_type, GL::type*> || std::is_same_v<iter_type::value_type, GL::type>) {
                                size_t i = 0;
                                for (; (iter != end) && (i < sig.argument_types_m.size()); ++i, ++iter) {
                                    if (!iter->can_free_cast(sig.argument_types_m[i])) {
                                        return false;
                                    }
                                }
                                for (; i < sig.argument_defaults_m.size(); ++i) {
                                    if (sig.argument_defaults_m[i].m_casted_type.is_void()) {
                                        return false;
                                    }
                                }
                                if (iter == end) {
                                    options[0] = &f;
                                    return true;
                                }
                                return false;
                            }
                            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*> || std::is_same_v<iter_type, GL::any*> || std::is_same_v<iter_type::value_type, GL::any::fast_any> || std::is_same_v<iter_type::value_type, GL::any>) {
                                size_t i = 0;
                                for (; (iter != end) && (i < sig.argument_types_m.size()); ++i, ++iter) {
                                    if (!iter->can_free_cast(sig.argument_types_m[i])) {
                                        return false;
                                    }
                                }
                                for (; i < sig.argument_defaults_m.size(); ++i) {
                                    if (sig.argument_defaults_m[i].m_casted_type.is_void()) {
                                        return false;
                                    }
                                }
                                if (iter == end) {
                                    options[0] = &f;
                                    return true;
                                }
                                return false;
                            }
                        }
                        else {
                            auto& sig = f->m_signature;
                            if constexpr (std::is_same_v<iter_type, GL::type*> || std::is_same_v<iter_type::value_type, GL::type>) {
                                size_t i = 0;
                                int cost = 0;
                                for (; (iter != end) && (i < sig.argument_types_m.size()); ++i, ++iter) {
                                    if (!iter->can_cast(sig.argument_types_m[i])) {
                                        if (auto f = converters.try_get_converter(*iter, sig.argument_types_m[i], 0, true); f) {
                                            cost += f->m_signature.numConversions;
                                        }
                                        else {
                                            return false;
                                        }
                                    }
                                }
                                for (; i < sig.argument_defaults_m.size(); ++i) {
                                    if (sig.argument_defaults_m[i].m_casted_type.is_void()) {
                                        return false;
                                    }
                                }
                                if (iter == end) {
                                    options[cost] = &f;
                                    return cost == 0; // stops looking if true
                                }
                                return false;
                            }
                            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*> || std::is_same_v<iter_type, GL::any*> || std::is_same_v<iter_type::value_type, GL::any::fast_any> || std::is_same_v<iter_type::value_type, GL::any>) {
                                size_t i = 0;
                                int cost = 0;
                                for (; (iter != end) && (i < sig.argument_types_m.size()); ++i, ++iter) {
                                    if (!iter->can_cast(sig.argument_types_m[i])) {
                                        if (auto f = converters.try_get_converter(iter->m_casted_type, sig.argument_types_m[i], 0, true); f) {
                                            cost += f->m_signature.numConversions;
                                        }
                                        else {
                                            return false;
                                        }
                                    }
                                }
                                for (; i < sig.argument_defaults_m.size(); ++i) {
                                    if (sig.argument_defaults_m[i].m_casted_type.is_void()) {
                                        return false;
                                    }
                                }
                                if (iter == end) {
                                    options[cost] = &f;
                                    return cost == 0; // stops looking if true
                                }
                                return false;
                            }
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

                void clear() {
                    if (functions.valid()) {
                        functions->clear();
                    }
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
                    UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath&&) = default;
                    UniformCostSearchNodeBestPath& operator=(UniformCostSearchNodeBestPath const&) = default;
                    UniformCostSearchNodeBestPath& operator=(UniformCostSearchNodeBestPath&&) = default;
                    ~UniformCostSearchNodeBestPath() = default;

                    UniformCostSearchNodeBestPath* previousBestPath{ nullptr };
                    GL::type thisNodePath;
                    size_t cached_size{ 0 };

                private:
                    void get_impl(std::vector<GL::type>& out) const {
                        if (previousBestPath) previousBestPath->get_impl(out);
                        out.push_back(thisNodePath);
                    };

                public:
                    GL::Proxy_Function make_converter(GL::type const& from, Functions const& srce) const {
                        std::vector<GL::type> path;
                        std::vector<GL::Proxy_Function const*> converters;
                        GL::type this_t = from;
                        get(path);
                        int state = std::numeric_limits<int>::max();
                        for (auto& t : path) {
                            if (this_t.can_free_cast(t)) {
                                this_t = t;
                            }
                            else if (auto& x = srce.try_find_callable(t, &this_t, &this_t + 1, free_cast_only | no_polymorphism); x) { // was originally "free_cast_only" only, and did not perform second loop
                                converters.push_back(&x);
                                this_t = t;
                                state &= x->m_signature.state_m;
                            }
                            else if (auto& x = srce.try_find_callable(t, &this_t, &this_t + 1, free_cast_only); x) {
                                converters.push_back(&x);
                                this_t = t;
                                state &= x->m_signature.state_m;
                            }

                            /*                           else if (auto& x = srce.try_find_callable("", &this_t, &this_t + 1, 0); x) {
                                                           converters.push_back(&x);
                                                           this_t = t;
                                                           state &= x->m_signature.state_m;
                                                       }*/
                            else {
                                return nullptr;
                            }
                        }

                        // no conversion function is necessary
                        if (converters.size() == 0) {
                            this_t = thisNodePath;
                            auto temp = GL::make_callable("`static_cast " + (this_t).name() + "`", [converters, this_t](GL::any::fast_any& from) -> GL::any::fast_any {
                                GL::any::fast_any out{ from };
                                out.m_casted_type = this_t;
                                return out;
                                }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Constructor | GL::function_signature::NoCost | GL::function_signature::Explicit, {}, { { "From", from } }, this_t);
                            temp->m_signature.numConversions = 0;
                            return temp;

                        }
                        // use a user-provided function exactly
                        else if (converters.size() == 1) {
                            return *converters[0];
                        }
                        // create a daisy-chain function
                        else {
                            // generic function that casts "from"-type objects to "this"-type by iteratively making the inner-type casts. 
                            // e.g. int->ldouble might take the path of int->long->float->double->ldouble, for a total of 4 casts hidden as a single cast or a single call to this Proxy_Function. 
                            auto temp = GL::make_callable("`operator " + (this_t - GL::type::Temporary).name() + "`", [converters](GL::any::fast_any& from) -> GL::any::fast_any {
                                GL::any::fast_any out;
                                int pos{ 0 };
                                for (; pos < converters.size() && pos < 1; ++pos) out = (*converters[pos])->operator()(from);
                                for (; pos < converters.size(); ++pos) out = (*converters[pos])->operator()(out);
                                return out;
                                }, state | GL::function_signature::Explicit, {}, { { "From", from } }, this_t);
                            temp->m_signature.numConversions = 0;
                            for (auto& x : converters) temp->m_signature.numConversions += (*x)->m_signature.numConversions;
                            return temp;
                        }
                    };
                    void get(std::vector<GL::type>& out) const {
                        out.clear();
                        get_impl(out);
                    };
                    size_t size() const {
                        if (cached_size == 0) {
                            if (previousBestPath) {
                                const_cast<size_t&>(cached_size) = 1 + previousBestPath->size();
                            }
                            else {
                                const_cast<size_t&>(cached_size) = 1;
                            }
                        }
                        return cached_size;
                    };
                };
                class UniformCostSearchNode {
                public:
                    UniformCostSearchNode() = default;
                    UniformCostSearchNode(GL::type const& a, double b, UniformCostSearchNodeBestPath* c)
                        : thisVertexType(a)
                        , distanceFromTarget(std::move(b))
                        , bestPath(std::move(c))
                    {};
                    UniformCostSearchNode(UniformCostSearchNode&&) = default;
                    UniformCostSearchNode(UniformCostSearchNode const&) = default;
                    UniformCostSearchNode& operator=(UniformCostSearchNode&&) = default;
                    UniformCostSearchNode& operator=(UniformCostSearchNode const&) = default;
                    ~UniformCostSearchNode() = default;
                public:
                    GL::type thisVertexType;
                    double distanceFromTarget; // if not known, then we can simply guess. 
                    UniformCostSearchNodeBestPath* bestPath{ nullptr };

                public:
                    size_t size() const {
                        if (bestPath) {
                            return bestPath->size();
                        }
                        else {
                            return 0;
                        }
                    };
                    bool operator()(const UniformCostSearchNode* a, const UniformCostSearchNode* b) const {
                        return ((a->size() + 1) > (b->size() + 1)) || (a->distanceFromTarget > b->distanceFromTarget);
                    };
                    bool operator()(const std::shared_ptr<UniformCostSearchNode>& a, const std::shared_ptr<UniformCostSearchNode>& b) const {
                        return ((a->size() + 1) > (b->size() + 1)) || (a->distanceFromTarget > b->distanceFromTarget);
                    };
                };
            public:
                // Solves the Uniform Cost Search Algorithm to determine the shortest path for "From" to all available, castable types.
                // If a type that was desired to be casted to is not in the collection, that means no available path was found to accomplish the requested cast.
                std::unordered_map<GL::type, UniformCostSearchNode*> CreateConversionPaths(
                    GL::atomic_allocator<std::variant<UniformCostSearchNode, UniformCostSearchNodeBestPath>, 1024>& alloc,
                    GL::type const& From
                ) {
                    std::unordered_set<GL::type> AllTypes;
                    std::unordered_map<GL::type, std::unordered_map<GL::type, GL::Proxy_Function const*>> AllConversions; // all built-in conversions, e.g. int->float, float->double, etc.
                    (void)this->for_each([&AllConversions, &AllTypes](GL::Proxy_Function const& func)->bool {
                        AllTypes.insert(func->m_signature.returns_m);
                        for (auto& x : func->m_signature.argument_types_m) AllTypes.insert(x);

                        if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                            if (func->m_signature.argument_types_m.size() == 1) {
                                auto& from = func->m_signature.argument_types_m[0];
                                auto& to = func->m_signature.returns_m;
                                AllConversions[from][to] = &func;
                            }
                        }
                        return false;
                        });

#if 1 // allow polymorphic/dynamic casts
                    // Also, we should add the conversions for "inherited& -> base&", "inherited -> base const&", "inherited&& -> base&&".
                    for (auto& rawType : AllTypes) {
                        auto Type = GL::type(rawType.get_base_hash()) | (rawType.is_cpp_type() ? GL::type::CppType : 0);
                        for (auto& base_type : Type.all_base_types()) {

                            if (1) {
                                auto temp_func = GL::make_callable("`dynamic_cast " + (base_type + GL::type::Reference).name() + "`", [base = base_type](GL::any::fast_any const& inherited) -> GL::any {
                                    GL::any from = inherited;
                                    from.m_casted_type = base | GL::type::Reference;
                                    return from;
                                    }, /*GL::function_signature::NoCost | */GL::function_signature::Async | GL::function_signature::Constant, {}, { { "from", Type | GL::type::Reference } }, base_type | GL::type::Reference);


                                if (auto* p = functions->operator[](temp_func->m_signature.name_m)->try_at(temp_func->m_signature.get_hash()); p == nullptr) {
                                    this->add_function(temp_func);
                                }

                                if (auto* func = functions->operator[](temp_func->m_signature.name_m)->try_at(temp_func->m_signature.get_hash()); func != nullptr) {
                                    AllConversions[func->operator->()->m_signature.argument_types_m[0]][func->operator->()->m_signature.returns_m] = func;
                                }
                            }
                            if (0) {
                                auto temp_func = GL::make_callable("`dynamic_cast " + (base_type + GL::type::Reference + GL::type::Const).name() + "`", [base = base_type](GL::any::fast_any const& inherited) -> GL::any {
                                    GL::any from = inherited;
                                    from.m_casted_type = base | GL::type::Reference | GL::type::Const;
                                    return from;
                                    }, /*GL::function_signature::NoCost | */GL::function_signature::Async | GL::function_signature::Constant, {}, { { "from", Type } }, base_type | GL::type::Reference | GL::type::Const);

                                if (auto* p = functions->operator[](temp_func->m_signature.name_m)->try_at(temp_func->m_signature.get_hash()); p == nullptr) {
                                    this->add_function(temp_func);
                                }

                                if (auto* func = functions->operator[](temp_func->m_signature.name_m)->try_at(temp_func->m_signature.get_hash()); func != nullptr) {
                                    AllConversions[func->operator->()->m_signature.argument_types_m[0]][func->operator->()->m_signature.returns_m] = func;
                                }
                            }
                            if (1) {
                                auto temp_func = GL::make_callable("`dynamic_cast " + (base_type + GL::type::Temporary).name() + "`", [base = base_type](GL::any::fast_any const& inherited) -> GL::any {
                                    GL::any from = inherited;
                                    from.m_casted_type = base | GL::type::Temporary;
                                    return from;
                                    }, /*GL::function_signature::NoCost |*/ GL::function_signature::Async | GL::function_signature::Constant, {}, { { "from", Type | GL::type::Temporary } }, base_type | GL::type::Temporary);

                                if (auto* p = functions->operator[](temp_func->m_signature.name_m)->try_at(temp_func->m_signature.get_hash()); p == nullptr) {
                                    this->add_function(temp_func);
                                }

                                if (auto* func = functions->operator[](temp_func->m_signature.name_m)->try_at(temp_func->m_signature.get_hash()); func != nullptr) {
                                    AllConversions[func->operator->()->m_signature.argument_types_m[0]][func->operator->()->m_signature.returns_m] = func;
                                }
                            }
                        }
                    }
#endif

                    // create the shortest paths from "From" to all possible vertices.
                    std::unordered_map<GL::type, UniformCostSearchNode*> vertices;
                    if (1) {
                        // create an empty vertex set
                        std::priority_queue<
                            UniformCostSearchNode*
                            , std::vector<UniformCostSearchNode*>
                            , UniformCostSearchNode
                        > vertexSet;

                        // Add the source vertex into the set
                        vertexSet.push(&std::get<UniformCostSearchNode>(*alloc.Alloc(UniformCostSearchNode{ From, 0.0, nullptr })));

                        // is the vertex set empty?
                        double conversionCost{ 0 };
                        // conversionTreeType::iterator f;
                        UniformCostSearchNode* smallestDistanceNode;
                        //conversionTreeType::value_type::second_type::const_iterator fSecondIter;
                        //conversionTreeType::value_type::second_type::const_iterator fSecondEnd;
                        //GoodLang::details::Type_Conversion_Base* func;
                        while (vertexSet.size() != 0) {
                            // extract the vertex with the smallest distance value from the set
                            smallestDistanceNode = vertexSet.top();
                            vertexSet.pop();

                            // for each neighbor of the extracted vertex... 
                            for (auto& x : AllConversions) {
                                if (smallestDistanceNode->thisVertexType.can_free_cast(x.first, false)) { // was originally "true" 
                                    for (auto fSecondIter = x.second.cbegin(), fSecondEnd = x.second.cend(); fSecondIter != fSecondEnd; ++fSecondIter) {
                                        auto& connection = *fSecondIter;
                                        if (connection.second != nullptr) {
                                            if (auto& func = *connection.second) {
                                                //conversionCost = GoodLang::details::TypeConversionBaselineCost;
                                                //if (!func->IsDaisyChained()) // do not use daisy-chained functions as candidates for new ones, since it can be harder to determine the actual conversion chain length
                                                conversionCost = ((func->m_signature.state_m & GL::function_signature::NoCost) > 0) ? 0.01 : 1.0;

                                                if (1) {
                                                    // Is the neighbor already in the vertex set? 
                                                    auto& toVertex = vertices[connection.first];

                                                    if (!toVertex) { // Instance it before we start working with it on an as-needed basis
                                                        toVertex = &std::get<UniformCostSearchNode>(*alloc.Alloc(
                                                            UniformCostSearchNode{
                                                                connection.first,
                                                                std::numeric_limits<double>::infinity(),
                                                                &std::get<UniformCostSearchNodeBestPath>(*alloc.Alloc(UniformCostSearchNodeBestPath{ nullptr, connection.first }))
                                                            }
                                                        ));
                                                    }
                                                    if ((toVertex->size() + 1) > (smallestDistanceNode->size() + 1)) {
                                                        toVertex->distanceFromTarget = (smallestDistanceNode->distanceFromTarget + conversionCost);
                                                        toVertex->bestPath = &std::get<UniformCostSearchNodeBestPath>(*alloc.Alloc(UniformCostSearchNodeBestPath{ smallestDistanceNode->bestPath, toVertex->thisVertexType }));
                                                        vertexSet.push(toVertex);
                                                    }
                                                    else if (toVertex->distanceFromTarget > (smallestDistanceNode->distanceFromTarget + conversionCost)) {
                                                        toVertex->distanceFromTarget = (smallestDistanceNode->distanceFromTarget + conversionCost);
                                                        toVertex->bestPath = &std::get<UniformCostSearchNodeBestPath>(*alloc.Alloc(UniformCostSearchNodeBestPath{ smallestDistanceNode->bestPath, toVertex->thisVertexType }));
                                                        vertexSet.push(toVertex);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    return vertices;
                };


                std::unordered_map<GL::type, GL::Proxy_Function> CreateConversions(GL::type const& From) {
                    std::unordered_map<GL::type, GL::Proxy_Function> out;

                    GL::atomic_allocator<std::variant<GL::scope::impl::Functions::UniformCostSearchNode, GL::scope::impl::Functions::UniformCostSearchNodeBestPath>, 1024>
                        temp_alloc;
                    auto converters
                        = CreateConversionPaths(temp_alloc, From);
                    GL::type t;
                    for (auto& To : converters)
                        if (To.second) {
                            if (auto p = To.second->bestPath->make_converter(From, *this)) {
                                t = p->m_signature.returns_m;
                                out[t] = std::move(p);
                            }
                        }
                    return out;
                };



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

                GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_get_converter(GL::type const& from, GL::type const& to, int depth = 0, bool in_function = false) {
                    if (depth >= 2) return {};
                    if (auto& cached = converters.TryGetCache<0>(constructors_version.load()); cached) {
                        if (auto f1 = cached->find(from), e1 = cached->end(); f1 != e1) {
                            for (int attemptN = 0; attemptN < 2; ++attemptN) {
                                if (attemptN == 1) {
                                    // we have (potentially) never made the converters for this "from" type.                                     
                                    if (depth == 0) {
                                        auto locked{ std::scoped_lock(converter_lock) };
                                        //for (auto& local_converter : constructors.CreateConversions(from))
                                        //    cached->operator[](from)[local_converter.first] = std::move(local_converter.second);
                                        //(void)cached->operator[](from);
                                    }
                                }

                                if (auto f2 = f1->second.find(to), e2 = f1->second.end(); f2 != e2) {
                                    // we have made (or tried to make) the converter for this before. 
                                    if (f2->second) {
                                        return f2->second.load_fast();
                                    }
                                    else if (in_function && (to.is_base() || to.is_const_ref())) {
                                        return try_get_converter(from, to | GL::type::Temporary, 0, true);
                                    }
                                    else {
                                        return nullptr;
                                    }
                                }
                                else {
                                    // we have not made this SPECIFIC conversion for "from" to "to", but we have called "CreateConversions" already. 
                                    // This means we should attempt to find the lowest-cost conversion using what is already available in this list, if possible.

                                    // best-case scenario, we can free-cast to the requested type. 
                                    if (from.can_free_cast(to)) {
                                        auto temp = GL::make_callable("`static_cast " + to.name() + "`", [To = to](GL::any::fast_any const& From) -> GL::any {
                                            GL::any casted = From;
                                            casted.m_casted_type = To;
                                            return casted;
                                            }, GL::function_signature::Cached | GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Explicit, {}, { { "From", from } }, to);
                                        temp->m_signature.numConversions = 0;
                                        f1->second[to] = std::move(temp);
                                        return f1->second[to].load_fast();
                                    }

                                    // next-best-case scenario, we are perfect forwarding a temp-type to a base type. 
                                    if (from.can_cast(to)) {
                                        if (from.is_temp() && to.is_base()) {
                                            auto temp = GL::make_callable("`forward_cast" + to.name() + "`", [To = to](GL::any::fast_any const& From) -> GL::any {
                                                GL::any casted = From;
                                                casted.m_casted_type = To;
                                                return casted;
                                                }, GL::function_signature::Cached | GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Explicit, {}, { { "From", from } }, to);
                                            temp->m_signature.numConversions = 0;
                                            f1->second[to] = std::move(temp);
                                            return f1->second[to].load_fast();
                                        }
                                    }

                                    // We need to make the cast from the available cast for this same type. Sort options by preference? 
                                    std::map<int, GL::Proxy_Function> options;
                                    for (auto& potential_conversion : f1->second) {
                                        if (potential_conversion.second) {
                                            if (auto func = potential_conversion.second.load(); func.get() != nullptr) {
                                                if ((func->m_signature.state_m & GL::function_signature::Cached) > 0) continue;

                                                if (func->m_signature.returns_m.can_free_cast(to, false)) {
                                                    f1->second[to] = std::move(func);
                                                    return f1->second[to].load_fast();
                                                }

                                                if (func->m_signature.returns_m.can_free_cast(to)) {
                                                    if (options.count(0) == 0) {
                                                        options[0] = std::move(func);
                                                    }
                                                    continue;
                                                    //f1->second[to] = std::move(func);
                                                    //return f1->second[to].load();
                                                }

                                                if (func->m_signature.returns_m.can_cast(to)) {
                                                    if (func->m_signature.returns_m.is_temp() && to.is_base()) {
                                                        if (options.count(1) == 0) {
                                                            options[1] = GL::make_callable("`call_and_cast " + func->m_signature.name_m + "`", [To = to, caster = func](GL::any::fast_any const& From) -> GL::any {
                                                                GL::any::fast_any _from = From;
                                                                GL::any casted = caster->operator()(&_from, &_from + 1);
                                                                casted.m_casted_type = To;
                                                                return casted;
                                                                }, func->m_signature.state_m | GL::function_signature::Cached | GL::function_signature::Explicit, {}, { { "From", from } }, to);
                                                            options[1]->m_signature.numConversions = func->m_signature.numConversions;
                                                        }
                                                        // return f1->second[to].load();
                                                        continue;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    if (options.size() > 0) {
                                        f1->second[to] = std::move(options.begin()->second);
                                        return f1->second[to].load_fast();
                                    }
                                }

#if 1
                                // before we give up, we should check to see if we can try our basic, castable types. 
                                if (from.is_temp() && (!to.is_ref() || to.is_const_ref())) {
                                    // then also try the base type
                                    auto try_type = from.is_cpp_type() ? (GL::type(from.get_base_hash()) + GL::type::CppType) : GL::type(from.get_base_hash());
                                    if (auto p = try_get_converter(try_type, to, depth + 1, in_function)) {
                                        auto temp = GL::make_callable("`forward_cast " + to.name() + "`", [To = to, FromT = try_type, inF = in_function, this](GL::any::fast_any const& From) -> GL::any {
                                            GL::any::fast_any input = From;
                                            input.m_casted_type = FromT;

                                            if (auto p = this->try_get_converter(FromT, To, 0, inF); p) {
                                                input = p->operator()(&input, &input + 1);
                                            }
                                            else {
                                                throw std::runtime_error("Could not find converter function");
                                            }

                                            input.m_casted_type = To;
                                            return input;
                                            }, GL::function_signature::Cached | p->m_signature.state_m, {}, { { "From", from } }, to);
                                        temp->m_signature.numConversions = p->m_signature.numConversions;
                                        f1->second[to] = std::move(temp);
                                        return f1->second[to].load_fast();
                                    }
                                }
#if 0
                                // before we give up, we should check to see if we can try our basic, castable types. 
                                if (from.is_const_ref() && (!to.is_ref() || to.is_const_ref())) {
                                    // then also try the base type
                                    auto base_from_type = from.is_cpp_type() ? (GL::type(from.get_base_hash()) + GL::type::CppType) : GL::type(from.get_base_hash());
                                    if (auto p1 = try_get_converter(from, base_from_type, depth + 1, in_function)) {
                                        if (auto p2 = try_get_converter(base_from_type, to, depth + 1, in_function)) {
                                            f1->second[to] = GL::make_callable("`forward_cast " + to.name() + "`", [To = to, From1 = from, From2 = base_from_type, inF = in_function, this](GL::any::fast_any const& From) -> GL::any {
                                                GL::any::fast_any input = From;
                                                input.m_casted_type = From1;
                                                if (auto p1 = this->try_get_converter(From1, From2, 0, inF); p1) {
                                                    if (auto p2 = this->try_get_converter(From2, To, 0, inF); p2) {
                                                        input = p1->operator()(&input, &input + 1).fast();
                                                        input = p2->operator()(&input, &input + 1).fast();
                                                        input.m_casted_type = To;
                                                        return input;
                                                    }
                                                }
                                                throw std::runtime_error("Could not find converter function");
                                                }, GL::function_signature::Cached | p1->m_signature.state_m | p2->m_signature.state_m, {}, { { "From", from } }, to);
                                            return f1->second[to].load();
                                        }
                                    }
                                }
#endif

#endif
                            }

                            // nothing was found
                            (*cached)[from].insert({ to, nullptr });
                            if (in_function && (to.is_base() || to.is_const_ref())) {
                                return try_get_converter(from, to | GL::type::Temporary, 0, true);
                            }
                            else {
                                return nullptr;
                            }

                        }
                        else {
                            // we have never made the converters for this "from" type.                             
                            if (depth == 0) {
                                auto locked{ std::scoped_lock(converter_lock) };
                                for (auto& local_converter : constructors.CreateConversions(from))
                                    cached->operator[](from)[local_converter.first] = std::move(local_converter.second);
                                (void)cached->operator[](from);
                            }
                            return try_get_converter(from, to, depth + 1, in_function);
                        }
                    }
                    else {
                        // we need to re-make this cache.
                        converters.EmplaceCache<0>(constructors_version.load(), GL::make_shared<concurrency::concurrent_unordered_map<GL::type, concurrency::concurrent_unordered_map<GL::type, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>>>>());
                        return try_get_converter(from, to, depth, in_function);
                    }
                };
                bool can_convert(GL::type const& from, GL::type const& to, bool in_function = false) {
                    if (try_get_converter(from, to, 0, in_function)) { return true; }
                    if (in_function && (to.is_base() || to.is_const_ref())) {
                        if (try_get_converter(from, to | GL::type::Temporary, 0, true)) {
                            return true;
                        }
                    }
                    return false;
                };

                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func) {
                    if (func) {
                        return func->operator()();
                    }
                    return {};
                };
                template <typename iter_type> GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, iter_type _begin, iter_type const& end) {
                    if constexpr (!std::is_same_v< iter_type, GL::any::fast_any*>) {
                        static_assert(std::is_same_v<iter_type::value_type, GL::any::fast_any>, "iterator must be for a GL::any::fast_any class");
                    }

                    if (!func) return {};
                    
                    bool did_conversion = false;
                    short skipped = 0;
                    short pos = 0;
                    std::array<GL::any::fast_any, 16> params;
                    for (iter_type begin = _begin; (begin != end) && (pos < 16); ++begin, ++pos) {
                        if (!const_cast<any::fast_any*>(&*begin)->can_free_cast(func->m_signature.argument_types_m[pos])) {
                            did_conversion = true;
                            if (GL::fast_shared_ptr<GL::details::Proxy_Function_Base> conversion_func{ try_get_converter(const_cast<any::fast_any*>(&*begin)->m_casted_type, func->m_signature.argument_types_m[pos], 0, true) }; conversion_func) {
                                params[pos] = conversion_func->operator()(const_cast<any::fast_any&>(*begin));
                            }
                            else {
                                throw std::runtime_error("Could not make the cast happen");
                            }
                        }
                        else {
                            if (!did_conversion) {
                                ++skipped;
                            }
                            else {
                                params[pos] = *const_cast<any::fast_any*>(&*begin);
                            }
                        }
                    }
                    if (did_conversion) {
                        for (short p = 0; p < skipped; ++p, ++_begin) params[p] = *const_cast<any::fast_any*>(&*_begin);
                        return func->operator()(&params[0], &params[0] + pos);
                    }
                    else {
                        return func->operator()(_begin, end);
                    }
                };
                // fast path
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, std::vector<any::fast_any>& params) {
                    return call_with_conversions(func, params.begin(), params.end());
                };
                // convenience path, requires casting to any::fast_any
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, const std::vector<any>& params) {
                    std::vector<any::fast_any> Params;
                    Params.resize(params.size());
                    std::transform(params.begin(), params.end(), Params.begin(), [](any const& from) { return from.fast(); });
                    return call_with_conversions(func, Params.begin(), Params.end());
                };
                // convenience path, requires casting to any::fast_any
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, any& param) {
                    any::fast_any p = param.fast();
                    return call_with_conversions(func, &p, &p + 1);
                };
                // fast path
                GL::any::fast_any call_with_conversions(const GL::details::Proxy_Function_Base* func, any::fast_any& param) {
                    return call_with_conversions(func, &param, &param + 1);
                };


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
                concurrency::concurrent_unordered_map<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>
                    using_m; // NOTE: calling "using" should split a normal, BasicScope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
                GL::epoch_map<GL::any, GL::string> // concurrency::concurrent_unordered_map
                    objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 

                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) {};
                template <bool overwriteIfExists> bool EmplaceObject_Impl(GL::string const& sv, GL::any&& Obj) {
                    if constexpr (overwriteIfExists) {
                        objects_m.insert_fast(sv, std::move(Obj));
                    }
                    else {
                        if (auto* f = objects_m.try_at(sv)) return false;
                        else {
                            objects_m.insert_fast(sv, std::move(Obj));
                        }
                    }
                    return true;
                };
                GL::any* GetObject_Impl(GL::string const& sv) {
                    if (auto* f = objects_m.try_at(sv))
                        return f;
                    else
                        return nullptr;
                };
                virtual bool AddUsing_Impl(Breadcrumb* scope) {
                    if (scope) {
                        if (scope->this_m.is_namespace()) {
                            if (auto f = using_m.find(scope); f == using_m.end()) {
                                using_m.insert(std::pair<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>{ scope, GL::callback<NamespaceScope>::ScopedListener() });
                                this->GetNamespace()->invalidate_cache();
                                return true;
                            }
                        }
                    }
                    return false;
                };

            protected:
                BasicScope(GL::string&& name, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
                    : breadcrumb_m(std::move(name), scope_type_p, parent)
                {
                    breadcrumb_m.this_m.scope = this;
                };

            public:
                // Returns true if this scope is a namespace scope
                bool is_namespace() const {
                    return this->breadcrumb_m.this_m.is_namespace();
                };
                // Returns true if this scope is a class scope
                bool is_class() const {
                    return this->breadcrumb_m.this_m.is_class();
                };
                // Returns true if this scope is a root scope
                bool is_root() const {
                    return this->breadcrumb_m.this_m.is_root();
                };
                // Get the immediate parent, if one exists.
                BasicScope* GetParent() const {
                    if (this->breadcrumb_m.parent_m) {
                        return this->breadcrumb_m.parent_m->this_m.scope;
                    }
                    else {
                        return nullptr;
                    }
                };
                // Get the current namespace (for inserting functions, etc)
                NamespaceScope* GetNamespace() const {
                    if (this->breadcrumb_m.namespace_m) {
                        return static_cast<NamespaceScope*>(this->breadcrumb_m.namespace_m->this_m.scope);
                    }
                    else {
                        return nullptr;
                    }
                };
                // Get the root of the entire scope tree
                RootScope* GetRoot() const {
                    if (this->breadcrumb_m.root_m) {
                        return static_cast<RootScope*>(this->breadcrumb_m.root_m->this_m.scope);
                    }
                    else {
                        return nullptr;
                    }
                };

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
                static check_cache& GetCheckMap() {
                    thread_local check_cache out;
                    out.clear();
                    return out;
                };
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

                    // test my personal "using" namespaces completely
                    if (using_m.size() > 0ull) {
                        for (auto& childNamespace : using_m) {
                            if ((check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) > 0) { continue; }
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
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
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if ((flag2 & CheckFlagState::all) > 0) continue;
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
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
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                                    if ((flag & CheckFlagState::all) > 0) { continue; }
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
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
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if ((flag2 & CheckFlagState::all) > 0) continue;
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }
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

                    return finalResult;
                };

                static __declspec(noinline) Breadcrumb* FindNamespace(GL::string const& Name, Breadcrumb* start) {
                    if (!start) return nullptr;
                    if (Name.empty()) return start->root_m;

                    NamespaceScope* NS = start->this_m.scope->GetNamespace();
                    if (NS) {
                        auto& current_cache = NS->namespace_search_cache.TryGetCache<0>(NS->cache_version);
                        if (current_cache) {
                            if (auto** cache = current_cache->try_at(Name.hash())) {
                                return (*cache);
                            }
                        }
                        else {
                            NS->namespace_search_cache.EmplaceCache<0>(NS->cache_version, GL::make_shared< GL::epoch_map<Breadcrumb*, size_t> >());
                        }
                    }

                    static thread_local size_t len;
                    len = Name.length();
                    static thread_local std::set< size_t> target_hash; {
                        target_hash.clear();
                        target_hash.insert(Name.hash());
                        auto temp = Name.remove_leading(':');
                        auto* BC = start;
                        while (BC) {
                            target_hash.insert(temp.hash(BC->GetCurrentNamespace().hash()));
                            BC = BC->parent_m;
                        }
                    }

                    if (target_hash.count(GL::string::namespace_colons().hash()) > 0) {
                        return start->root_m;
                    }
                    else if (Breadcrumb* BC = start->this_m.scope->FindNearestScopeWhere([stringified = GL::string(Name)](Breadcrumb* namespacePtr, int search_state)-> int {
                        if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                        GL::string const& currNS{ namespacePtr->GetCurrentNamespace() };
                        if (target_hash.count(currNS.hash()) > 0) return SearchResult::Success;
                        else if (search_state & SearchingChildren) {
                            if (len < currNS.length()) return SearchResult::Failure | SearchResult::StaticFailure;
                            else if (stringified.find(currNS/*, true, stringified.length() - currNS.length()*/) == GL::string::npos) return SearchResult::Failure | SearchResult::StaticFailure;
                            else return SearchResult::Failure;
                        }
                        else return SearchResult::Failure;
                    })) {
                        if (auto& current_cache = NS->namespace_search_cache.TryGetCache<0>(NS->cache_version); current_cache) {
                            current_cache->insert_fast(Name.hash(), (Breadcrumb*)BC);
                        }
                        return BC;
                    }
                    else {
                        if (auto& current_cache = NS->namespace_search_cache.TryGetCache<0>(NS->cache_version); current_cache) {
                            current_cache->insert_fast(Name.hash(), (Breadcrumb*)nullptr);
                        }
                        return nullptr;
                    }
                };

            public:
                BasicScope() = delete;
                virtual ~BasicScope() {
                    //if (!is_namespace()) {
                        if (using_m.size() > 0) {
                            GetNamespace()->invalidate_cache();
                        }
                    //}
                };

                /// <summary>
                /// Get the index that is unique to this scope, which will remain unique for the life of the scope. May be re-used after the scope ends. 
                /// </summary>
                /// <returns>size_t</returns>
                size_t get_unique_index() const {
                    return const_cast<BasicScope*>(this)->breadcrumb_m.GetScopeIndex();
                };

                /// <summary>
                /// Make a child scope from this scope. 
                /// Thread-safe, and allowed to make many children of this scope in parallel safely. 
                /// The created scope (and its children) are destroyed at the end of this C++ scope. 
                /// </summary>
                BasicScope make_scope() const {
                    return BasicScope("", ScopeType::Basic, const_cast<Breadcrumb*>(&this->breadcrumb_m));
                };

                /// <summary>
                /// Instruct this scope to "use" the provided namespace when searching for objects, functions, or other scopes by name. 
                /// If this scope is a namespace, this will reset the search cache.
                /// </summary>
                /// <param name="ptr"></param>
                /// <returns></returns>
                bool add_using_here(NamespaceScope const& ptr) {
                    if (auto p = static_cast<const BasicScope*>(&ptr)) {
                        if (this == p) return false; // may not "use" yourself.
                        if (this->is_root()) return false; // the root may not call the using statement
                        if (p->is_root()) return false; // the root may not be "used"
                        return this->AddUsing_Impl(const_cast<Breadcrumb*>(&p->breadcrumb_m));
                    }
                    return false;
                }

                /// <summary>
                /// Insert an object into this scope only if it does not yet exist. Does not search neighbors or review the object name.
                /// </summary>
                /// <param name="sv"></param>
                /// <param name="Obj"></param>
                /// <returns>bool</returns>
                __declspec(noinline) bool insert_object_here(GL::string const& sv, GL::any&& Obj) {
                    if (this->EmplaceObject_Impl<false>(sv, std::move(Obj))) {
                        // check the cache to make sure we aren't changing something from "empty" to "existing"
                        if (this->is_root()) {
                            if (Breadcrumb* BC = this->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> int {
                                if (namespacePtr->this_m.is_root()) return SearchResult::Failure;
                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure | SearchResult::StaticFailure;
                                namespacePtr->this_m.scope->invalidate_cache();
                                return SearchResult::Failure;
                                }, nullptr, SearchState::SkipParent)) {
                            };
                        }
                        else if (this->is_namespace()) {
                            if (Breadcrumb* BC = this->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> int {
                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure | SearchResult::StaticFailure;
                                namespacePtr->this_m.scope->invalidate_cache();
                                return SearchResult::Failure;
                                }, nullptr, 0)) {
                            };
                        }
                        return true;
                    }
                    return false;
                };

                /// <summary>
                /// Emplace an object into this scope whether or not it exists. Does not search neighbors or review the object name.
                /// </summary>
                /// <param name="sv"></param>
                /// <param name="Obj"></param>
                /// <returns>bool</returns>
                __declspec(noinline) bool emplace_object_here(GL::string const& sv, GL::any&& Obj) {
                    if (this->EmplaceObject_Impl<true>(sv, std::move(Obj))) {
                        // check the cache to make sure we aren't changing something from "empty" to "existing"
                        if (this->is_root()) {
                            if (Breadcrumb* BC = this->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> int {
                                if (namespacePtr->this_m.is_root()) return SearchResult::Failure;
                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure | SearchResult::StaticFailure;
                                namespacePtr->this_m.scope->invalidate_cache();
                                return SearchResult::Failure;
                                }, nullptr, SearchState::SkipParent)) {
                            };
                        }
                        else if (this->is_namespace()) {
                            if (Breadcrumb* BC = this->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> int {
                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure | SearchResult::StaticFailure;
                                namespacePtr->this_m.scope->invalidate_cache();
                                return SearchResult::Failure;
                                }, nullptr, 0)) {
                            };
                        }
                        return true;
                    }
                    return false;
                };

                /// <summary>
                /// Try to find an object in this scope. Does not search neighbors or review the object name. Since objects cannot be removed, it safely returns a pointer. 
                /// </summary>
                /// <returns>ObjectWrapper</returns>
                GL::any* find_object_here(GL::string const& sv) const {
                    return const_cast<BasicScope*>(this)->GetObject_Impl(sv);
                };

                // Searches for a namespace that best fits the provided information, starting from this scope's namespace or position.
                Breadcrumb* find_namespace(GL::string const& Name) const {
                    auto* NS = this->GetNamespace();
                    //if (auto* cache = NS->search_cache.TryGetCache<0>(NS->cache_version, Name.hash())) {
                    //    if (cache == reinterpret_cast<Breadcrumb*>(1)) {
                    //        return nullptr;
                    //    }
                    //    else {
                    //        return cache;
                    //    }
                    //}

                    if (auto* out = FindNamespace(
                        make_scope_name(Name), // "std" or "::std" or "::std::" -> "::std::" 
                        &const_cast<BasicScope*>(this)->breadcrumb_m // where
                    )) {
                        //NS->search_cache.EmplaceCache<0>(NS->cache_version, Name.hash(), out);
                        return out;
                    }
                    else {
                        //NS->search_cache.EmplaceCache<0>(NS->cache_version, Name.hash(), reinterpret_cast<Breadcrumb*>(1));
                        return nullptr;
                    }
                };
            private:
                Breadcrumb* FindNamespaceImpl(GL::string const& Name, Breadcrumb*& nearest_scope) const {
                    if (auto* out = find_namespace(Name)) {
                        nearest_scope = out;
                        return out;
                    }
                    else {
                        if (!nearest_scope) nearest_scope = this->breadcrumb_m.root_m;
                        const auto& [left, right] = Name.remove_leading_and_trailing(':').left_and_right_of_last("::");
                        if (right.length() > 0) {
                            if (auto* out = FindNamespaceImpl(left, nearest_scope)) {
                                nearest_scope = out;
                                return out->this_m.scope->FindNamespaceImpl(right, nearest_scope);
                            }
                            else {
                                return nullptr;
                            }
                        }
                        else {
                            // no colons inside
                            return nullptr;
                        }
                    }
                };
            public:
                // Searches for a namespace while also specifying the "closest" it was able to get to the requested namespace. Useful for debugging where the search last ended. 
                Breadcrumb* find_namespace(GL::string const& Name, Breadcrumb*& nearest_scope) const {
                    Breadcrumb* out = FindNamespaceImpl(Name, nearest_scope);
                    if (out) {
                        return out->this_m.scope->find_namespace(Name);
                    }
                    return nullptr;
                };

            public:
                // User is allowed to request a scoped object, e.g. "x" or "::x" or "::std::string::npos"
                GL::any* find_object(GL::string const& PossiblyScopedName, Breadcrumb* search_from = nullptr) const {
                    GL::any
                        * p = nullptr;
                    if (search_from) {
                        if (p = search_from->this_m.scope->find_object_here(PossiblyScopedName)) {
                            return p;
                        }

                        auto* NS = this->GetNamespace();
                        //if (auto* cache = NS->search_cache.TryGetCache<1>(NS->cache_version, PossiblyScopedName.hash())) {
                        //    if (cache == reinterpret_cast<Breadcrumb*>(1)) {
                        //        return nullptr;
                        //    }
                        //    else {
                        //        p = reinterpret_cast<GL::any*>(cache);
                        //        return p;
                        //    }
                        //}

                        // we have a scope with a specific object name
                        // PossiblyScopedName should NOT have colons in this case. 
                        if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state)-> int {
                            // we should not be able to find objects in children scopes
                            if ((SearchState::SearchingChildren & search_state) > 0) {
                                throw std::runtime_error("This should never happen");
                                // p = nullptr;
                                // return SearchResult::Failure | SearchResult::StaticFailure;
                            }

                            if (p = namespacePtr->this_m.scope->find_object_here(PossiblyScopedName)) {
                                return SearchResult::Success;
                            }
                            else {
                                p = nullptr;
                                return SearchResult::Failure;
                            }
                            }, nullptr, SearchState::SkipChildren)) {
                            //NS->search_cache.EmplaceCache<1>(NS->cache_version, PossiblyScopedName.hash(), reinterpret_cast<Breadcrumb*>(p));
                            return p;
                        }
                        else {
                            //NS->search_cache.EmplaceCache<1>(NS->cache_version, PossiblyScopedName.hash(), reinterpret_cast<Breadcrumb*>(1));
                            return nullptr;
                        }
                    }
                    else {
                        // we don't have a scope (yet)
                        const auto& [optionalScope, optionalName] = PossiblyScopedName.left_and_right_of_last("::");
                        if (optionalName.length() == 0) {
                            // We only have an object name -- just do the normal search from here.
                            p = find_object(optionalScope, &const_cast<BasicScope*>(this)->breadcrumb_m);
                            return p;
                        }
                        else {
                            Breadcrumb* closest_scope{ nullptr };
                            if (optionalScope.length() == 0) {
                                p = find_object(optionalName, this->breadcrumb_m.root_m);
                                return p;
                            }
                            else if (auto nameSpace = find_namespace(optionalScope, closest_scope)) {
                                return nameSpace->this_m.scope->find_object(optionalName, nameSpace);
                            }
                            else if (closest_scope) { // namespace was not found                        
                                return nullptr; //  throw GoodLang::exception::not_found_error(GoodLang::printf("Could not located object '%s' in namespace '%s'", optionalName.c_str().data(), closest_scope->GetCurrentNamespace().c_str().data()));
                            }
                            else { // namespace was not found AND no nearest was discovered     
                                return nullptr; // throw GoodLang::exception::not_found_error(GoodLang::printf("Could not located object '%s'", optionalName.c_str().data()));
                            }
                        }
                    }
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
                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override {
                    InterlockedIncrement(static_cast<volatile size_t*>(&cache_version));
                    sockets_for_cache_versions.speak(parent_alive, call_number);                    
                };

            protected:
                virtual bool AddUsing_Impl(Breadcrumb* scope) override {
                    if (scope) {
                        if (scope->this_m.is_namespace()) {
                            if (auto* p = dynamic_cast<NamespaceScope*>(scope->this_m.scope)) {
                                using_m.insert(std::pair<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>{ scope, p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this) });
                                invalidate_cache();
                                return true;
                            }
                        }
                    }
                    return false;
                };

            public:
                // instancing a child namespace should only be done from an existing namespace
                NamespaceScope(GL::string&& name, int scope_type_p = ScopeType::Basic | ScopeType::Namespace, Breadcrumb* parent = nullptr)
                    : BasicScope(std::move(name), scope_type_p, parent)
                    , children{}
                    , search_cache{}
                    , sockets_for_cache_versions(&NamespaceScope::invalidate_cache)
                    , connection_for_cache_version{}
                    , cache_version{ 0 }
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
                    for (auto& x : this->using_m) x.second = {};
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
                    if (using_m.size() > 0ull) {
                        for (auto& childNamespace : using_m) {
                            if ((check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) > 0) { continue; }
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
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
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if ((flag2 & CheckFlagState::all) > 0) continue;
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
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
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                                    if ((flag & CheckFlagState::all) > 0) { continue; }
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
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
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if ((flag2 & CheckFlagState::all) > 0) continue;
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
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

                /// <summary>
                /// Finds or creates a new namespace scope as a child of this one, and keeps it in memory. 
                /// The created scope will survive for the life of this parent scope. 
                /// If a namespace already exists with the provided name, it will return the existing namespace without creating a new one or overwritting the existing one.
                /// </summary>
                /// <returns>NamespaceScope</returns>
                NamespaceScope& make_namespace(GL::string const& name) {
                    if (auto f = children.find(name.hash()); f != children.end()) {
                        return *f->second;
                    }
                    else {
                        this->invalidate_cache();
                        return *children.insert(
                            { name.hash(), std::shared_ptr<NamespaceScope>(new NamespaceScope((GL::string)name, ScopeType::Basic | ScopeType::Namespace, const_cast<Breadcrumb*>(&this->breadcrumb_m))) }
                        ).first->second;
                    }
                };

                /// <summary>
                /// Finds or creates a new class scope as a child of this one, and keeps it in memory. 
                /// The created class scope will survive for the life of this parent scope. 
                /// If a namespace already exists with the provided name or type, it will return the existing namespace without creating a new one or overwritting the existing one.
                /// </summary>
                /// <returns>NamespaceScope</returns>
                ClassScope& make_class(GL::type class_type) {
                    class_type -= GL::type::Const;
                    class_type -= GL::type::Reference;
                    class_type -= GL::type::Temporary;

                    if (auto f = children.find(class_type.name().hash()); (f != children.end()) && (f->second) && (f->second->is_class())) {                        
                        return *std::dynamic_pointer_cast<ClassScope>(f->second);
                    }
                    else {
                        this->invalidate_cache();
                        auto new_class = std::dynamic_pointer_cast<ClassScope>(children.insert(
                            { class_type.name().hash(), std::dynamic_pointer_cast<NamespaceScope>(std::shared_ptr<ClassScope>(new ClassScope(class_type, ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, const_cast<Breadcrumb*>(&this->breadcrumb_m)))) }
                        ).first->second);
                        this->GetRoot()->classes.insert({ class_type.get_base_hash(), &new_class->breadcrumb_m });
                        this->GetRoot()->classes_by_name.insert({ class_type.name(), &new_class->breadcrumb_m });
                        return *new_class;
                    }
                };

                // attempts to find a suitable function from this set that is callable with the given parameters. 
                // searches for object-lambdas (lambda or not), non-template-functions, and template-functions, in that order of preference. 
                template<typename iter> const GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_find_callable(GL::string const& PossiblyScopedName, iter const& from_iter, iter const& from_end, int search_state = 0, Breadcrumb* search_from = nullptr) const {
                    if (search_from) {
                        auto total_hash = GL::util::inline_hash(search_from->this_m.scope->GetNamespace()->cache_version, search_from->this_m.scope->GetRoot()->constructors_version.load());

                        size_t search_hash = PossiblyScopedName.hash();
                        if (from_iter != from_end) {
                            if constexpr (std::is_same_v<iter, GL::type*> || std::is_same_v<iter::value_type, GL::type>) {
                                for (iter _iter = from_iter; (_iter != from_end); ++_iter) {
                                    GL::util::hash(search_hash, _iter->get_hash());
                                }
                            }
                            else if constexpr (std::is_same_v<iter, GL::any::fast_any*> || std::is_same_v<iter, GL::any*> || std::is_same_v<iter::value_type, GL::any::fast_any> || std::is_same_v<iter::value_type, GL::any>) {
                                for (iter _iter = from_iter; (_iter != from_end); ++_iter) {
                                    GL::util::hash(search_hash, _iter->m_casted_type.get_hash());
                                }
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
                                    }
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
                                            search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)o->cast<GL::Proxy_Function>());
                                            return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                        }
                                    }
                                    else {
                                        if (o->cast<GL::details::Proxy_Function_Base const&>().m_signature.can_call_with_cast(from_iter, from_end)) {
                                            search_from->this_m.scope->GetNamespace()->search_cache.at<0>(total_hash)->insert_fast(search_hash, (GL::Proxy_Function)o->cast<GL::Proxy_Function>());
                                            return GL::fast_shared_ptr<GL::details::Proxy_Function_Base>(o->cast<GL::Proxy_Function>());
                                        }
                                    }
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

                        // search for template match?
                        if ((search_state & Functions::search_conditions::ignore_templates) == 0) {
                            if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int)-> int {
                                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                                if (auto const& f = namespacePtr->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::only_templates | search_state); f) {
                                    return SearchResult::Success;
                                }
                                else {
                                    return SearchResult::Failure;
                                }
                                }, nullptr, SkipChildren)) {
                                // re-do the search
                                if (auto const& f = BC->this_m.scope->GetNamespace()->functions.try_find_callable(PossiblyScopedName, from_iter, from_end, Functions::search_conditions::only_templates | search_state); f) {
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
                            GL::type this_t;

                            if constexpr (std::is_same_v<iter, GL::type*> || std::is_same_v<iter::value_type, GL::type>) {
                                this_t = *from_iter;                                
                            }
                            else if constexpr (std::is_same_v<iter, GL::any::fast_any*> || std::is_same_v<iter, GL::any*> || std::is_same_v<iter::value_type, GL::any::fast_any> || std::is_same_v<iter::value_type, GL::any>) {
                                this_t = from_iter->m_casted_type;                                
                            }                

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
                        // we don't have a scope (yet)
                        const auto& [optionalScope, optionalName] = PossiblyScopedName.left_and_right_of_last("::");
                        if (optionalName.length() == 0) {
                            // We only have an object name -- just do the normal search from here.
                            if (auto f = try_find_callable(optionalScope, from_iter, from_end, search_state, &const_cast<NamespaceScope*>(this)->breadcrumb_m); f) {
                                return std::move(f);
                            }
                            else {
                                //auto total_hash = GL::util::inline_hash(this->GetNamespace()->cache_version, this->GetRoot()->constructors_version.load());
                                //size_t search_hash = optionalScope.hash();
                                //if (from_iter != from_end) {
                                //    if constexpr (std::is_same_v<iter, GL::type*> || std::is_same_v<iter::value_type, GL::type>) {
                                //        for (iter _iter = from_iter; (_iter != from_end); ++_iter) {
                                //            GL::util::hash(search_hash, _iter->get_hash());
                                //        }
                                //    }
                                //    else if constexpr (std::is_same_v<iter, GL::any::fast_any*> || std::is_same_v<iter, GL::any*> || std::is_same_v<iter::value_type, GL::any::fast_any> || std::is_same_v<iter::value_type, GL::any>) {
                                //        for (iter _iter = from_iter; (_iter != from_end); ++_iter) {
                                //            GL::util::hash(search_hash, _iter->m_casted_type.get_hash());
                                //        }
                                //    }
                                //}                                
                                //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                //    // cache exists for this moment - no new constructor or function or object has been added recently. 
                                //    if (auto* node = cache->try_at(search_hash); node) return node->load_fast();
                                //}
                                //else {
                                //    this->GetNamespace()->search_cache.EmplaceCache<0>(total_hash, GL::make_shared< GL::epoch_map<GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>, size_t>>());
                                //}

                                if (from_iter != from_end) {
                                    if constexpr (std::is_same_v<iter, GL::type*> || std::is_same_v<iter::value_type, GL::type>) {
                                        std::queue< GL::type > types_to_try;
                                        types_to_try.push(*from_iter);
                                        std::set<GL::type> attempted_types;
                                        while (types_to_try.size() > 0) {
                                            GL::type this_t = types_to_try.front();
                                            types_to_try.pop();
                                            if (attempted_types.find(this_t) == attempted_types.end()) {
                                                attempted_types.insert(this_t);

                                                if (auto search = this->GetRoot()->classes.find(this_t.get_base_hash()), e = this->GetRoot()->classes.end(); search != e) {
                                                    if (auto ff = try_find_callable(optionalScope, from_iter, from_end, search_state, search->second); ff) {
                                                        //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                                        //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                                        //}
                                                        return std::move(ff);
                                                    }
                                                }

                                                for (GL::type const& base_type : this_t.all_base_types()) { types_to_try.push(base_type); }
                                            }
                                        }                                        
                                    }
                                    else if constexpr (std::is_same_v<iter, GL::any::fast_any*> || std::is_same_v<iter, GL::any*> || std::is_same_v<iter::value_type, GL::any::fast_any> || std::is_same_v<iter::value_type, GL::any>) {
                                        std::queue< GL::type > types_to_try;
                                        types_to_try.push(from_iter->m_casted_type);
                                        std::set<GL::type> attempted_types;
                                        while (types_to_try.size() > 0) {
                                            GL::type this_t = types_to_try.front();
                                            types_to_try.pop();
                                            if (attempted_types.find(this_t) == attempted_types.end()) {
                                                attempted_types.insert(this_t);

                                                if (auto search = this->GetRoot()->classes.find(this_t.get_base_hash()), e = this->GetRoot()->classes.end(); search != e) {
                                                    if (auto ff = try_find_callable(optionalScope, from_iter, from_end, search_state, search->second); ff) {
                                                        //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                                        //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                                        //}
                                                        return std::move(ff);
                                                    }
                                                }

                                                for (GL::type const& base_type : this_t.all_base_types()) { types_to_try.push(base_type); }
                                            }
                                        }                                        
                                    }
                                }
                                // is the "optionalScope" name an exact match for a class name? They may be trying to invoke a class... 
                                if (auto search = this->GetRoot()->classes_by_name.find(optionalScope), end = this->GetRoot()->classes_by_name.end(), search2 = search; search != end) {
                                    ++search2;
                                    if ((search2 != end) && (search2->first == search->first)) {
                                        // handle the case where multiple classes share the same name
                                        if (Breadcrumb* BC = this->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int) -> int {
                                            if (!namespacePtr->this_m.is_class()) return SearchResult::Failure;
                                            if (namespacePtr->this_m.scope_name == optionalScope) return SearchResult::Success;
                                            return SearchResult::Failure;
                                        })) {
                                            if (auto ff = try_find_callable(optionalScope, from_iter, from_end, search_state, BC); ff) {
                                                //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                                //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                                //}
                                                return std::move(ff);
                                            }
                                        };
                                    }
                                    else {
                                        // special case when there is only one class that shares this unique name.
                                        if (auto ff = try_find_callable(optionalScope, from_iter, from_end, search_state, search->second); ff) {
                                            //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                            //    cache->insert_fast(search_hash, (GL::Proxy_Function)ff);
                                            //}
                                            return std::move(ff);
                                        }
                                    }
                                }
                                //if (auto& cache = this->GetNamespace()->search_cache.TryGetCache<0>(total_hash); cache) {
                                //    cache->insert_fast(search_hash, (GL::Proxy_Function)nullptr);
                                //}
                                return nullptr;                                
                            }
                        }
                        else {
                            Breadcrumb* closest_scope{ nullptr };
                            if (optionalScope.length() == 0) {
                                return try_find_callable(optionalName, from_iter, from_end, search_state, this->breadcrumb_m.root_m);
                            }
                            else if (auto nameSpace = find_namespace(optionalScope, closest_scope)) {
                                return dynamic_cast<NamespaceScope*>(nameSpace->this_m.scope)->try_find_callable(optionalName, from_iter, from_end, search_state, nameSpace);
                            }
                            else if (closest_scope) { // namespace was not found                        
                                return nullptr;
                            }
                            else { // namespace was not found AND no nearest was discovered     
                                return nullptr;
                            }
                        }
                    }
                };

                template<typename iter_type> GL::any::fast_any call(GL::string const& PossiblyScopedName, iter_type const& from_iter, iter_type const& from_end) const {
                    if (auto f = try_find_callable(PossiblyScopedName, from_iter, from_end); f) {
                        return this->GetRoot()->get_converters().call_with_conversions(&*f, from_iter, from_end);
                    }
                    else {
                        GL::string params;
                        for (iter_type i = from_iter; i != from_end; ++i) {
                            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                                params = params.add_to_delim(i->name(), ", ");
                            }
                            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                                params = params.add_to_delim(i->m_casted_type.name(), ", ");
                            }
                            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                                params = params.add_to_delim(i->m_casted_type.name(), ", ");
                            }
                            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                                params = params.add_to_delim(i->name(), ", ");
                            }
                            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                                params = params.add_to_delim(i->m_casted_type.name(), ", ");
                            }
                            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                                params = params.add_to_delim(i->m_casted_type.name(), ", ");
                            }
                            else {
                                static_assert("iter_type not allowed");
                            }
                        }

                        GL::string err = GL::string("Could not find callable matching `") + PossiblyScopedName + "`(" + params + ").";
                        throw std::runtime_error(err.to_string());
                    }
                };
                GL::any::fast_any call(GL::string const& PossiblyScopedName, std::vector<GL::any::fast_any> const& params) const {
                    return call(PossiblyScopedName, params.begin(), params.end());
                };

                // insert a function into the storage
                void add_function(GL::Proxy_Function const& func) {
                    functions.add_function(func);
                    if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                        this->GetRoot()->add_constructor(func);
                    }
                    this->invalidate_cache();
                };

            };

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
                {};
                ClassScope() = delete;
                virtual ~ClassScope() {};
            public:
                const GL::type this_type;

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
                // TypedCache<concurrency::concurrent_unordered_map<GL::type, concurrency::concurrent_unordered_map<GL::type, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>>>, 1>
                    // converters;
                TypedCache<concurrency::concurrent_unordered_map<GL::type, concurrency::concurrent_unordered_map<GL::type, GL::atomic_shared_ptr<GL::details::Proxy_Function_Base>>>, 1>
                    converters;
                std::mutex // GL::fast_shared_mutex
                    converter_lock{};
                std::atomic<long long>
                    constructors_version{ 0 };
                concurrency::concurrent_unordered_map<size_t, Breadcrumb*>
                    classes; // list of all unique classes, which cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option.  
                concurrency::concurrent_unordered_multimap<GL::string, Breadcrumb*>
                    classes_by_name; // list of all unique classes, which cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option.  

            public:
                RootScope()
                    : NamespaceScope("::", ScopeType::Basic | ScopeType::Namespace | ScopeType::Root, nullptr)
                {};
                virtual ~RootScope() {
                    this->unload(); // must call the namespace's unload function BEFORE this destroys itself, otherwise connections are unable to resolve themselves. 
                    converters.unsafe_unload();
                };

                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override {
                    InterlockedIncrement(static_cast<volatile size_t*>(&this->cache_version));
                    this->sockets_for_cache_versions.speak(parent_alive, call_number);

                    // constructors.clear();
                };
                void add_constructor(GL::Proxy_Function const& func) {
                    constructors.add_function(func);
                    ++constructors_version;
                };

                Converter get_converters() {
                    return Converter(constructors, converters, converter_lock, constructors_version);
                };

                GL::fast_shared_ptr<GL::details::Proxy_Function_Base> try_get_converter(GL::type const& from, GL::type const& to, bool in_function = false) {
                    return get_converters().try_get_converter(from, to, 0, in_function);
                };
                bool can_convert(GL::type const& from, GL::type const& to, bool in_function = false) {
                    return get_converters().can_convert(from, to, in_function);
                };

                std::vector<GL::type> all_convertable_types() const;
                void perform_builtins();
                void preload_conversions();

            };


        };



    }
}