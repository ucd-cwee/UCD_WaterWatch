#pragma once 
#include "Scopes.h"

namespace GoodLang {
	bool Scope::AddUsing(std::weak_ptr<Namespace> namespacePtr) {
		if (auto p = std::dynamic_pointer_cast<Scope>(namespacePtr.lock())) {
			if (p_using.emplace(Hasher()(p), namespacePtr)) {
				if (p->IsNamespace()) {
					// if this "namespacePtr" belongs to our same library, then we do not care. We only care to track other libraries being used.
					if (p->GetLibrary() != this->GetLibrary()) {
						(void)RecordUsing(std::dynamic_pointer_cast<Namespace>(p));
					}
				}
				return true;
			}
		}
		return false;
	};
	bool Scope::AddChild(std::shared_ptr<Namespace> NamespacePtr) {
		if (auto p = std::dynamic_pointer_cast<Scope>(NamespacePtr)) {
			auto name = p->GetName();
			auto ptr = p_children.get_or_insert(name, UnorderedMap<size_t, std::shared_ptr<Namespace>>());
			if (ptr->emplace(Hasher()(NamespacePtr), NamespacePtr)) {
				if (p->IsClass()) {
					(void)RecordClass(std::dynamic_pointer_cast<Class>(p));
				}
				return true;
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
		for (auto& childNamespace : this->p_using) {
			if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second.lock())) {
				if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
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
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
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
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
				if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
					if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
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
		for (auto& childNamespace : this->p_using) {
			if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second.lock())) {
				if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
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
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
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
			
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
				if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
					if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}				
			}			
		}

		return false;
	};
	std::shared_ptr< Functions > Scope::GetFunctions() const {
		if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
			return namespacePtr->GetFunctions();
		}
		else {
			return nullptr;
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
	Proxy_Function Scope::GetFunction(std::string const& name, std::vector<Any> const& params, TypeConverter& tree) {
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
		static auto fixNamespace{ [](std::string x) -> std::string {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

			return x;
		} };

		QualifiedOrUnqualifiedNamespaceName = fixNamespace(QualifiedOrUnqualifiedNamespaceName);

		if (QualifiedOrUnqualifiedNamespaceName == "" || QualifiedOrUnqualifiedNamespaceName == "::") { return std::dynamic_pointer_cast<Namespace>(this->GetLibrary()); }

		std::shared_ptr<Namespace> out;

#ifdef useCachedData
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (!this->IsNamespace()) {
			if (this->p_using.size() == 0) {
				if (this->p_children.size() == 0) {
					if (auto p = this->p_parent.lock()) {
						return p->FindNamespace(QualifiedOrUnqualifiedNamespaceName);
					}
				}
			}
		}

		auto treeV = this->GetTypeConverterTreeVersion();
		if (TryGetCached<1>(treeV, out, QualifiedOrUnqualifiedNamespaceName)) {
			return out;
		}

		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (!this->IsNamespace()) {
			if (this->p_using.size() == 0) {
				if (this->p_children.size() == 0) {
					if (auto p = this->p_parent.lock()) {
						return p->FindNamespace(QualifiedOrUnqualifiedNamespaceName);
					}
				}
			}
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
	};
	std::shared_ptr<Class> Scope::FindClass(std::string const& QualifiedOrUnqualifiedNamespaceName) const {
		return std::dynamic_pointer_cast<Class>(FindNamespace(QualifiedOrUnqualifiedNamespaceName));
	};
	std::shared_ptr<Class> Scope::FindClass(std::weak_ptr<Type_Info> typeInfo) const {
		if (auto p = typeInfo.lock()) typeInfo = p->MakeBase(); // revert to base to help with searching

		std::shared_ptr<Namespace> out;

#ifdef useCachedData
		auto treeV = this->GetTypeConverterTreeVersion();
		{
			std::shared_ptr<Class> out2;
			if (TryGetCached<2>(treeV, out2, typeInfo)) {
				return out2;
			}
		}

		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (!this->IsNamespace()) {
			if (this->p_using.size() == 0) {
				if (this->p_children.size() == 0) {
					if (auto p = this->p_parent.lock()) {
						return p->FindClass(typeInfo);
					}
				}
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
	std::shared_ptr<Scope> Scope::FindScopeWithObj(std::string objName) const {
		static auto fixNamespace{ [](std::string x) -> std::string {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

			return x;
		} };
		objName = fixNamespace(objName);

#ifdef useCachedData
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (!this->IsNamespace()) {
			if (this->p_using.size() == 0) {
				if (this->p_children.size() == 0) {
					if (auto objFound = GetObj(objName)) {
						return p_self.lock();
					}
					if (auto p = this->p_parent.lock()) {
						return p->FindScopeWithObj(objName);
					}
				}
			}
		}

		auto treeV = this->GetTypeConverterTreeVersion();
		{
			std::shared_ptr<Scope> out;

			if (TryGetCached<3>(treeV, out, objName)) {
				if (out->TryFindNearestScopeWhere(out, [&objName](std::shared_ptr<Scope> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
						if (auto objFound = ptr->GetObj(objName)) {
							return true;
						}
					}
					return false;
					})) {
					return out;
				}
			}
		}
#endif

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			std::shared_ptr<Scope> out;
			if (TryFindNearestScopeWhere(out, [&objName](std::shared_ptr<Scope> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (auto objFound = ptr->GetObj(objName)) {
						return true;
					}
				}
				return false;
				})) {
				InsertCached<3>(treeV, out, objName);
				return out;
			}
			else {
				InsertCached<3>(treeV, nullptr, objName);
				return nullptr;
			}
		}
		else {
			std::string Namespace = objName.substr(0, lastOfColons - 1);
			objName = objName.substr(lastOfColons + 1);
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
				auto out = namespacePtr->FindScopeWithObj(objName);
				InsertCached<3>(treeV, out, objName);
				return out;
			}
			else {
				InsertCached<3>(treeV, nullptr, objName);
				return nullptr;
			}
		}
	};
	std::shared_ptr<Any> Scope::FindObj(std::string objName) const {
		static auto fixNamespace{ [](std::string x) -> std::string {
			while (x.find("::") == 0 && x.length() > 2) {
				x = x.substr(2);
			}

			while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

			return x;
		} };
		objName = fixNamespace(objName);

		auto lastOfColons = objName.find_last_of("::");
		if ((lastOfColons == std::string::npos) || (lastOfColons == 0)) {
			if (auto ptr = std::dynamic_pointer_cast<Scope>(FindScopeWithObj(objName))) {
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
	std::shared_ptr<Namespace> Scope::FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
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
	Proxy_Function Scope::FindFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
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
	size_t Scope::GetTypeConverterTreeVersion() const {
		if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
			return p->GetTypeConverterTreeVersion();
		}
		else {
			return 0;
		}
	};
	std::shared_ptr<TypeConverter> Scope::GetTypeConverterTree() const {
		if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
			return p->GetTypeConverterTree();
		}
		else {
			return nullptr;
		}
	};
	std::shared_ptr<Namespace> Scope::FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params) {
		return FindNamespaceWithFunction(functionName, params, *GetTypeConverterTree());
	};
	Proxy_Function Scope::FindFunction(std::string functionName, std::vector<Any> const& params) {
		auto tree = GetTypeConverterTree();
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
	bool Scope::TryFindFunctionImpl(std::string const& functionName, std::vector<Any>  const& params, std::shared_ptr<TypeConverter> const& m_conversionTree, Proxy_Function& out) const {
		if (!m_conversionTree) return false;
#ifdef useCachedData
		auto paramsHash = ParamTypes::CalculateHash(params);
		auto treeV = this->GetTypeConverterTreeVersion();
		if (TryGetCached<0>(treeV, out, functionName, paramsHash)) {
			return (bool)out;
		}
		defer(if (out) InsertCachedIfNotExist<0>(treeV, out, functionName, paramsHash));
#endif
		//auto tree_hash = Hasher()(std::weak_ptr<TypeConverter>(m_conversionTree));
		//auto cache1 = GetCache(FindFunctionCache, tree_hash);
		//defer(EraseAllCacheExcept(FindFunctionCache, tree_hash));
		//auto cache2 = GetCache(GetCache(FindFunctionCache, tree_hash), functionName);
		//if (TryGetCache(cache2, params, out)) 
		//	return (bool)out;
		//defer(EmplaceCache(cache2, params, out));

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
#if 1
					(void)firstParamScopePtr->FindNearestNamespaceWhere([&sort, &functionName, &params, &m_conversionTree](std::shared_ptr<Namespace> const& namespace_ptr) -> bool {
						if (auto scope = std::dynamic_pointer_cast<Scope>(namespace_ptr)) {
							if (auto ptr = scope->GetFunctions()) {
								if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
									auto cost = func->conversion_cost(params, *m_conversionTree);
									if (cost < details::TypeConversionWorstCaseCost) {
										sort.emplace(cost, func);
									}
									// return true; // found a nearby match? Give up? Add a give-up criteria? 
								}
							}
						}
						return false;
						});
#else
					if (auto ptr = firstParamScopePtr->GetFunctions()) {
						if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
							// The function is available and requires (potentially) conversion of other parameters. 
							out = func;
							// EmplaceCache(cache2, params, out);
							return true;
						}
					}
#endif
				}

				// try to find the function from nearby scopes... 
				(void)FindNearestNamespaceWhere([&sort, &functionName, &params, &m_conversionTree](std::shared_ptr<Namespace> const& namespace_ptr) -> bool {
					if (auto scope = std::dynamic_pointer_cast<Scope>(namespace_ptr)) {
						if (auto ptr = scope->GetFunctions()) {
							if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
								auto cost = func->conversion_cost(params, *m_conversionTree);
								if (cost < details::TypeConversionWorstCaseCost) {
									sort.emplace(cost + 1, func);
								}
								// return true; // found a nearby match? Give up? Add a give-up criteria? 
							}
						}
					}
					return false;
					});

				// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, BUT NO TEMPLATES)
				if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
					// Is there a pre-defined constructor that this could work with?
					if (auto functions = constructorScopePtr->GetFunctions()) {
						if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
							auto cost = func->conversion_cost(params, *m_conversionTree);
							if (cost < details::TypeConversionWorstCaseCost) {
								sort.emplace(cost + 2, func);
							}
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
						if (auto ptr = scope->GetFunctions()) {
							if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
								auto cost = func->conversion_cost(params, *m_conversionTree);
								if (cost < details::TypeConversionWorstCaseCost) {
									sort.emplace(cost + 3, func);
								}
							}
						}
					}
				}
				if (firstParamScopePtr) {
					for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
						if (scope) {
							if (auto ptr = scope->GetFunctions()) {
								if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
									auto cost = func->conversion_cost(params, *m_conversionTree);
									if (cost < details::TypeConversionWorstCaseCost) {
										sort.emplace(cost + 4, func);
									}
								}
							}
						}
					}
				}
				// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, AND TEMPLATES)
				if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
					// Is there a pre-defined constructor that this could work with?
					if (auto functions = constructorScopePtr->GetFunctions()) {
						if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
							auto cost = func->conversion_cost(params, *m_conversionTree);
							if (cost < details::TypeConversionWorstCaseCost) {
								sort.emplace(cost + 5, func);
							}
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
					if (auto functions = constructorScopePtr->GetFunctions()) {
						if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
							if (func->conversion_cost(params, *m_conversionTree) < details::TypeConversionWorstCaseCost) {
								out = func;
								return true;
							}
						}
					}
				}
			}
		}
		else {
			std::string functionNameActual{ functionName.substr(lastOfColons + 1) };
			std::string scopeName{ functionName.substr(0, lastOfColons - 1) };

			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(scopeName))) {
				if (namespacePtr->TryFindFunctionImpl(functionNameActual, params, m_conversionTree, out)) {
					return true;
				}
				else {
					return false;
				}
			}
		}
		return false;
	};
	std::pair<Proxy_Function, std::shared_ptr<TypeConverter>> Scope::BuildFunction(std::string const& functionName, std::vector<Any> const& params) const {
		// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
		if (!this->IsNamespace()) {
			if (this->p_using.size() == 0) {
				if (this->p_children.size() == 0) {
					if (auto p = this->p_parent.lock()) {
						return p->BuildFunction(functionName, params);
					}
				}
			}
		}

		std::pair<Proxy_Function, std::shared_ptr<TypeConverter>> out{ nullptr, this->GetTypeConverterTree() };
		if (TryFindFunctionImpl(functionName, params, out.second, out.first)) {
			return out;
		}
		else {
			return { nullptr, nullptr };
		}
	};
	Any Scope::CallFunction(std::string const& functionName, std::vector<Any> const& params) const {
		auto [func, tree] = BuildFunction(functionName, params);
		if (func) {
			return call(func, params, *tree);
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
	Any Scope::CallFunction(Proxy_Function const& func, std::vector<Any> const& params) const {
		auto tree{ this->GetTypeConverterTree() };
		if (func) {
			return call(func, params, *tree);
		}
		else {
			throw std::runtime_error("Empty function was provided to CallFunction with direct instancing -- this shouldn't be allowed normally by design.");
		}
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
						auto tree = this->GetTypeConverterTree();
						if (auto func = inputClass->GetFunction(inputClass->GetName(), {}, *tree)) {
							if (auto inputParamImpl = func->operator()({}, *tree)) {
								return (bool)p_functions->emplace(name, function, overrideIfAlreadyExists);
							}
						}
					}
					catch (...) {}

				}
			}
			return (bool)p_functions->emplace(name, function, overrideIfAlreadyExists);
		}
		return (bool)p_functions->emplace(name, function, overrideIfAlreadyExists);
	};
	std::shared_ptr< Functions > Namespace::GetFunctions() const {
		return p_functions;
	};
	std::shared_ptr< Functions::FunctionSort > Namespace::GetFunctions(std::string const& name) const {
		static auto hasher{ std::hash<std::string>() };
		// movable shared lock
		auto locked{ std::make_shared< std::shared_lock<std::shared_mutex> >(p_functions->m_mut) };
		auto f = p_functions->m_functions.find(hasher(name));
		if (f != p_functions->m_functions.end()) {
			return std::shared_ptr< Functions::FunctionSort >(&f->second.second, [lockedCopy = locked](Functions::FunctionSort*) { if (!lockedCopy) { throw(std::runtime_error("ERR")); }; });
		}
		return nullptr;
	};
	Proxy_Function Namespace::GetFunction(std::string const& name, std::vector<Any> const& params, TypeConverter& tree) {
		return p_functions->BuildMatch(name, params, tree);
	};
	Proxy_Function Namespace::GetFunction(std::string const& name, std::vector<Any> const& params) {
		auto tree = GetTypeConverterTree();
		return GetFunction(name, params, *tree);
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
		for (auto& childNamespace : this->p_using) {
			if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second.lock())) {
				if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
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
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
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
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
				if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
					if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
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
		for (auto& childNamespace : this->p_using) {
			if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace.second.lock())) {
				if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
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
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
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
		}

		// test my parents and their children
		if (auto p = this->p_parent.lock()) {
			if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
				return true;
			}
		}

		// test my children's children.
		for (auto& childNamespace : this->p_children) {
			for (auto& innerChildNamespace : childNamespace.second) {
				if (innerChildNamespace.second) {
					if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace.second)) {
						if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
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
	void Global::CreateTypeConverterTree(std::shared_ptr<TypeConverter>& out) const {
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
	std::shared_ptr<TypeConverter> Global::GetTypeConverterTree() const {
		auto oldVersion = CachedTypeConverterTreeVersion.load();
		if (oldVersion != RecordVersion) {
			auto guard{ std::scoped_lock(const_cast<Global*>(this)->CachedTypeConverterTreeMutex) };
			if (const_cast<Global*>(this)->CachedTypeConverterTreeVersion.CompareExchange(oldVersion, RecordVersion.GetValue())) {
				auto tree = std::make_shared<TypeConverter>();
				CreateTypeConverterTree(tree);
				return const_cast<Global*>(this)->CachedTypeConverterTree = tree;
			}
		}

		if (1) {
			auto guard{ std::shared_lock(const_cast<Global*>(this)->CachedTypeConverterTreeMutex) };
			return CachedTypeConverterTree;
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
