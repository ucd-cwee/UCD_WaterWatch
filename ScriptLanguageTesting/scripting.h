#pragma once

#include "functions.h"

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
                    , scope_type{ std::move(scope_type_p) }
                {}
                ScopeID(ScopeID&& rhs)
                    : scope_name{ std::move(rhs.scope_name) }
                    , scope{ std::move(rhs.scope) }
                    , scope_type{ std::move(rhs.scope_type) }
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
                template<int category> void EmplaceCache(size_t cache_version, size_t input_hash, Breadcrumb* result) {
                    auto g{ _current_cache.ProtectCurrentEpoch() };
                    bool success = false;
                    while (!success) {
                        if (!_current_cache.do_at_end([&](size_t curr_version, std::array<GL::deferred<ResultForInputType>, numCategories>& cache) {
                            if (curr_version >= cache_version) {
                                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&cache[category]->operator[](input_hash)), reinterpret_cast<PVOID>(result));
                                success = true;
                            }
                            })) {
                        };
                        if (!success) {
                            (void)_current_cache.operator[](cache_version); // default-initializes the item at the specified index if it does not already exist. 
                            _current_cache.pop_front_if([&](size_t curr_version, std::array<GL::deferred<ResultForInputType>, numCategories>& cache) -> bool {
                                return curr_version < cache_version;
                                });
                        }
                    }
                };

                // Try to copy an item from the cache.
                template<int category> Breadcrumb* TryGetCache(size_t cache_version, size_t input_hash) {
                    auto g{ _current_cache.ProtectCurrentEpoch() };
                    Breadcrumb* out{ nullptr };
                    _current_cache.do_at_end([&](size_t curr_version, std::array<GL::deferred<ResultForInputType>, numCategories>& cache) {
                        if (curr_version >= cache_version) {
                            if (cache[category].valid()) {
                                if (auto f = cache[category]->find(input_hash); f != cache[category]->end()) {
                                    out = f->second;
                                }
                            }

                            // out = cache[category]->operator[](input_hash);
                        }
                        });
                    return out;
                };

            };

            /// <summary>
            /// Foundational element of a scope. Should not be created on its own, and instead should be issued by a parent.
            /// </summary>
            class BasicScope {
                friend class NamespaceScope;
                friend class RootScope;
                friend class Breadcrumb;
            protected:
                Breadcrumb
                    breadcrumb_m;
                concurrency::concurrent_unordered_map<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>
                    using_m; // NOTE: calling "using" should split a normal, BasicScope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
                concurrency::concurrent_unordered_map<GL::string, GL::any>
                    objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 

                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) {};
                template <bool overwriteIfExists> bool EmplaceObject_Impl(GL::string const& sv, GL::any&& Obj) {
                    if constexpr (overwriteIfExists)
                        objects_m[sv] = std::move(Obj);
                    else
                        objects_m.insert({ sv, std::move(Obj) });
                    return true;
                };
                GL::any* GetObject_Impl(GL::string const& sv) {
                    if (auto f = objects_m.find(sv), e = objects_m.end(); f != e)
                        return &f->second;
                    return nullptr;
                };
                virtual void AddUsing_Impl(Breadcrumb* scope) {
                    if (scope) {
                        if (scope->this_m.is_namespace()) {
                            if (auto f = using_m.find(scope); f == using_m.end()) {
                                using_m.insert(std::pair<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>{ scope, GL::callback<NamespaceScope>::ScopedListener() });
                                invalidate_cache(); // does nothing for normal scopes
                            }
                        }
                    }
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
                virtual Breadcrumb* FindNearestScopeWhere(
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
                            check_flags.resize(numTickets);
                        }
                        for (auto& x : check_flags) x = CheckFlagState::none;
                    }

                    // Prevent Duplication
                    if (check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all) {
                        finalResult = nullptr;
                        return finalResult;
                    }
                    if (searchState & SkipChildren) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
                    }

                    // test myself directly	
                    if (!(check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::self)) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::self;

                        auto res = func(&selfPtr, searchState);
                        if (res & SearchResult::Success) {
                            finalResult = &selfPtr;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            finalResult = nullptr;
                            return finalResult;
                        }
                    }

                    // test my personal "using" namespaces completely
                    if (using_m.size() > 0ull) {
                        for (auto& childNamespace : using_m) {
                            if (check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) { continue; }
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
                            if (flag & CheckFlagState::self) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            // check the using statements of the parent.
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if (flag2 & CheckFlagState::all) continue;
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
                            if (!(flag1 & CheckFlagState::self)) {
                                flag1 |= CheckFlagState::self;

                                auto res = func(thisParent, searchState);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    finalResult = nullptr;
                                    return finalResult;
                                }
                            }

                            // test my personal "using" namespaces completely
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                                    if (flag & CheckFlagState::all) { continue; }
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }
                            }
                        }
                        while (thisParent = thisParent->parent_m) {
                            auto& flag = check_flags[thisParent->GetScopeIndex()];
                            if (flag & CheckFlagState::self) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            // check the using statements of the parent.
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if (flag2 & CheckFlagState::all) continue;
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }
                            }
                        }
                    }

                    // Test my parent completely.
                    if (!(searchState & SkipParent)) {
                        if (selfPtr.parent_m) {
                            auto& flag = check_flags[selfPtr.parent_m->GetScopeIndex()];
                            if (!(flag & CheckFlagState::all)) {
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
                        return BC;
                    }
                    else {
                        return nullptr;
                    }
                };

            public:
                BasicScope() = delete;
                virtual ~BasicScope() = default;

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
                void add_using_here(NamespaceScope const& ptr) {
                    if (auto p = static_cast<const BasicScope*>(&ptr)) {
                        if (this == p) return; // may not "use" yourself.
                        this->AddUsing_Impl(const_cast<Breadcrumb*>(&p->breadcrumb_m));
                    }
                }

                /// <summary>
                /// Insert an object into this scope only if it does not yet exist. Does not search neighbors or review the object name.
                /// </summary>
                /// <param name="sv"></param>
                /// <param name="Obj"></param>
                /// <returns>bool</returns>
                bool insert_object_here(GL::string const& sv, GL::any&& Obj) {
                    return this->EmplaceObject_Impl<false>(sv, std::move(Obj));
                };

                /// <summary>
                /// Emplace an object into this scope whether or not it exists. Does not search neighbors or review the object name.
                /// </summary>
                /// <param name="sv"></param>
                /// <param name="Obj"></param>
                /// <returns>bool</returns>
                bool emplace_object_here(GL::string const& sv, GL::any&& Obj) {
                    return this->EmplaceObject_Impl<true>(sv, std::move(Obj));
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
                    if (auto* cache = NS->search_cache.TryGetCache<0>(NS->cache_version, Name.hash())) {
                        return cache;
                    }

                    if (auto* out = FindNamespace(
                        make_scope_name(Name), // "std" or "::std" or "::std::" -> "::std::" 
                        &const_cast<BasicScope*>(this)->breadcrumb_m // where
                    )) {
                        NS->search_cache.EmplaceCache<0>(NS->cache_version, Name.hash(), out);
                        return out;
                    }
                    else {
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
                    GL::any* p{ nullptr };
                    if (search_from) {
                        // we have a scope with a specific object name
                        // PossiblyScopedName should NOT have colons in this case. 
                        if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state)-> int {
                            // we should not be able to find objects in children scopes
                            // search_state & Scopes::BasicScope::
                            if (SearchState::SearchingChildren & search_state) {
                                return SearchResult::Failure | SearchResult::StaticFailure;
                            }

                            if (p = namespacePtr->this_m.scope->find_object_here(PossiblyScopedName)) {
                                return SearchResult::Success;
                            }
                            else {
                                return SearchResult::Failure;
                            }
                            }, nullptr, SearchState::SkipChildren)) {
                            return p;
                        }
                        else {
                            return nullptr;
                        }
                    }
                    else {
                        auto* NS = this->GetNamespace();
                        if (auto* cache = NS->search_cache.TryGetCache<1>(NS->cache_version, PossiblyScopedName.hash())) {
                            p = reinterpret_cast<GL::any*>(cache);
                            return p;
                        }

                        // we don't have a scope (yet)
                        const auto& [optionalScope, optionalName] = PossiblyScopedName.left_and_right_of_last("::");
                        if (optionalName.length() == 0) {
                            // We only have an object name -- just do the normal search from here.
                            p = find_object(optionalScope, &const_cast<BasicScope*>(this)->breadcrumb_m);
                        }
                        else {
                            Breadcrumb* closest_scope{ nullptr };
                            if (optionalScope.length() == 0) {
                                p = find_object(optionalName, this->breadcrumb_m.root_m);
                            }
                            else if (auto nameSpace = find_namespace(optionalScope, closest_scope)) {
                                p = find_object(optionalName, nameSpace);
                            }
                            else if (closest_scope) { // namespace was not found                        
                                p = nullptr; //  throw GoodLang::exception::not_found_error(GoodLang::printf("Could not located object '%s' in namespace '%s'", optionalName.c_str().data(), closest_scope->GetCurrentNamespace().c_str().data()));
                            }
                            else { // namespace was not found AND no nearest was discovered     
                                p = nullptr; // throw GoodLang::exception::not_found_error(GoodLang::printf("Could not located object '%s'", optionalName.c_str().data()));
                            }
                        }
                        if (p) NS->search_cache.EmplaceCache<1>(NS->cache_version, PossiblyScopedName.hash(), reinterpret_cast<Breadcrumb*>(p));
                        return p;
                    }
                };
            };

            /// <summary>
            /// A special type of scope that serves as the "nodes" in the script tree, hosting functions and the caches. 
            /// Should not be created on its own, and instead should be issued by a parent.
            /// </summary>
            class NamespaceScope : public BasicScope {
                friend class BasicScope;
                friend class RootScope;
                friend class Breadcrumb;
            protected:
                // explicit children namespaces, with strongly-held protections to their memory.
                concurrency::concurrent_unordered_map<size_t, std::shared_ptr<NamespaceScope>>
                    children; // children cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option. 

            protected:
                Cache<4>
                    search_cache; // while thread-safe, it does seem to singificantly decrease the performance of creating new BasicScope's, hence moving it here. 

            protected:
                GL::callback<NamespaceScope>
                    sockets_for_cache_versions; // socket(s) for others to connect to for listening to changes to THIS scope. Thread-safe. 
                GL::callback<NamespaceScope>::ScopedListener
                    connection_for_cache_version; // socket connection for this scope to its parent, to listen to changes to THEIR scope. Not thread-safe.
                size_t
                    cache_version; // the current cache version of this scope. Thread-safe to read. Will be updated during every call to "invalidate_cache()"
            public:
                virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override {
                    InterlockedIncrement(static_cast<volatile size_t*>(&cache_version));
                    sockets_for_cache_versions.speak(parent_alive, call_number);
                };

            protected:
                virtual void AddUsing_Impl(Breadcrumb* scope) override {
                    if (scope) {
                        if (scope->this_m.is_namespace()) {
                            if (auto* p = dynamic_cast<NamespaceScope*>(scope->this_m.scope)) {
                                using_m.insert(std::pair<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>{ scope, p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this) });
                                invalidate_cache();
                            }
                        }
                    }
                };

            protected:
                // instancing a child namespace should only be done from an existing namespace
                NamespaceScope(GL::string&& name, int scope_type_p = ScopeType::Basic & ScopeType::Namespace, Breadcrumb* parent = nullptr)
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
                // unloads the connections to other namespaces before deletion, which can prevent a memory-access crash. 
                void unload() {
                    this->connection_for_cache_version = {};
                    for (auto& x : this->using_m) x.second = {};
                    for (auto& child : this->children) child.second->unload();
                    this->children.clear();
                };

            protected:
                virtual Breadcrumb* FindNearestScopeWhere(
                    std::function<int(Breadcrumb*, int)> const& func,
                    Breadcrumb* SecondaryPriortyScope = nullptr,
                    int searchState = 0,
                    check_cache& check_flags = GetCheckMap(),
                    int depth = 0
                ) const override {
                    auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);
                    Breadcrumb* finalResult = nullptr;
                    if (depth == 0) {
                        if (auto numTickets = GetRoot()->scope_indexs.num_tickets(); check_flags.size() < numTickets) {
                            check_flags.resize(numTickets);
                        }
                        for (auto& x : check_flags) x = CheckFlagState::none;
                    }

                    // Prevent Duplication
                    if (check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all) {
                        finalResult = nullptr;
                        return finalResult;
                    }
                    if (searchState & SkipChildren) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
                    }

                    // test myself directly	
                    if (!(check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::self)) {
                        check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::self;

                        auto res = func(&selfPtr, searchState);
                        if (res & SearchResult::Success) {
                            finalResult = &selfPtr;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
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
                            if (check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) { continue; }
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
                            if (flag & CheckFlagState::self) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            // check the using statements of the parent.
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if (flag2 & CheckFlagState::all) continue;
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
                            if (!(flag1 & CheckFlagState::self)) {
                                flag1 |= CheckFlagState::self;

                                auto res = func(thisParent, searchState);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    finalResult = nullptr;
                                    return finalResult;
                                }
                            }

                            // test my personal "using" namespaces completely
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                                    if (flag & CheckFlagState::all) { continue; }
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }
                            }
                        }
                        while (thisParent = thisParent->parent_m) {
                            auto& flag = check_flags[thisParent->GetScopeIndex()];
                            if (flag & CheckFlagState::self) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                                if (res & SearchResult::Success) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if (res & SearchResult::StaticFailure) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            // check the using statements of the parent.
                            if (thisParent->this_m.scope->using_m.size() > 0) {
                                for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                                    auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                                    if (flag2 & CheckFlagState::all) continue;
                                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                        return finalResult;
                                    }
                                }
                            }
                        }
                    }

                    // Test my children themselves. 
                    if (!RequestedSkipChildren && (!(searchState & SkipChildren)) && this->children.size() > 0ull) {
                        for (auto& child : this->children) {
                            auto* child_bc = &child.second->breadcrumb_m;
                            auto& flag = check_flags[child_bc->GetScopeIndex()];

                            if (flag & CheckFlagState::self) continue;

                            flag |= CheckFlagState::self;

                            auto res = func(child_bc, searchState | SearchingChildren | SkipChildren | SkipParent);
                            if (res & SearchResult::Success) {
                                finalResult = child_bc;
                                return finalResult;
                            }
                            else if (res & SearchResult::StaticFailure) {
                                flag |= CheckFlagState::all;
                            }
                        }
                    }

                    // Test my parent completely.
                    if (!(searchState & SkipParent)) {
                        if (selfPtr.parent_m) {
                            auto& flag = check_flags[selfPtr.parent_m->GetScopeIndex()];
                            if (!(flag & CheckFlagState::all)) {
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
                    if (!RequestedSkipChildren && (!(searchState & SkipChildren)) && this->children.size() > 0ull) {
                        for (auto& child : this->children) {
                            auto* child_bc = &child.second->breadcrumb_m;
                            auto& flag = check_flags[child_bc->GetScopeIndex()];

                            if (flag & CheckFlagState::all) continue;

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
                        return *children.insert(
                            { name.hash(), std::shared_ptr<NamespaceScope>(new NamespaceScope((GL::string)name, ScopeType::Basic | ScopeType::Namespace, const_cast<Breadcrumb*>(&this->breadcrumb_m))) }
                        ).first->second;
                    }
                };

            };

            /// <summary>
            /// The only scope that should be instanced on its own. 
            /// </summary>
            class RootScope : public NamespaceScope {
                friend class BasicScope;
                friend class NamespaceScope;
                friend class Breadcrumb;
            protected:
                // When a scope is born it will get the smallest-possible unique index for itself. 
                // This "ticket" or unique index will be unique to the scope for its life, after which it returns the ticket to here.
                GL::ticket_dispensor<false>
                    scope_indexs;
                GL::atomic_vector<Breadcrumb*>
                    scopes; // namespaces and classes may add themselves to this list (order not guarranteed) to help with debugging or other activities. 

            public:
                RootScope()
                    : NamespaceScope("::", ScopeType::Basic& ScopeType::Namespace& ScopeType::Root, nullptr)
                {};
                virtual ~RootScope() {
                    this->unload(); // must call the namespace's unload function BEFORE this destroys itself, otherwise connections are unable to resolve themselves. 
                };

            };




        };

    }
}