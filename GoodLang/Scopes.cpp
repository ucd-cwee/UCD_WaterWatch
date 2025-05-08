#pragma once 
#include "Scopes.h"
#include <boost/unordered_set.hpp>

namespace GoodLang {
	// emplace a function, if not already exists, using the provided params
	FunctionsMap::TupleType* FunctionsMap::emplace(std::string_view const& name, GoodLang::ParamTypes const& params, GoodLang::Function const& func) {
		size_t hash{ 0 };
		if (func.m_function && (name.size() > 0)) {
			MapType& map = FirstCharToFunctionNameMap[CharToIndex(name[0])]; // try to reduce conflict by splitting on the first letter

			GoodLang::details::hash_combine(hash, name);

			auto& F = map.first.get_or_make(hash, [&]() -> auto* {
				return alloc1.Alloc();
			});
			bool previouslyExisted = false;
			auto& F2 = F->first.get_or_make(params.hash(), [&]() -> TupleType* {
				return alloc2.Alloc(
					std::string(name),
					params,
					func,
					nullptr,
					true
				);
				}, &previouslyExisted);
			if (!previouslyExisted) InterlockedIncrementNoFence(&count);
			return &*F2;
		}
		return nullptr;
	};
	// emplace a function, if not already exists
	FunctionsMap::TupleType* FunctionsMap::emplace(std::string_view const& name, GoodLang::Function const& func) {
		size_t hash{ 0 };
		if (func.m_function && (name.size() > 0)) {
			MapType& map = FirstCharToFunctionNameMap[CharToIndex(name[0])]; // try to reduce conflict by splitting on the first letter

			GoodLang::details::hash_combine(hash, name);

			auto& F = map.first.get_or_make(hash, [&]() -> auto* {
				return alloc1.Alloc();
			});
			bool previouslyExisted = false;
			auto& F2 = F->first.get_or_make(func.m_function->Arguments().Types().hash(), [&]() -> TupleType* {
				return alloc2.Alloc(
					std::string(name),
					func.m_function->Arguments().Types(),
					func,
					nullptr,
					true
				);
				}, &previouslyExisted);
			if (!previouslyExisted) InterlockedIncrementNoFence(&count);
			return &*F2;
		}
		return nullptr;
	};
	// emplace an object, if not already exists
	FunctionsMap::TupleType* FunctionsMap::emplace(std::string_view const& name, std::shared_ptr<GoodLang::Any> const& obj) {
		size_t hash{ 0 };
		if (name.size() > 0) {
			MapType& map = FirstCharToFunctionNameMap[CharToIndex(name[0])]; // try to reduce conflict by splitting on the first letter

			GoodLang::details::hash_combine(hash, name);

			auto& F = map.first.get_or_make(hash, [&]() -> auto* {
				return alloc1.Alloc();
			});
			bool previouslyExisted = false;
			auto& F2 = F->first.get_or_make(ParamTypes().hash(), [&]() -> TupleType* {
				return alloc2.Alloc(
					std::string(name),
					ParamTypes{},
					Function{},
					obj,
					false
				);
				}, &previouslyExisted);
			if (!previouslyExisted) InterlockedIncrementNoFence(&count);
			return &*F2;
		}
		return nullptr;
	};
	// Get the tuple (function, likely) that exists at this name and param types
	FunctionsMap::TupleType* FunctionsMap::at(std::string_view const& name, GoodLang::ParamTypes const& params) const {
		size_t hash{ 0 };
		if ((this->size() > 0) && (name.size() > 0)) {
			const MapType& map = FirstCharToFunctionNameMap[CharToIndex(name[0])]; // try to reduce conflict by splitting on the first letter
			//if (map.first.size() > 0) {
			GoodLang::details::hash_combine(hash, name);
			if (auto* F = map.first.try_at(hash, const_cast<long&>(map.second))) {
				//if ((*F)->first.size() > 0) {
				if (auto* P = (*F)->first.try_at(params.hash(), (*F)->second)) {
					return *P;
				}
				//}
			}
			//}
		}
		return nullptr;
	};
	// Get the tuple (function, likely) that exists at this name and param types
	FunctionsMap::TupleType* FunctionsMap::operator()(std::string_view const& name, GoodLang::ParamTypes const& params) const {
		return at(name, params);
	};
	// Get the tuple (object, likely) that exists at this name and with empty params
	FunctionsMap::TupleType* FunctionsMap::at(std::string_view const& name) const {
		size_t hash{ 0 };
		if ((this->size() > 0) && (name.size() > 0)) {
			const MapType& map = FirstCharToFunctionNameMap[CharToIndex(name[0])]; // try to reduce conflict by splitting on the first letter
			//if (map.first.size() > 0) {
			GoodLang::details::hash_combine(hash, name);
			if (auto* F = map.first.try_at(hash, const_cast<long&>(map.second))) {
				//if ((*F)->first.size() > 0) {
				if (auto* P = (*F)->first.try_at(ParamTypes().hash(), (*F)->second)) {
					return *P;
				}
				//}
			}
			//}
		}
		return nullptr;
	};
	// Get the tuple (object, likely) that exists at this name and with empty params
	FunctionsMap::TupleType* FunctionsMap::operator()(std::string_view const& name) const {
		return at(name);
	};
	// Allows looping over all contained Tuples with the requested name. 
	FunctionsMap::iterator FunctionsMap::find(std::string_view const& name) {
		auto iter = end();
		if (this->size() > 0) {
			if (name.size() > 0) {
				iter.state.outtermost_index = CharToIndex(name[0]);
				iter.state.outtermost_index_max = iter.state.outtermost_index + 1;

				size_t hash{ 0 };
				GoodLang::details::hash_combine(hash, name);

				iter.state.middle_iter = FirstCharToFunctionNameMap[iter.state.outtermost_index].first.begin(); // also does a weak_lock on the map
				long index = -1;
				if (auto* ptr = FirstCharToFunctionNameMap[iter.state.outtermost_index].first.try_at(hash, index)) {
					if ((*ptr)->first.size() > 0) {
						iter.state.middle_index = index;
						iter.state.middle_index_max = index + 1;
						iter.state.middle_iter += index; // continues to hold the weak_lock till the iterator is destroyed. 
						iter.state.final_iter = (*iter.state.middle_iter->second)->first.begin();
						iter.state.final_index = 0;
						iter.state.final_index_max = (*ptr)->first.size();
						return iter;
					}
				}
				iter.state.outtermost_index = 0;
				iter.state.outtermost_index_max = 0;
				iter.state.middle_iter = {}; // release the weak_lock
				iter.state.middle_index = 0;
				iter.state.middle_index_max = 0;
				iter.state.final_iter = {};
				iter.state.final_index = 0;
				iter.state.final_index_max = 0;
			}
		}
		return iter;
	};
	FunctionsMap::TupleType* FunctionsMap::BuildMatch(std::string_view const& Name, ParamTypes const& params, TypeConverter& m_typeConverters, bool AllowTemplateInstantiation, bool AllowTypeConversion) {
		// Note: we cannot in good faith match a function to a template parameter -- we could not predict what function would return.
		if ((this->size() == 0) || params.IsTemplate()) {
			return nullptr;
			// throw std::runtime_error("Cannot build a matching function with template or empty types. This is meant to be called with KNOWN types");
		};

		// Check that we don't already have an exact match for these params
		if (auto* tuple = at(Name, params)) { // this would have included objects or functions with no parameters
			return tuple;
		};

		if (1) {
			// Three sorted groups of candidates. 
			// Group 1 = exact matches, Group 2 = type conversions, Group 3 = template functions
			// First ordered by the number of parameters (more matches are preferred over fewer matches)
			// Then ordered by preference for exact matches, then matches with type conversions, then (finally) template functions.
			thread_local static std::map< size_t, std::array<std::map<double, Function, std::less<double>>, 3>, std::greater<size_t>>
				candidates{};
			candidates.clear(); // clear from the last search

			/* Create candidates. */ {
				std::vector<std::shared_ptr<Type_Info>> paramTypes;
				for (auto& x : params) paramTypes.push_back(x.lock());

				if (Name.size() > 0) {
					double conversionCost;
					for (auto Iter = this->find(Name), End = this->end(); Iter != End; ++Iter) {
						auto& function = Iter->get<2>();

						if (!function.m_function) continue;
						if (function.m_isCached) continue; // ignoring pre-cached functions. Only interested in "true" functions. 
						bool isTemplateFunc = function.m_function->GetSignature().IsTemplate();
						bool isExplicitFunc = function.m_isEplicit;

						// NOTE: if the hash for Params and function.second.second->m_function-> Arguments().Types() match, doesn't that mean the types inside exactly match?
						if ((0 == params.size()) && (0 == function.m_function->Arguments().Types().size())) {
							// both have a matching size...
							conversionCost = 0;
						}
						else if (params.hash() == function.m_function->Arguments().Types().hash()) {
							conversionCost = 0;
						}
						else {
							conversionCost = function.m_function->conversion_cost_fast(paramTypes, m_typeConverters);
						}

						if (conversionCost >= details::TypeConversionWorstCaseCost) continue;

						// try to early exit...
						if (params.size() == function.m_function->Arguments().size()) {
							if (!isTemplateFunc) {
								if (conversionCost == 0) {
									Function FunctionToCache{ function.m_function };
									FunctionToCache.m_isCached = true;
									// if someone already beat us to it, it should return the "current" value
									Iter = this->end();
									return this->emplace(Name, params, FunctionToCache);
								}
							}
						}

						if (isTemplateFunc) {
							if (AllowTemplateInstantiation) {
								candidates[function.m_function->NumArguments()][2][conversionCost] = function;
							}
						}
						else {
							if (conversionCost == 0) {
								candidates[function.m_function->NumArguments()][0][conversionCost] = function;
							}
							else if (AllowTypeConversion && !isExplicitFunc) {
								candidates[function.m_function->NumArguments()][1][conversionCost] = function;
							}
						}
					}
				}
			}

			// Get the "cheapest" or fastest conversion option available at this scope, with the largest number of arguments, in order of group (e.g. preference).
			for (auto& numParams : candidates) {
				for (auto& preference_order : numParams.second) {
					for (auto& candidate : preference_order) {
						if (candidate.first >= details::TypeConversionWorstCaseCost) continue; // this shouldn't happen, but just in case

						Function FunctionToCache{ candidate.second.m_function };
						FunctionToCache.m_isCached = true;
						// if someone already beat us to it, it should return the "current" value
						return this->emplace(Name, params, FunctionToCache);
					}
				}
			}
		}

		return nullptr;
	};
	Any FunctionsMap::Call(std::string_view const& Name, std::vector<Any> const& params, TypeConverter& m_typeConverters) {
		if (this->size() > 0) {
			if (auto* f = BuildMatch(Name, ParamTypes(params), m_typeConverters)) {
				if (f->get<4>()) {
					// function
					if (f->get<2>().m_function) {
						return f->get<2>().m_function->operator()(params, m_typeConverters);
					}
				}
				else {
					// object
					return f->get<3>();
				}
			}
		}

		std::string params_str;
		for (auto& p : params) {
			std::string className = p.TypeName(); {
				//if (auto classPtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(p.Type()))) {
				//	className = classPtr->GetName();
				//}
			}

			if (params_str.empty()) {
				params_str += className;
			}
			else {
				params_str += ", ";
				params_str += className;
			}
		}

		throw exception::not_found_error(
			std::string("`") + GoodLang::ToString(Name) + std::string("`(") + params_str.c_str() + std::string(")")
		);
	};







	// try and find the object with the requested key.
	std::shared_ptr<Any> Scope::GetObj(std::string const& name) const {
		if (p_objects.DataExists()) {
			auto f = p_objects.data->find(name);
			if (f != p_objects.data->end()) {
				return f->second;
			}
		}
		return nullptr;
	};
	// Returns true if successful. Returns false is replaceIfExisting==false and the object already existed on the Scope.
	bool Scope::AddObj(std::string const& name, std::shared_ptr<Any> const& obj, bool updateObjectTree) {
		p_objects.EnsureDataExists();

		p_objects.data->insert(std::pair<std::string, std::shared_ptr<Any>>{ name, obj });
		// p_objects[name] = obj;
		if (updateObjectTree) this->RecordObject(name, obj);
		return true;
	};

	bool Scope::AddUsing(std::weak_ptr<Namespace> namespacePtr) {
		if (auto p = std::dynamic_pointer_cast<Scope>(namespacePtr.lock())) {
			bool existedAlready = false;

			p_using.EnsureDataExists();

			(void)p_using.data->get_or_make(Hasher()(p), [&]() -> std::weak_ptr<Namespace> {
				return namespacePtr;
				}, &existedAlready);
			if (!existedAlready) {
				if (p->IsNamespace()) {
					(void)RecordUsing(std::dynamic_pointer_cast<Namespace>(p));
				}
				is_basic_scope = false;
				return true;
			}
		}
		return false;
	};
	bool Scope::AddChild(std::shared_ptr<Namespace> NamespacePtr) {
		if (auto p = std::dynamic_pointer_cast<Scope>(NamespacePtr)) {
			auto name = p->GetName();

			auto ptr = p_children.get_or_insert(name, NamespacePtr);
			if (*ptr == NamespacePtr) {
				// it was inserted! 
				if (p->IsClass()) {
					(void)RecordClass(std::dynamic_pointer_cast<Class>(p));
				}
				is_basic_scope = false;
				return true;
			}
			else {
				// it was not inserted!
				return false;
			}
		}
		return false;
	};

	bool Scope::TryFindNearestScopeWhere(
		std::shared_ptr<Scope>& bestMatch,
		std::function<bool(std::shared_ptr<Scope> const&)> const& func,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
		auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.count(selfPtr) >= 1) { return false; }
		checkedAll.emplace(selfPtr);

		// test myself			
		if (!(checkedSelf.count(selfPtr) >= 1)) {
			checkedSelf.emplace(selfPtr);
			if (1) {
				if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
					if (func(p)) {
						bestMatch = p;
						return true;
					}
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}

			}
		}
		// test all of my parents
		auto parentPtr = this->p_parent.lock();
		while (parentPtr) {
			if (!(checkedSelf.count(parentPtr) >= 1)) {
				checkedSelf.emplace(parentPtr);
				if (1) {
					if (auto p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}
			else {
				break; // we've checked this before! Quick, get out. 
			}
			parentPtr = parentPtr->p_parent.lock();
		}

		// Test my children themselves.
		for (auto& childNamespace : this->p_children) {
			if (childNamespace.second) {
				auto ptr = std::dynamic_pointer_cast<Scope>(childNamespace.second);
				if (!(checkedSelf.count(ptr) >= 1)) {
					checkedSelf.emplace(ptr);
					if (auto p = std::dynamic_pointer_cast<Scope>(ptr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};
	bool Scope::TryFindNearestNamespaceWhere(
		std::shared_ptr<Namespace>& bestMatch,
		std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
		auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.count(selfPtr) >= 1) { return false; }
		checkedAll.emplace(selfPtr);

		// test myself			
		if (!(checkedSelf.count(selfPtr) >= 1)) {
			checkedSelf.emplace(selfPtr);
			if (this->IsNamespace()) {
				if (auto p = std::dynamic_pointer_cast<Namespace>(selfPtr)) {
					if (func(p)) {
						bestMatch = p;
						return true;
					}
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}
		// test all of my parents
		auto parentPtr = this->p_parent.lock();
		while (parentPtr) {
			if (!(checkedSelf.count(parentPtr) >= 1)) {
				checkedSelf.emplace(parentPtr);
				if (parentPtr->IsNamespace()) {
					if (auto p = std::dynamic_pointer_cast<Namespace>(parentPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}
			else {
				break; // we've checked this before! Quick, get out. 
			}
			parentPtr = parentPtr->p_parent.lock();
		}

		// Test my children themselves.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!(checkedSelf.count(ptr) >= 1)) {
					checkedSelf.emplace(ptr);
					if (ptr->IsNamespace()) {
						if (auto p = std::dynamic_pointer_cast<Namespace>(ptr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};	
	Functions& Scope::GetFunctions() const {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->GetFunctions();
		}
		else {
			throw std::runtime_error("Cannot find functions map");
		}
	};
	std::shared_ptr< Functions::FunctionSort > Scope::GetFunctions(std::string const& name) const {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->GetFunctions(name);
		}
		else {
			return nullptr;
		}
	};
	Proxy_Function Scope::GetFunction(std::string const& name, std::vector<Any> const& params) {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->GetFunction(name, params);
		}
		else {
			return {};
		}
	};
	Proxy_Function Scope::GetFunction(std::string const& name, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree) {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->GetFunction(name, params, Params, tree);
		}
		else {
			return {};
		}
	};
	Proxy_Function Scope::GetFunction(std::string const& name, ParamTypes& params, TypeConverter& tree) {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->GetFunction(name, params, tree);
		}
		else {
			return {};
		}
	};
	std::shared_ptr<Scope> Scope::FindNearestScopeWhere(std::function<bool(std::shared_ptr<Scope> const&)> const& func) const {
		std::shared_ptr<Scope> out;
		if (TryFindNearestScopeWhere(out, func)) {
			return out;
		}
		else {
			return nullptr;
		}
	};
	std::shared_ptr<Namespace> Scope::FindNearestNamespaceWhere(std::function<bool(std::shared_ptr<Namespace> const&)> const& func) const {
		std::shared_ptr<Namespace> out;
		if (TryFindNearestNamespaceWhere(out, func)) {
			return out;
		}
		else {
			return nullptr;
		}
	};
	std::shared_ptr<Namespace> Scope::FindNamespace(std::string QualifiedOrUnqualifiedNamespaceName) const {
#if 1
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(QualifiedOrUnqualifiedNamespaceName);
		if (QualifiedOrUnqualifiedNamespaceName == "" || QualifiedOrUnqualifiedNamespaceName == "::") { return std::dynamic_pointer_cast<Namespace>(this->GetLibrary()); }

		std::shared_ptr<Namespace> out;

#ifdef useCachedData
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (this->IsBasicScope()) {
			if (auto p = this->p_parent.lock()) {
				return p->FindNamespace(QualifiedOrUnqualifiedNamespaceName);
			}
		}

		auto treeV = this->GetTypeConverterTreeVersion();
		if (TryGetCached<1>(treeV, out, QualifiedOrUnqualifiedNamespaceName)) {
			return out;
		}
#endif
		auto ScopedQualifiedOrUnqualifiedNamespaceName = "::" + QualifiedOrUnqualifiedNamespaceName;
		long long len = ScopedQualifiedOrUnqualifiedNamespaceName.length();
		if (TryFindNearestNamespaceWhere(out, [&len, tryFind = ScopedQualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace> const& namespacePtr)->bool {
			std::string const& qualifiedName = std::dynamic_pointer_cast<Scope>(namespacePtr)->GetQualifiedNamespace();
			if ((qualifiedName.size() >= 2) && (qualifiedName.substr(qualifiedName.length() - 2) == "::")) {
				// remove "::" from end
				std::string QualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2);
				while (QualifiedName.size() >= 2 && (QualifiedName.substr(QualifiedName.length() - 2) == "::")) QualifiedName = QualifiedName.substr(0, QualifiedName.length() - 2);

				long long QualifiedNameLen = QualifiedName.length();
				auto F = QualifiedName.find(tryFind);
				if ((F != std::string::npos) && (F == (QualifiedNameLen - len))) return true;
				return false;
			}
			else {
				long long qualifiedNameLen = qualifiedName.length();
				auto F = qualifiedName.find(tryFind);
				if ((F != std::string::npos) && (F == (qualifiedNameLen - len))) return true;
				return false;
			}
			})) {
#ifdef useCachedData
			InsertCached<1>(treeV, out, QualifiedOrUnqualifiedNamespaceName);
#endif
			return out;
		}
		return nullptr;
#else
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };

		fixNamespace(QualifiedOrUnqualifiedNamespaceName);

		if (QualifiedOrUnqualifiedNamespaceName == "" || QualifiedOrUnqualifiedNamespaceName == "::") { return std::dynamic_pointer_cast<Namespace>(this->GetLibrary()); }

		std::shared_ptr<Namespace> out;

#ifdef useCachedData
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (this->IsBasicScope()) {
			if (auto p = this->p_parent.lock()) {
				return p->FindNamespace(QualifiedOrUnqualifiedNamespaceName);
			}
		}

		auto treeV = this->GetTypeConverterTreeVersion();
		if (TryGetCached<1>(treeV, out, QualifiedOrUnqualifiedNamespaceName)) {
			return out;
		}
#endif

		long long len = QualifiedOrUnqualifiedNamespaceName.length();
		if (TryFindNearestNamespaceWhere(out, [&len, tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace> const& namespacePtr)->bool {
			std::string const& qualifiedName = std::dynamic_pointer_cast<Scope>(namespacePtr)->GetQualifiedNamespace();
			if ((qualifiedName.size() >= 2) && (qualifiedName.substr(qualifiedName.length() - 2) == "::")) {
				// remove "::" from end
				std::string QualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2);
				while (QualifiedName.size() >= 2 && (QualifiedName.substr(QualifiedName.length() - 2) == "::")) QualifiedName = QualifiedName.substr(0, QualifiedName.length() - 2);

				long long QualifiedNameLen = QualifiedName.length();
				auto F = QualifiedName.find(tryFind);
				if ((F != std::string::npos) && (F == (QualifiedNameLen - len))) return true;
				return false;
			}
			else {
				long long qualifiedNameLen = qualifiedName.length();
				auto F = qualifiedName.find(tryFind);
				if ((F != std::string::npos) && (F == (qualifiedNameLen - len))) return true;
				return false;
			}
			})) {
#ifdef useCachedData
			InsertCached<1>(treeV, out, QualifiedOrUnqualifiedNamespaceName);
#endif
			return out;
		}
		return nullptr;

#endif






	};
	std::shared_ptr<Class> Scope::FindClass(std::string const& QualifiedOrUnqualifiedNamespaceName) const {
		return std::dynamic_pointer_cast<Class>(FindNamespace(QualifiedOrUnqualifiedNamespaceName));
	};
	std::shared_ptr<Class> Scope::FindClass(std::weak_ptr<Type_Info> typeInfo) const {
		if (auto p = typeInfo.lock()) typeInfo = p->MakeBase(); // revert to base to help with searching

		std::shared_ptr<Namespace> out;

#ifdef useCachedData
		if (this->IsBasicScope()) {
			if (auto p = this->p_parent.lock()) {
				return p->FindClass(typeInfo);
			}
		}

		auto treeV = this->GetTypeConverterTreeVersion();
		{
			std::shared_ptr<Class> out2;
			if (TryGetCached<2>(treeV, out2, typeInfo)) {
				return out2;
			}
		}
#endif

		if (TryFindNearestNamespaceWhere(out, [tryFind = typeInfo](std::shared_ptr<Namespace> const& namespacePtr)->bool {
			if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
				if (ptr->IsClass()) {
					if (tryFind == ptr->GetClassType()) {
						return true;
					}
				}
			}
			return false;
			})) {
#ifdef useCachedData
			InsertCached<2>(treeV, std::dynamic_pointer_cast<Class>(out), typeInfo);
#endif				
			return std::dynamic_pointer_cast<Class>(out);
		}
		else {
#ifdef useCachedData
			InsertCached<2>(treeV, nullptr, typeInfo);
#endif				
			return nullptr;
		}
	};

	std::shared_ptr<Scope>  Scope::FindScopeWithObjImpl(std::string const& objName, std::shared_ptr<Any>* found_obj) const {
#ifdef useCachedData
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (this->IsBasicScope()) {

			if ((!this->p_objects.DataExists()) || (this->p_objects.data->size() == 0)) {
				if (auto p = this->p_parent.lock()) {
					return p->FindScopeWithObjImpl(objName, found_obj);
				}
			}
			else {
				if (auto objFound = GetObj(objName)) {
					if (found_obj) {
						*found_obj = objFound;
					}
					return p_self.lock();
				}
				if (auto p = this->p_parent.lock()) {
					return p->FindScopeWithObjImpl(objName, found_obj);
				}
			}
		}

		auto treeV = this->GetObjectCacheVersion();
		{
			std::shared_ptr<Scope> out;
			if (TryGetCached<3>(treeV, out, objName)) {
				if (out) {
					if (auto objFound = out->GetObj(objName)) {
						if (found_obj) {
							*found_obj = objFound;
						}
						return out;
					}
					else {
						return nullptr;
					}
				}
			}
		}

		if (auto objFound = this->GetObj(objName)) {
			if (found_obj) *found_obj = objFound;
			InsertCached<3>(treeV, p_self.lock(), objName);
			return p_self.lock();
		}

#endif

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Scope> out;
			if (TryFindNearestScopeWhere(out, [&objName, &found_obj](std::shared_ptr<Scope> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (auto objFound = ptr->GetObj(objName)) {
						if (found_obj) *found_obj = objFound;
						return true;
					}
				}
				return false;
				})) {
				InsertCached<3>(treeV, out, objName);
				return out;
			}
			else {
				InsertCached<3>(treeV, this->GetSelf(), objName);
				return this->GetSelf();
			}
		}
		else {
			std::string Namespace = objName.substr(0, lastOfColons - 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				auto out = namespacePtr->FindScopeWithObjImpl(objName.substr(lastOfColons + 1), found_obj);
				InsertCached<3>(treeV, out, objName.substr(lastOfColons + 1));
				return out;
			}
			else {
				InsertCached<3>(treeV, this->GetSelf(), objName.substr(lastOfColons + 1));
				return this->GetSelf();
			}
		}
	};

	std::shared_ptr<Scope> Scope::FindScopeWithObj(std::string const& objName, std::shared_ptr<Any>* found_obj) const {
		return FindScopeWithObjImpl(objName, found_obj);
	};
	std::shared_ptr<Any> Scope::FindObj(std::string objName) const {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(objName);

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Any> out{ nullptr };
			if (auto ptr = FindScopeWithObj(objName, &out)) {
				if (out)
					return out;
				else
					return ptr->GetObj(objName);
			}
			else {
				return nullptr;
			}
		}
		else {
			std::string Namespace = objName.substr(0, lastOfColons - 1);
			objName = objName.substr(lastOfColons + 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindObj(objName);
			}
			else {
				return nullptr;
			}
		}
	};
	std::shared_ptr<Namespace> Scope::FindNamespaceWithFunction(std::string functionName) const {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(functionName);

		auto lastOfColons = functionName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Namespace> out;
			if (TryFindNearestNamespaceWhere(out, [&functionName](std::shared_ptr<Namespace> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (auto objFound = ptr->GetFunctions(functionName)) {
						return true;
					}
				}
				return false;
				})) {
				return out;
			}
			else {
				return nullptr;
			}
		}
		else {
			std::string Namespace = functionName.substr(0, lastOfColons - 1);
			functionName = functionName.substr(lastOfColons + 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindNamespaceWithFunction(functionName);
			}
			else {
				return nullptr;
			}
		}
	};
	std::shared_ptr< Functions::FunctionSort > Scope::FindFunctions(std::string functionName) const {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(functionName);

		auto lastOfColons = functionName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			if (auto ptr = std::dynamic_pointer_cast<Scope>(FindNamespaceWithFunction(functionName))) {
				return ptr->GetFunctions(functionName);
			}
			else {
				return nullptr;
			}
		}
		else {
			std::string Namespace = functionName.substr(0, lastOfColons - 1);
			functionName = functionName.substr(lastOfColons + 1);

			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindFunctions(functionName);
			}
			else {
				return nullptr;
			}
		}
	};
	std::shared_ptr<Namespace> Scope::FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree) {
		static auto fixNamespace{ [](std::string x) -> std::string {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

			return x;
		} };
		functionName = fixNamespace(functionName);

		auto lastOfColons = functionName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Namespace> out;
			if (TryFindNearestNamespaceWhere(out, [&functionName, &params, &Params, &tree](std::shared_ptr<Namespace> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (auto func = ptr->GetFunction(functionName, params, Params, tree)) {
						return true;
					}
				}
				return false;
				})) {
				return out;
			}
			else {
				return nullptr;
			}
		}
		else {
			std::string Namespace = functionName.substr(0, lastOfColons - 1);
			functionName = functionName.substr(lastOfColons + 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindNamespaceWithFunction(functionName, params, Params, tree);
			}
			else {
				return nullptr;
			}
		}
	};
	Proxy_Function Scope::FindFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(functionName);

		auto lastOfColons = functionName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			ParamTypes Params{ params };
			if (auto ptr = std::dynamic_pointer_cast<Scope>(FindNamespaceWithFunction(functionName, params, Params, tree))) {
				return ptr->GetFunction(functionName, params, Params, tree);
			}
			else {
				return {};
			}
		}
		else {
			std::string Namespace = functionName.substr(0, lastOfColons - 1);
			functionName = functionName.substr(lastOfColons + 1);

			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindFunction(functionName, params, tree);
			}
			else {
				return {};
			}
		}
	};

	bool Scope::TryFindNearestScopeWhere_2(
		std::shared_ptr<Scope>& bestMatch,
		std::function<bool(std::shared_ptr<Scope> const&, bool, bool)> const& func,
		bool isExporingParent,
		bool allowFindObject,
		GoodLang::details::flat_set< Scope* > const& CheckedSelf,
		GoodLang::details::flat_set< Scope* > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<GoodLang::details::flat_set< Scope* >&>(CheckedSelf);
		auto& checkedAll = const_cast<GoodLang::details::flat_set< Scope* >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.contains(selfPtr.get())) { return false; }
		checkedAll.emplace(selfPtr.get());

		std::shared_ptr<Scope> p;

		// test myself			
		if (!checkedSelf.contains(selfPtr.get())) {
			checkedSelf.emplace(selfPtr.get());
			if (p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
				if (func(p, isExporingParent, allowFindObject)) {
					bestMatch = p;
					return true;
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestScopeWhere_2(bestMatch, func, isExporingParent, allowFindObject, CheckedSelf, CheckedAll)) {
						return true;
					}
				}

			}
		}

		// test all of my parents
		auto parentPtr = this->p_parent.lock();
		while (parentPtr) {
			if (!checkedSelf.contains(parentPtr.get())) {
				checkedSelf.emplace(parentPtr.get());
				if (p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
					if (func(p, true, allowFindObject)) {
						bestMatch = p;
						return true;
					}
				}
			}
			else {
				break; // we've checked this before! Quick, get out. 
			}
			parentPtr = parentPtr->p_parent.lock();
		}

		// Test my children themselves.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				// p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!checkedSelf.contains(innerChildNamespace.second.get())) {
					checkedSelf.emplace(innerChildNamespace.second.get());
					if (innerChildNamespace.second) {
						if (func(innerChildNamespace.second, isExporingParent, allowFindObject)) {
							bestMatch = innerChildNamespace.second;
							return true;
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my parents and their children
		if (p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere_2(bestMatch, func, true, allowFindObject, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestScopeWhere_2(bestMatch, func, isExporingParent, allowFindObject, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};
	bool FunctionScope::TryFindNearestScopeWhere_2(
		std::shared_ptr<Scope>& bestMatch,
		std::function<bool(std::shared_ptr<Scope> const&, bool, bool)> const& func,
		bool isExporingParent,
		bool allowFindObject,
		GoodLang::details::flat_set< Scope* > const& CheckedSelf,
		GoodLang::details::flat_set< Scope* > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<GoodLang::details::flat_set< Scope* >&>(CheckedSelf);
		auto& checkedAll = const_cast<GoodLang::details::flat_set< Scope* >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.contains(selfPtr.get())) { return false; }
		checkedAll.emplace(selfPtr.get());

		// test myself			
		if (!checkedSelf.contains(selfPtr.get())) {
			checkedSelf.emplace(selfPtr.get());
			if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
				if (func(p, isExporingParent, allowFindObject)) {
					bestMatch = p;
					return true;
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestScopeWhere_2(bestMatch, func, isExporingParent, allowFindObject, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}

		// test all of my parents
		auto parentPtr = this->p_parent.lock();
		while (parentPtr) {
			if (!checkedSelf.contains(parentPtr.get())) {
				checkedSelf.emplace(parentPtr.get());
				if (auto p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
					if (func(p, true, false)) {
						bestMatch = p;
						return true;
					}
				}
			}
			else {
				break; // we've checked this before! Quick, get out. 
			}
			parentPtr = parentPtr->p_parent.lock();
		}

		// Test my children themselves.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				// auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!checkedSelf.contains(innerChildNamespace.second.get())) {
					checkedSelf.emplace(innerChildNamespace.second.get());
					if (innerChildNamespace.second) {
						if (func(innerChildNamespace.second, isExporingParent, allowFindObject)) {
							bestMatch = innerChildNamespace.second;
							return true;
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere_2(bestMatch, func, true, false, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestScopeWhere_2(bestMatch, func, isExporingParent, allowFindObject, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};
	bool Class::TryFindNearestScopeWhere_2(
		std::shared_ptr<Scope>& bestMatch,
		std::function<bool(std::shared_ptr<Scope> const&, bool, bool)> const& func,
		bool isExporingParent,
		bool allowFindObject,
		GoodLang::details::flat_set< Scope* > const& CheckedSelf,
		GoodLang::details::flat_set< Scope* > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<GoodLang::details::flat_set< Scope* >&>(CheckedSelf);
		auto& checkedAll = const_cast<GoodLang::details::flat_set< Scope* >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.contains(selfPtr.get())) { return false; }
		checkedAll.emplace(selfPtr.get());

		// test myself			
		if (!checkedSelf.contains(selfPtr.get())) {
			checkedSelf.emplace(selfPtr.get());
			if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
				if (func(p, isExporingParent, allowFindObject)) {
					bestMatch = p;
					return true;
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestScopeWhere_2(bestMatch, func, isExporingParent, allowFindObject, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}

		// test my inherited namespace.
		for (auto& Parent : DerivedFrom) {
			if (auto p = std::dynamic_pointer_cast<Scope>(Parent.lock())) {
				if (p->TryFindNearestScopeWhere_2(bestMatch, func, true, allowFindObject, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		// test all of my parents
		auto parentPtr = this->p_parent.lock();
		while (parentPtr) {
			if (!checkedSelf.contains(parentPtr.get())) {
				checkedSelf.emplace(parentPtr.get());
				if (auto p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
					if (func(p, true, allowFindObject)) {
						bestMatch = p;
						return true;
					}
				}
			}
			else {
				break; // we've checked this before! Quick, get out. 
			}
			parentPtr = parentPtr->p_parent.lock();
		}

		// Test my children themselves.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				// auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!checkedSelf.contains(innerChildNamespace.second.get())) {
					checkedSelf.emplace(innerChildNamespace.second.get());
					if (innerChildNamespace.second) {
						if (func(innerChildNamespace.second, isExporingParent, allowFindObject)) {
							bestMatch = innerChildNamespace.second;
							return true;
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}

		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere_2(bestMatch, func, true, allowFindObject, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestScopeWhere_2(bestMatch, func, isExporingParent, allowFindObject, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};


	std::shared_ptr<Scope> Scope::FindScopeWithObjOrFunction(
		std::string objName,
		std::vector<Any> const& params,
		ParamTypes const& Params,
		TypeConverter& tree,
		std::shared_ptr<Any>* found_obj,
		Proxy_Function* found_function
	) {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(objName);

		std::shared_ptr<Scope> out;

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			// If this scope has no children (e.g. no namespaces declared) we can speed this up by manually checking for the requested thing and going backwards to the parent. 
			if (this->is_basic_scope) {
				if ((!this->p_objects.DataExists()) || (this->p_objects.data->size() == 0)) {
					if (auto parent = this->p_parent.lock()) {
						out = parent->FindScopeWithObjOrFunction(objName, params, Params, tree, found_obj, found_function);
						return out;
					}
				}
				else {
					if (auto objFound = this->GetObj(objName)) {
						if (found_obj) *found_obj = objFound;
						out = this->GetSelf();
						return out;
					}
					else {
						if (auto parent = this->p_parent.lock()) {
							out = parent->FindScopeWithObjOrFunction(objName, params, Params, tree, found_obj, found_function);
							return out;
						}
					}
				}

			}
			else {
				bool success = TryFindNearestScopeWhere_2(
					out,
					[&objName, &params, &Params, &tree, &found_obj, &found_function](
						std::shared_ptr<Scope> const& ptr, bool isExploringParent, bool allowFindObject
						)->bool {
							if (ptr) {
								// always prefer objects if we are able
								if (allowFindObject) {
									if (auto objFound = ptr->GetObj(objName)) {
										if (found_obj) *found_obj = objFound;
										return true;
									}
								}
								if (auto func = ptr->GetFunction(objName, params, Params, tree)) {
									if (found_function) *found_function = func;
									return true;
								}
							}
							return false;
					}, false, true);
				if (success) {
					// do the save
					return out;
				}
				else {
					return this->GetSelf();
				}
			}
		}
		else {
			std::string Namespace = objName.substr(0, lastOfColons - 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				out = namespacePtr->FindScopeWithObjOrFunction(objName.substr(lastOfColons + 1), params, Params, tree, found_obj, found_function);
				// do the save
				return out;
			}
			else {
				// out = this->GetSelf();
				return this->GetSelf();
			}
		}
	};
	std::shared_ptr<Scope> Scope::FindScopeWithObjOrFunction(
		std::string objName,
		ParamTypes const& Params,
		TypeConverter& tree,
		std::shared_ptr<Any>* found_obj,
		Proxy_Function* found_function
	) {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(objName);

		std::shared_ptr<Scope> out;

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			// If this scope has no children (e.g. no namespaces declared) we can speed this up by manually checking for the requested thing and going backwards to the parent. 
			if (this->is_basic_scope) {
				if ((!this->p_objects.DataExists()) || (this->p_objects.data->size() == 0)) {
					if (auto parent = this->p_parent.lock()) {
						out = parent->FindScopeWithObjOrFunction(objName, Params, tree, found_obj, found_function);
						return out;
					}
				}
				else {
					if (auto objFound = this->GetObj(objName)) {
						if (found_obj) *found_obj = objFound;
						out = this->GetSelf();
						return out;
					}
					else {
						if (auto parent = this->p_parent.lock()) {
							out = parent->FindScopeWithObjOrFunction(objName, Params, tree, found_obj, found_function);
							return out;
						}
					}
				}
			}
			else {
				bool success = TryFindNearestScopeWhere_2(
					out,
					[&objName, &Params, &tree, &found_obj, &found_function](
						std::shared_ptr<Scope> const& ptr, bool isExploringParent, bool allowFindObject
						)->bool {
							if (ptr) {
								// always prefer objects if we are able
								if (allowFindObject) {
									if (auto objFound = ptr->GetObj(objName)) {
										if (found_obj) *found_obj = objFound;
										return true;
									}
								}
								if (auto func = ptr->GetFunction(objName, const_cast<ParamTypes&>(Params), tree)) {
									if (found_function) *found_function = func;
									return true;
								}
							}
							return false;
					}, false, true);
				if (success) {
					// do the save
					return out;
				}
				else {
					return this->GetSelf();
				}
			}
		}
		else {
			std::string Namespace = objName.substr(0, lastOfColons - 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				out = namespacePtr->FindScopeWithObjOrFunction(objName.substr(lastOfColons + 1), Params, tree, found_obj, found_function);
				// do the save
				return out;
			}
			else {
				// out = this->GetSelf();
				return this->GetSelf();
			}
		}
	};

	bool Scope::FindObjOrFunction(std::string const& objName, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree, std::shared_ptr<Any>* found_obj, Proxy_Function* found_function) {
		// If I am a scope, with no objects, no using statements, and no children, then there's no point in asking me any of this. 
		if (this->is_basic_scope) {
			if ((!this->p_objects.DataExists()) || (this->p_objects.data->size() == 0)) {
				if (auto parent = this->p_parent.lock()) {
					return parent->FindObjOrFunction(objName, params, Params, tree, found_obj, found_function);
				}
			}
			else {
				if (auto objFound = this->GetObj(objName)) {
					if (found_obj) *found_obj = objFound;
					return true;
				}
				else if (auto parent = this->p_parent.lock()) {
					return parent->FindObjOrFunction(objName, params, Params, tree, found_obj, found_function);
				}
			}
		}

		auto objVersion = this->GetObjectCacheVersion();
		auto funcVersion = this->GetTypeConverterTreeVersion();
		if (found_obj) {
			if (TryGetCached<4>(objVersion, *found_obj, objName)) {
				return true;
			}
		}
		if (found_function) {
			if (TryGetCached<5>(funcVersion, *found_function, objName, Params.hash())) {
				return true;
			}
		}

		if (auto found_scope = FindScopeWithObjOrFunction(objName, params, Params, tree, found_obj, found_function)) {
			if (found_obj && *found_obj) {
				InsertCachedIfNotExist<4>(objVersion, *found_obj, objName);
				return true;
			}
			if (found_function && *found_function) {
				InsertCachedIfNotExist<5>(funcVersion, *found_function, objName, Params.hash());
				return true;
			}

			if (auto objFound = found_scope->GetObj(objName)) {
				if (found_obj) *found_obj = objFound;
				InsertCachedIfNotExist<4>(objVersion, objFound, objName);
				return true;
			}

			if (auto func = found_scope->GetFunction(objName, params, Params, tree)) {
				if (found_function) *found_function = func;
				InsertCachedIfNotExist<5>(funcVersion, func, objName, Params.hash());
				return true;
			}
		}
		return false;
	};
	bool Scope::FindObjOrFunction(std::string const& objName, ParamTypes const& Params, TypeConverter& tree, std::shared_ptr<Any>* found_obj, Proxy_Function* found_function) {
		// If I am a scope, with no objects, no using statements, and no children, then there's no point in asking me any of this. 
		if (this->is_basic_scope) {
			if ((!this->p_objects.DataExists()) || (this->p_objects.data->size() == 0)) {
				if (auto parent = this->p_parent.lock()) {
					return parent->FindObjOrFunction(objName, Params, tree, found_obj, found_function);
				}
			}
			else {
				if (auto objFound = this->GetObj(objName)) {
					if (found_obj) *found_obj = objFound;
					return true;
				}
				else if (auto parent = this->p_parent.lock()) {
					return parent->FindObjOrFunction(objName, Params, tree, found_obj, found_function);
				}
			}
		}

		auto objVersion = this->GetObjectCacheVersion();
		auto funcVersion = this->GetTypeConverterTreeVersion();
		if (found_obj) {
			if (TryGetCached<4>(objVersion, *found_obj, objName)) {
				return true;
			}
		}
		if (found_function) {
			if (TryGetCached<5>(funcVersion, *found_function, objName, Params.hash())) {
				return true;
			}
		}

		if (auto found_scope = FindScopeWithObjOrFunction(objName, Params, tree, found_obj, found_function)) {
			if (found_obj && *found_obj) {
				InsertCachedIfNotExist<4>(objVersion, *found_obj, objName);
				return true;
			}
			if (found_function && *found_function) {
				InsertCachedIfNotExist<5>(funcVersion, *found_function, objName, Params.hash());
				return true;
			}

			if (auto objFound = found_scope->GetObj(objName)) {
				if (found_obj) *found_obj = objFound;
				InsertCachedIfNotExist<4>(objVersion, objFound, objName);
				return true;
			}

			if (auto func = found_scope->GetFunction(objName, const_cast<ParamTypes&>(Params), tree)) {
				if (found_function) *found_function = func;
				InsertCachedIfNotExist<5>(funcVersion, func, objName, Params.hash());
				return true;
			}
		}
		return false;
	};

	std::shared_ptr<Namespace> Scope::FindNamespaceWithFunction(std::string functionName, ParamTypes& params, TypeConverter& tree) {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(functionName);

		auto lastOfColons = functionName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Namespace> out;
			if (TryFindNearestNamespaceWhere(out, [&functionName, &params, &tree](std::shared_ptr<Namespace> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (auto func = ptr->GetFunction(functionName, params, tree)) {
						return true;
					}
				}
				return false;
				})) {
				return out;
			}
			else {
				return nullptr;
			}
		}
		else {
			std::string Namespace = functionName.substr(0, lastOfColons - 1);
			functionName = functionName.substr(lastOfColons + 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindNamespaceWithFunction(functionName, params, tree);
			}
			else {
				return nullptr;
			}
		}
	};
	Proxy_Function Scope::FindFunction(std::string functionName, ParamTypes& params, TypeConverter& tree) {
		static auto fixNamespace{ [](std::string& x) {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
		} };
		fixNamespace(functionName);

		auto lastOfColons = functionName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			if (auto ptr = std::dynamic_pointer_cast<Scope>(FindNamespaceWithFunction(functionName, params, tree))) {
				return ptr->GetFunction(functionName, params, tree);
			}
			else {
				return {};
			}
		}
		else {
			std::string Namespace = functionName.substr(0, lastOfColons - 1);
			functionName = functionName.substr(lastOfColons + 1);

			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				return namespacePtr->FindFunction(functionName, params, tree);
			}
			else {
				return {};
			}
		}
	};


	size_t Scope::GetObjectCacheVersion() const {
		// Theory: combine the hash for this scope and its parents
		if (auto p = this->p_parent.lock()) {
			//size_t seed{ 0 };
			//details::hash_combine(seed, CachedObjectVersion.load(), p->GetObjectCacheVersion());
			//return seed;
			return CachedObjectVersion.load() + p->GetObjectCacheVersion();
		}
		else {
			return CachedObjectVersion.load();
		}
		// return CachedObjectVersion.load();
	};
	size_t Scope::GetTypeConverterTreeVersion() const {
		if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
			return p->GetTypeConverterTreeVersion();
		}
		else {
			return 0;
		}
	};
	GoodLang::shared_ptr<TypeConverter>& Scope::GetTypeConverterTree() const {
		if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
			return p->GetTypeConverterTree();
		}
		else {
			throw std::runtime_error("Cannot capture conversion tree without a global scope.");
		}
	};
	std::shared_ptr<Namespace> Scope::FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, ParamTypes const& Params) {
		return FindNamespaceWithFunction(functionName, params, Params, *GetTypeConverterTree());
	};
	Proxy_Function Scope::FindFunction(std::string functionName, std::vector<Any> const& params) {
		auto& tree = GetTypeConverterTree();
		return FindFunction(functionName, params, *tree);
	};
	std::vector<std::shared_ptr<Scope>> Scope::GetScopesForObjectSearch() const {
		std::vector<std::shared_ptr<Scope>> out;
		// will loop over all available scopes in the order we like
		(void)FindNearestScopeWhere([&](std::shared_ptr<Scope> const& ptr) -> bool {
			out.push_back(ptr);
			return false;
			});
		return out;
	};
	bool Scope::AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists) {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->AddFunction(name, function, overrideIfAlreadyExists);
		}
		else {
			return false;
		}
	};
	bool Scope::TryFindFunctionImpl(std::string const& functionName, std::vector<Any>  const& params, ParamTypes const& Params, GoodLang::shared_ptr<TypeConverter> const& m_conversionTree, Proxy_Function& out, size_t paramsHash) const {
		if (!m_conversionTree) return false;
#ifdef useCachedData
		if (paramsHash == 0) paramsHash = Params.hash();
		auto treeV = this->GetTypeConverterTreeVersion();
		if (TryGetCached<0>(treeV, out, functionName, paramsHash)) {
			return (bool)out;
		}
		defer(if (out) InsertCachedIfNotExist<0>(treeV, out, functionName, paramsHash));
#endif

		size_t lastOfColons{ functionName.find_last_of("::") };
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Scope> firstParamScopePtr{ nullptr };
			std::shared_ptr<Scope> constructorScopePtr{ nullptr };

			// FIRST SEARCH DOES ALLOW FOR CONVERSIONS, BUT NO TEMPLATES
			if (1) {
				std::multimap<double, Proxy_Function> sort;

				// FIRST, WE CHECK TO SEE IF THE DESIRED FUNCTION IS AVAILABLE FROM THE CLASS OF THE FIRST PARAM (e.g. to_string(Units::foot()) would search the Units::foot class before anything else)
				{
					auto firstParam = params.begin();
					if (firstParam != params.end()) {
						firstParamScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(firstParam->ActualType())); // firstParam->Type()
					}
				}
				// While we normally try to minimize the conversion cost, 
				if (firstParamScopePtr) {
					(void)firstParamScopePtr->FindNearestNamespaceWhere([&sort, &functionName, &params, &Params, &m_conversionTree](std::shared_ptr<Namespace> const& namespace_ptr) -> bool {
						if (auto scope = std::dynamic_pointer_cast<Scope>(namespace_ptr)) {
							auto& ptr = scope->GetFunctions();
							if (auto func = ptr.BuildMatch(functionName, params, Params, *m_conversionTree, false, true)) {
								auto cost = func->conversion_cost(params, *m_conversionTree);
								if (cost < details::TypeConversionWorstCaseCost) {
									sort.emplace(cost, func);
								}
								// return true; // found a nearby match? Give up? Add a give-up criteria? 
							}
							
						}
						return false;
					});
				}

				// try to find the function from nearby scopes... 
				(void)FindNearestNamespaceWhere([&sort, &functionName, &params, &Params, &m_conversionTree](std::shared_ptr<Namespace> const& namespace_ptr) -> bool {
					if (auto scope = std::dynamic_pointer_cast<Scope>(namespace_ptr)) {
						auto& ptr = scope->GetFunctions();
						if (auto func = ptr.BuildMatch(functionName, params, Params, *m_conversionTree, false, true)) {
							auto cost = func->conversion_cost(params, *m_conversionTree);
							if (cost < details::TypeConversionWorstCaseCost) {
								sort.emplace(cost + 1, func);
							}
							// return true; // found a nearby match? Give up? Add a give-up criteria? 
						}
						
					}
					return false;
					});

				// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, BUT NO TEMPLATES)
				if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
					// Is there a pre-defined constructor that this could work with?
					auto& functions = constructorScopePtr->GetFunctions();
					if (auto func = functions.BuildMatch(functionName, params, Params, *m_conversionTree, false, true)) {
						auto cost = func->conversion_cost(params, *m_conversionTree);
						if (cost < details::TypeConversionWorstCaseCost) {
							sort.emplace(cost + 2, func);
						}
					}					
				}
				for (auto& s : sort) {
					if (s.first < details::TypeConversionWorstCaseCost) {
						out = s.second;
						// EmplaceCache(cache2, params, out);
						return true;
					}
				}
			}

			// SECOND SEARCH ALLOWS FOR TEMPLATE FUNCTIONS
			if (1) {
				std::multimap<double, Proxy_Function> sort;
				for (auto& scope : GetScopesForObjectSearch()) {
					if (scope) {
						auto& ptr = scope->GetFunctions();
						if (auto func = ptr.BuildMatch(functionName, params, Params, *m_conversionTree, true, true)) {
							auto cost = func->conversion_cost(params, *m_conversionTree);
							if (cost < details::TypeConversionWorstCaseCost) {
								sort.emplace(cost + 3, func);
							}
						}						
					}
				}
				if (firstParamScopePtr) {
					for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
						if (scope) {
							auto& ptr = scope->GetFunctions();
							if (auto func = ptr.BuildMatch(functionName, params, Params, *m_conversionTree, true, true)) {
								auto cost = func->conversion_cost(params, *m_conversionTree);
								if (cost < details::TypeConversionWorstCaseCost) {
									sort.emplace(cost + 4, func);
								}
							}
							
						}
					}
				}
				// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, AND TEMPLATES)
				if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
					// Is there a pre-defined constructor that this could work with?
					auto& functions = constructorScopePtr->GetFunctions();
					if (auto func = functions.BuildMatch(functionName, params, Params, *m_conversionTree, true, true)) {
						auto cost = func->conversion_cost(params, *m_conversionTree);
						if (cost < details::TypeConversionWorstCaseCost) {
							sort.emplace(cost + 5, func);
						}
					}
					
				}
				for (auto& s : sort) {
					if (s.first < details::TypeConversionWorstCaseCost) {
						out = s.second;
						return true;
					}
				}
			}

			// IF ALL SEARCHES FAILED, PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS
			if (1) {
				if (constructorScopePtr) {
					auto& functions = constructorScopePtr->GetFunctions();
					if (auto func = functions.BuildMatch(functionName, params, Params, *m_conversionTree, true, true)) {
						if (func->conversion_cost(params, *m_conversionTree) < details::TypeConversionWorstCaseCost) {
							out = func;
							return true;
						}
					}					
				}
			}
		}
		else {
			std::string functionNameActual{ functionName.substr(lastOfColons + 1) };
			std::string scopeName{ functionName.substr(0, lastOfColons - 1) };

			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(scopeName))) {
				if (namespacePtr->TryFindFunctionImpl(functionNameActual, params, Params, m_conversionTree, out, Params.hash())) {
					return true;
				}
				else {
					return false;
				}
			}
		}
		return false;
	};
	Any Scope::BuildAndCallFunction(std::string const& functionName, std::vector<Any> const& params, ParamTypes const& Params, size_t paramsHash) const {
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (this->IsBasicScope()) {
			if (auto p = this->p_parent.lock()) {
				return p->BuildAndCallFunction(functionName, params, Params, paramsHash);
			}
		}

		std::pair<Proxy_Function, GoodLang::shared_ptr<TypeConverter>> out{ nullptr, this->GetTypeConverterTree() };
		if (TryFindFunctionImpl(functionName, params, Params, out.second, out.first, paramsHash)) {
			return call(out.first, params, *out.second.get());
		}
		else {
			// function was not found with the given params
			std::string params_str;
			for (auto& p : params) {
				std::string className = p.TypeName(); {
					if (auto classPtr = std::dynamic_pointer_cast<Scope>(this->FindClass(p.ActualType()))) {
						className = classPtr->GetName();
					}
				}

				if (params_str.empty()) {
					params_str += className;
				}
				else {
					params_str += ", ";
					params_str += className;
				}
			}

			throw exception::not_found_error(GoodLang::printf("`%s`(%s)", functionName.c_str(), params_str.c_str()));
		}			
	};
	std::pair<Proxy_Function, std::reference_wrapper<GoodLang::shared_ptr<TypeConverter>>> Scope::BuildFunction(std::string const& functionName, std::vector<Any> const& params, ParamTypes const& Params, size_t paramsHash) const {
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (this->IsBasicScope()) {
			if (auto p = this->p_parent.lock()) {
				return p->BuildFunction(functionName, params, Params, paramsHash);
			}
		}

		if (1) {
			std::pair<Proxy_Function, std::reference_wrapper<GoodLang::shared_ptr<TypeConverter>>> out{ nullptr, std::ref(this->GetTypeConverterTree()) };
			if (TryFindFunctionImpl(functionName, params, Params, out.second, out.first, paramsHash)) {
				return out;
			}
			else {
				return out;
			}
		}
	};
	Any Scope::CallFunction(std::string const& functionName, std::vector<Any> const& params) const {
		ParamTypes Params{ params };
		return BuildAndCallFunction(functionName, params, Params, Params.hash());
	};
	Any Scope::CallFunction(Proxy_Function const& func, std::vector<Any> const& params) const {
		auto& tree{ this->GetTypeConverterTree() };
		if (func) {
			return call(func, params, *tree);
		}
		else {
			throw std::runtime_error("Empty function was provided to CallFunction with direct instancing -- this shouldn't be allowed normally by design.");
		}
	};
	Any Scope::CallFunction(std::string const& functionName, Any& params) const {
		std::vector<Any> temp{ params };
		ParamTypes Params{ temp };
		return BuildAndCallFunction(functionName, temp, Params, Params.hash());
	};
	Any Scope::CallFunction(Proxy_Function const& func, Any& params) const {
		auto& tree{ this->GetTypeConverterTree() };
		if (func) {
			return call(func, params, *tree);
		}
		else {
			throw std::runtime_error("Empty function was provided to CallFunction with direct instancing -- this shouldn't be allowed normally by design.");
		}
	};

	std::shared_ptr<Scope>  FunctionScope::FindScopeWithObjImpl(std::string const& objName, std::shared_ptr<Any>* found_obj) const {
#ifdef useCachedData
		auto treeV = this->GetObjectCacheVersion();
		{
			std::shared_ptr<Scope> out;
			if (TryGetCached<3>(treeV, out, objName)) {
				if (out) {
					if (auto objFound = out->GetObj(objName)) {
						if (found_obj) {
							*found_obj = objFound;
						}
						return out;
					}
					else {
						return nullptr;
					}
				}
			}
		}

		if (auto objFound = this->GetObj(objName)) {
			if (found_obj) *found_obj = objFound;
			InsertCached<3>(treeV, p_self.lock(), objName);
			return p_self.lock();
		}

#endif

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Scope> out;
			if (TryFindNearestScopeWhere(out, [&objName, &found_obj](std::shared_ptr<Scope> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (auto objFound = ptr->GetObj(objName)) {
						if (found_obj) *found_obj = objFound;
						return true;
					}
				}
				return false;
				})) {
				InsertCached<3>(treeV, out, objName);
				return out;
			}
			else {
				InsertCached<3>(treeV, this->GetSelf(), objName);
				return this->GetSelf();
			}
		}
		else {
			std::string Namespace = objName.substr(0, lastOfColons - 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				auto out = namespacePtr->FindScopeWithObjImpl(objName.substr(lastOfColons + 1), found_obj);
				InsertCached<3>(treeV, out, objName.substr(lastOfColons + 1));
				return out;
			}
			else {
				InsertCached<3>(treeV, this->GetSelf(), objName.substr(lastOfColons + 1));
				return this->GetSelf();
			}
		}
	};
	bool FunctionScope::TryFindNearestScopeWhere(
		std::shared_ptr<Scope>& bestMatch,
		std::function<bool(std::shared_ptr<Scope> const&)> const& func,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
		auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.count(selfPtr) >= 1) { return false; }
		checkedAll.emplace(selfPtr);

		// test myself			
		if (!(checkedSelf.count(selfPtr) >= 1)) {
			checkedSelf.emplace(selfPtr);
			if (1) {
				if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
					if (func(p)) {
						bestMatch = p;
						return true;
					}
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}

		// Test my children themselves.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!(checkedSelf.count(ptr) >= 1)) {
					checkedSelf.emplace(ptr);
					if (auto p = std::dynamic_pointer_cast<Scope>(ptr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};

	bool Namespace::AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists) {
		const_cast<Function&>(function).m_function->GetSignature().Name(name);
		const_cast<Function&>(function).m_function->GetSignature().QualifiedName(this->GetQualifiedNamespace() + name);

		function.m_function->GetSignature().Name(name);
		defer(this->RecordFunction(name, function));
		if (this->IsClass() && (this->GetName() == name) && (function.m_function->Arguments().size() <= 1)) {
			if (function.m_function->Arguments().size() == 1) {
				if (auto inputClass = std::dynamic_pointer_cast<Scope>(this->FindClass(function.m_function->Arguments().Types()[0]))) {
					try {
						//auto& tree = this->GetTypeConverterTree();
						//if (auto func = inputClass->GetFunction(inputClass->GetName(), {}, *tree)) {
							//if (auto inputParamImpl = func->operator()({}, *tree)) {
						return (bool)p_functions.emplace(name, function, overrideIfAlreadyExists);
						//}
					//}
					}
					catch (...) {}

				}
			}
			return (bool)p_functions.emplace(name, function, overrideIfAlreadyExists);
		}
		return (bool)p_functions.emplace(name, function, overrideIfAlreadyExists);
	};
	Functions& Namespace::GetFunctions() const {
		return const_cast<Functions&>(p_functions);
	};
	std::shared_ptr< Functions::FunctionSort > Namespace::GetFunctions(std::string const& name) const {
		static auto hasher{ std::hash<std::string>() };

		if (name.size() > 0) {
			auto& m_functions = const_cast<Functions & >(p_functions).FirstCharToFunctionNameMap[p_functions.CharToIndex(name[0])]; // try to reduce conflict by splitting on the first letter

			auto f = m_functions.find(hasher(name));
			if (f != m_functions.end()) {
				return std::shared_ptr< Functions::FunctionSort >(&f->second.second, [/*lockedCopy = locked*/](Functions::FunctionSort*) { /*if (!lockedCopy) { throw(std::runtime_error("ERR")); };*/ });
			}
		}
		return nullptr;
	};
	Proxy_Function Namespace::GetFunction(std::string const& name, ParamTypes& params, TypeConverter& tree) {
		return p_functions.BuildMatch(name, params, tree);
	};
	Proxy_Function Namespace::GetFunction(std::string const& name, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree) {
		return p_functions.BuildMatch(name, params, Params, tree);
	};
	Proxy_Function Namespace::GetFunction(std::string const& name, std::vector<Any> const& params) {
		auto& tree = GetTypeConverterTree();
		return GetFunction(name, params, ParamTypes(params), *tree);
	};


	void Class::ConstructMemberObjects(DynamicObject& obj) const {
		for (auto& Parent : DerivedFrom) {
			if (auto parentType = Parent.lock()) {
				parentType->ConstructMemberObjects(obj);
			}
		}

		for (auto& member_obj : p_declared_member_objects) {
			std::string const& memberObjectName = member_obj.first;
			if (auto memberObjectType = member_obj.second.first.lock()) {
				if (auto memberObjectClassType = this->FindClass(memberObjectType)) {
					auto& memberObjectDefaultInstance = member_obj.second.second;
					if (memberObjectDefaultInstance) {
						// default value was provided -- try to create a copy.
						try {
							Any defaultParam = memberObjectClassType->CallFunction(memberObjectClassType->GetName(), { memberObjectDefaultInstance });
							obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
							continue;
						}
						catch (...) {
							// could not create the copy for some reason. Place the default value directly.
							Any defaultParam = memberObjectDefaultInstance;
							obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
							continue;
						}
					}
					else {
						// undeclared default value -- try to create a new instance.
						Any defaultParam = memberObjectClassType->CallFunction(memberObjectClassType->GetName(), {});
						obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
						continue;
					}
				}
			}

			// something went wrong -- set it to void. The class type was not provided, could not be found, or could not be instanced.
			obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>();

		}
	};
	void Class::ConstructMemberObjects(DynamicObject& obj, DynamicObject const& CopyFrom) const {
		for (auto& Parent : DerivedFrom) {
			if (auto parentType = Parent.lock()) {
				parentType->ConstructMemberObjects(obj, CopyFrom);
			}
		}

		for (auto& member_obj : p_declared_member_objects) {
			std::string const& memberObjectName = member_obj.first;
			if (auto memberObjectType = member_obj.second.first.lock()) {
				if (auto memberObjectClassType = this->FindClass(memberObjectType)) {
					auto copyObjPtr = CopyFrom.m_objects->find(memberObjectName);
					if ((copyObjPtr != CopyFrom.m_objects->end()) && copyObjPtr->second) {
						auto& memberObjectDefaultInstance = copyObjPtr->second;
						if (memberObjectDefaultInstance) {
							// default value was provided -- try to create a copy.
							try {
								Any defaultParam = memberObjectClassType->CallFunction(memberObjectClassType->GetName(), { memberObjectDefaultInstance });
								obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
								continue;
							}
							catch (...) {
								// could not create the copy for some reason. Place the default value directly.
								Any defaultParam = memberObjectDefaultInstance;
								obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
								continue;
							}
						}
						else {
							// undeclared default value -- try to create a new instance.
							Any defaultParam = memberObjectClassType->CallFunction(memberObjectClassType->GetName(), {});
							obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
							continue;
						}
					}
					else {
						auto& memberObjectDefaultInstance = member_obj.second.second;
						if (memberObjectDefaultInstance) {
							// default value was provided -- try to create a copy.
							try {
								Any defaultParam = memberObjectClassType->CallFunction(memberObjectClassType->GetName(), { memberObjectDefaultInstance });
								obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
								continue;
							}
							catch (...) {
								// could not create the copy for some reason. Place the default value directly.
								Any defaultParam = memberObjectDefaultInstance;
								obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
								continue;
							}
						}
						else {
							// undeclared default value -- try to create a new instance.
							Any defaultParam = memberObjectClassType->CallFunction(memberObjectClassType->GetName(), {});
							obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
							continue;
						}
					}
				}
			}

			// something went wrong -- set it to void. The class type was not provided, could not be found, or could not be instanced.
			obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>();

		}
	};
	// Gets the member objects of just this class
	std::map<std::string, std::weak_ptr<Type_Info>> Class::GetMemberObjects() const {
		std::map<std::string, std::weak_ptr<Type_Info>> out;

		for (auto x : p_declared_member_objects) {
			out[x.first] = x.second.first;
		}
		return out;
	};
	// Gets the member objects of this class all all inherited classes (recursively)
	std::map<std::string, std::weak_ptr<Type_Info>> Class::GetAllMemberObjects() const {
		std::map<std::string, std::weak_ptr<Type_Info>> out;

		for (auto& Parent : DerivedFrom) {
			if (auto parentType = Parent.lock()) {
				out = parentType->GetAllMemberObjects();
			}
		}

		for (auto x : p_declared_member_objects) {
			out[x.first] = x.second.first;
		}
		return out;
	};
	void Class::AddDefaultConstructors() {
		// note, only do this if this class is a scripted type
		if (ClassType && !ClassType->IsBuiltInType()) {
			// Default constructor
			this->AddFunction(this->GetName(), make_callable([selfPtr = std::weak_ptr<Class>(std::dynamic_pointer_cast<Class>(p_self.lock()))]()->Any {
				if (auto self = selfPtr.lock()) {
					DynamicObject out{ self->GetClassType() };
					self->ConstructMemberObjects(out); // should automatically construct parent's objects in-order 
					return out;
				}
				else {
					throw(exception::not_found_error("Custom class type was no longer available"));
				}
			}));

			// Copy constructor
			this->AddFunction(this->GetName(), make_callable([selfPtr = std::weak_ptr<Class>(std::dynamic_pointer_cast<Class>(p_self.lock()))](Any const& from)->Any {
				DynamicObject const& obj = from.cast<DynamicObject const&>();
				if (auto self = selfPtr.lock()) {
					DynamicObject out{ self->GetClassType() };
					self->ConstructMemberObjects(out, obj);
					return out;
				}
				else {
					throw(exception::not_found_error("Custom class type was no longer available"));
				}
			}, ParamTypes({ ClassType->MakeConstRef() })));

			// assignment operator
			this->AddFunction("=", make_callable([selfPtr = std::weak_ptr<Class>(std::dynamic_pointer_cast<Class>(p_self.lock()))](Any const& to, Any const& from)->Any {
				DynamicObject& To = to.cast<DynamicObject&>();
				// To.m_objects->clear(); // NOT CONCURRENT-SAFE...
				DynamicObject const& From = from.cast<DynamicObject const&>();
				if (auto self = selfPtr.lock()) {
					self->ConstructMemberObjects(To, From);
					return to;
				}
				else {
					throw(exception::not_found_error("Custom class type was no longer available"));
				}
			}, ParamTypes({ ClassType->MakeRef(), ClassType->MakeConstRef() })));

			// upcast constructor for inherited types
			for (auto& derivedFrom : DerivedFrom) {
				if (auto parentClass = derivedFrom.lock()) {
					// Upcast (const&)
					parentClass->AddFunction(parentClass->GetName(), make_callable([selfPtr = std::weak_ptr<Class>(std::dynamic_pointer_cast<Class>(p_self.lock()))](Any const& from)->Any {
						DynamicObject const& obj = from.cast<DynamicObject const&>();
						if (auto p = selfPtr.lock()) {
							return DynamicObject(p->GetClassType(), obj);
						}
						else {
							return from;
						}
					}, ParamTypes({ ClassType->MakeConstRef() }), parentClass->GetClassType()));
				}
			}

			//// Member objects
			//for (auto& member_obj : this->GetMemberObjects()) {
			//	// ref access
			//	this->AddFunction(member_obj.first, make_callable([objName = member_obj.first](Any const& from)->Any {
			//		DynamicObject& From = from.cast<DynamicObject&>();
			//		return From.m_objects->at(objName);
			//	}, ParamTypes({ ClassType->MakeRef() }), member_obj.second.lock()->MakeRef()));
			//	// const ref access
			//	this->AddFunction(member_obj.first, make_callable([objName = member_obj.first](Any const& from)->Any {
			//		DynamicObject const& From = from.cast<DynamicObject const&>();
			//		return From.m_objects->at(objName);
			//	}, ParamTypes({ ClassType->MakeConstRef() }), member_obj.second.lock()->MakeConstRef()));
			//}
		}
	};
	void Class::DeclareMemberObject(std::string const& name, std::weak_ptr<Type_Info> type, std::shared_ptr<Any> defaultValue) {
		if (ClassType && !ClassType->IsBuiltInType()) {

			p_declared_member_objects.emplace(name, std::pair<std::weak_ptr<Type_Info>, std::shared_ptr<Any>>{ type, defaultValue });

			// ref access
			this->AddFunction(name, make_callable([objName = name](Any const& from)->Any {
				DynamicObject& From = from.cast<DynamicObject&>();
				return From.m_objects->at(objName);
				}, ParamTypes({ ClassType->MakeRef() }), type.lock()->MakeRef()));

			// const ref access
			this->AddFunction(name, make_callable([objName = name](Any const& from)->Any {
				DynamicObject const& From = from.cast<DynamicObject const&>();
				return From.m_objects->at(objName);
				}, ParamTypes({ ClassType->MakeConstRef() }), type.lock()->MakeConstRef()));
		}
	};
	bool Class::TryFindNearestScopeWhere(
		std::shared_ptr<Scope>& bestMatch,
		std::function<bool(std::shared_ptr<Scope> const&)> const& func,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
		auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.count(selfPtr) >= 1) { return false; }
		checkedAll.emplace(selfPtr);

		// test myself			
		if (!(checkedSelf.count(selfPtr) >= 1)) {
			checkedSelf.emplace(selfPtr);
			if (1) {
				if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
					if (func(p)) {
						bestMatch = p;
						return true;
					}
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}

		// test my inherited namespace.
		for (auto& Parent : DerivedFrom) {
			if (auto p = std::dynamic_pointer_cast<Scope>(Parent.lock())) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		// test all of my parents
		auto parentPtr = this->p_parent.lock();
		while (parentPtr) {
			if (!(checkedSelf.count(parentPtr) >= 1)) {
				checkedSelf.emplace(parentPtr);
				if (1) {
					if (auto p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}
			else {
				break; // we've checked this before! Quick, get out. 
			}
			parentPtr = parentPtr->p_parent.lock();
		}

		// Test my children themselves.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!(checkedSelf.count(ptr) >= 1)) {
					checkedSelf.emplace(ptr);
					if (1) {
						if (auto p = std::dynamic_pointer_cast<Scope>(ptr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		return false;
	};
	bool Class::TryFindNearestNamespaceWhere(
		std::shared_ptr<Namespace>& bestMatch,
		std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf,
		std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll
	) const {
		auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
		auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
		auto selfPtr = this->p_self.lock();

		// Prevent Duplication
		if (checkedAll.count(selfPtr) >= 1) { return false; }
		checkedAll.emplace(selfPtr);

		// test myself			
		if (!(checkedSelf.count(selfPtr) >= 1)) {
			checkedSelf.emplace(selfPtr);
			if (this->IsNamespace()) {
				if (auto p = std::dynamic_pointer_cast<Namespace>(selfPtr)) {
					if (func(p)) {
						bestMatch = p;
						return true;
					}
				}
			}
		}

		// test my "using" namespaces and their children.
		if (this->p_using.DataExists()) {
			for (auto& childNamespace : *this->p_using.data) {
				if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second->lock())) {
					if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}

		// test my inherited namespace.
		for (auto& Parent : DerivedFrom) {
			if (auto p = std::dynamic_pointer_cast<Scope>(Parent.lock())) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}
		}

		// test all of my parents 
		if (1) {
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!(checkedSelf.count(parentPtr) >= 1)) {
					checkedSelf.emplace(parentPtr);
					if (parentPtr->IsNamespace()) {
						if (auto p = std::dynamic_pointer_cast<Namespace>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();
			}
		}

		// test my children themselves
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second);
				if (!(checkedSelf.count(ptr) >= 1)) {
					checkedSelf.emplace(ptr);
					if (ptr->IsNamespace()) {
						if (auto p = std::dynamic_pointer_cast<Namespace>(ptr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
			}
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& innerChildNamespace : this->p_children) {
			if (innerChildNamespace.second) {
				if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
					if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}
		}

		return false;
	};


	std::vector<std::weak_ptr<Class>> Global::GetClasses() const {
		std::vector<std::weak_ptr<Class>> out;
		out.reserve(Classes.size() + 16);
		static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

		bool DoCleanup = false;

		size_t hash{ 0 };
		for (auto& x : Classes) {
			hash = Scope::Hasher()(x.second);
			if (hash == badHash) {
				DoCleanup = true;
			}
			else {
				out.push_back(x.second);
			}
		}

		if (DoCleanup) {
			//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
			const_cast<Global*>(this)->RemoveStaleReferences();
		}

		return out;
	};
	std::vector<std::weak_ptr<Namespace>> Global::GetUsing() const {
		std::vector<std::weak_ptr<Namespace>> out;
		out.reserve(Classes.size() + 16);
		static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

		bool DoCleanup = false;

		size_t hash{ 0 };
		for (auto& x : Usings) {
			hash = Scope::Hasher()(x.second);
			if (hash == badHash) {
				DoCleanup = true;
			}
			else {
				out.push_back(x.second);
			}

		}

		if (DoCleanup) {
			//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
			const_cast<Global*>(this)->RemoveStaleReferences();
		}

		return out;
	};
	void Global::GetClasses(std::unordered_map<size_t, std::weak_ptr<Class>>& out) const {
		bool DoCleanup = false;
		static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

		size_t hash{ 0 };
		for (auto& x : Classes) {
			hash = Scope::Hasher()(x.second);
			if (hash == badHash) {
				DoCleanup = true;
			}
			else {
				out.insert({ hash, x.second });
			}
		}

		if (DoCleanup) {
			const_cast<Global*>(this)->RemoveStaleReferences();
		}
	};
	void Global::GetAllAvailableClassesImpl(
		std::unordered_map<size_t, std::weak_ptr<Class>>& out,
		std::unordered_map<size_t, std::weak_ptr<Scope>>& uniqueLibraries
	) const {
		auto hashed{ Scope::Hasher()(this->p_self) };
		if (uniqueLibraries.count(hashed) > 0) return;
		uniqueLibraries.insert({ hashed, this->p_self });

		this->GetClasses(out);
		static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };
		size_t hash{ 0 };
		bool DoCleanup = false;
		for (auto& x : Usings) {
			hash = Scope::Hasher()(x.second);
			if (hash == badHash) {
				DoCleanup = true;
			}
			else {
				if (auto ptr = x.second.lock()) {
					if (auto p2 = ptr->GetLibrary()) {
						p2->GetAllAvailableClassesImpl(out, uniqueLibraries);
					}
				}
			}
		}
		if (DoCleanup) {
			//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
			const_cast<Global*>(this)->RemoveStaleReferences();
		}
	};
	// Searches for all classes that are defined in the current and "used" libraries. 
	std::unordered_map<size_t, std::weak_ptr<Class>> Global::GetAllAvailableClassesImpl() const {
		thread_local static std::unordered_map<size_t, std::weak_ptr<Class>> allClasses{};
		thread_local static std::unordered_map<size_t, std::weak_ptr<Scope>> uniqueList{};
		defer(allClasses.clear());
		defer(uniqueList.clear());

		GetAllAvailableClassesImpl(allClasses, uniqueList);
		return allClasses;
	};
	std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>> Global::GetAllAvailableClasses() const {
		auto oldVersion = CachedClassListVersion.load();
		if (oldVersion != RecordVersion) {
			auto guard{ std::scoped_lock(const_cast<Global*>(this)->CachedClassListMutex) };
			if (const_cast<Global*>(this)->CachedClassListVersion.CompareExchange(oldVersion, RecordVersion.GetValue())) {
				return const_cast<Global*>(this)->CachedClassList = std::make_shared<std::unordered_map<size_t, std::weak_ptr<Class>>>(GetAllAvailableClassesImpl());
			}
		}

		if (1) {
			auto guard{ std::shared_lock(const_cast<Global*>(this)->CachedClassListMutex) };
			return CachedClassList;
		}
	};
	// Creates a tree of type-converter functions using the classes found with GetAllAvailableClasses()
	void Global::CreateTypeConverterTree(GoodLang::shared_ptr<TypeConverter>& out) const {
		if (auto parent = std::dynamic_pointer_cast<Global>(this->p_parent.lock())) {
			parent->CreateTypeConverterTree(out);
		}

		if (auto classes = GetAllAvailableClasses()) {
			for (auto& FoundClass : *classes) {
				if (auto p = FoundClass.second.lock()) {
					if (auto& outputType = p->ClassType) {
						// templated conversions are not acceptable
						if (outputType->is_any()) continue; // note: this shouldn't happen. A Class with "Any" as its type is ill-formed.

						// Type Conversions are identical to Constructors with one input type and named after their class. Therefore ...
						auto className = p->GetName();
						// ... find all constructors with the name of the class ...
						if (auto constructors = p->GetFunctions(className)) {
							for (auto& constructor : *constructors) {
								if (constructor.second.second && constructor.second.second->m_function) {
									// ... whose inputs are size of 1 ...
									if (constructor.second.second->m_function->NumArguments() == 1) {
										// Explicit or cached (e.g. built from previous tree) conversions are not acceptable. 

										// Cached conversions are built from "true" conversions, which will be re-built again by this new tree anyways, so their
										// inclusion in the new tree is by definition not required. It may introduce a speed-up, but introduces lifetime issues and 
										// so I prefer not to include those previous caches. 
										if (!constructor.second.second->m_isCached && !constructor.second.second->m_isEplicit) {
											if (auto inputType = constructor.second.second->m_function->Arguments().Types()[0].lock()) {
												// templated conversions are not acceptable
												if (inputType->is_any()) continue;

												out->AddConverter([func = constructor.second.second->m_function](Any const& input)->Any {
													return func->operator()(const_cast<Any&>(input));
												}, inputType, outputType);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	};

	size_t Global::GetTypeConverterTreeVersion() const {
		return CachedTypeConverterTreeVersion.load();
	};
	GoodLang::shared_ptr<TypeConverter>& Global::GetTypeConverterTree() const {
		auto oldVersion = CachedTypeConverterTreeVersion.load();
		if (oldVersion != RecordVersion) {
			if (const_cast<Global*>(this)->CachedTypeConverterTreeVersion.CompareExchange(oldVersion, RecordVersion.GetValue())) {
				auto tree = GoodLang::make_shared<TypeConverter>();
				CreateTypeConverterTree(tree);
				return const_cast<Global*>(this)->CachedTypeConverterTree = tree;
			}
		}

		if (1) {
			return const_cast<GoodLang::shared_ptr<TypeConverter>&>(CachedTypeConverterTree);
		}
	};


};


namespace GoodLang {
	std::string Scope::ToString() const {
		return "Scope";
	};
	std::vector< Impl::NodeCache > Scope::GetChildren() const {
		return {
			GoodLang::GetChildren(this->p_children),
			GoodLang::GetChildren(this->p_objects)
		};
	};
	bool Scope::TryDisconnectChild() const {
		return false;
	};

	std::string FunctionScope::ToString() const {
		return "FunctionScope";
	};
	std::vector< Impl::NodeCache > FunctionScope::GetChildren() const {
		return {
			GoodLang::GetChildren(this->p_children),
			GoodLang::GetChildren(this->p_objects)
		};
	};
	bool FunctionScope::TryDisconnectChild() const {
		return false;
	};

	std::string Namespace::ToString() const {
		return "Namespace";
	};
	std::vector< Impl::NodeCache > Namespace::GetChildren() const {
		return {
			GoodLang::GetChildren(this->p_children),
			GoodLang::GetChildren(this->p_objects),
			GoodLang::GetChildren(this->p_functions)
		};
	};
	bool Namespace::TryDisconnectChild() const {
		return false;
	};

	std::string Class::ToString() const {
		return "Class";
	};
	std::vector< Impl::NodeCache > Class::GetChildren() const {
		return {
			GoodLang::GetChildren(this->p_children),
			GoodLang::GetChildren(this->p_objects),
			GoodLang::GetChildren(this->p_functions),
			GoodLang::GetChildren(this->p_declared_member_objects)
		};
	};
	bool Class::TryDisconnectChild() const {
		return false;
	};


	bool Global::RecordClass(std::shared_ptr<Class> ptr) {
		Classes.insert(Scope::Hasher()(ptr), ptr);
		{
			RecordVersion++;
			CachedObjectVersion++; // new class = new search path for objects
			return true;
		}
		return false;
	};

	bool Global::RecordUsing(std::shared_ptr<Namespace> ptr) {
		Usings.insert(Scope::Hasher()(ptr), ptr);
		{
			RecordVersion++;
			CachedObjectVersion++;
			return true;
		}
		return false;
	};

	bool Global::RecordFunction(std::string const& Name, Function const& ptr) {
		//if (Functions.emplace(std::hash<Proxy_Function>()(ptr.m_function), std::pair<std::string, std::weak_ptr<details::Proxy_Function_Base>>{ Name, ptr.m_function })) {
		RecordVersion++;
		return true;
		//}
		//return false;
	};

	// Creates a temporary "fake" scope that will act as if it is a global scope, but whose changes will never effect it.
	// Benefits from being able to share the real parent's cached functions and type conversions, which should be a significant performance boost. 
	std::shared_ptr<Global> Global::CreateTemporaryGlobalChild(std::shared_ptr<Global> const& parent) {
		std::shared_ptr<std::string> childName = std::make_shared<std::string>();
		auto globalScope2 = std::shared_ptr<GoodLang::Global>(new GoodLang::Global(), [childName](GoodLang::Global* p) {
			// p->RemoveChild_Unsafe(*childName);
			delete p;
			}); // the "fake" global
		globalScope2->SetName_Unsafe("");
		globalScope2->SetSelf(globalScope2);
		// parent->AddChild(globalScope2);
		globalScope2->SetParent_Unsafe(parent);
		*childName = globalScope2->GetName();

		return globalScope2;
	};

	std::shared_ptr<Global> StartScope(std::shared_ptr<Scope> const& parent) {
		static thread_local std::shared_ptr<Global> globalScope{ nullptr };
		static std::pair<std::mutex, std::shared_ptr<Global>> shared_global{};

		if (parent) {
			if (auto globalParent = std::dynamic_pointer_cast<Global>(parent->GetLibrary())) {
				return globalParent->CreateTemporaryGlobalChild(globalParent);
			}
		}
		if (1) {
			if (!globalScope) {
				shared_global.first.lock();
				if (globalScope = shared_global.second) {
					shared_global.first.unlock();
				}
				else {
					globalScope = shared_global.second = std::make_shared<GoodLang::Global>();
					shared_global.second->SetSelf(shared_global.second);
					shared_global.second->AddBuiltIns();
					shared_global.first.unlock();
				}
			}
			return globalScope->CreateTemporaryGlobalChild(globalScope);
		}

	};





	std::string Global::ToString() const {
		return "Global";
	};
	std::vector< Impl::NodeCache > Global::GetChildren() const {
		return {
			GoodLang::GetChildren(this->p_children),
			GoodLang::GetChildren(this->p_objects),
			GoodLang::GetChildren(this->p_functions)
		};
	};
	bool Global::TryDisconnectChild() const {
		return false;
	};

	namespace Impl {
		void ToString(Tag<Scope>, Scope const& r, std::string& out) {
			out = r.ToString();
		};
		void GetChildren(Tag<Scope>, Scope const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		void TryDisconnectChild(Tag<Scope>, Scope const& r, bool& out) {
			out = r.TryDisconnectChild();
		};

		void ToString(Tag<FunctionScope>, FunctionScope const& r, std::string& out) {
			out = r.ToString();
		};
		void GetChildren(Tag<FunctionScope>, FunctionScope const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		void TryDisconnectChild(Tag<FunctionScope>, FunctionScope const& r, bool& out) {
			out = r.TryDisconnectChild();
		};

		void ToString(Tag<Namespace>, Namespace const& r, std::string& out) {
			out = r.ToString();
		};
		void GetChildren(Tag<Namespace>, Namespace const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		void TryDisconnectChild(Tag<Namespace>, Namespace const& r, bool& out) {
			out = r.TryDisconnectChild();
		};

		void ToString(Tag<Class>, Class const& r, std::string& out) {
			out = r.ToString();
		};
		void GetChildren(Tag<Class>, Class const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		void TryDisconnectChild(Tag<Class>, Class const& r, bool& out) {
			out = r.TryDisconnectChild();
		};

		void ToString(Tag<Global>, Global const& r, std::string& out) {
			out = r.ToString();
		};
		void GetChildren(Tag<Global>, Global const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		void TryDisconnectChild(Tag<Global>, Global const& r, bool& out) {
			out = r.TryDisconnectChild();
		};
	};
};
