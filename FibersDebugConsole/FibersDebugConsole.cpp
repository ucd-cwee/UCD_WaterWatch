// FibersDebugConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <string>
#include "../GoodLang/Any.h"
#include "../GoodLang/Proxy_Function.h"
#include "../GoodLang/ThreadSafeContainers.h"
#include "../GoodLang/Units.h"
#include "../GoodLang/DateTime.h"
#include "../GoodLang/Parallel.h"
#include "../WaterWatchCpp/Clock.h"
#include "../GoodLang/Scopes.h"

#include <concurrent_vector.h>

#include "../FiberTasks/Concurrent_Queue.h"
#include <regex>

class stackThing {
public:
	std::string varName;
	bool perform_cout;

public:
	stackThing() : varName(), perform_cout{ true }{};
	stackThing(std::string const& name) : varName(name), perform_cout{ true } {};
	stackThing(std::string const& name, bool DoCout) : varName(name), perform_cout{ DoCout } {};
	stackThing(stackThing const& r) = default;
	stackThing(stackThing&& r) = default;
	stackThing& operator=(stackThing const& r) = default;
	stackThing& operator=(stackThing&& r) = default;
	~stackThing() { 
		if (perform_cout && (!varName.empty())) {
			std::cout << GoodLang::printf("DELETING %s\n", varName.c_str()) << std::endl;
		}
	};
	
	int length() const { return varName.length(); };
	std::string& get_var_name() { return varName; };
	bool operator==(stackThing const& a) const { return varName == a.varName; };
	bool operator!=(stackThing const& a) const { return varName != a.varName; };
};

#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ_PRINTF(A,B) [a = (A), b = (B)]()->bool{ \
    if (a == b) { return true; } else { \
        std::string tempA{std::to_string(a)}, tempB{std::to_string(b)}; \
        std::cout << GoodLang::printf("FAILURE AT LINE %i: (%s != %s)\n", (int)__LINE__, tempA.c_str(), tempB.c_str()); \
    return false; } \
}()
#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ print(GoodLang::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#define EXPECT_NE(a, b) if (a == b){ print(GoodLang::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }



namespace GoodLang {
	class Scopes {
	public:
		class ScopeID;
		class Breadcrumb; 
		
		class BasicScope;
		class NamespaceScope;
		class ClassScope;
		class RootScope;
		
		enum ScopeType {
			Basic = 1,
			Namespace = 2,
			Class = 4, 
			Root = 8
		};

		// Used to identify a individual scope
		class ScopeID {
		public:
			std::string_view 
				scope_name; // e.g. "Color"
			size_t 
				scope_name_hash; // hash of "Color"
			std::weak_ptr<Scopes::BasicScope>
				scope;
			int
				scope_type;

			ScopeID(int type = ScopeType::Basic, std::string_view scopeName_p = "", std::weak_ptr<Scopes::BasicScope> scope_p = {})
				: scope_type(type)
				, scope_name(scopeName_p)
				, scope_name_hash(scopeName_p == "" ? 0 : GetHash(scopeName_p))
				, scope(scope_p)
			{}

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
			static bool replace(std::string& str, const std::string& from, const std::string& to) {
				size_t start_pos = str.find(from);
				if (start_pos == std::string::npos)
					return false;
				str.replace(start_pos, from.length(), to);
				return true;
			}
			static bool replaceAll(std::string& str, const std::string& from, const std::string& to) {
				if (from.empty())
					return false;
				if (from == to)
					return false;
				size_t start_pos = 0;
				bool retval = false;
				bool retval_t = true;
				while (retval_t) {
					start_pos = 0;
					retval_t = false;
					while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
						str.replace(start_pos, from.length(), to);
						start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
						retval = true;
						retval_t = true;
					}
				}
				//if (str == from) {
				//	str = to;
				//	retval = true;
				//}
				return retval;
			}
			static void RemoveLeadingAndTrailing(std::string_view& x, char what) {
				while (
					(x.length() > 0)
					&& (x[0] == what)
				) {
					x.remove_prefix(1);
				}
				while (
					(x.length() > 0)
					&& ((x[x.length() - 1] == what))
				) {
					x.remove_suffix(1);
				}
			};
			static void RemoveTrailing(std::string_view& x, char what) {
				while (
					(x.length() > 0)
					&& ((x[x.length() - 1] == what))
					) {
					x.remove_suffix(1);
				}
			};
			static std::string_view RemoveLeadingAndTrailing(std::string const& text, char what) {
				std::string_view x(text);
				RemoveLeadingAndTrailing(x, what);
				return x;
			};
			static std::string_view RemoveTrailing(std::string const& text, char what) {
				std::string_view x(text);
				RemoveTrailing(x, what);
				return x;
			};

			// clean-up the name of a scope
			static std::string CleanUpScopeName(std::string x) {	
				x = "::" + x + "::"; // std::string(RemoveLeadingAndTrailing(x, ':'))
				(bool)replaceAll(x, "::::", "::");
				return x;
			};


		public:
			size_t
				hash_m; // the resulting hash of the chain
			ScopeID*
				this_m; // will always point to the owner node's scope ID
			Breadcrumb*
				parent_m; // may be nullptr for root nodes, otherwise will point to the parent breadcrumb node
			Breadcrumb*
				root_m; // may point to this
			Breadcrumb*
				namespace_m; // may point to this
			std::string
				current_namespace; // e.g. "::" or "::UI::Color::"

			Breadcrumb(ScopeID& thisNode, Breadcrumb* parent = nullptr) 
				: hash_m(0) 
				, this_m(&thisNode)
				, parent_m(parent)
				, root_m(nullptr)
				, namespace_m(nullptr)
			{
				// ROOT
				if (parent_m) root_m = parent_m->root_m;
				else root_m = this;

				// NAMESPACE
				if (this_m->is_namespace()) namespace_m = this;
				else if (parent_m) namespace_m = parent_m->namespace_m;
				else namespace_m = this->root_m;

				// HASH
				if (parent_m) GoodLang::details::hash_combine(hash_m, parent_m->hash_m, this_m->scope_name_hash);				
				else hash_m = this_m->scope_name_hash;

				// current_namespace
				if (this_m->scope_name != "") {
					if (parent_m) current_namespace = CleanUpScopeName(parent_m->current_namespace + "::" + std::string(this_m->scope_name) + "::");
					else current_namespace = CleanUpScopeName("::" + std::string(this_m->scope_name) + "::");
				}
				else {
					if (parent_m) current_namespace = parent_m->current_namespace;
					else current_namespace = "::";
				}
			};
			Breadcrumb(Breadcrumb const& other)
				: hash_m(other.hash_m)
				, this_m(other.this_m)
				, parent_m(other.parent_m)
				, root_m(nullptr)
				, namespace_m(nullptr)
			{
				// ROOT
				if (parent_m) root_m = parent_m->root_m;
				else root_m = this;

				// NAMESPACE
				if (this_m->is_namespace()) namespace_m = this;
				else if (parent_m) namespace_m = parent_m->namespace_m;
				else namespace_m = this->root_m;

				// current_namespace
				if (this_m->scope_name != "") {
					if (parent_m) current_namespace = CleanUpScopeName(parent_m->current_namespace + "::" + std::string(this_m->scope_name) + "::");
					else current_namespace = CleanUpScopeName("::" + std::string(this_m->scope_name) + "::");
				}
				else {
					if (parent_m) current_namespace = parent_m->current_namespace;
					else current_namespace = "::";
				}
			};
			Breadcrumb(Breadcrumb && other)
				: hash_m(other.hash_m)
				, this_m(other.this_m)
				, parent_m(other.parent_m)
				, root_m(nullptr)
				, namespace_m(nullptr)
			{
				// ROOT
				if (parent_m) root_m = parent_m->root_m;
				else root_m = this;

				// NAMESPACE
				if (this_m->is_namespace()) namespace_m = this;
				else if (parent_m) namespace_m = parent_m->namespace_m;
				else namespace_m = this->root_m;

				// current_namespace
				if (this_m->scope_name != "") {
					if (parent_m) current_namespace = CleanUpScopeName(parent_m->current_namespace + "::" + std::string(this_m->scope_name) + "::");
					else current_namespace = CleanUpScopeName("::" + std::string(this_m->scope_name) + "::");
				}
				else {
					if (parent_m) current_namespace = parent_m->current_namespace;
					else current_namespace = "::";
				}
			};
			Breadcrumb& operator=(Breadcrumb const& other) {
				hash_m = other.hash_m;
				this_m = other.this_m;
				parent_m = other.parent_m;

				// ROOT
				if (parent_m) root_m = parent_m->root_m;
				else root_m = this;

				// NAMESPACE
				if (this_m->is_namespace()) namespace_m = this;
				else if (parent_m) namespace_m = parent_m->namespace_m;
				else namespace_m = this->root_m;

				// current_namespace
				if (this_m->scope_name != "") {
					if (parent_m) current_namespace = CleanUpScopeName(parent_m->current_namespace + "::" + std::string(this_m->scope_name) + "::");
					else current_namespace = CleanUpScopeName("::" + std::string(this_m->scope_name) + "::");
				}
				else {
					if (parent_m) current_namespace = parent_m->current_namespace;
					else current_namespace = "::";
				}

				return *this;
			};
			Breadcrumb& operator=(Breadcrumb&& other) {
				hash_m = other.hash_m;
				this_m = other.this_m;
				parent_m = other.parent_m;

				// ROOT
				if (parent_m) root_m = parent_m->root_m;
				else root_m = this;

				// NAMESPACE
				if (this_m->is_namespace()) namespace_m = this;
				else if (parent_m) namespace_m = parent_m->namespace_m;
				else namespace_m = this->root_m;

				// current_namespace
				if (this_m->scope_name != "") {
					if (parent_m) current_namespace = CleanUpScopeName(parent_m->current_namespace + "::" + std::string(this_m->scope_name) + "::");
					else current_namespace = CleanUpScopeName("::" + std::string(this_m->scope_name) + "::");
				}
				else {
					if (parent_m) current_namespace = parent_m->current_namespace;
					else current_namespace = "::";
				}

				return *this;
			};
			~Breadcrumb() = default;
		};

		class ObjectWrapper {
		public:
			enum ObjectState {
				Normal = 0,
				Static = 1,
				Constant = 2
			};

			ObjectWrapper(Any const& obj = {}, int s = 0)
				: object{ std::make_shared<Any>(obj) }
				, state { s }
			{
				if (object->GetFlag(AnyData::Flag::constant)) {
					state = state | Constant;
				}
				if (is_const()) {
					object->SetFlag(AnyData::Flag::constant, true);
				}
			};

			std::shared_ptr<Any> object;
			int state = 0;

			bool is_const() const {
				return state & Constant;
			};
			bool is_static() const {
				return state & Static;
			};
		};

		class BasicScope {
		friend class NamespaceScope;
		friend class ClassScope;
		friend class RootScope;
		protected:			
			ScopeID
				self_id_m; // this scope's identifiers, including its name and type
			Breadcrumb
				breadcrumb_m; // contains the self ptr
			GoodLang::details::flat_map< std::string, ObjectWrapper >
				objects_m; // Objects that live inside this scope
			GoodLang::details::flat_map< Breadcrumb*, Breadcrumb* >
				using_m; 

		public:
			BasicScope(std::string_view name = "", int type = ScopeType::Basic, std::shared_ptr< BasicScope > const& parent = nullptr)
				: self_id_m(type, name)
				, breadcrumb_m(self_id_m)
				, objects_m()
			{
				if (parent)
					breadcrumb_m = Breadcrumb(self_id_m, &parent->breadcrumb_m);
				else
					breadcrumb_m = Breadcrumb(self_id_m);
			};
			BasicScope(BasicScope const&) = delete;
			BasicScope(BasicScope &&) = delete;
			BasicScope& operator=(BasicScope const&) = delete;
			BasicScope& operator=(BasicScope&&) = delete;
			virtual ~BasicScope() = default;
			virtual void SetSelf(std::shared_ptr<BasicScope> const& Self) {
				self_id_m.scope = Self;
			};

			std::string_view Name() const { return self_id_m.scope_name; };
			std::string_view CurrentNamespacePath(bool removeLeadingAndTrailingColons = true) const { 
				std::string_view out{ this->breadcrumb_m.current_namespace };
				if (removeLeadingAndTrailingColons) Breadcrumb::RemoveLeadingAndTrailing(out, ':');
				return out;
			};

			// Returns true if this scope is a namespace scope
			bool is_namespace() const {
				return self_id_m.is_namespace();
			};
			// Returns true if this scope is a class scope
			bool is_class() const {
				return self_id_m.is_class();
			};
			// Returns true if this scope is a root scope
			bool is_root() const {
				return self_id_m.is_root();
			};

			// Get the immediate parent
			std::shared_ptr< BasicScope > GetParent() const {
				if (breadcrumb_m.parent_m) {
					return breadcrumb_m.parent_m->this_m->scope.lock();
				}
				else {
					return nullptr;
				}
			};
			// Get the current namespace (for inserting functions, etc)
			std::shared_ptr< NamespaceScope > GetNamespace() const {
				if (breadcrumb_m.namespace_m) {
					return std::dynamic_pointer_cast<NamespaceScope>(breadcrumb_m.namespace_m->this_m->scope.lock());
				}
				else {
					return nullptr;
				}
			};
			// Get the root of the entire scope tree
			std::shared_ptr< RootScope > GetRoot() const {
				if (breadcrumb_m.root_m) {
					return std::dynamic_pointer_cast<RootScope>(breadcrumb_m.root_m->this_m->scope.lock());
				}
				else {
					return nullptr;
				}
			};
			// Get the pointer to the self
			std::shared_ptr< BasicScope > GetSelf() const {
				return self_id_m.scope.lock();
			};

			enum SearchState {
				MayFindObject = 1,
				MayFindFunction = 2,				
				SearchingParents = 4,
				SearchingUsings = 8,
				SearchingChildren = 16,
				SearchUpHitNamespace = 32,
				SkipChildren = 64,
				SkipParent = 128
			};
			virtual Breadcrumb* FindNearestScopeWhere(
				std::function<bool(Breadcrumb*, int)> const& func,
				int searchState = MayFindObject | MayFindFunction,
				std::set< Breadcrumb* > const& CheckedSelf = {},
				std::set< Breadcrumb* > const& CheckedAll = {}
			) const {
				auto& checkedSelf = const_cast<std::set< Breadcrumb* >&>(CheckedSelf);
				auto& checkedAll = const_cast<std::set< Breadcrumb* >&>(CheckedAll);
				
				auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);
				
				if (selfPtr.this_m->is_namespace()) {
					searchState = searchState | SearchUpHitNamespace;
				}

				// Prevent Duplication
				if (checkedAll.count(&selfPtr) > 0) { return nullptr; }
				if (!(searchState & SkipChildren)) checkedAll.emplace(&selfPtr);
				
				// test myself directly	
				if (checkedSelf.count(&selfPtr) <= 0) {
					checkedSelf.emplace(&selfPtr);
					if (func(&selfPtr, searchState)) {
						return &selfPtr;
					}
				}

				// test my personal "using" namespaces completely
				if (using_m.size() > 0ull) {
					for (auto& childNamespace : using_m) {
						if (childNamespace.first && *childNamespace.first) {
							if (checkedSelf.count(*childNamespace.first) > 0) continue;
							if (auto ptr = (*childNamespace.first)->this_m->scope.lock()) {
								if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
									return result;
								}
							}
						}
					}
				}

				// test all of my parents directly -- hoping to quickly find "it"
				if (!(searchState & SkipParent)) {
					Breadcrumb* thisParent = &selfPtr;
					while (thisParent = thisParent->parent_m) {
						if (checkedSelf.count(thisParent) > 0) break;
						checkedSelf.emplace(thisParent);
						if (func(thisParent, searchState | SearchingParents | SkipChildren)) {
							return thisParent;
						}
						// check the using statements of the parent.
						if (auto ptr = thisParent->this_m->scope.lock()) {
							if (ptr->using_m.size() > 0ull) {
								for (auto& childNamespace : ptr->using_m) {
									if (childNamespace.first && *childNamespace.first) {
										if (checkedSelf.count(*childNamespace.first) > 0) continue;
										if (auto ptr = (*childNamespace.first)->this_m->scope.lock()) {
											if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
												return result;
											}
										}
									}
								}
							}
						}
					}
				}

				// Test my parent completely.
				if (!(searchState & SkipParent)) {
					if (selfPtr.parent_m) {
						if (auto ptr = selfPtr.parent_m->this_m->scope.lock()) {
							if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingParents, CheckedSelf, CheckedAll)) {
								return result;
							}
						}
					}
				}

				return nullptr;
			};

			std::shared_ptr<NamespaceScope> FindNamespace(std::string name) const {
				name = Breadcrumb::CleanUpScopeName(name); // instead of "Color", it searches for "::Color::"
				auto len = name.length();
				if (auto BC = FindNearestScopeWhere([&len, &name](Breadcrumb* namespacePtr, int search_state)->bool {
					if (!namespacePtr->this_m->is_namespace()) return false;
					long long QualifiedNameLen = namespacePtr->current_namespace.length();
					auto F = namespacePtr->current_namespace.rfind(name);
					if ((F != std::string::npos) && (F == (QualifiedNameLen - len))) return true;
					return false;
				})) {
					return std::dynamic_pointer_cast<NamespaceScope>(BC->this_m->scope.lock());
				}
				else {
					return nullptr;
				}
			};
			std::shared_ptr<ClassScope> FindClass(std::string name) const {
				name = Breadcrumb::CleanUpScopeName(name); // instead of "Color", it searches for "::Color::"
				auto len = name.length();
				if (auto BC = FindNearestScopeWhere([&len, &name](Breadcrumb* namespacePtr, int search_state)->bool {
					if (!namespacePtr->this_m->is_class()) return false;
					long long QualifiedNameLen = namespacePtr->current_namespace.length();
					auto F = namespacePtr->current_namespace.rfind(name);
					if ((F != std::string::npos) && (F == (QualifiedNameLen - len))) return true;
					return false;
				})) {
					return std::dynamic_pointer_cast<ClassScope>(BC->this_m->scope.lock());
				}
				else {
					return nullptr;
				}
			};
			std::shared_ptr<ClassScope> FindClass(std::shared_ptr<Type_Info> const& type) const {
				auto hash = GetHash(type);
				if (auto BC = FindNearestScopeWhere([&hash](Breadcrumb* namespacePtr, int search_state)->bool {
					if (!namespacePtr->this_m->is_class()) return false;
					if (auto p = std::dynamic_pointer_cast<ClassScope>(namespacePtr->this_m->scope.lock())) {
						if (GetHash(p->ClassType) == hash) {
							return true;
						}
					}
					return false;
				})) {
					return std::dynamic_pointer_cast<ClassScope>(BC->this_m->scope.lock());
				}
				else {
					return nullptr;
				}
			};

			// Explicitely looks for an object with the given name. Name may include specialization for the namespace to expect the object in. E.g.: 
			// std::string::npos -> finds namespace "::std::string::" and finds object "npos"
			std::shared_ptr<Any> FindObject(std::string_view Name) const {
				auto name = Breadcrumb::CleanUpScopeName(std::string(Name)); // "npos" -> "::npos::"   OR    "std::string::npos" -> "::std::string::npos::"    
				auto sv = Breadcrumb::RemoveTrailing(name, ':'); // "npos" -> "::npos"   OR    "std::string::npos" -> "::std::string::npos"
				std::string objName;
				std::string_view namespaceName;
				if (auto f = sv.rfind("::"); f != std::string::npos) {
					objName = std::string(sv.substr(f + 2, sv.length() - f));
					namespaceName = sv.substr(0, f + 2);
				}

				std::shared_ptr<Any> out{ nullptr };
				if (auto BC = FindNearestScopeWhere([&namespaceName, &objName, &sv, &out](Breadcrumb* namespacePtr, int search_state)->bool {
					bool StaticOnly = (search_state & SearchUpHitNamespace) | (search_state & SearchingChildren);
					// we hit a namespace during our search up the node tree -- this prevents us from finding any more objects except for the static type. 

					if (namespaceName.length() > 0) {

						long long QualifiedNameLen = namespacePtr->current_namespace.length();
						auto F = namespacePtr->current_namespace.rfind(namespaceName);
						if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
							// the namespace is technically a candidate
							if (auto ptr = namespacePtr->this_m->scope.lock()) {
								auto f = ptr->objects_m.find(objName);
								if (f != ptr->objects_m.end()) {
									if (StaticOnly && !f->second->is_static()) return false;
									out = f->second->object;
									return true;
								}
							}
						}
					}
					else {
						// any namespace is technically a candidate
						if (auto ptr = namespacePtr->this_m->scope.lock()) {
							auto f = ptr->objects_m.find(objName);
							if (f != ptr->objects_m.end()) {
								if (StaticOnly && !f->second->is_static()) return false;
								out = f->second->object;
								return true;
							}
						}
					}
					return false;
				})) {
					return out;
				}
				else {
					return out;
				}
			};

			// Explicitely looks for a function with the given name. Name may include specialization for the namespace to expect the function in. E.g.: 
			// std::string::npos -> finds namespace "::std::string::" and finds function "npos"
			Proxy_Function FindFunction(std::string_view Name, ParamTypes const& params) const {
				std::string name = Breadcrumb::CleanUpScopeName(std::string(Name)); // "npos" -> "::npos::"   OR    "std::string::npos" -> "::std::string::npos::"    
				auto sv = Breadcrumb::RemoveTrailing(name, ':'); // "npos" -> "::npos"   OR    "std::string::npos" -> "::std::string::npos"
				std::string objName;
				std::string_view namespaceName;
				if (auto f = sv.rfind("::"); f != std::string::npos) {
					objName = std::string(sv.substr(f + 2, sv.length() - f));
					namespaceName = sv.substr(0, f + 2);
				}

				auto converter = this->GetRoot()->GetTypeConverterTree();
				Proxy_Function out{ nullptr };

				std::shared_ptr<ClassScope> firstParamScopePtr{ nullptr };
				std::shared_ptr<ClassScope> constructorScopePtr{ nullptr };

				// FIRST SEARCH PREFERENCES EXACT MATCHES
				// SECOND SEARCH DOES ALLOW FOR CONVERSIONS, BUT NO TEMPLATES
				// FINAL SEARCH ALLOWS FOR TEMPLATE FUNCTIONS
				if (1) {
					std::array<std::multimap<double, Proxy_Function>,3> sort;

					// FIRST, WE CHECK TO SEE IF THE DESIRED FUNCTION IS AVAILABLE FROM THE CLASS OF THE FIRST PARAM (e.g. to_string(Units::foot()) would search the Units::foot class before anything else)
					{
						auto firstParam = params.begin();
						if (firstParam != params.end()) {
							firstParamScopePtr = this->FindClass(firstParam->lock());
							if (!firstParamScopePtr) {
								if (auto typePtr = firstParam->lock()) {
									if (!typePtr->IsBuiltInType()) {
										firstParamScopePtr = this->FindClass(std::dynamic_pointer_cast<Scripted_Type_Info>(typePtr)->m_full_name);
									}
								}
							}
						}
					}

					// While we normally try to minimize the conversion cost, 
					if (firstParamScopePtr) {
						if (firstParamScopePtr->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> bool {
							long long QualifiedNameLen = namespacePtr->current_namespace.length();
							auto F = namespacePtr->current_namespace.rfind(namespaceName);
							if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
								// the namespace is technically a candidate
								if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(namespacePtr->this_m->scope.lock())) {
									//if (out = ptr->functions_m.BuildMatch(objName, params, *converter, false, false)) {
									//	auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
									//	if (cost < details::TypeConversionWorstCaseCost) {
									//		sort[0].emplace(cost, out);
									//	}
									//}
									if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, false, true)) {
										auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
										if (cost < details::TypeConversionWorstCaseCost) {
											sort[1].emplace(cost, out);
										}
									}
									if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, true, true)) {
										auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
										if (cost < details::TypeConversionWorstCaseCost) {
											sort[2].emplace(cost, out);
										}
									}
								}
							}
							return false;
						})) {

						}
					}

					// try to find the function from nearby scopes... 
					if (FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> bool {
						long long QualifiedNameLen = namespacePtr->current_namespace.length();
						auto F = namespacePtr->current_namespace.rfind(namespaceName);
						if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
							// the namespace is technically a candidate
							if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(namespacePtr->this_m->scope.lock())) {
								//if (out = ptr->functions_m.BuildMatch(objName, params, *converter, false, false)) {
								//	auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
								//	if (cost < details::TypeConversionWorstCaseCost) {
								//		sort[0].emplace(cost + 1, out);
								//	}
								//	out = nullptr;
								//}
								if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, false, true)) {
									auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
									if (cost < details::TypeConversionWorstCaseCost) {
										sort[1].emplace(cost + 1, out);
									}
									out = nullptr;
								}
								if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, true, true)) {
									auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
									if (cost < details::TypeConversionWorstCaseCost) {
										sort[2].emplace(cost + 1, out);
									}
									out = nullptr;
								}
							}
						}
						return false;
					})) {

					};

					// walk through it!
					for (auto& s1 : sort) {
						for (auto& s : s1) {
							if (s.first < details::TypeConversionWorstCaseCost) {
								out = s.second;
								// EmplaceCache(cache2, params, out);
								return out;
							}
						}
					}
				}
			};

			// Finds either an object or function with the given name, with preference to objects when searching within a scope. 
			// Name may include specialization for the namespace to expect the object or function in. E.g.: 
			// std::string::npos -> finds namespace "::std::string::" and finds function "npos"
			bool FindObjectOrFunction(std::string_view Name, ParamTypes const& params, std::shared_ptr<Any>& out1, Proxy_Function& out2) const {
				auto name = Breadcrumb::CleanUpScopeName(std::string(Name)); // "npos" -> "::npos::"   OR    "std::string::npos" -> "::std::string::npos::"    
				auto sv = Breadcrumb::RemoveTrailing(name, ':'); // "npos" -> "::npos"   OR    "std::string::npos" -> "::std::string::npos"
				std::string objName;
				std::string_view namespaceName;
				if (auto f = sv.rfind("::"); f != std::string::npos) {
					objName = std::string(sv.substr(f + 2, sv.length() - f));
					namespaceName = sv.substr(0, f + 2);
				}

				auto converter = this->GetRoot()->GetTypeConverterTree();
				Proxy_Function out{ nullptr };

				std::shared_ptr<ClassScope> firstParamScopePtr{ nullptr };
				std::shared_ptr<ClassScope> constructorScopePtr{ nullptr };

				// FIRST SEARCH PREFERENCES EXACT MATCHES
				// SECOND SEARCH DOES ALLOW FOR CONVERSIONS, BUT NO TEMPLATES
				// FINAL SEARCH ALLOWS FOR TEMPLATE FUNCTIONS
				if (1) {
					std::array<std::multimap<double, Proxy_Function>,3> sort;

					// FIRST, WE CHECK TO SEE IF THE DESIRED FUNCTION IS AVAILABLE FROM THE CLASS OF THE FIRST PARAM (e.g. to_string(Units::foot()) would search the Units::foot class before anything else)
					{
						auto firstParam = params.begin();
						if (firstParam != params.end()) {
							firstParamScopePtr = this->FindClass(firstParam->lock());
							if (!firstParamScopePtr) {
								if (auto typePtr = firstParam->lock()) {
									if (!typePtr->IsBuiltInType()) {
										firstParamScopePtr = this->FindClass(std::dynamic_pointer_cast<Scripted_Type_Info>(typePtr)->m_full_name);
									}
								}
							}
						}
					}

					// While we normally try to minimize the conversion cost, 
					if (firstParamScopePtr) {
						if (firstParamScopePtr->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> bool {
							bool StaticOnly = (search_state & SearchUpHitNamespace) | (search_state & SearchingChildren);
							long long QualifiedNameLen = namespacePtr->current_namespace.length();
							auto F = namespacePtr->current_namespace.rfind(namespaceName);
							if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
								// the namespace is technically a candidate
								if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(namespacePtr->this_m->scope.lock())) {
									if (1) {
										auto f = ptr->objects_m.find(objName);
										if (f != ptr->objects_m.end()) {
											if (!(StaticOnly && !f->second->is_static())) {
												if (params.size() == 0) {
													out1 = f->second->object;
													return true;
												}
												else if (f->second->object && f->second->object->IsTypeOf<Proxy_Function>()) {
													out1 = f->second->object;
													return true;
												}
											}
										}
									}
									if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, false, true)) {
										auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
										if (cost == 0) {
											out2 = out;
											return true;
										}
										if (cost < details::TypeConversionWorstCaseCost) {
											sort[1].emplace(cost, out);
										}
									}
									if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, true, true)) {
										auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
										if (cost == 0) {
											out2 = out;
											return true;
										}
										if (cost < details::TypeConversionWorstCaseCost) {
											sort[2].emplace(cost, out);
										}
									}
								}
							}
							return false;
						})) {
							return true;
						}
					}

					// try to find the function from nearby scopes... 
					if (FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state) -> bool {
						bool StaticOnly = (search_state & SearchUpHitNamespace) | (search_state & SearchingChildren);
						long long QualifiedNameLen = namespacePtr->current_namespace.length();
						auto F = namespacePtr->current_namespace.rfind(namespaceName);
						if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
							// the namespace is technically a candidate
							if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(namespacePtr->this_m->scope.lock())) {
								if (1) {
									auto f = ptr->objects_m.find(objName);
									if (f != ptr->objects_m.end()) {
										if (!(StaticOnly && !f->second->is_static())) {
											if (params.size() == 0) {
												out1 = f->second->object;
												return true;
											}
											else if (f->second->object && f->second->object->IsTypeOf<Proxy_Function>()) {
												out1 = f->second->object;
												return true;
											}
										}
									}
								}
								if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, false, true)) {
									auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
									if (cost == 0) {
										out2 = out;
										return true;
									}
									if (cost < details::TypeConversionWorstCaseCost) {
										sort[1].emplace(cost + 1, out);
									}
									out = nullptr;
								}
								if (out = ptr->functions_m.BuildMatch(objName, const_cast<ParamTypes&>(params), *converter, true, true)) {
									auto cost = out->conversion_cost(params, out->GetSignature().Arguments().Types(), *converter);
									if (cost == 0) {
										out2 = out;
										return true;
									}
									if (cost < details::TypeConversionWorstCaseCost) {
										sort[2].emplace(cost + 1, out);
									}
									out = nullptr;
								}
							}
						}
						return false;
					})) {
						return true;
					};

					// walk through it!
					for (auto& s1 : sort) {
						for (auto& s : s1) {
							if (s.first < details::TypeConversionWorstCaseCost) {
								out2 = s.second;
								return true;
							}
						}
					}
				}
			};

			// Emplaces an object with the given name. Name may include specialization for the namespace to place the object in. E.g.:
			// std::string::npos -> finds namespace "::std::string::" and emplaces object "npos"
			std::shared_ptr<Any> EmplaceObject(std::string_view Name, Any const& object, int objectState = ObjectWrapper::ObjectState::Normal)  {
				auto name = Breadcrumb::CleanUpScopeName(std::string(Name)); // "npos" -> "::npos::"   OR    "std::string::npos" -> "::std::string::npos::"    
				auto sv = Breadcrumb::RemoveTrailing(name, ':'); // "npos" -> "::npos"   OR    "std::string::npos" -> "::std::string::npos"
				std::string objName;
				std::string_view namespaceName;
				if (auto f = sv.rfind("::"); f != std::string::npos) {
					objName = std::string(sv.substr(f + 2, sv.length() - f));
					namespaceName = sv.substr(0, f + 2);
				}
				std::shared_ptr<Any> out{ nullptr };
				if (auto BC = FindNearestScopeWhere([&namespaceName, &objName, &sv, &object, &out, &objectState](Breadcrumb* namespacePtr, int search_state)->bool {
					if (namespaceName.length() > 0) {
						long long QualifiedNameLen = namespacePtr->current_namespace.length();
						auto F = namespacePtr->current_namespace.rfind(namespaceName);
						if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
							// the namespace is technically a candidate
							if (auto ptr = namespacePtr->this_m->scope.lock()) {
								if (namespacePtr->this_m->is_class()) {
									if (objectState & ObjectWrapper::ObjectState::Static) {
										out = ptr->objects_m.emplace(objName, ObjectWrapper(object, objectState)).second.get().object;
									}
									else { // the user is adding a member object, NOT a static object. 
										if (auto classPtr = std::dynamic_pointer_cast<ClassScope>(ptr)) {
											if (classPtr->ClassType->IsBuiltInType()) {
												// cannot add member vars to a built-in type. This should not be possible in a real script.
												throw std::runtime_error("Attempting to add a member object to a built-in type, such as an int or double. This is not legal.");
											}
											else {
												classPtr->DeclareMemberObject(objName, object.Type(), std::make_shared<Any>(object));
											}
										}										
									}
								}
								else {
									if (namespacePtr->this_m->is_namespace() && !namespacePtr->this_m->is_root()) {
										out = ptr->objects_m.emplace(objName, ObjectWrapper(object, objectState | ObjectWrapper::ObjectState::Static)).second.get().object;
									}
									else {
										out = ptr->objects_m.emplace(objName, ObjectWrapper(object, objectState)).second.get().object;
									}
								}
								return true;
							}
						}
					}
					else {
						// any namespace is technically a candidate
						if (auto ptr = namespacePtr->this_m->scope.lock()) {
							if (namespacePtr->this_m->is_namespace() && !namespacePtr->this_m->is_root()) {
								out = ptr->objects_m.emplace(objName, ObjectWrapper(object, objectState | ObjectWrapper::ObjectState::Static)).second.get().object;
							}
							else {
								out = ptr->objects_m.emplace(objName, ObjectWrapper(object, objectState)).second.get().object;
							}
							return true;							
						}
					}
					return false;
				})) {
					return out;
				}
				else {
					return out;
				}
			};

			// Emplaces an Function with the given name. Name may include specialization for the namespace to place the function in. E.g.:
			// std::string::npos -> finds namespace "::std::string::" and emplaces function "npos"
			bool EmplaceFunction(std::string_view Name, Function const& object)  {
				auto name = Breadcrumb::CleanUpScopeName(std::string(Name)); // "npos" -> "::npos::"   OR    "std::string::npos" -> "::std::string::npos::"    
				auto sv = Breadcrumb::RemoveTrailing(name, ':'); // "npos" -> "::npos"   OR    "std::string::npos" -> "::std::string::npos"
				std::string objName;
				std::string_view namespaceName;
				if (auto f = sv.rfind("::"); f != std::string::npos) {
					objName = std::string(sv.substr(f + 2, sv.length() - f));
					namespaceName = sv.substr(0, f + 2);
				}
				if (auto BC = FindNearestScopeWhere([&namespaceName, &objName, &sv, &object](Breadcrumb* namespacePtr, int search_state)->bool {
					if (namespaceName.length() > 0) {
						long long QualifiedNameLen = namespacePtr->current_namespace.length();
						auto F = namespacePtr->current_namespace.rfind(namespaceName);
						if ((F != std::string::npos) && (F == (QualifiedNameLen - namespaceName.length()))) {
							// the namespace is technically a candidate
							if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(namespacePtr->this_m->scope.lock())) {
								ptr->AddFunction(objName, object);
								return true;
							}
						}
					}
					else {
						// any namespace is technically a candidate
						if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(namespacePtr->this_m->scope.lock())) {
							ptr->AddFunction(objName, object);
							return true;							
						}
					}
					return false;
				})) {
					return true;
				}
				else {
					return false;
				}
			};

			// "Using" a namespace allows you to search that namespace for functions more easily.
			bool AddUsing(std::shared_ptr<NamespaceScope> const& ns_ptr) {
				if (ns_ptr) {
					return (bool)(this->using_m.emplace(&ns_ptr->breadcrumb_m, &ns_ptr->breadcrumb_m).second.get());
				}
				return false;
			};

			Any Call(std::string_view const& functionName, std::vector<Any> const& params) const {
				ParamTypes Params{ params };
				std::shared_ptr<Any> out1;
				Proxy_Function out2;
				
				if (FindObjectOrFunction(functionName, Params, out1, out2)) {
					if (out1) return out1;
					auto converter = this->GetRoot()->GetTypeConverterTree();
					if (out2) return out2->operator()(params, *converter);
				}

				// function was not found with the given params
				std::string params_str;
				for (auto& p : params) {
					std::string className = p.TypeName(); {
						if (auto classPtr = this->FindClass(p.ActualType().lock())) {
							className = classPtr->self_id_m.scope_name;
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
				auto fN = std::string(functionName);
				throw exception::not_found_error(GoodLang::printf("`%s`(%s)", fN.c_str(), params_str.c_str()));
			};

			template <typename T>
			T Cast(Any const& from) const {
				auto ToType = user_type_shared_ptr<T>();
				auto FromType = from.Type().lock();

				// see if it already matches (best option)
				if ((int)FromType->is_const() <= (int)ToType->is_const()) {
					// if (ToType->is_ref()) {
						if (FromType->underlyingHash == ToType->underlyingHash) {
							return from.cast<T>();
						}
					// }
				}

				return Cast(from, user_type_shared<T>()).cast<T>();
			};
			Any Cast(Any const& from, std::weak_ptr<Type_Info> const& To) const {
				auto ToType = To.lock();
				auto FromType = from.Type().lock();

				// see if it already matches (best option)
				if ((int)FromType->is_const() <= (int)ToType->is_const()) {
					//if (ToType->is_ref()) {
						if (FromType->underlyingHash == ToType->underlyingHash) {
							return from;
						}
					//}
				}

				// see if we can convert (fastest option)
				if (auto Tree = GetRoot()->GetTypeConverterTree()) {
					if (Tree->Converts(from, ToType)) {
						try {
							return Tree->Convert(from, ToType);
						}
						catch (exception::bad_any_cast&) {}
					}
				}

				auto ToClass = this->FindClass(ToType);
				if (ToClass) {
					// see if he can convert (fastest option)
					if (auto Tree2 = ToClass->GetRoot()->GetTypeConverterTree()) {
						if (Tree2->Converts(from, ToType)) {
							try {
								return Tree2->Convert(from, ToType);
							}
							catch (exception::bad_any_cast&) {}
						}
					}

					// search for a function that can do it
					if (1) {
						std::vector<Any> params = { from };

						// call a functor from our scope
						try {
							return this->Call(ToClass->self_id_m.scope_name, params);
						}
						catch (exception::not_found_error) {}

						// call a functor from their scope
						try {
							return ToClass->Call(ToClass->self_id_m.scope_name, params);
						}
						catch (exception::not_found_error) {}
					}

					// Failure to cast From -> To
					throw exception::bad_any_cast(FromType, ToType, __LINE__);
				}

				// Failure
				throw exception::not_found_error(GetTypeName(ToType));
			};
			std::string GetTypeName(std::weak_ptr<Type_Info> const& ti) const {
				if (auto p = ti.lock()) {
					if (auto c = std::dynamic_pointer_cast<Scope>(this->FindClass(p))) {
						if (p->is_const()) {
							if (p->is_ref()) {
								return std::string("const ") + c->GetName() + "&";
							}
							else {
								return std::string("const ") + c->GetName();
							}
						}
						else {
							if (p->is_ref()) {
								return c->GetName() + "&";
							}
							else {
								return c->GetName();
							}
						}
					}
					else {
						if (p->is_any()) {
							return "Any";
						}
						else {
							return p->name();
						}
					}
				}
				return user_type<void>().name();
			};








			std::shared_ptr<BasicScope> MakeChildScope() const {
				auto ptr = std::make_shared<BasicScope>("", ScopeType::Basic, GetSelf());
				ptr->SetSelf(ptr);
				return ptr;
			};
		};

		class NamespaceScope : public BasicScope {
		friend class ClassScope;
		friend class BasicScope;
		friend class RootScope;
		protected:
			std::shared_ptr<std::string> name_m;
			GoodLang::details::flat_map< std::string, std::shared_ptr<BasicScope> >
				children_m;
			Functions
				functions_m; // Functions that live inside this scope

			bool AddFunction(std::string_view const& name, Function const& function) {
				const_cast<Function&>(function).m_function->GetSignature().Name(std::string(name));
				const_cast<Function&>(function).m_function->GetSignature().QualifiedName(this->breadcrumb_m.current_namespace + std::string(name));

				// one argument, and this is a class...
				if (this->self_id_m.is_class() && (function.m_function->GetSignature().Arguments().size() == 1)) {
					// the function name matches the class name... 							
					if (!function.m_isCached) {
						if (!function.m_isEplicit) {
							if (auto type_ptr = function.m_function->GetSignature().Arguments().Type(0).lock()) {
								if (!type_ptr->is_any()) {
									if (type_ptr = function.m_function->GetSignature().Returns().lock()) {
										if (!type_ptr->is_any()) {
											// the function seems to meet our needs. Update our list of conversion functions, which will be grabbed when making a conversion tree
											if (auto ptr = std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock())) {
												if (name == ptr->Name()) {
													ptr->conversion_functions->push_back(function);
													// let the Root know that a new conversion function was added
													if (auto Root = std::dynamic_pointer_cast<RootScope>(this->breadcrumb_m.root_m->this_m->scope.lock())) {
														++Root->conversion_function_live_version;
													}
												}
											}
										}
									}
								}
							}
						}
					}

					// To-Do, handle `=` and `:=` operators
				}

				return (bool)functions_m.emplace(std::string(name), function, true);
			};

		public:
			NamespaceScope(std::shared_ptr<std::string> name, int type = ScopeType::Basic | ScopeType::Namespace, std::shared_ptr< BasicScope > const& parent = nullptr)
				: name_m(name)
				, BasicScope(*name, type, parent)
			{};
			NamespaceScope(NamespaceScope const&) = delete;
			NamespaceScope(NamespaceScope&&) = delete;
			NamespaceScope& operator=(NamespaceScope const&) = delete;
			NamespaceScope& operator=(NamespaceScope&&) = delete;
			virtual ~NamespaceScope() = default;

			virtual Breadcrumb* FindNearestScopeWhere(
				std::function<bool(Breadcrumb*, int)> const& func,
				int searchState = MayFindObject | MayFindFunction,
				std::set< Breadcrumb* > const& CheckedSelf = {},
				std::set< Breadcrumb* > const& CheckedAll = {}
			) const override {
				auto& checkedSelf = const_cast<std::set< Breadcrumb* >&>(CheckedSelf);
				auto& checkedAll = const_cast<std::set< Breadcrumb* >&>(CheckedAll);

				auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);

				if (selfPtr.this_m->is_namespace()) {
					searchState = searchState | SearchUpHitNamespace;
				}

				// Prevent Duplication
				if (checkedAll.count(&selfPtr) > 0) { return nullptr; }
				if (!(searchState & SkipChildren)) checkedAll.emplace(&selfPtr);

				// test myself directly	
				if (checkedSelf.count(&selfPtr) <= 0) {
					checkedSelf.emplace(&selfPtr);
					if (func(&selfPtr, searchState)) {
						return &selfPtr;
					}
				}

				// test my personal "using" namespaces completely
				if (using_m.size() > 0ull) {
					for (auto& childNamespace : using_m) {
						if (childNamespace.first && *childNamespace.first) {
							if (checkedSelf.count(*childNamespace.first) > 0) continue;
							if (auto ptr = (*childNamespace.first)->this_m->scope.lock()) {
								if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
									return result;
								}
							}
						}
					}
				}

				// test all of my parents directly -- hoping to quickly find "it"
				if (!(searchState & SkipParent)) {
					Breadcrumb* thisParent = &selfPtr;
					while (thisParent = thisParent->parent_m) {
						if (checkedSelf.count(thisParent) > 0) break;
						checkedSelf.emplace(thisParent);
						if (func(thisParent, searchState | SearchingParents | SkipChildren)) {
							return thisParent;
						}
						// check the using statements of the parent.
						if (auto ptr = thisParent->this_m->scope.lock()) {
							if (ptr->using_m.size() > 0ull) {
								for (auto& childNamespace : ptr->using_m) {
									if (childNamespace.first && *childNamespace.first) {
										if (checkedSelf.count(*childNamespace.first) > 0) continue;
										if (auto ptr = (*childNamespace.first)->this_m->scope.lock()) {
											if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
												return result;
											}
										}
									}
								}
							}
						}
					}
				}

				// Test my children themselves. 
				if ((!(searchState & SkipChildren)) && children_m.size() > 0ull) {
					for (auto& childNamespace : this->children_m) {
						if (childNamespace.second && *childNamespace.second) {
							if (checkedSelf.count(&(*childNamespace.second)->breadcrumb_m) > 0) continue;
							if (func(&(*childNamespace.second)->breadcrumb_m, searchState | SearchingChildren | SkipChildren | SkipParent)) {
								return &(*childNamespace.second)->breadcrumb_m;
							}
						}
					}
				}

				// Test my parent completely.
				if (!(searchState & SkipParent)) {
					if (selfPtr.parent_m) {
						if (auto ptr = selfPtr.parent_m->this_m->scope.lock()) {
							if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingParents, CheckedSelf, CheckedAll)) {
								return result;
							}
						}
					}
				}

				// Test my children completely. 
				if ((!(searchState & SkipChildren)) && children_m.size() > 0ull) {
					for (auto& childNamespace : this->children_m) {
						if (childNamespace.second && *childNamespace.second) {
							if (auto ptr = (*childNamespace.second)->self_id_m.scope.lock()) {
								if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingChildren | SkipParent, CheckedSelf, CheckedAll)) {
									return result;
								}
							}
						}
					}
				}

				return nullptr;
			};

			std::shared_ptr<NamespaceScope> MakeChildNamespace(std::string_view name) {
				auto ptr = std::make_shared<NamespaceScope>(std::make_shared<std::string>(std::string(name)), ScopeType::Basic | ScopeType::Namespace, GetSelf());
				ptr->SetSelf(ptr);
				return std::dynamic_pointer_cast<NamespaceScope>(this->children_m.emplace(std::string(name), std::move(ptr)).second.get());
			};
			std::shared_ptr<ClassScope> MakeChildClass(std::string_view name) {
				auto ptr = std::make_shared<ClassScope>(std::make_shared<std::string>(std::string(name)), GetSelf());				
				ptr->SetSelf(ptr);
				ptr->AddDefaultConstructors();
				return std::dynamic_pointer_cast<ClassScope>(this->children_m.emplace(std::string(name), std::move(ptr)).second.get());
			};
			std::shared_ptr<ClassScope> MakeChildClass(std::string_view name, std::shared_ptr<Type_Info> type) {
				auto ptr = std::make_shared<ClassScope>(std::make_shared<std::string>(std::string(name)), GetSelf(), type);
				ptr->SetSelf(ptr);
				ptr->AddDefaultConstructors();
				return std::dynamic_pointer_cast<ClassScope>(this->children_m.emplace(std::string(name), std::move(ptr)).second.get());
			};
		};

		class ClassScope : public NamespaceScope {
		friend class NamespaceScope;
		friend class BasicScope;
		friend class RootScope;
		protected:
			std::vector<std::weak_ptr<ClassScope>>
				DerivedFrom; // e.g. this class derives from other Classes
			std::shared_ptr<Type_Info>
				ClassType;
			GoodLang::details::flat_map<std::string, std::pair<std::weak_ptr<Type_Info>, std::shared_ptr<Any>>>
				p_declared_member_objects; // declared member objects for the custom, scripted class which will be instantiated upon construction of the scripted class
			std::shared_ptr<concurrency::concurrent_vector<Function>>
				conversion_functions{ std::make_shared<concurrency::concurrent_vector<Function>>() }; // a duplicate list of functions that meet the definition for constructor or conversions to this class type

		public:
			ClassScope(
				std::shared_ptr<std::string> name, 
				std::shared_ptr< BasicScope > const& parent = nullptr,
				std::shared_ptr<Type_Info> type = nullptr,
				std::vector<std::weak_ptr<ClassScope>> inheritance = {} // e.g. this class derives from another Class
			)
				: NamespaceScope(name, ScopeType::Basic | ScopeType::Namespace | ScopeType::Class, parent)
				, DerivedFrom(inheritance)
			{
				for (int i = DerivedFrom.size() - 1; i >= 0; i--) {
					if (auto InteritedClass = DerivedFrom[i].lock()) {
						if (auto InteritedClassType = InteritedClass->ClassType) {
							if (InteritedClassType->IsBuiltInType()) { // cannot include built-in types
								DerivedFrom.erase(DerivedFrom.begin() + i); // remove this inheritance from the list, and consider throwing an error
								// currently not throwing because that would prevent calling the destructor, which is 100% a requirement to prevent a memory leak.
							}
						}
					}
				}

				for (auto& p : DerivedFrom) {
					if (auto ptr = std::dynamic_pointer_cast<NamespaceScope>(p.lock())) {
						this->AddUsing(ptr);
					}
				}

				if ((!type) || (type->is_void())) {
					ClassType = std::dynamic_pointer_cast<Type_Info>(std::make_shared<Scripted_Type_Info>(
						std::string(Breadcrumb::RemoveTrailing(this->breadcrumb_m.parent_m->current_namespace, ':')), std::string(this->self_id_m.scope_name), false, false
					));
				}
				else {
					ClassType = type;
				}	
			};
			ClassScope(ClassScope const&) = delete;
			ClassScope(ClassScope&&) = delete;
			ClassScope& operator=(ClassScope const&) = delete;
			ClassScope& operator=(ClassScope&&) = delete;
			virtual ~ClassScope() = default;
			virtual void SetSelf(std::shared_ptr<BasicScope> const& Self) override {
				self_id_m.scope = Self;
				if (auto Root = std::dynamic_pointer_cast<RootScope>(this->breadcrumb_m.root_m->this_m->scope.lock())) {
					Root->ReferenceConversionFunction(std::dynamic_pointer_cast<ClassScope>(Self), this->conversion_functions);
				}
			};

			virtual Breadcrumb* FindNearestScopeWhere(
				std::function<bool(Breadcrumb*, int)> const& func,
				int searchState = MayFindObject | MayFindFunction,
				std::set< Breadcrumb* > const& CheckedSelf = {},
				std::set< Breadcrumb* > const& CheckedAll = {}
			) const override {
				auto& checkedSelf = const_cast<std::set< Breadcrumb* >&>(CheckedSelf);
				auto& checkedAll = const_cast<std::set< Breadcrumb* >&>(CheckedAll);

				auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);

				if (selfPtr.this_m->is_namespace()) {
					searchState = searchState | SearchUpHitNamespace;
				}

				// Prevent Duplication
				if (checkedAll.count(&selfPtr) > 0) { return nullptr; }
				if (!(searchState & SkipChildren)) checkedAll.emplace(&selfPtr);

				// test myself directly	
				if (checkedSelf.count(&selfPtr) <= 0) {
					checkedSelf.emplace(&selfPtr);
					if (func(&selfPtr, searchState)) {
						return &selfPtr;
					}
				}

				// test my personal "using" namespaces completely
				if (using_m.size() > 0ull) {
					for (auto& childNamespace : using_m) {
						if (childNamespace.first && *childNamespace.first) {
							if (checkedSelf.count(*childNamespace.first) > 0) continue;
							if (auto ptr = (*childNamespace.first)->this_m->scope.lock()) {
								if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
									return result;
								}
							}
						}
					}
				}

				// test my inherited namespace.
				for (auto& Parent : DerivedFrom) {
					if (auto ptr = Parent.lock()) {
						if (checkedSelf.count(&ptr->breadcrumb_m) > 0) continue;
						if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
							return result;
						}
					}
				}

				// test all of my parents directly -- hoping to quickly find "it"
				if (!(searchState & SkipParent)) {
					Breadcrumb* thisParent = &selfPtr;
					while (thisParent = thisParent->parent_m) {
						if (checkedSelf.count(thisParent) > 0) break;
						checkedSelf.emplace(thisParent);
						if (func(thisParent, searchState | SearchingParents | SkipChildren)) {
							return thisParent;
						}
						// check the using statements of the parent.
						if (auto ptr = thisParent->this_m->scope.lock()) {
							if (ptr->using_m.size() > 0ull) {
								for (auto& childNamespace : ptr->using_m) {
									if (childNamespace.first && *childNamespace.first) {
										if (checkedSelf.count(*childNamespace.first) > 0) continue;
										if (auto ptr = (*childNamespace.first)->this_m->scope.lock()) {
											if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingUsings, CheckedSelf, CheckedAll)) {
												return result;
											}
										}
									}
								}
							}
						}
					}
				}

				// Test my children themselves. 
				if ((!(searchState & SkipChildren)) && children_m.size() > 0ull) {
					for (auto& childNamespace : this->children_m) {
						if (childNamespace.second && *childNamespace.second) {
							if (checkedSelf.count(&(*childNamespace.second)->breadcrumb_m) > 0) continue;
							if (func(&(*childNamespace.second)->breadcrumb_m, searchState | SearchingChildren | SkipChildren | SkipParent)) {
								return &(*childNamespace.second)->breadcrumb_m;
							}
						}
					}
				}

				// Test my parent completely.
				if (!(searchState & SkipParent)) {
					if (selfPtr.parent_m) {
						if (auto ptr = selfPtr.parent_m->this_m->scope.lock()) {
							if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingParents, CheckedSelf, CheckedAll)) {
								return result;
							}
						}
					}
				}

				// Test my children completely. 
				if ((!(searchState & SkipChildren)) && children_m.size() > 0ull) {
					for (auto& childNamespace : this->children_m) {
						if (childNamespace.second && *childNamespace.second) {
							if (auto ptr = (*childNamespace.second)->self_id_m.scope.lock()) {
								if (auto* result = ptr->FindNearestScopeWhere(func, searchState | SearchingChildren | SkipParent, CheckedSelf, CheckedAll)) {
									return result;
								}
							}
						}
					}
				}

				return nullptr;
			};

			void ConstructMemberObjects(DynamicObject& obj) const {
				for (auto& Parent : DerivedFrom) {
					if (auto parentType = Parent.lock()) {
						parentType->ConstructMemberObjects(obj);
					}
				}

				for (auto& member_obj : p_declared_member_objects) {
					std::string const& memberObjectName = *member_obj.first;
					if (auto memberObjectType = member_obj.second->first.lock()) {
						if (auto memberObjectClassType = this->FindClass(memberObjectType)) {
							auto& memberObjectDefaultInstance = member_obj.second->second;
							if (memberObjectDefaultInstance) {
								// default value was provided -- try to create a copy.
								try {
									Any defaultParam = memberObjectClassType->Call(memberObjectClassType->self_id_m.scope_name, { memberObjectDefaultInstance });
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
								Any defaultParam = memberObjectClassType->Call(memberObjectClassType->self_id_m.scope_name, {});
								obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
								continue;
							}
						}
					}

					// something went wrong -- set it to void. The class type was not provided, could not be found, or could not be instanced.
					obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>();
				}
			};
			void ConstructMemberObjects(DynamicObject& obj, DynamicObject const& CopyFrom) const {
				for (auto& Parent : DerivedFrom) {
					if (auto parentType = Parent.lock()) {
						parentType->ConstructMemberObjects(obj, CopyFrom);
					}
				}

				for (auto& member_obj : p_declared_member_objects) {
					std::string const& memberObjectName = *member_obj.first;
					if (auto memberObjectType = member_obj.second->first.lock()) {
						if (auto memberObjectClassType = this->FindClass(memberObjectType)) {
							auto copyObjPtr = CopyFrom.m_objects->find(memberObjectName);
							if ((copyObjPtr != CopyFrom.m_objects->end()) && copyObjPtr->second) {
								auto& memberObjectDefaultInstance = copyObjPtr->second;
								if (memberObjectDefaultInstance) {
									// default value was provided -- try to create a copy.
									try {
										Any defaultParam = memberObjectClassType->Call(memberObjectClassType->self_id_m.scope_name, { memberObjectDefaultInstance });
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
									Any defaultParam = memberObjectClassType->Call(memberObjectClassType->self_id_m.scope_name, {});
									obj.m_objects->operator[](memberObjectName) = std::make_shared<Any>(std::move(defaultParam));
									continue;
								}
							}
							else {
								auto& memberObjectDefaultInstance = member_obj.second->second;
								if (memberObjectDefaultInstance) {
									// default value was provided -- try to create a copy.
									try {
										Any defaultParam = memberObjectClassType->Call(memberObjectClassType->self_id_m.scope_name, { memberObjectDefaultInstance });
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
									Any defaultParam = memberObjectClassType->Call(memberObjectClassType->self_id_m.scope_name, {});
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
			std::map<std::string, std::weak_ptr<Type_Info>> GetMemberObjects() const {
				std::map<std::string, std::weak_ptr<Type_Info>> out;

				for (auto x : p_declared_member_objects) {
					out[*x.first] = x.second->first;
				}
				return out;
			};
			// Gets the member objects of this class all all inherited classes (recursively)
			std::map<std::string, std::weak_ptr<Type_Info>> GetAllMemberObjects() const {
				std::map<std::string, std::weak_ptr<Type_Info>> out;

				for (auto& Parent : DerivedFrom) {
					if (auto parentType = Parent.lock()) {
						out = parentType->GetAllMemberObjects();
					}
				}

				for (auto x : p_declared_member_objects) {
					out[*x.first] = x.second->first;
				}
				return out;
			};
			// note, only call this if this class is a scripted type
			void AddDefaultConstructors() {				
				if (ClassType && !ClassType->IsBuiltInType()) {
					// Default constructor
					this->AddFunction(this->self_id_m.scope_name, make_callable([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))]()->Any {
						if (auto self = selfPtr.lock()) {
							DynamicObject out{ self->ClassType };
							self->ConstructMemberObjects(out); // should automatically construct parent's objects in-order 
							return out;
						}
						else {
							throw(exception::not_found_error("Custom class type was no longer available"));
						}
					}, {}, ClassType));

					// Copy constructor
					this->AddFunction(this->self_id_m.scope_name, make_callable([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& from)->Any {
						DynamicObject const& obj = from.cast<DynamicObject const&>();
						if (auto self = selfPtr.lock()) {
							DynamicObject out{ self->ClassType };
							self->ConstructMemberObjects(out, obj);
							return out;
						}
						else {
							throw(exception::not_found_error("Custom class type was no longer available"));
						}
					}, ParamTypes({ ClassType->MakeConstRef() }), ClassType));

					// assignment operator
					this->AddFunction("=", make_callable([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& to, Any const& from)->Any {
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
					}, ParamTypes({ ClassType->MakeRef(), ClassType->MakeConstRef() }), ClassType->MakeRef()));

					// Comparisons
					this->AddFunction("==", make_callable([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& to, Any const& from)-> bool {
						DynamicObject& To = to.cast<DynamicObject&>();
						DynamicObject const& From = from.cast<DynamicObject const&>();
						if (auto self = selfPtr.lock()) {
							bool success = To.m_objects->size() == From.m_objects->size();
							if (!success) return false;
							for (auto& obj : *To.m_objects) {
								success = success && self->Call("==", { obj.second, From.m_objects->at(obj.first) });
								if (!success) return false;
							}
							return success;
						}
						else {
							throw(exception::not_found_error("Custom class type was no longer available"));
						}
					}, ParamTypes({ ClassType->MakeConstRef(), ClassType->MakeConstRef() })));
					this->AddFunction("!=", make_callable([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& to, Any const& from)-> bool {
						if (auto self = selfPtr.lock()) {
							return !self->Cast<bool>(self->Call("==", { to, from }));
						}
						else {
							throw(exception::not_found_error("Custom class type was no longer available"));
						}
					}, ParamTypes({ ClassType->MakeConstRef(), ClassType->MakeConstRef() })));

					//this->GetTypeConverterTree()->AddConverter([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& from)->Any {
					//	DynamicObject const& obj = from.cast<DynamicObject const&>();
					//	if (auto self = selfPtr.lock()) {
					//		DynamicObject out{ self->ClassType };
					//		self->ConstructMemberObjects(out, obj);
					//		return out;
					//	}
					//	else {
					//		throw(exception::not_found_error("Custom class type was no longer available"));
					//	}
					//}, ClassType->MakeBase(), ClassType->MakeBase());
					//this->GetTypeConverterTree()->AddConverter([](Any const& from)->Any {
					//	return from;
					//}, ClassType->MakeBase(), ClassType->MakeRef());
					//this->GetTypeConverterTree()->AddConverter([](Any const& from)->Any {
					//	return from;
					//}, ClassType->MakeBase(), ClassType->MakeConstRef());
					//this->GetTypeConverterTree()->AddConverter([](Any const& from)->Any {
					//	return from;
					//}, ClassType->MakeRef(), ClassType->MakeConstRef());
					//this->GetTypeConverterTree()->AddConverter([](Any const& from)->Any {
					//	return from;
					//}, ClassType->MakeConstRef(), ClassType->MakeConstRef());
					//this->GetTypeConverterTree()->AddConverter([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& from)->Any {
					//	DynamicObject const& obj = from.cast<DynamicObject const&>();
					//	if (auto self = selfPtr.lock()) {
					//		DynamicObject out{ self->ClassType };
					//		self->ConstructMemberObjects(out, obj);
					//		return out;
					//	}
					//	else {
					//		throw(exception::not_found_error("Custom class type was no longer available"));
					//	}
					//}, ClassType->MakeConstRef(), ClassType->MakeBase());

					// upcast constructor for inherited types
					for (auto& derivedFrom : DerivedFrom) {
						if (auto parentClass = derivedFrom.lock()) {
							// Upcast (const&)
							parentClass->AddFunction(parentClass->self_id_m.scope_name, make_callable([selfPtr = std::weak_ptr<ClassScope>(std::dynamic_pointer_cast<ClassScope>(this->self_id_m.scope.lock()))](Any const& from)->Any {
								DynamicObject const& obj = from.cast<DynamicObject const&>();
								if (auto p = selfPtr.lock()) {
									return DynamicObject(p->ClassType, obj);
								}
								else {
									return from;
								}
							}, ParamTypes({ ClassType->MakeConstRef() }), parentClass->ClassType));
						}
					}
				}
			};
			void DeclareMemberObject(std::string const& name, std::weak_ptr<Type_Info> type, std::shared_ptr<Any> defaultValue = nullptr) {
				if (ClassType && !ClassType->IsBuiltInType()) {

					p_declared_member_objects.emplace(name, std::pair<std::weak_ptr<Type_Info>, std::shared_ptr<Any>>{ type, defaultValue });

					// ref access
					this->AddFunction(name, make_callable([objName = name](Any const& from)->Any {
						DynamicObject& From = from.cast<DynamicObject&>();
						auto temp = From.m_objects->at(objName);
						return Any(temp);
						}, ParamTypes({ ClassType->MakeRef() }), type.lock()->MakeRef()));

					// const ref access
					this->AddFunction(name, make_callable([objName = name](Any const& from)->Any {
						DynamicObject const& From = from.cast<DynamicObject const&>();
						auto temp = From.m_objects->at(objName);
						return Any(temp);
						}, ParamTypes({ ClassType->MakeConstRef() }), type.lock()->MakeConstRef()));
				}
			};

		};

		class RootScope : public NamespaceScope {
		friend class NamespaceScope;
		friend class BasicScope;
		friend class ClassScope;
		protected:
			using conversion_function_type = std::pair<std::weak_ptr<ClassScope>, std::weak_ptr<concurrency::concurrent_vector<Function>>>;
			GoodLang::SharedLockable< std::vector<conversion_function_type> >
				conversion_functions; // a duplicate list of functions that meet the definition for constructor or conversions to this class type
			std::atomic<size_t>
				conversion_function_live_version{ 0 };
			GoodLang::shared_ptr< TypeConverter >
				conversion_tree{ GoodLang::make_shared<TypeConverter>() };
			std::atomic<size_t>
				conversion_function_processed_version{ 0 };

			bool ReferenceConversionFunction(std::shared_ptr<ClassScope> whom, std::shared_ptr<concurrency::concurrent_vector<Function>> what) {
				if (whom && what) {
					conversion_functions.Unique()->push_back(conversion_function_type{ std::move(whom), std::move(what) });
					return true;
				}
				return false;
			};
			void ForEachConversionFunction(std::function<void(std::shared_ptr<ClassScope> const&, std::shared_ptr<concurrency::concurrent_vector<Function>> const&)> const& func) {
				std::vector<size_t> to_delete;
				if (1) {
					auto ptr = conversion_functions.Shared();
					for (int i = ptr->size() - 1; i >= 0; --i) {
						auto& indexed = ptr->operator[](i);
						if (auto x = indexed.first.lock()) {
							if (auto y = indexed.second.lock()) {
								func(x, y);
								continue;
							}
						}
						to_delete.push_back(i);
					}
				}
				if (to_delete.size() > 0) {
					auto ptr = conversion_functions.Unique();
					for (auto& i : to_delete) {
						auto& indexed = ptr->operator[](i);
						if (auto x = indexed.first.lock()) {
							if (auto y = indexed.second.lock()) {								
								continue;
							}
						}
						
						ptr->operator[](i) = ptr->operator[](ptr->size() - 1);
						ptr->pop_back();
					}
				}
			};
			void PopulateTypeConverterTree(TypeConverter& out) {
				ForEachConversionFunction([&out](std::shared_ptr<ClassScope> const& whom, std::shared_ptr<concurrency::concurrent_vector<Function>> const& what) {
					if (auto& outputType = whom->ClassType) {
						if (outputType->is_any()) return;
						auto className = whom->self_id_m.scope_name;

						for (auto& constructor : *what) {
							if (!constructor.m_function) continue;
							if (constructor.m_function->NumArguments() != 1) continue;

							// Explicit or cached (e.g. built from previous tree) conversions are not acceptable. 
							// Cached conversions are built from "true" conversions, which will be re-built again by this new tree anyways, so their
							// inclusion in the new tree is by definition not required. It may introduce a speed-up, but introduces lifetime issues and 
							// so I prefer not to include those previous caches. 
							if (constructor.m_isCached) continue;
							if (constructor.m_isEplicit) continue;

							if (auto inputType = constructor.m_function->Argument(0).lock()) {
								// templated conversions are not acceptable
								if (inputType->is_any()) continue;

								out.AddConverter([func = constructor.m_function](Any const& input)->Any {
									return func->operator()(const_cast<Any&>(input));
								}, inputType, outputType);
							}
						}
					}
				});
			};

		public:
			RootScope()
				: NamespaceScope(
					std::make_shared<std::string>(""), 
					Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace | Scopes::ScopeType::Root, 
					nullptr)
			{};
			RootScope(RootScope const&) = delete;
			RootScope(RootScope&&) = delete;
			RootScope& operator=(RootScope const&) = delete;
			RootScope& operator=(RootScope&&) = delete;
			virtual ~RootScope() = default;

			GoodLang::shared_ptr< TypeConverter > GetTypeConverterTree() {
				if (conversion_function_processed_version == conversion_function_live_version) {
					return conversion_tree;
				}
				else {
					conversion_function_processed_version.store(conversion_function_live_version.load());
					auto temp = GoodLang::make_shared<TypeConverter>();
					PopulateTypeConverterTree(*temp);
					return conversion_tree = temp;
				}
			};
		};







	};

};


namespace GoodLang {
	namespace utility {
		struct Static_String {
			template<size_t N>
			constexpr Static_String(const char(&str)[N]) noexcept
				: m_size(N - 1)
				, data(&str[0]) {
			}

			constexpr size_t size() const noexcept { return m_size; }

			constexpr const char* c_str() const noexcept { return data; }

			constexpr auto begin() const noexcept { return data; }

			constexpr auto end() const noexcept { return data + m_size; }

			constexpr bool operator==(std::string_view other) const noexcept {
				// return std::string_view(data, m_size) == other;
				auto b1 = begin();
				const auto e1 = end();
				auto b2 = other.begin();
				const auto e2 = other.end();

				if (e1 - b1 != e2 - b2) {
					return false;
				}

				while (b1 != e1) {
					if (*b1 != *b2) {
						return false;
					}
					++b1;
					++b2;
				}
				return true;
			}

			bool operator==(const std::string& t_str) const noexcept { return std::equal(begin(), end(), std::cbegin(t_str), std::cend(t_str)); }

			operator const char* () const {
				return c_str();
			};

			const size_t m_size;
			const char* data = nullptr;
		};
		template<typename Itr> static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
			std::uint32_t h = 0x811c9dc5;
			while (begin != end) {
				h = (h ^ (*begin)) * 0x01000193;
				++begin;
			}
			return h;
		};
		template<size_t N> static constexpr std::uint32_t hash(const char(&str)[N]) noexcept { return hash(std::begin(str), std::end(str) - 1); };
		static constexpr std::uint32_t hash(std::string_view sv) noexcept { return hash(sv.begin(), sv.end()); };
	};

	namespace Engine {
		using SourceFile = std::string_view;
		BETTER_ENUM(AST_Node_Type, uint32_t,
			File, Noop,
			Id, Reference, Var_Decl, Assign_Decl, Constant,
			Fun_Call, Unused_Return_Fun_Call,
			Arg_List, Arg,
			Equation,
			Array_Call, Dot_Access,
			Lambda,
			FunctionBlock,
			DeclarationBlock, Block, Scopeless_Block,
			Def,
			If,
			Parallel, Parallel_For, Parallel_Ranged_For, For, Ranged_For, While,
			Inline_Array, Inline_Map,
			Return,
			Prefix, Postfix,
			Break, Continue,
			Map_Pair, Value_Range, Inline_Range,
			Do, Try, Catch, Finally,
			Method,
			Attr_Decl,
			Logical_And, Logical_Or,
			Switch, Case, Default,
			Class, Namespace,
			FunctionDecl,
			BinaryFoldRight, Binary,
			Global_Decl,
			Compiled,
			ControlBlock,
			Assign_Retroactively,
			TypeId,
			JustInTimeCompilation,
			AST_Node_Type_end
		);
		BETTER_ENUM(PreprocessorDirectives, uint32_t,
			Completed, None,
			Include, Define, Undefine,
			If, ElseIf, Else, EndIf, IfDefined, IfNotDefined, IfChain,
			Error, Warning, Pragma
		);
		enum Alphabet {
			symbol_alphabet = 0,
			keyword_alphabet,
			int_alphabet,
			float_alphabet,
			x_alphabet,
			hex_alphabet,
			b_alphabet,
			bin_alphabet,
			id_alphabet,
			white_alphabet,
			int_suffix_alphabet,
			float_suffix_alphabet,
			max_alphabet,
			lengthof_alphabet = 256
		};
		enum class Operator_Precedence {
			Ternary_Cond,
			Logical_Or,
			Logical_And,
			Bitwise_Or,
			Bitwise_Xor,
			Bitwise_And,
			Equality,
			Comparison,
			Shift,
			Addition,
			Multiplication,
			Prefix
		};

		struct File_Position {
			int line = 0;
			int column = 0;
			int position = 0;

			constexpr File_Position(int t_file_line, int t_file_column, int pos) noexcept
				: line(t_file_line)
				, column(t_file_column) 
				, position(pos)
			{}

			constexpr File_Position() noexcept = default;
		};
		struct Position {
			operator File_Position() const {
				return File_Position(line, col, pos);
			};
			constexpr Position() = default;
			constexpr Position(const char* t_pos, const char* t_end) noexcept
				: line(1)
				, col(1)
				, pos(0)
				, m_pos(t_pos)
				, m_end(t_end)
				, m_last_col(1) {
			}
			static std::string_view str(const Position& t_begin, const Position& t_end) noexcept {
				if (t_begin.m_pos != nullptr && t_end.m_pos != nullptr) {
					return std::string_view(t_begin.m_pos, std::size_t(std::distance(t_begin.m_pos, t_end.m_pos)));
				}
				else {
					return {};
				}
			}
			constexpr Position& operator++() noexcept {
				if (m_pos != m_end) {
					if (*m_pos == '\n') {
						++line;
						m_last_col = col;
						col = 1;
					}
					else {
						++col;
					}

					++pos;
					++m_pos;
				}
				return *this;
			}
			constexpr Position& operator--() noexcept {
				--pos;
				--m_pos;
				if (*m_pos == '\n') {
					--line;
					col = m_last_col;
				}
				else {
					--col;
				}
				return *this;
			}
			constexpr Position& operator+=(size_t t_distance) noexcept {
				*this = (*this) + t_distance;
				return *this;
			}
			constexpr Position operator+(size_t t_distance) const noexcept {
				Position ret(*this);
				for (size_t i = 0; i < t_distance; ++i) {
					++ret;
				}
				return ret;
			}
			constexpr Position& operator-=(size_t t_distance) noexcept {
				*this = (*this) - t_distance;
				return *this;
			}
			constexpr Position operator-(size_t t_distance) const noexcept {
				Position ret(*this);
				for (size_t i = 0; i < t_distance; ++i) {
					--ret;
				}
				return ret;
			}
			constexpr bool operator==(const Position& t_rhs) const noexcept { return m_pos == t_rhs.m_pos; }
			constexpr bool operator!=(const Position& t_rhs) const noexcept { return m_pos != t_rhs.m_pos; }
			constexpr bool has_more() const noexcept { return m_pos != m_end; }
			constexpr size_t remaining() const noexcept { return static_cast<size_t>(m_end - m_pos); }
			constexpr const char& operator*() const noexcept {
				if (m_pos == m_end) {
					return ""[0];
				}
				else {
					return *m_pos;
				}
			}

			int line = -1;
			int col = -1;
			int pos = -1;

			std::string to_string() const {
				return GoodLang::printf("L%iC%i(#%i)", line, col, pos);
			};

		private:
			const char* m_pos = nullptr;
			const char* m_end = nullptr;
			int m_last_col = -1;
		};

		struct Parse_Location {
			Parse_Location(Position _start, Position _end)
				: start(_start)
				, end(_end)
			{}
			Position start;
			Position end;

			std::string to_string() const {
				auto s = start.to_string();
				auto e = end.to_string();
				return GoodLang::printf("%s - %s", s.c_str(), e.c_str());
			};
		};

		template<typename string_type>
		struct Char_Parser_Helper {
			// common for all implementations
			static std::string u8str_from_ll(long long val) {
				using char_type = std::string::value_type;

				char_type c[2];
				c[1] = char_type(val);
				c[0] = char_type(val >> 8);

				if (c[0] == 0) {
					return std::string(1, c[1]); // size, character
				}

				return std::string(c, 2); // char buffer, size
			}

			static string_type str_from_ll(long long val) {
				using target_char_type = typename string_type::value_type;
				return string_type(1, target_char_type(val)); // size, character
			}
		};
		template<> struct Char_Parser_Helper<std::string> {
			static std::string str_from_ll(long long val) {
				// little SFINAE trick to avoid base class
				return Char_Parser_Helper<std::true_type>::u8str_from_ll(val);
			}
		};
		template<typename Itr> static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
			std::uint32_t h = 0x811c9dc5;
			while (begin != end) {
				h = (h ^ (*begin)) * 0x01000193;
				++begin;
			}
			return h;
		};
		template<size_t N> static constexpr std::uint32_t hash(const char(&str)[N]) noexcept { return hash(std::begin(str), std::end(str) - 1); };
		static constexpr std::uint32_t hash(std::string_view sv) noexcept { return hash(sv.begin(), sv.end()); };

		struct Operators {
			enum class Opers {
				equals,
				less_than,
				greater_than,
				less_than_equal,
				greater_than_equal,
				not_equal,
				assign_if_null,
				assign,
				pre_increment,
				pre_decrement,
				assign_product,
				assign_sum,
				assign_quotient,
				assign_difference,
				assign_bitwise_and,
				assign_bitwise_or,
				assign_shift_left,
				assign_shift_right,
				assign_remainder,
				assign_bitwise_xor,
				shift_left,
				shift_right,
				remainder,
				bitwise_and,
				bitwise_or,
				bitwise_xor,
				bitwise_complement,
				sum,
				quotient,
				product,
				difference,
				unary_plus,
				unary_minus,
				invalid,
				logical_list
			};

			constexpr static const char* to_string(Opers t_oper) noexcept {
				constexpr const char* opers[]
					= { "", "==", "<", ">", "<=", ">=", "!=", "?=", "", "=", "++", "--", "*=", "+=", "/=", "-=", "", "&=", "|=", "<<=", ">>=", "%=", "^=", "", "<<", ">>", "%", "&", "|", "^", "~", "", "+", "/", "*", "-", "+", "-", "", ".." };
				return opers[static_cast<int>(t_oper)];
			}

			constexpr static Opers to_operator(std::string_view t_str, bool t_is_unary = false) noexcept {
				const auto op_hash = hash(t_str);
				switch (op_hash) {
				case hash("?="): {
					return Opers::assign_if_null;
				}
				case hash("=="): {
					return Opers::equals;
				}
				case hash("<"): {
					return Opers::less_than;
				}
				case hash(">"): {
					return Opers::greater_than;
				}
				case hash("<="): {
					return Opers::less_than_equal;
				}
				case hash(">="): {
					return Opers::greater_than_equal;
				}
				case hash("!="): {
					return Opers::not_equal;
				}
				case hash("="): {
					return Opers::assign;
				}
				case hash("++"): {
					return Opers::pre_increment;
				}
				case hash("--"): {
					return Opers::pre_decrement;
				}
				case hash("*="): {
					return Opers::assign_product;
				}
				case hash("+="): {
					return Opers::assign_sum;
				}
				case hash("-="): {
					return Opers::assign_difference;
				}
				case hash("&="): {
					return Opers::assign_bitwise_and;
				}
				case hash("|="): {
					return Opers::assign_bitwise_or;
				}
				case hash("<<="): {
					return Opers::assign_shift_left;
				}
				case hash(">>="): {
					return Opers::assign_shift_right;
				}
				case hash("%="): {
					return Opers::assign_remainder;
				}
				case hash("^="): {
					return Opers::assign_bitwise_xor;
				}
				case hash("<<"): {
					return Opers::shift_left;
				}
				case hash(">>"): {
					return Opers::shift_right;
				}
				case hash("%"): {
					return Opers::remainder;
				}
				case hash("&"): {
					return Opers::bitwise_and;
				}
				case hash("|"): {
					return Opers::bitwise_or;
				}
				case hash("^"): {
					return Opers::bitwise_xor;
				}
				case hash("~"): {
					return Opers::bitwise_complement;
				}
				case hash("+"): {
					return t_is_unary ? Opers::unary_plus : Opers::sum;
				}
				case hash("-"): {
					return t_is_unary ? Opers::unary_minus : Opers::difference;
				}
				case hash("/"): {
					return Opers::quotient;
				}
				case hash("*"): {
					return Opers::product;
				}
				case hash(".."): {
					return Opers::logical_list;
				}
				default: {
					return Opers::invalid;
				}
				}
			};
		};
		struct AST_Node {
		public:
			struct AST_Node_Trace {
				const AST_Node_Type identifier;
				const std::string text;
				Parse_Location location;
				std::vector<AST_Node_Trace> children;

				const File_Position& start() const noexcept { return location.start; }
				const File_Position& end() const noexcept { return location.end; }
				std::string pretty_print() const {
					std::ostringstream oss;

					oss << text;

					for (const auto& elem : children) {
						oss << elem.pretty_print() << ' ';
					}

					return oss.str();
				}
				std::vector<AST_Node_Trace> get_children(const AST_Node& node) {
					const auto node_children = node.get_children();
					return std::vector<AST_Node_Trace>(node_children.begin(), node_children.end());
				};
				AST_Node_Trace(const AST_Node& node)
					: identifier(node.identifier)
					, text(node.text)
					, location(node.location)
					, children(get_children(node)) {
				};
			};

		public:
			const AST_Node_Type identifier;
			const std::string text;
			Parse_Location location;
			std::weak_ptr<Type_Info> return_type;

			const File_Position& start() const noexcept { return location.start; }
			const File_Position& end() const noexcept { return location.end; }

			std::string pretty_print() const {
				std::ostringstream oss;

				oss << text;

				for (auto& elem : get_children()) {
					oss << elem.get().pretty_print() << ' ';
				}

				return oss.str();
			}

			virtual std::vector<std::reference_wrapper<AST_Node>> get_children() const = 0;
			virtual Any eval(std::shared_ptr<Scope> const& currentScope) const = 0;

			/// Prints the contents of an AST node, including its children, recursively
			std::string to_string(const std::string& t_prepend = "") const {
				std::ostringstream oss;
				std::string str = GoodLang::ToString(std::string(this->identifier.ToString()));
				std::string returnType = ToString(this->return_type);
				std::string TimeSpentEvaling = ToString(this->TimeSpent_Total());
				std::string locationStr = this->location.to_string();
				oss << GoodLang::printf("%s(%s) [%s] %s : %s -> %s\n",
					t_prepend.c_str(), str.c_str(), TimeSpentEvaling.c_str(), this->text.c_str(),
					locationStr.c_str(),
					returnType.c_str()
				);

				for (auto& elem : get_children()) {
					oss << elem.get().to_string(t_prepend + "  ");
				}

				return oss.str();
			}

			virtual ~AST_Node() noexcept = default;
			AST_Node(AST_Node&&) = default;
			AST_Node& operator=(AST_Node&&) = delete;
			AST_Node(const AST_Node&) = delete;
			AST_Node& operator=(const AST_Node&) = delete;
			virtual Units::second TimeSpent_Total() const {
				return 0;
			};
			virtual Units::second TimeSpent_Self() const {
				return 0;
			};
		protected:
			AST_Node(std::string t_ast_node_text, AST_Node_Type t_id, Parse_Location t_loc)
				: identifier(t_id)
				, text(std::move(t_ast_node_text))
				, location(std::move(t_loc))
				, return_type{ GoodLang::user_type_shared<void>() }
			{}
		};
		class exception {
		public:
			/// Errors generated during parsing or evaluation
			struct eval_error : std::runtime_error {
				std::string reason;
				File_Position start_position;
				std::string filename;
				std::string detail;
				std::vector<AST_Node::AST_Node_Trace> call_stack;

				eval_error(const std::string& t_why, const File_Position& t_where, const std::string& t_fname = "__EVAL__") noexcept
					: std::runtime_error(format(t_why, t_where, t_fname))
					, reason(t_why)
					, start_position(t_where)
					, filename(t_fname) {
				}

				explicit eval_error(const std::string& t_why) noexcept
					: std::runtime_error(t_why)
					, reason(t_why) {
				}

				eval_error(const eval_error&) = default;

				std::string pretty_print() const {
					std::ostringstream ss;

					/*ss << what();
					if (!call_stack.empty()) {
						ss << " during evaluation at (" << fname(call_stack[0]) << " " << startpos(call_stack[0]) << ").";
						if (detail.length() != 0) {
							ss << " " << detail << ".";
						}
						ss << " " << fname(call_stack[0]) << " (" << startpos(call_stack[0]) << ") '" << pretty(call_stack[0]) << "'";

						for (size_t j = 1; j < call_stack.size(); ++j) {
							if (id(call_stack[j]) != chaiscript::AST_Node_Type::Block && id(call_stack[j]) != chaiscript::AST_Node_Type::File) {
								ss << " from " << fname(call_stack[j]) << " (" << startpos(call_stack[j]) << ") '" << pretty(call_stack[j]) << "'";
							}
						}
					}*/

					//ss << '\n';
					return ss.str();
				}

				~eval_error() noexcept override = default;

			private:
				template<typename T>
				static AST_Node_Type id(const T& t) noexcept {
					return t.identifier;
				};

				template<typename T>
				static std::string pretty(const T& t) {
					return t.pretty_print();
				};

				template<typename T>
				static const std::string& fname(const T& t) noexcept {
					return t.filename();
				};

				template<typename T>
				static std::string startpos(const T& t) {
					std::ostringstream oss;
					oss << t.start().line << ", " << t.start().column;
					return oss.str();
				};

				static std::string format_why(const std::string& t_why) { return "Error: \"" + t_why + "\""; };

				template<typename T>
				static std::string format_location(const T& t) {
					std::ostringstream oss;
					oss << "(" << t.filename() << " " << t.start().line << ", " << t.start().column << ")";
					return oss.str();
				};

				static std::string format_filename(const std::string& t_fname) {
					std::stringstream ss;

					if (t_fname != "__EVAL__") {
						ss << "in '" << t_fname << "' ";
					}
					else {
						ss << "during evaluation ";
					}

					return ss.str();
				};

				static std::string format_location(const File_Position& t_where) {
					std::stringstream ss;
					ss << "at (" << t_where.line << ", " << t_where.column << ")";
					return ss.str();
				};

				static std::string format(const std::string& t_why, const File_Position& t_where, const std::string& t_fname) {
					std::stringstream ss;

					ss << format_why(t_why);
					ss << " ";

					ss << format_filename(t_fname);
					ss << " ";

					ss << format_location(t_where);

					return ss.str();
				};



			};
		};

		class Compiler{
		public:
			//class ScriptText {				
			//	std::string old_text; // before being split into chunks
			//	std::vector<std::string> original; // as provided by the user. May have been broken into chunks during the processing. 
			//	std::vector<std::string> extended; // as updated by the preprocessor. May have been broken into chunks during the processing. 
			//	std::string new_text; // after being split into chunks
			//public:
			//	ScriptText() = default;
			//	ScriptText(std::string Script) : old_text(std::move(Script)) {
			//		Preprocessor::PreprocessorState state;
			//		auto processed_result = Preprocessor().Parse(old_text);
			//		processed_result->GenerateExpandedCode(state);
			//		for (auto& script_chunk : state.Final_Script) {
			//			size_t startPos = new_text.size();
			//			new_text += script_chunk.first;
			//			size_t endPos = new_text.size();
			//			extended.push_back(new_text.substr(startPos, endPos - startPos));
			//			
			//		}
			//	};
			//	auto& newIndex_to_newText(size_t index) {
			//		for (auto& chunk : extended) {
			//			if (index >= chunk.size()) {
			//				index -= chunk.size();
			//			}
			//			else {
			//				return chunk[index];
			//			}
			//		}
			//		throw std::out_of_range("Out of range of extended script text");
			//	};
			//	auto& oldIndex_to_oldText(size_t index) {
			//		for (auto& chunk : original) {
			//			if (index >= chunk.size()) {
			//				index -= chunk.size();
			//			}
			//			else {
			//				return chunk[index];
			//			}
			//		}
			//		throw std::out_of_range("Out of range of original script text");
			//	};
			//};





		public:
			// The preprocessor should take in source code and perform substitutions based on preprocessor directives
			class Preprocessor {
			public:
				class PreprocessorState {
				public:
					struct word_t {
						std::string_view word;
						long pos_start;
					};
				public:
					std::vector<std::pair<std::string, Parse_Location>>
						Final_Script;
					std::map<
						std::string, // e.g. macro name (to be found)
						std::string> // content (to be replaced with)
						macro_definitions;
					std::map<std::string, // e.g. function name (e.g. add_together)
						std::pair<
						std::string, // function content (e.g. x + y)
						std::vector<std::string> // function variables to look for and replace (e.g. x, y)
						> > macro_functions;
					std::vector<std::pair<std::string, Parse_Location>>
						preprocessor_warnings;

				public:
					PreprocessorState() { // Build-In Preprocessor Macros
						Define("__VERSION__", 
							"1.0"
						);
						Define("__DATE__",
							GoodLang::ToString(DateTime::Now().tm_mon() + 1) + "/"
							+ GoodLang::ToString(DateTime::Now().tm_mday()) + "/"
							+ GoodLang::ToString(DateTime::Now().tm_year() + 1900)
						);
						Define("__TIME__",
							GoodLang::ToString(DateTime::Now().tm_hour()) + ":"
							+ GoodLang::ToString(DateTime::Now().tm_min()) + ":"
							+ GoodLang::ToString(DateTime::Now().tm_sec())
						);
						Define("__TIMESTAMP__", 
							DateTime::Now().c_str()
						);
					};

				public:
					std::string
						GetFinalScript() const {
						std::string y;
						for (auto& x : Final_Script) {
							y += x.first;
						}
						return y;
					};
					void
						PrintFinalScript() const {
						for (auto& x : Final_Script) {
							print(x.first + "\t\t\t" + GoodLang::printf("L%iC%i-L%iC%i", x.second.start.operator GoodLang::Engine::File_Position().line, x.second.start.operator GoodLang::Engine::File_Position().column, x.second.end.operator GoodLang::Engine::File_Position().line, x.second.end.operator GoodLang::Engine::File_Position().column));
						}
					};
					bool Define(std::string const& Name, std::string const& Content) {
						macro_definitions[std::string(RemoveLeadingAndTrailingWhiteSpace(Name))] = std::string(RemoveLeadingAndTrailingWhiteSpace(Content));
						return true;
					};
					bool Define(std::string const& Name, std::vector<std::string> variables, std::string const& Content) {
						macro_functions[std::string(RemoveLeadingAndTrailingWhiteSpace(Name))] = { std::string(RemoveLeadingAndTrailingWhiteSpace(Content)), variables };
						return true;
					};
					bool Undefine(std::string const& name) {
						auto Name = std::string(RemoveLeadingAndTrailingWhiteSpace(name));
						if (macro_definitions.find(Name) != macro_definitions.end()) {
							macro_definitions.erase(Name);
							return true;
						}
						if (macro_functions.find(Name) != macro_functions.end()) {
							macro_functions.erase(Name);
							return true;
						}
						return false;					
					};
					// To-Do
					bool Include(std::string const& IncludePath) {
						auto include_path = RemoveLeadingAndTrailingWhiteSpaceAndQuotes(IncludePath);
						auto key = std::string(include_path) + "_" + macro_definitions["__DATE__"] + "_" + macro_definitions["__VERSION__"];
						if (this->macro_definitions.find(key) != this->macro_definitions.end()) {
							return false;
						}
						else {
							Define(key, IncludePath);
							return true;
						}
					};

					static std::vector<word_t> GetAllWords(std::string const& text) {
						std::vector<word_t> out; {
							static std::regex pattern{ R"("[^"]*"|\/\/[^\n]*\n|\/\*[^\*\/]*\*\/|[A-z0-9_:#]+)" };
							int position, length;
							for (std::sregex_iterator i = std::sregex_iterator(text.begin(), text.end(), pattern); 
								i != std::sregex_iterator(); 
								++i)
							{
								const std::smatch& m = *i;

								position = ((text.length() - m.suffix().length()) - m.length());
								length = m.length();

								auto word{ std::string_view(text.c_str() + position, length) };
								// RemoveLeadingAndTrailingWhiteSpace(word);

								out.push_back(word_t{ word, position });
							}
						}
						return out;
					};
					static std::vector<std::string> split(std::string const& s, std::string const& delimiter) {
						size_t pos_start = 0, pos_end, delim_len = delimiter.length();
						std::string token;
						std::vector<std::string> res;
						while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
							res.push_back(std::string(std::string_view(s.c_str() + pos_start, pos_end - pos_start)));
							pos_start = pos_end + delim_len;
						}
						res.push_back(std::string(std::string_view(s.c_str() + pos_start)));
						return res;
					}
					static bool TryGetFunctionParams(std::string const& text, std::vector<std::string>& splits) {
						std::string to_split;
						bool retval = false;
						std::regex pattern(R"(\(\s*([^)]+?)\s*\))"); // \(\s*([^)]+?)\s*\)
						for (std::sregex_iterator i = std::sregex_iterator(text.begin(), text.end(), pattern);
							i != std::sregex_iterator();
							++i) {
							std::smatch m = *i;
							retval = true;
							to_split = m.str();
							if (m.prefix().str().find(" ") != std::string::npos) {
								return false;
							}
							else {
								break;
							}
						}
						if (retval && (to_split.size() >= 2)) {
							if (to_split[0] == '(') to_split = to_split.substr(1);
							if (to_split[to_split.size()-1] == ')') to_split = to_split.substr(0, to_split.size() - 1);
						
							splits = split(to_split, ",");
							for (auto& split : splits) {
								RemoveLeadingAndTrailingWhiteSpace(split);
							}
						}
						return retval;
					};
					bool TryReplace(std::string& Text, word_t const& Where, std::string const& Find, std::string const& ReplaceWith) {
						// we don't do replacements on strings -- their content should be left as-is
						if (Where.word.length() >= 2) {
							if ((Where.word[0] == '\"') && (Where.word[Where.word.size()-1] == '\"')) {
								return false; 
							}
						}

						// we don't do replacements on comments -- their content should be left as-is
						if (Where.word.length() >= 2) {
							if (Where.word.find("//") == 0) {
								return false;
							}
							if (Where.word.find("/*") == 0) {
								return false;
							}
						}

						// we can't find nothing	
						if (Find.empty()) return false;

						// exact word match
						if (Where.word == Find) {
							if (Where.pos_start == 0) {
								Text = ReplaceWith + Text.substr(Where.word.length());
							}
							else if ((Where.pos_start + Where.word.length()) == Text.length()) {
								Text = Text.substr(0, Where.pos_start) + ReplaceWith;
							}
							else {
								Text = Text.substr(0, Where.pos_start) + ReplaceWith + Text.substr(Where.pos_start + Where.word.length());
							}
							return true;
						}

						// in-word replacement when explicitely requested
						if (auto f_p = Where.word.find("##" + Find); f_p != std::string::npos) {
							auto NewReplaceWith = Replace(std::string(Where.word), "##" + Find, ReplaceWith);

							if (Where.pos_start == 0) {
								Text = NewReplaceWith + Text.substr(Where.word.length());
							}
							else if ((Where.pos_start + (Where.word.length())) == Text.length()) {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith;
							}
							else {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith + Text.substr(Where.pos_start + (Where.word.length()));
							}
							return true;
						}

						// in-word string replacement when explicitely requested
						if (auto f_p = Where.word.find("#" + Find); f_p != std::string::npos) {
							auto NewReplaceWith = Replace(std::string(Where.word), "#" + Find, "\"" + ExpandCode(ReplaceWith) + "\"");

							if (Where.pos_start == 0) {
								Text = NewReplaceWith + Text.substr(Where.word.length());
							}
							else if ((Where.pos_start + (Where.word.length())) == Text.length()) {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith;
							}
							else {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith + Text.substr(Where.pos_start + (Where.word.length()));
							}
							return true;
						}

						return false;
					};
					static std::string Replace(std::string const& text, std::string const& find, std::string const& replaceWith) {
						std::regex rule(find);
						return std::regex_replace(text, rule, replaceWith);
					};
					static void RemoveLeadingAndTrailingWhiteSpace(std::string_view& x) {
						while (
							(x.length() > 0)
							&& ((x[0] == ' ') || (x[0] == '\t') || (x[0] == '\n'))
							) {
							x.remove_prefix(1);
						}
						while (
							(x.length() > 0)
							&& ((x[x.length() - 1] == ' ') || (x[x.length() - 1] == '\t') || (x[x.length() - 1] == '\n'))
							) {
							x.remove_suffix(1);
						}
					};
					static std::string_view RemoveLeadingAndTrailingWhiteSpace(std::string const& text) {
						std::string_view x(text);
						RemoveLeadingAndTrailingWhiteSpace(x);
						return x;
					};

					static void RemoveLeadingAndTrailingWhiteSpaceAndQuotes(std::string_view& x) {
						while (
							(x.length() > 0)
							&& ((x[0] == ' ') || (x[0] == '\t') || (x[0] == '\n') || (x[0] == '\"'))
							) {
							x.remove_prefix(1);
						}
						while (
							(x.length() > 0)
							&& ((x[x.length() - 1] == ' ') || (x[x.length() - 1] == '\t') || (x[x.length() - 1] == '\n') || (x[x.length() - 1] == '\"'))
							) {
							x.remove_suffix(1);
						}
					};
					static std::string_view RemoveLeadingAndTrailingWhiteSpaceAndQuotes(std::string const& text) {
						std::string_view x(text);
						RemoveLeadingAndTrailingWhiteSpaceAndQuotes(x);
						return x;
					};

					std::string ExpandCode(std::string Code) {
						bool MadeAnyChanges = true;
						int maxIterations = 1000000;
						while (MadeAnyChanges && (--maxIterations >= 0)) {
							// note, do not perform this work if we do not have to.
							bool anyReason = false;
							for (auto& macro_definition : macro_definitions) {
								if (Code.find(macro_definition.first) != std::string::npos) {
									anyReason = true;
									break;
								}
							}
							for (auto& macro_definition : macro_functions) {
								if (Code.find(macro_definition.first) != std::string::npos) {
									anyReason = true;
									break;
								}
							}
							if (!anyReason) break;
						
							MadeAnyChanges = false;
							auto words = GetAllWords(Code);

							if (MadeAnyChanges) continue;
							for (auto& macro_definition : macro_definitions) {
								if (MadeAnyChanges) break;
								for (auto& word : words) {
									if (MadeAnyChanges) break;
									MadeAnyChanges = TryReplace(Code, word, macro_definition.first, macro_definition.second);
								}							
							}
						
							if (MadeAnyChanges) continue;
							for (auto& macro_function : macro_functions) {
								if (MadeAnyChanges) break;
								for (int word_index = 0; word_index < words.size(); word_index++) {
									if (MadeAnyChanges) break;

									if (words[word_index].word == macro_function.first) { // print
										// see if the next non-empty character is a '(' character
										int start_pos = words[word_index].pos_start + words[word_index].word.length();
										int end_pos;
										while (Code[start_pos] == ' ' || Code[start_pos] == '\t') { start_pos++; }
										if (Code[start_pos] == '(') {
											int commaCount = macro_function.second.second.size() - 1;
											std::vector<std::string> functionParams;

											// find the end to this, just in case we need it later
											end_pos = start_pos;
											int parenCount = 1;
											int currentVarStart = start_pos + 1;
											while ((parenCount > 0) && (++end_pos < Code.size())) {
												if (Code[end_pos] == '(') parenCount++;
												else if (Code[end_pos] == '[') parenCount++;
												else if (Code[end_pos] == '{') parenCount++;
												else if (Code[end_pos] == ')') parenCount--;
												else if (Code[end_pos] == ']') parenCount--;
												else if (Code[end_pos] == '}') parenCount--;

												if ((parenCount == 1) && (Code[end_pos] == ',')) {
													functionParams.push_back(Code.substr(currentVarStart, (end_pos) - currentVarStart));
													currentVarStart = end_pos + 1;
												}
											}
											if ((currentVarStart < end_pos) && (functionParams.size() < macro_function.second.second.size())) {
												functionParams.push_back(Code.substr(currentVarStart, (end_pos) - currentVarStart));
											}

											// we have a candidate -- ensure there are enough "vars" for the variables
											if (functionParams.size() >= macro_function.second.second.size()) {
												auto& function_name = macro_function.first;
												auto& function_content = macro_function.second.first;
												auto& function_var_names = macro_function.second.second;
												auto& function_vars = functionParams;

												PreprocessorState newState;
												for (int var_index = 0; var_index < function_var_names.size(); var_index++) {
													newState.Define(function_var_names[var_index], function_vars[var_index]);
												}
												auto implimented_function = newState.ExpandCode(function_content);

												auto Fm = Code.substr(words[word_index].pos_start, (end_pos - words[word_index].pos_start) + 1);
												MadeAnyChanges = TryReplace(Code, word_t{
													 std::string_view{ Fm },
													 words[word_index].pos_start
												}, Fm, implimented_function);
											}
										}
									}
								}
							}
						}
						return Code;
					};
				};
				class PreprocessorToken {
				private:
					std::string
						Text;
				public:
					const PreprocessorDirectives 
						identifier;
					const std::string_view
						text;
					Parse_Location 
						location;
					std::vector<std::shared_ptr<PreprocessorToken>>
						children;

					const File_Position& start() const noexcept { return location.start; }
					const File_Position& end() const noexcept { return location.end; }

					std::string pretty_print() const {
						std::ostringstream oss;

						oss << text;

						for (auto& elem : children) {
							oss << elem->pretty_print() << ' ';
						}

						return oss.str();
					}

					virtual void GenerateExpandedCode(PreprocessorState& state) const = 0;

					static bool replace(std::string& str, const std::string& from, const std::string& to) {
						size_t start_pos = str.find(from);
						if (start_pos == std::string::npos)
							return false;
						str.replace(start_pos, from.length(), to);
						return true;
					}
					static bool replaceAll(std::string& str, const std::string& from, const std::string& to) {
						if (from.empty())
							return false;
						size_t start_pos = 0;
						bool retval = false;
						while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
							str.replace(start_pos, from.length(), to);
							start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
							retval = true;
						}
						return retval;
					}

					/// Prints the contents of an AST node, including its children, recursively
					std::string to_string(const std::string& t_prepend = "") const {
						std::ostringstream oss;
						std::string str = GoodLang::ToString(std::string(this->identifier.ToString()));
						std::string data = std::string(this->text);
					
						replaceAll(data, "\n", "\\n");
						replaceAll(data, "\r", "\\r");
						replaceAll(data, "\t", "\\t");

						oss << GoodLang::printf("%s(%s) %s : L%iC%i - L%iC%i\n",
							t_prepend.c_str(), str.c_str(), data.c_str(),
							this->location.start.operator GoodLang::Engine::File_Position().line, this->location.start.operator GoodLang::Engine::File_Position().column, this->location.end.operator GoodLang::Engine::File_Position().line, this->location.end.operator GoodLang::Engine::File_Position().column
						);

						for (auto& elem : children) {
							oss << elem->to_string(t_prepend + "  ");
						}

						return oss.str();
					}

					virtual ~PreprocessorToken() noexcept = default;
					PreprocessorToken(PreprocessorToken&&) = default;
					PreprocessorToken& operator=(PreprocessorToken&&) = delete;
					PreprocessorToken(const PreprocessorToken&) = delete;
					PreprocessorToken& operator=(const PreprocessorToken&) = delete;
				
				protected:
					PreprocessorToken(std::string_view t_ast_node_text, PreprocessorDirectives t_id, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: identifier(t_id)
						, text()
						, location(std::move(t_loc))
						, children(std::move(t_children))
						, Text()
					{
						if ((this->identifier == PreprocessorDirectives::None) || (this->identifier == PreprocessorDirectives::Completed)) {
							// accept the script as-is, no change. 
							const_cast<std::string_view&>(text) = t_ast_node_text;
						}
						else {
							// remove the leading and tailing white space
							PreprocessorState::RemoveLeadingAndTrailingWhiteSpace(t_ast_node_text);
							Text = std::string(t_ast_node_text);
							while (replaceAll(Text, "\\\n\t", "\\\n")
								|| replaceAll(Text, "\\\n ", "\\\n")
								|| replaceAll(Text, "\\\r\t", "\\\r")
								|| replaceAll(Text, "\\\r ", "\\\r")							
							) {}
							replaceAll(Text, "\\\n", "\n");
							replaceAll(Text, "\\\r", "\r");
							const_cast<std::string_view&>(text) = Text;
						}
					}
				};
				using PreprocessorTokenPtr = typename std::shared_ptr<PreprocessorToken>;			
				class CompletedPreprocessor final : public PreprocessorToken {
				public:
					CompletedPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Completed, std::move(t_loc), std::move(t_children)) {}
					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class NonePreprocessor final : public PreprocessorToken {
				public:
					NonePreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::None, std::move(t_loc), std::move(t_children)) {}
					void GenerateExpandedCode(PreprocessorState& state) const override { 
						state.Final_Script.push_back({ state.ExpandCode(std::string(this->text)), this->location });
					
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class DefinePreprocessor final : public PreprocessorToken {
				public:
					DefinePreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Define, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						// this->text will include something like:
						// __VERSION__ 
						//       |--> this should be defined as a definition named "__VERSION__" with content ""
						// 
						// __VERSION__ 100
						//       |--> this should be defined as a definition named "__VERSION__" with content "100"
						// 
						// __VERSION__(x) x + 1
						//       |--> this should be defined as a function "__VERSION__" with param "x" and content "x + 1"

						auto thisText = std::string(this->text);
						auto words = state.GetAllWords(thisText);
						if (words.size() > 0) {
							auto defName = std::string(words[0].word);
							std::vector<std::string> var_names;
							if (state.TryGetFunctionParams(thisText, var_names)) {
								// is function
								state.Define(defName, var_names, thisText.substr(thisText.find(")")+1));
							}
							else {
								// is basic definition
								auto sub = thisText.substr(defName.length());
								auto potentialDef = state.RemoveLeadingAndTrailingWhiteSpace(sub);
								while ((potentialDef.size() > 0) && (potentialDef[0] == '=')) {
									potentialDef.remove_prefix(1);
								}
								state.RemoveLeadingAndTrailingWhiteSpace(potentialDef);
								if (potentialDef.size() > 0) {
									state.Define(defName, std::string(potentialDef));
								}
								else {
									state.Define(defName, "");
								}
							}
						}
					};
				};
				class IncludePreprocessor final : public PreprocessorToken {
				public:
					IncludePreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Include, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 		
						if (state.Include(std::string(this->text))) {
							std::string downloadedScript;
							downloadedScript = /*state.ExpandCode(*/R"(		
								#define __print_todays_date__ print(__DATE__)
								namespace string {
									string ReplaceOnce(string findWhat, string With){
										__print_todays_date__;
									};
									string ReplaceAll(string findWhat, string With){
										__print_todays_date__
									};
								};
							)"/*)*/;
							const_cast<IncludePreprocessor*>(this)->children.push_back(Preprocessor().Parse(downloadedScript));
						}
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class UndefinePreprocessor final : public PreprocessorToken {
				public:
					UndefinePreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Undefine, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						state.Undefine(std::string(this->text));				
					};
				};
				class IfChainPreprocessor final : public PreprocessorToken {
				public:
					IfChainPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::IfChain, std::move(t_loc), {}) {

						// the first child will either be an If or an IfDefined
						// the final child will always be an EndIf

						std::shared_ptr<PreprocessorToken> currentIfStatement = t_children[0];
						int currentIfStatementIndex = 0;

						for (int i = 1; i < (t_children.size() - 1); ++i) {
							if (t_children[i]->identifier == PreprocessorDirectives::ElseIf) {
								// end the current PreprocessorToken and add it to our children
								for (int j = currentIfStatementIndex + 1; j < i; ++j) {
									currentIfStatement->children.push_back(t_children[j]);
								}
								this->children.push_back(currentIfStatement);
								currentIfStatement = t_children[i];
								currentIfStatementIndex = i;
							}
							else if (t_children[i]->identifier == PreprocessorDirectives::Else) {
								// end the current PreprocessorToken and add it to our children
								for (int j = currentIfStatementIndex + 1; j < i; ++j) {
									currentIfStatement->children.push_back(t_children[j]);
								}
								this->children.push_back(currentIfStatement);
								currentIfStatement = t_children[i];
								currentIfStatementIndex = i;
							}
							else {
								continue;
							}
						}
						// end the current PreprocessorToken and add it to our children
						for (int j = currentIfStatementIndex + 1; j < (t_children.size() - 1); ++j) {
							currentIfStatement->children.push_back(t_children[j]);
						}
						this->children.push_back(currentIfStatement);
						this->children.push_back(t_children[t_children.size() - 1]);
					}

					static bool Evaluate(PreprocessorState& state, PreprocessorDirectives const& NodeType, std::string_view Text) {
						std::string ToEvaluate;
						switch (NodeType) {
						case PreprocessorDirectives::If:
						case PreprocessorDirectives::ElseIf: {
							ToEvaluate = state.ExpandCode(std::string(Text));
							
							auto temp_master_scope = StartScope();
							auto temp_scope{ std::make_shared<GoodLang::Scope>(temp_master_scope) };
							temp_scope->SetSelf(temp_scope);
							
							auto parsed_result = Compiler::Interpreter::Parser().Parse(ToEvaluate);
							return temp_scope->Cast<bool>(parsed_result.first->eval(temp_scope));
						}
						case PreprocessorDirectives::IfDefined:
							if (state.macro_definitions.find(std::string(Text)) != state.macro_definitions.end()) {
								return true;
							}
							if (state.macro_functions.find(std::string(Text)) != state.macro_functions.end()) {
								return true;
							}
							return false;
						case PreprocessorDirectives::IfNotDefined:
							return !Evaluate(state, PreprocessorDirectives::IfDefined, Text);
						default:
							return false;
						}
					}

					void GenerateExpandedCode(PreprocessorState& state) const override {					
						for (auto& child : this->children) {
							if (
								(child->identifier == PreprocessorDirectives::If)
								|| (child->identifier == PreprocessorDirectives::IfDefined)
								|| (child->identifier == PreprocessorDirectives::IfNotDefined)
								|| (child->identifier == PreprocessorDirectives::ElseIf)
								) {
								if (Evaluate(state, child->identifier, child->text)) {
									// end the search
									child->GenerateExpandedCode(state);
									return;
								}
							}
							else if (
								(child->identifier == PreprocessorDirectives::Else)
							) {
								if (true) {
									// end the search
									child->GenerateExpandedCode(state);
									return;
								}
							}
							else {
								// do nothing?
							}
						}
					};
				};
				class IfPreprocessor final : public PreprocessorToken {
				public:
					IfPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::If, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class ElseIfPreprocessor final : public PreprocessorToken {
				public:
					ElseIfPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::ElseIf, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class ElsePreprocessor final : public PreprocessorToken {
				public:
					ElsePreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Else, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class EndIfPreprocessor final : public PreprocessorToken {
				public:
					EndIfPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::EndIf, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class IfDefinedPreprocessor final : public PreprocessorToken {
				public:
					IfDefinedPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::IfDefined, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class IfNotDefinedPreprocessor final : public PreprocessorToken {
				public:
					IfNotDefinedPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::IfNotDefined, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class ErrorPreprocessor final : public PreprocessorToken {
				public:
					ErrorPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Error, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						auto orig_error = std::string(this->text);
						throw exception::eval_error("Compilation error was thrown: " + std::string(state.RemoveLeadingAndTrailingWhiteSpace(orig_error)), this->location.start);
					};
				};
				class WarningPreprocessor final : public PreprocessorToken {
				public:
					WarningPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Warning, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						state.preprocessor_warnings.push_back({ std::string(this->text), this->location });
					};
				};
				// To-Do: Add pragma support, to enable changes to the compiler.
				class PragmaPreprocessor final : public PreprocessorToken {
				public:
					PragmaPreprocessor(std::string_view t_ast_node_text, Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, PreprocessorDirectives::Pragma, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override { 
						// To-Do
					};
				};
			
			public:
				Preprocessor() = default;
				~Preprocessor() = default;

				PreprocessorTokenPtr Parse(SourceFile t_input) { return parse(t_input); };

			private:
				Position m_position{};
				std::vector<PreprocessorTokenPtr> m_match_stack;

			private:
				template<typename string_type>
				struct Char_Parser {
					string_type& match;
					using char_type = typename string_type::value_type;
					bool is_escaped = false;
					bool is_interpolated = false;
					bool saw_interpolation_marker = false;
					bool is_octal = false;
					bool is_hex = false;
					std::size_t unicode_size = 0;
					const bool interpolation_allowed;

					string_type octal_matches;
					string_type hex_matches;

					Char_Parser(string_type& t_match, const bool t_interpolation_allowed)
						: match(t_match)
						, interpolation_allowed(t_interpolation_allowed) {
					}

					Char_Parser& operator=(const Char_Parser&) = delete;

					~Char_Parser() {
						try {
							if (is_octal) {
								process_octal();
							}

							if (is_hex) {
								process_hex();
							}

							if (unicode_size > 0) {
								process_unicode();
							}
						}
						catch (const std::invalid_argument&) {
						}
						catch (const exception::eval_error&) {
							// Something happened with parsing, we'll catch it later?
						}
					}

					void process_hex() {
						if (!hex_matches.empty()) {
							auto val = stoll(hex_matches, nullptr, 16);
							match.push_back(char_type(val));
						}
						hex_matches.clear();
						is_escaped = false;
						is_hex = false;
					}

					void process_octal() {
						if (!octal_matches.empty()) {
							auto val = stoll(octal_matches, nullptr, 8);
							match.push_back(char_type(val));
						}
						octal_matches.clear();
						is_escaped = false;
						is_octal = false;
					}

					void process_unicode() {
						const auto ch = static_cast<uint32_t>(std::stoi(hex_matches, nullptr, 16));
						const auto match_size = hex_matches.size();
						hex_matches.clear();
						is_escaped = false;
						const auto u_size = unicode_size;
						unicode_size = 0;

						char buf[4];
						if (u_size != match_size) {
							throw exception::eval_error("Incomplete unicode escape sequence");
						}
						if (u_size == 4 && ch >= 0xD800 && ch <= 0xDFFF) {
							throw exception::eval_error("Invalid 16 bit universal character");
						}

						if (ch < 0x80) {
							match += static_cast<char>(ch);
						}
						else if (ch < 0x800) {
							buf[0] = static_cast<char>(0xC0 | (ch >> 6));
							buf[1] = static_cast<char>(0x80 | (ch & 0x3F));
							match.append(buf, 2);
						}
						else if (ch < 0x10000) {
							buf[0] = static_cast<char>(0xE0 | (ch >> 12));
							buf[1] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
							buf[2] = static_cast<char>(0x80 | (ch & 0x3F));
							match.append(buf, 3);
						}
						else if (ch < 0x200000) {
							buf[0] = static_cast<char>(0xF0 | (ch >> 18));
							buf[1] = static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
							buf[2] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
							buf[3] = static_cast<char>(0x80 | (ch & 0x3F));
							match.append(buf, 4);
						}
						else {
							// this must be an invalid escape sequence?
							throw exception::eval_error("Invalid 32 bit universal character");
						}
					}

					void parse(const char_type t_char, File_Position pos) {
						const bool is_octal_char = t_char >= '0' && t_char <= '7';

						const bool is_hex_char = (t_char >= '0' && t_char <= '9') || (t_char >= 'a' && t_char <= 'f') || (t_char >= 'A' && t_char <= 'F');

						if (is_octal) {
							if (is_octal_char) {
								octal_matches.push_back(t_char);

								if (octal_matches.size() == 3) {
									process_octal();
								}
								return;
							}
							else {
								process_octal();
							}
						}
						else if (is_hex) {
							if (is_hex_char) {
								hex_matches.push_back(t_char);

								if (hex_matches.size() == 2 * sizeof(char_type)) {
									// This rule differs from the C/C++ standard, but ChaiScript
									// does not offer the same workaround options, and having
									// hexadecimal sequences longer than can fit into the char
									// type is undefined behavior anyway.
									process_hex();
								}
								return;
							}
							else {
								process_hex();
							}
						}
						else if (unicode_size > 0) {
							if (is_hex_char) {
								hex_matches.push_back(t_char);

								if (hex_matches.size() == unicode_size) {
									// Format is specified to be 'slash'uABCD
									// on collecting from A to D do parsing
									process_unicode();
								}
								return;
							}
							else {
								// Not a unicode anymore, try parsing any way
								// May be someone used 'slash'uAA only
								process_unicode();
							}
						}

						if (t_char == '\\') {
							if (is_escaped) {
								match.push_back('\\');
								is_escaped = false;
							}
							else {
								is_escaped = true;
							}
						}
						else {
							if (is_escaped) {
								if (is_octal_char) {
									is_octal = true;
									octal_matches.push_back(t_char);
								}
								else if (t_char == 'x') {
									is_hex = true;
								}
								else if (t_char == 'u') {
									unicode_size = 4;
								}
								else if (t_char == 'U') {
									unicode_size = 8;
								}
								else {
									switch (t_char) {
									case ('\''):
										match.push_back('\'');
										break;
									case ('\"'):
										match.push_back('\"');
										break;
									case ('?'):
										match.push_back('?');
										break;
									case ('a'):
										match.push_back('\a');
										break;
									case ('b'):
										match.push_back('\b');
										break;
									case ('f'):
										match.push_back('\f');
										break;
									case ('n'):
										match.push_back('\n');
										break;
									case ('r'):
										match.push_back('\r');
										break;
									case ('t'):
										match.push_back('\t');
										break;
									case ('v'):
										match.push_back('\v');
										break;
									case ('$'):
										match.push_back('$');
										break;
									default:
										throw exception::eval_error("Unknown escaped sequence in string", pos);
									}
									is_escaped = false;
								}
							}
							else if (interpolation_allowed && t_char == '$') {
								saw_interpolation_marker = true;
							}
							else {
								match.push_back(t_char);
							}
						}
					}
				};
				template<typename Array2D, typename First, typename Second>
				constexpr static void set_alphabet(Array2D& array, const First first, const Second second) noexcept {
					auto* first_ptr = &std::get<0>(array) + static_cast<std::size_t>(first);
					auto* second_ptr = &std::get<0>(*first_ptr) + static_cast<std::size_t>(second);
					*second_ptr = true;
				};
				static constexpr std::array<std::array<bool, lengthof_alphabet>, max_alphabet> build_alphabet() noexcept {
					std::array<std::array<bool, lengthof_alphabet>, max_alphabet> alphabet{};

					set_alphabet(alphabet, symbol_alphabet, '?');

					set_alphabet(alphabet, symbol_alphabet, '?');
					set_alphabet(alphabet, symbol_alphabet, '+');
					set_alphabet(alphabet, symbol_alphabet, '-');
					set_alphabet(alphabet, symbol_alphabet, '*');
					set_alphabet(alphabet, symbol_alphabet, '/');
					set_alphabet(alphabet, symbol_alphabet, '|');
					set_alphabet(alphabet, symbol_alphabet, '&');
					set_alphabet(alphabet, symbol_alphabet, '^');
					set_alphabet(alphabet, symbol_alphabet, '=');
					set_alphabet(alphabet, symbol_alphabet, '.');
					set_alphabet(alphabet, symbol_alphabet, '<');
					set_alphabet(alphabet, symbol_alphabet, '>');

					for (size_t c = 'a'; c <= 'z'; ++c) {
						set_alphabet(alphabet, keyword_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'Z'; ++c) {
						set_alphabet(alphabet, keyword_alphabet, c);
					}
					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, keyword_alphabet, c);
					}
					set_alphabet(alphabet, keyword_alphabet, '_');
					// set_alphabet(alphabet, keyword_alphabet, ':');

					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, int_alphabet, c);
					}
					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, float_alphabet, c);
					}
					set_alphabet(alphabet, float_alphabet, '.');

					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, hex_alphabet, c);
					}
					for (size_t c = 'a'; c <= 'f'; ++c) {
						set_alphabet(alphabet, hex_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'F'; ++c) {
						set_alphabet(alphabet, hex_alphabet, c);
					}

					set_alphabet(alphabet, x_alphabet, 'x');
					set_alphabet(alphabet, x_alphabet, 'X');

					for (size_t c = '0'; c <= '1'; ++c) {
						set_alphabet(alphabet, bin_alphabet, c);
					}
					set_alphabet(alphabet, b_alphabet, 'b');
					set_alphabet(alphabet, b_alphabet, 'B');

					for (size_t c = 'a'; c <= 'z'; ++c) {
						set_alphabet(alphabet, id_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'Z'; ++c) {
						set_alphabet(alphabet, id_alphabet, c);
					}
					set_alphabet(alphabet, id_alphabet, '_');
					set_alphabet(alphabet, id_alphabet, ':'); // RG
					for (size_t c = '0'; c <= '9'; ++c) { set_alphabet(alphabet, id_alphabet, c); } // RG

					set_alphabet(alphabet, white_alphabet, ' ');
					set_alphabet(alphabet, white_alphabet, '\t');

					set_alphabet(alphabet, int_suffix_alphabet, 'l');
					set_alphabet(alphabet, int_suffix_alphabet, 'L');
					set_alphabet(alphabet, int_suffix_alphabet, 'u');
					set_alphabet(alphabet, int_suffix_alphabet, 'U');

					set_alphabet(alphabet, float_suffix_alphabet, 'l');
					set_alphabet(alphabet, float_suffix_alphabet, 'L');
					set_alphabet(alphabet, float_suffix_alphabet, 'f');
					set_alphabet(alphabet, float_suffix_alphabet, 'F');

					return alphabet;
				}
				bool char_in_alphabet(char c, Alphabet a) const noexcept { return m_alphabet[a][static_cast<uint8_t>(c)]; } // test a char in an m_alphabet

			private:
				std::array<std::array<bool, lengthof_alphabet>, max_alphabet> m_alphabet{ build_alphabet() };
				constexpr static utility::Static_String m_multiline_comment_end{ "*/" };
				constexpr static utility::Static_String m_multiline_comment_begin{ "/*" };
				constexpr static utility::Static_String m_singleline_comment{ "//" };
				constexpr static utility::Static_String m_annotation{ "#" };
				constexpr static utility::Static_String m_cr_lf{ "\r\n" };

			private:
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				bool Symbol_(const utility::Static_String& sym) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym.c_str()[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				bool Symbol_(const std::string_view& sym) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a char from input if it matches the parameter, without skipping initial whitespace
				bool Char_(const char c) {
					if (m_position.has_more() && (*m_position == c)) {
						++m_position;
						return true;
					}
					else {
						return false;
					}
				};
				/// Reads an end-of-line group from input, without skipping initial whitespace
				bool Eol_(const bool t_eos = false) {
					bool retval = false;

					if (m_position.has_more() && (Symbol_(m_cr_lf) || Char_('\n'))) {
						retval = true;
						//++m_position.line;
						m_position.col = 1;
					}
					//else if (m_position.has_more() && !t_eos && Char_(';')) {
					//	retval = true;
					//}

					return retval;
				};
				/// Reads a string from input if it matches the parameter, without skipping initial whitespace
				bool Keyword_(const utility::Static_String& t_s) {
					const auto len = t_s.size();
					if (m_position.remaining() >= len) {
						auto tmp = m_position;
						for (size_t i = 0; tmp.has_more() && i < len; ++i) {
							if (*tmp != t_s.c_str()[i]) {
								return false;
							}
							++tmp;
						}
						m_position = tmp;
						return true;
					}

					return false;
				};

				/// Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
				bool Id_() {
					if (m_position.has_more() && char_in_alphabet(*m_position, id_alphabet)) {
						while (m_position.has_more() && char_in_alphabet(*m_position, id_alphabet)) { //keyword_alphabet)) {
							++m_position;
						}

						return true;
					}
					else if (m_position.has_more() && (*m_position == '`')) {
						++m_position;
						const auto start = m_position;

						while (m_position.has_more() && (*m_position != '`')) {
							if (Eol()) {
								throw exception::eval_error("Carriage return in identifier literal", (File_Position)m_position);
							}
							else {
								++m_position;
							}
						}

						if (start == m_position) {
							throw exception::eval_error("Missing contents of identifier literal", (File_Position)m_position);
						} 
						else if (!m_position.has_more()) {
							throw exception::eval_error("Incomplete identifier literal", (File_Position)m_position);
						}

						++m_position;

						return true;
					}
					return false;
				};

			private: // TO-DO, reimpliment the optimization sequence inside of "build_match"
				/// Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
				template<typename NodeType>
				void build_match(size_t t_match_start, std::string_view t_text) {
					bool is_deep = false;

					Parse_Location filepos = [&]() -> Parse_Location {
						// so we want to take everything to the right of this and make them children
						if (t_match_start != m_match_stack.size()) {
							is_deep = true;
							return Parse_Location(
								m_match_stack[t_match_start]->location.start,
								m_position);
						}
						else {
							return Parse_Location(m_position, m_position);
						}
					}();

					std::vector<PreprocessorTokenPtr> new_children;
					if (is_deep) {
						new_children.assign(std::make_move_iterator(m_match_stack.begin() + static_cast<int>(t_match_start)),
							std::make_move_iterator(m_match_stack.end()));
						m_match_stack.erase(m_match_stack.begin() + static_cast<int>(t_match_start), m_match_stack.end());
					}

					m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NodeType>(t_text, std::move(filepos), std::move(new_children))));
				};

				/// create a node
				template<typename T, typename... Param>
				PreprocessorTokenPtr make_node(std::string_view t_match, Position t_prev, Param &&...param) {
					auto out = std::make_shared<T>(
						t_match,
						Parse_Location(t_prev, m_position),
						std::forward<Param>(param)...
					);
					return std::dynamic_pointer_cast<PreprocessorToken>(out);
				};

			private:
				/// Skips whitespace, which means space and tab, but not cr/lf
				/// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
				/// AlekMosingiewicz: Added exception when illegal character detected
				bool SkipWS(bool skip_cr = false) {
					bool retval = false;

					while (m_position.has_more()) {
						if (static_cast<unsigned char>(*m_position) > 0x7e) {
							throw exception::eval_error("Illegal character", (File_Position)m_position);
						}
						auto end_line = (*m_position != 0) && ((*m_position == '\n') || (*m_position == '\r' && *(m_position + 1) == '\n'));

						if (char_in_alphabet(*m_position, white_alphabet) || (skip_cr && end_line)) {
							if (end_line) {
								if (*m_position == '\r') {
									// discards lf
									++m_position;
								}
							}

							++m_position;

							retval = true;
						}
						else {
							break;
						}
					}
					return retval;
				};
				/// Reads until the end of the current statement
				bool Eos() {
					SkipWS();
					return Eol_(true);
				};
				/// Reads (and potentially captures) an end-of-line group from input
				bool Eol() {
					SkipWS();
					return Eol_();
				};
				/// Reads (and potentially captures) a char from input if it matches the parameter
				bool Char(const char t_c) {
					SkipWS();
					return Char_(t_c);
				};
				/// Reads (and potentially captures) a string from input if it matches the parameter
				bool Keyword(const utility::Static_String& t_s) {
					SkipWS();
					const auto start = m_position;
					bool retval = Keyword_(t_s);
					// ignore substring matches
					if (retval && m_position.has_more() && char_in_alphabet(*m_position, keyword_alphabet)) {
						m_position = start;
						retval = false;
					}
					return retval;
				};
				/// Reads (and potentially captures) a symbol group from input if it matches the parameter
				bool Symbol(std::string_view t_s, const bool t_disallow_prevention = false) {
					SkipWS();
					const auto start = m_position;
					bool retval = Symbol_(t_s);

					// ignore substring matches
					if (retval && m_position.has_more() && (t_disallow_prevention == false) && char_in_alphabet(*m_position, symbol_alphabet)) {						
						m_position = start;
						retval = false;						
					}
					return retval;
				}

				PreprocessorTokenPtr parse(SourceFile t_input) {
					const auto begin = t_input.empty() ? nullptr : &t_input.front();
					const auto end = begin == nullptr ? nullptr : begin + t_input.size();
					m_position = Position(begin, end);

					// top level stack        
					if (Statements()) {
						if (m_position.has_more()) {
							throw exception::eval_error("Unparsed input", (File_Position)m_position);
						}
						else {
							build_match<CompletedPreprocessor>(0, t_input);
						}
					}
					else {
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NonePreprocessor>("", Parse_Location(m_position, m_position), std::vector<PreprocessorTokenPtr>{})));
					}

					PreprocessorTokenPtr retval = m_match_stack.front();
					m_match_stack.clear();

					return retval;
				};

				bool SkipToEndOfLine(bool InPreprocessorMacro = true) {
					SkipWS();
					bool retval = false;
					while (m_position.has_more()){
						SkipWS();
						if (InPreprocessorMacro && (Keyword_("\\\n") || Keyword_("\\\r"))) {

						}
						else {
							if (Eol()) {
								retval = true;
								break;
							}
							else {
								++m_position;
							}
						}						
					}
					return retval;
				}
				bool SearchFor(std::vector<utility::Static_String> const& options, std::string_view& foundOption) {
					SkipWS();
					while (m_position.has_more()) {
						for (auto const& option : options) {
							if (Keyword(option)) {
								foundOption = option.c_str();
								return true;								
							}
						}
						++m_position;						
					}
					return false;
				}

				/// Top level parser, starts parsing of all known parses
				bool Statements() {
					bool retval = false;
					bool has_more = true;
					bool saw_eol = true;

					while (has_more) {
						SkipWS(true);
						if (Warning() || Error() || Pragma() || Include() || If() || Define() || Undefine() || None()) {
							has_more = true;
							retval = true;
						}
						else {
							has_more = false;
						}
					}
					return retval;
				};

				bool Define() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;
					SkipWS(true);
					if (Keyword("#define")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<DefinePreprocessor>(
							this->m_position.str(prev_position, m_position),
							Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Undefine() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;
					SkipWS(true);
					if (Keyword("#undef")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<UndefinePreprocessor>(
							this->m_position.str(prev_position, m_position),
							Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool If() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;

					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						m_position = prev_position;
						return false;
					};

					bool foundIfOrElseIf = Keyword("#if");
					if (foundIfOrElseIf) {
						auto this_prev_position = this->m_position;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IfPreprocessor>(
							this->m_position.str(this_prev_position, m_position),
							Parse_Location{ this_prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					if (!foundIfOrElseIf) {
						foundIfOrElseIf = Keyword("#ifdef");
						if (foundIfOrElseIf) {
							auto this_prev_position = this->m_position;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IfDefinedPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
						}
					}
					if (!foundIfOrElseIf) {
						foundIfOrElseIf = Keyword("#ifndef");
						if (foundIfOrElseIf) {
							auto this_prev_position = this->m_position;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IfNotDefinedPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
						}
					}

					while (foundIfOrElseIf) {
						foundIfOrElseIf = false;
						retval = true;

						Statements();

						SkipWS(true);
						if (Keyword("#elif")) {
							Position this_prev_position = this->m_position;
							foundIfOrElseIf = true;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<ElseIfPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
							continue;
						}
						else if (Keyword("#else")) {
							Position this_prev_position = this->m_position;
							foundIfOrElseIf = true;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<ElsePreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
							continue;
						}
						else if (Keyword("#endif")) {
							Position this_prev_position = this->m_position;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<EndIfPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
						}
						else {
							return failure();
						}
						build_match<IfChainPreprocessor>(prev_stack_top, "");
					}
					
					return retval;
				};
				bool Error() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;
					SkipWS(true);
					if (Keyword("#error")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<ErrorPreprocessor>(
							this->m_position.str(prev_position, m_position),
							Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Warning() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;

					SkipWS(true);
					if (Keyword("#warning")) {
						prev_position = this->m_position;
						retval = true;		
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<WarningPreprocessor>(
							this->m_position.str(prev_position, m_position),
							Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Include() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;

					SkipWS(true);
					if (Keyword("#include")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IncludePreprocessor>(
							this->m_position.str(prev_position, m_position),
							Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Pragma() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;

					SkipWS(true);
					if (Keyword("#pragma")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<PragmaPreprocessor>(
							this->m_position.str(prev_position, m_position),
							Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool None() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Position prev_position = this->m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						m_position = prev_position;
						return false;
					};

					SkipWS(true);
					if (
						Keyword("#define") || 
						Keyword("#undef") ||
						Keyword("#if") ||
						Keyword("#elif") ||
						Keyword("#else") ||
						Keyword("#endif") ||
						Keyword("#ifdef") ||
						Keyword("#ifndef") ||
						Keyword("#error") ||
						Keyword("#warning") ||
						Keyword("#pragma") || 
						Keyword("#include")
					) {
						return failure();
					}

					while (SkipToEndOfLine(false)) {
						retval = true;
						SkipWS(true);

						const auto this_prev_stack_top = m_match_stack.size();
						Position this_prev_position = this->m_position;
						if (!m_position.has_more() ||
							Keyword("#define") ||
							Keyword("#undef") ||
							Keyword("#if") ||
							Keyword("#elif") ||
							Keyword("#else") ||
							Keyword("#endif") ||
							Keyword("#ifdef") ||
							Keyword("#ifndef") ||
							Keyword("#error") ||
							Keyword("#warning") ||
							Keyword("#pragma") ||
							Keyword("#include")
						) {
							while (m_match_stack.size() != this_prev_stack_top) m_match_stack.pop_back();
							m_position = this_prev_position;

							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NonePreprocessor>(
								this->m_position.str(prev_position, m_position),
								Parse_Location{ prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));

							return retval;
						}






						//m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NonePreprocessor>(
						//	this->m_position.str(prev_position, m_position), 
						//	Parse_Location{ prev_position.line, prev_position.col, m_position.line, m_position.col },
						//	std::vector<PreprocessorTokenPtr>{}
						//)));
					}

					return retval;					
				};
			};
		
			class Interpreter {
			public:
				struct AST_Node_Impl;
				using AST_Node_Impl_Ptr = typename std::shared_ptr<AST_Node_Impl>;
				using AST_NodePtr = typename std::shared_ptr<AST_Node>;

				class detail {
				public:
					/// Special type for returned values
					struct Return_Value {
						Any retval;
					};

					/// Special type indicating a call to 'break'
					struct Break_Loop {};

					/// Special type indicating a call to 'continue'
					struct Continue_Loop {};

					template<typename T>
					static bool GetTextImpl(T const& r, std::string_view& out) {
						if (!r->text.empty()) {
							out = r->text;
							return true;
						}
						else {
							for (auto& child : r->children) {
								if (GetTextImpl(child, out)) {
									return true;
								}
							}
						}
						return false;
					};
					template<typename T>
					static bool GetClassTypeImpl(T const& r, std::weak_ptr<Type_Info>& out) {
						if (r->identifier == AST_Node_Type::Id) {
							if (auto ptr = std::dynamic_pointer_cast<AST_Nodes::Id_AST_Node>(r)) {
								if (ptr->type == AST_Nodes::IdType::Class) {
									if (auto ptr2 = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(ptr)) {
										out = ptr2->TypeInfo;
										return true;
									}
								}
							}
						}

						for (auto& child : r->children) {
							if (GetClassTypeImpl(child, out)) {
								return true;
							}
						}

						return false;
					};

				};

				static std::weak_ptr<Type_Info> GetClassType(AST_Node_Impl_Ptr const& r, std::shared_ptr<Scope> const& currentScope) {
					std::weak_ptr<Type_Info> out;
					if (r) {
						if (!detail::GetClassTypeImpl(r, out)) {
							auto sv = GetText(r);
							if (auto Class = currentScope->FindClass(std::string(sv))) {
								out = Class->GetClassType();
							}
							else if (sv == "void") {
								out = user_type_shared<void>();
							}
						}
					}
					return out;
				}

				template<typename T> static std::string_view GetText(T const& r) {
					std::string_view out;
					(void)detail::GetTextImpl(r, out);
					return out;
				};
				static Any const_var(Any const& rhs) {
					Any out = rhs;
					out.SetFlag(AnyData::Flag::constant, true);
					return out;
				};

				struct AST_Node_Impl : public AST_Node {
					AST_Node_Impl(std::string t_ast_node_text,
						AST_Node_Type t_id,
						Parse_Location t_loc,
						std::vector<AST_Node_Impl_Ptr> t_children = std::vector<AST_Node_Impl_Ptr>())
						: AST_Node(std::move(t_ast_node_text), t_id, std::move(t_loc))
						, children(std::move(t_children))
						, time_spent_during_eval{ 0 }
						, num_evals{ 0 }
					{};

					std::vector<std::reference_wrapper<AST_Node>> get_children() const override final {
						std::vector<std::reference_wrapper<AST_Node>> retval;
						retval.reserve(children.size());
						for (const AST_Node_Impl_Ptr& child : children) {
							retval.emplace_back(*child);
						}
						return retval;
					};

					Any eval(std::shared_ptr<Scope> const& currentScope) const override final {
						Stopwatch sw;
						sw.Start();

						InterlockedIncrementAcquire64(&num_evals);
						defer(
							InterlockedAddAcquire64(&time_spent_during_eval, sw.Stop());
						);

						try {
							// T::trace(currentScope, this);
							return eval_internal(currentScope);
						}
						catch (exception::eval_error& ee) {
							ee.call_stack.push_back(*this);
							throw ee;
						}
						catch (std::runtime_error& ee) {
							auto e = exception::eval_error(ee.what(), this->location.start, "Compiled C++ Function");
							e.call_stack.push_back(*this);
							throw e;
						}
						catch (std::exception& ee) {
							auto e = exception::eval_error(ee.what(), this->location.start, "Compiled C++ Function");
							e.call_stack.push_back(*this);
							throw e;
						}
					}

					Units::second TimeSpent_Total() const override {
						Units::second out = Units::nanosecond(time_spent_during_eval);
						for (auto& child : children) {
							out += child->TimeSpent_Total();
						}
						return out;
					};
					Units::second TimeSpent_Self() const override {
						return Units::nanosecond(time_spent_during_eval) / std::max<long long>(1, num_evals);
					};

					mutable __int64 time_spent_during_eval;
					mutable __int64 num_evals;
					std::vector<AST_Node_Impl_Ptr> children;

				protected:
					virtual Any eval_internal(std::shared_ptr<Scope> const&) const {
						return Any();
						// throw std::runtime_error("Undispatched ast_node (internal error)");
					};
				};
				class AST_Nodes {
				public:
					// wrapper for an entire script
					struct File_AST_Node final : AST_Node_Impl {
						File_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::File, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() > 0) this->return_type = this->children.back()->return_type;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							try {
								const auto num_children = this->children.size();
								try {
									if (num_children > 0) {
										for (size_t i = 0; i < num_children - 1; ++i) {
											this->children[i]->eval(currentScope);
										}
										return this->children.back()->eval(currentScope);
									}
									else {
										return {};
									}
								}
								catch (detail::Return_Value& rv) {
									return rv.retval;
								}
							}
							catch (const detail::Continue_Loop&) {
								throw exception::eval_error("Unexpected `continue` statement outside of a loop", (File_Position)this->location.start);
							}
							catch (const detail::Break_Loop&) {
								throw exception::eval_error("Unexpected `break` statement outside of a loop", (File_Position)this->location.start);
							}
						}
					};
					// empty lines, comments, etc.
					struct Noop_AST_Node final : AST_Node_Impl {
						Noop_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc)
							: AST_Node_Impl(t_ast_node_text, AST_Node_Type::Noop, t_loc)
						{};

						Noop_AST_Node()
							: AST_Node_Impl("", AST_Node_Type::Noop, Parse_Location{ Position{}, Position{} })
						{};
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// It's a no-op, that evaluates to "void"
							return {};
						}
					};
					// return ARG
					struct Return_AST_Node final : AST_Node_Impl {
						Return_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Return, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() == 0) {

							}
							else if (this->children.size() == 1) {
								this->return_type = this->children[0]->return_type;
							}
							else {
								this->return_type = user_type_shared<Vector<Var>>();
							}
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							if (this->children.size() == 0) {
								throw detail::Return_Value{ Any() };
							}
							else if (this->children.size() == 1) {
								Any out{ this->children[0]->eval(currentScope) };
								if (out.IsEmpty()) // cannot return void						
									throw exception::eval_error("Cannot return void from a return statement.", (File_Position)this->location.start);
								else
									throw detail::Return_Value{ out };
							}
							else {
								Vector<Var> vec;
								for (const auto& child : this->children) {
									vec.push_back(Var(child->eval(currentScope)));
								}
								throw detail::Return_Value{ vec };
							}
						}
					};
					// built-in constants that could be understood by the compiler, such as integers, floating-point values, strings, vectors, etc.
					struct Constant_AST_Node final : public AST_Node_Impl {
						Constant_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, Any t_value)
							: AST_Node_Impl(t_ast_node_text, AST_Node_Type::Constant, std::move(t_loc))
							, m_value(std::move(t_value))
						{
							m_value.SetFlag(AnyData::Flag::constant, true);
							this->return_type = m_value.Type().lock()->MakeConstRef();
						}

						explicit Constant_AST_Node(Any t_value)
							: AST_Node_Impl("", AST_Node_Type::Constant, Parse_Location{ Position{}, Position{} })
							, m_value(std::move(t_value))
						{
							m_value.SetFlag(AnyData::Flag::constant, true);
							this->return_type = m_value.Type().lock()->MakeConstRef();
						}

						Any eval_internal(const std::shared_ptr<Scope>&) const override {
							return m_value;
						}

						Any m_value;
					};

					// intended for basic operations like "+"
					struct Binary_Operator_AST_Node : AST_Node_Impl {
						Binary_Operator_AST_Node(const std::shared_ptr<Scope>& currentScope, const std::string& t_oper, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(t_oper, AST_Node_Type::Binary, std::move(t_loc), std::move(t_children))
							, m_oper(Operators::to_operator(t_oper))
						{
							GoodLang::ParamTypes params({ this->children[0]->return_type, this->children[1]->return_type });
							if (Proxy_Function func = currentScope->FindFunction(this->text, params, *currentScope->GetTypeConverterTree())) {
								this->return_type = func->Returns();
							}
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return currentScope->CallFunction(this->text, { this->children[0]->eval(currentScope), this->children[1]->eval(currentScope) });
						};

					private:
						Operators::Opers m_oper;

					};

					enum class IdType { 
						Id, 
						Function, 
						Variable, 
						Class 
					};
					// keyname node, could be a function name, could be a variable name, etc.
					struct Id_AST_Node : AST_Node_Impl {
						Id_AST_Node(const std::shared_ptr<Scope>& currentScope, const std::string& t_ast_node_text, Parse_Location t_loc)
							: AST_Node_Impl(t_ast_node_text, AST_Node_Type::Id, std::move(t_loc))
						{}

						//Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
						//	if (auto obj = currentScope->FindObj(this->text)) {
						//		// const_cast<Id_AST_Node*>(this)->return_type = obj->Type();
						//		return obj;
						//	}
						//	throw exception::eval_error("Can not find object: " + this->text);
						//}
					public: 
						IdType type = IdType::Id;
					};
					// 
					struct FunctionName_AST_Node final : Id_AST_Node {
						FunctionName_AST_Node(const std::shared_ptr<Scope>& currentScope, const std::string& t_ast_node_text, Parse_Location t_loc)
							: Id_AST_Node(currentScope, t_ast_node_text, std::move(t_loc))
						{
							type = IdType::Function;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							if (auto obj = currentScope->FindObj(this->text)) {
								// const_cast<Id_AST_Node*>(this)->return_type = obj->Type();
								return obj;
							}
							throw exception::eval_error("Can not find object: " + this->text, (File_Position)this->location.start);
						}
					};
					// 
					struct VariableName_AST_Node final : Id_AST_Node {
						VariableName_AST_Node(const std::shared_ptr<Scope>& currentScope, const std::string& t_ast_node_text, Parse_Location t_loc)
							: Id_AST_Node(currentScope, t_ast_node_text, std::move(t_loc))
						{
							type = IdType::Variable;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							if (auto obj = currentScope->FindObj(this->text)) {
								// const_cast<Id_AST_Node*>(this)->return_type = obj->Type();
								return obj;
							}
							throw exception::eval_error("Can not find object: " + this->text, (File_Position)this->location.start);
						}
					};
					// 
					struct ClassName_AST_Node final : Id_AST_Node {
						ClassName_AST_Node(const std::shared_ptr<Scope>& currentScope, const std::string& t_ast_node_text, Parse_Location t_loc)
							: Id_AST_Node(currentScope, t_ast_node_text, std::move(t_loc))
						{
							type = IdType::Class;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							if (auto obj = currentScope->FindObj(this->text)) {
								// const_cast<Id_AST_Node*>(this)->return_type = obj->Type();
								return obj;
							}
							throw exception::eval_error("Can not find object: " + this->text, (File_Position)this->location.start);
						}

					public:
						std::weak_ptr<GoodLang::Type_Info> TypeInfo;
					};					
					//
					struct Arg_AST_Node final : AST_Node_Impl {
						Arg_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Arg, std::move(t_loc), std::move(t_children))
						{
							this->return_type = this->children.back()->return_type;
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return this->children.back()->eval(currentScope);
						}


					};
					//
					struct Arg_List_AST_Node final : AST_Node_Impl {
						Arg_List_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Arg_List, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() > 0) this->return_type = this->children.back()->return_type;
						};

						static std::string get_arg_name(const AST_Node_Impl& t_node) {
							if (t_node.children.empty()) {
								return t_node.text;
							}
							else if (t_node.children.size() == 1) {
								return t_node.children[0]->text;
							}
							else {
								return t_node.children[1]->text;
							}
						}
						static std::vector<std::string> get_arg_names(const AST_Node_Impl& t_node) {
							std::vector<std::string> retval;

							for (const auto& node : t_node.children) {
								retval.push_back(get_arg_name(*node));
							}

							return retval;
						}
						static std::weak_ptr<Type_Info> get_arg_type(const AST_Node_Impl& t_node) {
							if (t_node.children.empty()) {
								return user_type_shared<Any>();
							}
							else if (t_node.children.size() == 1) {
								return user_type_shared<Any>();
							}
							else if (t_node.children[0]->identifier == AST_Node_Type::Id) {
								if (auto ptr = std::dynamic_pointer_cast<AST_Nodes::Id_AST_Node>(t_node.children[0])) {
									if (ptr->type == AST_Nodes::IdType::Class) {
										if (auto ptr2 = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(ptr)) {
											return ptr2->TypeInfo;
										}
									}
								}
								if (GetText(t_node.children[0]) == "void") {
									return user_type_shared<void>();
								}
								return user_type_shared<Any>();
							}
							else {
								return user_type_shared<Any>();
							}							
						}
						static std::vector<std::weak_ptr<Type_Info>> get_arg_types(const AST_Node_Impl& t_node) {
							std::vector<std::weak_ptr<Type_Info>> retval;
							for (const auto& node : t_node.children) {
								retval.push_back(get_arg_type(*node));
							}
							return retval;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// evaluate all of the children, return the result of the last child
							int numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(currentScope);
								}
								return this->children.back()->eval(currentScope);
							}
							else {
								return Any();
							}
						}
					};
					// ID(ARG_LIST)
					struct Fun_Call_AST_Node : AST_Node_Impl {
						Fun_Call_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Fun_Call, std::move(t_loc), std::move(t_children))
						{
							assert(!this->children.empty());

							function_name = GetText(this->children[0]);

							std::vector<std::weak_ptr<Type_Info>> params;
							for (const AST_Node_Impl_Ptr& child : this->children[1]->children) {
								if (std::shared_ptr<Type_Info> Type = child->return_type.lock()) {
									if (Type->is_any() || Type->is_void()) { // return_types should be known at compile time to support caching functions during compilation.
										return;
									}
									params.push_back(Type);
								}
								else {
									return;
								}
							}
							ParamTypes Params{ params };
							std::shared_ptr<Any> obj;
							Proxy_Function found_function;

							if (currentScope->FindObjOrFunction(function_name, Params, *currentScope->GetTypeConverterTree(), &obj, &found_function)) {
								if (obj) {
									if (Proxy_Function func = obj->cast<Proxy_Function>()) {
										Function = func;
									}
								}
								else if (found_function) {
									Function = found_function;
								}
							}

							if (auto func = Function.lock()) this->return_type = func->Returns();
						};

						template <bool returnsValue>
						Any do_eval_internal(const std::shared_ptr<Scope>& currentScope) const {
							std::vector<Any> params{};

							params.reserve(this->children[1]->children.size());
							for (const AST_Node_Impl_Ptr& child : this->children[1]->children) {
								params.push_back(child->eval(currentScope));
							}
							ParamTypes Params{ params };

							if (auto func = Function.lock()) {
								if (
									func->NumArguments() == 2
									&& GoodLang::GetHash(func->Argument(0)) == GoodLang::GetHash(user_type_shared<Scope>())
									&& GoodLang::GetHash(func->Argument(1)) == GoodLang::GetHash(user_type_shared<std::vector<Any>>())
									) {
									if constexpr (returnsValue) {
										return func->operator()({ currentScope, params });
									}
									else {
										(void)func->operator()({ currentScope, params });
										return {};
									}
								}
								else {
									if constexpr (returnsValue)
										return currentScope->CallFunction(func, params);
									else {
										(void)currentScope->CallFunction(func, params);
										return {};
									}
								}
							}

							if (1) {
								auto tree = currentScope->GetTypeConverterTree();
								std::shared_ptr<Any> obj;
								Proxy_Function found_function;
								if (currentScope->FindObjOrFunction(function_name, params, Params, *tree, &obj, &found_function)) {
									if (obj) {
										if (Proxy_Function func = obj->cast<Proxy_Function>()) {
											if (
												func->NumArguments() == 2
												&& GoodLang::GetHash(func->Argument(0)) == GoodLang::GetHash(user_type_shared<Scope>())
												&& GoodLang::GetHash(func->Argument(1)) == GoodLang::GetHash(user_type_shared<std::vector<Any>>())
												) {
												if constexpr (returnsValue) {
													return func->operator()({ currentScope, params });
												}
												else {
													(void)func->operator()({ currentScope, params });
													return {};
												}
											}
											else {
												if constexpr (returnsValue)
													return currentScope->CallFunction(func, params);
												else {
													(void)currentScope->CallFunction(func, params);
													return {};
												}
											}
										}
										if (1) {
											std::vector<Any> params2{ obj };
											for (auto& x : params) params2.push_back(x);

											auto [func, tree] = currentScope->BuildFunction("()", params2, ParamTypes(params2));
											if (func) {
												if constexpr (returnsValue)
													return currentScope->CallFunction(func, params2);
												else {
													(void)currentScope->CallFunction(func, params2);
													return {};
												}
											}
										}
									}
									else if (found_function) {
										if constexpr (returnsValue)
											return currentScope->CallFunction(function_name, params);
										else {
											(void)currentScope->CallFunction(function_name, params);
											return {};
										}
									}
								}
								throw exception::eval_error("Can not find requested object or function: " + function_name, (File_Position)this->location.start);
							}
						};
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return do_eval_internal<true>(currentScope);
						};
						std::string function_name;
						std::weak_ptr<details::Proxy_Function_Base> Function;
					};
					struct Unused_Return_Fun_Call_AST_Node final : Fun_Call_AST_Node {
						Unused_Return_Fun_Call_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: Fun_Call_AST_Node(currentScope, std::move(t_ast_node_text), std::move(t_loc), std::move(t_children))
						{}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return this->template do_eval_internal<false>(currentScope);
						};
					};
					// 
					struct Equation_AST_Node final : AST_Node_Impl {
						Equation_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Equation, std::move(t_loc), std::move(t_children))
							, m_oper(Operators::to_operator(this->text))
						{
							assert(this->children.size() == 2);

							if ((m_oper == Operators::Opers::assign_if_null) || (this->text == "?=")) {
								// should actually test the return type of the first param first...
								this->return_type = this->children[1]->return_type;
							}
							else {
								auto lhs = this->children[0]->return_type;
								auto rhs = this->children[1]->return_type;
								ParamTypes params({ lhs, rhs });
								if (m_oper == Operators::Opers::assign) {
									this->return_type = this->children[0]->return_type;
								}
								else if (this->text == ":=") {
									this->return_type = this->children[1]->return_type;
								}
								else {
									if (auto func = currentScope->FindFunction(this->text, params, *currentScope->GetTypeConverterTree())) {
										this->return_type = func->Returns();
									}
									else {
										this->return_type = this->children[0]->return_type;
									}
								}
							}
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							if (m_oper == Operators::Opers::assign_if_null || this->text == "?=") {
								Any lhs = this->children[0]->eval(currentScope);
								if (lhs.IsTypeOf<void>()) {
									return currentScope->CallFunction(":=", { lhs, this->children[1]->eval(currentScope) });
								}
								else {
									return lhs;
								}
							}
							else {
								Any lhs = this->children[0]->eval(currentScope);
								Any rhs = this->children[1]->eval(currentScope);

								if (m_oper == Operators::Opers::assign) {
									return currentScope->CallFunction("=", { lhs, rhs });
								}
								else if (this->text == ":=") {
									return currentScope->CallFunction(":=", { lhs, rhs });
								}
								else {
									try {
										return currentScope->CallFunction(this->text, { lhs, rhs });
									}
									catch (GoodLang::exception::not_found_error const& e) {
										throw exception::eval_error("Unable to find appropriate'" + this->text + "' operator.", (File_Position)this->location.start);
									}
								}
							}
						}

					private:
						Operators::Opers m_oper;

					};
					// &&
					struct Logical_And_AST_Node final : AST_Node_Impl {
						Logical_And_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Logical_And, std::move(t_loc), std::move(t_children)) {
							assert(this->children.size() == 2);
							this->return_type = user_type_shared<bool>();
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return currentScope->Cast<bool>(this->children[0]->eval(currentScope)) && currentScope->Cast<bool>(this->children[1]->eval(currentScope));
						}
					};
					// ||
					struct Logical_Or_AST_Node final : AST_Node_Impl {
						Logical_Or_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Logical_Or, std::move(t_loc), std::move(t_children)) {
							assert(this->children.size() == 2);
							this->return_type = user_type_shared<bool>();
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return currentScope->Cast<bool>(this->children[0]->eval(currentScope)) || currentScope->Cast<bool>(this->children[1]->eval(currentScope));
						}
					};

					/*
					var x;
					*/
					struct Var_Decl_AST_Node final : AST_Node_Impl {
						Var_Decl_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Var_Decl, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() >= 1);
							this->return_type = user_type_shared<Var>();
						}

						/*! Empty variable assignment:
						  var j;
						*/
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							const std::string& idname = this->children[0]->text;
							currentScope->AddObj(idname, std::make_shared<Any>(Var()));
							return currentScope->FindObj(idname);							
						}
					};
					/*
					double x;
					*/
					struct Assign_Retroactively_AST_Node final : AST_Node_Impl {
						Assign_Retroactively_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Assign_Retroactively, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() >= 1);

							// type_name = std::string(GetText(this->children[0]->children[0]));//  ->text; // e.g. double, int, std::string
							idname = std::string(GetText(this->children[1]));// ->text; // e.g. x, y, z

							//if (auto Class = currentScope->FindClass(type_name)) this->return_type = Class->GetClassType();
							this->return_type = this->children[0]->return_type;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							Any defaultVal;
							if (this->children.size() == 3) {
								defaultVal = this->children[2]->eval(currentScope);
							}

							if (currentScope->IsClass()) {
								if (auto ClassPtr = std::dynamic_pointer_cast<Class>(currentScope)) {
									std::weak_ptr<Type_Info> out;
									if (detail::GetClassTypeImpl(this->children[0], out)) {
										if (out.lock()->is_ref()) {
											if (defaultVal.IsEmpty()) {
												ClassPtr->DeclareMemberObject(idname, user_type_shared<Var>(), std::make_shared<Any>(Var(this->children[0]->eval(currentScope))));
												return {};
											}
											else {
												auto newVal = std::make_shared<Any>(Var(this->children[0]->eval(currentScope)));
												currentScope->CallFunction(":=", { newVal, defaultVal });
												ClassPtr->DeclareMemberObject(idname, user_type_shared<Var>(), newVal);
												return {};
											}
										}
										else {
											if (defaultVal.IsEmpty()) {
												ClassPtr->DeclareMemberObject(idname, out, std::make_shared<Any>(this->children[0]->eval(currentScope)));
												return {};
											}
											else {
												auto newVal = std::make_shared<Any>(this->children[0]->eval(currentScope));
												currentScope->CallFunction(":=", { newVal, defaultVal });
												ClassPtr->DeclareMemberObject(idname, user_type_shared<Var>(), newVal);
												return {};
											}
										}
									}
								}
							}

							if (1) {
								bool refObj = false;
								std::weak_ptr<Type_Info> out;
								if (detail::GetClassTypeImpl(this->children[0], out)) {
									if (out.lock()->is_ref()) {
										// user wanted a reference object.
										currentScope->AddObj(idname, std::make_shared<Any>(Var(this->children[0]->eval(currentScope))));
										refObj = true;
									}
								}

								// child 0 = TypeID()
								// child 1 = Id()
								if (!refObj) currentScope->AddObj(idname, std::make_shared<Any>(this->children[0]->eval(currentScope)));
								if (!defaultVal.IsEmpty()) {
									std::shared_ptr<Any> thisObj = currentScope->FindObj(idname);
									(void)currentScope->CallFunction(":=", { thisObj, defaultVal });
									return thisObj;
								}
								else {
									return currentScope->FindObj(idname);
								}
							}
						};

						// std::string type_name;
						std::string idname;
					};
					/*
					var x = double();
					*/
					struct Assign_Decl_AST_Node final : AST_Node_Impl {
						Assign_Decl_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Assign_Decl, std::move(t_loc), std::move(t_children))
						{
							this->return_type = this->children[1]->return_type;
						};

						/*! Non-Empty variable assignment:
						  var j = 100;
						*/
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							const std::string& idname = this->children[0]->text;
							Any value = this->children[1]->eval(currentScope);
							if (value.GetFlag(AnyData::Flag::constant)) {
								// clone it
								currentScope->AddObj(idname, std::make_shared<Any>(value.Type().lock()->GetCopyConstructor()(value)));
							}
							else {
								// return as-is
								currentScope->AddObj(idname, std::make_shared<Any>(value));
							}
							return currentScope->FindObj(idname);
						}
					};					
					// ++x
					struct Prefix_AST_Node final : AST_Node_Impl {
						Prefix_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Prefix, std::move(t_loc), std::move(t_children))
							, m_oper(Operators::to_operator(this->text, true))
						{}

						// ++x;
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto temp{ this->children[0]->eval(currentScope) };
							return currentScope->CallFunction(this->text, temp); // we currently do not attempt to validate -- just process the request and see what lands. 
						};

					private:
						Operators::Opers m_oper = Operators::Opers::invalid;
					};
					// x++
					struct Postfix_AST_Node final : AST_Node_Impl {
						Postfix_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Postfix, std::move(t_loc), std::move(t_children))
							, m_oper(Operators::to_operator(this->text, true)) {
						}

						// x++; 
						// depending on the context, is either specifying the type (e.g. _ft, ull) or is modifying the underlying value (++, --)
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							Any var(this->children[0]->eval(currentScope)); // an int, float, etc.                 

							// the type is known. Short-circuit and get out fast. 
							if (m_oper == Operators::Opers::pre_increment) {
								auto out = var.Type().lock()->GetCopyConstructor()(var);
								(void)currentScope->CallFunction("++", var);
								return out;
							}
							else if (m_oper == Operators::Opers::pre_decrement) {
								auto out = var.Type().lock()->GetCopyConstructor()(var);
								(void)currentScope->CallFunction("--", var);
								return out;
							}
							else if (m_oper == Operators::Opers::invalid) {
								if (this->text != "" && this->text.length() >= 1) {
									for (auto& unit_type : Units::value::GetValueTypes()) {
										auto abbreviation = unit_type.second.UnitAbbreviation();
										if (this->text == abbreviation) {
											if (auto Class = currentScope->FindClass(unit_type.first)) {
												return Class->CallFunction(Class->GetName(), var);
											}
											else {
												auto out = Any(unit_type.second);
												currentScope->CallFunction("=", { out, var });
												return out;
											}
										}
									}
								}
								return var;
							}
							else {
								throw exception::eval_error("Only increment (i++) or decrement (i--) operators are supported in a postfix context, as well as custom postfixes.", (File_Position)this->location.start);
							}
						};

						Operators::Opers m_oper = Operators::Opers::invalid;
					};
					// if (Scopeless_Block_AST_Node) Block_AST_Node else Block_AST_Node	
					struct If_AST_Node final : AST_Node_Impl {
						If_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::If, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() >= 2); // CONDITION, IF_TRUE_BLOCK, ELSE_BLOCK
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// create a temporary scope for temporary variables made in the CONDITION
							if (1) {
								auto newScope = std::make_shared<Scope>(currentScope);
								newScope->SetSelf(newScope);
								// evaluate the CONDITION statement within this new scope -- note that the new scope only applies if true! 
								if (newScope->Cast<bool>(this->children[0]->eval(newScope))) {
									return this->children[1]->eval(newScope);
								}
							}

							// if an else-statement is available...
							if (this->children.size() >= 3) {
								auto newScope = std::make_shared<Scope>(currentScope);
								newScope->SetSelf(newScope);

								return this->children[2]->eval(newScope);
							}
							else return Any(); // returns void
						}
					};
					// while (Scopeless_Block_AST_Node) Block_AST_Node
					struct While_AST_Node final : AST_Node_Impl {
						While_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::While, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() >= 2);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							while (currentScope->Cast<bool>(this->children[0]->eval(currentScope))) {
								auto newScope = std::make_shared<Scope>(currentScope);
								newScope->SetSelf(newScope);

								try {
									(void)this->children[1]->eval(newScope);
								}
								catch (detail::Break_Loop&) {
									break;
								}
								catch (detail::Continue_Loop&) {}
							}
							return {};
						}
					};
					// for (INIT_BLOCK; CONDITION_BLOCK; PROGRESS_BLOCK) WORK_BLOCK
					struct For_AST_Node final : AST_Node_Impl {
						For_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::For, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() >= 4);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto forScope = std::make_shared<Scope>(currentScope);
							forScope->SetSelf(forScope);
							{
								// INIT_BLOCK
								if (this->children[0]->identifier != AST_Node_Type::Noop) {
									this->children[0]->eval(forScope); // may include declaring a variable, or nothing at all
								}

								// CONDITION_BLOCK
								while (this->children[1]->identifier == AST_Node_Type::Noop
									||
									forScope->Cast<bool>(this->children[1]->eval(forScope))
									) {
									auto newScope = std::make_shared<Scope>(forScope);
									newScope->SetSelf(newScope);

									try {
										(void)this->children[3]->eval(newScope);
									}
									catch (detail::Continue_Loop&) {}
									catch (detail::Break_Loop&) { break; }

									// PROGRESS_BLOCK
									this->children[2]->eval(forScope);
								}
							}
							return {};
						}
					};
					// for (range_declaration : range_expression) loop_statement
					struct Ranged_For_AST_Node final : AST_Node_Impl {
						Ranged_For_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Ranged_For, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() == 3);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							Any out;
							auto forScope = std::make_shared<Scope>(currentScope);
							forScope->SetSelf(forScope);

							Any item_decl = this->children[0]->eval(forScope); // var x;
							Any range = this->children[1]->eval(forScope); // 0..100 or [0,1,2,3] or vectorObjName etc;

							try {
								auto begin_func = forScope->FindFunction("begin", { range });
								auto end_func = forScope->FindFunction("end", { range });
								if (begin_func && end_func) {
									// user-defined functions for begin() and end() were found -- this is the ideal.
									for (
										auto begin = forScope->CallFunction(begin_func, range),
										end = forScope->CallFunction(end_func, range);
										forScope->Cast<bool>(forScope->CallFunction("!=", { begin, end }));
										forScope->CallFunction("++", begin)
										) {
										forScope->CallFunction(":=", { item_decl, forScope->CallFunction("get", begin) });
										try {
											auto innerScope = std::make_shared<Scope>(forScope);
											innerScope->SetSelf(innerScope);
											out = this->children[2]->eval(innerScope);
										}
										catch (detail::Continue_Loop&) {}
									}
								}
								else { // TO-DO, we should probably just throw an error and give-up...


									// try to fall-back to the built-in GetChildren function and see what we get.
									// Note that this causes a huge lift in the memory requirements for the call due to how this recursive function works... 
									auto cache = GoodLang::GetChildren(range).children[0];
									Any Child;
									for (auto& childWrapper : cache.children) {
										childWrapper.children.clear();
										Child.container = std::move(childWrapper.data);
										forScope->CallFunction(":=", { item_decl, Child });

										try {
											auto innerScope = std::make_shared<Scope>(forScope);
											innerScope->SetSelf(innerScope);
											out = this->children[2]->eval(innerScope);
										}
										catch (detail::Continue_Loop&) {}
									}
								}
							}
							catch (detail::Break_Loop&) {}
							return out;
						};
					};
					// x[1]
					struct Array_Call_AST_Node final : AST_Node_Impl {
						Array_Call_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Array_Call, std::move(t_loc), std::move(t_children))
						{
							auto lhs = this->children[0]->return_type;
							auto rhs = this->children[1]->return_type;
							ParamTypes params({ lhs, rhs });
							if (auto func = currentScope->FindFunction("[]", params, *currentScope->GetTypeConverterTree())) {
								this->return_type = func->Returns();
							}
							else {
								this->return_type = user_type_shared<Var>();
							}
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							try {
								return currentScope->CallFunction("[]", {
									this->children[0]->eval(currentScope),
									this->children[1]->eval(currentScope)
									});
							}
							catch (const GoodLang::exception::not_found_error& e) {
								throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", (File_Position)this->location.start);
							}
						}

					private:
						mutable std::atomic_uint_fast32_t m_loc = { 0 };
					};
					// x.first
					struct Dot_Access_AST_Node final : AST_Node_Impl {
						Dot_Access_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Dot_Access, std::move(t_loc), std::move(t_children))
							, m_fun_name(((this->children[1]->identifier == AST_Node_Type::Fun_Call) || (this->children[1]->identifier == AST_Node_Type::Array_Call))
								? this->children[1]->children[0]->text
								: this->children[1]->text)
						{

						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							std::vector<Any> params{ this->children[0]->eval(currentScope) };
							std::string functionName;

							// what happens next depends on the RHS
							switch (this->children[1]->identifier) {
							case AST_Node_Type::Fun_Call:
								functionName = this->children[1]->children[0]->text;
								for (auto& child : this->children[1]->children[1]->children)
									params.push_back(child->eval(currentScope));
								break;
							default: // case AST_Node_Type::Id:
								functionName = this->children[1]->text;
								break;
							}
							return currentScope->CallFunction(functionName, params);
						}
						const std::string m_fun_name;

					private:
						mutable std::atomic_uint_fast32_t m_loc = { 0 };
						mutable std::atomic_uint_fast32_t m_array_loc = { 0 };

					};
					// [x](FF) async -> int { return x+FF; };
					struct Lambda_AST_Node final : AST_Node_Impl {
						Lambda_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(t_ast_node_text,
								AST_Node_Type::Lambda,
								std::move(t_loc),
								std::vector<AST_Node_Impl_Ptr>(t_children))
							, m_param_names(Arg_List_AST_Node::get_arg_names(*this->children[1]))
							, m_param_types(Arg_List_AST_Node::get_arg_types(*this->children[1]))
							//, m_this_capture(has_this_capture(this->children[0]->children))
							, m_lambda_node(t_children.back())
						{
							const_cast<std::shared_ptr<AST_Node_Impl>&>(m_lambda_node) = this->children.back() = optimizer::optimize(this->children.back(), currentScope);
							this->return_type = user_type_shared<Proxy_Function>();

							// need to immediately optimize the lambda node if at all possible, and reduce the likelihood of throwing (which significantly impacts performance). 
							while (m_lambda_node->identifier == AST_Node_Type::Return) {
								if (m_lambda_node->children.size() == 0) {
									const_cast<std::shared_ptr<AST_Node_Impl>&>(m_lambda_node) = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<Noop_AST_Node>());
								}
								else if (m_lambda_node->children.size() == 1) {
									const_cast<std::shared_ptr<AST_Node_Impl>&>(m_lambda_node) = std::move(m_lambda_node->children[0]);
								}
								else {
									break;
								}
							}
							this->children.back() = m_lambda_node;
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// children[0] -> list of Id's or variables to be captured. 
							// children[1] -> list of (possibly type'd) variables that define the inputs to the function.
							// children[2] -> either Noop or Id whose name is the desired return type
							// children[3] -> m_lambda_node -> function to call

							std::map<std::string, std::shared_ptr<Any>>
								captures;

							for (auto& var_name : Arg_List_AST_Node::get_arg_names(*this->children[0])) {
								if (auto obj = currentScope->FindObj(var_name)) {
									captures[var_name] = obj;
								}
								else {
									throw exception::eval_error("Cannot find captured variable", (File_Position)this->location.start);
								}
							}

							
							std::weak_ptr<Type_Info> returnType = GetClassType(this->children[2], currentScope);
							bool returnVoid = (GetHash(returnType) == GetHash(user_type_shared<void>()));

							if (is_async) {
								if (returnVoid) {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
										param_names = this->m_param_names,
										captures, 
										paramTypes = this->m_param_types
									](
										std::shared_ptr<Scope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = std::make_shared<FunctionScope>(currentScope);
											function_scope->SetSelf(function_scope);

											// insert the captures
											for (auto& capture : captures) {
												function_scope->AddObj(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->AddObj(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i])), false);
												// function_scope->AddObj(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											return parallel::async([
												function_scope,
													lambda_node
											]() -> Any {
													try {
														lambda_node->eval(function_scope);
													}
													catch (detail::Return_Value& rv) {
														// if the retval is anything but void, we should throw an error
														if (!rv.retval.IsEmpty()) {
															throw exception::eval_error("Cannot return with a value inside of a lambda that expects to return void.");
														}
													}
													return Any();
												}).as_promise();
										}, ParamTypes({ user_type_shared<Scope>(), user_type_shared<std::vector<Any>>() }), GoodLang::user_type_shared<GoodLang::parallel::promise>()
											);
								}
								else {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
											param_names = this->m_param_names,
											captures,
											thisReturnType = returnType,
											paramTypes = this->m_param_types
									](
										std::shared_ptr<Scope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = std::make_shared<FunctionScope>(currentScope);
											function_scope->SetSelf(function_scope);

											// insert the captures
											for (auto& capture : captures) {
												function_scope->AddObj(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->AddObj(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i])), false);
												// function_scope->AddObj(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											return parallel::async([
												currentScope_t = currentScope,
													function_scope_t = function_scope,
													lambda_node_t = lambda_node,
													thisReturnType_t = thisReturnType
											]() -> Any {
													Any lambda_result;
													try {
														lambda_result = lambda_node_t->eval(function_scope_t);
													}
													catch (detail::Return_Value& rv) {
														lambda_result = rv.retval;
													}

													if (thisReturnType_t.expired()) {
														return lambda_result;
													}
													else {
														return function_scope_t->Cast(lambda_result, thisReturnType_t);
													}
												}).as_promise();
										}, ParamTypes({ user_type_shared<Scope>(), user_type_shared<std::vector<Any>>() }), GoodLang::user_type_shared<GoodLang::parallel::promise>()
											);
								}
							}
							else {
								if (returnVoid) {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
											param_names = this->m_param_names,
											captures,
											paramTypes = this->m_param_types
									](
										std::shared_ptr<Scope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = std::make_shared<FunctionScope>(currentScope);
											function_scope->SetSelf(function_scope);

											// insert the captures
											for (auto& capture : captures) {
												function_scope->AddObj(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->AddObj(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i])), false);
												// function_scope->AddObj(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											try {
												lambda_node->eval(function_scope);
											}
											catch (detail::Return_Value& rv) {
												// if the retval is anything but void, we should throw an error
												if (!rv.retval.IsEmpty()) {
													throw exception::eval_error("Cannot return with a value inside of a lambda that expects to return void.");
												}
											}
											return Any();
										}, ParamTypes({ user_type_shared<Scope>(), user_type_shared<std::vector<Any>>() })
											);
								}
								else {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
										param_names = this->m_param_names,
										captures,
										thisReturnType = returnType,
										paramTypes = this->m_param_types
									](
										std::shared_ptr<Scope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = std::make_shared<FunctionScope>(currentScope);
											function_scope->SetSelf(function_scope);

											// insert the captures
											for (auto& capture : captures) {
												function_scope->AddObj(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->AddObj(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i])), false);
												// function_scope->AddObj(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											Any lambda_result;
											try {
												lambda_result = lambda_node->eval(function_scope);
											}
											catch (detail::Return_Value& rv) {
												lambda_result = rv.retval;
											}

											if (thisReturnType.expired()) {
												return lambda_result;
											}
											else {
												return function_scope->Cast(lambda_result, thisReturnType);
											}
										}, ParamTypes({ user_type_shared<Scope>(), user_type_shared<std::vector<Any>>()/*, user_type_shared<AST_Node_Impl>()*/ }), returnType.expired() ? user_type_shared<Any>() : returnType
											);
								}
							}
						}

					public:
						bool is_async = false;

					private:
						const std::vector<std::weak_ptr<Type_Info>> m_param_types; 
						const std::vector<std::string> m_param_names;
						const std::shared_ptr<AST_Node_Impl> m_lambda_node;
					};
					// [0, 1, 2, 3]
					struct Inline_Array_AST_Node final : AST_Node_Impl {
						Inline_Array_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Inline_Array, std::move(t_loc), std::move(t_children))
						{
							this->return_type = user_type_shared<Vector<Var>>();
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// assumes the first child is an ArgList or Arg
							Vector<Var> vec;
							if (!this->children.empty()) {
								vec.reserve(this->children[0]->children.size());
								for (auto& child : this->children[0]->children) {
									vec.push_back(Var(child->eval(currentScope)));
								}
							}
							return vec;
						}

					private:
						mutable std::atomic_uint_fast32_t m_loc = { 0 };
					};
					struct Map_Pair_AST_Node final : AST_Node_Impl {
						Map_Pair_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Map_Pair, std::move(t_loc), std::move(t_children))
						{}
					};
					// ["":10, 10:10, Vector():10, 20:Vector()]
					struct Inline_Map_AST_Node final : AST_Node_Impl {
						Inline_Map_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Inline_Map, std::move(t_loc), std::move(t_children))
						{
							this->return_type = user_type_shared<Map<size_t, std::pair<Var, Var>>>();
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// assumes the first child is an ArgList or Arg
							Any out{ Map<size_t, std::pair<Var, Var>>() };
							if (!this->children.empty()) {
								for (const auto& child : this->children[0]->children) {
									currentScope->CallFunction("emplace", { out, child->children[0]->eval(currentScope), child->children[1]->eval(currentScope) });
								}
							}
							return out;
						};

					};

					// parallel_for (var x = START_VALUE ; END_VALUE) WORK_BLOCK; // this approach means every iteration will see it's own local "x"
					// parallel_for (START_VALUE ; END_VALUE) WORK_BLOCK // this approach means every iteration will NOT see any "x" at all
					struct Parallel_For_AST_Node final : AST_Node_Impl {
						Parallel_For_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Parallel_For, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() == 3);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto forScope = std::make_shared<Scope>(currentScope);
							forScope->SetSelf(forScope);

							if (1) { // parallel_for (var x = START_VALUE; END_VALUE) WORK_BLOCK
								int startPos{ 0 }, endPos{ 0 };
								if (1) {
									auto temp_memory = std::make_shared<Scope>(forScope);
									temp_memory->SetSelf(temp_memory);
									startPos = temp_memory->Cast<int>(this->children[0]->eval(temp_memory));
									endPos = temp_memory->Cast<int>(this->children[1]->eval(temp_memory));
								}
								if (startPos > endPos) {
									int temp = endPos;
									endPos = startPos;
									startPos = temp;
								}
								if (endPos > startPos) {
									impl::context ctx;
									using DateStorageType = GoodLang::Union<Any, std::shared_ptr<Scope>, std::weak_ptr<details::Proxy_Function_Base>>;
									impl::Dispatch(ctx,
										endPos - startPos /* count of jobs */,
										[&](impl::JobArgs const& _args)-> void {
											DateStorageType& shared_memory
												= *((DateStorageType*)_args.sharedmemory);
											if (_args.groupIndex == 0) {
												// start of a group
												shared_memory.get<1>()->CallFunction(":=", { shared_memory.get<0>(), _args.jobIndex });
											}
											else {
												//if (auto func = shared_memory.get<2>().lock()) 
												//	shared_memory.get<1>()->CallFunction(func, shared_memory.get<0>());
												//else 
												shared_memory.get<1>()->CallFunction("++", shared_memory.get<0>());
											}

											// do the work
											try {
												auto newScope = std::make_shared<Scope>(shared_memory.get<1>());
												newScope->SetSelf(newScope);

												this->children[2]->eval(newScope);
											}
											catch (detail::Continue_Loop&) {}
										},
										sizeof(DateStorageType) /* size of shared memory */,
											[&](void* p) -> void {
											new (p) DateStorageType{
												Any{},
												std::make_shared<Scope>(forScope),
												Proxy_Function{}
											}; // initialize the shared memory
											DateStorageType& iter =
												*static_cast<DateStorageType*>(p);
											iter.get<1>()->SetSelf(iter.get<1>());
											iter.get<0>() = this->children[0]->eval(iter.get<1>()); // e.g. int x = 0 or var& x = 0
											//if (auto Type = iter.get<0>().TypePtr()) {
											//	if (Type->is_any() || Type->is_void()) { }
											//	else iter.get<2>() = iter.get<1>()->FindFunction("++", { iter.get<0>() }, *iter.get<1>()->GetTypeConverterTree());							
											//}
										},
											[&](void* p) -> void {
											((DateStorageType*)p)->~DateStorageType(); // destroy the shared memory
										}
										);
									try {
										impl::Wait(ctx);
									}
									catch (detail::Break_Loop&) {};
								}
							}

							return Any();
						}
					};

					// parallel_for (range_declaration : range_expression) loop_statement;
					struct Parallel_Ranged_For_AST_Node final : AST_Node_Impl {
						Parallel_Ranged_For_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Parallel_Ranged_For, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() == 3);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto forScope = std::make_shared<Scope>(currentScope);
							forScope->SetSelf(forScope);

							Any range = this->children[1]->eval(forScope); // 0..100 or [0,1,2,3] or vectorObjName etc;
							try {
								auto begin_func = forScope->FindFunction("begin", { range });
								auto end_func = forScope->FindFunction("end", { range });
								if (begin_func && end_func) {
									Any begin = forScope->CallFunction(begin_func, range);
									Any end = forScope->CallFunction(end_func, range);
									auto& copyConstructorFunctor = begin.Type().lock()->GetCopyConstructor();

									// if we can get the distance quickly, then great
									size_t count = 0;
									if (auto distanceFunction = forScope->FindFunction("-", { end, begin })) {
										count = forScope->Cast<size_t>(forScope->CallFunction(distanceFunction, { end, begin }));
									}
									else {
										while (forScope->Cast<bool>(forScope->CallFunction("!=", { begin, end }))) {
											count++;
											forScope->CallFunction("++", begin);
										}
										begin = forScope->CallFunction(begin_func, range);
									}

									using shared_type = std::pair< std::pair<Any, Any>, std::shared_ptr<Scope>>;
									impl::context ctx;
									impl::Dispatch(ctx,
										count,
										[&](impl::JobArgs const& _args)-> void {
											shared_type& iter = *static_cast<shared_type*>(_args.sharedmemory);
											if (_args.groupIndex == 0) {
												// start of a group
												if (auto jumpFunction = iter.second->FindFunction("+=", { iter.first.first, _args.jobIndex })) {
													iter.second->CallFunction(jumpFunction, { iter.first.first, _args.jobIndex });
												}
												else {
													for (int i = 0; i < _args.jobIndex; i++) iter.second->CallFunction("++", iter.first.first);
												}
											}
											else {
												// within a group, we know the jobs are done in sequence, so we can safely increment by 1.
												iter.second->CallFunction("++", iter.first.first);
											}
											iter.second->CallFunction(":=", { iter.first.second, iter.second->CallFunction("get", iter.first.first) });

											// do the work
											try {
												auto innerScope = std::make_shared<Scope>(iter.second);
												innerScope->SetSelf(innerScope);
												this->children[2]->eval(innerScope);
											}
											catch (detail::Continue_Loop&) {}
										},
										sizeof(shared_type),
											[&](void* p) -> void {
											new (p) shared_type{ std::pair<Any,Any>{}, std::make_shared<Scope>(forScope) };
											shared_type& iter = *static_cast<shared_type*>(p);
											iter.second->SetSelf(iter.second);
											iter.first.first = copyConstructorFunctor(begin); // iterator
											iter.first.second = this->children[0]->eval(iter.second); // var x;
										},
											[](void* p) -> void {
											((shared_type*)p)->~shared_type();
										}
										);
									impl::Wait(ctx);
								}
								else {
									throw exception::eval_error("begin() and/or end() functions were not found for the provided type", (File_Position)this->location.start);
								}
							}
							catch (detail::Break_Loop&) {}

							return Any();
						}
					};

					struct Break_AST_Node final : AST_Node_Impl {
						Break_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Break, std::move(t_loc), std::move(t_children)) {
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override { throw detail::Break_Loop(); }
					};

					struct Continue_AST_Node final : AST_Node_Impl {
						Continue_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Continue, std::move(t_loc), std::move(t_children)) {
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override { throw detail::Continue_Loop(); }
					};

					struct Case_AST_Node final : AST_Node_Impl {
						Case_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Case, std::move(t_loc), std::move(t_children))
						{
							assert(this->children.size() == 2);
							// if this is a constant, its hash should also be a constant
							if (this->children[0]->identifier == AST_Node_Type::Constant) {
								try {
									constexprHash = currentScope->Cast<size_t>(currentScope->CallFunction("to_hash", { std::dynamic_pointer_cast<Constant_AST_Node>(this->children[0])->m_value }));
									// don't need the first child anymore
									const_cast<std::string&>(this->text) = this->children.front()->text;
									this->children.front() = this->children.back();
									this->children.pop_back();
								}
								catch (...) {}
							}
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto thisScope = std::make_shared<Scope>(currentScope);
							thisScope->SetSelf(thisScope);
							return this->children.back()->eval(thisScope);
						}

						std::optional<size_t> constexprHash;
					};

					struct Switch_AST_Node final : AST_Node_Impl {
						Switch_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Switch, std::move(t_loc), std::move(t_children)) {
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto thisScope = std::make_shared<Scope>(currentScope);
							thisScope->SetSelf(thisScope);

							bool breaking = false;
							size_t currentCase = 1;
							bool hasMatched = false;

							size_t match_hash = currentScope->Cast<size_t>(currentScope->CallFunction("to_hash", { this->children[0]->eval(thisScope) }));

							Any out;
							while (!breaking && (currentCase < this->children.size())) {
								try {
									if (this->children[currentCase]->identifier == AST_Node_Type::Case) {
										if (hasMatched) {
											out = this->children[currentCase]->eval(thisScope);
										}
										else {
											std::optional<size_t>& constexprHash = std::dynamic_pointer_cast<Case_AST_Node>(this->children[currentCase])->constexprHash;
											size_t this_hash;
											if (constexprHash.has_value()) {
												// best-case scenario
												this_hash = constexprHash.value();
											}
											else {
												this_hash = currentScope->Cast<size_t>(currentScope->CallFunction("to_hash", { this->children[currentCase]->children[0]->eval(thisScope) }));
											}

											// This is a little odd, but because want to see both the switch and the case simultaneously, I do a downcast here.
											if (hasMatched || (this_hash == match_hash)) {
												out = this->children[currentCase]->eval(thisScope);
												hasMatched = true;
											}
										}
									}
									else if (this->children[currentCase]->identifier == AST_Node_Type::Default) {
										out = this->children[currentCase]->eval(thisScope);
										// hasMatched = true;
									}
								}
								catch (detail::Break_Loop&) {
									breaking = true;
								}
								++currentCase;
							}
							return out;
						}

						mutable std::atomic_uint_fast32_t m_loc = { 0 };
					};

					struct Default_AST_Node final : AST_Node_Impl {
						Default_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Default, std::move(t_loc), std::move(t_children)) {
							assert(this->children.size() == 1);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto thisScope = std::make_shared<Scope>(currentScope);
							thisScope->SetSelf(thisScope);
							return this->children[0]->eval(thisScope);
						}
					};

					struct Do_AST_Node final : AST_Node_Impl {
						Do_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Do, std::move(t_loc), std::move(t_children))
						{}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							Any retval;

							auto thisScope = std::make_shared<Scope>(currentScope);
							thisScope->SetSelf(thisScope);

							std::exception_ptr err{ nullptr };

							try { retval = this->children[0]->eval(thisScope); }
							catch (...) { err = std::current_exception(); }

							if (this->children.back()->identifier == AST_Node_Type::Finally)
								this->children.back()->children[0]->eval(thisScope);

							if (err)
								std::rethrow_exception(err);

							return retval;
						}
					};

					struct Try_AST_Node final : AST_Node_Impl {
						Try_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Try, std::move(t_loc), std::move(t_children))
						{}

						Any handle_exception(const std::shared_ptr<Scope>& currentScope, const Any& t_except, bool& rethrow) const {
							Any retval;
							bool wasCaught = false;
							for (int i = 1; i < this->children.size(); i++) {
								auto& catch_block = *this->children[i];
								if (catch_block.identifier == AST_Node_Type::Finally) {
									continue;
								}
								//else if (catch_block.identifier == AST_Node_Type::Noop) {
								//	// catches anything, but doesn't provide a type or var name. Therefore this will always catch, regardless of t_except's type
								//	retval = catch_block.children[1]->eval(currentScope);
								//}
								else if (catch_block.identifier == AST_Node_Type::Catch) {
									if (catch_block.children.size() == 1) { // catch{ ... }
										// No variable capture
										retval = catch_block.children[0]->eval(currentScope);
										wasCaught = true;
										break;
									}
									else {
										// variable capture
										if (catch_block.children[0]->identifier == AST_Node_Type::Arg) {
											if (catch_block.children[0]->children.size() == 1) {
												// catch(e){...}
												auto& varName = catch_block.children[0]->children[0]->text; // e.g. x

												currentScope->AddObj(varName, std::make_shared<Any>(t_except));
												retval = catch_block.children[1]->eval(currentScope);
												wasCaught = true;
												break;
											}
											else {
												// catch(exception& e){...}
												auto& varTypeName = catch_block.children[0]->children[0]->text; // e.g. exception&
												auto& varName = catch_block.children[0]->children[1]->text; // e.g. x

												// ensure the var type matches

												// TO-DO

												currentScope->AddObj(varName, std::make_shared<Any>(t_except));
												retval = catch_block.children[1]->eval(currentScope);
												wasCaught = true;
												break;
											}
										}
										else {
											throw exception::eval_error("Internal error: catch block variable unrecognized", (File_Position)this->location.start);
										}
									}
								}
								else {
									throw exception::eval_error("Internal error: catch block type unrecognized", (File_Position)this->location.start);
								}
							}

							if (!wasCaught) {
								rethrow = true;
							}

							return retval;
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							Any retval;
							Any err;
							std::exception_ptr Error;
							bool rethrow = false;
							auto thisScope = std::make_shared<Scope>(currentScope);
							thisScope->SetSelf(thisScope);

							try {
								retval = this->children[0]->eval(thisScope);
							}
							catch (const std::exception& e) {
								Error = std::current_exception();
								// this must be handled within this scope before the exception goes out-of-scope.
								auto exception = Any(std::shared_ptr<std::exception>(const_cast<std::exception*>(&e), [](std::exception* p) { /* do nothing */ }));
								retval = handle_exception(thisScope, exception, rethrow);
							}
							catch (Any& e) {
								Error = std::current_exception();
								retval = handle_exception(thisScope, e, rethrow);
							}
							catch (...) {
								Error = std::current_exception();
								// unhandled exception type
								if (this->children.back()->identifier == AST_Node_Type::Finally) {
									this->children.back()->children[0]->eval(thisScope);
								}
								rethrow = true;
							}

							if (rethrow) {
								std::rethrow_exception(Error);
							}

							if (this->children.back()->identifier == AST_Node_Type::Finally) {
								retval = this->children.back()->children[0]->eval(thisScope);
							}

							return retval;
						}
					};

					struct Catch_AST_Node final : AST_Node_Impl {
						Catch_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Catch, std::move(t_loc), std::move(t_children))
						{}
					};

					struct Finally_AST_Node final : AST_Node_Impl {
						Finally_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Finally, std::move(t_loc), std::move(t_children))
						{}
					};

					/*! Currently, the JIT compilation does not support preprocessor macros or other preprocessor activities. */
					struct JustInTimeCompilation_AST_Node final : AST_Node_Impl {
						JustInTimeCompilation_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::JustInTimeCompilation, std::move(t_loc), std::move(t_children))
						{
							if (this->children[0]->identifier == AST_Node_Type::Constant) {
								auto& scriptVar = std::dynamic_pointer_cast<Constant_AST_Node>(this->children[0])->m_value;
								if (scriptVar.IsTypeOf<std::string>()) {
									Compile(scriptVar.cast<std::string&>(), currentScope, true);
								}
								else {
									auto SCRIPT = currentScope->Cast<std::string>(currentScope->CallFunction("to_string", { scriptVar }));
									Compile(SCRIPT, currentScope, true);
								}
							}
						}

						// eval("x + 1");
						// Each thread will compile its own code. If a thread sees the same code again, it will not re-compile it.
						// Pre-processor macros are not supported at this time. To do so would require text splicing, running the preprocessor, and then resuming the code here.
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							if (this->children[0]->identifier == AST_Node_Type::Constant) {
								// consider this already done
							}
							else {
								auto SCRIPT = currentScope->Cast<std::string>(currentScope->CallFunction("to_string", { this->children[0]->eval(currentScope) }));
								Compile(SCRIPT, currentScope);
							}
							return TLS->second->eval(currentScope);
						};

					private:
						GoodLang::ThreadLocalInstance <
							std::pair < std::string, AST_Node_Impl_Ptr >
						> TLS;
						void Compile(std::string const& SCRIPT, const std::shared_ptr<Scope>& currentScope, bool UpdateAll = false) const {
							if (UpdateAll) {
								auto PARSER = GoodLang::Engine::Compiler::Interpreter::Parser();
								auto PARSED_RESULT = PARSER.Parse(SCRIPT, currentScope);
								const_cast<GoodLang::ThreadLocalInstance<std::pair<std::string, AST_Node_Impl_Ptr>>&>(TLS) =
									std::pair < std::string, AST_Node_Impl_Ptr >(SCRIPT, PARSED_RESULT.first);
							}
							else {
								if ((!TLS->second) || (TLS->first != SCRIPT)) {
									auto PARSER = GoodLang::Engine::Compiler::Interpreter::Parser();
									auto PARSED_RESULT = PARSER.Parse(SCRIPT, currentScope);

									const_cast<std::string&>(TLS->first) = SCRIPT;
									const_cast<AST_Node_Impl_Ptr&>(TLS->second) = PARSED_RESULT.first;
								}
							}
						};
					};

					struct Scopeless_Block_AST_Node final : AST_Node_Impl {
						Scopeless_Block_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Scopeless_Block, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() > 0) this->return_type = this->children.back()->return_type;
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							// evaluate all of the children, return the result of the last child
							int numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(currentScope);
								}
								return this->children.back()->eval(currentScope);
							}
							else {
								return Any();
							}
						}
					};

					struct Block_AST_Node final : AST_Node_Impl {
						Block_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Block, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() > 0) this->return_type = this->children.back()->return_type;
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto newScope = std::make_shared<Scope>(currentScope);
							newScope->SetSelf(newScope);

							const auto numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(newScope);
								}
								return this->children.back()->eval(newScope);
							}
							else {
								return Any();
							}
						};
					};

					struct Function_Block_AST_Node final : AST_Node_Impl {
						Function_Block_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::FunctionBlock, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() > 0) this->return_type = this->children.back()->return_type;
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							auto newScope = std::make_shared<FunctionScope>(currentScope);
							newScope->SetSelf(newScope);

							const auto numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(newScope);
								}
								return this->children.back()->eval(newScope);
							}
							else {
								return Any();
							}
						};
					};

					struct Fold_Right_Binary_Operator_AST_Node : AST_Node_Impl {
						Fold_Right_Binary_Operator_AST_Node(const std::shared_ptr<Scope>& currentScope, const std::string& t_oper, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children, Any t_rhs)
							: AST_Node_Impl(t_oper, AST_Node_Type::BinaryFoldRight, std::move(t_loc), std::move(t_children))
							, m_oper(Operators::to_operator(t_oper))
							, m_rhs(std::move(t_rhs))
						{}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return currentScope->CallFunction(this->text, { this->children[0]->eval(currentScope), m_rhs });
						};

					private:
						Operators::Opers m_oper;
						Any m_rhs;
					};

					struct Namespace_AST_Node final : AST_Node_Impl {
						Namespace_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Namespace, std::move(t_loc), std::move(t_children))
						{
							(void)this->children.back()->eval(currentScope);
						}
						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							return {};
						};
					};

					struct Class_AST_Node final : AST_Node_Impl {
						Class_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::Namespace, std::move(t_loc), std::move(t_children))
						{
							(void)this->children.back()->eval(currentScope);
						}

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {							
							return {};
						}
					};

					struct Declaration_Block_AST_Node final : AST_Node_Impl {
						Declaration_Block_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::DeclarationBlock, std::move(t_loc), std::move(t_children))
						{
							if (this->children.size() > 0) this->return_type = this->children.back()->return_type;
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {
							const auto numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(currentScope);
								}
								this->children.back()->eval(currentScope);
							}
							else {
								return {};
							}
						};
					};

					struct FunctionDecl_AST_Node final : AST_Node_Impl {
						FunctionDecl_AST_Node(const std::shared_ptr<Scope>& currentScope, std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), AST_Node_Type::FunctionDecl, std::move(t_loc), std::move(t_children))
							, inputArgNames(Arg_List_AST_Node::get_arg_names(*this->children[2]))
							, inputArgTypes(Arg_List_AST_Node::get_arg_types(*this->children[2]))
						{
							assert(this->children.size() == 4); // Id -> return_type, Id -> function_name, Arg_List, Block

							function_name = GetText(this->children[1]);
							numArgs = this->children[2]->children.size();
							FunctionBlock = this->children[3] = optimizer::optimize(this->children[3], currentScope);
							return_type_name = GetText(this->children[0]);
							this->return_type = GetClassType(this->children[0], currentScope);
						};

						static void AddObjects(int startposition, std::shared_ptr< Scope> const& thisScope, std::vector<std::string> const& argNames, std::vector<std::weak_ptr<Type_Info>> const& argTypes) {};
						template<typename T, typename... R> static void AddObjects(int startposition, std::shared_ptr< Scope> const& thisScope, std::vector<std::string> const& argNames, std::vector<std::weak_ptr<Type_Info>> const& argTypes, T const& argument, R const&... arguments) {
							thisScope->AddObj(argNames[startposition], std::make_shared<Any>(thisScope->Cast(argument, argTypes[startposition])), false);
							AddObjects(startposition + 1, thisScope, argNames, argTypes, arguments...);
						};

						Any eval_internal(const std::shared_ptr<Scope>& currentScope) const override {							
							if (currentScope->IsClass()) {
								if (auto ptr = std::dynamic_pointer_cast<Class>(currentScope)) {
									auto p_locked = initialized.Unique();
									if (!*p_locked) {
										auto& types = const_cast<std::vector<std::weak_ptr<Type_Info>>&>(inputArgTypes);
										auto& names = const_cast<std::vector<std::string>&>(inputArgNames);

										types.insert(types.begin(), ptr->GetClassType().lock()->MakeConstRef());
										names.insert(names.begin(), "this");
										const_cast<int&>(numArgs)++;
										const_cast<bool&>(*p_locked) = true;
									}
								}
							}

							// the current scope should be a namespace... HOPEFULLY! Need to confirm... Perhaps also need to change the impl based on whether we are in a Class or in a Namespace or in a Global?
							Proxy_Function func;
							if (GetHash(this->return_type) == GetHash(user_type<void>())) {
								switch (numArgs) {
								case 0:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									]() {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 1:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& in1) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 2:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& in1, Any const& in2) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1, in2);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 3:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 4:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 5:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 6:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 7:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 8:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 9:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 10:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 11:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 12:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l) {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 13:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 14:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 15:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n, Any const& o) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;
								}
							}
							else {
								switch (numArgs) {
								case 0:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									]()->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 1:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& in1)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 2:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& in1, Any const& in2)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1, in2);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 3:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 4:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 5:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 6:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 7:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 8:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 9:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 10:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 11:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 12:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 13:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 14:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;

								case 15:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scope>(currentScope), returnType = this->return_type
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n, Any const& o)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = std::make_shared<Scope>(parentScope);
											thisScope->SetSelf(thisScope);

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											return thisScope->Cast(result, returnType);
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->return_type);
									break;





								}
							};

							if (func) {
								currentScope->AddFunction(function_name, func);
							}
							else {
								throw exception::eval_error("Function has too many parameters!");
							}
							return {};
						};

						GoodLang::SharedLockable<bool> initialized{ false };
						std::string return_type_name;
						std::string function_name;
						int numArgs;
						AST_Node_Impl_Ptr FunctionBlock;
						std::vector<std::weak_ptr<Type_Info>> inputArgTypes;
						std::vector<std::string> inputArgNames;
					};
				};

				class optimizer {
				private:
					template<typename... T> struct Optimizer : T... {
						Optimizer() = default;
						explicit Optimizer(T... t) : T(std::move(t))... { };
						AST_Node_Impl_Ptr optimize(AST_Node_Impl_Ptr p, const std::shared_ptr<Scope>& currentScope) {
							long long maxDepth = 100;
							while (--maxDepth >= 0) {
								bool successful = false;
								((successful = (successful || static_cast<T&>(*this).optimize(p, currentScope))), ...); // this line performs all optimizations in-line
								if (!successful) break;
							}
							return p;
						};
					};

					static AST_Node_Impl& child_at(AST_Node_Impl& node, const size_t offset) noexcept {
							return *node.children[offset];
					};
					static const AST_Node_Impl& child_at(const AST_Node_Impl& node, const size_t offset) noexcept {
						return *node.children[offset];
					};
					static size_t child_count(const AST_Node_Impl& node) noexcept {
						return node.children.size();
					};
					static bool contains_var_decl_in_scope(const AST_Node_Impl& node) noexcept {
						if (
							node.identifier == AST_Node_Type::Var_Decl
							|| node.identifier == AST_Node_Type::Assign_Decl
							|| node.identifier == AST_Node_Type::Reference
							|| node.identifier == AST_Node_Type::Assign_Retroactively
							|| node.identifier == AST_Node_Type::Def
							|| node.identifier == AST_Node_Type::Class
							) {
							return true;
						}

						const auto num = child_count(node);

						for (size_t i = 0; i < num; ++i) {
							const auto& child = child_at(node, i);
							if (child.identifier != AST_Node_Type::Block
								&& child.identifier != AST_Node_Type::For
								&& child.identifier != AST_Node_Type::Ranged_For
								&& child.identifier != AST_Node_Type::Parallel_For
								&& child.identifier != AST_Node_Type::Parallel_Ranged_For
								&& contains_var_decl_in_scope(child)
								) {
								return true;
							}
						}

						return false;
					};

					// re-arrange the return statement, to avoid throwing whenever possible
					struct Example {
						bool optimize(AST_Node_Impl_Ptr& p, const std::shared_ptr<Scope>& currentScope) {
							return false; // does nothing
						}
					};

					// String embedding results in a structure that may resemble:
					//		ArgList -> {  File -> {   Constant   }  }
					// This should be simplified to: 
					//		ArgList -> {  Constant  }
					struct ArgListFileConstant {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Arg_List
								&& node->children.size() == 1
								&& node->children[0]->identifier == AST_Node_Type::File
								&& node->children[0]->children.size() == 1
								&& node->children[0]->children[0]->identifier == AST_Node_Type::Constant
								) {
								node->children[0] = std::move(node->children[0]->children[0]);
								return true;
							}
							return false;
						}
					};

					// converts from:
					//		var x = int(1)
					// to:
					//		int x{ 1 };
					struct VarDeclEquation_To_RetroactiveAssignment {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Equation
								&& node->children.size() == 2
								&& ((node->children[0]->identifier == AST_Node_Type::Reference) || (node->children[0]->identifier == AST_Node_Type::Var_Decl))
								&& node->children[0]->children.size() == 1
								// && node->children[0]->children[0]->identifier == AST_Node_Type::Id
								// && node->children[1]->identifier == AST_Node_Type::Fun_Call
								&& ((node->text == "=") || (node->text == ":="))
								) {
								node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Assign_Retroactively_AST_Node>(
									currentScope, node->text, node->location, std::vector<AST_Node_Impl_Ptr>{
										std::move(node->children[1]),
										std::move(node->children[0])
									}
								));
								return true;
							}

							if (node->identifier == AST_Node_Type::Equation
								&& node->children.size() == 2
								&& node->children[0]->identifier == AST_Node_Type::Assign_Retroactively
								&& ((node->text == "=") || (node->text == ":="))
								) {
								node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Assign_Retroactively_AST_Node>(
									currentScope, node->text, node->location, std::vector<AST_Node_Impl_Ptr>{
										std::move(node->children[0]->children[0]),
										std::move(node->children[0]->children[1]),
										std::move(node->children[1])
									}
								));
								return true;
							}




							return false;
						}
					};

					// String embedding results in a structure that may resemble:
					//		Fun_Call -> {  Id{ to_string }, Arg_list{ Constant{} } }
					// This should be simplified and completed:
					//      Constant(to_string(Constant()))
					struct ToStringFunctionCallWithConstant {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Fun_Call
								&& node->children.size() == 2
								&& node->children[0]->identifier == AST_Node_Type::Id
								&& node->children[1]->identifier == AST_Node_Type::Arg_List
								&& node->children[1]->children.size() == 1
								&& node->children[1]->children[0]->identifier == AST_Node_Type::Constant
								&& node->children[0]->text == "to_string"
								) {
								const Any& rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[1]->children[0].get())->m_value;
								try {
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, currentScope->CallFunction("to_string", const_cast<Any&>(rhs))
									));
									return true;
								}
								catch (...) {
									// failure to fold -- that's OK
									return false;
								}
							}
							return false;
						}
					};

					// removes items from Blocks that are unecessary (e.g. floating code) or will never be hit (e.g. following return statements)
					struct Dead_Code {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if ((node->identifier == AST_Node_Type::Block) || (node->identifier == AST_Node_Type::Scopeless_Block)) {
								std::vector<size_t> keepers;
								const auto num_children = node->children.size();
								keepers.reserve(num_children);
								bool foundReturnStatement = false;
								for (size_t i = 0; i < (num_children - 1); ++i) {
									const auto& child = *node->children[i];
									switch (child.identifier) {
									case AST_Node_Type::Constant: // 50.0f;
									case AST_Node_Type::Noop: // comments
									case AST_Node_Type::Id: // y, x, etc.
										break;
									case AST_Node_Type::Return: // return; return x; return 50; etc.
										keepers.push_back(i);
										i = num_children; // stop considering the remaining items -- they'll never be found anyways. 
										foundReturnStatement = true;
										break;
									default:
										keepers.push_back(i);
										break;
									}
								}
								if ((!foundReturnStatement) && (num_children > 0)) { keepers.push_back(num_children - 1); };

								if (keepers.size() == num_children) {
									return false;
								}
								else {
									const auto new_children = [&]() {
										std::vector<AST_Node_Impl_Ptr> retval;
										for (const auto x : keepers) {
											retval.push_back(std::move(node->children[x]));
										}
										return retval;
									};

									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Block_AST_Node>(currentScope, node->text, node->location, new_children()));

									return true;
								}
							}
							else {
								return false;
							}
						}
					};

					// re-arrange the return statement, to avoid throwing whenever possible
					struct Return {
						bool optimize(AST_Node_Impl_Ptr& p, const std::shared_ptr<Scope>& currentScope) {
							if ((p->identifier == AST_Node_Type::Lambda) && !p->children.empty()) {
								auto& last_child = p->children.back();
								if (last_child->identifier == AST_Node_Type::Block || last_child->identifier == AST_Node_Type::Scopeless_Block) {
									auto& block_last_child = last_child->children.back();
									if (block_last_child->identifier == AST_Node_Type::Return) {
										if (block_last_child->children.size() == 1) {
											block_last_child = std::move(block_last_child->children[0]);
											return true;
										}
										else if (block_last_child->children.size() == 0) {
											block_last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
											return true;
										}
									}
								}
								//if (last_child->identifier == AST_Node_Type::Return) {
								//	if (last_child->children.size() == 1) {
								//		last_child = std::move(last_child->children[0]);
								//		return true;
								//	}
								//	else if (last_child->children.size() == 0) {
								//		last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<Noop_AST_Node>());
								//		return true;
								//	}
								//}
							}
							if ((p->identifier == AST_Node_Type::Def) && !p->children.empty()) {
								auto& last_child = p->children.back();
								if (last_child->identifier == AST_Node_Type::Block || last_child->identifier == AST_Node_Type::Scopeless_Block) {
									auto& block_last_child = last_child->children.back();
									if (block_last_child->identifier == AST_Node_Type::Return) {
										if (block_last_child->children.size() == 1) {
											last_child->children.back() = std::move(block_last_child->children[0]);
											return true;
										}
									}
								}
							}
							if (p->identifier == AST_Node_Type::File && !p->children.empty()) {
								auto& last_child = p->children.back();
								if (last_child->identifier == AST_Node_Type::Block || last_child->identifier == AST_Node_Type::Scopeless_Block) {
									auto& block_last_child = last_child->children.back();
									if (block_last_child->identifier == AST_Node_Type::Return) {
										if (block_last_child->children.size() == 0) {
											last_child->children.back() = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
											return true;
										}
										else if (block_last_child->children.size() == 1) {
											last_child->children.back() = std::move(block_last_child->children[0]);
											return true;
										}
									}
								}
								else if (last_child->identifier == AST_Node_Type::Return) {
									if (last_child->children.size() == 0) {
										last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
										return true;
									}
									else if (last_child->children.size() == 1) {
										last_child = std::move(last_child->children[0]);
										return true;
									}
								}
							}
							return false;
						}
					};

					// removes the scope from blocks if they do not have declarations at all
					struct Block {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Block) {
								if (!contains_var_decl_in_scope(*node)) {
									if (node->children.size() == 1) {
										node = std::move(node->children[0]);
										return true;
									}
									else {
										node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Scopeless_Block_AST_Node>(
											currentScope,
											node->text,
											node->location,
											std::move(node->children)
										));
										return true;
									}
								}
							}
							else if (node->identifier == AST_Node_Type::Scopeless_Block) {
								if (!contains_var_decl_in_scope(*node)) {
									if (node->children.size() == 1) {
										node = std::move(node->children[0]);
										return true;
									}
								}
							}
							return false;
						}
					};

					// If a function call's return value was going to be unused, there may be no point to holding onto it. 
					struct Unused_Fun_Return {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							bool result = false;
							if ((node->identifier == AST_Node_Type::Block || node->identifier == AST_Node_Type::Scopeless_Block) && !node->children.empty()) {
								for (size_t i = 0; i < node->children.size() - 1; ++i) {
									auto child = node->children[i].get();
									if (child->identifier == AST_Node_Type::Fun_Call) {
										node->children[i] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Unused_Return_Fun_Call_AST_Node>(
											currentScope,
											child->text,
											child->location,
											std::move(child->children)
											));
										result = true;
									}
								}
							}
							else if ((node->identifier == AST_Node_Type::For || node->identifier == AST_Node_Type::While) && child_count(*node) > 0) {
								auto& child = child_at(*node, child_count(*node) - 1);
								if (child.identifier == AST_Node_Type::Block || child.identifier == AST_Node_Type::Scopeless_Block) {
									auto num_sub_children = child_count(child);
									for (size_t i = 0; i < num_sub_children; ++i) {
										auto& sub_child = child_at(child, i);
										if (sub_child.identifier == AST_Node_Type::Fun_Call) {
											child.children[i] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Unused_Return_Fun_Call_AST_Node>(
												currentScope,
												sub_child.text,
												sub_child.location,
												std::move(sub_child.children)
												));
											result = true;
										}
									}
								}
							}
							return result;
						}
					};

					// If the condition of an If statement is constant and known, then simply skip the check and hard-code the correct path. 
					struct If {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if ((node->identifier == AST_Node_Type::If) && node->children.size() >= 2 && node->children[0]->identifier == AST_Node_Type::Constant) {
								try {
									if (currentScope->Cast<bool>(dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[0].get())->m_value)) {
										// "TRUE" statement is the exclusive path
										node = std::move(node->children[1]);
										return true;
									}
									else if (node->children.size() == 3) {
										// "FALSE" statement is the exclusive path (and a false path is even present)
										node = std::move(node->children[2]);
										return true;
									}
									else {
										// do nothing?
										node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
										return true;
									}
								}
								catch (...) {
									return false;
								}
							}
							return false;
						}
					};

					// Try to fold a basic prefix operation with a constant value
					struct PrefixFold {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Prefix
								&& node->children.size() == 1
								&& node->children[0]->identifier == AST_Node_Type::Constant
								) {
								const Any& rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[0].get())->m_value;
								try {
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, currentScope->CallFunction(node->text, const_cast<Any&>(rhs))
									));
									return true;
								}
								catch (...) {
									// failure to fold -- that's OK. 
									return false;
								}
							}
							return false;
						}
					};

					// postfix's (++/--) on constant values should simply return the same constant value before the change anyways. 
					struct PostfixFold {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Postfix
								&& node->children.size() == 1
								&& node->children[0]->identifier == AST_Node_Type::Constant
								&& ((node->text == "++") || (node->text == "--"))
								) {
								node = std::move(node->children[0]);
								return true;
							}
							return false;
						}
					};

					// Try to fold a basic binary operation between two constant values (e.g. std::string + std::string, or Units::foot == Units::meter)
					struct BinaryFold {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::Binary
								&& node->children.size() == 2
								&& node->children[0]->identifier == AST_Node_Type::Constant
								&& node->children[1]->identifier == AST_Node_Type::Constant
							) {
								const Any& lhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[0].get())->m_value;
								const Any& rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[1].get())->m_value;

								try {
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, currentScope->CallFunction(node->text, { lhs, rhs })
									));
									return true;
								}
								catch (...) {
									// failure to fold -- that's OK
									return false;
								}
							}
							return false;
						}
					};

					// Try to fold a basic binary operation (e.g. +/-/*) with one constant value, to speed-up evaluation in the future
					struct PartialBinaryFold {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							// Fold right side
							if (node->identifier == AST_Node_Type::Binary
								&& node->children.size() == 2
								&& node->children[0]->identifier != AST_Node_Type::Constant
								&& node->children[1]->identifier == AST_Node_Type::Constant
								) {
								try {
									const auto& oper = node->text;
									const auto parsed = Operators::to_operator(oper);
									if (parsed != Operators::Opers::invalid) {
										const auto rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[1].get())->m_value;
										node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Fold_Right_Binary_Operator_AST_Node>(
											currentScope, node->text, node->location, std::move(node->children), rhs
											));
										return true;
									}
								}
								catch (const std::exception&) {
									// failure to fold, that's OK
									return false;
								}
							}

							return false;
						}
					};

					// If an Inline_Array is made-up of const elements, then evaluate and store it as constexpr too.
					struct ConstArray {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							// Fold right side
							if (node->identifier == AST_Node_Type::Inline_Array
								&& node->children.size() == 1
								&& node->children[0]->identifier == AST_Node_Type::Arg_List
								) {
								auto& argList = *node->children.back();

								bool allItemsAreConst = true;
								for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
									if (argList.children[childIndex]->identifier != AST_Node_Type::Constant) {
										allItemsAreConst = false;
										break;
									}
								}

								if (allItemsAreConst) {
									Vector<Var> constArray;
									for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
										Any rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(argList.children[childIndex].get())->m_value;
										rhs.SetFlag(AnyData::Flag::constant, true);
										constArray.push_back(Var(std::move(rhs)));
									}
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, std::move(constArray)
									));
									return true;
								}
							}
							return false;
						}
					};

					// If an Inline_Map is made-up of const elements, then evaluate and store it as constexpr too.
					struct ConstMap {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							// Fold right side
							if (node->identifier == AST_Node_Type::Inline_Map
								&& node->children.size() == 1
								&& node->children[0]->identifier == AST_Node_Type::Arg_List
								) {
								auto& argList = *node->children.back();

								bool allItemsAreConst = true;
								for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
									if (argList.children[childIndex]->identifier == AST_Node_Type::Map_Pair) {
										auto& map_pair = *dynamic_cast<AST_Nodes::Map_Pair_AST_Node*>(argList.children[childIndex].get());
										if (
											(map_pair.children[0]->identifier == AST_Node_Type::Constant) &&
											(map_pair.children[1]->identifier == AST_Node_Type::Constant)
											) {
											continue;
										}
										else {
											allItemsAreConst = false;
											break;
										}
									}
									else {
										allItemsAreConst = false;
										break;
									}
								}

								if (allItemsAreConst) {
									Any constArray{ Map<size_t, std::pair<Var, Var>>() };
									if (!node->children.empty()) {
										for (const auto& child : node->children[0]->children) {
											auto rhs_key = child->children[0]->eval(currentScope);
											auto rhs_val = child->children[1]->eval(currentScope);

											rhs_key.SetFlag(AnyData::Flag::constant, true);
											rhs_val.SetFlag(AnyData::Flag::constant, true);

											currentScope->CallFunction("emplace", {
												constArray,
												rhs_key,
												rhs_val
												});
										}
									}
									constArray.SetFlag(AnyData::Flag::constant, true);

									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, std::move(constArray)
									));
									return true;
								}
							}
							return false;
						}
					};

					// If an Inline_Map is made-up of const elements, then evaluate and store it as constexpr too.
					struct ForLoopSignature {
						bool optimize(AST_Node_Impl_Ptr& node, const std::shared_ptr<Scope>& currentScope) {
							if (node->identifier == AST_Node_Type::For
								&& node->children.size() >= 4
								) {
								// x++ into ++x;
								if (node->children[2]->identifier == AST_Node_Type::Postfix) { // x++
									switch (hash(GetText(node->children[2]))) {
									case hash("++"):
										node->children[2] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Prefix_AST_Node>(
											currentScope, "++", node->children[2]->location, std::move(node->children[2]->children)
										));
										return true;
									case hash("--"):
										node->children[2] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Prefix_AST_Node>(
											currentScope, "--", node->children[2]->location, std::move(node->children[2]->children)
										));
										return true;
									default:
										return false;
									};
								}
							}
							return false;
						};
					};

					using Optimizer_Default = Optimizer<
						optimizer::PostfixFold,
						optimizer::PrefixFold,
						optimizer::BinaryFold,
						optimizer::PartialBinaryFold,
						optimizer::Unused_Fun_Return,
						optimizer::ArgListFileConstant,
						optimizer::VarDeclEquation_To_RetroactiveAssignment,
						optimizer::ToStringFunctionCallWithConstant,
						optimizer::ConstArray,
						optimizer::ConstMap,
						optimizer::If,
						optimizer::Return,
						optimizer::Dead_Code,
						optimizer::ForLoopSignature,
						optimizer::Block
					>;

				public:
					static AST_Node_Impl_Ptr optimize(AST_Node_Impl_Ptr p, const std::shared_ptr<Scope>& currentScope) {
						return Optimizer_Default().optimize(p, currentScope);
					};
				}; // namespace optimizer

				class Parser {
				private:
					constexpr static utility::Static_String m_multiline_comment_end{ "*/" };
					constexpr static utility::Static_String m_multiline_comment_begin{ "/*" };
					constexpr static utility::Static_String m_singleline_comment{ "//" };
					constexpr static utility::Static_String m_annotation{ "#" };
					constexpr static utility::Static_String m_cr_lf{ "\r\n" };
					
					template<typename string_type> struct Char_Parser {
						string_type& match;
						using char_type = typename string_type::value_type;
						bool is_escaped = false;
						bool is_interpolated = false;
						bool saw_interpolation_marker = false;
						bool is_octal = false;
						bool is_hex = false;
						std::size_t unicode_size = 0;
						const bool interpolation_allowed;

						string_type octal_matches;
						string_type hex_matches;

						Char_Parser(string_type& t_match, const bool t_interpolation_allowed)
							: match(t_match)
							, interpolation_allowed(t_interpolation_allowed) {
						}

						Char_Parser& operator=(const Char_Parser&) = delete;

						~Char_Parser() {
							try {
								if (is_octal) {
									process_octal();
								}

								if (is_hex) {
									process_hex();
								}

								if (unicode_size > 0) {
									process_unicode();
								}
							}
							catch (const std::invalid_argument&) {
							}
							catch (const exception::eval_error&) {
								// Something happened with parsing, we'll catch it later?
							}
						}

						void process_hex() {
							if (!hex_matches.empty()) {
								auto val = stoll(hex_matches, nullptr, 16);
								match.push_back(char_type(val));
							}
							hex_matches.clear();
							is_escaped = false;
							is_hex = false;
						}

						void process_octal() {
							if (!octal_matches.empty()) {
								auto val = stoll(octal_matches, nullptr, 8);
								match.push_back(char_type(val));
							}
							octal_matches.clear();
							is_escaped = false;
							is_octal = false;
						}

						void process_unicode() {
							const auto ch = static_cast<uint32_t>(std::stoi(hex_matches, nullptr, 16));
							const auto match_size = hex_matches.size();
							hex_matches.clear();
							is_escaped = false;
							const auto u_size = unicode_size;
							unicode_size = 0;

							char buf[4];
							if (u_size != match_size) {
								throw exception::eval_error("Incomplete unicode escape sequence");
							}
							if (u_size == 4 && ch >= 0xD800 && ch <= 0xDFFF) {
								throw exception::eval_error("Invalid 16 bit universal character");
							}

							if (ch < 0x80) {
								match += static_cast<char>(ch);
							}
							else if (ch < 0x800) {
								buf[0] = static_cast<char>(0xC0 | (ch >> 6));
								buf[1] = static_cast<char>(0x80 | (ch & 0x3F));
								match.append(buf, 2);
							}
							else if (ch < 0x10000) {
								buf[0] = static_cast<char>(0xE0 | (ch >> 12));
								buf[1] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
								buf[2] = static_cast<char>(0x80 | (ch & 0x3F));
								match.append(buf, 3);
							}
							else if (ch < 0x200000) {
								buf[0] = static_cast<char>(0xF0 | (ch >> 18));
								buf[1] = static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
								buf[2] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
								buf[3] = static_cast<char>(0x80 | (ch & 0x3F));
								match.append(buf, 4);
							}
							else {
								// this must be an invalid escape sequence?
								throw exception::eval_error("Invalid 32 bit universal character");
							}
						}

						void parse(const char_type t_char, File_Position pos) {
							const bool is_octal_char = t_char >= '0' && t_char <= '7';

							const bool is_hex_char = (t_char >= '0' && t_char <= '9') || (t_char >= 'a' && t_char <= 'f') || (t_char >= 'A' && t_char <= 'F');

							if (is_octal) {
								if (is_octal_char) {
									octal_matches.push_back(t_char);

									if (octal_matches.size() == 3) {
										process_octal();
									}
									return;
								}
								else {
									process_octal();
								}
							}
							else if (is_hex) {
								if (is_hex_char) {
									hex_matches.push_back(t_char);

									if (hex_matches.size() == 2 * sizeof(char_type)) {
										// This rule differs from the C/C++ standard, but ChaiScript
										// does not offer the same workaround options, and having
										// hexadecimal sequences longer than can fit into the char
										// type is undefined behavior anyway.
										process_hex();
									}
									return;
								}
								else {
									process_hex();
								}
							}
							else if (unicode_size > 0) {
								if (is_hex_char) {
									hex_matches.push_back(t_char);

									if (hex_matches.size() == unicode_size) {
										// Format is specified to be 'slash'uABCD
										// on collecting from A to D do parsing
										process_unicode();
									}
									return;
								}
								else {
									// Not a unicode anymore, try parsing any way
									// May be someone used 'slash'uAA only
									process_unicode();
								}
							}

							if (t_char == '\\') {
								if (is_escaped) {
									match.push_back('\\');
									is_escaped = false;
								}
								else {
									is_escaped = true;
								}
							}
							else {
								if (is_escaped) {
									if (is_octal_char) {
										is_octal = true;
										octal_matches.push_back(t_char);
									}
									else if (t_char == 'x') {
										is_hex = true;
									}
									else if (t_char == 'u') {
										unicode_size = 4;
									}
									else if (t_char == 'U') {
										unicode_size = 8;
									}
									else {
										switch (t_char) {
										case ('\''):
											match.push_back('\'');
											break;
										case ('\"'):
											match.push_back('\"');
											break;
										case ('?'):
											match.push_back('?');
											break;
										case ('a'):
											match.push_back('\a');
											break;
										case ('b'):
											match.push_back('\b');
											break;
										case ('f'):
											match.push_back('\f');
											break;
										case ('n'):
											match.push_back('\n');
											break;
										case ('r'):
											match.push_back('\r');
											break;
										case ('t'):
											match.push_back('\t');
											break;
										case ('v'):
											match.push_back('\v');
											break;
										case ('$'):
											match.push_back('$');
											break;
										default:
											throw exception::eval_error("Unknown escaped sequence in string", pos);
										}
										is_escaped = false;
									}
								}
								else if (interpolation_allowed && t_char == '$') {
									saw_interpolation_marker = true;
								}
								else {
									match.push_back(t_char);
								}
							}
						}
					};
					static std::map<std::string_view, Any> build_constants() {
						std::map<std::string_view, Any> out;
						out["true"] = const_var(true);
						out["false"] = const_var(false);
						out["Infinity"] = const_var(std::numeric_limits<double>::infinity());
						out["NaN"] = const_var(std::numeric_limits<double>::quiet_NaN());
						out["nullptr"] = const_var(Any());
						out["null"] = const_var(Any());
						return out;
					};
					static std::map<std::string_view, Any> const& constants() {
						static auto out{ build_constants() };
						return out;
					};


					template<typename Array2D, typename First, typename Second>
					static void set_alphabet(Array2D& array, const First first, const Second second) noexcept {
						auto* first_ptr = &std::get<0>(array) + static_cast<std::size_t>(first);
						auto* second_ptr = &std::get<0>(*first_ptr) + static_cast<std::size_t>(second);
						*second_ptr = true;
					};
					static std::array<std::array<bool, lengthof_alphabet>, max_alphabet> build_alphabet() noexcept {
						std::array<std::array<bool, lengthof_alphabet>, max_alphabet> alphabet{};

						set_alphabet(alphabet, symbol_alphabet, '?');

						set_alphabet(alphabet, symbol_alphabet, '?');
						set_alphabet(alphabet, symbol_alphabet, '+');
						set_alphabet(alphabet, symbol_alphabet, '-');
						set_alphabet(alphabet, symbol_alphabet, '*');
						set_alphabet(alphabet, symbol_alphabet, '/');
						set_alphabet(alphabet, symbol_alphabet, '|');
						set_alphabet(alphabet, symbol_alphabet, '&');
						set_alphabet(alphabet, symbol_alphabet, '^');
						set_alphabet(alphabet, symbol_alphabet, '=');
						set_alphabet(alphabet, symbol_alphabet, '.');
						set_alphabet(alphabet, symbol_alphabet, '<');
						set_alphabet(alphabet, symbol_alphabet, '>');

						for (size_t c = 'a'; c <= 'z'; ++c) {
							set_alphabet(alphabet, keyword_alphabet, c);
						}
						for (size_t c = 'A'; c <= 'Z'; ++c) {
							set_alphabet(alphabet, keyword_alphabet, c);
						}
						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, keyword_alphabet, c);
						}
						set_alphabet(alphabet, keyword_alphabet, '_');
						// set_alphabet(alphabet, keyword_alphabet, ':');

						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, int_alphabet, c);
						}
						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, float_alphabet, c);
						}
						set_alphabet(alphabet, float_alphabet, '.');

						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, hex_alphabet, c);
						}
						for (size_t c = 'a'; c <= 'f'; ++c) {
							set_alphabet(alphabet, hex_alphabet, c);
						}
						for (size_t c = 'A'; c <= 'F'; ++c) {
							set_alphabet(alphabet, hex_alphabet, c);
						}

						set_alphabet(alphabet, x_alphabet, 'x');
						set_alphabet(alphabet, x_alphabet, 'X');

						for (size_t c = '0'; c <= '1'; ++c) {
							set_alphabet(alphabet, bin_alphabet, c);
						}
						set_alphabet(alphabet, b_alphabet, 'b');
						set_alphabet(alphabet, b_alphabet, 'B');

						for (size_t c = 'a'; c <= 'z'; ++c) {
							set_alphabet(alphabet, id_alphabet, c);
						}
						for (size_t c = 'A'; c <= 'Z'; ++c) {
							set_alphabet(alphabet, id_alphabet, c);
						}
						set_alphabet(alphabet, id_alphabet, '_');
						set_alphabet(alphabet, id_alphabet, ':'); // RG
						for (size_t c = '0'; c <= '9'; ++c) { set_alphabet(alphabet, id_alphabet, c); } // RG

						set_alphabet(alphabet, white_alphabet, ' ');
						set_alphabet(alphabet, white_alphabet, '\t');

						set_alphabet(alphabet, int_suffix_alphabet, 'l');
						set_alphabet(alphabet, int_suffix_alphabet, 'L');
						set_alphabet(alphabet, int_suffix_alphabet, 'u');
						set_alphabet(alphabet, int_suffix_alphabet, 'U');

						set_alphabet(alphabet, float_suffix_alphabet, 'l');
						set_alphabet(alphabet, float_suffix_alphabet, 'L');
						set_alphabet(alphabet, float_suffix_alphabet, 'f');
						set_alphabet(alphabet, float_suffix_alphabet, 'F');

						return alphabet;
					}
					static std::array<std::array<bool, lengthof_alphabet>, max_alphabet> const& alphabet() {
						static auto out{ build_alphabet() };
						return out;
					};

					struct Operator_Matches {
						using SS = utility::Static_String;

						struct Operator_Matches_Impl {
							using SS = utility::Static_String;
							// should match the order and categories from create_operators()
							const std::array<utility::Static_String, 2> m_0{ {SS("?"), SS("?=")} };
							const std::array<utility::Static_String, 1> m_1{ {SS("||")} };
							const std::array<utility::Static_String, 1> m_2{ {SS("&&")} };
							const std::array<utility::Static_String, 1> m_3{ {SS("|")} };
							const std::array<utility::Static_String, 1> m_4{ {SS("&")} };
							const std::array<utility::Static_String, 3> m_5{ {SS("=="), SS("!="), SS("..")} };
							const std::array<utility::Static_String, 4> m_6{ {SS("<"), SS("<="), SS(">"), SS(">=")} };
							const std::array<utility::Static_String, 2> m_7{ {SS("<<"), SS(">>")} };
							const std::array<utility::Static_String, 2> m_8{ {SS("+"), SS("-")} };
							const std::array<utility::Static_String, 3> m_9{ {SS("*"), SS("/"), SS("%")} };
							const std::array<utility::Static_String, 1> m_10{ {SS("^")} };
							const std::array<utility::Static_String, 6> m_11{ SS("++"), SS("--"), SS("-"), SS("+"), SS("!"), SS("~") };
						};
						static auto const& Data() {
							static Operator_Matches_Impl out;
							return out;
						};
						static bool is_match(std::string_view t_str) noexcept {
							constexpr std::array<std::size_t, 12> groups{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } };
							return std::any_of(groups.begin(), groups.end(), [&t_str](const std::size_t group) { return is_match(group, t_str); });
						};
						template<typename Predicate> static bool any_of(const std::size_t t_group, Predicate&& predicate) {
							auto match = [&predicate](const auto& array) { return std::any_of(array.begin(), array.end(), predicate); };

							switch (t_group) {
							case 0:
								return match(Data().m_0);
							case 1:
								return match(Data().m_1);
							case 2:
								return match(Data().m_2);
							case 3:
								return match(Data().m_3);
							case 4:
								return match(Data().m_4);
							case 5:
								return match(Data().m_5);
							case 6:
								return match(Data().m_6);
							case 7:
								return match(Data().m_7);
							case 8:
								return match(Data().m_8);
							case 9:
								return match(Data().m_9);
							case 10:
								return match(Data().m_10);
							case 11:
								return match(Data().m_11);
							default:
								return false;
							}
						}
						static bool is_match(const std::size_t t_group, std::string_view t_str)  noexcept {
							auto match = [&t_str](const auto& array) {
								return std::any_of(array.begin(), array.end(), [&t_str](const auto& v) { return v == t_str; });
							};

							switch (t_group) {
							case 0:
								return match(Data().m_0);
							case 1:
								return match(Data().m_1);
							case 2:
								return match(Data().m_2);
							case 3:
								return match(Data().m_3);
							case 4:
								return match(Data().m_4);
							case 5:
								return match(Data().m_5);
							case 6:
								return match(Data().m_6);
							case 7:
								return match(Data().m_7);
							case 8:
								return match(Data().m_8);
							case 9:
								return match(Data().m_9);
							case 10:
								return match(Data().m_10);
							case 11:
								return match(Data().m_11);
							default:
								return false;
							}
						}
					};
					static std::array<Operator_Precedence, 12> build_operators() noexcept {
						return std::array<Operator_Precedence, 12>{
							{
								Operator_Precedence::Ternary_Cond,
									Operator_Precedence::Logical_Or,
									Operator_Precedence::Logical_And,
									Operator_Precedence::Bitwise_Or,
									Operator_Precedence::Bitwise_And,
									Operator_Precedence::Equality,
									Operator_Precedence::Comparison,
									Operator_Precedence::Shift,
									Operator_Precedence::Addition,
									Operator_Precedence::Multiplication,
									Operator_Precedence::Bitwise_Xor,
									Operator_Precedence::Prefix
							}
						};
					};
					static std::array<Operator_Precedence, 12> const& operators() noexcept {
						static auto out{ build_operators() };
						return out;
					};

					constexpr bool char_in_alphabet(char c, Alphabet a) const noexcept { return alphabet()[a][static_cast<uint8_t>(c)]; } // test a char in an m_alphabet
					
				public:
					using ParseNode = std::pair< std::shared_ptr<AST_Node_Impl>, std::shared_ptr<Scope> >;
				private:
					Position m_position{};					
					std::vector<ParseNode> m_match_stack;
					std::vector<ParseNode> m_comment_stack;

				private:
					// check if the string is a valid operator
					static bool is_operator(std::string_view t_s) noexcept { return Operator_Matches::is_match(t_s); }
					static void validate_object_name(std::string_view const& name, Position const& m_position) {
						switch (hash(name)) {
						case hash(""):
							throw exception::eval_error("Id names cannot be empty", (File_Position)m_position);
						case hash("#define"):
						case hash("#undef"):
						case hash("#ifdef"):
						case hash("#ifndef"):
						case hash("#elif"):
						case hash("#else"):
						case hash("#endif"):
						case hash("#error"):
						case hash("#warning"):
						case hash("#include"):
						case hash("#pragma"):
						case hash("auto"):
						case hash("var"):
						case hash("global"):
						case hash("while"):
						case hash("for"):
						case hash("parallel_for"):
						case hash("break"):
						case hash("conitnue"):
						case hash("case"):
						case hash("default"):
						case hash("switch"):
						case hash("try"):
						case hash("catch"):
						case hash("finally"):
						case hash("do"):
						case hash("evaluate"):
						case hash("namespace"):
						case hash("return"):
						case hash("if"):
						case hash("else"):
						{
							std::string temp = std::string(name);
							throw exception::eval_error(GoodLang::printf("Id name '%s' was reserved for the langauge", temp.c_str()), (File_Position)m_position);
						}
						default:
							return;
						}
					};
					
				private:
					/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
					bool Symbol_(const utility::Static_String& sym) noexcept {
						const auto len = sym.size();
						if (m_position.remaining() >= len) {
							const char* file_pos = &(*m_position);
							for (size_t pos = 0; pos < len; ++pos) {
								if (sym.c_str()[pos] != file_pos[pos]) {
									return false;
								}
							}
							m_position += len;
							return true;
						}
						return false;
					};
					/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
					bool Symbol_(const std::string_view& sym) noexcept {
						const auto len = sym.size();
						if (m_position.remaining() >= len) {
							const char* file_pos = &(*m_position);
							for (size_t pos = 0; pos < len; ++pos) {
								if (sym[pos] != file_pos[pos]) {
									return false;
								}
							}
							m_position += len;
							return true;
						}
						return false;
					};
					/// Reads a char from input if it matches the parameter, without skipping initial whitespace
					bool Char_(const char c) {
						if (m_position.has_more() && (*m_position == c)) {
							++m_position;
							return true;
						}
						else {
							return false;
						}
					};
					/// Reads an end-of-line group from input, without skipping initial whitespace
					bool Eol_(const bool t_eos = false) {
						bool retval = false;

						if (m_position.has_more() && (Symbol_(m_cr_lf) || Char_('\n'))) {
							retval = true;
							//++m_position.line;
							m_position.col = 1;
						}
						else if (m_position.has_more() && !t_eos && Char_(';')) {
							retval = true;
						}

						return retval;
					};
					/// Reads a string from input if it matches the parameter, without skipping initial whitespace
					bool Keyword_(const utility::Static_String& t_s) {
						const auto len = t_s.size();
						if (m_position.remaining() >= len) {
							auto tmp = m_position;
							for (size_t i = 0; tmp.has_more() && i < len; ++i) {
								if (*tmp != t_s.c_str()[i]) {
									return false;
								}
								++tmp;
							}
							m_position = tmp;
							return true;
						}

						return false;
					};
					/// Reads the optional exponent (scientific notation) and suffix for a Float, without skipping initial whitespace
					/// Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
					bool read_exponent_and_suffix_() noexcept {
						// Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
						if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
							++m_position;
							if (m_position.has_more() && ((*m_position == '-') || (*m_position == '+'))) {
								++m_position;
							}
							auto exponent_pos = m_position;
							while (m_position.has_more() && char_in_alphabet(*m_position, int_alphabet)) {
								++m_position;
							}
							if (m_position == exponent_pos) {
								// Require at least one digit after the exponent
								return false;
							}
						}

						// Parse optional float suffix
						while (m_position.has_more() && char_in_alphabet(*m_position, float_suffix_alphabet)) {
							++m_position;
						}

						return true;
					};
					/// Reads a floating point value from input, without skipping initial whitespace
					bool Float_() noexcept {
						if (m_position.has_more() && char_in_alphabet(*m_position, float_alphabet)) {
							while (m_position.has_more() && char_in_alphabet(*m_position, int_alphabet)) {
								++m_position;
							}

							if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
								// The exponent is valid even without any decimal in the Float (1e8, 3e-15)
								return read_exponent_and_suffix_();
							}
							else if (m_position.has_more() && (*m_position == '.')) {
								++m_position;
								if (m_position.has_more() && char_in_alphabet(*m_position, int_alphabet)) {
									while (m_position.has_more() && char_in_alphabet(*m_position, int_alphabet)) {
										++m_position;
									}
									// After any decimal digits, support an optional exponent (3.7e3)
									return read_exponent_and_suffix_();
								}
								else {
									--m_position;
								}
							}
						}
						return false;
					};
					/// Reads a hex value from input, without skipping initial whitespace
					bool Hex_() noexcept {
						if (m_position.has_more() && (*m_position == '0')) {
							++m_position;

							if (m_position.has_more() && char_in_alphabet(*m_position, x_alphabet)) {
								++m_position;
								if (m_position.has_more() && char_in_alphabet(*m_position, hex_alphabet)) {
									while (m_position.has_more() && char_in_alphabet(*m_position, hex_alphabet)) {
										++m_position;
									}
									while (m_position.has_more() && char_in_alphabet(*m_position, int_suffix_alphabet)) {
										++m_position;
									}

									return true;
								}
								else {
									--m_position;
								}
							}
							else {
								--m_position;
							}
						}

						return false;
					}
					/// Reads an integer suffix, without skipping initial whitespace
					void IntSuffix_() {
						while (m_position.has_more() && char_in_alphabet(*m_position, int_suffix_alphabet)) {
							++m_position;
						}
					}
					/// Reads a binary value from input, without skipping initial whitespace
					bool Binary_() {
						if (m_position.has_more() && (*m_position == '0')) {
							++m_position;

							if (m_position.has_more() && char_in_alphabet(*m_position, b_alphabet)) {
								++m_position;
								if (m_position.has_more() && char_in_alphabet(*m_position, bin_alphabet)) {
									while (m_position.has_more() && char_in_alphabet(*m_position, bin_alphabet)) {
										++m_position;
									}
									return true;
								}
								else {
									--m_position;
								}
							}
							else {
								--m_position;
							}
						}

						return false;
					};
					template<typename T> constexpr static auto parse_num_(const std::string_view t_str) noexcept -> typename std::enable_if<std::is_integral<T>::value, T>::type {
						T t = 0;
						for (const auto c : t_str) {
							if (c < '0' || c > '9') {
								return t;
							}
							t *= 10;
							t += c - '0';
						}
						return t;
					};
					template<typename T> static auto parse_num_(const std::string_view t_str) -> typename std::enable_if<!std::is_integral<T>::value, T>::type {
						T t = 0;
						T base{};
						T decimal_place = 0;
						int exponent = 0;

						for (const auto c : t_str) {
							switch (c) {
							case '.':
								decimal_place = 10;
								break;
							case 'e':
							case 'E':
								exponent = 1;
								decimal_place = 0;
								base = t;
								t = 0;
								break;
							case '-':
								exponent = -1;
								break;
							case '+':
								break;
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
							case '9':
								if (decimal_place < 10) {
									t *= 10;
									t += static_cast<T>(c - '0');
								}
								else {
									t += static_cast<T>(c - '0') / decimal_place;
									decimal_place *= 10;
								}
								break;
							default:
								break;
							}
						}
						return exponent ? base * std::pow(T(10), t * static_cast<T>(exponent)) : t;
					};
					/// Parses a floating point value
					static Units::value buildFloat(std::string_view t_val) {
						bool float_ = false;
						bool long_ = false;

						auto i = t_val.size();

						for (; i > 0; --i) {
							char val = t_val[i - 1];

							if (val == 'f' || val == 'F') {
								float_ = true;
							}
							else if (val == 'l' || val == 'L') {
								long_ = true;
							}
							else {
								break;
							}
						}

						if (float_) {
							return Units::value(parse_num_<float>(t_val.substr(0, i)));
						}
						else if (long_) {
							return Units::value(parse_num_<long double>(t_val.substr(0, i)));
						}
						else {
							return Units::value(parse_num_<double>(t_val.substr(0, i)));
						}
					}
					/// Parses a integer value and returns a wrapped representation of it
					static Units::value buildInt(const int base, std::string_view t_val, const bool prefixed) {
						bool unsigned_ = false;
						bool long_ = false;
						bool longlong_ = false;

						auto i = t_val.size();

						for (; i > 0; --i) {
							const char val = t_val[i - 1];

							if (val == 'u' || val == 'U') {
								unsigned_ = true;
							}
							else if (val == 'l' || val == 'L') {
								if (long_) {
									longlong_ = true;
								}

								long_ = true;
							}
							else {
								break;
							}
						}

						if (prefixed) {
							t_val.remove_prefix(2);
						}

						try {
							/// TODO fix this to use from_chars
							auto u = std::stoll(std::string(t_val), nullptr, base);

							if (!unsigned_ && !long_ && u >= std::numeric_limits<int>::min() && u <= std::numeric_limits<int>::max()) {
								return static_cast<int>(u);
							}
							else if ((unsigned_ || base != 10) && !long_ && u >= std::numeric_limits<unsigned int>::min()
								&& u <= std::numeric_limits<unsigned int>::max()) {
								return static_cast<unsigned int>(u);
							}
							else if (!unsigned_ && !longlong_ && u >= std::numeric_limits<long>::min() && u <= std::numeric_limits<long>::max()) {
								return static_cast<long>(u);
							}
							else if ((unsigned_ || base != 10) && !longlong_ && u >= std::numeric_limits<unsigned long>::min()
								&& u <= std::numeric_limits<unsigned long>::max()) {
								return static_cast<unsigned long>(u);
							}
							else if (!unsigned_ && u >= std::numeric_limits<long long>::min() && u <= std::numeric_limits<long long>::max()) {
								return static_cast<long long>(u);
							}
							else {
								return static_cast<unsigned long long>(u);
							}
						}
						catch (const std::out_of_range&) {
							// too big to be signed
							try {
								/// TODO fix this to use from_chars
								auto u = std::stoull(std::string(t_val), nullptr, base);

								if (!longlong_ && u >= std::numeric_limits<unsigned long>::min() && u <= std::numeric_limits<unsigned long>::max()) {
									return static_cast<unsigned long>(u);
								}
								else {
									return static_cast<unsigned long long>(u);
								}
							}
							catch (const std::out_of_range&) {
								// it's just simply too big
								return std::numeric_limits<long long>::max();
							}
						}
					}
					/// Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
					bool Id_(std::string_view* out = nullptr) {
						const auto start = m_position;
						if (m_position.has_more() && char_in_alphabet(*m_position, id_alphabet)) {
							while (m_position.has_more() && char_in_alphabet(*m_position, id_alphabet)) { //keyword_alphabet)) {
								++m_position;
							}
							if (out) *out = Position::str(start, m_position);
							return true;
						}
						else if (m_position.has_more() && (*m_position == '`')) {
							++m_position;
							const auto start = m_position;

							while (m_position.has_more() && (*m_position != '`')) {
								if (Eol()) {
									throw exception::eval_error("Carriage return in identifier literal", (File_Position)m_position);
								}
								else {
									++m_position;
								}
							}

							if (start == m_position) {
								throw exception::eval_error("Missing contents of identifier literal", (File_Position)m_position);
							}
							else if (!m_position.has_more()) {
								throw exception::eval_error("Incomplete identifier literal", (File_Position)m_position);
							}

							++m_position;
							if (out) *out = Position::str(start, m_position);
							return true;
						}
						return false;
					};
					/// Reads a quoted string from input, without skipping initial whitespace
					bool Quoted_String_() {
						if (m_position.has_more() && (*m_position == '\"')) {
							char prev_char = *m_position;
							++m_position;

							int in_interpolation = 0;
							bool in_quote = false;

							while (m_position.has_more() && ((*m_position != '\"') || (in_interpolation > 0) || (prev_char == '\\'))) {
								if (!Eol_()) {
									if (prev_char == '$' && *m_position == '{') {
										++in_interpolation;
									}
									else if (prev_char != '\\' && *m_position == '"') {
										in_quote = !in_quote;
									}
									else if (*m_position == '}' && !in_quote) {
										--in_interpolation;
									}

									if (prev_char == '\\') {
										prev_char = 0;
									}
									else {
										prev_char = *m_position;
									}
									++m_position;
								}
							}

							if (m_position.has_more()) {
								++m_position;
							}
							else {
								throw exception::eval_error("Unclosed quoted string", (File_Position)m_position);
							}

							return true;
						}
						return false;
					};
					/// Reads (and potentially captures) a number from the input, detecting if it's an integer or floating point, without skipping initial whitespace
					bool Num_() {
						const auto start = m_position;
						if (m_position.has_more() && char_in_alphabet(*m_position, float_alphabet)) {
							try {
								if (Hex_()) {
									auto match = Position::str(start, m_position);
									auto bv = buildInt(16, match, true);
									m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									return true;
								}
								else if (Binary_()) {
									auto match = Position::str(start, m_position);
									auto bv = buildInt(2, match, true);
									m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									return true;
								}
								else if (Float_()) {
									auto match = Position::str(start, m_position);
									auto bv = buildFloat(match);
									m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									return true;
								}
								else {
									IntSuffix_();
									auto match = Position::str(start, m_position);
									if (!match.empty() && (match[0] == '0')) {
										auto bv = buildInt(8, match, false);
										m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									}
									else if (!match.empty()) {
										auto bv = buildInt(10, match, false);
										m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									}
									else {
										return false;
									}
									return true;
								}
							}
							catch (const std::invalid_argument&) {
								// error parsing number passed in to buildFloat/buildInt
								return false;
							}
						}
						else {
							return false;
						}
					};

				private:
					/// Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
					template<typename NodeType> std::shared_ptr<NodeType> build_match(const std::shared_ptr<Scope>& currentScope, size_t t_match_start, std::string t_text = "") {
						bool is_deep = false;

						Parse_Location filepos = [&]() -> Parse_Location {
							// so we want to take everything to the right of this and make them children
							if (t_match_start != m_match_stack.size()) {
								is_deep = true;
								return Parse_Location(
									m_match_stack[t_match_start].first->location.start,
									m_position);
							}
							else {
								return Parse_Location(m_position, m_position);
							}
						}();

						std::vector<ParseNode> new_children_ParseNodes;
						std::vector<AST_Node_Impl_Ptr> new_children;
						if (is_deep) {
							new_children_ParseNodes.assign(std::make_move_iterator(m_match_stack.begin() + static_cast<int>(t_match_start)),
								std::make_move_iterator(m_match_stack.end()));
							m_match_stack.erase(m_match_stack.begin() + static_cast<int>(t_match_start), m_match_stack.end());
						}
						for (auto& x : new_children_ParseNodes) {
							new_children.push_back(std::dynamic_pointer_cast<AST_Node_Impl>(x.first));
						}

						auto ptr = optimizer::optimize(std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<NodeType>(
							currentScope
							, std::move(t_text)
							, std::move(filepos)
							, std::move(new_children)
							)), currentScope);
						m_match_stack.push_back(ParseNode{ ptr, currentScope });
						return std::dynamic_pointer_cast<NodeType>(ptr);
					};
					/// create a node
					template<typename T, typename... Param> std::shared_ptr<AST_Node_Impl> make_node(const std::shared_ptr<Scope>& currentScope, std::string_view t_match, Position t_prev, Param &&...param) {
						auto out = std::make_shared<T>(
							currentScope,
							std::string(t_match),
							Parse_Location(t_prev, m_position),
							std::forward<Param>(param)...
							);
						return std::dynamic_pointer_cast<AST_Node_Impl>(out);
					};
					/// create a node
					template<typename... Param> std::shared_ptr<AST_Nodes::Constant_AST_Node> make_const(std::string_view t_match, Position t_prev, Param &&...param) {
						return std::make_shared<AST_Nodes::Constant_AST_Node>(
							std::string(t_match),
							Parse_Location(t_prev, m_position),
							std::forward<Param>(param)...
						);
					};

				private:
					/// Skips (and potentially captures w/ nullptr scope) any multi-line or single-line comment
					bool SkipComment() {
						const auto start = m_position;
						if (Symbol_(m_multiline_comment_begin)) {
							while (m_position.has_more()) {
								if (Symbol_(m_multiline_comment_end)) {
									break;
								}
								else if (!Eol_()) {
									++m_position;
								}
							}
							std::string_view comment = Position::str(start, m_position);
							auto parseLoc = Parse_Location(start, m_position);
							m_comment_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(nullptr, std::string(comment), parseLoc), nullptr });

							return true;
						}
						else if (Symbol_(m_singleline_comment)) {
							while (m_position.has_more()) {
								if (Symbol_(m_cr_lf)) {
									m_position -= 2;
									break;
								}
								else if (Char_('\n')) {
									--m_position;
									break;
								}
								else {
									++m_position;
								}
							}

							std::string_view comment = Position::str(start, m_position);
							auto parseLoc = Parse_Location(start, m_position);
							m_comment_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(nullptr, std::string(comment), parseLoc), nullptr });

							return true;

						}
						else if (Symbol_(m_annotation)) {
							while (m_position.has_more()) {
								if (Symbol_(m_cr_lf)) {
									m_position -= 2;
									break;
								}
								else if (Char_('\n')) {
									--m_position;
									break;
								}
								else {
									++m_position;
								}
							}
							std::string_view comment = Position::str(start, m_position);
							auto parseLoc = Parse_Location(start, m_position);
							m_comment_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(nullptr, std::string(comment), parseLoc), nullptr });

							return true;
						}
						return false;
					}
					/// Skips whitespace, which means space and tab, but not cr/lf
					/// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
					/// AlekMosingiewicz: Added exception when illegal character detected
					bool SkipWS(bool skip_cr = false) {
						bool retval = false;

						while (m_position.has_more()) {
							if (static_cast<unsigned char>(*m_position) > 0x7e) {
								throw exception::eval_error("Illegal character", (File_Position)m_position);
							}
							auto end_line = (*m_position != 0) && ((*m_position == '\n') || (*m_position == '\r' && *(m_position + 1) == '\n'));

							if (char_in_alphabet(*m_position, white_alphabet) || (skip_cr && end_line)) {
								if (end_line) {
									if (*m_position == '\r') {
										// discards lf
										++m_position;
									}
								}

								++m_position;

								retval = true;
							}
							else if (SkipComment()) {
								retval = true;
							}
							else {
								break;
							}
						}
						return retval;
					};
					/// Reads until the end of the current statement
					bool Eos() {
						SkipWS();
						return Eol_(true);
					};
					/// Reads (and potentially captures) an end-of-line group from input
					bool Eol() {
						SkipWS();
						return Eol_();
					};
					/// Reads (and potentially captures) a char from input if it matches the parameter
					bool Char(const char t_c) {
						SkipWS();
						return Char_(t_c);
					};
					/// Reads (and potentially captures) a string from input if it matches the parameter
					bool Keyword(const utility::Static_String& t_s) {
						SkipWS();
						const auto start = m_position;
						bool retval = Keyword_(t_s);
						// ignore substring matches
						if (retval && m_position.has_more() && char_in_alphabet(*m_position, keyword_alphabet)) {
							m_position = start;
							retval = false;
						}
						return retval;
					};
					/// Reads (and potentially captures) a symbol group from input if it matches the parameter
					bool Symbol(const std::string_view& t_s, const bool t_disallow_prevention = false) {
						SkipWS();
						const auto start = m_position;
						bool retval = Symbol_(t_s);

						// ignore substring matches
						if (retval && m_position.has_more() && (t_disallow_prevention == false) && char_in_alphabet(*m_position, symbol_alphabet)) {
							if (*m_position != '=' && is_operator(Position::str(start, m_position)) && !is_operator(Position::str(start, m_position + 1))) {
								// don't throw this away, it's a good match and the next is not
							}
							else {
								m_position = start;
								retval = false;
							}
						}
						return retval;
					}
					/// Reads (and potentially captures) a number from the input, detecting if it's an integer or floating point
					bool Num() {
						SkipWS();
						return Num_();
					};
					/// 
					bool Operator_Helper(const size_t t_precedence, std::string& oper) {
						return Operator_Matches::any_of(t_precedence, [&oper, this](const auto& elem) {
							if (Symbol(elem.c_str())) {
								oper = elem.c_str();
								return true;
							}
							else {
								return false;
							}
						});
					};
					/// Reads (and potentially captures) a quoted string from input.  Translates escaped sequences.
					bool Quoted_String(const std::shared_ptr<Scope>& currentScope) {
						SkipWS();

						const auto start = m_position;

						if (Quoted_String_()) {
							std::string match;
							const auto prev_stack_top = m_match_stack.size();

							bool is_interpolated = [&]() -> bool {
								Char_Parser<std::string> cparser(match, true);

								auto s = start + 1, end = m_position - 1;

								while (s != end) {
									if (cparser.saw_interpolation_marker) {
										if (*s == '{') {
											// We've found an interpolation point

											m_match_stack.push_back({ make_const(match, start, match), nullptr });

											if (cparser.is_interpolated) {
												// If we've seen previous interpolation, add on instead of making a new one
												build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, "+");
											}

											// We've finished with the part of the string up to this point, so clear it
											match.clear();

											std::string eval_match;

											++s;
											while ((s != end) && (*s != '}')) {
												eval_match.push_back(*s);
												++s;
											}

											if (*s == '}') {
												cparser.is_interpolated = true;
												++s;

												const auto tostr_stack_top = m_match_stack.size();

												m_match_stack.push_back({ make_node<AST_Nodes::FunctionName_AST_Node>(currentScope, "to_string", start), currentScope }); //  Id_AST_Node

												const auto ev_stack_top = m_match_stack.size();

												try {
													m_match_stack.push_back(parse_instr_eval(eval_match, currentScope));
												}
												catch (const exception::eval_error& e) {
													throw exception::eval_error(e.what(), (File_Position)start);
												}

												build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, ev_stack_top);
												build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, tostr_stack_top);
												build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, "+");
											}
											else {
												throw exception::eval_error("Unclosed in-string eval", (File_Position)start);
											}
										}
										else {
											match.push_back('$');
										}
										cparser.saw_interpolation_marker = false;
									}
									else {
										cparser.parse(*s, start);

										++s;
									}
								}

								if (cparser.saw_interpolation_marker) {
									match.push_back('$');
								}

								return cparser.is_interpolated;
							}();

							m_match_stack.push_back({ make_const(match, start, match), nullptr });

							if (is_interpolated) {
								build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, "+");
							}

							return true;
						}
						else {
							return false;
						}
					};
					/// Reads (and potentially captures) an identifier from input
					bool Id(const bool validate, const std::shared_ptr<Scope>& currentScope, AST_Nodes::IdType T) {
						SkipWS();
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						if (T == AST_Nodes::IdType::Class) {
							std::string_view className;

							auto Success = [&](bool Const, bool Ref) -> bool {
								if (className == "void") {
									auto ptr = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(make_node<AST_Nodes::ClassName_AST_Node>(currentScope, className, prev_pos));
									ptr->TypeInfo = user_type_shared<void>();
									m_match_stack.push_back({ std::dynamic_pointer_cast<AST_Node_Impl>(ptr), currentScope }); // e.g. "x", "Units::meter", etc.
									return true;
								}
								else if (auto Class = currentScope->FindClass(std::string(className))) {
									auto ptr = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(make_node<AST_Nodes::ClassName_AST_Node>(currentScope, className, prev_pos));
									if (Const && Ref) {
										ptr->TypeInfo = Class->GetClassType().lock()->MakeConstRef();
									}
									else if (Const && !Ref) {
										ptr->TypeInfo = Class->GetClassType().lock()->MakeConst();
									}
									else if(!Const && Ref) {
										ptr->TypeInfo = Class->GetClassType().lock()->MakeRef();
									}
									else {
										ptr->TypeInfo = Class->GetClassType();
									}
									m_match_stack.push_back({ std::dynamic_pointer_cast<AST_Node_Impl>(ptr), currentScope }); // e.g. "x", "Units::meter", etc.
									return true;
								}
								else {
									return failure();
								}
							};

							// valid arrangements: 
							//     typename
							//     const typename
							//     typename&
							//     const typename&
							//     typename const
							//     typename const&
							//     const& typename
							if (Keyword("const")) {
								if (Char('&')) {
									SkipWS();
									if (Id_(&className)) {
										// const& typename
										return Success(true, true);
									}
								}
								else if (Id_(&className)) {
									const auto prev_pos_temp = m_position;
									if (Char('&')) {
										// const typename&
										return Success(true, true);
									}
									else {
										m_position = prev_pos_temp;
										// const typename
										return Success(true, false);
									}
								}
							}
							else if (Id_(&className)) {
								auto prev_pos_temp = m_position;
								//     typename
								//     typename&
								//     typename const
								//     typename const&
								if (Keyword("const")) {
									prev_pos_temp = m_position;
									if (Char('&')) {
										//     typename const&
										return Success(true, true);
									}
									else {										
										m_position = prev_pos_temp;
										//     typename const
										return Success(true, false);
									}
								}
								else if (Char('&')) {
									//     typename&
									return Success(false, true);
								}
								else {									
									m_position = prev_pos_temp;
									//     typename
									return Success(false, false);
								}
							}
							return failure();

						}
						else {
							if (Id_()) {
								auto text = Position::str(prev_pos, m_position);
								if (validate) { validate_object_name(text, m_position); }

								auto foundConstant = constants().find(text);
								if (foundConstant != constants().end()) {
									if (AST_Nodes::IdType::Class == T) throw exception::eval_error(GoodLang::printf("Cannot use constant value \"%s\" as a class or type name", text.data()), m_position);
									m_match_stack.push_back({ make_const(text, prev_pos, foundConstant->second), nullptr });
								}
								else {
									switch (hash(text)) {
									case hash("__LINE__"): {
										if (AST_Nodes::IdType::Class == T) throw exception::eval_error(GoodLang::printf("Cannot use constant value \"%s\" as a class or type name", text.data()), m_position);
										m_match_stack.push_back({ make_const(text, prev_pos, const_var(prev_pos.line)), nullptr });
									} break;
										//case hash("__FILE__"): {
										//	m_match_stack.push_back(make_node<eval::Constant_AST_Node>(currentScope, text, prev_pos.line, prev_pos.col, const_var(m_filename)));
										//} break;
									default: {
										auto val = text;
										if (*prev_pos == '`') { // 'escaped' literal, like an operator name ( e.g. `[]`(...) )
											val = Position::str(prev_pos + 1, m_position - 1);
										}
										if (1) {
											switch (T) {
											default:
												m_match_stack.push_back({ make_node<AST_Nodes::Id_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											case AST_Nodes::IdType::Function:
												m_match_stack.push_back({ make_node<AST_Nodes::FunctionName_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											case AST_Nodes::IdType::Variable:
												m_match_stack.push_back({ make_node<AST_Nodes::VariableName_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											case AST_Nodes::IdType::Class:
												m_match_stack.push_back({ make_node<AST_Nodes::ClassName_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											}
										}
									} break;
									}
								}
								return true;
							}
							else {
								return false;
							}
						}
					};
					/// Reads (and potentially captures) an type or class identifier from input
					bool TypeName(const std::shared_ptr<Scope>& currentScope, bool allowAuto = false) {
						if (Id(false, currentScope, AST_Nodes::IdType::Class)) {
							return true;
						}
						else if (allowAuto) {
							const auto prev_pos = m_position;
							if (Keyword("auto")) {
								auto ptr = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(make_node<AST_Nodes::ClassName_AST_Node>(currentScope, Position::str(prev_pos, m_position), prev_pos));
								ptr->TypeInfo = user_type_shared<Any>();
								m_match_stack.push_back({ std::dynamic_pointer_cast<AST_Node_Impl>(ptr), currentScope }); // e.g. "x", "Units::meter", etc.
							}
							else {
								return false;
							}
						}
						else {
							return false;
						}
					};
		
					/// Reads an argument from input
					bool Arg(const std::shared_ptr<Scope>& currentScope, const bool t_type_allowed = true) {
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						SkipWS();

						bool foundType = false;
						if (t_type_allowed) {
							foundType = TypeName(currentScope);
						}

						if (!Id(true, currentScope, AST_Nodes::IdType::Variable)) {
							return failure();
						}

						build_match<AST_Nodes::Arg_AST_Node>(currentScope, prev_stack_top);

						return true;
					};

					/// Reads a comma-separated list of values from input. Id's only, no types allowed
					bool Id_Arg_List(const std::shared_ptr<Scope>& currentScope) {
						SkipWS(true);

						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Arg(currentScope, false)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Arg(currentScope, false)) {
									throw exception::eval_error("Unexpected value in parameter list", (File_Position)m_position);
								}
							}
						}
						build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);

						SkipWS(true);

						return retval;
					};

					/// Reads a comma-separated list of values from input, for function declarations
					bool Decl_Arg_List(const std::shared_ptr<Scope>& currentScope) {
						SkipWS(true);

						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Arg(currentScope, true)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Arg(currentScope, true)) {
									throw exception::eval_error("Unexpected value in parameter list", (File_Position)m_position);
								}
							}
						}
						build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);

						SkipWS(true);

						return retval;
					};

					/// Reads a comma-separated list of values from input
					bool Arg_List(const std::shared_ptr<Scope>& currentScope, int maxNumArgs = std::numeric_limits<int>::max()) {
						SkipWS(true);
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Equation(currentScope)) {
							retval = true;
							SkipWS(true);
							while (((--maxNumArgs) > 0)) {
								SkipWS(true);
								if (!Char(',')) break;
								SkipWS(true);
								if (!Equation(currentScope)) {
									throw exception::eval_error("Unexpected value in parameter list", (File_Position)m_position);
								}
							}
						}

						build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);

						SkipWS(true);

						return retval;
					};

					/// Reads a C-style type-cast from input (e.g. (int)0.0f )
					bool TypeCastOperation(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						SkipWS(true);

						// (string)100
						if (Char('(') && TypeName(currentScope) && Char(')')) {
							if (Operator(currentScope)) {
								retval = true;
								build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top + 1);
								build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, prev_stack_top); // Id(fun name), Arg_List()
							}
						}

						if (!retval) {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
						}
						return retval;
					};

					/// Parses a string of binary equation operators
					bool Equation(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						using SS = utility::Static_String;

						if (TypeCastOperation(currentScope)) {
							return true;
						}

						if (Operator(currentScope)) {
							for (const auto& sym :
								{ SS{"="}, SS{":="}, SS{"?="}, SS{".."}, SS{"+="}, SS{"-="}, SS{"*="}, SS{"/="}, SS{"%="}, SS{"<<="}, SS{">>="}, SS{"&="}, SS{"^="}, SS{"|="} }) {
								if (Symbol(sym.c_str(), true)) {
									SkipWS(true);
									if (!Equation(currentScope)) {
										throw exception::eval_error("Incomplete equation", (File_Position)m_position);
									}

									build_match<AST_Nodes::Equation_AST_Node>(currentScope, prev_stack_top, sym.c_str());
									return true;
								}
							}
							return true;
						}

						return false;
					};
					// int x;
					// int const& x;
					bool SpecifiedType_Var_Decl(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						if (TypeName(currentScope)) { // typename was specified and found
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top + 1); // {no_params}
							build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, prev_stack_top); // collapse all into a function call (i.e. int({no_params}))

							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) {
								retval = true;
								build_match<AST_Nodes::Var_Decl_AST_Node>(currentScope, prev_stack_top + 1);  // var i;                           
							}

							if (retval) {
								// Fun_Call ("Typename()") , Id or Ref ("Variable name");
								build_match < AST_Nodes::Assign_Retroactively_AST_Node>(currentScope, prev_stack_top);
							}
						}
						if (!retval) {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
						}

						return retval;
					}

					/// Reads a variable declaration from input
					bool Var_Decl(const std::shared_ptr<Scope>& currentScope) {
						const bool t_namespace_context = currentScope->IsNamespace();

						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						if (t_namespace_context) { // Classes and namespaces must explicitely define their variable types. They must not be left implicit e.g. auto or var
							return SpecifiedType_Var_Decl(currentScope);
						}
						else { // Normal scopes may utilize implicit or late-definition style variables if they so choose, for ease.
							if (Keyword("auto") || Keyword("var")) {
								(void)Char('&');
								if (Id(true, currentScope, AST_Nodes::IdType::Variable)) {									
									build_match<AST_Nodes::Var_Decl_AST_Node>(currentScope, prev_stack_top);
									retval = true;
								}
								else {
									throw exception::eval_error("Incomplete variable declaration ", (File_Position)m_position);
								}
							}
							else {
								return SpecifiedType_Var_Decl(currentScope);
							}
						}
						return retval;
					};

					/// Reads a unary prefixed expression from input
					bool Prefix(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						using SS = utility::Static_String;
						constexpr std::array<utility::Static_String, 6> prefix_opers{ SS{"++"}, SS{"--"}, SS{"-"}, SS{"+"}, SS{"!"}, SS{"~"} };
						for (const auto& oper : prefix_opers) {
							const bool is_char = oper.size() == 1;
							if ((is_char && Char(oper.c_str()[0])) || (!is_char && Symbol(oper.c_str()))) {
								if (!Operator(currentScope, operators().size() - 1)) {
									throw exception::eval_error("Incomplete prefix '" + std::string(oper.c_str()) + "' expression", (File_Position)m_position);
								}
								build_match<AST_Nodes::Prefix_AST_Node>(currentScope, prev_stack_top, oper.c_str());
								return true;
							}
						}
						return false;
					};

					static auto make_postfix_operators() {
						std::map<int, std::vector<std::pair<std::weak_ptr<GoodLang::Type_Info>, Units::value>>, std::greater_equal<int>> out;
						for (auto& unit_type : Units::value::GetValueTypes()) {
							auto abbreviation = std::string("_") + std::string(unit_type.second.UnitAbbreviation());
							out[abbreviation.length()].push_back(unit_type);
						}	
						return out;
					}

					bool Postfix(const std::shared_ptr<Scope>& currentScope, bool gotValueAlready) {
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						// add support for custom post-fixes
						// Examples: 
						// 12_in = inch(12)
						// 1_gal = gallon(1)

						if (gotValueAlready) {
							if (Symbol("++")) {
								build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top - 1, "++");
								return true;
							}
							else if (Symbol("--")) {
								build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top - 1, "--");
								return true;
							}
							else {
								static std::map<int, std::vector<std::pair<std::weak_ptr<GoodLang::Type_Info>, Units::value>>, std::greater_equal<int>>
									customOperators{ make_postfix_operators() };

								// evaluate the custom operators...
								if (prev_stack_top > 0 && m_match_stack[prev_stack_top - 1].first->text != "" && m_match_stack[prev_stack_top - 1].first->identifier == AST_Node_Type::Constant) {
									// this path means the incoming value is constant and known									
									for (auto& abbreviation_length_to_category : customOperators) {
										for (auto& unit_type : abbreviation_length_to_category.second) {
											auto abbreviation = std::string("_") + std::string(unit_type.second.UnitAbbreviation());
											if (Symbol(abbreviation)) {
												auto& rhs = std::dynamic_pointer_cast<AST_Nodes::Constant_AST_Node>(m_match_stack[prev_stack_top - 1].first)->m_value;
												Any lhs;
												if (auto Class = currentScope->FindClass(unit_type.first)) {
													lhs = Class->CallFunction(Class->GetName(), rhs);
												}
												else {
													lhs = Any(unit_type.second);
													currentScope->CallFunction("=", { lhs, rhs });
												}
												std::string temp = GoodLang::ToString(lhs);

												Parse_Location loc = m_match_stack[prev_stack_top - 1].first->location;
												loc.end.col += abbreviation.length();

												m_match_stack[prev_stack_top - 1].first =
													std::dynamic_pointer_cast<AST_Node_Impl>(
														std::make_shared<AST_Nodes::Constant_AST_Node>(temp, loc, lhs)
														);

												return true;
											}
										}
									}
								}
								else if (prev_stack_top > 0 && m_match_stack[prev_stack_top - 1].first->text != "") {
									// this path means the incoming value is NOT constant and is not known. 
									for (auto& abbreviation_length_to_category : customOperators) {
										for (auto& unit_type : abbreviation_length_to_category.second) {
											auto abbreviation = std::string("_") + std::string(unit_type.second.UnitAbbreviation());
											if (Symbol(abbreviation)) {
												Any lhs;
												if (auto Class = currentScope->FindClass(unit_type.first)) {
													lhs = Class->CallFunction(Class->GetName(), {});
												}
												else {
													lhs = Any(unit_type.second);;
												}

												Parse_Location loc = m_match_stack[prev_stack_top - 1].first->location;
												loc.end.col += abbreviation.length();

												m_match_stack[prev_stack_top - 1].first =
													std::dynamic_pointer_cast<AST_Node_Impl>(
														std::make_shared<AST_Nodes::Equation_AST_Node>(currentScope, "=", loc, std::vector<AST_Node_Impl_Ptr>{
													std::dynamic_pointer_cast<AST_Node_Impl>(
														std::make_shared<AST_Nodes::Constant_AST_Node>(GoodLang::ToString(lhs), loc, lhs)
														),
														std::move(m_match_stack[prev_stack_top - 1].first)
												    })
												);

												return true;


												//// To-Do, finish this analysis!
												//// Insert a node that evaluates the function `=`(lhs, rhs) and returns lhs.







												//throw std::runtime_error("FIX ME!");
												//while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
												//m_position = prev_pos;


											}
										}
									}
								}
							}
						}
						else {
							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) {
								if (Symbol("++")) {
									build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top, "++");
									return true;
								}
								else if (Symbol("--")) {
									build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top, "--");
									return true;
								}
								while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
								m_position = prev_pos;
							}
						}
						return false;
					}

					/// Reads a pair of values used to create a map initialization from input
					bool Map_Pair(const std::shared_ptr<Scope>& currentScope) {
						SkipWS(true);
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						if (Operator(currentScope)) {
							if (Symbol(":")) {
								retval = true;
								if (!Operator(currentScope)) { throw exception::eval_error("Incomplete map pair", (File_Position)m_position); }

								build_match<AST_Nodes::Map_Pair_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								m_position = prev_pos;
								while (prev_stack_top != m_match_stack.size()) {
									m_match_stack.pop_back();
								}
							}
						}

						return retval;
					}

					/// Reads possible special container values, including ranges and map_pairs
					bool Container_Arg_List(const std::shared_ptr<Scope>& currentScope) {
						SkipWS(true);
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Map_Pair(currentScope)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Map_Pair(currentScope)) {
									throw exception::eval_error("Unexpected value in container", (File_Position)m_position);
								}
							}
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);
						}
						else if (Operator(currentScope)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Operator(currentScope)) {
									throw exception::eval_error("Unexpected value in container", (File_Position)m_position);
								}
								SkipWS(true);
							}
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);
						}

						SkipWS(true);

						return retval;
					}

					/// Reads, and identifies, a short-form container initialization from input
					bool Inline_Container(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();

						if (Char('[')) {
							SkipWS(true);
							Container_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(']')) {
								throw exception::eval_error("Missing closing square bracket ']' in container initializer", (File_Position)m_position);
							}
							if ((prev_stack_top != m_match_stack.size()) && (!m_match_stack.back().first->children.empty())) {
								if (m_match_stack.back().first->children[0]->identifier == AST_Node_Type::Map_Pair) {
									build_match<AST_Nodes::Inline_Map_AST_Node>(currentScope, prev_stack_top);
								}
								else {
									build_match<AST_Nodes::Inline_Array_AST_Node>(currentScope, prev_stack_top);
								}
							}
							else {
								build_match<AST_Nodes::Inline_Array_AST_Node>(currentScope, prev_stack_top);
							}

							return true;
						}
						else {
							return false;
						}
					}

					/// Reads a lambda (anonymous function) from input
					bool Lambda(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						/* All of the following should be examples of valid forms of lambda functions */
						// [...](...) async -> typename {...} 
						// [...](...) -> typename {...} 
						// [...](...) async {...} 
						// (...) async {...} 
						// (...) {...} 

						// Arg_List
						if (Char('[')) {
							SkipWS(true);
							Id_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(']')) {
								return failure();
							}
						}
						else {
							// make sure we always have the same number of nodes
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);
						}

						// Arg_List
						if (Char('(')) {
							SkipWS(true);
							Decl_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(')')) {
								return failure();
							}
						}
						else {
							return failure();
						}

						// KeyWords / modifiers
						bool is_async = false;
						bool foundKeyWord = true;
						while (foundKeyWord) {
							SkipWS(true);
							foundKeyWord = false;

							if (Keyword("async")) {
								foundKeyWord = true;
								is_async = true;
							}
						}

						SkipWS(true);

						// Noop or Id
						if (Symbol("->")) {
							SkipWS(true);
							const auto start = m_position;
							if (!TypeName(currentScope)) {
								return failure();
							}
						}
						else {
							// make sure we always have the same number of nodes
							m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
						}

						// Block
						SkipWS(true);
						if (!Block(currentScope)) {
							return failure();
						}

						auto lambda_node = build_match<AST_Nodes::Lambda_AST_Node>(currentScope, prev_stack_top);
						lambda_node->is_async = is_async;

						return true;
					};

					bool Dot_Fun_Array(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (Lambda(currentScope) || Num() || Quoted_String(currentScope) || Paren_Expression(currentScope) || Inline_Container(currentScope) || Id(false, currentScope, AST_Nodes::IdType::Variable)) {
							retval = true;
							bool has_more = true;

							while (has_more) {
								has_more = false;
								if (Char('(')) {
									has_more = true;
									SkipWS(true);
									Arg_List(currentScope);
									SkipWS(true);
									if (!Char(')')) {
										throw exception::eval_error("Incomplete function call", (File_Position)m_position);
									}

									build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, prev_stack_top, "()");
									/// \todo Work around for method calls until we have a better solution
									if (!m_match_stack.back().first->children.empty()) {
										if (m_match_stack.back().first->children[0]->identifier == AST_Node_Type::Dot_Access) {
											if (m_match_stack.empty()) {
												throw exception::eval_error("Incomplete dot access fun call", (File_Position)m_position);
											}
											if (m_match_stack.back().first->children.empty()) {
												throw exception::eval_error("Incomplete dot access fun call", (File_Position)m_position);
											}
											auto dot_access = std::move(m_match_stack.back().first->children[0]);
											auto func_call = std::move(m_match_stack.back());
											m_match_stack.pop_back();
											func_call.first->children.erase(func_call.first->children.begin());
											if (dot_access->children.empty()) {
												throw exception::eval_error("Incomplete dot access fun call", (File_Position)m_position);
											}
											func_call.first->children.insert(func_call.first->children.begin(), std::move(dot_access->children.back()));
											dot_access->children.pop_back();
											dot_access->children.push_back(std::move(func_call.first));
											if (dot_access->children.size() != 2) {
												throw exception::eval_error("Incomplete dot access fun call", (File_Position)m_position);
											}
											m_match_stack.push_back({ dot_access, func_call.second });
										}
									}
								}
								else if (Char('[')) {
									has_more = true;
									if (!(Operator(currentScope) && Char(']'))) {
										// TO-DO, Extend to allow matrix accessors, i.e. matrix_obj[0,0] = 10.0;
										throw exception::eval_error("Incomplete array access", (File_Position)m_position);
									}

									build_match<AST_Nodes::Array_Call_AST_Node>(currentScope, prev_stack_top, "[]");
								}
								else if (Symbol(".")) {
									has_more = true;
									if (!(Id(true, currentScope, AST_Nodes::IdType::Function))) {
										throw exception::eval_error("Incomplete dot access fun call", (File_Position)m_position);
									}

									if (std::distance(m_match_stack.begin() + static_cast<int>(prev_stack_top), m_match_stack.end()) != 2) {
										throw exception::eval_error("Incomplete dot access fun call", (File_Position)m_position);
									}

									build_match<AST_Nodes::Dot_Access_AST_Node>(currentScope, prev_stack_top, ".");
								}
								else if (Eol()) {
									auto start = (--m_position);
									SkipWS(true);
									if (Symbol(".")) {
										has_more = true;
										--m_position;
									}
									else {
										m_position = start;
									}
								}
							}
						}

						return retval;
					};

					/// Parses any of a group of 'value' style ast_node groups from input
					bool Value(const std::shared_ptr<Scope>& currentScope) {
						if (Var_Decl(currentScope) || Dot_Fun_Array(currentScope) || Prefix(currentScope)) {
							Postfix(currentScope, true);
							return true;
						}
						else {
							return Postfix(currentScope, false);
						}
					};

					bool Operator(const std::shared_ptr<Scope>& currentScope, const size_t t_precedence = 0) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (operators()[t_precedence] != Operator_Precedence::Prefix) {
							if (Operator(currentScope, t_precedence + 1)) {
								std::string oper;
								retval = true;
								while (Operator_Helper(t_precedence, oper)) {
									while (Eol()) {}

									if (!Operator(currentScope, t_precedence + 1)) {
										throw exception::eval_error("Incomplete '" + oper + "' expression", (File_Position)m_position);
									}

									switch (operators()[t_precedence]) {
									case (Operator_Precedence::Ternary_Cond):
										if (oper == "?=") {
											build_match<AST_Nodes::Equation_AST_Node>(currentScope, prev_stack_top, oper);
										}
										else {
											if (Symbol(":")) {
												if (!Operator(currentScope, t_precedence + 1)) {
													throw exception::eval_error("Incomplete '" + oper + "' expression", (File_Position)m_position);
												}
												build_match<AST_Nodes::If_AST_Node>(currentScope, prev_stack_top);
											}
											else {
												throw exception::eval_error("Incomplete '" + oper + "' expression", (File_Position)m_position);
											}
										}
										break;

									case (Operator_Precedence::Addition):
									case (Operator_Precedence::Multiplication):
									case (Operator_Precedence::Shift):
									case (Operator_Precedence::Equality):
									case (Operator_Precedence::Bitwise_And):
									case (Operator_Precedence::Bitwise_Xor):
									case (Operator_Precedence::Bitwise_Or):
									case (Operator_Precedence::Comparison):
										build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, oper);
										break;

									case (Operator_Precedence::Logical_And):
										build_match<AST_Nodes::Logical_And_AST_Node>(currentScope, prev_stack_top, oper);
										break;
									case (Operator_Precedence::Logical_Or):
										build_match<AST_Nodes::Logical_Or_AST_Node>(currentScope, prev_stack_top, oper);
										break;
									case (Operator_Precedence::Prefix):
										assert(false); // cannot reach here because of if() statement at the top
										break;
									}
								}
							}
						}
						else {
							return Value(currentScope);
						}

						return retval;
					}

					/// Reads an expression surrounded by parentheses from input
					bool Paren_Expression(const std::shared_ptr<Scope>& currentScope) {
						if (Char('(')) {
							SkipWS(true);
							if (!Operator(currentScope)) {
								throw exception::eval_error("Incomplete expression", (File_Position)m_position);
							}
							SkipWS(true);
							if (!Char(')')) {
								throw exception::eval_error("Missing closing parenthesis ')'", (File_Position)m_position);
							}
							return true;
						}
						else {
							return false;
						}
					};

					/// Reads a while block from input
					bool While(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("while")) {
							retval = true;

							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'while' expression", (File_Position)m_position);
							}

							if (!(Operator(currentScope) && Char(')'))) {
								throw exception::eval_error("Incomplete 'while' expression", (File_Position)m_position);
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'while' block", (File_Position)m_position);
							}

							build_match<AST_Nodes::While_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads the C-style `for` conditions from input
					bool For_Guards(const std::shared_ptr<Scope>& currentScope) {
						if (!(Equation(currentScope) && Eol())) {
							if (!Eol()) {
								return false;
							}
							else {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}
						}

						if (!(Equation(currentScope) && Eol())) {
							if (!Eol()) {
								return false;
							}
							else {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Constant_AST_Node>(Any(true)), nullptr });
							}
						}

						if (!Equation(currentScope)) {
							m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
						}

						return true;
					}

					/// Reads the C-style `for` conditions from input
					bool Parallel_For_Guards(const std::shared_ptr<Scope>& currentScope) {
						if (!(Equation(currentScope) && Eol())) {
							if (!Eol()) {
								return false;
							}
							else {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}
						}

						if (!(Equation(currentScope))) {
							m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Constant_AST_Node>(Any(true)), nullptr });
						}

						return true;
					}

					/// Reads the ranged `for` conditions from input
					bool Range_Expression(const std::shared_ptr<Scope>& currentScope) {
						// the first element will have already been captured by the For_Guards() call that preceeds it
						return Char(':') && Equation(currentScope);
					}

					/// Reads a for block from input
					bool For(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("for")) {
							retval = true;

							SkipWS(true);

							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'for' expression", (File_Position)m_position);
							}

							SkipWS(true);

							bool classic_for = For_Guards(currentScope);
							SkipWS(true);
							if (classic_for) classic_for = classic_for && Char(')');
							if (!classic_for) {
								classic_for = Range_Expression(currentScope);
								SkipWS(true);
								if (classic_for) classic_for = classic_for && Char(')');

								if (!classic_for) {
									throw exception::eval_error("Incomplete 'for' expression", (File_Position)m_position);
								}

								classic_for = false;
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'for' block", (File_Position)m_position);
							}

							const auto num_children = m_match_stack.size() - prev_stack_top;

							if (classic_for) {
								if (num_children != 4) {
									throw exception::eval_error("Incomplete 'for' expression", (File_Position)m_position);
								}
								build_match<AST_Nodes::For_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								if (num_children != 3) {
									throw exception::eval_error("Incomplete ranged-for expression", (File_Position)m_position);
								}
								build_match<AST_Nodes::Ranged_For_AST_Node>(currentScope, prev_stack_top);
							}
						}
						else if (Keyword("parallel_for")) {
							// parallel_for (var x = START_VALUE ; END_VALUE) WORK_BLOCK; // this approach means every iteration will see it's own local "x"
							// parallel_for (START_VALUE ; END_VALUE) WORK_BLOCK // this approach means every iteration will NOT see any "x" at all
							// parallel_for (range_declaration : range_expression) loop_statement;
							retval = true;

							SkipWS(true);

							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'parallel_for' expression", (File_Position)m_position);
							}

							SkipWS(true);

							bool classic_for = Parallel_For_Guards(currentScope);
							SkipWS(true);
							if (classic_for) classic_for = classic_for && Char(')');
							if (!classic_for) {
								classic_for = Range_Expression(currentScope);
								SkipWS(true);
								if (classic_for) classic_for = classic_for && Char(')');

								if (!classic_for) {
									throw exception::eval_error("Incomplete 'parallel_for' expression", (File_Position)m_position);
								}

								classic_for = false;
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'parallel_for' block", (File_Position)m_position);
							}

							const auto num_children = m_match_stack.size() - prev_stack_top;

							if (classic_for) {
								if (num_children != 3) {
									throw exception::eval_error("Incomplete 'parallel_for' expression", (File_Position)m_position);
								}
								build_match<AST_Nodes::Parallel_For_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								if (num_children != 3) {
									throw exception::eval_error("Incomplete ranged-parallel_for expression", (File_Position)m_position);
								}
								build_match<AST_Nodes::Parallel_Ranged_For_AST_Node>(currentScope, prev_stack_top);
							}
						}

						return retval;
					}

					/// Reads a break statement from input
					bool Break(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("break")) {
							build_match<AST_Nodes::Break_AST_Node>(currentScope, prev_stack_top);
							return true;
						}
						else {
							return false;
						}
					}

					/// Reads a continue statement from input
					bool Continue(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("continue")) {
							build_match<AST_Nodes::Continue_AST_Node>(currentScope, prev_stack_top);
							return true;
						}
						else {
							return false;
						}
					}

					/// Reads a case block from input
					bool Case(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						// case "option": { ... }
						// case "option" { ... }
						if (Keyword("case")) {
							retval = true;

							SkipWS(true);

							if (!Operator(currentScope)) {
								throw exception::eval_error("Incomplete 'case' expression", (File_Position)m_position);
							}

							SkipWS(true);

							Char(':'); // optional

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'case' block", (File_Position)m_position);
							}

							build_match<AST_Nodes::Case_AST_Node>(currentScope, prev_stack_top);
						}
						// default: { ... }
						// default { ... }
						else if (Keyword("default")) {
							retval = true;

							SkipWS(true);

							Char(':'); // optional

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'default' block", (File_Position)m_position);
							}

							build_match<AST_Nodes::Default_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a switch statement from input
					bool Switch(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("switch")) {
							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'switch' expression", (File_Position)m_position);
							}

							if (!(Operator(currentScope) && Char(')'))) {
								throw exception::eval_error("Incomplete 'switch' expression", (File_Position)m_position);
							}

							SkipWS(true);

							if (Char('{')) {
								SkipWS(true);

								while (Case(currentScope)) {
									SkipWS(true);
								}

								SkipWS(true);

								if (!Char('}')) {
									throw exception::eval_error("Incomplete block", (File_Position)m_position);
								}
							}
							else {
								throw exception::eval_error("Incomplete block", (File_Position)m_position);
							}

							build_match<AST_Nodes::Switch_AST_Node>(currentScope, prev_stack_top);
							return true;

						}
						else {
							return false;
						}
					}

					/// Reads a try-catch from input
					bool Try(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("try")) {
							retval = true;

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'try' block", (File_Position)m_position);
							}

							bool has_matches = true;
							while (has_matches) {
								SkipWS(true);
								has_matches = false;
								if (Keyword("catch")) {
									const auto catch_stack_top = m_match_stack.size();
									if (Char('(')) {
										bool success = false;
										if (Symbol("...")) {
											// captures anything...
											if (!Char(')')) {
												throw exception::eval_error("Incomplete 'catch(...)' expression", (File_Position)m_position);
											}
											success = true;
										}

										if (Arg(currentScope, true)) {
											if (!Char(')')) {
												throw exception::eval_error("Incomplete 'catch' expression", (File_Position)m_position);
											}
											success = true;
										}

										if (!success) {
											throw exception::eval_error("Incomplete 'catch' expression", (File_Position)m_position);
										}
									}

									SkipWS(true);

									if (!Block(currentScope)) {
										throw exception::eval_error("Incomplete 'catch' block", (File_Position)m_position);
									}
									build_match<AST_Nodes::Catch_AST_Node>(currentScope, catch_stack_top);
									has_matches = true;
								}
							}
							SkipWS(true);
							if (Keyword("finally")) {
								const auto finally_stack_top = m_match_stack.size();

								SkipWS(true);

								if (!Block(currentScope)) {
									throw exception::eval_error("Incomplete 'finally' block", (File_Position)m_position);
								}
								build_match<AST_Nodes::Finally_AST_Node>(currentScope, finally_stack_top);
							}

							build_match<AST_Nodes::Try_AST_Node>(currentScope, prev_stack_top);
						}
						else if (Keyword("do")) {
							retval = true;

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'do' block", (File_Position)m_position);
							}
							SkipWS(true);
							if (Keyword("finally")) {
								const auto finally_stack_top = m_match_stack.size();

								SkipWS(true);

								if (!Block(currentScope)) {
									throw exception::eval_error("Incomplete 'finally' block", (File_Position)m_position);
								}
								build_match<AST_Nodes::Finally_AST_Node>(currentScope, finally_stack_top);
							}

							build_match<AST_Nodes::Do_AST_Node>(currentScope, prev_stack_top);
						}
						return retval;
					}

					/// Reads a just-in-time compilation request from input
					bool Eval(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("evaluate")) {
							retval = true;
							SkipWS(true);
#if 1
							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'evaluate' expression", (File_Position)m_position);
							}
							SkipWS(true);
							if (!Equation(currentScope)) {
								throw exception::eval_error("Incomplete 'evaluate' expression", (File_Position)m_position);
							}
							SkipWS(true);
							if (!Char(')')) {
								throw exception::eval_error("Incomplete 'evaluate' expression", (File_Position)m_position);
							}
#else
							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'evaluate' block", (File_Position)m_position);
							}
#endif
							build_match<AST_Nodes::JustInTimeCompilation_AST_Node>(currentScope, prev_stack_top);
						}
						return retval;
					}

					/// Reads a namespace block from input
					/// namespace Thing{ ... };
					bool DeclClass(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						// class TypeName { ... }
						// class TypeName : ParentTypeName { ... }

						if (Keyword("class")) {
							retval = true;
							SkipWS(true);

							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) { // variable becase this namespace may not exist yet! 
								/* Great! Got the desired name of the new namespace */
							}
							else {
								throw exception::eval_error("Incomplete 'class' block: class must have a name", (File_Position)m_position);
							}


							std::string desired_namespace = currentScope->GetQualifiedNamespace() + "::" + std::string(GetText(m_match_stack.back().first));

							static auto fixNamespace{ [](std::string& x) {
								while (x.find("::") == 0 && x.length() > 2) {
									x = x.substr(2);
								}

								while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
							} };
							fixNamespace(desired_namespace);

							// if the namespace already exists, then we should "resume" it, rather than creating a new one. 
							auto newNamespace = currentScope->FindClass(desired_namespace);
							if (!newNamespace) {
								newNamespace = std::make_shared<Class>(currentScope, std::string(GetText(m_match_stack.back().first)));
								newNamespace->SetSelf(newNamespace);
								currentScope->AddChild(newNamespace); // add the new namespace as a child

								// Default Constructors
								newNamespace->AddDefaultConstructors();
							}


							// instead of collecting statements, we want to collect declarations...
							if (!DeclarationsBlock(newNamespace)) {
								throw exception::eval_error("Incomplete 'class' block: class declarations must be wrapped in a curly-bracket block", (File_Position)m_position);
							}

							build_match<AST_Nodes::Class_AST_Node>(newNamespace, prev_stack_top);
						}
						return retval;
					};

					/// Reads a namespace block from input
					/// namespace Thing{ ... };
					bool DeclNamespace(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("namespace")) {
							retval = true;
							SkipWS(true);

							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) { // variable becase this namespace may not exist yet! 
								/* Great! Got the desired name of the new namespace */
							}
							else {
								throw exception::eval_error("Incomplete 'namespace' block: namespace must have a name", (File_Position)m_position);
							}

							std::string desired_namespace = currentScope->GetQualifiedNamespace() + "::" + std::string(GetText(m_match_stack.back().first));
							static auto fixNamespace{ [](std::string& x) {
								while (x.find("::") == 0 && x.length() > 2) {
									x = x.substr(2);
								}
								while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }
							} };
							fixNamespace(desired_namespace);
							auto newNamespace = currentScope->FindNamespace(desired_namespace);
							if (!newNamespace) {
								newNamespace = std::make_shared<Namespace>(currentScope, std::string(GetText(m_match_stack.back().first)));
								newNamespace->SetSelf(newNamespace);
								currentScope->AddChild(newNamespace); // add the new namespace as a child
							}
							// instead of collecting statements, we want to collect declarations...
							if (!DeclarationsBlock(newNamespace)) {
								throw exception::eval_error("Incomplete 'namespace' block: namespace declarations must be wrapped in a curly-bracket block", (File_Position)m_position);
							}
							build_match<AST_Nodes::Namespace_AST_Node>(newNamespace, prev_stack_top);
						}
						return retval;
					};

					/// Reads a declared function from input
					/// Type Foo(...){ ... };
					bool DeclFunction(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						// return type (Id)
						if (!TypeName(currentScope, true)) { // Id
							return failure();
						}

						// function name (Id)
						if (!Id(true, currentScope, AST_Nodes::IdType::Function)) {
							return failure();
						}

						// Arg_List 
						if (Char('(')) {
							SkipWS(true);
							Decl_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(')')) {
								return failure();
							}
						}
						else {
							return failure();
						}

						// Block
						auto this_scope = std::make_shared<Scope>(currentScope);
						this_scope->SetSelf(this_scope);						
						if (!Block(this_scope)) {
							return failure();
						}

						build_match<AST_Nodes::FunctionDecl_AST_Node>(currentScope, prev_stack_top);

						return true;
					};

					/// Top level parser, starts parsing of all known parses
					bool Declarations(const std::shared_ptr<Scope>& currentScope) {
						SkipWS();

						bool retval = false;
						bool has_more = true;
						bool saw_eol = true;

						while (has_more) {
							const auto start = m_position;

							// TO-DO, complete impl of these evaluations:

							if (DeclNamespace(currentScope) || DeclFunction(currentScope) || DeclClass(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two function definitions missing line separator", (File_Position)start);
								}
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else if (Equation(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two expressions missing line separator", (File_Position)start);
								}
								has_more = true;
								retval = true;
								saw_eol = false;
							}
							else if (DeclarationsBlock(currentScope) || Eol()) {
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else {
								has_more = false;
							}
						}
						return retval;
					};

					/// Top level parser, starts parsing of all known parses
					bool Statements(const std::shared_ptr<Scope>& currentScope) {
						SkipWS();

						bool retval = false;
						bool has_more = true;
						bool saw_eol = true;

						while (has_more) {
							const auto start = m_position;
							if (DeclNamespace(currentScope) || DeclClass(currentScope) || DeclFunction(currentScope) || /*Def(currentScope) || */ Try(currentScope) || If(currentScope) || While(currentScope) || /* Class(currentScope) || */ For(currentScope) || Switch(currentScope) || Eval(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two function definitions missing line separator", (File_Position)start);
								}
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else if (Return(currentScope) || Break(currentScope) || Continue(currentScope) || Equation(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two expressions missing line separator", (File_Position)start);
								}
								has_more = true;
								retval = true;
								saw_eol = false;
							}
							else if (Block(currentScope) || Eol()) {
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else {
								has_more = false;
							}
						}
						return retval;
					};

					/// Reads a curly-brace C-style block from input
					bool Block(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Char('{')) {
							retval = true;

							Statements(currentScope);

							if (!Char('}')) {
								throw exception::eval_error("Incomplete block", (File_Position)m_position);
							}

							if (m_match_stack.size() == prev_stack_top) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}

							build_match<AST_Nodes::Block_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a curly-brace C-style block from input which only allows for declarations
					bool DeclarationsBlock(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Char('{')) {
							retval = true;

							Declarations(currentScope);

							if (!Char('}')) {
								throw exception::eval_error("Incomplete declaration block", (File_Position)m_position);
							}

							if (m_match_stack.size() == prev_stack_top) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}

							build_match<AST_Nodes::Declaration_Block_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a curly-brace C-style block from input -- note that this scope is special and cannot find objects from parent scopes. 
					bool FunctionBlock(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Char('{')) {
							retval = true;

							Statements(currentScope);

							if (!Char('}')) {
								throw exception::eval_error("Incomplete function block", (File_Position)m_position);
							}

							if (m_match_stack.size() == prev_stack_top) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}

							build_match<AST_Nodes::Function_Block_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a return statement from input
					bool Return(const std::shared_ptr<Scope>& currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("return")) {
							Operator(currentScope);
							build_match<AST_Nodes::Return_AST_Node>(currentScope, prev_stack_top);
							return true;
						}
						else {
							return false;
						}
					}

					/// Reads an if/else if/else block from input
					bool If(const std::shared_ptr<Scope>& currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						// SkipWS(true);
						if (Keyword("if")) {
							retval = true;
							SkipWS(true);
							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'if' expression: cannot find '('", (File_Position)m_position);
							}
							SkipWS(true);
							if (!Equation(currentScope)) {
								throw exception::eval_error("Incomplete 'if' expression: cannot find equation block", (File_Position)m_position);
							}
							SkipWS(true);
							const bool is_if_init = Eol() && Equation(currentScope);
							SkipWS(true);
							if (!Char(')')) {
								throw exception::eval_error("Incomplete 'if' expression: cannot find ')'", (File_Position)m_position);
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'if' block", (File_Position)m_position);
							}

							bool has_matches = true;
							while (has_matches) {
								SkipWS(true);
								has_matches = false;
								if (Keyword("else")) {
									SkipWS(true);
									if (If(currentScope)) {
										has_matches = true;
									}
									else {
										SkipWS(true);
										if (!Block(currentScope)) {
											throw exception::eval_error("Incomplete 'else' block", (File_Position)m_position);
										}
										has_matches = true;
									}
								}
							}

							const auto num_children = m_match_stack.size() - prev_stack_top;

							if ((is_if_init && num_children == 3) || (!is_if_init && num_children == 2)) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), currentScope });
							}

							if (!is_if_init) {
								build_match<AST_Nodes::If_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								build_match<AST_Nodes::If_AST_Node>(currentScope, prev_stack_top + 1);
								build_match<AST_Nodes::Block_AST_Node>(currentScope, prev_stack_top);
							}
						}

						return retval;
					};

				public:
					Parser() = default;
					~Parser() = default;

					// highest-level parse request, which starts a new scope from scratch and completes it. 
					ParseNode Parse(const std::string& t_input, std::shared_ptr<Scope> parentScope = nullptr) {
						if (!parentScope) {
							parentScope = std::make_shared<Global>();
							parentScope->SetSelf(parentScope);
							std::dynamic_pointer_cast<Global>(parentScope)->AddBuiltIns();
						}
						return parse(t_input, parentScope);
					};

				private:
					ParseNode parse(const std::string& t_input, const std::shared_ptr<Scope>& currentScope) {
						return parse_internal(t_input, currentScope);
					};
					ParseNode parse_internal(const std::string& t_input, const std::shared_ptr<Scope>& currentScope) {
						auto this_scope = std::make_shared<Scope>(currentScope);
						this_scope->SetSelf(this_scope);

						const auto begin = t_input.empty() ? nullptr : &t_input.front();
						const auto end = begin == nullptr ? nullptr : begin + t_input.size();
						m_position = Position(begin, end);

						// top level stack        
						if (Statements(this_scope)) {
							if (m_position.has_more()) {
								throw exception::eval_error("Unparsed input", (File_Position)m_position);
							}
							else {
								// add the comment nodes to the front of the stack, to not interupt the automatic return behavior
								auto i = m_comment_stack.rbegin();
								while (i != m_comment_stack.rend()) {
									m_match_stack.insert(m_match_stack.begin(), std::move(*i));
									i = decltype(i)(m_comment_stack.erase(std::next(i).base()));
								}
								build_match<AST_Nodes::File_AST_Node>(this_scope, 0);
							}
						}
						else {
							m_match_stack.push_back({ std::make_shared<AST_Node_Impl, AST_Nodes::Noop_AST_Node>(AST_Nodes::Noop_AST_Node()), nullptr });
						}

						ParseNode retval = m_match_stack.front();
						m_match_stack.clear();
						// retval.second = currentScope;
						return retval;
					};
					ParseNode parse_instr_eval(const std::string& t_input, const std::shared_ptr<Scope>& currentScope) {
						auto last_position = m_position;
						auto last_match_stack = std::exchange(m_match_stack, decltype(m_match_stack){});

						auto retval = parse_internal(t_input, currentScope);

						m_position = std::move(last_position);

						m_match_stack = std::move(last_match_stack);

						return retval;
					};



                };

			};
		};
		

	};

	namespace Impl {
		__forceinline void ToString(Tag< GoodLang::Engine::Compiler::Interpreter::Parser::ParseNode >, GoodLang::Engine::Compiler::Interpreter::Parser::ParseNode const& r, std::string& out) {
			out = r.first->to_string();
		};
		__forceinline void GetChildren(Tag< GoodLang::Engine::Compiler::Interpreter::Parser::ParseNode >, GoodLang::Engine::Compiler::Interpreter::Parser::ParseNode const& r, std::vector< NodeCache >& out) {
			// out.push_back(GoodLang::GetChildren(r.load()));
		};
	};
};

int main() {
	// pre-warm the heap
	for (int i = 0; i < 100000; i++) delete (new int(5));

	using namespace GoodLang;

	while (1) {
		auto Root = std::make_shared<Scopes::RootScope>();
		Root->SetSelf(Root);		
		EXPECT_EQ(true, Root->is_root());
		EXPECT_EQ(true, Root->is_namespace());
		EXPECT_EQ(false, Root->is_class());

		if (1) {
			auto Class = Root->MakeChildClass("int", user_type_shared_ptr<int>());
			Class->EmplaceFunction("int", make_callable([]() -> int { return 0; }));
			Class->EmplaceFunction("int", make_callable([](int const& rhs) -> int { return rhs; }));
			Class->EmplaceFunction("int", make_callable([](long const& rhs) -> int { return rhs; }));
			Class->EmplaceFunction("int", make_callable([](float const& rhs) -> int { return rhs; }));
			Class->EmplaceFunction("int", make_callable([](double const& rhs) -> int { return rhs; }));
		}
		if (1) {
			auto Class = Root->MakeChildClass("long", user_type_shared_ptr<long>());
			Class->EmplaceFunction("long", make_callable([]() -> long { return 0; }));
			Class->EmplaceFunction("long", make_callable([](int const& rhs) -> long { return rhs; }));
			Class->EmplaceFunction("long", make_callable([](long const& rhs) -> long { return rhs; }));
			Class->EmplaceFunction("long", make_callable([](float const& rhs) -> long { return rhs; }));
			Class->EmplaceFunction("long", make_callable([](double const& rhs) -> long { return rhs; }));
		}
		if (1) {
			auto Class = Root->MakeChildClass("float", user_type_shared_ptr<float>());
			Class->EmplaceFunction("float", make_callable([]() -> float { return 0; }));
			Class->EmplaceFunction("float", make_callable([](int const& rhs) -> float { return rhs; }));
			Class->EmplaceFunction("float", make_callable([](long const& rhs) -> float { return rhs; }));
			Class->EmplaceFunction("float", make_callable([](float const& rhs) -> float { return rhs; }));
			Class->EmplaceFunction("float", make_callable([](double const& rhs) -> float { return rhs; }));
		}
		if (1) {
			auto Class = Root->MakeChildClass("double", user_type_shared_ptr<double>());
			Class->EmplaceFunction("double", make_callable([]() -> double { return 0; }));
			Class->EmplaceFunction("double", make_callable([](int const& rhs) -> double { return rhs; }));
			Class->EmplaceFunction("double", make_callable([](long const& rhs) -> double { return rhs; }));
			Class->EmplaceFunction("double", make_callable([](float const& rhs) -> double { return rhs; }));
			Class->EmplaceFunction("double", make_callable([](double const& rhs) -> double { return rhs; }));
		}

		EXPECT_EQ(Root->Cast<double>(100), 100.0);






		auto Namespace = Root->MakeChildNamespace("UI");
		EXPECT_EQ(false, Namespace->is_root());
		EXPECT_EQ(true, Namespace->is_namespace());
		EXPECT_EQ(false, Namespace->is_class());

		auto Class = Namespace->MakeChildClass("Color");
		EXPECT_EQ(false, Class->is_root());
		EXPECT_EQ(true, Class->is_namespace());
		EXPECT_EQ(true, Class->is_class());
		Class->DeclareMemberObject("R", user_type_shared<int>(), std::make_shared<Any>((int)123));
		Class->DeclareMemberObject("G", user_type_shared<int>(), std::make_shared<Any>((int)124));
		Class->DeclareMemberObject("B", user_type_shared<int>(), std::make_shared<Any>((int)125));
		Class->DeclareMemberObject("A", user_type_shared<int>(), std::make_shared<Any>((int)126));		
		EXPECT_EQ(123, Class->Cast<int>(Class->Call("R", { Class->Call("Color", {}) })));
		EXPECT_EQ(126, Class->Cast<int>(Class->Call("A", { Class->Call("=", { Class->Call("Color", {}), Class->Call("Color", {}) }) })));



		auto Scope = Class->MakeChildScope();
		EXPECT_EQ(false, Scope->is_root());
		EXPECT_EQ(false, Scope->is_namespace());
		EXPECT_EQ(false, Scope->is_class());

		auto Scope2 = Scope->MakeChildScope();
		EXPECT_EQ(false, Scope2->is_root());
		EXPECT_EQ(false, Scope2->is_namespace());
		EXPECT_EQ(false, Scope2->is_class());

		auto String = Root->MakeChildClass("string");
		EXPECT_EQ(false, String->is_root());
		EXPECT_EQ(true, String->is_namespace());
		EXPECT_EQ(true, String->is_class());

		EXPECT_EQ(Namespace.get(), Class->GetParent().get());
		EXPECT_EQ(Class.get(), Class->GetNamespace().get());
		EXPECT_EQ(Root.get(), Class->GetRoot().get());

		EXPECT_EQ((bool)Root->FindClass("Color"), true);
		EXPECT_EQ((bool)Root->FindClass("UI::Color"), true);
		EXPECT_EQ((bool)Root->FindClass("::UI::Color::"), true);
		EXPECT_EQ((bool)Root->FindClass("::Color"), true);
		EXPECT_EQ((bool)Root->FindNamespace("Color"), true);
		EXPECT_EQ((bool)Root->FindNamespace("UI::Color"), true);
		EXPECT_EQ((bool)Root->FindNamespace("::UI::Color::"), true);
		EXPECT_EQ((bool)Root->FindNamespace("::Color"), true);

		EXPECT_EQ((bool)Namespace->FindClass("Color"), true);
		EXPECT_EQ((bool)Namespace->FindClass("UI::Color"), true);
		EXPECT_EQ((bool)Namespace->FindClass("::UI::Color::"), true);
		EXPECT_EQ((bool)Namespace->FindClass("::Color"), true);
		EXPECT_EQ((bool)Namespace->FindNamespace("Color"), true);
		EXPECT_EQ((bool)Namespace->FindNamespace("UI::Color"), true);
		EXPECT_EQ((bool)Namespace->FindNamespace("::UI::Color::"), true);
		EXPECT_EQ((bool)Namespace->FindNamespace("::Color"), true);

		EXPECT_EQ((bool)Namespace->EmplaceFunction("max", make_callable([](int const& x, int const& y) { return std::max(x, y); })), true);
		EXPECT_EQ((bool)Namespace->EmplaceFunction("max", make_callable([](long const& x, long const& y) { return std::max(x, y); })), true);
		EXPECT_EQ((bool)Namespace->EmplaceFunction("max", make_callable([](float const& x, float const& y) { return std::max(x, y); })), true);
		EXPECT_EQ((bool)Namespace->EmplaceFunction("max", make_callable([](double const& x, double const& y) { return std::max(x, y); })), true);
		EXPECT_EQ(Namespace->Cast<int>(Namespace->Call("max", { -50, 50 })), 50);
		EXPECT_EQ(Namespace->Cast<long>(Namespace->Call("max", { -50l, 50l })), 50l);
		EXPECT_EQ(Namespace->Cast<float>(Namespace->Call("max", { -50.0f, 50.0f })), 50.0f);
		EXPECT_EQ(Namespace->Cast<double>(Namespace->Call("max", { -50.0, 50.0 })), 50.0);

		EXPECT_EQ((bool)Class->FindClass("Color"), true);
		EXPECT_EQ((bool)Class->FindClass("UI::Color"), true);
		EXPECT_EQ((bool)Class->FindClass("::UI::Color::"), true);
		EXPECT_EQ((bool)Class->FindClass("::Color"), true);
		EXPECT_EQ((bool)Class->FindNamespace("Color"), true);
		EXPECT_EQ((bool)Class->FindNamespace("UI::Color"), true);
		EXPECT_EQ((bool)Class->FindNamespace("::UI::Color::"), true);
		EXPECT_EQ((bool)Class->FindNamespace("::Color"), true);

		EXPECT_EQ((bool)Class->FindFunction("Color", {}), true);
		EXPECT_EQ((bool)Scope2->FindFunction("::UI::Color::Color", {}), true);

		EXPECT_EQ((bool)Root->EmplaceObject("string::npos", std::string::npos, Scopes::ObjectWrapper::ObjectState::Static), true);
		EXPECT_EQ((bool)String->FindObject("npos"), true);
		EXPECT_EQ((bool)Root->FindObject("string::npos"), true);
		EXPECT_EQ((bool)String->FindObject("string::npos"), true);
		EXPECT_EQ((bool)Class->FindObject("npos"), true);
		EXPECT_EQ((bool)Scope->FindObject("npos"), true);

		EXPECT_EQ((bool)Scope->EmplaceObject("x", 100, Scopes::ObjectWrapper::ObjectState::Normal), true);
		EXPECT_EQ((bool)Scope->FindObject("x"), true); 
		EXPECT_EQ((bool)Scope->EmplaceObject("npos", 50, Scopes::ObjectWrapper::ObjectState::Constant), true);
		auto ptr = Scope->FindObject("npos");
		EXPECT_EQ((bool)ptr, true);		
		EXPECT_EQ((bool)Root->FindObject("x"), false);
		EXPECT_EQ((bool)String->FindObject("x"), false);
		EXPECT_EQ((bool)Class->FindObject("x"), false);

		EXPECT_EQ((bool)Scope2->EmplaceObject("y", 50, Scopes::ObjectWrapper::ObjectState::Constant), true);
		EXPECT_EQ((bool)Scope2->FindObject("x"), true);
		EXPECT_EQ((bool)Scope2->FindObject("y"), true);
		EXPECT_EQ((bool)Root->FindObject("y"), false);
		EXPECT_EQ((bool)String->FindObject("y"), false);
		EXPECT_EQ((bool)Class->FindObject("y"), false);
		EXPECT_EQ((bool)Scope->FindObject("y"), false);

		EXPECT_EQ(Scope2->AddUsing(Scope2->FindNamespace("string")), true);
		ptr = Scope2->FindObject("npos");
		EXPECT_EQ((bool)ptr, true);

		// this will fail to find a namespace called "String", since the closest available is called "string". 
		EXPECT_EQ((bool)Root->EmplaceObject("String::npos", std::string::npos, Scopes::ObjectWrapper::ObjectState::Static), false); 

		












		/* {
			namespace string{ ... }; 
			string::npos = 100; // appends "npos" to the string namespace

			namespace UI {
				class Color {
					{
						x = 100;
						{
							y = 50;
						}
					}
				}
			}
		} */
















		EXPECT_EQ((bool)Root->EmplaceFunction("string::replace", make_callable([]() {})), true);
		EXPECT_EQ((bool)String->FindFunction("replace", {}), true);
		EXPECT_EQ((bool)Root->FindFunction("string::replace", {}), true);


		EXPECT_EQ((bool)Root->EmplaceFunction("string::npos", make_callable([]() {})), true);
		std::shared_ptr<Any> result1;
		Proxy_Function result2;
		EXPECT_EQ(String->FindObjectOrFunction("npos", {}, result1, result2), true);
		EXPECT_EQ((bool)result1, true);
		EXPECT_EQ((bool)result2, false);





	}











	if (1) {
		std::vector<std::string_view> DebugScripts;
		if (1) {
			DebugScripts = {
				R"(
					namespace UI {
						class Color{
							double R = 0;
							double G = 0;
							double B = 0;

							string to_string() {
								return "r${ this.R.int }g${ this.G.int }b${ this.B.int }";
							};
							double lum() {
								return (this.R + this.B + this.G) / 3.0;
							};
						};
					};
					UI::Color fill; 
                    fill = UI::Color();
                    {
						fill.R = 255;
						fill.G = 0;
						fill.B = 0;
					}
					return fill.to_string;
				)",
				R"(
					namespace UI {
						class Color{
							double R = 0;
							double G = 0;
							double B = 0;

							string to_string() {
								return "r${ this.R.int }g${ this.G.int }b${ this.B.int }";
							};
							double lum() {
								return (this.R + this.B + this.G) / 3.0;
							};
						};
						class Rectangle {
							Color fill = Color();
							Color border = Color();
							Vector border_thickness = [0.0, 0.0, 0.0, 0.0];							

							string to_string() {
								return "Fill: ${ fill }, Border: ${ border } @ ${ border_thickness }"
							};
						};

					};
					UI::Rectangle rect; {
						rect.fill().R() = 255;
						rect.fill().G() = 0;
						rect.fill().B() = 0;
						rect.border_thickness()[0] = 1.0;
						rect.border_thickness()[0] = 1.1;
						rect.border_thickness()[0] = 1.2;
						rect.border_thickness()[0] = 1.3;
					}
					return rect.to_string();
				)",
				R"(
					print(ONE_HUNDRED);

					#define as_foot(x) ##x_ft
					return as_foot(100.0);
					#undef as_foot

					#define print(x) std::cout << x << std::endl
					#define ONE_HUNDRED = 100

					print(__DATE__);
					print(ONE_HUNDRED);
					print(__TIMESTAMP__);

					#undef ONE_HUNDRED
					#undef print
				
					print(ONE_HUNDRED);

					version ##__DATE__v##__VERSION__
					version #__DATE__v.##__VERSION__

					#define print(x) std::cout << #x + ": " << x << std::endl
					for (DateTime i = __DATE__; i < __DATE__ + 365_d; ++i){
						print(i);	
					}
					// #undef print

					#define I_AM_DEFINED
					#ifdef I_AM_DEFINED
						print("YAY");	
					#else
						print("THIS SHOULD NOT HAPPEN");	
					#endif
					#undef I_AM_DEFINED
					#ifdef I_AM_DEFINED
						print("THIS SHOULD NOT HAPPEN");
					#else
						print("YAY");
					#endif
				)",
				R"(
					// this is a conversion function:
					#define AddConv(a,b) tree.AddConverter<a, b>()
					#define AddConvs(a) AddConv(a, char); \
						AddConv(a, bool); \
						AddConv(a, int); \
						AddConv(a, long); \
						AddConv(a, float); \
						AddConv(a, long long); \
						AddConv(a, long double); \
						AddConv(a, double); \
						AddConv(a, unsigned int); \
						AddConv(a, unsigned long)

					AddConvs(char);
					// AddConvs(int);
					/* 
					AddConvs(float);
					*/

					#undef AddConvs
					#undef AddConv

					#if __VERSION__ >= 1
						This means __VERSION__ was greater than or equal to 1
					#else
						// this will never be executed. 
					#endif

					// To-Do: Support importing script content from other sources.
					#include "filepath"
					#include www.github.com/thing/thing2
					#include Name

					// To-Do: Compilation Warnings
					#warning This is a warning

					// To-Do: Pragmas
					#pragma region(Name)
					#pragma endregion
					#pragma warning(disable: 6123)
				)",
				R"(
					#define __WINDOWS__ 1
					#if __WINDOWS__
						#if 1_ft < 1_in
							#error "This code failed"
						#elif  1_ft > 1_in
							#define try_print(x) print(to_string( x ))
							try_print( [1,2,3,4,5] );
							return true;	
						#else
							#error "This code failed"
						#endif			
					#else
						#error "This code path is only supported on Windows"		
					#endif	
				)",
				R"(
					#define Append(Vector, Item) Vector.push_back(Item)
					Vector x;
					Append(x, 1);
					Append(x, 2);
					Append(x, 3);
					Append(x, 4);
					Append(x, 5);
					return x;
				)",
				R"(
					namespace math {
						double pi = 3.14;

						auto max(a, b) {
							return a > b ? a : b;
						};
						auto min(a, b) {
							return a <= b ? a : b;
						};
						auto range(Min, Val, Max) {
							return max(Val, min(Max, Val));
						};
						auto lerp(From, To, ZeroToOne) {
							return (From*(1.0-range(0.0, ZeroToOne, 1.0))) + To*range(0.0, ZeroToOne, 1.0);
						};
					};
					return [math::pi, math::lerp(0, 100, 0.5), math::lerp(0_ft, 1000.0_ft, 0.5), math::lerp(0.5, 1.0, 0.5)];					
				)",
				R"(
					class Color{
						double R = 128;
						double G = 128;
						double B = 128;

						string to_string() {
							return "r${ this.R.int }g${ this.G.int }b${ this.B.int }";
						};
						double lum() {
							return (this.R + this.B + this.G) / 3.0;
						};
					};
					var& out := Color(); 
					out.R = 256.0;
					return [out.to_string, out.Type.to_string, lum(out)];
				)",
	            R"(
					class Color{
						double R = 128;
						double G = 128;
						double B = 128;

						string to_string() {
							return "r${ this.R.int }g${ this.G.int }b${ this.B.int }";
						};
						double lum() {
							return (this.R + this.B + this.G) / 3.0;
						};
					};
					Color out; 
					out.R = 256.0;
					return [out.to_string, out.Type.to_string, lum(out)];
				)",
				R"(
					"${ "100" }";
				)",
				R"(
					int x = 100;
					auto lambda := [](){ 100; };
					return "100 == ${ "100" } == ${ [ 100 ] } == ${ [ "100":100, 100:"100" ] } == ${ x } == ${ lambda() }";
				)",
				R"(
					float x = 0;
					float& y := x;
					float& z = x;
					float w = x;
					auto Lambda = [](float x, float& y, float const& z, double const& w, A) {
						++x;
						y += 5;
						print("value=${ x }, name=${ x.Type.name }, const=${ x.Type.is_const } ref=${ x.Type.is_ref }");
						print("value=${ y }, name=${ y.Type.name }, const=${ y.Type.is_const } ref=${ y.Type.is_ref }");
						print("value=${ z }, name=${ z.Type.name }, const=${ z.Type.is_const } ref=${ z.Type.is_ref }");
						print("value=${ w }, name=${ w.Type.name }, const=${ w.Type.is_const } ref=${ w.Type.is_ref }");
						print("value=${ A }, name=${ A.Type.name }, const=${ A.Type.is_const } ref=${ A.Type.is_ref }");
					};
					Lambda(x, x, x, x, x);
					return [x,y,z,w];
				)",
				R"(
					float x = 0;
					float& y := x;				
					float& z = x;
					float w = x;
					void Lambda(float x, float& y, float const& z, double const& w, A) {
						++x;
						y += 5;
						print("value=${ x }, name=${ x.Type.name }, const=${ x.Type.is_const } ref=${ x.Type.is_ref }");
						print("value=${ y }, name=${ y.Type.name }, const=${ y.Type.is_const } ref=${ y.Type.is_ref }");
						print("value=${ z }, name=${ z.Type.name }, const=${ z.Type.is_const } ref=${ z.Type.is_ref }");
						print("value=${ w }, name=${ w.Type.name }, const=${ w.Type.is_const } ref=${ w.Type.is_ref }");
						print("value=${ A }, name=${ A.Type.name }, const=${ A.Type.is_const } ref=${ A.Type.is_ref }");
					};
					Lambda(x, x, x, x, x);
					return [x,y,z,w];
				)",
				R"(
					var x;
				)",
				R"(
					int x;
				)",
				R"(
					int x = 10;
				)",
				R"(
					var& x = 10;
				)",
				R"(
					int& x = 10;
				)",
				R"(
					return 10_ft;
				)",
				R"(
					Units::meter x;
				)",
				R"(
					Var String() { 
						return "Static Function";
					};				
					auto String = [](){
						return "Lambda Function"
					};
	
					// At this point, it is ambiguous who "String" is. 
					// It is resolved in preference of objects to functions, in order of reverse scope search.

					return String();
				)",
				R"(
					auto String = [](){
						return "Lambda Function"
					};

					{					
						Var String() { 
							return "Static Function";
						};
						return String();
					}	
				)",
				R"(
					Units::foot FunctionName(float x, float y) { 
						return x + y;
					};
					Vector FunctionName(a,b, c,d,e,f,g, h,i,j,k,l,m,n, o) { // these may be typed or un-typed. If untyped, the name must not match another existing type.
						return [a,b,c,d,e,f,g,h, i, j, k, l, m, n, o];
					};
					return FunctionName(1,2); // will fail since the TEST namespace is dead and deleted by now	
				)",
				R"(
					if (int x = 50_ft){
						return x;
					}else{
						return false;
					}
				)",
				R"(
					namespace TEST {
						int DoWork(){
							Units::meter x;
							x.double;
							x++;
							x*x;
							var& y = x % x ^ x.double;						
						};
						void DoWork(int x){
							x.double;
							x++;
							x*x;
							var& y = x * x ^ x.double;
						};
						void DoWork(int x, int y){
							x.double;
							x++;
							x*x;
							x ^ (x % x).double;
						};
					};
					for (int i = 0; i < 1000; ++i) {
						TEST::DoWork();
						TEST::DoWork(i.double);
						TEST::DoWork(i.int, Var(i));
					};
				)",
				R"(
					auto lambda = []() async -> int {
						return 5;
					};
					return lambda().Await();
				)",
				R"(
					auto lambda = []() async -> int {
						Sleep(2_s); // ...sleeping...
						return 50;
					};
					auto job = lambda();
					while (!job.Done()){ /* ...waiting... */ }
					return job.Await();
				)",
				R"(
					auto lambda = []() async {
						Sleep(2_s);
						return 55;
					};
					return lambda().Await();
				)",
				R"(
					auto lambda = []() async {
						Sleep(2_s);
						return;
					};
					return lambda().Await();
				)",
				R"(
					auto lambda;
					if (1) {
						auto x = 10;
						lambda := [x]() async {
							Sleep(2_s);
							return x + 1;
						};
					}
					return lambda().Await();
				)",
				R"(
					var& x = "print( \"PRINT ME FROM A JIT COMPILATION\" );";
					evaluate( x ); // compiles the code into AST nodes, and then evaluates those nodes.
				)",
				R"(
					auto& X = "I " + "am " + "a " + "constant " + "expression."; // this will be reduced to a constant string value at compile-time
					int j = 0;
					for (int i = 0; i < 1000000; ++i) {
						evaluate( "j++;" ); // compiles the code into AST nodes, and then evaluates those nodes...
						// If the script text for those AST nodes does not change, then the nodes are not reevaluated every call and can be re-used. 
					}
					return j;
				)",
				R"(
					var j = 0_ft;
					parallel_for (int i = 0; 1415){
						evaluate( "j += ${ i }_ft" ); // compiles the code into AST nodes, and then evaluates those nodes.
					}
					return j;
				)",
				R"(
					var& x = 10;
					var& out;
					if (1){
						namespace TEST { // resumes adding content to the same namespace called "TEST" if found, otherwise starts a new one
							// valid static objects should reduce down to Assign_Retroactively:
							int valid_static_object1;
							int valid_static_object5 = 100;

							// functions are easy to declare and use:
							Units::foot FunctionName() { 
								return 0;
							};
						};
						namespace TEST { // resumes adding content to the same namespace called "TEST" if found, otherwise starts a new one
							Units::foot FunctionName(float x) { 
								return x;
							};
						}
						namespace TEST { // resumes adding content to the same namespace called "TEST" if found, otherwise starts a new one
							Units::foot FunctionName(float x, float y) { 
								return x + y;
							};
							Vector FunctionName(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) { 
								return [a,b,c,d,e,f,g,h,i,j,k,l,m,n,o];
							};

							// invalid static objects may reduce down to (for example) Var_Decl and should be rejected.
							// auto invalid_static_object; << this would throw
						};

						out := [ TEST::FunctionName(), TEST::FunctionName(5), TEST::FunctionName(5, 5) ];
					}	

					try{
						return TEST::FunctionName(); // will fail since the TEST namespace is dead and deleted by now
					}
					catch(e) {
						return (out[2] + 5)_m;
					}	
				)",
				R"(
					if (int x = 50_ft){
						return x;
					}else{
						return false;
					}
				)",
				R"(
					var& lambda;
					int x = 100_mm;
					{
						lambda := [x](Vector y){
							y.push_back(x);
							return y;
						};
					}
					return lambda([]);
				)",
				R"(
					int x = 0; print(x); /* BLOCK1 */ {
						double x = 1; print(x); /* BLOCK2 */ {
							float x = 2; print(x); /* BLOCK3 */ {
								Units::foot x = 3; print(x); /* BLOCK4 */ {
									Units::meter x = 4; print(x); 
									/* BLOCK5 */ {}
									print(x); 
								}
								print(x); 
							}
							print(x); 
						}
						print(x); 
					}
					print(x); 
				)",
				R"(
					(75_gal / 2_d).gallon_per_minute
				)",
				R"(
					(75_gal / 2_d)_gpm
				)",
				R"(
					50_m = 50
				)",
				R"(
					var& x = 250_m; x = 50; return x;
				)",
				R"(
					{ Units::meter x = 50; x += 50_ft; return x; }
				)",
				R"(
					{
						Vector x; 
						x.push_back(x.size().double); 					
						x.push_back(x[x.size-1]); 
						x.push_back("TESTING"); 
						x.push_back(Units::meter(100)); 
						return x;
					}
				)",
				R"(
					[
						__LINE__, 
						__LINE__, 
						__LINE__, 
						__LINE__, 
						__LINE__, 
						__LINE__
					];	
				)",
				R"(
					{
						Vector x := [10, 20, 30, 40, 50, 60, 70, 80, 90, 100];
						Map y := [ "a":10, 20:x, string("TEST"):30.0f ];
						print(to_string(y));
						return y;
					}
				)",
				R"(
					if (true) {
						print("10");
					}else{
						print(20);
					}
				)",
				R"(
					if (true) {
						print("10");
					}
					else if (true) {
						print(20);
					}
					else if (true) {
						print(Vector("item1", 0.05l, ["a":10]));
					}
					else{

					}
				)",
				R"(
					var i; i := 100.0_s; ++i;
				)",
				R"(
					for (int i = 0; i < 100; i++){
						!i;
						return ~i;
					}
				)",
				R"(
					for (int& i : [0, 20, 30, 50]){
						!i;
						return ~i;
					}
				)",
				R"(
					parallel_for (int i = 0 ; 100){
						!i;
						return ~i;
					}
				)",
				R"(
					parallel_for (int& i : [0, 20, 30, 50]){
						!i;
						return ~i;
					}
				)",
				R"(
					int i = 0;
					while (true){
						++i++;
						break;
					}
					return i;
				)",
				R"(
					var& x = "TEST";
					switch (x) {
						case "T": { break; }
						case "TE": { break; }
						case "TES": { break; }
						case "TEST": { return true; }
						default: {  }
					};
					return false;
				)",
				R"(
					switch (12_in) {
						case 0_ft: { return Units::meter(0_ft); }
						case 1_ft: { return Units::meter(1_ft); }
						case 2_ft: { return Units::meter(2_ft); }
						case 3_ft: { return Units::meter(3_ft); }
						default: { return Units::meter(12_in);  }
					};
				)",
				R"(
					Units::meter x = 100_ft;
					switch (x) {
						case 1: { break; }
						case 2: { break; }
						case 3: { break; }					
						case [1,2,3,4]: { break; }
						case ([1,2,3,4]): { break; }
						case (["1":1]): { break; }
						case ("TEST"): { break; }
						case ("TESTING"): { break; }
						case (1_ft): { break; }
						case (1_ft + 10_ft): { break; }
						case (100_ft): { print("This makes sense."); break; }
						case (x): { print("This is legal?!"); break; }
						default: { print("Guess it failed!"); break; }
					};

					return false;
				)",
				R"(
					int i = 0;
					try{
						i++;
					}
					//catch (exception e) { /* catch an exception */
					//}
					catch (int e) { // catch an int
					}
					catch (e) { // catch anything as "e"
					}
					catch (...) { // catch anything
					}
					catch { // catch anything
					}
					finally { // do after the try, and catch, regardless of whether an error occured.  
						return i;
					}
				)",
				R"(
					float x = 0;
					var& lambda := [x]() { 
						return 1_ft + x + 1_ft;
					};
					return lambda();
				)",
				R"(
					var x := int(50);
					var& lambda := [](int x) { 
						return 1_ft + x + 1_ft;
					};
					return lambda(x);
				)",
				R"(
					var x := 150_ft * 50_ft;
					var& lambda := [](x) { 
						return x;
					};
					return lambda(x);
				)",
				R"(
					var x := 0_ft;
					var& lambda := [](x) { 
						x += (Units::foot)100;
						return;
					};
					lambda(x);
					return x;
				)",
				R"(
					var x := 0_ft;
					var& lambda := (x){ x = 100; };
					lambda(x);
					return x;
				)",
				R"(
					var& lambda := () -> Units::meter { return 50.0f; }; // specifies the lambda will return a meter, regardless of the output.
					return lambda();
				)",
				R"(
					var& lambda := () -> void { 50.0f; return; }; // specifies the lambda will return void. This lambda will no longer support return statements with values. 
					lambda(); // cannot return a void obj.
				)",
				R"(
					int x = 100;
					auto lambda := [](){ 100; };
					return "100 == ${ "100" } == ${ [ 100 ] } == ${ [ "100":100, 100:"100" ] } == ${ x } == ${ lambda() }";
				)",
				R"(
					Vector thingy = [ 55_ft ];				
					for (auto x = 0; x < 60; x++){
						// do some maths real fast...
						thingy[0] += ( x )_ft;
						for (int y = x; y >= 0; --y){
							--thingy[0];
						}
						while (true){ break; }
						DateTime::Now();
					}
					return thingy[0];
				)",
				R"(
					Units::value y;
					for (Units::value i = 0; i < 1000000; ++i) {
						++y;
					}
					return y;
				)",
				R"(
					Units::meter y;
					parallel_for (int i = 0 ; 1000000) {
						++y;
					}
					return y;
				)",
				R"(
					Units::meter y;
					Units::foot x;
					auto Lambda := [x](Units::meter y) { 
						++y;
						return x; // this is optimized to "x;", which avoids the throw
						++y; // these are skipped
						return x;  // these are skipped
					};
					parallel_for (int i = 0 ; 1000000) {
						Lambda(y);
					}
					return y;
				)",
				R"(
					parallel_for (int i = 0 ; 1000000) {
						Units::meter x;
						x.double;
						x++;
						x*x;
						var& y = x*x*x;
					}
				)",
				R"(
					parallel_for (int i = 0 ; 1000) {
						parallel_for (int j = 0 ; 1000) {
							Units::meter x;
							x.double;
							x++;
							x*x;
							var& y = x*x*x;
						}
					}
				)",
				R"(
					parallel_for (int i = 0 ; 100) {
						parallel_for (int j = 0 ; 100) {
							parallel_for (int k = 0 ; 100) {
								Units::meter x;
								x.double;
								x++;
								x*x;
								var& y = x*x*x;
							}
						}
					}
				)",
				R"(
					parallel_for (int i = 0 ; 10) {
						parallel_for (int j = 0 ; 10) {
							parallel_for (int k = 0 ; 10) {
								parallel_for (int L = 0 ; 10) {
									parallel_for (int M = 0 ; 10) {
										parallel_for (int N = 0 ; 10) {
											Units::meter x;
											x.double;
											x++;
											x*x;
											var& y = x*x*x;
										}
									}
								}
							}
						}
					}
				)",
				R"(
					int loopCount = 0;
					while (true){ // loopCount < 10000
						if ((++loopCount % 1000) == 0) {
							print("Loop ${ loopCount }");
						}
						auto Lambda := [](Units::meter y) { 
							++y;
						};
						parallel_for (int x = 0; 1000) {
							Units::meter y = x;
							Lambda(y);
							to_string( [ x , y ] );
						}
					}
				)"
			};
		}
	
		auto globalScope = StartScope();
		int scriptN = 0;
		for (auto& script : DebugScripts) {
			GoodLang::Engine::Compiler::Preprocessor::PreprocessorState state;
			auto Interpretted = GoodLang::Engine::Compiler::Preprocessor().Parse(script);
			Interpretted->GenerateExpandedCode(state);
			auto expandedScript = state.GetFinalScript();
			
			print(ToString("\nScript #") + ToString(scriptN++) + ToString(": "));

			auto this_scope = StartScope(globalScope);
			try {				
				auto parsed_result = GoodLang::Engine::Compiler::Interpreter::Parser().Parse(expandedScript, this_scope);
				Stopwatch sw;
				sw.Start();
				auto result = parsed_result.first->eval(parsed_result.second);
				sw.Stop();
				print(ToString(parsed_result));
				print(GoodLang::printf("\t (%f sec) -> \t", (float)sw.Seconds_Passed()) + ToString(result));
			}
			catch (GoodLang::Engine::exception::eval_error& e) {
				if (e.start_position.position >= 0 && e.start_position.position < expandedScript.length()) {
					print("####");
					print(expandedScript.substr(0, e.start_position.position));
					print(GoodLang::ToString(" __ ||| __ ") + e.reason);
					print(expandedScript.substr(e.start_position.position));
					print("####");
				}
				else {
					print("####");
					print(e.what());
					print("####");
				}
			}
			catch (std::exception& e) {
				print("####");
				print(e.what());
				print("####");
			}
		}
	}

	return 0;
};
#undef EXPECT_EQ
#undef EXPECT_NE