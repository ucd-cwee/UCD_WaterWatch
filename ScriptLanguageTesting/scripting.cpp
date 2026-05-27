#pragma once
#include "scripting.h"
// #include "datetime.h"
#include <queue>

namespace GL {
	namespace scope {
        bool impl::ScopeID::is_namespace() const {
            return scope_type & ScopeType::Namespace;
        };
        bool impl::ScopeID::is_class() const {
            return scope_type & ScopeType::Class;
        };
        bool impl::ScopeID::is_root() const {
            return scope_type & ScopeType::Root;
        };
        size_t impl::Breadcrumb::GetScopeIndex() {
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
        GL::string const& impl::Breadcrumb::GetCurrentNamespace() const {
            if (this->this_m.is_namespace()) {
                return this->this_m.current_namespace;
            }
            else {
                return this->namespace_m->this_m.current_namespace;
            }
        };

        GL::Proxy_Function const& impl::Functions::add_function(GL::Proxy_Function&& func) {
            return functions[func->m_signature.name_m].lock()->insert({ func->m_signature.get_hash(), std::move(func) }).first->second;
        };
        GL::Proxy_Function const& impl::Functions::add_function(GL::string name_m, GL::Proxy_Function&& func) {
            return functions[name_m].lock()->insert({ func->m_signature.get_hash(), std::move(func) }).first->second;
        };
        GL::Proxy_Function const& impl::Functions::add_function(GL::Proxy_Function&& func, std::remove_pointer_t<typename decltype(functions)::iterator::value_type::second_type>::shared_locked& locked) {
            return locked->insert({ func->m_signature.get_hash(), std::move(func) }).first->second;
        };
        void impl::Functions::UniformCostSearchNodeBestPath::get_impl(std::vector<GL::type>& out) const {
            if (previousBestPath) previousBestPath->get_impl(out);
            out.push_back(thisNodePath);
        };
        GL::Proxy_Function impl::Functions::UniformCostSearchNodeBestPath::make_converter(GL::type const& from, Functions const& srce) const {
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

                /*                           else if (auto& x = srce.try_find_callable(GL::string::empty_string(), &this_t, &this_t + 1, 0); x) {
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
                Proxy_Function temp;
                if (converters.size() == 2) {
                    temp = GL::make_callable("`operator " + (this_t - GL::type::Temporary).name() + "`", [converters](GL::any::fast_any& from) -> GL::any::fast_any {
                        return (*converters[1])->operator()((*converters[0])->operator()(from));
                        }, state | GL::function_signature::Explicit, {}, { { "From", from } }, this_t);
                }
                else if (converters.size() == 3) {
                    temp = GL::make_callable("`operator " + (this_t - GL::type::Temporary).name() + "`", [converters](GL::any::fast_any& from) -> GL::any::fast_any {
                        return (*converters[2])->operator()((*converters[1])->operator()((*converters[0])->operator()(from)));
                        }, state | GL::function_signature::Explicit, {}, { { "From", from } }, this_t);
                }
                else {
                    temp = GL::make_callable("`operator " + (this_t - GL::type::Temporary).name() + "`", [converters](GL::any::fast_any& from) -> GL::any::fast_any {
                        GL::any::fast_any out;
                        int pos{ 0 };
                        for (; pos < converters.size() && pos < 1; ++pos) out = (*converters[pos])->operator()(from);
                        for (; pos < converters.size(); ++pos) out = (*converters[pos])->operator()(out);
                        return out;
                        }, state | GL::function_signature::Explicit, {}, { { "From", from } }, this_t);
                }
                temp->m_signature.numConversions = 0;
                for (auto& x : converters) temp->m_signature.numConversions += (*x)->m_signature.numConversions;
                return temp;
            }
        };
        void impl::Functions::UniformCostSearchNodeBestPath::get(std::vector<GL::type>& out) const {
            out.clear();
            get_impl(out);
        };
        size_t impl::Functions::UniformCostSearchNodeBestPath::size() const {
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
        size_t impl::Functions::UniformCostSearchNode::size() const {
            if (bestPath) {
                return bestPath->size();
            }
            else {
                return 0;
            }
        };
        bool impl::Functions::UniformCostSearchNode::operator()(const UniformCostSearchNode* a, const UniformCostSearchNode* b) const {
            return ((a->size() + 1) > (b->size() + 1)) || (a->distanceFromTarget > b->distanceFromTarget);
        };
        bool impl::Functions::UniformCostSearchNode::operator()(const std::shared_ptr<UniformCostSearchNode>& a, const std::shared_ptr<UniformCostSearchNode>& b) const {
            return ((a->size() + 1) > (b->size() + 1)) || (a->distanceFromTarget > b->distanceFromTarget);
        };
        std::unordered_map<GL::type, impl::Functions::UniformCostSearchNode*> impl::Functions::CreateConversionPaths(
            GL::atomic_allocator<std::variant<UniformCostSearchNode, UniformCostSearchNodeBestPath>, 1024>& alloc,
            GL::type const& From
        ) {
            std::unordered_set<GL::type> AllTypes;
            std::map<GL::type, std::map<GL::type, GL::Proxy_Function const*>> AllConversions; // all built-in conversions, e.g. int->float, float->double, etc.
            (void)this->for_each([&AllConversions, &AllTypes](GL::Proxy_Function const& func)->bool {
                AllTypes.insert(func->m_signature.returns_m);
                for (auto& x : func->m_signature.argument_types_m) AllTypes.insert(x);

                if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                    if (func->m_signature.argument_types_m.size() == 1) {
                        if ((func->m_signature.state_m & GL::function_signature::Explicit) == 0) {
                            auto& from = func->m_signature.argument_types_m[0];
                            auto& to = func->m_signature.returns_m;
                            AllConversions[from][to] = &func;
                        }
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
                        auto temp_func = GL::make_callable("`dynamic_cast " + (base_type + GL::type::Reference).name() + "`", [base = base_type](GL::any::fast_any inherited) -> GL::any {
                            GL::any from = inherited;
                            from.m_casted_type = base | GL::type::Reference;
                            return from;
                            }, /*GL::function_signature::NoCost | */GL::function_signature::Async | GL::function_signature::Constant, {}, { { "from", Type | GL::type::Reference } }, base_type | GL::type::Reference);
                        auto first_arg_t = temp_func->m_signature.argument_types_m[0];
                        auto returns_t = temp_func->m_signature.returns_m;
                        auto hash_v = temp_func->m_signature.get_hash();
                        auto funcs = functions[temp_func->m_signature.name_m].lock_shared();
                        if (auto f = funcs->find(hash_v), e = funcs->end(); f == e) {
                            funcs.upgrade_lock();
                            AllConversions[first_arg_t][returns_t] = &this->add_function(std::move(temp_func), funcs);
                        }
                        else {
                            AllConversions[first_arg_t][returns_t] = &f->second;
                        }
                    }
                    if (0) {
                        auto temp_func = GL::make_callable("`dynamic_cast " + (base_type + GL::type::Reference + GL::type::Const).name() + "`", [base = base_type](GL::any::fast_any inherited) -> GL::any {
                            GL::any from = inherited;
                            from.m_casted_type = base | GL::type::Reference | GL::type::Const;
                            return from;
                            }, /*GL::function_signature::NoCost | */GL::function_signature::Async | GL::function_signature::Constant, {}, { { "from", Type } }, base_type | GL::type::Reference | GL::type::Const);
                        auto first_arg_t = temp_func->m_signature.argument_types_m[0];
                        auto returns_t = temp_func->m_signature.returns_m;
                        auto hash_v = temp_func->m_signature.get_hash();
                        auto funcs = functions[temp_func->m_signature.name_m].lock_shared();
                        if (auto f = funcs->find(hash_v), e = funcs->end(); f == e) {
                            funcs.upgrade_lock();
                            AllConversions[first_arg_t][returns_t] = &this->add_function(std::move(temp_func), funcs);
                        }
                        else {
                            AllConversions[first_arg_t][returns_t] = &f->second;
                        }
                    }
                    if (1) {
                        auto temp_func = GL::make_callable("`dynamic_cast " + (base_type + GL::type::Temporary).name() + "`", [base = base_type](GL::any::fast_any inherited) -> GL::any {
                            GL::any from = inherited;
                            from.m_casted_type = base | GL::type::Temporary;
                            return from;
                            }, /*GL::function_signature::NoCost |*/ GL::function_signature::Async | GL::function_signature::Constant, {}, { { "from", Type | GL::type::Temporary } }, base_type | GL::type::Temporary);
                        auto first_arg_t = temp_func->m_signature.argument_types_m[0];
                        auto returns_t = temp_func->m_signature.returns_m;
                        auto hash_v = temp_func->m_signature.get_hash();
                        auto funcs = functions[temp_func->m_signature.name_m].lock_shared();
                        if (auto f = funcs->find(hash_v), e = funcs->end(); f == e) {
                            funcs.upgrade_lock();
                            AllConversions[first_arg_t][returns_t] = &this->add_function(std::move(temp_func), funcs);
                        }
                        else {
                            AllConversions[first_arg_t][returns_t] = &f->second;
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
                                if (connection.second) {
                                    if (auto& func = *connection.second; func) {
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

            // Insert the "explicit" conversions, overriding any "implicit" ones. 
            (void)this->for_each([&](GL::Proxy_Function const& func)->bool {
                if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                    if (func->m_signature.argument_types_m.size() == 1) {
                        if ((func->m_signature.state_m & GL::function_signature::Explicit) > 0) {
                            if (From.can_free_cast(func->m_signature.argument_types_m[0])) {
                                auto& to = func->m_signature.returns_m;
                                vertices[to] = &std::get<UniformCostSearchNode>(*alloc.Alloc(UniformCostSearchNode{
                                    From,
                                    ((func->m_signature.state_m & GL::function_signature::NoCost) > 0) ? 0.01 : 1.0,
                                    &std::get<UniformCostSearchNodeBestPath>(*alloc.Alloc(UniformCostSearchNodeBestPath{ nullptr, to })) // From or to?
                                    }));
                            }
                        }
                    }
                }
                return false;
                });

            return vertices;
        };
        std::unordered_map<GL::type, GL::Proxy_Function> impl::Functions::CreateConversions(GL::type const& From) {
            std::unordered_map<GL::type, GL::Proxy_Function> out;

            GL::atomic_allocator<std::variant<GL::scope::impl::Functions::UniformCostSearchNode, GL::scope::impl::Functions::UniformCostSearchNodeBestPath>, 1024>
                temp_alloc;
            auto converters
                = CreateConversionPaths(temp_alloc, From);
            GL::type t;
            for (auto& To : converters)
                if (To.second && To.second->bestPath) {
                    if (auto p = To.second->bestPath->make_converter(From, *this)) {
                        t = p->m_signature.returns_m;
                        out[t] = std::move(p);
                    }
                }
            return out;
        };

        GL::fast_shared_ptr<GL::details::Proxy_Function_Base> impl::Converter::try_get_converter(GL::type const& from, GL::type const& to, int depth, bool in_function) {
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
                            if (auto f3 = f2->second.load_fast(); f3) {
                                return std::move(f3);
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
                                auto temp = GL::make_callable("`static_cast " + to.name() + "`", [To = to](GL::any::fast_any From) -> GL::any {
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
                                    auto temp = GL::make_callable("`forward_cast" + to.name() + "`", [To = to](GL::any::fast_any From) -> GL::any {
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
                            std::map<double, GL::Proxy_Function> options;
                            for (auto& potential_conversion : f1->second) {
                                if (auto func = potential_conversion.second.load(); func) {
                                    if ((func->m_signature.state_m & GL::function_signature::Cached) > 0) continue;

                                    if (func->m_signature.returns_m.can_free_cast(to, false)) {
                                        //double cost = 0; 
                                        //if (func->m_signature.returns_m.is_const() != to.is_const()) {
                                        //    cost += 0.25;
                                        //}
                                        //if (func->m_signature.returns_m.is_ref() != to.is_ref()) {
                                        //    cost += 0.25;
                                        //}
                                        //options[cost] = std::move(func);
                                        //continue;
                                        f1->second[to] = std::move(func);
                                        return f1->second[to].load_fast();
                                    }

                                    if (func->m_signature.returns_m.can_free_cast(to)) {
                                        if (options.count(0) == 0) {
                                            //double cost = 0;
                                            //if (func->m_signature.returns_m.is_const() != to.is_const()) {
                                            //    cost += 0.25;
                                            //}
                                            //if (func->m_signature.returns_m.is_ref() != to.is_ref()) {
                                            //    cost += 0.25;
                                            //}
                                            //options[cost] = std::move(func);
                                            //continue;

                                            options[0] = std::move(func);
                                        }
                                        continue;
                                        //f1->second[to] = std::move(func);
                                        //return f1->second[to].load();
                                    }

                                    if (func->m_signature.returns_m.can_cast(to)) {
                                        if (func->m_signature.returns_m.is_temp() && to.is_base()) {
                                            if (options.count(1) == 0) {
                                                options[1] = GL::make_callable("`call_and_cast " + func->m_signature.name_m + "`", [To = to, caster = func](GL::any::fast_any From) -> GL::any {
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
                                auto temp = GL::make_callable("`forward_cast " + to.name() + "`", [To = to, FromT = try_type, inF = in_function, this](GL::any::fast_any From) -> GL::any {
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
                                    f1->second[to] = GL::make_callable("`forward_cast " + to.name() + "`", [To = to, From1 = from, From2 = base_from_type, inF = in_function, this](GL::any::fast_any From) -> GL::any {
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
        bool impl::Converter::can_convert(GL::type const& from, GL::type const& to, bool in_function) {
            if (try_get_converter(from, to, 0, in_function)) { return true; }
            if (in_function && (to.is_base() || to.is_const_ref())) {
                if (try_get_converter(from, to | GL::type::Temporary, 0, true)) {
                    return true;
                }
            }
            return false;
        };
        GL::any::fast_any impl::Converter::call_with_conversions(const GL::details::Proxy_Function_Base* func, bool keep_return_value) {
            if (func) {
                return func->operator()();
            }
            return {};
        };
        GL::any::fast_any impl::Converter::call_with_conversions(const GL::details::Proxy_Function_Base* func, std::vector<any::fast_any>& params, bool keep_return_value) {
            return call_with_conversions(func, &params[0], &params[0] + params.size(), keep_return_value);
        };
        GL::any::fast_any impl::Converter::call_with_conversions(const GL::details::Proxy_Function_Base* func, const std::vector<any>& params, bool keep_return_value) {
            std::vector<any::fast_any> Params;
            Params.resize(params.size());
            std::transform(params.begin(), params.end(), Params.begin(), [](any const& from) { return from.fast(); });
            return call_with_conversions(func, &Params[0], &Params[0] + Params.size(), keep_return_value);
        };
        GL::any::fast_any impl::Converter::call_with_conversions(const GL::details::Proxy_Function_Base* func, any& param, bool keep_return_value) {
            any::fast_any p = param.fast();
            return call_with_conversions(func, &p, &p + 1, keep_return_value);
        };
        GL::any::fast_any impl::Converter::call_with_conversions(const GL::details::Proxy_Function_Base* func, any::fast_any& param, bool keep_return_value) {
            return call_with_conversions(func, &param, &param + 1, keep_return_value);
        };
        bool impl::Converter::can_call_with_conversions(const GL::details::Proxy_Function_Base* func) {
            if (func) {
                return true;
            }
            return false;
        };
        bool impl::Converter::can_call_with_conversions(const GL::details::Proxy_Function_Base* func, std::vector<any::fast_any>& params) {
            return can_call_with_conversions(func, params.begin(), params.end());
        };
        bool impl::Converter::can_call_with_conversions(const GL::details::Proxy_Function_Base* func, const std::vector<any>& params) {
            std::vector<any::fast_any> Params;
            Params.resize(params.size());
            std::transform(params.begin(), params.end(), Params.begin(), [](any const& from) { return from.fast(); });
            return can_call_with_conversions(func, Params.begin(), Params.end());
        };
        bool impl::Converter::can_call_with_conversions(const GL::details::Proxy_Function_Base* func, any& param) {
            any::fast_any p = param.fast();
            return can_call_with_conversions(func, &p, &p + 1);
        };
        bool impl::Converter::can_call_with_conversions(const GL::details::Proxy_Function_Base* func, any::fast_any& param) {
            return can_call_with_conversions(func, &param, &param + 1);
        };

        GL::any::fast_any* impl::BasicScope::GetObject_Impl(GL::string const& sv) {
            if (auto f = objects_m.find(sv), e = objects_m.end(); f != e) return &f->second;            
            return nullptr;
        };
        bool impl::BasicScope::AddUsing_Impl(Breadcrumb* scope) {
            if (scope) {
                if (scope->this_m.is_namespace()) {
                    if (auto f = using_m->find(scope); f == using_m->end()) {
                        using_m->insert(std::pair<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>{ scope, GL::callback<NamespaceScope>::ScopedListener() });
                        this->GetNamespace()->invalidate_cache();
                        return true;
                    }
                }
            }
            return false;
        };
        impl::BasicScope* impl::BasicScope::GetCurrentCaller() {
            if (scope_stack().size() > 0)
                return const_cast<BasicScope*>(scope_stack().back());
            else
                return nullptr;
        };
        impl::BasicScope::operator bool() const {
            return breadcrumb_m.this_m.scope;
        };
        bool impl::BasicScope::is_namespace() const {
            return this->breadcrumb_m.this_m.is_namespace();
        };
        bool impl::BasicScope::is_class() const {
            return this->breadcrumb_m.this_m.is_class();
        };
        bool impl::BasicScope::is_root() const {
            return this->breadcrumb_m.this_m.is_root();
        };
        impl::BasicScope* impl::BasicScope::GetParent() const {
            if (this->breadcrumb_m.parent_m) {
                return this->breadcrumb_m.parent_m->this_m.scope;
            }
            else {
                return nullptr;
            }
        };
        impl::NamespaceScope* impl::BasicScope::GetNamespace() const {
            if (this->breadcrumb_m.namespace_m) {
                return static_cast<NamespaceScope*>(this->breadcrumb_m.namespace_m->this_m.scope);
            }
            else {
                return nullptr;
            }
        };
        impl::RootScope* impl::BasicScope::GetRoot() const {
            if (this->breadcrumb_m.root_m) {
                return static_cast<RootScope*>(this->breadcrumb_m.root_m->this_m.scope);
            }
            else {
                return nullptr;
            }
        };
        impl::BasicScope::check_cache& impl::BasicScope::GetCheckMap() {
            static thread_local check_cache out;
            out.clear();
            return out;
        };
        impl::Breadcrumb* impl::BasicScope::FindNearestScopeWhere(
            std::function<int(Breadcrumb*, int)> const& func,
            Breadcrumb* SecondaryPriortyScope,
            int searchState,
            check_cache& check_flags,
            int depth
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
        impl::Breadcrumb* impl::BasicScope::FindNamespace(GL::string const& Name, Breadcrumb* start) {
            if (!start) return nullptr;
            if (Name.empty()) return start->root_m;

            NamespaceScope* NS = start->this_m.scope->GetNamespace();
            if (NS) {
                auto& current_cache = NS->namespace_search_cache.TryGetCache<0>(NS->cache_version);
                if (current_cache) {
                    if (auto f = current_cache->find(Name.hash()), e = current_cache->end(); f != e) {
                        return f->second;
                    }
                }
                else {
                    NS->namespace_search_cache.EmplaceCache<0>(NS->cache_version, GL::make_shared< concurrency::concurrent_unordered_map<size_t, Breadcrumb*> >());
                }
            }

            static size_t colon_hash{ GL::string(GL::string::namespace_colons()).hash() };
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

            if (target_hash.count(colon_hash) > 0) {
                return start->root_m;
            }
            else if (Breadcrumb* BC = start->this_m.scope->FindNearestScopeWhere([stringified = GL::string(Name)/*, &target_hash, &len*/](Breadcrumb* namespacePtr, int search_state)-> int {
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
                if (NS) {
                    if (auto& current_cache = NS->namespace_search_cache.TryGetCache<0>(NS->cache_version); current_cache) {
                        current_cache->insert({ Name.hash(), (Breadcrumb*)BC });
                    }
                }
                return BC;
            }
            else {
                if (NS) {
                    if (auto& current_cache = NS->namespace_search_cache.TryGetCache<0>(NS->cache_version); current_cache) {
                        current_cache->insert({ Name.hash(), (Breadcrumb*)nullptr });
                    }
                }
                return nullptr;
            }
        };
        GL::type impl::BasicScope::DetermineType(GL::string from) {
            from = from.remove_leading_and_trailing(' ');
            unsigned long state_modifiers = 0;
            if (from.begins_with("const ")) { from = from.right(from.size() - 6); state_modifiers |= GL::type::Const; }
            if (from.ends_with("&&")) { from = from.left(from.size() - 2); state_modifiers |= GL::type::Temporary; }
            if (from.ends_with("&")) { from = from.left(from.size() - 1); state_modifiers |= GL::type::Reference; }
            if (from.ends_with(" const")) { from = from.left(from.size() - 6); state_modifiers |= GL::type::Const; }

            if (auto [prefix, middle_and_postfix] = from.left_and_right_of("<"); !middle_and_postfix.empty()) {
                prefix = prefix.remove_leading_and_trailing(' ');
                middle_and_postfix = middle_and_postfix.remove_leading_and_trailing(' ');
                if (auto [middle, postfix] = middle_and_postfix.left_and_right_of_last(">"); (middle_and_postfix.find(">") != GL::string::npos) && !middle.empty()) {
                    middle = middle.remove_leading_and_trailing(' ');
                    postfix = postfix.remove_leading_and_trailing(' ');

                    GL::type original_template_type = DetermineType(prefix) | state_modifiers;
                    std::vector<std::pair<GL::string, GL::type>> inner_types;
                    bool templated_result = false;
                    if (middle.with_split_nested(",", "<", ">", [&](GL::string const& with_split, bool is_last) -> bool {
                        auto f = DetermineType(with_split);
                        if (f == GL::type_of<GL::undefined>()) {
                            return true;
                        }
                        if (f.is_template()) {
                            templated_result = true;
                        }
                        inner_types.push_back({ with_split, f });
                        return false;
                    })) {
                        // undefined type was returned, so we will also return undefined. 
                        return GL::type_of<GL::undefined>();
                    }
                    else {
                        //if (templated_result) {
                        //    // hit a brick wall and was not able to finish identification
                        //    return original_template_type | state_modifiers;
                        //}
                        if (auto* BC = this->GetRoot()->try_find_class(original_template_type); BC && BC->this_m.is_class()) {
                            if (dynamic_cast<ClassScope*>(BC->this_m.scope)->template_types.size() == inner_types.size()) {
                                for (int i = 0; i < dynamic_cast<ClassScope*>(BC->this_m.scope)->template_types.size(); i++) {
                                    inner_types[i].first = dynamic_cast<ClassScope*>(BC->this_m.scope)->template_types[i].first;
                                }

                                // must have the same number of template arguments. 
                                return dynamic_cast<ClassScope*>(BC->this_m.scope)->make_inherited_template_class(inner_types).this_type | state_modifiers;
                            }
                        }
                    }
                }
            }

            // check the "classes_by_name" multimap -- if there are no other classes with the same name, then we assume we are done
            if (auto f = this->GetRoot()->classes_by_name.find(from), e = this->GetRoot()->classes_by_name.end(), f2 = f; f != e) {
                ++f2;
                if ((f2 == e) || (f2->first != f->first)) {
                    return dynamic_cast<ClassScope*>(f->second->this_m.scope)->this_type | state_modifiers;
                }
            }

            if (auto* BC = this->find_namespace(from); BC && BC->this_m.is_class())
                return dynamic_cast<ClassScope*>(BC->this_m.scope)->this_type | state_modifiers;

            // check to see if it matches the template name for any of the template arguments from up above
            auto* this_p = this;
            while (this_p) {
                if (this_p->is_class()) {
                    for (auto& x : dynamic_cast<ClassScope*>(this_p)->template_types) {
                        if (x.first == from) {
                            return x.second;
                        }
                    }
                    this_p = this_p->GetParent();
                }
                else {
                    auto new_p = this_p->GetNamespace();
                    if (new_p == this_p) {
                        this_p = new_p->GetParent();
                    }
                    else {
                        this_p = new_p;
                    }
                }                
                if (this_p && this_p->is_root()) break;
            }

            GL::type out = GL::type_of<GL::undefined>();
            (void)this->FindNearestScopeWhere([&from, &out](Breadcrumb* BC, int state) -> int {
                if ((state & SearchState::SearchingUsings) != 0) return SearchResult::StaticFailure;
                if ((state & SearchState::SearchingChildren) != 0) return SearchResult::StaticFailure;
                if (BC->this_m.is_class()) {
                    for (auto& x : dynamic_cast<ClassScope*>(BC->this_m.scope)->template_types) {
                        if (x.first == from) {
                            //if (!x.second.is_template()) {
                                out = x.second;
                                return SearchResult::Success;
                            //}
                        }
                    }
                }
                return SearchResult::Failure;
            }, nullptr, SearchState::SkipChildren);
            return out;
        };
        std::pair<GL::string, const impl::Breadcrumb*> impl::BasicScope::ParsePossiblyScopedName(GL::string const& PossiblyScopedName) const {
            if (PossiblyScopedName.begins_with(GL::string::namespace_colons()))
                return this->GetRoot()->ParsePossiblyScopedName(PossiblyScopedName.remove_leading(':')); // this search needs to start from the root

            static thread_local int recursion_depth{ 0 };
            class recursion_depth_manager {
            private:
                int& r;

            public:
                recursion_depth_manager(int& R) : r(R) {
                    ++r;
                };
                ~recursion_depth_manager() {
                    --r;
                };
            };
            recursion_depth_manager manager(recursion_depth);
            if (recursion_depth > 16) return { PossiblyScopedName, nullptr };

            // "std::wrapper<std::string>::npos" -> ["std", "wrapper<std::string>", "npos"]
            const Breadcrumb* current_scope = &this->breadcrumb_m;
            bool do_nested_search = false;
            bool replace_spaces = false;
            bool has_diamonds = false;
            if ((PossiblyScopedName.find("<") != GL::string::npos) && (PossiblyScopedName.find(">") != GL::string::npos)) {
                has_diamonds = true;
                do_nested_search = true;
            }
            if (PossiblyScopedName.find(" ") != GL::string::npos) {
                replace_spaces = true;
                do_nested_search = true;
            }
            if (!do_nested_search 
                && PossiblyScopedName.find("::") != GL::string::npos) {
                do_nested_search = true;
            }
            if (do_nested_search) {
                GL::string accumulated;
                Breadcrumb* closest_scope;
                bool accumulating = false;
                if (replace_spaces) {
                    (void)PossiblyScopedName.replace(" ", GL::string::empty_string()).with_split_nested(GL::string::namespace_colons(), "<", ">", [&](GL::string const& this_split, bool is_final) -> bool {
                        if (accumulating) {
                            accumulated = accumulated + this_split;
                            return false;
                        }

                        if (!is_final) {
                            if (auto* found_scope = current_scope->this_m.scope->find_namespace(this_split, closest_scope); found_scope) {
                                current_scope = found_scope;
                            }
                            else {
                                // couldn't find this scope... see if we can "instance" it. 
                                if (this_split.find("<") != GL::string::npos) {
                                    if (auto new_type = current_scope->this_m.scope->DetermineType(this_split); new_type != GL::type_of<GL::undefined>()) {
                                        if (auto* BC = dynamic_cast<RootScope*>(current_scope->root_m->this_m.scope)->try_find_class(new_type); BC) {
                                            current_scope = BC;
                                        }
                                        else {
                                            // cannot continue the search?
                                            accumulating = true;
                                            accumulated = accumulated + this_split;
                                            return false;
                                        }
                                    }
                                    else {
                                        // cannot continue the search?
                                        accumulating = true;
                                        accumulated = accumulated + this_split;
                                        return false;
                                    }
                                }
                                else {
                                    // cannot continue the search?        
                                    accumulating = true;
                                    accumulated = accumulated + this_split;
                                    return false;
                                }
                            }
                        }
                        else {
                            accumulated = this_split;
                            return true;
                        }
                        return false;
                        });
                }
                else {
                    (void)PossiblyScopedName.with_split_nested(GL::string::namespace_colons(), "<", ">", [&](GL::string const& this_split, bool is_final) -> bool {
                        if (accumulating) {
                            accumulated = accumulated + this_split;
                            return false;
                        }

                        if (!is_final) {
                            if (auto* found_scope = current_scope->this_m.scope->find_namespace(this_split, closest_scope); found_scope) {
                                current_scope = found_scope;
                            }
                            else {
                                // couldn't find this scope... see if we can "instance" it. 
                                if (this_split.find("<") != GL::string::npos) {
                                    if (auto new_type = current_scope->this_m.scope->DetermineType(this_split); new_type != GL::type_of<GL::undefined>()) {
                                        if (auto* BC = dynamic_cast<RootScope*>(current_scope->root_m->this_m.scope)->try_find_class(new_type); BC) {
                                            current_scope = BC;
                                        }
                                        else {
                                            // cannot continue the search?
                                            accumulating = true;
                                            accumulated = accumulated + this_split;
                                            return false;
                                        }
                                    }
                                    else {
                                        // cannot continue the search?
                                        accumulating = true;
                                        accumulated = accumulated + this_split;
                                        return false;
                                    }
                                }
                                else {
                                    // cannot continue the search?        
                                    accumulating = true;
                                    accumulated = accumulated + this_split;
                                    return false;
                                }
                            }
                        }
                        else {
                            accumulated = this_split;
                            return true;
                        }
                        return false;
                        });
                }
                // try and offer name correction for typenames
                if (has_diamonds) {
                    if (auto new_type = current_scope->this_m.scope->DetermineType(accumulated); new_type != GL::type_of<GL::undefined>()) {
                        if (auto* BC = dynamic_cast<RootScope*>(current_scope->root_m->this_m.scope)->try_find_class(new_type); BC) {
                            return std::pair<GL::string, const Breadcrumb*>{ BC->this_m.scope_name, current_scope };
                        }
                    }
                }
                return std::pair<GL::string, const Breadcrumb*>{ accumulated, current_scope };
            }
            else {
                if (replace_spaces) {
                    if (has_diamonds) {
                        if (auto new_type = current_scope->this_m.scope->DetermineType(PossiblyScopedName.replace(" ", GL::string::empty_string())); new_type != GL::type_of<GL::undefined>()) {
                            if (auto* BC = dynamic_cast<RootScope*>(current_scope->root_m->this_m.scope)->try_find_class(new_type); BC) {
                                return std::pair<GL::string, const Breadcrumb*>{ BC->this_m.scope_name, current_scope };
                            }
                        }
                    }
                    return std::pair<GL::string, const Breadcrumb*>{ PossiblyScopedName.replace(" ", GL::string::empty_string()), current_scope };
                }
                else {
                    if (has_diamonds) {
                        if (auto new_type = current_scope->this_m.scope->DetermineType(PossiblyScopedName); new_type != GL::type_of<GL::undefined>()) {
                            if (auto* BC = dynamic_cast<RootScope*>(current_scope->root_m->this_m.scope)->try_find_class(new_type); BC) {
                                return std::pair<GL::string, const Breadcrumb*>{ BC->this_m.scope_name, current_scope };
                            }
                        }
                    }
                    return std::pair<GL::string, const Breadcrumb*>{ PossiblyScopedName, current_scope };
                }
            }
        };

        size_t impl::BasicScope::get_unique_index() const {
            return const_cast<BasicScope*>(this)->breadcrumb_m.GetScopeIndex();
        };        
        impl::BasicScope impl::BasicScope::make_scope() const {
            return BasicScope(GL::string::empty_string(), ScopeType::Basic, const_cast<Breadcrumb*>(&this->breadcrumb_m));
        };        
        bool impl::BasicScope::add_using_here(NamespaceScope const& ptr) {
            if (auto p = static_cast<const BasicScope*>(&ptr)) {
                if (this == p) return false; // may not "use" yourself.
                if (this->is_root()) return false; // the root may not call the using statement
                if (p->is_root()) return false; // the root may not be "used"
                return this->AddUsing_Impl(const_cast<Breadcrumb*>(&p->breadcrumb_m));
            }
            return false;
        }        
        bool impl::BasicScope::insert_object_here(GL::string const& sv, GL::any::fast_any&& Obj) {
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
        bool impl::BasicScope::emplace_object_here(GL::string const& sv, GL::any::fast_any&& Obj) {
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
        GL::any::fast_any* impl::BasicScope::find_object_here(GL::string const& sv) const {
            return const_cast<BasicScope*>(this)->GetObject_Impl(sv);
        };
        impl::Breadcrumb* impl::BasicScope::find_namespace(GL::string const& Name) const {
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
        impl::Breadcrumb* impl::BasicScope::FindNamespaceImpl(GL::string const& Name, Breadcrumb*& nearest_scope) const {
            if ((Name.length() > 0) && (Name.front() == ':')) {
                return this->GetRoot()->FindNamespaceImpl(Name.remove_leading(':'), nearest_scope);
            }

            if (auto* out = find_namespace(Name)) {
                nearest_scope = out;
                return out;
            }
            else {
                nearest_scope = this->breadcrumb_m.namespace_m;
                return nullptr;

                if (!nearest_scope) nearest_scope = this->breadcrumb_m.root_m;

                //auto [remainder, BC] = this->ParsePossiblyScopedName(Name);
                //if (remainder.empty()) return const_cast<Breadcrumb*>(BC);
                //else {
                //    nearest_scope = const_cast<Breadcrumb*>(BC);
                //    return BC->this_m.scope->FindNamespaceImpl(remainder, nearest_scope);
                //}
            }
        };
        impl::Breadcrumb* impl::BasicScope::find_namespace(GL::string const& Name, Breadcrumb*& nearest_scope) const {
            Breadcrumb* out = FindNamespaceImpl(Name, nearest_scope);
            if (out) {
                return out->this_m.scope->find_namespace(Name);
            }
            return nullptr;
        };
        std::pair<GL::any::fast_any, bool> impl::BasicScope::try_find_object(GL::string const& PossiblyScopedName, Breadcrumb* search_from) const {
            GL::any::fast_any
                * p = nullptr;
            if (search_from) {
                if (p = search_from->this_m.scope->find_object_here(PossiblyScopedName); p) {
                    return { p->fast(), true };
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
                    return { p->fast(), true };
                }
                else {
                    //NS->search_cache.EmplaceCache<1>(NS->cache_version, PossiblyScopedName.hash(), reinterpret_cast<Breadcrumb*>(1));
                }
            }
            else {
                auto [remainder, BC] = this->ParsePossiblyScopedName(PossiblyScopedName);
                if (!remainder.empty()) {
                    return BC->this_m.scope->try_find_object(remainder, const_cast<Breadcrumb*>(BC));
                }
            }

            return { nullptr, false };
        };
        GL::any::fast_any impl::BasicScope::find_object(GL::string const& PossiblyScopedName, Breadcrumb* search_from) const {
            if (auto [p, success] = try_find_object(PossiblyScopedName, search_from); success) return p;
            auto err = GL::printf("Could not locate object '%s'", PossiblyScopedName.c_str().data()).to_string();
            throw std::runtime_error(err);
        };
        GL::any::fast_any impl::BasicScope::call(GL::string const& PossiblyScopedName, std::vector<GL::any::fast_any> const& params, bool keep_return_value) const {
            return call_impl(PossiblyScopedName, const_cast<GL::any::fast_any*>(&params[0]), const_cast<GL::any::fast_any*>(&params[0] + params.size()), keep_return_value);
        };
        GL::any::fast_any impl::BasicScope::call(const GL::details::Proxy_Function_Base* func, std::vector<GL::any::fast_any> const& params, bool keep_return_value) const {
            auto handler = push_back_caller(this);
            return this->GetRoot()->get_converters().call_with_conversions(func, const_cast<GL::any::fast_any*>(&params[0]), const_cast<GL::any::fast_any*>(&params[0] + params.size()), keep_return_value);
        };

        void impl::NamespaceScope::invalidate_cache(long* parent_alive, size_t call_number) {
            InterlockedIncrement(static_cast<volatile size_t*>(&cache_version));
            sockets_for_cache_versions.speak(parent_alive, call_number);
        };
        bool impl::NamespaceScope::AddUsing_Impl(Breadcrumb* scope) {
            if (scope) {
                if (scope->this_m.is_namespace()) {
                    if (auto* p = dynamic_cast<NamespaceScope*>(scope->this_m.scope)) {
                        using_m->insert(std::pair<Breadcrumb*, GL::callback<NamespaceScope>::ScopedListener>{ scope, p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this) });
                        invalidate_cache();
                        return true;
                    }
                }
            }
            return false;
        };
        GL::string impl::NamespaceScope::path() const {
            return this->breadcrumb_m.GetCurrentNamespace();
        };
        impl::NamespaceScope& impl::NamespaceScope::make_namespace(GL::string const& name) {
            if (this->breadcrumb_m.this_m.is_class()) {
                throw std::runtime_error("It is invalid to declare a namespace within a class. Classes may only declare further classes.");
            }
            if (auto* f = children.try_at(name.hash()); f != nullptr) {
                return **f;
            }
            if (1) {
                this->invalidate_cache();
                while (true) {
                    return *children.insert(name.hash(), std::shared_ptr<NamespaceScope>(new NamespaceScope((GL::string)name, ScopeType::Basic | ScopeType::Namespace, const_cast<Breadcrumb*>(&this->breadcrumb_m)))).second;
                }
            }
        };
        impl::ClassScope& impl::NamespaceScope::make_class(GL::type class_type) {
            if (!(class_type & GL::type::CppType)) {
                GL::string error = "Classes defined with GL::type must be C++ built-in types. Was provided `" + class_type.name() + "` instead, which was not identified was a C++ type.";
                throw std::runtime_error(error.to_string());
            }

            class_type -= GL::type::Const;
            class_type -= GL::type::Reference;
            class_type -= GL::type::Temporary;

            if (auto* f = children.try_at(class_type.name().hash()); f != nullptr && f->operator->()->is_class()) {
                return *dynamic_cast<ClassScope*>((*f).get());
            }
            if (1) {
                this->GetRoot()->invalidate_cache();
                ++this->GetRoot()->constructors_version;
                while (true) {
                    children.insert(class_type.name().hash(), std::dynamic_pointer_cast<NamespaceScope>(std::shared_ptr<ClassScope>(new ClassScope(class_type, ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, const_cast<Breadcrumb*>(&this->breadcrumb_m)))));
                    if (auto* f = children.try_at(class_type.name().hash()); (f != nullptr) && (f->operator->()->is_class())) {
                        auto new_class = std::dynamic_pointer_cast<ClassScope>(*f);
                        this->GetRoot()->classes.insert({ class_type.get_base_hash(), &new_class->breadcrumb_m });
                        this->GetRoot()->classes_by_name.insert({ class_type.name(), &new_class->breadcrumb_m });
                        return *new_class;
                    }
                }
            }
        };
        impl::ClassScope& impl::NamespaceScope::make_class(GL::string const& class_type) {
            if (this->is_class()) {
                if (dynamic_cast<ClassScope*>(this)->template_types.size() > 0) {
                    throw std::runtime_error("It is invalid to declare a class within a templated class<...>. Templated classes must not declare any further classes or namespaces.");
                }
            }


            if (auto* f = children.try_at(class_type.hash()); (f != nullptr) && (f->operator->()->is_class())) {
                return *dynamic_cast<ClassScope*>(f->get());
            }
            if (1) {
                this->GetRoot()->invalidate_cache();
                ++this->GetRoot()->constructors_version;
                while (true) {
                    children.insert(
                        class_type.hash(), std::dynamic_pointer_cast<NamespaceScope>(std::shared_ptr<ClassScope>(new ClassScope(class_type, ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, const_cast<Breadcrumb*>(&this->breadcrumb_m))))
                    );
                    if (auto* f = children.try_at(class_type.hash()); (f != nullptr) && (f->operator->()->is_class())) {
                        auto* new_class = dynamic_cast<ClassScope*>(f->get());
                        this->GetRoot()->classes.insert({ new_class->this_type.get_base_hash(), &new_class->breadcrumb_m });
                        this->GetRoot()->classes_by_name.insert({ class_type, &new_class->breadcrumb_m });
                        return *new_class;
                    }
                }
            }
        };
        GL::Proxy_Function const& impl::NamespaceScope::add_function(GL::Proxy_Function&& func) {
            GL::Proxy_Function copy = func;
            auto& out = functions.add_function(std::move(copy));
            if ((func->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                this->GetRoot()->add_constructor(&this->breadcrumb_m, std::move(func));
            }
            this->invalidate_cache();
            return out;
        }; 

        void impl::ClassScope::default_construct(GL::dynamic_object& destination) {
            for (auto& member_object : this->member_objects) {
                if (member_object.second.second && // default provided...
                    member_object.second.first.is_ref() && // ... and the user is requesting a reference...
                    member_object.second.second.can_cast(member_object.second.first) // ... and the correct type was provided for use as a reference.
                    ) {
                    destination.m_objects.insert({ member_object.first, GL::make_shared<GL::any>(member_object.second.second) });
                }
                else if (auto* BC = this->GetRoot()->try_find_class(member_object.second.first); BC && BC->this_m.is_class()) {
                    auto* Class = dynamic_cast<ClassScope*>(BC->this_m.scope);
                    if (member_object.second.second) { // default provided
                        destination.m_objects.insert({ member_object.first, GL::make_shared<GL::any>(Class->call(Class->breadcrumb_m.this_m.scope_name, { member_object.second.second })) });
                    }
                    else { // no default. Attempt to construct the member object. 
                        destination.m_objects.insert({ member_object.first, GL::make_shared<GL::any>(Class->call(Class->breadcrumb_m.this_m.scope_name, {})) });
                    }
                }
                else {
                    // not a reference, and class was not found. We have no choice but to accept the provided data as-is. 
                    GL::any f = member_object.second.second;
                    f.m_casted_type = member_object.second.first;
                    destination.m_objects.insert({ member_object.first, GL::make_shared<GL::any>((f | GL::type::Reference)) });
                }
            }
        };
        void impl::ClassScope::add_member_object(GL::string const& member_name, GL::type const& member_type, GL::any::fast_any const& default_value) {
            member_objects.insert({ member_name, { member_type, default_value } });
        };
        void impl::ClassScope::initialize_basic_member_functions() {
            // default constructor
            this->add_function(GL::make_callable(breadcrumb_m.this_m.scope_name, [this]() -> GL::any::fast_any {
                auto temp = GL::dynamic_object(this->this_type);
                for (auto& base_type : this->this_type.all_base_types(false)) {
                    if (auto* BC = this->GetRoot()->try_find_class(base_type); BC) {
                        dynamic_cast<ClassScope*>(BC->this_m.scope)->default_construct(temp);
                    }
                }
                auto out = GL::any::fast_any::instance(std::move(temp));
                const_cast<GL::type&>(out.get_actual_type()) = this->this_type;
                out.m_casted_type = this->this_type;
                return out;
            }, GL::function_signature::Constructor | GL::function_signature::Static | GL::function_signature::Async, std::vector<any>{}, std::vector<std::pair<GL::string, GL::type>>{}, this->this_type));

            // copy constructor
            this->add_function(GL::make_callable(breadcrumb_m.this_m.scope_name, [this](GL::any::fast_any rhs) -> GL::any::fast_any {
                auto temp = GL::dynamic_object(this->this_type);
                for (auto& base_type : this->this_type.all_base_types(false)) {
                    if (auto* BC = this->GetRoot()->try_find_class(base_type); BC) {
                        dynamic_cast<ClassScope*>(BC->this_m.scope)->default_construct(temp);
                    }
                }

                for (auto& x : rhs.cast<GL::dynamic_object>().m_objects) {
                    if (auto f = temp.m_objects.find(x.first), e = temp.m_objects.end(); f != e) {
                        this->call("=", { f->second->fast(), x.second->fast() });
                        /*if (auto F = this->try_find_callable("=", { f->second->m_casted_type, x.second->m_casted_type })) {
                            F->operator()({ f->second->fast(), x.second->fast() });
                        }
                        else {
                            *f->second = x.second->fast();
                        }*/
                    }
                }

                auto out = GL::any::fast_any::instance(std::move(temp));
                out.m_casted_type = this->this_type;
                const_cast<GL::type&>(out.get_actual_type()) = this->this_type;
                return out;
                }, GL::function_signature::Constructor, {}, { { "rhs", this->this_type | GL::type::Const | GL::type::Reference } }, this->this_type));

            // assignment operator
            this->GetRoot()->add_function(GL::make_callable("=", [this](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                auto& LHS = lhs.cast<GL::dynamic_object>();
                auto& RHS = rhs.cast<GL::dynamic_object>();

                for (auto& x : RHS.m_objects) {
                    if (auto f = LHS.m_objects.find(x.first), e = LHS.m_objects.end(); f != e) {
                        this->call("=", { f->second->fast(), x.second->fast() });
                        /*if (auto F = this->try_find_callable("=", { f->second->m_casted_type, x.second->m_casted_type })) {
                            F->operator()({ f->second->fast(), x.second->fast() });
                        }
                        else {
                            *f->second = x.second->fast();
                        }*/
                    }
                }

                GL::any::fast_any out = lhs;
                out.m_casted_type |= GL::type::Reference;
                return out;
                }, 0, {}, { { "lhs", this->this_type | GL::type::Reference } , { "rhs", this->this_type | GL::type::Const | GL::type::Reference } }, this->this_type | GL::type::Reference));

            // member access functions. Note that it does not include member access functions for base classes -- that is assumed to be picked up automatically through polymorphic casting. 
            for (auto& member_object : member_objects) {
                if (!member_object.first.begins_with("~")) {
                    // non-const reference access to member objects
                    auto& ref_access = this->add_function(GL::make_callable(member_object.first, [member_name = GL::string(member_object.first), expected_type = member_object.second.first](GL::any::fast_any rhs)->GL::any::fast_any {
                        return GL::dynamic_object::object_access(member_name, rhs)->fast() + GL::type::Reference;
                        // return GL::any::fast_any(GL::dynamic_object::object_access(member_name, rhs), expected_type | GL::type::Reference);
                    }, GL::function_signature::MemberObject | GL::function_signature::Async, {}, { { "rhs", this->this_type | GL::type::Reference } }, member_object.second.first | GL::type::Reference));
                    // const reference access to member objects
                    auto& const_ref_access = this->add_function(GL::make_callable(member_object.first, [member_name = GL::string(member_object.first), expected_type = member_object.second.first](GL::any::fast_any rhs)->GL::any::fast_any {
                        return GL::dynamic_object::object_access(member_name, rhs)->fast() + GL::type::Const + GL::type::Reference;

                        // return GL::any::fast_any(GL::dynamic_object::object_access(member_name, rhs), expected_type | GL::type::Reference | GL::type::Const);
                    }, GL::function_signature::MemberObject | GL::function_signature::Constant | GL::function_signature::Async, {}, { { "rhs", this->this_type | GL::type::Reference | GL::type::Const } }, member_object.second.first | GL::type::Reference | GL::type::Const));

                    // std::cout << breadcrumb_m.this_m.scope_name << " : " << member_object.first << " : " << ref_access->m_signature.display() << std::endl;
                }
            }

            // to_string and to_hash functions
            this->add_function(GL::make_callable("to_string", [this](GL::any::fast_any lhs) -> GL::string {
                auto& o = lhs.cast<GL::dynamic_object>();
                GL::any::fast_any out = GL::any::fast_any::instance(GL::string());
                for (auto& obj : o.m_objects) {
                    if (obj.second) {
                        if (!obj.first.begins_with("~")) {
                            if (!obj.first.begins_with("!")) {
                                auto this_pair = GetCurrentCaller()->call("+", { GL::any::fast_any::instance(GL::string(obj.first + ":")), GetCurrentCaller()->call("to_string", {
                                    obj.second->fast() + GL::type::Const + GL::type::Reference
                                }) });
                                out = GetCurrentCaller()->call("add_to_delim", { out, this_pair, GL::any::fast_any::instance(GL::string(", ")) });
                            }
                        }
                        else {
                            return GetCurrentCaller()->call<GL::string>("to_string", { obj.second->fast() + GL::type::Const + GL::type::Reference });
                        }
                    }
                }
                return "[" + this->GetRoot()->call<GL::string>("::string", { out }) + "]";
                }, GL::function_signature::Constant, {}, { { "rhs", this->this_type | GL::type::Const | GL::type::Reference } }, GL::type_of<GL::string>()));
            this->add_function(GL::make_callable("to_hash", [this](GL::any::fast_any lhs) -> size_t {
                auto& o = lhs.cast<GL::dynamic_object>();
                size_t out = 0;
                for (auto& obj : o.m_objects) {
                    if (obj.second) {
                        if (!obj.first.begins_with("~")) {
                            if (!obj.first.begins_with("!")) {
                                GL::util::hash(out, obj.first.hash());
                                GL::util::hash(out, GetCurrentCaller()->call("to_hash", {
                                    obj.second->fast() + GL::type::Const + GL::type::Reference
                                }).cast<size_t>());
                            }
                        }
                        else {
                            return GetCurrentCaller()->call("to_hash", {
                                obj.second->fast() + GL::type::Const + GL::type::Reference
                            }).cast<size_t>();
                        }
                    }
                }
                return out;
                }, GL::function_signature::Constant, {}, { { "rhs", this->this_type | GL::type::Const | GL::type::Reference } }, GL::type_of<size_t>()));
        };
        static std::pair<GL::type, bool> TryFinalizeType(GL::type const& candidtate, GL::scope::impl::ClassScope* NewClass) {
            // Prevent infinite looping with `make_inherited_template_class`
            static thread_local std::deque<GL::type> recent_attempts;
            class push_pop {
            public:
                push_pop(GL::type const& rhs) {
                    recent_attempts.push_back(rhs);
                };
                ~push_pop() {
                    recent_attempts.pop_back();
                };
            };
            for (auto& recent : recent_attempts) if (recent == candidtate) return { candidtate, false };           
            push_pop pusher_popper(candidtate);
            
            if (GL::is_template::index(candidtate) >= 0) {                
                // this is a template type. seek to discover the type from the parent types / classes
                GL::type out = candidtate;
                bool result = NewClass->FindNearestScopeWhere([&](GL::scope::impl::Breadcrumb* BC, int state) -> int {
                    if (!BC->this_m.is_class()) return GL::scope::impl::BasicScope::SearchResult::Failure;
                    auto& Class = *dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope);
                    auto& potential_templates = Class.template_types;
                    for (int i = 0; i < potential_templates.size(); ++i) {
                        if (!potential_templates[i].second.is_template()) { // ...do not want to replace with another template type...
                            if (GL::is_template::type(i, potential_templates[i].first).get_base_hash() == candidtate.get_base_hash()) { // this would have been a good match.
                                out = potential_templates[i].second;
                                return GL::scope::impl::BasicScope::SearchResult::Success;
                            }
                        }
                    }
                    return GL::scope::impl::BasicScope::SearchResult::Failure;
                }, nullptr, GL::scope::impl::BasicScope::SearchState::SkipChildren);
                return { out, result };
            }
            else {
                // std::cout << candidtate.name() << std::endl;



                if (auto* Class_p = NewClass->GetRoot()->try_find_class(candidtate)) {
                    auto& Class = *dynamic_cast<GL::scope::impl::ClassScope*>(Class_p->this_m.scope);
                    std::vector<std::pair<GL::string, GL::type>> corrected_template_params;
                    bool need_to_recalc = false;
                    for (auto& template_t : Class.template_types) {
                        if (auto [newT, successful] = TryFinalizeType(template_t.second, NewClass); successful) {
                            need_to_recalc = true;
                            corrected_template_params.push_back({ template_t.first, newT });
                        }
                        else {
                            corrected_template_params.push_back({ template_t.first, template_t.second });
                        }
                    }
                    if (need_to_recalc) {
                        if (Class.this_type.name().find("<") == GL::string::npos) {                            
                            auto& correctedClass = Class.make_inherited_template_class(corrected_template_params);
                            return { correctedClass.this_type, true };
                        }
                        else {
                            if (auto* BC = Class.GetRoot()->try_find_class(Class.DetermineType(Class.this_type.name().left_of("<")))) {
                                auto& correctedClass = dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope)->make_inherited_template_class(corrected_template_params);
                                return { correctedClass.this_type, true };
                            }
                            else {
                                auto& correctedClass = Class.make_inherited_template_class(corrected_template_params);
                                return { correctedClass.this_type, true };
                            }
                        }
                    }
                }
            }
            return { candidtate, false };
        };
        impl::ClassScope& impl::ClassScope::make_inherited_template_class(std::vector< std::pair<GL::string, GL::type> > const& templates) {
            GL::string name; for (auto& x : templates) {
                if (auto* BC = this->GetRoot()->try_find_class(x.second); BC && BC->this_m.is_class()) {
                    GL::string out = dynamic_cast<ClassScope*>(BC->this_m.scope)->path().remove_leading_and_trailing(':');
                    if (x.second.is_const()) out = "const " + out;
                    if (x.second.is_ref()) out = out + "&";
                    if (x.second.is_temp()) out = out + "&&";
                    name = name.add_to_delim(out, ",");
                }
                else {
                    name = name.add_to_delim(x.second.name(), ",");
                }
            }
            if (!name.empty()) {
                name = this->this_type.name() + "<" + name + ">"; // vector<int>
            }
            else {
                name = this->this_type.name();
            }

            auto _parent = dynamic_cast<NamespaceScope*>(this->GetParent());
            if (auto f = _parent->children.try_at(name.hash()); f != nullptr) {
                return *dynamic_cast<ClassScope*>(f->get());
            }
            if (1) {
                this->GetRoot()->invalidate_cache();
                ++this->GetRoot()->constructors_version;
                while (true) {
                    if (auto new_class = std::make_shared< ClassScope>(name, ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, const_cast<Breadcrumb*>(&_parent->breadcrumb_m))) {
                        new_class->template_types = templates;
                        const_cast<GL::type&>(new_class->this_type).add_base(this->this_type);

                        _parent->children.insert(
                            name.hash(), std::dynamic_pointer_cast<NamespaceScope>(new_class)
                        );
                        if (auto f = _parent->children.try_at(name.hash()); f != nullptr) {
                            auto* newclass = dynamic_cast<ClassScope*>(f->get());
                            this->GetRoot()->classes.insert({ newclass->this_type.get_base_hash(), &newclass->breadcrumb_m });
                            this->GetRoot()->classes_by_name.insert({ name, &newclass->breadcrumb_m });
                        }

                        this->for_each_function([&](GL::Proxy_Function const& f)->bool {
                            if ((f->m_signature.state_m & GL::function_signature::Constructor) > 0) {
                                // assume that constructors will be unique to this invocation
                                return false;
                            }
                            else if ((f->m_signature.state_m & GL::function_signature::MemberObject) > 0) {
                                // assume that member objects will be either inherited, or converted below.
                                return false;
                            }

                            auto new_f = f->duplicate();

                            bool necessary = false;
                            if (new_f->m_signature.returns_m.get_base_hash() == this->this_type.get_base_hash()) {
                                new_f->m_signature.returns_m = new_class->this_type + (new_f->m_signature.returns_m - GL::type::CppType - GL::type::TemplateType).get_qualifiers();
                                necessary = true;
                            }
                            if (new_f->m_signature.returns_m.is_template()) {
                                auto& replace_me = new_f->m_signature.returns_m;
                                if (auto [newT, successful] = TryFinalizeType(replace_me, new_class.get()); successful) {
                                    replace_me = newT + (replace_me - GL::type::CppType - GL::type::TemplateType).get_qualifiers();
                                    necessary = true;
                                }
                            }
                            for (auto& x : new_f->m_signature.argument_types_m) {
                                auto& replace_me = x;
                                if (replace_me.get_base_hash() == this->this_type.get_base_hash()) {
                                    replace_me = new_class->this_type + (replace_me - GL::type::CppType - GL::type::TemplateType).get_qualifiers();
                                    necessary = true;
                                }
                                else if (auto [newT, successful] = TryFinalizeType(replace_me, new_class.get()); successful) {
                                    replace_me = newT + (replace_me - GL::type::CppType - GL::type::TemplateType).get_qualifiers();
                                    necessary = true;
                                }
                            }
                            if (necessary) {
                                for (auto& x : new_f->m_signature.argument_types_m) {
                                    if (x.get_base_hash() == this->this_type.get_base_hash()) {
                                        x = new_class->this_type + (x - GL::type::CppType - GL::type::TemplateType).get_qualifiers();
                                        necessary = true;
                                    }
                                }
                            }
                            if (necessary) {
                                new_f->m_signature.evaluate_if_template_function();
                                new_f->m_signature.reevaluate_hash();
                                new_class->add_function(std::move(new_f));
                            }

                            return false;
                        });
                        for (auto& member_o : this->member_objects) {
                            auto& replace_me = member_o.second.first; 
                            if (auto [newT, successful] = TryFinalizeType(replace_me, new_class.get()); successful) {
                                if (member_o.second.first != GL::type_of<GL::undefined>()) {
                                    new_class->add_member_object(
                                        member_o.first,
                                        newT + (replace_me - GL::type::CppType - GL::type::TemplateType).get_qualifiers(),
                                        member_o.second.second
                                    );
                                }
                                else {
                                    new_class->add_member_object(
                                        member_o.first,
                                        newT + (replace_me - GL::type::CppType - GL::type::TemplateType).get_qualifiers()
                                    );
                                }
                            }
                        }
                        if (1) {
                            auto& objects = this->objects_m;
                            for (auto& obj : objects) {
                                auto& replace_me = obj.second.m_casted_type;
                                if (auto [newT, successful] = TryFinalizeType(replace_me, new_class.get()); successful) {
                                    auto this_static_obj_type = newT + (obj.second.m_casted_type - GL::type::CppType - GL::type::TemplateType).get_qualifiers();
                                    if (auto* BCP = this->GetRoot()->try_find_class(this_static_obj_type); BCP) {
                                        new_class->insert_object_here(obj.first, BCP->this_m.scope->call(this_static_obj_type.name(), {}));
                                    }
                                    else {
                                        new_class->insert_object_here(obj.first, this->call(this_static_obj_type.name(), {}));
                                    }
                                }
                            }
                        }

                        //for (auto& child_scope : this->children) {
                        //    if (child_scope.second->is_class()) {
                        //        ClassScope& Class = *dynamic_cast<ClassScope*>(child_scope.second.get());
                        //        Class.make_inherited_template_class(templates);
                        //    }
                        //}

                        new_class->initialize_basic_member_functions();
                        return *new_class;
                    }
                    if (auto f = _parent->children.try_at(name.hash()); f != nullptr) {
                        auto* newclass = dynamic_cast<ClassScope*>(f->get());
                        this->GetRoot()->classes.insert({ newclass->this_type.get_base_hash(), &newclass->breadcrumb_m });
                        this->GetRoot()->classes_by_name.insert({ name, &newclass->breadcrumb_m });
                        newclass->initialize_basic_member_functions();
                        return *newclass;
                    }
                }
            }
        };
        impl::Breadcrumb* impl::ClassScope::FindNearestScopeWhere(
            std::function<int(Breadcrumb*, int)> const& func,
            Breadcrumb* SecondaryPriortyScope,
            int searchState,
            check_cache& check_flags,
            int depth
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

            // test my "base" classes directly -- hoping to quickly find "it"
            if (!(searchState & SkipParent)) {
                if (selfPtr.this_m.is_class()) {
                    auto* p = reinterpret_cast<ClassScope*>(selfPtr.this_m.scope);
                    for (auto& this_t : p->this_type.all_base_types(false)) {
                        auto* thisParent = this->GetRoot()->try_find_class(this_t);
                        while (thisParent) {
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
                            thisParent = thisParent->parent_m;
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
                    auto* child_bc = &(*child.second)->breadcrumb_m;
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

            // test my "base" classes directly -- hoping to quickly find "it"
            if (!(searchState & SkipParent)) {
                if (selfPtr.this_m.is_class()) {
                    auto* p = reinterpret_cast<ClassScope*>(selfPtr.this_m.scope);
                    for (auto& this_t : p->this_type.all_base_types(false)) {
                        auto* thisParent = this->GetRoot()->try_find_class(this_t);
                        while (thisParent) {
                            auto& flag = check_flags[thisParent->GetScopeIndex()];
                            if ((flag & CheckFlagState::self) > 0) break;
                            else {
                                flag |= CheckFlagState::self;
                            }
                            if (thisParent->this_m.is_namespace()) {
                                auto res = func(thisParent, searchState | SearchingParents | SearchUpHitNamespace);
                                if ((res & SearchResult::Success) > 0) {
                                    finalResult = thisParent;
                                    return finalResult;
                                }
                                else if ((res & SearchResult::StaticFailure) > 0) {
                                    flag |= CheckFlagState::all;
                                }
                            }
                            else {
                                auto res = func(thisParent, searchState | SearchingParents);
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

                            thisParent = thisParent->parent_m;
                        }
                    }
                }
            }

            // Test my children completely. 
            if (!RequestedSkipChildren && ((searchState & SkipChildren) == 0) && this->children.size() > 0ull) {
                for (auto& child : this->children) {
                    auto* child_bc = &(*child.second)->breadcrumb_m;
                    auto& flag = check_flags[child_bc->GetScopeIndex()];

                    if ((flag & CheckFlagState::all) > 0) continue;

                    if (finalResult = child_bc->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingChildren | SkipParent, check_flags, depth + 1)) {
                        return finalResult;
                    }
                }
            }

            return finalResult;
        };

        void impl::RootScope::invalidate_cache(long* parent_alive, size_t call_number) {
            InterlockedIncrement(static_cast<volatile size_t*>(&this->cache_version));
            this->sockets_for_cache_versions.speak(parent_alive, call_number);

            // constructors.clear();
        };
        void impl::RootScope::add_constructor(Breadcrumb* BC, GL::Proxy_Function&& func) {
            constructors.add_function(BC->GetCurrentNamespace() + GL::string::namespace_colons() + func->m_signature.name_m, std::move(func));
            ++constructors_version;
        };
        impl::Converter impl::RootScope::get_converters() {
            return Converter(constructors, converters, converter_lock, constructors_version);
        };
        GL::fast_shared_ptr<GL::details::Proxy_Function_Base> impl::RootScope::try_get_converter(GL::type const& from, GL::type const& to, bool in_function) {
            return get_converters().try_get_converter(from, to, 0, in_function);
        };
        bool impl::RootScope::can_convert(GL::type const& from, GL::type const& to, bool in_function) {
            return get_converters().can_convert(from, to, in_function);
        };
        std::vector<GL::type> impl::RootScope::all_convertable_types() const {
            std::unordered_set<GL::type> AllTypes;
            std::unordered_map<GL::type, std::unordered_map<GL::type, GL::Proxy_Function const*>> AllConversions; // all built-in conversions, e.g. int->float, float->double, etc.
            (void)this->constructors.for_each([&AllConversions, &AllTypes](GL::Proxy_Function const& func)->bool {
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

            std::vector<GL::type> out;
            for (auto& x : AllTypes) { 
                out.push_back(x);
            }
            return out;
        };
        void impl::RootScope::perform_builtins() {
            // type-casting or language support
            if (1) {
                this->add_function(GL::make_callable("printf", [](GL::string const& str) -> void { 
                    std::cout << str << std::endl;
                }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static | GL::function_signature::Volatile));

                // this->add_function(GL::make_callable("reference_cast", [](GL::any::fast_any any_type) -> GL::any::fast_any { return any_type | GL::type::Reference; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                // this->add_function(GL::make_callable("reinterpret_cast", [](GL::any::fast_any from, GL::type const& to_type) -> GL::any::fast_any { from.m_casted_type = to_type; return from; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                // this->add_function(GL::make_callable("const_cast", [](GL::any::fast_any any_type) -> GL::any::fast_any { return any_type - GL::type::Const; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
            }

            // void
            if (1) {
                this->add_function(GL::make_callable("to_string", 
                    [](GL::any::fast_any rhs) -> GL::string { return GL::string::empty_string(); },
                    GL::function_signature::Async | GL::function_signature::Constant, {}, { { "rhs", GL::type_of<void>() } }, GL::type_of<GL::string>()
                ));
                this->add_function(GL::make_callable("to_hash",
                    [](GL::any::fast_any rhs) -> size_t { return 0; },
                    GL::function_signature::Async | GL::function_signature::Constant, {}, { { "rhs", GL::type_of<void>() } }, GL::type_of<size_t>()
                ));
            }

            // basic numbers
            if (1) {
#define add_a(type) \
                this->make_class(GL::type_of< type >()).add_function(GL::make_callable(GL::type_of< type >().name(), []() -> type { return 0; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< type >())); \
                this->make_class(GL::type_of< type >()).add_function(GL::make_callable("to_string", [this](type const& rhs) -> GL::string { \
                    if constexpr (std::is_same_v<type, char> || std::is_same_v<type, unsigned char>){ \
                        return GL::string(std::string(1, rhs)); \
                    } else { \
                        return GL::string(std::to_string(rhs)); \
                    } \
                }, GL::function_signature::Constant)); \
                this->make_class(GL::type_of< type >()).add_function(GL::make_callable("to_hash", [this](type const& rhs) -> size_t { return std::hash<type>()(rhs); }, GL::function_signature::Constant)); \
                add_function(GL::make_callable("=", [](GL::any::fast_any lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type &>() = rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type &>() }, { "rhs", GL::type_of<type const&>() }}, GL::type_of< type& >())); \
                this->make_class(GL::type_of< bool >()).add_function(GL::make_converter<type, bool>()); \
                this->make_class(GL::type_of< char >()).add_function(GL::make_converter<type, char>()); \
                this->make_class(GL::type_of< unsigned char >()).add_function(GL::make_converter<type, unsigned char>()); \
                this->make_class(GL::type_of< int >()).add_function(GL::make_converter<type, int>()); \
                this->make_class(GL::type_of< unsigned int >()).add_function(GL::make_converter<type, unsigned int>()); \
                this->make_class(GL::type_of< long >()).add_function(GL::make_converter<type, long>()); \
                this->make_class(GL::type_of< unsigned long >()).add_function(GL::make_converter<type, unsigned long>()); \
                this->make_class(GL::type_of< long long >()).add_function(GL::make_converter<type, long long>()); \
                this->make_class(GL::type_of< size_t >()).add_function(GL::make_converter<type, size_t>()); \
                this->make_class(GL::type_of< float >()).add_function(GL::make_converter<type, float>()); \
                this->make_class(GL::type_of< double >()).add_function(GL::make_converter<type, double>()); \
                this->make_class(GL::type_of< long double >()).add_function(GL::make_converter<type, long double>()); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("==", [](type const& lhs, type const& rhs) -> bool { return lhs == rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("!=", [](type const& lhs, type const& rhs) -> bool { return lhs != rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable(">", [](type const& lhs, type const& rhs) -> bool { return lhs > rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("<", [](type const& lhs, type const& rhs) -> bool { return lhs < rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable(">=", [](type const& lhs, type const& rhs) -> bool { return lhs >= rhs; }, GL::function_signature::Constant)); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("<=", [](type const& lhs, type const& rhs) -> bool { return lhs <= rhs; }, GL::function_signature::Constant))

#define add_c(type) \
                add_a(type); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("+=", [](GL::any::fast_any lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() += rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("-=", [](GL::any::fast_any lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() -= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("*=", [](GL::any::fast_any lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() *= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("/=", [](GL::any::fast_any lhs, type const& rhs) -> GL::any::fast_any { lhs.cast<type&>() /= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() }, { "rhs", GL::type_of<type const&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("++", [](GL::any::fast_any lhs) -> GL::any::fast_any { ++lhs.cast< type& >(); return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() } }, GL::type_of< type& >())); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("--", [](GL::any::fast_any lhs) -> GL::any::fast_any { --lhs.cast< type& >(); return lhs; }, 0, {}, { { "lhs", GL::type_of<type&>() } }, GL::type_of< type& >()))

#define add_d(type) \
                add_c(type); \
                /*this->make_class(GL::type_of< type >()).*/add_function(GL::make_callable("-", [](type const& lhs) -> type { \
                    if constexpr (std::is_unsigned_v< type > || std::is_same_v< type, size_t >) { return type{ 0 }; } \
                    else { return -lhs; } \
                }, GL::function_signature::Constant))

                add_a(bool);
                /*this->make_class(GL::type_of< bool >()).*/add_function(GL::make_callable("!", [](bool const& lhs) -> bool { return !lhs; }, GL::function_signature::Constant));
                add_d(char);
                add_c(unsigned char);
                add_d(int);
                add_c(unsigned int);
                add_d(long);
                add_c(unsigned long);
                add_d(long long);
                add_c(size_t);
                add_d(float);
                add_d(double);
                add_d(long double);
#undef add_a
#undef add_c
#undef add_d

                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("|=", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() |= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("|", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs | rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("&=", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() &= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("&", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs & rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("^=", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() ^= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("^", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs ^ rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("~", [](size_t const& lhs) -> size_t { return ~lhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("<<=", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() <<= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("<<", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs << rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable(">>=", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<size_t&>() >>= rhs; return lhs; }, 0, {}, { { "lhs", GL::type_of<size_t&>() }, { "rhs", GL::type_of<size_t const&>() } }, GL::type_of< size_t& >()));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable(">>", [](size_t const& lhs, size_t const& rhs) -> size_t { return lhs >> rhs; }, GL::function_signature::Constant));

                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("&&", [](bool const& lhs, bool const& rhs) -> bool { return lhs && rhs; }, GL::function_signature::Constant));
                /*this->make_class(GL::type_of< size_t >()).*/add_function(GL::make_callable("||", [](bool const& lhs, bool const& rhs) -> bool { return lhs || rhs; }, GL::function_signature::Constant));

            }

            // units 
            if (1) {
// The bug is located here - fix this.  
#define DerivedUnitType(type, category, abbreviation, Ratio) \
        this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable(GL::type_of< type >().name(), []() -> type { return type{ 0.0f }; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< type >())); \
        this->make_class(GL::type_of< GL::value >()).add_function(GL::make_converter<GL::type, GL::value>()); \
        this->make_class(GL::type_of< GL::type >()).add_function(GL::make_converter<GL::value, GL::type>()); \
        this->make_class(GL::type_of< GL::type >()).add_function(GL::make_callable("to_hash", [](GL::type const& rhs) -> size_t { return std::hash<float>()((float)rhs); }))

#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) \
        DerivedUnitType(prefix ## type, 0, 0, 0)

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
    DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, pico, p); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, nano, n); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, micro, u); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, milli, m); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, centi, c); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deci, d); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deca, da); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, hecto, h); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, kilo, k); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, mega, M); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, giga, G); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, tera, T); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, peta, P)

                DerivedUnitList; // this loops through the definitions for DerivedUnitTypeWithMetricPrefixes() and DerivedUnitType() for all units. Change thosse macro definitions to change the implimentations. 

                DerivedUnitType(kelvin, 0, 0, 0);
                DerivedUnitType(fahrenheit, 0, 0, 0);

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitType
#undef CalculateMetricPrefixV

                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable(GL::type_of< GL::value >().name(), []() -> GL::value { return GL::value{ 0.0f }; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< GL::value >()));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable(GL::type_of< GL::value >().name(), [](GL::value const& rhs) -> GL::value { return rhs; }, GL::function_signature::Constructor | GL::function_signature::Async));
                this->make_class(GL::type_of< float >()).add_function(GL::make_converter<GL::value, float>());
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_converter<float, GL::value>());
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("=", [](GL::any::fast_any lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() = rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("==", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs == rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("!=", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs != rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable(">", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs > rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("<", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs < rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable(">=", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs >= rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("<=", [](GL::value const& lhs, GL::value const& rhs) -> bool { return lhs <= rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("+", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs + rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("-", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs - rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("*", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs * rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("/", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs / rhs; }, GL::function_signature::Async | GL::function_signature::Constant));
                
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("%", [](GL::value const& lhs, GL::value const& rhs) -> GL::value { return lhs.mod(rhs); }, GL::function_signature::Async | GL::function_signature::Constant));



                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("+=", [](GL::any::fast_any lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() += rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("-=", [](GL::any::fast_any lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() -= rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("*=", [](GL::any::fast_any lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() *= rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("/=", [](GL::any::fast_any lhs, GL::value const& rhs) -> GL::any::fast_any { lhs.cast<GL::value&>() /= rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() }, { "rhs", GL::type_of<GL::value const&>() } }, GL::type_of< GL::value& >()));

                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("++", [](GL::any::fast_any lhs) -> GL::any::fast_any { ++lhs.cast<GL::value&>(); return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("--", [](GL::any::fast_any lhs) -> GL::any::fast_any { --lhs.cast<GL::value&>(); return lhs; }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::value&>() } }, GL::type_of< GL::value& >()));
                /*this->make_class(GL::type_of< GL::value >()).*/add_function(GL::make_callable("-", [](GL::value const& lhs) -> GL::value { return -lhs; }, GL::function_signature::Async | GL::function_signature::Constant));

                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::pow));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::pow_value));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sqrt));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::rsqrt));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::rsqrt_fast));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::floor));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::ceiling));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::abs));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::clamp));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable("round", [this](GL::value const& lhs, float magnitude) -> GL::value { return lhs.round(magnitude); },
                    GL::function_signature::Async | GL::function_signature::Constant, 
                    { GL::any{ 1.0f } }, { {"lhs", GL::type_of<GL::value const&>() }, {"magnitude", GL::type_of<float>() } }, GL::type_of<GL::value>() 
                ));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::max));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::min));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log2));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log10));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::log1p));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::exp));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::exp2));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::expm1));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sign));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::mod));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::wrap));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::lerp));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sin));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::sin_fast));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::cos));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::tan));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::asin));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::acos));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::decl_func(&GL::value::atan));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable("to_string", [this](GL::value const& lhs) -> GL::string {
                    GL::string Num = std::to_string((float)lhs);
                    auto& ab = lhs.abbreviation();
                    if (ab.empty()) {
                        return Num.remove_trailing('0').remove_trailing('.');
                    }
                    else {
                        return Num.remove_trailing('0').remove_trailing('.') + " " + ab;
                    }
                    
                }, GL::function_signature::Async | GL::function_signature::Constant));
                this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable("to_hash", [this](GL::value const& rhs) -> size_t {
                    return std::hash<float>()((float)rhs);
                }));

                //this->make_class(GL::type_of< GL::value >()).add_function(GL::make_callable("rename_unit", [](GL::value const& at, GL::string const& name, GL::string const& abbreviation) {
                //    const_cast<GL::string&>(at.unsafe_impl().name) = name.to_string();
                //    const_cast<GL::string&>(at.unsafe_impl().abbreviation) = abbreviation.to_string();
                //}));

                auto& constants_NS = this->make_namespace("constants");
                constants_NS.insert_object_here("pi", GL::any::fast_any::instance(GL::constants::pi()));
                constants_NS.insert_object_here("viscosity", GL::any::fast_any::instance(GL::constants::viscosity()));
                constants_NS.insert_object_here("half_pi", GL::any::fast_any::instance(GL::constants::half_pi()));
                constants_NS.insert_object_here("g", GL::any::fast_any::instance(GL::constants::g()));
                constants_NS.insert_object_here("G", GL::any::fast_any::instance(GL::constants::G()));
                constants_NS.insert_object_here("d", GL::any::fast_any::instance(GL::constants::d()));
                constants_NS.insert_object_here("c", GL::any::fast_any::instance(GL::constants::c()));
            }

            // types
            if (1) {
                using class_t = GL::type;
                auto& Class = this->make_class(GL::type_of<class_t>());
                /* default constructor */ Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return {}; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< class_t >()));
                /* copy constructor */    Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor | GL::function_signature::Async));
                /* assignment operator */ this->add_function(GL::make_callable("=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, GL::function_signature::Async, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));

                this->add_function(GL::make_callable("type_of", [](GL::any::fast_any any_type) -> GL::type { return any_type.m_casted_type; }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("size", [](GL::type const& any_type) -> size_t { return any_type.size(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->add_function(GL::make_callable("type_name", [](GL::any::fast_any any_type) -> GL::string { 
                    return any_type.m_casted_type.name(); 
                }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("type_name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                this->add_function(GL::make_callable("size_of", [](GL::any::fast_any any_type) -> size_t { return any_type.m_casted_type.size(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_void", [](GL::type const& any_type) -> bool { return any_type.is_void(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_any", [](GL::type const& any_type) -> bool { return any_type.is_any(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_const", [](GL::type const& any_type) -> bool { return any_type.is_const(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_cpp_type", [](GL::type const& any_type) -> bool { return any_type.is_cpp_type(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_ref", [](GL::type const& any_type) -> bool { return any_type.is_ref(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_temp", [](GL::type const& any_type) -> bool { return any_type.is_temp(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_const_ref", [](GL::type const& any_type) -> bool { return any_type.is_const_ref(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_base", [](GL::type const& any_type) -> bool { return any_type.is_base(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_base_of", [](GL::type const& a, GL::type const& b) -> bool { return a.is_base_of(b); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("is_derived_from", [](GL::type const& a, GL::type const& b) -> bool { return a.is_derived_from(b); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("get_hash", [](GL::type const& a) -> size_t { return a.get_hash(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));
                Class.add_function(GL::make_callable("get_base_hash", [](GL::type const& a) -> size_t { return a.get_base_hash(); }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static));

                Class.add_function(GL::make_callable("to_string", [](class_t const& rhs) -> GL::string { return rhs.name(); }));
                Class.add_function(GL::make_callable("to_hash", [](class_t const& rhs) -> size_t { return std::hash<class_t>()(rhs); }));
            }

            // var (generic "variable" container, for wrapping assignment of any type within the script language. Effectively the scripting language's version of 'any')
            if (1) {
                auto& var_class = this->make_class(GL::type_of< GL::var >());
                // default constructor
                var_class.add_function(GL::make_callable(GL::type_of< GL::var >().name(), []() -> GL::var { return {}; }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, GL::type_of< GL::var >()));
                // copy constructor
                var_class.add_function(GL::make_callable(GL::type_of< GL::var >().name(), [](GL::var const& rhs) -> GL::var { return rhs; }, GL::function_signature::Constructor | GL::function_signature::Async));
                // template constructor, create a var from anything
                var_class.add_function(GL::make_callable(GL::type_of< GL::var >().name(), [](GL::any::fast_any rhs) -> GL::var {
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        return rhs.cast<GL::var&>();
                    }
                    else {
                        return GL::var(GL::make_shared<GL::any>(rhs));
                    }
                    
                }, GL::function_signature::Constructor | GL::function_signature::Async, {}, { { "rhs", GL::type_of<GL::any>() + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::var>()));
                // assignment operator. Anything can be assigned to an empty var object.
                this->add_function(GL::make_callable("=", [](GL::any::fast_any& lhs, GL::any::fast_any rhs) -> GL::any::fast_any {                    
                    GL::any::fast_any& out = lhs;
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        lhs.cast<GL::var&>() = rhs.cast<GL::var&>();   
                        out.m_casted_type = rhs.m_casted_type | GL::type::Reference;
                    }
                    else {
                        lhs.cast<GL::var&>() = GL::var(GL::make_shared<GL::any>(rhs));
                        
                    }
                    return out;
                }, GL::function_signature::Async, {}, { { "lhs",GL::type_of<GL::var&>() }, { "rhs", GL::type_of<GL::any>() } }/*, GL::type_of<GL::var&>()*/));
                // forced assignment operator. Anything can be assigned to an var object using this operator. If something was already assigned, it is over-written. 
                this->add_function(GL::make_callable(":=", [](GL::any::fast_any& lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    GL::any::fast_any& out = lhs; 
                    if (rhs.can_cast(GL::type_of<GL::var&>())) {
                        lhs.cast<GL::var&>() = rhs.cast<GL::var&>();
                        out.m_casted_type = rhs.m_casted_type | GL::type::Reference;
                    }
                    else {
                        lhs.cast<GL::var&>() = GL::var(GL::make_shared<GL::any>(rhs));
                        out.m_casted_type = rhs.m_casted_type;
                    }
                    return out;
                }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::var&>() }, { "rhs", GL::type_of<GL::any>() } }, GL::type_of<GL::var&>()));                
                // boolean test for vars, to ensure they are "valid". Note that this may actually call the conversion on the stored object, so this has been cut (for now)
                //this->make_class(GL::type_of< bool >()).add_function(GL::make_callable(GL::type_of< bool >().name(), [](GL::var const& rhs) -> bool {
                //    return rhs.get_type().get_base_hash() != GL::type_of<GL::var>().get_base_hash();
                //}, GL::function_signature::Explicit | GL::function_signature::Constructor, {}, {}, GL::type_of<bool>()));
                // boolean test for vars, to ensure they are "valid"
                var_class.add_function(GL::make_callable("valid", [](GL::var const& rhs) -> bool { return rhs.get_type().get_base_hash() != GL::type_of<GL::var>().get_base_hash(); }, GL::function_signature::Async));

                var_class.add_function(GL::make_callable("to_string", [](GL::var const& rhs) -> GL::string {  
                    static thread_local int recursion_depth{ 0 };
                    class recursion_depth_manager {
                    private:
                        int& r;
                    public:
                        recursion_depth_manager(int& R) : r(R) {
                            ++r;
                        };
                        ~recursion_depth_manager() {
                            --r;
                        };
                    };
                    recursion_depth_manager manager(recursion_depth);
                    if (recursion_depth > 2) return GL::string::empty_string();
                    GL::any::fast_any this_any;
                    GL::var* this_var = &const_cast<GL::var&>(rhs);
                    while (this_var) {
                        if (auto p = this_var->get_data(); p) {
                            if (this_any = p->fast(); this_any) {
                                if (this_any.can_cast(GL::type_of<GL::var&>())) {
                                    this_var = &this_any.cast<GL::var&>();
                                }
                                else {
                                    if (auto* C = GL::scope::GetClass(this_any.m_casted_type); C) {
                                        return C->call<GL::string>("to_string", { this_any });
                                    }
                                    else {
                                        return GL::scope::GetCurrentCaller()->GetRoot()->call<GL::string>("to_string", { this_any });
                                    }
                                }
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            break;
                        }
                    }
                    return GL::string::empty_string();
                }, GL::function_signature::Async | GL::function_signature::Explicit));
                var_class.add_function(GL::make_callable("to_hash", [](GL::var const& rhs) -> size_t { 
                    static thread_local int recursion_depth{ 0 };
                    class recursion_depth_manager {
                    private:
                        int& r;
                    public:
                        recursion_depth_manager(int& R) : r(R) {
                            ++r;
                        };
                        ~recursion_depth_manager() {
                            --r;
                        };
                    };
                    recursion_depth_manager manager(recursion_depth);
                    if (recursion_depth > 2) return 0;
                    GL::any::fast_any this_any;
                    GL::var* this_var = &const_cast<GL::var&>(rhs);
                    while (this_var) {
                        if (auto p = this_var->get_data(); p) {
                            if (this_any = p->fast(); this_any) {
                                if (this_any.can_cast(GL::type_of<GL::var&>())) {
                                    this_var = &this_any.cast<GL::var&>();
                                }
                                else {
                                    if (auto* C = GL::scope::GetClass(this_any.m_casted_type); C) {
                                        return C->call<size_t>("to_hash", { this_any });
                                    }
                                    else {
                                        return GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { this_any });
                                    }
                                }
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            break;
                        }
                    }
                    return 0;
                }, GL::function_signature::Async | GL::function_signature::Explicit));
            }

            // std::string support
            if (1) {
                using class_t = std::string;
                auto& std_ns = this->make_namespace("std");
                auto& Class = std_ns.make_class(GL::type_of<class_t>());
                // default constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type));
                // copy constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor));
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GL::string const& rhs) -> class_t { return rhs.to_string(); }, GL::function_signature::Constructor));
                // assignment operator
                this->add_function(GL::make_callable("=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));                               

                Class.add_function(GL::make_callable("to_string", [](class_t const& rhs) -> GL::string { return (class_t)rhs; }));
                Class.add_function(GL::make_callable("to_hash", [](class_t const& rhs) -> size_t { return std::hash<class_t>()(rhs); }));
            }

#if 0
            // datetime
            if (1) {
                using class_t = GL::datetime;
                GL::type_of<class_t>().try_update_name("datetime");
                auto& Class = this->make_class(GL::type_of<class_t>());
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type));
                Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor));
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GL::string const& rhs) -> class_t { return class_t(rhs); }, GL::function_signature::Constructor | GL::function_signature::Explicit));

                // assignment operator
                this->add_function(GL::make_callable("=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                this->add_function(GL::make_callable("==", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs == rhs; }));
                this->add_function(GL::make_callable("!=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs != rhs; }));
                this->add_function(GL::make_callable(">", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs > rhs; }));
                this->add_function(GL::make_callable(">=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs >= rhs; }));
                this->add_function(GL::make_callable("<", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs < rhs; }));
                this->add_function(GL::make_callable("<=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs <= rhs; }));
                this->add_function(GL::make_callable("+", [](class_t const& lhs, GL::minute const& rhs) -> class_t { return lhs + rhs; }));
                this->add_function(GL::make_callable("+", [](GL::minute const& lhs, class_t const& rhs) -> class_t { return lhs + rhs; }));
                this->add_function(GL::make_callable("-", [](class_t const& lhs, GL::minute const& rhs) -> class_t { return lhs - rhs; }));
                this->add_function(GL::make_callable("-", [](class_t const& lhs, class_t const& rhs) -> GL::minute { return lhs - rhs; }));
                this->add_function(GL::make_callable("*", [](class_t const& lhs, double const& rhs) -> class_t { return lhs * rhs; }));
                this->add_function(GL::make_callable("*", [](double const& lhs, class_t const& rhs) -> class_t { return rhs * lhs; }));
                this->add_function(GL::make_callable("/", [](class_t const& lhs, double const& rhs) -> class_t { return lhs / rhs; }));
                
                this->add_function(GL::make_callable("+=", [](GL::any::fast_any lhs, GL::minute const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() += rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<GL::minute const&>() } }, Class.this_type | GL::type::Reference));
                this->add_function(GL::make_callable("-=", [](GL::any::fast_any lhs, GL::minute const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() += rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<GL::minute const&>() } }, Class.this_type | GL::type::Reference));
                this->add_function(GL::make_callable("*=", [](GL::any::fast_any lhs, double const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() *= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<double const&>() } }, Class.this_type | GL::type::Reference));
                this->add_function(GL::make_callable("/=", [](GL::any::fast_any lhs, double const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() /= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<double const&>() } }, Class.this_type | GL::type::Reference));

                Class.add_function(GL::decl_func(&class_t::Epoch));
                Class.add_function(GL::make_callable("Now", []() -> datetime { return datetime::Now(); }, GL::function_signature::Static | GL::function_signature::Constant | GL::function_signature::Volatile, {}, {}));
                Class.add_function(GL::decl_func(&class_t::getNumDaysInSameMonth));
                Class.add_function(GL::decl_func(&class_t::GetUtcOffset));
                Class.add_function(GL::make_callable("make_time", [](int year, int month, int day, int hour, int minute, float second) -> datetime { return datetime::make_time(year,month,day,hour,minute,second); }, { 1970,1,1,0,0,0 }));

                Class.add_function(GL::decl_func(&class_t::ToStartOfMonth));
                Class.add_function(GL::decl_func(&class_t::ToStartOfDay));
                Class.add_function(GL::decl_func(&class_t::ToStartOfHour));
                Class.add_function(GL::decl_func(&class_t::ToStartOfMinute));
                Class.add_function(GL::decl_func(&class_t::ToEndOfMonth));
                Class.add_function(GL::decl_func(&class_t::ToEndOfDay));
                Class.add_function(GL::decl_func(&class_t::ToEndOfHour));
                Class.add_function(GL::decl_func(&class_t::ToEndOfMinute));
                Class.add_function(GL::decl_func(&class_t::ToNextMonth));
                Class.add_function(GL::decl_func(&class_t::ToNextDay));
                Class.add_function(GL::decl_func(&class_t::ToNextHour));
                Class.add_function(GL::decl_func(&class_t::ToNextMinute));

                Class.add_function(GL::decl_func(&class_t::tm_fractionalsec));
                Class.add_function(GL::decl_func(&class_t::tm_sec));
                Class.add_function(GL::decl_func(&class_t::tm_min));
                Class.add_function(GL::decl_func(&class_t::tm_hour));
                Class.add_function(GL::decl_func(&class_t::tm_mday));
                Class.add_function(GL::decl_func(&class_t::tm_mon));
                Class.add_function(GL::decl_func(&class_t::tm_year));
                Class.add_function(GL::decl_func(&class_t::tm_wday));
                Class.add_function(GL::decl_func(&class_t::tm_yday));

                Class.add_function(GL::make_callable("to_string", [](class_t const& rhs) -> GL::string { return rhs; }));
                Class.add_function(GL::make_callable("to_hash", [](class_t const& rhs) -> size_t { return std::hash<long long>()(rhs); }));
            }
#endif

            // pair<T0,T1>
            if (1) {
                auto& BaseClass = this->make_class("pair");
                BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") }, { "T1", GL::is_template::type<1>("T1") } };

                BaseClass.add_member_object("first", GL::is_template::type<0>("T0"));
                BaseClass.add_member_object("second", GL::is_template::type<1>("T1"));
                BaseClass.initialize_basic_member_functions();
            }

            // iterator<T>
            if (1) {
                auto& BaseClass = this->make_class("iterator");
                BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") } };

                BaseClass.add_member_object("parent", GL::type_of<GL::var>());
                BaseClass.add_function(GL::make_callable("begin", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    *this_iter["parent"] = rhs;
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }));
                BaseClass.add_function(GL::make_callable("end", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    *this_iter["parent"] = rhs;
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }));
                BaseClass.add_function(GL::make_callable("++", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        Class->call("++", { this_iter["parent"]->fast(), lhs });
                    }
                    return lhs;
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } }, BaseClass.this_type | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("+", [&BaseClass](GL::any::fast_any lhs, size_t const& pos) -> GL::any::fast_any {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        return Class->call("+", { this_iter["parent"]->fast(), lhs, GL::any::fast_any::instance(pos) });
                    }
                    return lhs;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "pos", GL::type_of<size_t const&>() } }, BaseClass.this_type));
                BaseClass.add_function(GL::make_callable("--", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        Class->call("--", { this_iter["parent"]->fast(), lhs });
                    }
                    return lhs;
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } }, BaseClass.this_type | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        return Class->call("get", { this_iter["parent"]->fast(), lhs }) | GL::type::Reference;
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } }));                
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        return Class->call("get", { this_iter["parent"]->fast(), lhs }) | GL::type::Const | GL::type::Reference;
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } }));
                BaseClass.add_function(GL::make_callable("==", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        if (auto f = Class->try_find_callable("==", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                        else if (auto f = Class->try_find_callable("!=", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return !f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("!=", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        if (auto f = Class->try_find_callable("!=", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                        else if (auto f = Class->try_find_callable("==", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return !f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                    }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        if (auto f = Class->try_find_callable(">", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                        else if (auto f = Class->try_find_callable("<=", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return !f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                    }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">=", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        if (auto f = Class->try_find_callable(">=", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                        else if (auto f = Class->try_find_callable("<", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return !f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                    }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        if (auto f = Class->try_find_callable("<", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                        else if (auto f = Class->try_find_callable(">=", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return !f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                    }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<=", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto& this_iter = lhs.cast<GL::dynamic_object&>();
                    if (auto* Class = GL::scope::GetClass(this_iter["parent"]->m_casted_type); Class) {
                        if (auto f = Class->try_find_callable("<=", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                        else if (auto f = Class->try_find_callable(">", { Class->this_type, lhs.m_casted_type, rhs.m_casted_type })) {
                            return !f->operator()({ this_iter["parent"]->fast(), lhs, rhs }).cast<bool>();
                        }
                    }
                    throw std::out_of_range("iterator out-of-bounds");
                    }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }, GL::type_of<bool>()));
                
                BaseClass.initialize_basic_member_functions();
            }

            // range(A, B) or A..B
            if (1) {
                auto& BaseClass = this->make_class("range");
                BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") } };
                BaseClass.add_member_object("from", GL::is_template::type<0>("T0"));
                BaseClass.add_member_object("to", GL::is_template::type<0>("T0"));

                BaseClass.add_function(GL::make_callable("begin", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    if (auto* impl_class = GetClass(lhs.m_casted_type)) {
                        auto new_iterator = impl_class->call("iterator<" + impl_class->this_type.name() + ">", {});
                        impl_class->call("begin", { new_iterator, lhs }); // initializes the base iterator 
                        auto& iterator = new_iterator.cast<GL::dynamic_object&>();
                        iterator["position"] = GL::make_shared<GL::any>((size_t)0ull);
                        return new_iterator;
                    }
                    throw std::runtime_error("Could not find the associated class");
                    }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }));
                BaseClass.add_function(GL::make_callable("end", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    if (auto* impl_class = GetClass(lhs.m_casted_type)) {
                        auto new_iterator = impl_class->call("iterator<" + impl_class->this_type.name() + ">", {});
                        impl_class->call("end", { new_iterator, lhs }); // initializes the base iterator 
                        
                        auto& iterator = new_iterator.cast<GL::dynamic_object&>();
                        iterator["position"] = GL::make_shared<GL::any>((size_t)impl_class->call<size_t>("-", { impl_class->call("to", { lhs }), impl_class->call("from", { lhs }) }));
                        
                        return new_iterator;
                    }
                    throw std::runtime_error("Could not find the associated class");
                    }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& source = lhs.cast<GL::dynamic_object&>();
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    return impl_class->call("+", { source["from"]->fast(), iterator["position"]->fast() }) | GL::type::Reference;
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::is_template::type<0>("T0") | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& source = lhs.cast<GL::dynamic_object&>();
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    return impl_class->call("+", { source["from"]->fast(), iterator["position"]->fast() }) | (GL::type::Reference | GL::type::Const);
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::is_template::type<0>("T0") | GL::type::Reference | GL::type::Const));
                BaseClass.add_function(GL::make_callable("++", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    ++iterator["position"]->cast<size_t>();
                    // if we needed to do anything with the iterator, this would have been the time.
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("--", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    --iterator["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("+", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs, size_t  const& offset) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);                    
                    auto new_iter = impl_class->call("begin", { lhs });
                    auto& old_iterator = rhs.cast<GL::dynamic_object&>();
                    auto& new_iterator = new_iter.cast<GL::dynamic_object&>();
                    new_iterator["position"]->cast<size_t>() = old_iterator["position"]->cast<size_t>() + offset;
                    return new_iter;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() }, { "offset", GL::type_of<size_t const&>() } }));
                BaseClass.add_function(GL::make_callable("==", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() == iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("!=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() != iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() > iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() >= iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() < iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() <= iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));

                BaseClass.initialize_basic_member_functions();

                this->add_function(GL::make_callable("..", [](GL::any::fast_any LHS, GL::any::fast_any RHS) -> GL::any::fast_any {
                    if (auto* x = GetClass(LHS.m_casted_type)) {
                        auto range = GetCurrentCaller()->call("range<" + x->this_type.name() + ">", {});
                        GetCurrentCaller()->call("=", { GetCurrentCaller()->call("from", { range }), LHS });
                        GetCurrentCaller()->call("=", { GetCurrentCaller()->call("to", { range }), RHS });
                        return range;
                    }
                    throw std::runtime_error("Could not find the associated class");
                }, GL::function_signature::Constant | GL::function_signature::Static));
            }

            // vector<T>
            if (1) {
                auto& BaseClass = this->make_class("vector");
                BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") } };

                if (1) {
                    // teach it how to create the generic map
                    GL::type_of<GL::atomic_constructable_vector<GL::any::fast_any>>().try_update_name("vector_impl");
                    auto& AnyMap = BaseClass.make_class(GL::type_of<GL::atomic_constructable_vector<GL::any::fast_any>>());
                    AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), [&AnyMap]() -> GL::shared_ptr<GL::atomic_constructable_vector<GL::any::fast_any>> {
                        return GL::make_shared<GL::atomic_constructable_vector<GL::any::fast_any>>([]() -> GL::any::fast_any {
                            auto* p = GL::scope::GetCurrentCaller()->GetNamespace();
                            if (p->is_class()) {
                                if (auto* BC = p->GetRoot()->try_find_class(dynamic_cast<GL::scope::impl::ClassScope*>(p)->template_types[0].second); BC) {
                                    return BC->this_m.scope->call(BC->this_m.scope_name, {});
                                }                                
                            }
                            throw std::runtime_error("Must be called from the impl class constructor");
                        });
                    }, GL::function_signature::Constructor + GL::function_signature::Async, {}, {}, AnyMap.this_type));
                    AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), [&AnyMap](GL::atomic_constructable_vector<GL::any::fast_any> const& rhs) -> GL::shared_ptr<GL::atomic_constructable_vector<GL::any::fast_any>> {
                        auto out = GL::make_shared<GL::atomic_constructable_vector<GL::any::fast_any>>([]() -> GL::any::fast_any {
                            auto* p = GL::scope::GetCurrentCaller()->GetNamespace();
                            if (p->is_class()) {
                                if (auto* BC = p->GetRoot()->try_find_class(dynamic_cast<GL::scope::impl::ClassScope*>(p)->template_types[0].second); BC) {
                                    return BC->this_m.scope->call(BC->this_m.scope_name, {});
                                }                                
                            }
                            throw std::runtime_error("Must be called from the impl class constructor");
                        });
                        for (auto& x : rhs) {
                            out->push_back([&]() -> GL::any::fast_any {
                                if (auto* BC = AnyMap.GetRoot()->try_find_class(x.m_casted_type)) {
                                    return BC->this_m.scope->call(BC->this_m.scope_name, { x.fast() });
                                }
                                return x.fast();
                            }());
                        }
                        return out;
                        }, GL::function_signature::Constructor + GL::function_signature::Async, {}, { { "rhs", AnyMap.this_type + GL::type::Const + GL::type::Reference }}, AnyMap.this_type));
                    AnyMap.GetRoot()->add_function(GL::make_callable("=", [&AnyMap](GL::any::fast_any Lhs, GL::atomic_constructable_vector<GL::any::fast_any> const& rhs) -> GL::any::fast_any {
                        auto& out = Lhs.cast<GL::atomic_constructable_vector<GL::any::fast_any>&>();
                        int L = 0;
                        for (auto& x : rhs) {
                            out.grow_to_at_least(++L);
                            auto& p = out.at(L - 1);
                            if (auto p_f = p.fast(); p_f) {
                                p = AnyMap.GetRoot()->call("=", { p_f, x.fast() });
                            }
                            else {
                                if (auto* BC = AnyMap.GetRoot()->try_find_class(x.m_casted_type)) {
                                    p = BC->this_m.scope->call(BC->this_m.scope_name, { x.fast() });
                                }
                                else {
                                    p = x;
                                }
                            }
                        }
                        return Lhs;
                    }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<GL::atomic_constructable_vector<GL::any::fast_any>&>() }, { "rhs", GL::type_of<GL::atomic_constructable_vector<GL::any::fast_any> const&>() } }, GL::type_of<GL::atomic_constructable_vector<GL::any::fast_any>&>()));

                    AnyMap.add_function(GL::make_callable("push_back", [&AnyMap](GL::atomic_constructable_vector<GL::any::fast_any>& lhs, GL::any::fast_any rhs) -> size_t {
                        return lhs.push_back(rhs);
                    }, GL::function_signature::Async));
                    AnyMap.add_function(GL::make_callable("grow_to_at_least", [&AnyMap](GL::atomic_constructable_vector<GL::any::fast_any>& lhs, size_t const& rhs) -> bool {
                        return lhs.grow_to_at_least(rhs);
                    }, GL::function_signature::Async));

                    // to_string and to_hash functions
                    AnyMap.add_function(GL::make_callable("to_string", [](GL::atomic_constructable_vector<GL::any::fast_any> const& rhs) -> GL::string {
                        GL::string out;
                        for (size_t i = 0; i < rhs.size(); ++i) {
                            auto first_str = GL::scope::GetCurrentCaller()->call<GL::string>("to_string", { rhs[i] });
                            out = out.add_to_delim(first_str, ", ");
                        }
                        return "[" + out + "]";
                    }, GL::function_signature::Async | GL::function_signature::Constant));
                    AnyMap.add_function(GL::make_callable("to_hash", [](GL::atomic_constructable_vector<GL::any::fast_any> const& rhs) -> size_t {
                        size_t out = 0;
                        for (size_t i = 0; i < rhs.size(); ++i) {
                            auto first_hash = GL::scope::GetCurrentCaller()->call<size_t>("to_hash", { rhs[i] });
                            GL::util::hash(out, first_hash);
                        }
                        return out;
                    }, GL::function_signature::Async | GL::function_signature::Constant));
                }
                // the use of "~" at the start of this member object's name is not arbitrary. This is a special code that means this is an intended-to-be-hidden wrapper for the dynamic_object.
                BaseClass.add_member_object("~impl", GL::type_of<GL::atomic_constructable_vector<GL::any::fast_any>>());
                BaseClass.add_function(GL::make_callable("push_back", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto* impl_class = GetClass(lhs.m_casted_type);
                        auto* param0_class = GetClass(rhs.m_casted_type);
                        auto copied = param0_class->call(param0_class->this_type.name(), { rhs });
                        return impl_class->call("push_back", { (*implp)->fast() + GL::type::Reference, copied });
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }, GL::type_of<size_t>()));
                BaseClass.add_function(GL::make_callable("push_back", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto* impl_class = GetClass(lhs.m_casted_type);
                        return impl_class->call("push_back", { (*implp)->fast() + GL::type::Reference, rhs - GL::type::Temporary });
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Temporary } }, GL::type_of<size_t>()));
                BaseClass.add_function(GL::make_callable("size", [&BaseClass](GL::any::fast_any lhs) -> size_t {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<GL::atomic_constructable_vector<GL::any::fast_any>>();
                        return impl.size();
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } }, GL::type_of<size_t>()));
                BaseClass.add_function(GL::make_callable("at", [&BaseClass](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<GL::atomic_constructable_vector<GL::any::fast_any>>();
                        return impl.at(rhs) | GL::type::Const | GL::type::Reference;
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::type_of<size_t>() | GL::type::Const | GL::type::Reference } }, GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("at", [&BaseClass](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<GL::atomic_constructable_vector<GL::any::fast_any>>();
                        return impl.at(rhs) | GL::type::Reference;
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::type_of<size_t>() | GL::type::Const | GL::type::Reference } }, GL::is_template::type<0>("T0") | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("[]", [&BaseClass](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<GL::atomic_constructable_vector<GL::any::fast_any>>();                        
                        if (rhs < impl.size()) 
                            return impl[rhs] | GL::type::Const | GL::type::Reference;
                        throw std::runtime_error("Index was out of bounds for the array");
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::type_of<size_t>() | GL::type::Const | GL::type::Reference } }, GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("[]", [&BaseClass](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<GL::atomic_constructable_vector<GL::any::fast_any>>();
                        if (rhs < impl.size())
                            return impl[rhs] | GL::type::Reference;
                        throw std::runtime_error("Index was out of bounds for the array");
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::type_of<size_t>() | GL::type::Const | GL::type::Reference } }, GL::is_template::type<0>("T0") | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("grow_to_at_least", [&BaseClass](GL::any::fast_any lhs, size_t const& rhs) -> bool {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto* impl_class = GetClass(lhs.m_casted_type);
                        return impl_class->call<bool>("grow_to_at_least", { (*implp)->fast() + GL::type::Reference, GL::any::fast_any::instance(rhs) });
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::type_of<size_t>() | GL::type::Const | GL::type::Reference } }, GL::type_of<bool>()));
                
                BaseClass.add_function(GL::make_callable("begin", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    if (auto* impl_class = GetClass(lhs.m_casted_type)) {
                        auto new_iterator = impl_class->call("iterator<" + impl_class->this_type.name() + ">", {});
                        impl_class->call("begin", { new_iterator, lhs }); // initializes the base iterator 
                        auto& iterator = new_iterator.cast<GL::dynamic_object&>();
                        iterator["position"] = GL::make_shared<GL::any>((size_t)0ull);
                        return new_iterator;
                    }
                    throw std::runtime_error("Could not find the associated class");
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }));
                BaseClass.add_function(GL::make_callable("end", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    if (auto* impl_class = GetClass(lhs.m_casted_type)) {
                        auto new_iterator = impl_class->call("iterator<" + impl_class->this_type.name() + ">", {});
                        impl_class->call("end", { new_iterator, lhs }); // initializes the base iterator 
                        auto& iterator = new_iterator.cast<GL::dynamic_object&>();
                        if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                            auto& impl = (*implp)->cast<GL::atomic_constructable_vector<GL::any::fast_any>>();
                            iterator["position"] = GL::make_shared<GL::any>((size_t)impl.size());
                        }
                        return new_iterator;
                    }
                    throw std::runtime_error("Could not find the associated class");
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    return impl_class->call("[]", { lhs, iterator["position"]->fast() }) | GL::type::Reference;                    
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::is_template::type<0>("T0") | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    return impl_class->call("[]", { lhs, iterator["position"]->fast() }) | GL::type::Reference | GL::type::Const;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::is_template::type<0>("T0") | GL::type::Reference | GL::type::Const));
                BaseClass.add_function(GL::make_callable("++", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    ++iterator["position"]->cast<size_t>();
                    // if we needed to do anything with the iterator, this would have been the time.
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("--", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    --iterator["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("+", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs, size_t  const& offset) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto new_iter = impl_class->call("begin", { lhs });
                    auto& old_iterator = rhs.cast<GL::dynamic_object&>();
                    auto& new_iterator = new_iter.cast<GL::dynamic_object&>();
                    new_iterator["position"]->cast<size_t>() = old_iterator["position"]->cast<size_t>() + offset;
                    return new_iter;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() }, { "offset", GL::type_of<size_t const&>() } }));
                BaseClass.add_function(GL::make_callable("==", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() == iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("!=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() != iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() > iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() >= iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() < iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["position"]->cast<size_t>() <= iterator2["position"]->cast<size_t>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));

                BaseClass.initialize_basic_member_functions();
            }

            // string functions
            if (1) {
                using class_t = GL::string;
                auto& Class = this->make_class(GL::type_of<class_t>());
                // default constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type));
                // copy constructor
                Class.add_function(GL::make_callable(Class.this_type.name(), [](class_t const& rhs) -> class_t { return rhs; }, GL::function_signature::Constructor));
                Class.add_function(GL::make_callable(Class.this_type.name(), [](std::string const& rhs) -> class_t { return class_t((std::string)rhs); }, GL::function_signature::Constructor));
                // assignment operator
                this->add_function(GL::make_callable("=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                this->add_function(GL::make_callable("==", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs == rhs; }));
                this->add_function(GL::make_callable("!=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs != rhs; }));
                this->add_function(GL::make_callable(">", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs > rhs; }));
                this->add_function(GL::make_callable(">=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs >= rhs; }));
                this->add_function(GL::make_callable("<", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs < rhs; }));
                this->add_function(GL::make_callable("<=", [](class_t const& lhs, class_t const& rhs) -> bool { return lhs <= rhs; }));
                this->add_function(GL::make_callable("+", [](class_t const& lhs, class_t const& rhs) -> class_t { return lhs + rhs; }));
                Class.add_function(GL::make_callable("[]", [&Class](class_t const& lhs, size_t const& rhs) -> const char& {
                    return lhs.operator[](rhs);
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", GL::type_of<class_t const&>() } , { "rhs", GL::type_of<size_t>() | GL::type::Const | GL::type::Reference } }, GL::type_of<const char&>()));

                Class.add_function(GL::decl_func(&class_t::add_to_delim));
                Class.add_function(GL::decl_func(&class_t::at));
                Class.add_function(GL::decl_func(&class_t::back));
                Class.add_function(GL::decl_func(&class_t::begins_with));
                Class.add_function(GL::make_callable("distance", [](class_t const& lhs, class_t const& rhs, bool case_sensitive) -> size_t { return lhs.distance(rhs, case_sensitive); }, GL::function_signature::Async | GL::function_signature::Constant, { true }, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const}, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const }, { "case_sensitive", GL::type_of<bool const&>() } }, GL::type_of<size_t>()));
                Class.add_function(GL::decl_func(&class_t::empty));
                Class.add_function(GL::make_callable("empty_string", []() -> GL::any::fast_any { 
                    static GL::string colons{ GL::string::empty_string() };
                    static GL::any::fast_any out{ GL::any::ref(colons).fast() + GL::type::Reference };
                    return out;
                }, GL::function_signature::Static, {}, {}, Class.this_type | GL::type::Reference));
                Class.add_function(GL::decl_func(&class_t::ends_with));
                Class.add_function(GL::make_callable("find", [](class_t const& lhs, class_t const& rhs, bool case_sensitive, long long start, long long end) -> size_t { return lhs.find(rhs, case_sensitive, start, end); }, { true, 0ll, -1ll }));
                Class.add_function(GL::decl_func(&class_t::front));
                Class.add_function(GL::decl_func(&class_t::hash));
                Class.add_function(GL::decl_func(&class_t::has_lower));
                Class.add_function(GL::decl_func(&class_t::has_upper));
                Class.add_function(GL::decl_func(&class_t::left));
                Class.add_function(GL::make_callable("left_and_right_of", [&Class](class_t const& lhs, class_t const& rhs) -> GL::any::fast_any {
                    auto results = lhs.left_and_right_of(rhs);
                    auto Pair = Class.call("pair<string,string>", {});
                    Class.call("=", { Class.call("first", { Pair }), GL::any::fast_any::instance(results.first) });
                    Class.call("=", { Class.call("second", { Pair }), GL::any::fast_any::instance(results.second) });
                    return Pair;                    
                }, GL::function_signature::Constant, {}, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.DetermineType("pair<string,string>")));
                Class.add_function(GL::make_callable("left_and_right_of_last", [&Class](class_t const& lhs, class_t const& rhs) -> GL::any::fast_any {
                    auto results = lhs.left_and_right_of_last(rhs);
                    auto Pair = Class.call("pair<string,string>", {});
                    Class.call("=", { Class.call("first", { Pair }), GL::any::fast_any::instance(results.first) });
                    Class.call("=", { Class.call("second", { Pair }), GL::any::fast_any::instance(results.second) });
                    return Pair;                    
                }, GL::function_signature::Constant, {}, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.DetermineType("pair<string,string>")));
                Class.add_function(GL::decl_func(&class_t::left_of));
                Class.add_function(GL::decl_func(&class_t::left_of_last));
                Class.add_function(GL::decl_func(&class_t::length));
                Class.add_function(GL::make_callable("namespace_colons", []() -> GL::any::fast_any { 
                    static GL::string colons{ GL::string::namespace_colons() };
                    static GL::any::fast_any out{ GL::any::ref(colons).fast() + GL::type::Reference };
                    return out;
                }, GL::function_signature::Static, {}, {}, Class.this_type | GL::type::Reference));
                Class.insert_object_here("npos", GL::any::ref(class_t::npos).fast());
                Class.add_function(GL::decl_func(&class_t::remove_leading));
                Class.add_function(GL::decl_func(&class_t::remove_leading_and_trailing));                
                Class.add_function(GL::make_callable("remove_prefix", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_prefix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::make_callable("remove_prefix", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_prefix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<size_t const&>() } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::make_callable("remove_suffix", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_suffix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::make_callable("remove_suffix", [](GL::any::fast_any lhs, size_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>().remove_suffix(rhs); return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", GL::type_of<size_t const&>() } }, Class.this_type | GL::type::Reference));
                Class.add_function(GL::decl_func(&class_t::remove_trailing));
                Class.add_function(GL::decl_func(&class_t::replace));
                Class.add_function(GL::decl_func(&class_t::rfind));
                Class.add_function(GL::decl_func(&class_t::right));
                Class.add_function(GL::decl_func(&class_t::right_of));
                Class.add_function(GL::decl_func(&class_t::right_of_last));
                Class.add_function(GL::decl_func(&class_t::size));
                Class.add_function(GL::make_callable("split", [&Class](class_t const& lhs, class_t const& rhs) -> GL::any::fast_any {
                    auto vec = Class.call("vector<string>", {});
                    for (auto& result : lhs.split(rhs)) {
                        Class.call("push_back", { vec, GL::any::fast_any::instance(result) });
                    }
                    return vec;
                }, GL::function_signature::Constant, {}, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.DetermineType("vector<string>")));
                Class.add_function(GL::make_callable("substr", [](class_t const& lhs, size_t _off, size_t _count) -> class_t { return lhs.substr(_off, _count); }, GL::function_signature::Async | GL::function_signature::Constant, { 0ull, class_t::npos }, { { "lhs", Class.this_type | GL::type::Reference | GL::type::Const}, { "_Off", GL::type_of<size_t const&>() }, { "_Count", GL::type_of<size_t const&>() } }, Class.this_type));
                Class.add_function(GL::decl_func(&class_t::to_lower));                
                Class.add_function(GL::decl_func(&class_t::to_number));
                Class.add_function(GL::decl_func(&class_t::to_upper));

                Class.add_function(GL::make_callable("to_string", [](class_t const& rhs) -> GL::string { return rhs; }));
                Class.add_function(GL::make_callable("to_hash", [](class_t const& rhs) -> size_t { return std::hash<class_t>()(rhs); }));
            }

#if 0
            // map<T0, T1>
            if (1) {
                auto& BaseClass = this->make_class("map");
                BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") }, { "T1", GL::is_template::type<1>("T1") } };

                using impl_t = concurrency::concurrent_unordered_map<size_t, std::pair<GL::any, GL::any>>;
                if (1) {
                    // teach it how to create the generic map
                    GL::type_of<impl_t>().try_update_name("map_impl");
                    auto& AnyMap = BaseClass.make_class(GL::type_of<impl_t>());
                    AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), []() -> GL::shared_ptr<impl_t> { return GL::make_shared<impl_t>(); }, GL::function_signature::Constructor + GL::function_signature::Async));
                    AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), [&AnyMap](impl_t const& rhs) -> GL::shared_ptr<impl_t> {
                        auto out = GL::make_shared<impl_t>();
                        for (auto& x : rhs) {
                            out->insert({ x.first, {
                                [&]() -> GL::any::fast_any {
                                    if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second.first.m_casted_type)) {
                                        return BC->this_m.scope->call(BC->this_m.scope_name, { x.second.first.fast() });
                                    }
                                    return GL::any::fast_any();
                                }(), [&]() {
                                    if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second.second.m_casted_type)) {
                                        return BC->this_m.scope->call(BC->this_m.scope_name, { x.second.second.fast() });
                                    }
                                    return GL::any::fast_any();
                                }()
                            } });
                        }
                        return out;
                        }, GL::function_signature::Constructor + GL::function_signature::Async));
                    AnyMap.GetRoot()->add_function(GL::make_callable("=", [&AnyMap](GL::any::fast_any Lhs, impl_t const& rhs) -> GL::any::fast_any {
                        auto& out = Lhs.cast<impl_t&>();
                        for (auto& x : rhs) {
                            auto& destination = out.get_or_make(x.first, [&]() -> std::pair<GL::any, GL::any> { return {
                                    [&]() -> GL::any::fast_any {
                                        if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second.first.m_casted_type)) {
                                            return BC->this_m.scope->call(BC->this_m.scope_name, { x.second.first.fast() });
                                        }
                                        return GL::any::fast_any();
                                    }(),
                                    [&]() {
                                        if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second.second.m_casted_type)) {
                                            return BC->this_m.scope->call(BC->this_m.scope_name, { x.second.second.fast() });
                                        }
                                        return GL::any::fast_any();
                                    }()
                                };
                            });
                            AnyMap.GetRoot()->call("=", { destination.first.fast(), x.second.first.fast() });
                            AnyMap.GetRoot()->call("=", { destination.second.fast(), x.second.second.fast() });
                        }
                        return Lhs;
                        }, GL::function_signature::Async, {}, { { "lhs", GL::type_of<impl_t&>() }, { "rhs", GL::type_of<impl_t const&>() } }, GL::type_of<impl_t&>()));

                    // to_string and to_hash functions
                    AnyMap.add_function(GL::make_callable("to_string", [](impl_t const& rhs) -> GL::string {
                        GL::string out;
                        for (auto& obj : rhs) {
                            auto& first = obj.second.first;
                            auto& second = obj.second.second;

                            auto first_str = GL::scope::GetCurrentCaller()->call<GL::string>("to_string", { first.fast() });
                            auto second_str = GL::scope::GetCurrentCaller()->call<GL::string>("to_string", { second.fast() });
                            auto this_pair = first_str + ":" + second_str;
                            out = out.add_to_delim(this_pair, ", ");
                        }
                        return "[" + out + "]";
                    }, GL::function_signature::Async | GL::function_signature::Constant));
                    AnyMap.add_function(GL::make_callable("to_hash", [](impl_t const& rhs) -> size_t {
                        size_t out = 0;
                        for (auto& obj : rhs) {
                            auto& first = obj.second.first;
                            auto& second = obj.second.second;

                            auto first_hash = GL::scope::GetCurrentCaller()->call<size_t>("to_hash", { first.fast() });
                            auto second_hash = GL::scope::GetCurrentCaller()->call<size_t>("to_hash", { second.fast() });

                            GL::util::hash(out, first_hash);
                            GL::util::hash(out, second_hash);
                        }
                        return out;
                    }, GL::function_signature::Async | GL::function_signature::Constant));
                }
                // the use of "~" at the start of this member object's name is not arbitrary. This is a special code that means this is an intended-to-be-hidden wrapper for the dynamic_object.
                BaseClass.add_member_object("~impl", GL::type_of<impl_t>());
                BaseClass.add_function(GL::make_callable("[]", [](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();                            
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { rhs });


                        if (auto* p = impl.try_at(hash); p) {
                            return p->second.fast() | GL::type::Reference | GL::type::Const;
                        }
                        else {
                            throw std::range_error("Key was not found in map");
                        }                            
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }, GL::is_template::type<1>("T1") | GL::type::Const | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("[]", [](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { rhs });

                        return impl.get_or_make(hash, [LHS = std::move(lhs), RHS = std::move(rhs)]() -> std::pair<GL::any, GL::any> {
                            GL::any::fast_any obj0_t;
                            GL::any::fast_any obj1_t;
                            if (auto* impl_class = GL::scope::GetCurrentCaller()->GetRoot()->try_find_class(LHS.m_casted_type); impl_class && impl_class->this_m.is_class()) {
                                auto* impl_class_p = dynamic_cast<GL::scope::impl::ClassScope*>(impl_class->this_m.scope);
                                if (impl_class_p->template_types.size() >= 1) {
                                    if (auto* obj_type_class = GL::scope::GetCurrentCaller()->GetRoot()->try_find_class(impl_class_p->template_types[0].second); obj_type_class && obj_type_class->this_m.is_class()) {
                                        auto* obj_type_class_p = dynamic_cast<GL::scope::impl::ClassScope*>(obj_type_class->this_m.scope);
                                        obj0_t = obj_type_class_p->call(obj_type_class_p->this_type.name(), { RHS });
                                    }
                                }
                                if (impl_class_p->template_types.size() >= 2) {
                                    if (auto* obj_type_class = GL::scope::GetCurrentCaller()->GetRoot()->try_find_class(impl_class_p->template_types[1].second); obj_type_class && obj_type_class->this_m.is_class()) {
                                        auto* obj_type_class_p = dynamic_cast<GL::scope::impl::ClassScope*>(obj_type_class->this_m.scope);
                                        obj1_t = obj_type_class_p->call(obj_type_class_p->this_type.name(), {});
                                    }
                                }
                            }
                            return { obj0_t, obj1_t };
                        }).second.fast() + GL::type::Reference;
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }, GL::is_template::type<1>("T1") | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("size", [](GL::any::fast_any lhs) -> size_t {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        return impl.size();
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } }, GL::type_of<size_t>()));
                BaseClass.add_function(GL::make_callable("at", [](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { rhs });
                        if (auto* p = impl.try_at(hash); p) {
                            return p->second.fast() | GL::type::Reference | GL::type::Const;
                        }
                        else {
                            throw std::range_error("Key was not found in map");
                        }
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }, GL::is_template::type<1>("T1") | GL::type::Const | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("at", [](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();                        
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { rhs });
                        if (auto* p = impl.try_at(hash); p) {
                            return p->second.fast() | GL::type::Reference;
                        }
                        else {
                            throw std::range_error("Key was not found in map");
                        }
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }, GL::is_template::type<1>("T1") | GL::type::Reference));
                BaseClass.add_function(GL::make_callable("clear", [](GL::any::fast_any lhs) -> void {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        impl.clear();
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } }));
                BaseClass.add_function(GL::make_callable("erase", [](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { rhs });
                        return impl.erase(hash);
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }));
                BaseClass.add_function(GL::make_callable("erase", [](GL::any::fast_any lhs, GL::any::fast_any rhs, GL::any::fast_any copy) -> bool {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { rhs });
                        std::pair<GL::any, GL::any> temp_copy;
                        if (impl.erase(hash, &temp_copy)) {
                            GL::scope::GetCurrentCaller()->GetRoot()->call("=", { copy, temp_copy.second.fast() });
                            return true;
                        }
                        return false;
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                    }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference }, { "copy", GL::is_template::type<1>("T1") | GL::type::Reference }}));
                BaseClass.add_function(GL::make_callable("insert", [](GL::any::fast_any lhs, GL::any::fast_any key, GL::any::fast_any obj) -> void {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        auto hash = GL::scope::GetCurrentCaller()->GetRoot()->call<size_t>("to_hash", { key });
                        /*auto wrapped_ref = */impl.insert_fast(hash, { key, obj });

                        //auto out = GL::scope::GetCurrentCaller()->call("pair<" + impl_class->template_types[0].second.name() + "," + impl_class->template_types[1].second.name() + ">", {});
                        //auto out_wrapped = out.cast< GL::dynamic_object& >();
                        //*out_wrapped["first"] = wrapped_ref.second.first;
                        //*out_wrapped["second"] = wrapped_ref.second.second;
                        //return out | GL::type::Reference;

                        return;
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "key", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference }, { "obj", GL::is_template::type<1>("T1") | GL::type::Const | GL::type::Reference } }));

                BaseClass.add_function(GL::make_callable("begin", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto new_iterator = impl_class->call("iterator<" + impl_class->this_type.name() + ">", {});
                    impl_class->call("begin", { new_iterator, lhs }); // initializes the base iterator 
                    auto& iterator = new_iterator.cast<GL::dynamic_object&>();
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        iterator["!iter"] = GL::make_shared<GL::any>(impl.begin());
                    }
                    return new_iterator;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }));
                BaseClass.add_function(GL::make_callable("end", [&BaseClass](GL::any::fast_any lhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto new_iterator = impl_class->call("iterator<" + impl_class->this_type.name() + ">", {});
                    impl_class->call("end", { new_iterator, lhs }); // initializes the base iterator 
                    auto& iterator = new_iterator.cast<GL::dynamic_object&>();
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        iterator["!iter"] = GL::make_shared<GL::any>(impl.end());
                    }
                    return new_iterator;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const } }));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    auto& actual_data = iterator["!iter"]->cast<impl_t::iterator>()->second;
                    auto out = GL::scope::GetCurrentCaller()->call("pair<" + impl_class->template_types[0].second.name() + "," + impl_class->template_types[1].second.name() + ">", {});
                    auto out_wrapped = out.cast< GL::dynamic_object& >();
                    *out_wrapped["first"] = actual_data->first;
                    *out_wrapped["second"] = actual_data->second;
                    return out | GL::type::Reference;
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("get", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    auto& actual_data = iterator["!iter"]->cast<impl_t::iterator>()->second;
                    auto out = GL::scope::GetCurrentCaller()->call("pair<" + impl_class->template_types[0].second.name() + "," + impl_class->template_types[1].second.name() + ">", {});
                    auto out_wrapped = out.cast< GL::dynamic_object& >();
                    *out_wrapped["first"] = actual_data->first;
                    *out_wrapped["second"] = actual_data->second;
                    return out | GL::type::Reference | GL::type::Const;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("++", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    // auto* iter_class = GetClass(rhs.m_casted_type);
                    auto& iterator = rhs.cast<GL::dynamic_object&>();
                    iterator["!iter"]->cast<impl_t::iterator>().operator++();                    
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("--", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs) -> void {
                    throw std::runtime_error("Cannot iterate in reverse with this iterator type");
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() } }));
                BaseClass.add_function(GL::make_callable("+", [&BaseClass](GL::any::fast_any lhs, GL::any::fast_any rhs, size_t  const& offset) -> GL::any::fast_any {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto new_iter = impl_class->call("begin", { lhs });
                    auto& old_iterator = rhs.cast<GL::dynamic_object&>();
                    auto& new_iterator = new_iter.cast<GL::dynamic_object&>();
                    while (new_iterator["!iter"]->cast<impl_t::iterator>() != old_iterator["!iter"]->cast<impl_t::iterator>()) {
                        ++new_iterator["!iter"]->cast<impl_t::iterator>();
                    }
                    std::advance(new_iterator["!iter"]->cast<impl_t::iterator>(), offset);
                    return new_iter;
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "rhs", GL::type_of<GL::any::fast_any>() }, { "offset", GL::type_of<size_t const&>() } }));
                BaseClass.add_function(GL::make_callable("==", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["!iter"]->cast<impl_t::iterator>() == iterator2["!iter"]->cast<impl_t::iterator>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("!=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["!iter"]->cast<impl_t::iterator>() != iterator2["!iter"]->cast<impl_t::iterator>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["!iter"]->cast<impl_t::iterator>() > iterator2["!iter"]->cast<impl_t::iterator>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable(">=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["!iter"]->cast<impl_t::iterator>() >= iterator2["!iter"]->cast<impl_t::iterator>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["!iter"]->cast<impl_t::iterator>() < iterator2["!iter"]->cast<impl_t::iterator>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));
                BaseClass.add_function(GL::make_callable("<=", [&BaseClass](GL::any::fast_any parent, GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    auto& iterator1 = lhs.cast<GL::dynamic_object&>();
                    auto& iterator2 = rhs.cast<GL::dynamic_object&>();
                    return iterator1["!iter"]->cast<impl_t::iterator>() <= iterator2["!iter"]->cast<impl_t::iterator>();
                }, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", BaseClass.this_type | GL::type::Reference | GL::type::Const }, { "lhs", GL::type_of<GL::any::fast_any>() }, { "rhs", GL::type_of<GL::any::fast_any>() } }, GL::type_of<bool>()));

                BaseClass.initialize_basic_member_functions();
            }
#endif

#if 0
            // queue<T0>
            if (1) {
                auto& BaseClass = this->make_class("queue");
                BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") } };

                using impl_t = GL::atomic_parallel_queue<GL::any::fast_any>;
                if (1) {
                    // teach it how to create the generic map
                    GL::type_of<impl_t>().try_update_name("queue_impl");
                    auto& AnyMap = BaseClass.make_class(GL::type_of<impl_t>());
                    AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), []() -> GL::shared_ptr<impl_t> { return GL::make_shared<impl_t>(); }, GL::function_signature::Constructor + GL::function_signature::Async));
                    
                    // to_string and to_hash functions
                    AnyMap.add_function(GL::make_callable("to_string", [](impl_t const& rhs) -> GL::string {
                        GL::string out;
                        return out;
                    }, GL::function_signature::Async | GL::function_signature::Constant));
                    AnyMap.add_function(GL::make_callable("to_hash", [](impl_t const& rhs) -> size_t {
                        size_t out = 0;                       
                        return out;
                    }, GL::function_signature::Async | GL::function_signature::Constant));
                }
                // the use of "~" at the start of this member object's name is not arbitrary. This is a special code that means this is an intended-to-be-hidden wrapper for the dynamic_object.
                BaseClass.add_member_object("~impl", GL::type_of<impl_t>());

                BaseClass.add_function(GL::make_callable("size", [](GL::any::fast_any lhs) -> size_t {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        return impl.size();
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } }, GL::type_of<size_t>()));
                BaseClass.add_function(GL::make_callable("try_pop", [](GL::any::fast_any lhs, GL::any::fast_any rhs) -> bool {
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        GL::any::fast_any out;
                        if (impl.try_pop(out)) {
                            GL::scope::GetCurrentCaller()->GetRoot()->call("=", { rhs, out });
                            return true;
                        }
                        else {
                            return false;
                        }
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference }, { "copy", GL::is_template::type<0>("T0") | GL::type::Reference } }));
                BaseClass.add_function(GL::make_callable("push", [](GL::any::fast_any lhs, GL::any::fast_any obj) -> size_t {
                    auto* impl_class = GetClass(lhs.m_casted_type);
                    if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("~impl"); implp && *implp) {
                        auto& impl = (*implp)->cast<impl_t>();
                        return impl.push(obj);
                    }
                    throw std::runtime_error("Could not instantiate the map internals");
                }, GL::function_signature::Async, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "obj", GL::is_template::type<0>("T0") | GL::type::Const | GL::type::Reference } }));
            }
#endif

#if 0
            // GPU-accelerated arrays
            if (1) {
                // initialize the classes and their names
#define add_matrix(TypeT) if (1) { \
                GL::type_of<GPU::matrix<TypeT>>().try_update_name(GL::type_of<TypeT>().name() + "_matrix"); \
                using class_t = GPU::matrix<TypeT>; \
                auto& Class = this->make_class(GL::type_of<class_t>()); \
                }
                add_matrix(char);
                add_matrix(unsigned char);
                add_matrix(int);
                add_matrix(unsigned int);
                add_matrix(long);
                add_matrix(unsigned long);
                add_matrix(float);
#undef add_matrix

                // matrix kernel is always a float-type, so keep it simple.
                if (1) {
                    using class_t = GPU::matrix_kernel<float>;
                    GL::type_of<class_t>().try_update_name("matrix_kernel");
                    auto& Class = this->make_class(GL::type_of<class_t>());
                    Class.add_function(GL::make_callable("to_string", [](class_t const& lhs) -> std::string {
                        if (lhs.mat) return lhs.mat->to_string();                        
                        throw std::runtime_error("kernel was uninitialized");                        
                    }));
                    Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<float> const& matrix) -> class_t { return class_t((GPU::matrix<float>)matrix); }, GL::function_signature::Constructor | GL::function_signature::Async));
                }

                // most matrix functions
#define add_matrix(TypeT) if (1) { \
                using class_t = GPU::matrix<TypeT>; \
                auto& Class = this->make_class(GL::type_of<class_t>()); \
                Class.add_function(GL::make_callable(Class.this_type.name(), []() -> class_t { return class_t(); }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Class.this_type)); \
                this->add_function(GL::make_callable("=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() = rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<char> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<unsigned char> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<int> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<unsigned int> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<long> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<unsigned long> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::make_callable(Class.this_type.name(), [](GPU::matrix<float> const& rhs) { return rhs.cast<typename class_t::type>(); }, GL::function_signature::Constructor)); \
                Class.add_function(GL::decl_func(&class_t::abs)); \
                Class.add_function(GL::decl_func(&class_t::acos)); \
                Class.add_function(GL::decl_func(&class_t::acosh)); \
                Class.add_function(GL::decl_func(&class_t::adjoint)); \
                Class.add_function(GL::decl_func(&class_t::ASCII)); \
                Class.add_function(GL::decl_func(&class_t::asin)); \
                Class.add_function(GL::decl_func(&class_t::asinh)); \
                Class.add_function(GL::decl_func(&class_t::atan)); \
                Class.add_function(GL::decl_func(&class_t::atanh)); \
                Class.add_function(GL::decl_func(&class_t::avg)); \
                Class.add_function(GL::decl_func(&class_t::binomial_search_smallest_gre)); \
                Class.add_function(GL::decl_func(&class_t::ceil)); \
                Class.add_function(GL::decl_func(&class_t::cofactor)); \
                Class.add_function(GL::decl_func(&class_t::constant)); \
                Class.add_function(GL::make_callable("convolve", [](class_t const& parent, GPU::matrix_kernel<float> const& rhs) -> class_t { return parent.convolve(rhs); })); \
                Class.add_function(GL::decl_func(&class_t::cos)); \
                Class.add_function(GL::decl_func(&class_t::cosh)); \
                Class.add_function(GL::decl_func(&class_t::determinant)); \
                Class.add_function(GL::decl_func(&class_t::diagonal)); \
                Class.add_function(GL::decl_func(&class_t::doublesize)); \
                Class.add_function(GL::decl_func(&class_t::exp)); \
                Class.add_function(GL::decl_func(&class_t::exp10)); \
                Class.add_function(GL::decl_func(&class_t::exp2)); \
                Class.add_function(GL::decl_func(&class_t::expm1)); \
                Class.add_function(GL::decl_func(&class_t::floor)); \
                Class.add_function(GL::decl_func(&class_t::fma)); \
                Class.add_function(GL::decl_func(&class_t::grow_by_wrapping)); \
                Class.add_function(GL::make_callable("guassian_kernel", [](unsigned int X, unsigned int Y) { return GPU::matrix<float>::guassian_kernel(X, Y); })); \
                Class.add_function(GL::decl_func(&class_t::halfsize)); \
                Class.add_function(GL::decl_func(&class_t::identity)); \
                Class.add_function(GL::decl_func(&class_t::inverse)); \
                Class.add_function(GL::decl_func(&class_t::is_colinear)); \
                Class.add_function(GL::decl_func(&class_t::join)); \
                Class.add_function(GL::decl_func(&class_t::lgamma)); \
                Class.add_function(GL::decl_func(&class_t::linear)); \
                Class.add_function(GL::decl_func(&class_t::log)); \
                Class.add_function(GL::decl_func(&class_t::log10)); \
                Class.add_function(GL::decl_func(&class_t::log1p)); \
                Class.add_function(GL::decl_func(&class_t::log2)); \
                Class.add_function(GL::decl_func(&class_t::make_square)); \
                Class.add_function(GL::decl_func(&class_t::matrix_multiply)); \
                Class.add_function(GL::make_callable("max", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.max(rhs); })); \
                Class.add_function(GL::make_callable("max", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.max(rhs); })); \
                Class.add_function(GL::make_callable("min", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.min(rhs); })); \
                Class.add_function(GL::make_callable("min", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.min(rhs); })); \
                Class.add_function(GL::make_callable("mod", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.mod(rhs); })); \
                Class.add_function(GL::make_callable("mod", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.mod(rhs); })); \
                Class.add_function(GL::make_callable("pow", [](class_t const& parent, class_t const& rhs) -> class_t { return parent.pow(rhs); })); \
                Class.add_function(GL::make_callable("pow", [](class_t const& parent, typename class_t::type const& rhs) -> class_t { return parent.pow(rhs); })); \
                Class.add_function(GL::make_callable("pown", [](class_t const& parent, GPU::matrix<int> const& rhs) -> class_t { return parent.pown(rhs); })); \
                Class.add_function(GL::make_callable("pown", [](class_t const& parent, int const& rhs) -> class_t { return parent.pown(rhs); })); \
                Class.add_function(GL::decl_func(&class_t::quadruplesize)); \
                Class.add_function(GL::decl_func(&class_t::quartersize)); \
                Class.add_function(GL::decl_func(&class_t::random)); \
                Class.add_function(GL::decl_func(&class_t::random_between)); \
                Class.add_function(GL::decl_func(&class_t::resample)); \
                Class.add_function(GL::make_callable("resize", [](class_t const& lhs, unsigned int x, unsigned int y, unsigned int z) { return lhs.resize(x, y, z); })); \
                Class.add_function(GL::decl_func(&class_t::resize_stretch)); \
                Class.add_function(GL::decl_func(&class_t::round)); \
                Class.add_function(GL::decl_func(&class_t::row)); \
                Class.add_function(GL::decl_func(&class_t::sin)); \
                Class.add_function(GL::decl_func(&class_t::sinh)); \
                Class.add_function(GL::make_callable("size", [](class_t const& lhs) { return lhs.size(); })); \
                Class.add_function(GL::make_callable("size", [](class_t const& lhs, unsigned int d) { return lhs.size(d); })); \
                Class.add_function(GL::decl_func(&class_t::sqrt)); \
                Class.add_function(GL::decl_func(&class_t::subsample_1D)); \
                Class.add_function(GL::decl_func(&class_t::subsample_pat)); \
                Class.add_function(GL::decl_func(&class_t::sum)); \
                Class.add_function(GL::decl_func(&class_t::tan)); \
                Class.add_function(GL::decl_func(&class_t::tanh)); \
                Class.add_function(GL::decl_func(&class_t::transpose)); \
                Class.add_function(GL::make_callable("to_string", [](class_t const& lhs) -> GL::string { return lhs.to_string({}, true); })); \
                Class.add_function(GL::make_callable("to_hash", [](class_t const& lhs) -> size_t { return std::hash<GL::string>()(lhs.to_string({}, true)); })); \
                this->add_function(GL::make_callable("[]", [](class_t const& lhs, unsigned int x, unsigned int y, unsigned int z) -> typename class_t::type { return lhs.operator()(x,y,z); })); \
                this->add_function(GL::make_callable("[]", [](class_t const& lhs, unsigned int x) -> typename class_t::type { return lhs.operator[](x); })); \
                this->add_function(GL::make_callable("+", [](class_t const& lhs, class_t const& rhs) { return lhs + rhs; })); \
                this->add_function(GL::make_callable("-", [](class_t const& lhs, class_t const& rhs) { return lhs - rhs; })); \
                this->add_function(GL::make_callable("*", [](class_t const& lhs, class_t const& rhs) { return lhs * rhs; })); \
                this->add_function(GL::make_callable("/", [](class_t const& lhs, class_t const& rhs) { return lhs / rhs; })); \
                this->add_function(GL::make_callable("+", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs + rhs; })); \
                this->add_function(GL::make_callable("-", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs - rhs; })); \
                this->add_function(GL::make_callable("*", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs * rhs; })); \
                this->add_function(GL::make_callable("/", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs / rhs; })); \
                this->add_function(GL::make_callable("+=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() += rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("-=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() -= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("*=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() *= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("/=", [](GL::any::fast_any lhs, class_t const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() /= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("+=", [](GL::any::fast_any lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() += rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("-=", [](GL::any::fast_any lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() -= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("*=", [](GL::any::fast_any lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() *= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("/=", [](GL::any::fast_any lhs, typename class_t::type const& rhs) -> GL::any::fast_any { lhs.cast<class_t&>() /= rhs; return lhs; }, 0, {}, { { "lhs", Class.this_type | GL::type::Reference }, { "rhs", Class.this_type | GL::type::Reference | GL::type::Const } }, Class.this_type | GL::type::Reference)); \
                this->add_function(GL::make_callable("%", [](class_t const& lhs, class_t const& rhs) { return lhs % rhs; })); \
                this->add_function(GL::make_callable("%", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs % rhs; })); \
                this->add_function(GL::make_callable("&&", [](class_t const& lhs, class_t const& rhs) { return lhs && rhs; })); \
                this->add_function(GL::make_callable("&&", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs && rhs; })); \
                this->add_function(GL::make_callable("||", [](class_t const& lhs, class_t const& rhs) { return lhs || rhs; })); \
                this->add_function(GL::make_callable("||", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs || rhs; })); \
                this->add_function(GL::make_callable("!", [](class_t const& lhs) { return !lhs; })); \
                this->add_function(GL::make_callable("==", [](class_t const& lhs, class_t const& rhs) { return lhs == rhs; })); \
                this->add_function(GL::make_callable("!=", [](class_t const& lhs, class_t const& rhs) { return lhs != rhs; })); \
                this->add_function(GL::make_callable(">", [](class_t const& lhs, class_t const& rhs) { return lhs > rhs; })); \
                this->add_function(GL::make_callable(">=", [](class_t const& lhs, class_t const& rhs) { return lhs >= rhs; })); \
                this->add_function(GL::make_callable("<", [](class_t const& lhs, class_t const& rhs) { return lhs < rhs; })); \
                this->add_function(GL::make_callable("<=", [](class_t const& lhs, class_t const& rhs) { return lhs <= rhs; })); \
                this->add_function(GL::make_callable("==", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs == rhs; })); \
                this->add_function(GL::make_callable("!=", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs != rhs; })); \
                this->add_function(GL::make_callable(">", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs > rhs; })); \
                this->add_function(GL::make_callable(">=", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs >= rhs; })); \
                this->add_function(GL::make_callable("<", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs < rhs; })); \
                this->add_function(GL::make_callable("<=", [](class_t const& lhs, typename class_t::type const& rhs) { return lhs <= rhs; })); \
                this->add_function(GL::make_callable("read", [](class_t const& lhs) -> typename class_t::reader { return lhs.read(); })); \
                this->add_function(GL::make_callable("write", [](class_t& lhs) -> typename class_t::writer { return lhs.write(false); })); \
                }
                add_matrix(char);
                add_matrix(unsigned char);
                add_matrix(int);
                add_matrix(unsigned int);
                add_matrix(long);
                add_matrix(unsigned long);
                add_matrix(float);
#undef add_matrix

                // writer
#define add_matrix(TypeT) if (1) { \
                using class_t = typename GPU::matrix<TypeT>::writer; \
                GL::type_of<class_t>().try_update_name(GL::type_of<TypeT>().name() + "_matrix_writer"); \
                auto& Class = this->make_class(GL::type_of<class_t>()); \
                this->make_class(GL::type_of<bool>()).add_function(GL::make_callable(GL::type_of<bool>().name(), [](class_t const& lhs) -> bool { return (bool)lhs; })); \
                this->add_function(GL::make_callable("[]", [](GL::any::fast_any lhs, unsigned int x, unsigned int y, unsigned int z) -> GL::any::fast_any { return GL::any::fast_any::wrap_member(lhs, lhs.cast<class_t&>()(x,y,z)); }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "parent", GL::type_of<class_t const&>() }, { "x", GL::type_of<unsigned int const&>() }, { "y", GL::type_of<unsigned int const&>() }, { "z", GL::type_of<unsigned int const&>() } }, GL::type_of<TypeT &>() )); \
                this->add_function(GL::make_callable("[]", [](GL::any::fast_any lhs, unsigned int x) -> GL::any::fast_any { return GL::any::fast_any::wrap_member(lhs, lhs.cast<class_t&>()[x]); }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "parent", GL::type_of<class_t const&>() }, { "x", GL::type_of<unsigned int const&>() } }, GL::type_of<TypeT &>() )); \
                }
                add_matrix(char);
                add_matrix(unsigned char);
                add_matrix(int);
                add_matrix(unsigned int);
                add_matrix(long);
                add_matrix(unsigned long);
                add_matrix(float);
#undef add_matrix

                // reader
#define add_matrix(TypeT) if (1) { \
                using class_t = typename GPU::matrix<TypeT>::reader; \
                GL::type_of<class_t>().try_update_name(GL::type_of<TypeT>().name() + "_matrix_reader"); \
                auto& Class = this->make_class(GL::type_of<class_t>()); \
                this->make_class(GL::type_of<bool>()).add_function(GL::make_callable(GL::type_of<bool>().name(), [](class_t const& lhs) -> bool { return (bool)lhs; })); \
                this->add_function(GL::make_callable("[]", [](GL::any::fast_any lhs, unsigned int x, unsigned int y, unsigned int z) -> GL::any::fast_any { return GL::any::fast_any::wrap_member(lhs, lhs.cast<class_t&>()(x,y,z)); }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "parent", GL::type_of<class_t const&>() }, { "x", GL::type_of<unsigned int const&>() }, { "y", GL::type_of<unsigned int const&>() }, { "z", GL::type_of<unsigned int const&>() } }, GL::type_of<TypeT const &>() )); \
                this->add_function(GL::make_callable("[]", [](GL::any::fast_any lhs, unsigned int x) -> GL::any::fast_any { return GL::any::fast_any::wrap_member(lhs, lhs.cast<class_t&>()[x]); }, GL::function_signature::Constant | GL::function_signature::Async, {}, { { "parent", GL::type_of<class_t const&>() }, { "x", GL::type_of<unsigned int const&>() } }, GL::type_of<TypeT const &>() )); \
                }
                add_matrix(char);
                add_matrix(unsigned char);
                add_matrix(int);
                add_matrix(unsigned int);
                add_matrix(long);
                add_matrix(unsigned long);
                add_matrix(float);
#undef add_matrix

            }
#endif
		};
        void impl::RootScope::preload_conversions() {
            for (auto& _type : all_convertable_types()) {
                (void)this->get_converters().try_get_converter(_type, _type);
            }
        };
        impl::Breadcrumb* impl::RootScope::try_find_class(GL::type this_t) const {
            if (auto search = this->classes.find(this_t.get_base_hash()), e = this->classes.end(); search != e) {
                return search->second;
            }
            else {
                return nullptr;
            }
        };

        // Get the nearest calling scope for the current thread.
        impl::BasicScope* GetCurrentCaller() {
            return impl::BasicScope::GetCurrentCaller();
        };
        // attempts to find the scripting class for the provided type from the nearest script scope for the current thread.
        impl::ClassScope* GetClass(GL::type const& rhs) {
            if (auto* caller = GetCurrentCaller(); caller) {
                if (auto* BC = caller->GetRoot()->try_find_class(rhs); BC) {
                    return dynamic_cast<impl::ClassScope*>(BC->this_m.scope);
                }
            }
            return nullptr;
        };
	}
}