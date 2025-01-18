#pragma once

#include "Fibers.h"
#include "Actions2.h"

namespace GoodLang {
	class Scope;
	class Namespace;
	class Class;
	class Global;

	class Scope {
	public:
		friend class Namespace;
		friend class Class;
		friend class Global;

		Scope(std::shared_ptr<Scope> const& parent)
			: p_UniqueName(" _ _ _ _ _ _")
			, p_self()
			, p_parent(parent)
			, p_namespace()
			, p_library()
			, p_using()
		{
			static auto randN{ [](double min, double max) -> double { return (((double)std::rand() / (double)RAND_MAX) * (max - min)) + min; } };
			for (int i = 0; i < 12; i++) {
				if (i < 2)
					p_UniqueName[i] = (char)(int)randN((int)('A'), (int)('Z'));
				else if (i < 6)
					p_UniqueName[i] = (char)(int)randN((int)('0'), (int)('9'));
				else if (i < 8)
					p_UniqueName[i] = (char)(int)randN((int)('A'), (int)('Z'));
				else if (i < 12)
					p_UniqueName[i] = (char)(int)randN((int)('0'), (int)('9'));
			};

			if (auto p = p_parent.lock()) { p_namespace = p->GetNamespaceImpl(); }
			else { p_namespace = std::dynamic_pointer_cast<Namespace>(p_self.lock()); }

			if (auto p = p_parent.lock()) { p_library = p->GetLibraryImpl(); }
			else { p_library = std::dynamic_pointer_cast<Global>(p_self.lock()); }

			qualifiedNamespaceWithQualifiers = GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = GetQualifiedNamespaceImpl();
		};
		Scope(std::shared_ptr<Namespace> const& parent) : Scope(std::dynamic_pointer_cast<Scope>(parent)) {};
		Scope(std::shared_ptr<Class> const& parent) : Scope(std::dynamic_pointer_cast<Scope>(parent)) {};
		Scope(std::shared_ptr<Global> const& parent) : Scope(std::dynamic_pointer_cast<Scope>(parent)) {};

		Scope(Scope const&) = default;
		Scope(Scope&&) = default;
		Scope& operator=(Scope const&) = default;
		Scope& operator=(Scope&&) = default;
		virtual ~Scope() = default;

	private:
		std::string // randomly generated, truly unique name. 
			p_UniqueName;
	public:
		class Hasher {
		public:
			Hasher() = default;
			~Hasher() = default;
			Hasher(Hasher const&) = default;
			Hasher(Hasher&&) = default;
			Hasher& operator=(Hasher const&) = default;
			Hasher& operator=(Hasher&&) = default;

			size_t operator()(std::weak_ptr<Scope> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(ptr.lock());
			};
			size_t operator()(std::weak_ptr<Namespace> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(ptr.lock()));
			};
			size_t operator()(std::weak_ptr<Class> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(ptr.lock()));
			};
			size_t operator()(std::weak_ptr<Global> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(ptr.lock()));
			};
			size_t operator()(std::weak_ptr<TypeConverter> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<TypeConverter>>()(ptr.lock());
			};

			size_t operator()(std::shared_ptr<Scope> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(p);
			};
			size_t operator()(std::shared_ptr<Namespace> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(p));
			};
			size_t operator()(std::shared_ptr<Class> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(p));
			};
			size_t operator()(std::shared_ptr<Global> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(p));
			};
		};

	private:
		std::string GetQualifiedNamespaceImpl(bool GetUniqueQualifier = false) const {
			std::string path = "::";

			auto parent = p_parent.lock();

			if (!parent) {
				path = "::";
			}
			else {
				auto name{ GetName() };
				if (GetUniqueQualifier) {
					if (name == "") {
						name = p_UniqueName;
					}
					else {
						name = p_UniqueName + "_" + name;
					}
				}
				if (!name.empty()) {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier) + name + "::";
				}
				else {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier);
				}
			}

			while (path.find("::::") != std::string::npos) {
				size_t start_pos = 0;
				while ((start_pos = path.find("::::", start_pos)) != std::string::npos) {
					path = path.replace(start_pos, 4, "::");
					start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}

			return path;
		};

	private:
		std::weak_ptr<Scope> // shared_pointer to itself. MUST be set immediately after creating the Scope/Class/Namespace/Global.
			p_self{};
		std::string
			qualifiedNamespaceWithQualifiers{};
		std::string
			qualifiedNamespaceWithoutQualifiers{};

	public:
		void SetSelf(std::shared_ptr<Scope>& p) { p_self = p; };
		virtual bool IsClass() const { return false; };
		virtual bool IsNamespace() const { return false; };
		virtual std::string GetName() const { return ""; };
		std::string const& GetQualifiedNamespace(bool GetUniqueQualifier = false) const {
			if (GetUniqueQualifier) {
				return qualifiedNamespaceWithQualifiers;
			}
			else {
				return qualifiedNamespaceWithoutQualifiers;
			}
		};

	public: // private:
		std::weak_ptr<Scope> // parent scope, for navigation. Could be anything, or null.
			p_parent{};
		std::weak_ptr<Namespace> // parent's parent's ... parent's scope. The logical result of asking for p_parent on repeat until you get the first Namespace type. 
			p_namespace{};
		// if Namespace or Class or Global, returns self. Otherwise, returns the parent's Namespace. 
		std::weak_ptr<Namespace> GetNamespaceImpl() const {
			if (auto p = std::dynamic_pointer_cast<Namespace>(p_self.lock())) {
				return p;
			}
			else {
				return p_namespace;
			}
		};

	public:
		// if Namespace or Class or Global, returns self. Otherwise, returns the parent's Namespace. 
		std::shared_ptr<Namespace> GetNamespace() const { return p_namespace.lock(); };

	private:
		fibers::containers::Map<std::string, std::shared_ptr<Any>>
			p_objects; // scopes of all types may declare objects. Namespace objects may be global objects, but still. 

	public:
		// try and find the object with the requested key.
		std::shared_ptr<Any> GetObj(std::string const& name) const {
			return p_objects.at_or(name, nullptr);
		};
		// Returns true if successful. Returns false is replaceIfExisting==false and the object already existed on the Scope.
		bool AddObj(std::string const& name, std::shared_ptr<Any> const& obj, bool replaceIfExisting = true) {
			return p_objects.emplace(name, obj, replaceIfExisting);
		};
		// Returns true if successful.
		bool EraseObj(std::string const& name) {
			return p_objects.erase(name);
		};
		// Returns true if successful.
		bool EraseObj(std::shared_ptr<Any> const& Obj) {
			std::string key;
			bool doErasure = false;
			for (auto& obj : p_objects) {
				if (obj && (obj->second == Obj)) {
					key = obj->first;
					doErasure = true;
					break;
				}
			}
			if (doErasure) return EraseObj(key);
			else return false;
		};

	private:
		std::weak_ptr<Global> // parent's parent's ... parent's scope. The logical result of asking for p_parent on repeat until you get the end. 
			p_library{};
		// if Global, returns self. Otherwise, returns the parent's Library. 
		std::weak_ptr<Global> GetLibraryImpl() const {
			if (auto p = std::dynamic_pointer_cast<Global>(p_self.lock())) {
				return p;
			}
			else {
				return p_library;
			}
		};

	public:
		// if Global, returns self. Otherwise, returns the parent's Library. 
		std::shared_ptr<Global> GetLibrary() const {
			if (auto p = p_library.lock())
				return p;
			else if (auto p = std::dynamic_pointer_cast<Global>(p_self.lock()))
				return p;
			else
				return nullptr;
		};

	public:
		fibers::containers::Map< size_t, std::weak_ptr<Namespace>> // allows this scope to use the children of other scopes as if they were their own.
			p_using;
		// the Library should know about our "using" list
		virtual bool RecordUsing(std::shared_ptr<Namespace> ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->RecordUsing(ptr, overrideIfExists);
			}
			return false;
		};

	public:
		// allows this scope to use the children of other scopes as if they were their own.
		bool AddUsing(std::weak_ptr<Namespace> namespacePtr) {
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

	public: // private:
		fibers::containers::Map<std::string,
			std::shared_ptr<
			fibers::containers::Map<size_t, std::shared_ptr<Namespace>>
			>
		> // children namespaces - may be classes or namespaces. By this design, imported namespaces may be "unloaded" on scope unloading, which is on purpose.
			p_children;
		// the Library should know about our "Class" list
		virtual bool RecordClass(std::shared_ptr<Class> ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->RecordClass(ptr, overrideIfExists);
			}
			return false;
		};

	public:
		bool AddChild(std::shared_ptr<Namespace> NamespacePtr) {
			if (auto p = std::dynamic_pointer_cast<Scope>(NamespacePtr)) {
				auto name = p->GetName();
				auto ptr = p_children.get_or_insert(name, std::make_shared<fibers::containers::Map<size_t, std::shared_ptr<Namespace>>>());
				if (ptr->emplace(Hasher()(NamespacePtr), NamespacePtr)) {
					if (p->IsClass()) {
						(void)RecordClass(std::dynamic_pointer_cast<Class>(p));
					}
					return true;
				}
			}
			return false;
		};

	private:
		virtual void RemoveStaleReferences() {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_using.erase(toRemove);
				}
				else {
					break;
				}
			}
		};

		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
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
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
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
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
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
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};


		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
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
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
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
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
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
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

	protected:
		virtual std::weak_ptr<Type_Info> GetClassType() const { return user_type_shared<void>(); };

	public:
		virtual bool AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists = true) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->AddFunction(name, function, overrideIfAlreadyExists);
			}
			else {
				return false;
			}
		};

	public:
		virtual std::shared_ptr< Functions > GetFunctions() const {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunctions();
			}
			else {
				return nullptr;
			}
		};
		virtual std::shared_ptr< Functions::FunctionSort > GetFunctions(std::string const& name) const {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunctions(name);
			}
			else {
				return nullptr;
			}
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunction(name, params);
			}
			else {
				return {};
			}
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params, TypeConverter& tree) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunction(name, params, tree);
			}
			else {
				return {};
			}
		};

	public:
		std::shared_ptr<Scope> FindNearestScopeWhere(std::function<bool(std::shared_ptr<Scope> const&)> const& func) const {
			std::shared_ptr<Scope> out;
			if (TryFindNearestScopeWhere(out, func)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		std::shared_ptr<Namespace> FindNearestNamespaceWhere(std::function<bool(std::shared_ptr<Namespace> const&)> const& func) const {
			std::shared_ptr<Namespace> out;
			if (TryFindNearestNamespaceWhere(out, func)) {
				return out;
			}
			else {
				return nullptr;
			}
		};

	public:
#define useCachedData
#ifdef useCachedData
		inline static void hash_combine(std::size_t& seed) { };
		template <typename T, typename... Rest>
		inline static void hash_combine(std::size_t& seed, T&& v, Rest &&... rest) {
			if constexpr (std::is_same_v<typename std::decay_t<T>, size_t>) {
				seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			else {
				std::hash<T> hasher{};
				seed ^= hasher(std::forward<T>(v)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			hash_combine(seed, std::forward<Rest>(rest)...);
		};
		template <typename T, typename... Rest>
		inline static void hash_combine(std::size_t& seed, T const& v, Rest const&... rest) {
			if constexpr (std::is_same_v<typename std::decay_t<T>, size_t>) {
				seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			else {
				std::hash<T> hasher{};
				seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			hash_combine(seed, rest...);
		};
	private:
		using CacheContainer = std::pair < std::shared_mutex, concurrency::concurrent_unordered_map<size_t, std::weak_ptr<void>>>;
		using TemplatedCacheContainer = std::pair<std::shared_mutex, concurrency::concurrent_unordered_map<size_t, std::shared_ptr<CacheContainer>>>; // organizes multiple caches for several purposes...
		using VersionedCacheContainer = std::pair<std::shared_mutex, concurrency::concurrent_unordered_map<size_t, std::shared_ptr<TemplatedCacheContainer>>>; // use this mutex to delete entire caches once the version is out-of-date
		std::shared_ptr<VersionedCacheContainer>
			SearchCache{ std::make_shared<VersionedCacheContainer>() };

		std::shared_ptr<TemplatedCacheContainer> GetVersionedCacheContainer(size_t version)const {
			while (SearchCache) {
				// test if exists
				if (1) {
					auto locked{ std::shared_lock(SearchCache->first) };
					auto f = SearchCache->second.find(version);
					if (f != SearchCache->second.end()) {
						return f->second;
					}
				}

				// didn't exist yet
				if (1) {
					auto locked{ std::scoped_lock(SearchCache->first) };
					auto f = SearchCache->second.find(version);
					if (f != SearchCache->second.end()) {
						return f->second;
					}
					else {
						SearchCache->second.insert(std::pair<size_t, std::shared_ptr<TemplatedCacheContainer>>{ version, std::make_shared<TemplatedCacheContainer>() });
					}
				}
			}
			return nullptr;
		};
		template<size_t CacheID> std::shared_ptr<CacheContainer> GetCacheContainer(size_t version)const {
			if (auto version_container = GetVersionedCacheContainer(version)) {
				// test if exists
				if (1) {
					auto locked{ std::shared_lock(version_container->first) };
					auto f = version_container->second.find(CacheID);
					if (f != version_container->second.end()) {
						return f->second;
					}
				}

				// didn't exist yet
				if (1) {
					auto locked{ std::scoped_lock(version_container->first) };
					auto f = version_container->second.find(CacheID);
					if (f != version_container->second.end()) {
						return f->second;
					}
					else {
						version_container->second.insert(std::pair<size_t, std::shared_ptr<CacheContainer>>{ CacheID, std::make_shared<CacheContainer>() });
					}
				}
			}
			return nullptr;
		};
		template<size_t CacheID, typename T, typename... Rest> bool TryGetCached(size_t version, std::shared_ptr<T>& out, Rest const&... rest) const {
			if (std::shared_ptr<CacheContainer> cache_container = GetCacheContainer<CacheID>(version)) {
				size_t hash = 0;
				hash_combine(hash, rest...);

				// test if exists
				if (1) {
					auto locked{ std::shared_lock(cache_container->first) };
					auto f = cache_container->second.find(hash);
					if (f != cache_container->second.end()) {
						out = std::static_pointer_cast<T>(f->second.lock());
						return true;
					}
				}
			}
			out = nullptr;
			return false;
		};
		template<size_t CacheID, typename... Rest> void InsertCached(size_t version, std::shared_ptr<void> const& obj, Rest const&... rest) const {
			if (std::shared_ptr<CacheContainer> cache_container = GetCacheContainer<CacheID>(version)) {
				size_t hash = 0;
				hash_combine(hash, rest...);

				// didn't exist yet
				if (1) {
					auto locked{ std::scoped_lock(cache_container->first) };
					auto f = cache_container->second.find(hash);
					if (f != cache_container->second.end()) {
						// do nothing?
						f->second = obj;
					}
					else {
						cache_container->second.insert(std::pair<size_t, std::weak_ptr<void>>{ hash, obj });
					}
				}
			}
		};
		template<size_t CacheID, typename... Rest> void InsertCachedIfNotExist(size_t version, std::shared_ptr<void> const& obj, Rest const&... rest) const {
			if (std::shared_ptr<CacheContainer> cache_container = GetCacheContainer<CacheID>(version)) {
				size_t hash = 0;
				hash_combine(hash, rest...);

				// didn't exist yet
				if (1) {
					auto locked{ std::scoped_lock(cache_container->first) };
					auto f = cache_container->second.find(hash);
					if (f != cache_container->second.end()) {
						// do nothing?
						// f->second = obj;
					}
					else {
						cache_container->second.insert(std::pair<size_t, std::weak_ptr<void>>{ hash, obj });
					}
				}
			}
		};
#endif
	public:
		std::shared_ptr<Namespace> FindNamespace(std::string QualifiedOrUnqualifiedNamespaceName) const {
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
		std::shared_ptr<Class> FindClass(std::string const& QualifiedOrUnqualifiedNamespaceName) const {
			return std::dynamic_pointer_cast<Class>(FindNamespace(QualifiedOrUnqualifiedNamespaceName));
		};

	public:
		std::shared_ptr<Class> FindClass(std::weak_ptr<Type_Info> const& typeInfo) const {
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

		std::shared_ptr<Scope> FindScopeWithObj(std::string objName) const {
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
			if (lastOfColons == std::string::npos) {
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
		std::shared_ptr<Any> FindObj(std::string objName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			objName = fixNamespace(objName);

			auto lastOfColons = objName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
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

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
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
		std::shared_ptr< Functions::FunctionSort > FindFunctions(std::string functionName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
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

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
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
		Proxy_Function FindFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
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

		virtual size_t GetTypeConverterTreeVersion() const {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->GetTypeConverterTreeVersion();
			}
			else {
				return 0;
			}
		};
		virtual std::shared_ptr<TypeConverter> GetTypeConverterTree() const {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->GetTypeConverterTree();
			}
			else {
				return nullptr;
			}
		};

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params) {
			return FindNamespaceWithFunction(functionName, params, *GetTypeConverterTree());
		};
		Proxy_Function FindFunction(std::string functionName, std::vector<Any> const& params) {
			auto tree = GetTypeConverterTree();
			return FindFunction(functionName, params, *tree);
		};

		std::vector<std::shared_ptr<Scope>> GetScopesForObjectSearch() const {
			std::vector<std::shared_ptr<Scope>> out;
			// will loop over all available scopes in the order we like
			(void)FindNearestScopeWhere([&](std::shared_ptr<Scope> const& ptr) -> bool {
				out.push_back(ptr);
				return false;
				});
			return out;
		};

	public: // private:
		bool TryFindFunctionImpl(std::string const& functionName, std::vector<Any>  const& params, std::shared_ptr<TypeConverter> const& m_conversionTree, Proxy_Function& out) const {
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

			size_t lastOfColons{ 0 };
			if ((lastOfColons = functionName.find_last_of("::")) == std::string::npos) {
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
										sort.emplace(cost+1, func);
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
									sort.emplace(cost+2, func);
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
										sort.emplace(cost+3, func);
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
											sort.emplace(cost+4, func);
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
									sort.emplace(cost+5, func);
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

	public:
		std::pair<Proxy_Function, std::shared_ptr<TypeConverter>> BuildFunction(std::string const& functionName, std::vector<Any> const& params) const {			
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
		Any CallFunction(std::string const& functionName, std::vector<Any> const& params) const {
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

				throw exception::not_found_error(Units::printf("`%s`(%s)", functionName.c_str(), params_str.c_str()));
			}
		};

		template <typename T>
		T Cast(Any const& from) const {
			auto ToType = user_type_shared<T>();
			auto FromType = from.Type();

			// see if it already matches (best option)
			if (from.IsTypeOf(ToType)) {
				return from.cast<T>();
			}

			// see if we can convert (fastest option)
			if (auto Tree = this->GetTypeConverterTree()) {
				if (Tree->Converts<T>(FromType)) {
					try {
						return Tree->Convert<T>(from);
					}
					catch (exception::bad_any_cast&) {}
				}
			}

			auto ToClass = std::dynamic_pointer_cast<Scope>(this->FindClass(user_type_shared<T>()));
			if (ToClass) {
				// see if he can convert (fastest option)
				if (auto Tree2 = ToClass->GetTypeConverterTree()) {
					if (Tree2->Converts<T>(FromType)) {
						try {
							return Tree2->Convert<T>(from);
						}
						catch(exception::bad_any_cast&){}
					}
				}

				// search for a function that can do it
				if (1) {
					std::vector<Any> params = { from };

					// call a functor from our scope
					try {
						return this->CallFunction(ToClass->GetName(), params).cast<T>();
					}
					catch (exception::not_found_error) {}

					// call a functor from their scope
					try {
						return ToClass->CallFunction(ToClass->GetName(), params).cast<T>();
					}
					catch (exception::not_found_error) {}
				}

				// Failure
				throw exception::not_found_error(ToClass->GetName());
			}

			// Failure
			throw exception::not_found_error(user_type_shared<T>().lock()->name());
		};

	};

	class Namespace : public Scope {
	public:
		friend class Class;
		friend class Global;

		Namespace(std::shared_ptr<Scope> const& parent, std::string const& Name)
			: Scope(parent)
			, p_Name(Name)
		{
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		Namespace(std::shared_ptr<Namespace> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};
		Namespace(std::shared_ptr<Class> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};
		Namespace(std::shared_ptr<Global> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};

		virtual ~Namespace() {};
		void SetSelf(std::shared_ptr<Namespace>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };
		virtual bool IsClass() const override { return false; };
		virtual bool IsNamespace() const override { return true; };
		virtual std::string GetName() const override { return p_Name; };

	private:
		std::string  // e.g. "", or "_NAMESPACE_NAME_", or "_CLASS_NAME_"
			p_Name;
	public:

	private:
		fibers::containers::Map<std::string, std::weak_ptr<Class>> // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate are not allowed.
			p_postfixes;

	public:


	private:
		std::shared_ptr<Functions> // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 
			p_functions{ std::make_shared<Functions>() };

	private:
		virtual void RemoveStaleReferences() override {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_using.erase(toRemove);
				}
				else {
					break;
				}
			}

			// p_postfixes
			while (true) {
				std::string toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_postfixes) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_postfixes.erase(toRemove);
				}
				else {
					break;
				}
			}

			// p_functions


		};
		virtual bool RecordFunction(std::string const& Name, Function const& ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Namespace>(GetLibrary())) {
				return p->RecordFunction(Name, ptr, overrideIfExists);
			}
			return false;
		};
	public:
		virtual bool AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists = true) override {

			defer(this->RecordFunction(name, function, overrideIfAlreadyExists));
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

	public:

		virtual std::shared_ptr< Functions > GetFunctions() const override {
			return p_functions;
		};
		virtual std::shared_ptr< Functions::FunctionSort > GetFunctions(std::string const& name) const override {
			static auto hasher{ std::hash<std::string>() };
			// movable shared lock
			auto locked{ std::make_shared< std::shared_lock<std::shared_mutex> >(p_functions->m_mut) };
			auto f = p_functions->m_functions.find(hasher(name));
			if (f != p_functions->m_functions.end()) {
				return std::shared_ptr< Functions::FunctionSort >(&f->second.second, [lockedCopy = locked](Functions::FunctionSort*) { if (!lockedCopy) { std::cout << "ERR" << std::endl; }; });
			}
			return nullptr;
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params, TypeConverter& tree) override {
			return p_functions->BuildMatch(name, params, tree);
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params) override {
			auto tree = GetTypeConverterTree();
			return GetFunction(name, params, *tree);
		};

	};

	class Class final : public Namespace {
	public:
		friend class Global;

		// TO-DO, throw an error when constructing Class if the inheritance list utilizes ANY built-in types.
		// The current design simply does not support built-in types as inherited objects. 
		Class(
			std::shared_ptr<Scope> const& parent
			, std::string const& Name
			, std::shared_ptr<Type_Info> type
			, std::vector<std::weak_ptr<Class>> inheritance // e.g. this class derives from another Class
		)
			: Namespace(parent, Name)
			, DerivedFrom(inheritance)
		{
			for (int i = DerivedFrom.size() - 1; i >= 0; i--) {
				if (auto InteritedClass = DerivedFrom[i].lock()) {
					if (auto InteritedClassType = InteritedClass->GetClassType().lock()) {
						if (InteritedClassType->IsBuiltInType()) { // cannot include built-in types
							DerivedFrom.erase(DerivedFrom.begin() + i); // remove this inheritance from the list, and consider throwing an error
							// currently not throwing because that would prevent calling the destructor, which is 100% a requirement to prevent a memory leak.
						}
					}
				}
			}

			for (auto& p : DerivedFrom) {
				if (auto ptr = std::dynamic_pointer_cast<Namespace>(p.lock())) {
					this->AddUsing(ptr);
				}
			}

			if ((!type) || (type->is_void())) {
				ClassType = std::dynamic_pointer_cast<Type_Info>(std::make_shared<Scripted_Type_Info>(this->p_UniqueName, Name, false, false));
			}
			else {
				ClassType = type;
			}

			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		Class(std::shared_ptr<Namespace> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Class> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Global> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};

		Class(std::shared_ptr<Scope> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>()) 
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, std::vector<std::weak_ptr<Class>>{ inheritance }) {};
		Class(std::shared_ptr<Namespace> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Class> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Global> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};

		virtual ~Class() {};
		void SetSelf(std::shared_ptr<Class>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };
		virtual bool IsClass() const override { return true; };

	private:
		std::vector<std::weak_ptr<Class>>
			DerivedFrom; // e.g. this class derives from other Classes
		std::shared_ptr<Type_Info>
			ClassType;
		fibers::containers::Map<std::string, std::pair<std::weak_ptr<Type_Info>, std::shared_ptr<Any>>>
			p_declared_member_objects; // declared member objects for the custom, scripted class which will be instantiated upon construction of the scripted class

	public:
		void ConstructMemberObjects(DynamicObject& obj) const {
			for (auto& Parent : DerivedFrom) {
				if (auto parentType = Parent.lock()) {
					parentType->ConstructMemberObjects(obj);
				}
			}

			for (auto& member_obj : p_declared_member_objects) {
				if (member_obj) {
					std::string const& memberObjectName = member_obj->first;
					if (auto memberObjectType = member_obj->second.first.lock()) {
						if (auto memberObjectClassType = this->FindClass(memberObjectType)) {
							auto& memberObjectDefaultInstance = member_obj->second.second;
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
			}
		};
		void ConstructMemberObjects(DynamicObject& obj, DynamicObject const& CopyFrom) const {
			for (auto& Parent : DerivedFrom) {
				if (auto parentType = Parent.lock()) {
					parentType->ConstructMemberObjects(obj, CopyFrom);
				}
			}

			for (auto& member_obj : p_declared_member_objects) {
				if (member_obj) {
					std::string const& memberObjectName = member_obj->first;
					if (auto memberObjectType = member_obj->second.first.lock()) {
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
								auto& memberObjectDefaultInstance = member_obj->second.second;
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
			}
		};
		// Gets the member objects of just this class
		std::map<std::string, std::weak_ptr<Type_Info>> GetMemberObjects() const {
			std::map<std::string, std::weak_ptr<Type_Info>> out;

			for (auto x : p_declared_member_objects) {
				if (x) {
					out[x->first] = x->second.first;
				}
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
				if (x) {
					out[x->first] = x->second.first;
				}
			}
			return out;
		};

	public:
		void AddDefaultConstructors() {
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

	public:
		virtual std::weak_ptr<Type_Info> GetClassType() const override { return ClassType; };
		void DeclareMemberObject(std::string const& name, std::weak_ptr<Type_Info> type, std::shared_ptr<Any> defaultValue = nullptr) {
			if (ClassType && !ClassType->IsBuiltInType()) {

				p_declared_member_objects.emplace(name, { type, defaultValue });

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

	private:
		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
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
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test my inherited namespace.
			for (auto& Parent : DerivedFrom){
				if (auto p = std::dynamic_pointer_cast<Scope>(Parent.lock())) {
					if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
						return true;
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
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
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
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
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
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
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
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
					if (!checkedSelf.count(parentPtr) >= 1) {
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
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
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
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

	};

	// Support for Units
	namespace UnitsLibrary {
		template<typename T>
		__forceinline static void AddUnit(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
			std::string UnitName = T().UnitName();
			std::shared_ptr<Class> foot_namespace{ std::make_shared<Class>(std_namespace, UnitName, user_type_shared<T>().lock(), value_namespace) };
			foot_namespace->SetSelf(foot_namespace);
			std_namespace->AddChild(foot_namespace);
			{
				// Constructors
				// foot()
				foot_namespace->AddFunction(UnitName, Function(make_callable([]() -> T { return T{}; }), false));
				// foot(value)
				foot_namespace->AddFunction(UnitName, Function(make_callable([](Units::value const& makeCopy) -> T { return makeCopy; }), false));
				// foot() = value();
				foot_namespace->AddFunction("=", Function(make_callable([](Any const& a, Units::value const& b) -> Any { T& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<T>(), user_type_shared<Units::value>() })), false));

				// value(foot)
				value_namespace->AddFunction(value_namespace->GetName(), Function(make_callable([](Any const& from) -> std::shared_ptr<Units::value> {
					return std::dynamic_pointer_cast<Units::value>(from.cast<std::shared_ptr<T>>());
					}, ParamTypes({ foot_namespace->GetClassType().lock()->MakeConstRef() })), false));
			}
		};

		class UnitsLibrary {
		public:
			static void Part1(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part2(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part3(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part4(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part5(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part6(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		};
	};

	class Global final : public Namespace {
	public:
		Global()
			: Namespace(std::shared_ptr<Scope>(), "")
		{
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		virtual ~Global() {};
		void SetSelf(std::shared_ptr<Global>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };

		std::vector<std::weak_ptr<Class>> GetClasses() const {
			std::vector<std::weak_ptr<Class>> out;
			out.reserve(Classes.size() + 16);
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

			bool DoCleanup = false;

			size_t hash{ 0 };
			for (auto& x : Classes) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.push_back(x->second);
					}
				}
			}

			if (DoCleanup) {
				//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global*>(this)->RemoveStaleReferences();
			}

			return out;
		};
		std::vector<std::weak_ptr<Namespace>> GetUsing() const {
			std::vector<std::weak_ptr<Namespace>> out;
			out.reserve(Classes.size() + 16);
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

			bool DoCleanup = false;

			size_t hash{ 0 };
			for (auto& x : Usings) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.push_back(x->second);
					}
				}
			}

			if (DoCleanup) {
				//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global*>(this)->RemoveStaleReferences();
			}

			return out;
		};

	public:

		void AddBuiltIns() {
			auto defineBuiltInType = [this](auto typeImpl, std::string const& Name) -> void {
				// make it a class
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), Name, user_type_shared<decltype(typeImpl)>().lock()));
				}
				classPtr->SetSelf(classPtr);
				this->AddChild(classPtr);

				// add converters
				classPtr->AddFunction(Name, make_callable([](bool const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](int const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](float const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](double const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](size_t const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](fibers::containers::number < double > const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](signed char const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned char const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char16_t const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char32_t const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](wchar_t const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](short const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned short const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned int const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned long const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long long const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long double const& from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));

				// Constructors
				classPtr->AddFunction(Name, make_callable([]() ->  decltype(typeImpl) { return  decltype(typeImpl){}; }));
				// classPtr->AddFunction(Name, make_callable([](decltype(typeImpl) const& makeCopy) ->  decltype(typeImpl) { return makeCopy; }));
				classPtr->AddFunction("=", make_callable(
					[](Any const& a, decltype(typeImpl) const& b) -> Any { decltype(typeImpl)& x = a.cast(); x = b; return a; }
					, ParamTypes({ user_type_shared<decltype(typeImpl)>().lock()->MakeRef(), user_type_shared<decltype(typeImpl)>().lock()->MakeConstRef() })
				));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x != y; }));
				classPtr->AddFunction("<", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x < y; }));
				classPtr->AddFunction("<=", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x <= y; }));
				classPtr->AddFunction(">", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x > y; }));
				classPtr->AddFunction(">=", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x >= y; }));
				classPtr->AddFunction("+", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x + y; }));
				classPtr->AddFunction("-", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x - y; }));
				classPtr->AddFunction("*", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x * y; }));
				classPtr->AddFunction("/", make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { if (y == 0) return std::numeric_limits<decltype(typeImpl)>::max(); else return x / y; }));
				classPtr->AddFunction("+=", make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x += y; }));
				classPtr->AddFunction("-=", make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x -= y; }));
				classPtr->AddFunction("*=", make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x *= y; }));
				classPtr->AddFunction("/=", make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { if (y == 0) x = std::numeric_limits<decltype(typeImpl)>::max(); else x /= y; }));

				// Functions
				classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::max(); }));
				classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::lowest(); }));
				classPtr->AddFunction("to_string", make_callable([](decltype(typeImpl) const& o) -> std::string { return std::to_string(o); }));
				if constexpr (fibers::utilities::is_std_hashable_v<decltype(typeImpl)>) {
					classPtr->AddFunction("to_hash", make_callable([](decltype(typeImpl) const& o) -> size_t { return std::hash<decltype(typeImpl)>()(o); }));
				}
			};

			// Built-in types
			if (1) {
				defineBuiltInType(bool{}, "bool");
				defineBuiltInType(char{}, "char");
				defineBuiltInType(int{}, "int");
				defineBuiltInType(long{}, "long");
				defineBuiltInType(float{}, "float");
				defineBuiltInType(double{}, "double");
				defineBuiltInType(size_t{}, "size_t");
				defineBuiltInType(fibers::containers::number<double>(), "Number");
				defineBuiltInType(char16_t{}, "char16_t");
				defineBuiltInType(char32_t{}, "char32_t");
				defineBuiltInType(wchar_t{}, "wchar_t");
				defineBuiltInType(short{}, "short");
				defineBuiltInType(unsigned char(0), "uchar");
				defineBuiltInType(unsigned short(0), "ushort");
				defineBuiltInType(unsigned int(0), "uint");
				defineBuiltInType(unsigned long(0), "ulong");
				defineBuiltInType(long long(0), "llong");
				defineBuiltInType(long double(), "ldouble");

				// String
				if (1) {
					// make it a class
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), "string", user_type_shared<std::string>().lock()));
					}
					classPtr->SetSelf(classPtr);
					this->AddChild(classPtr);

					// add converters
					classPtr->AddFunction("string", make_callable([](bool const& from) -> std::string { if (from) return "true"; else return "false"; }));
					classPtr->AddFunction("string", make_callable([](char const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](int const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](float const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](double const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](size_t const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](fibers::containers::number < double > const& from) -> std::string { return std::to_string(from.load()); }));
					classPtr->AddFunction("string", make_callable([](signed char const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned char const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](char16_t const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](char32_t const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](wchar_t const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](short const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned short const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned int const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned long const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long long const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long double const& from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						if (auto p = from.lock()) return p->name();
						else return user_type<void>().name();
					}));

					// Constructors
					classPtr->AddFunction("string", make_callable([]() -> std::string { return std::string{}; }));
					classPtr->AddFunction("string", make_callable([](std::string const& makeCopy) -> std::string { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<std::string>().lock()->MakeRef(), user_type_shared<std::string>().lock()->MakeConstRef() })));

					// Comparisons
					classPtr->AddFunction("==", make_callable([](std::string const& x, std::string const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", make_callable([](std::string const& x, std::string const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", make_callable([](std::string const& x, std::string const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", make_callable([](std::string const& x, std::string const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", make_callable([](std::string const& x, std::string const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", make_callable([](std::string const& x, std::string const& y) -> bool { return x >= y; }));
					classPtr->AddFunction("+", make_callable([](std::string const& x, std::string const& y) -> std::string { return x + y; }));
					classPtr->AddFunction("+=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out += b; return a; }, ParamTypes({ user_type_shared<std::string>().lock()->MakeRef(), user_type_shared<std::string>().lock()->MakeConstRef() })));

					// Functions
					classPtr->AddFunction("length", make_callable([](std::string const& a) -> size_t { return a.length(); }));
					classPtr->AddFunction("size", make_callable([](std::string const& a) -> size_t { return a.size(); }));
					classPtr->AddFunction("[]", make_callable([](std::string const& a, size_t index) -> char { return a[index]; }));
					classPtr->AddFunction("front", make_callable([](std::string const& a) -> char { return a.front(); }));
					classPtr->AddFunction("back", make_callable([](std::string const& a) -> char { return a.back(); }));
					classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind) -> size_t { return a.find(toFind); }));
					classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind, size_t startPos) -> size_t { return a.find(toFind, startPos); }));
					classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off) -> std::string { return x.substr(Off); })/*, { "input", "Off" }*/);
					classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off, size_t Count) -> std::string { return x.substr(Off, Count); })/*, { "input", "Off", "Count" }*/);
					classPtr->AddFunction("to_string", make_callable([](std::string const& o) -> std::string { return o; }));
					classPtr->AddFunction("to_hash", make_callable([](std::string const& o) -> size_t { return std::hash<std::string>()(o); }));

					// Objects or Constants
					classPtr->AddObj("npos", std::make_shared<Any>(std::string::npos));
				}

				// Types
				if (1) {
					// make it a class
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), "Type_Info", user_type_shared<std::weak_ptr<Type_Info>>().lock()));
					}
					classPtr->p_self = classPtr;
					this->AddChild(classPtr);

					// Constructors
					//classPtr->AddFunction("Type_Info", make_callable([]() -> std::weak_ptr<Type_Info> { return std::weak_ptr<Type_Info>{}; }));
					//classPtr->AddFunction("Type_Info", make_callable([](std::weak_ptr<Type_Info> const& makeCopy) -> std::weak_ptr<Type_Info> { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, std::weak_ptr<Type_Info> const& b) -> Any { std::weak_ptr<Type_Info>& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeRef(), user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeConstRef() })));

					// Comparisons
					classPtr->AddFunction("==", make_callable([](std::weak_ptr<Type_Info> const& x, std::weak_ptr<Type_Info> const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", make_callable([](std::weak_ptr<Type_Info> const& x, std::weak_ptr<Type_Info> const& y) -> bool { return x != y; }));

					// Functions
					classPtr->AddFunction("to_string", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						else if (auto p = from.lock()) return p->name();
						else return user_type<void>().name();
					}));
					classPtr->AddFunction("to_hash", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from) -> size_t {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return std::hash<std::string>()(p2->GetName());
						else if (auto p = from.lock()) return std::hash<std::string>()(p->name());
						else return std::hash<std::string>()(user_type<void>().name());
					}));
					classPtr->AddFunction("name", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						else if (auto p = from.lock()) return p->name();
						else return user_type<void>().name();
					}));
					classPtr->AddFunction("cpp_name", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
						if (auto p = from.lock()) return p->name();
						else return user_type<void>().name();
					}));
					classPtr->AddFunction("is_const", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)-> bool {
						if (auto p = from.lock()) return p->is_const();
						else return false;
					}));
					classPtr->AddFunction("is_ref", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)-> bool {
						if (auto p = from.lock()) return p->is_ref();
						else return false;
					}));
					classPtr->AddFunction("is_void", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)-> bool {
						if (auto p = from.lock()) return p->is_void();
						else return true;
					}));
					//classPtr->AddFunction("member_objects", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from) -> 
					//	fibers::containers::Map<std::string, Any> {
					//	if (auto p = self.lock()) {
					//		if (auto p2 = p->FindClass(from)) {
					//			fibers::containers::Map<std::string, Any> out;
					//			for (auto& x : p2->GetAllMemberObjects()) {
					//				out.emplace(x.first, x.second);
					//			}
					//			return out;
					//		}
					//	}
					//	return {};
					//}));


				}

				// Units
				if (1) {
					auto selfPtr = this->p_self.lock();
					auto std_namespace{ std::make_shared<Namespace>(selfPtr, "Units") };
					std_namespace->SetSelf(std_namespace);
					this->AddChild(std_namespace);

					// the "std" namespace imports the "string" namespace...
					{
						auto value_namespace{ std::make_shared<Class>(std_namespace, "value", user_type_shared<Units::value>().lock()) };
						value_namespace->SetSelf(value_namespace);
						std_namespace->AddChild(value_namespace);

						// which has the following types groups... 
						{
							// value -> double
							if (auto p = std_namespace->FindClass(user_type_shared<double>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> double { return o(); }));
							}
							// value -> float
							if (auto p = std_namespace->FindClass(user_type_shared<float>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> float { return o(); }));
							}
							// value -> int
							if (auto p = std_namespace->FindClass(user_type_shared<int>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> int { return o(); }));
							}
							// value -> string
							if (auto p = std_namespace->FindClass(user_type_shared<std::string>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> std::string { return o.ToString(); }));
							}

							value_namespace->AddFunction("abbreviation", make_callable([](Units::value const& x)->std::string {
								return x.Abbreviation();
							}));
							value_namespace->AddFunction("name", make_callable([](Units::value const& x)->std::string {
								return x.UnitName();
							}));
							value_namespace->AddFunction("to_string", make_callable([](Units::value const& x)->std::string {
								return x.ToString();
							}));
							// hashes do not exist for Units because their values are only approximations

							// Constructors
							value_namespace->AddFunction("value", make_callable([]() -> Units::value { return Units::value{}; }));
							value_namespace->AddFunction("value", make_callable([](Units::value const& makeCopy) -> Units::value { return makeCopy; }));
							value_namespace->AddFunction("value", make_callable([](int const& o)->Units::value { return o; }));
							value_namespace->AddFunction("value", make_callable([](float const& o)->Units::value { return o; }));
							value_namespace->AddFunction("value", make_callable([](double const& o)->Units::value { return o; }));
							value_namespace->AddFunction("=", make_callable(
								[](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));

							// Comparisons & operators
							value_namespace->AddFunction("==", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x == y; }));
							value_namespace->AddFunction("!=", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x != y; }));
							value_namespace->AddFunction("<", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x < y; }));
							value_namespace->AddFunction(">", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x > y; }));
							value_namespace->AddFunction("<=", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x <= y; }));
							value_namespace->AddFunction(">=", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x >= y; }));
							value_namespace->AddFunction("+", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x + y; }));
							value_namespace->AddFunction("-", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x - y; }));
							value_namespace->AddFunction("*", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x * y; }));
							value_namespace->AddFunction("/", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x / y; }));
							value_namespace->AddFunction("+=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out += b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
							value_namespace->AddFunction("-=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out -= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
							value_namespace->AddFunction("*=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out *= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
							value_namespace->AddFunction("/=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out /= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
						}

						if (1) {
							UnitsLibrary::UnitsLibrary::Part1(std_namespace, value_namespace);
							UnitsLibrary::UnitsLibrary::Part2(std_namespace, value_namespace);
							UnitsLibrary::UnitsLibrary::Part3(std_namespace, value_namespace);
							UnitsLibrary::UnitsLibrary::Part4(std_namespace, value_namespace);
							UnitsLibrary::UnitsLibrary::Part5(std_namespace, value_namespace);
							UnitsLibrary::UnitsLibrary::Part6(std_namespace, value_namespace);
						}
					}
				}

				// DateTime
				if (1) {
					using thisType = DateTime;
					std::string thisTypeName = "DateTime";

					// make it a class
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
					}
					classPtr->SetSelf(classPtr);
					this->AddChild(classPtr);
					auto thisTypeInfo = classPtr->ClassType;

					// Constructors
					classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
					classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })));
					classPtr->AddFunction(thisTypeName, make_callable([](Units::second const& from) -> thisType { return from; }));
					classPtr->AddFunction(thisTypeName, Function(make_callable([](std::string const& from) -> thisType { return thisType(from); }), true)); // explicit = cannot be used for conversion trees, and must be called directly with exact type match, e.g. DateTime("string")

					// Converters
					if (auto p = this->FindClass(user_type_shared<std::string>())) {
						p->AddFunction(p->GetName(), Function(make_callable([](thisType const& from) -> std::string { return from.c_str(); }), true)); // explicit
					}
					if (auto p = this->FindClass(user_type_shared<Units::second>())) {
						p->AddFunction(p->GetName(), make_callable([](thisType const& from) -> Units::second { return (Units::second)from; }));
					}

					// Comparisons
					classPtr->AddFunction("==", make_callable([](thisType const& x, thisType const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", make_callable([](thisType const& x, thisType const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", make_callable([](thisType const& x, thisType const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", make_callable([](thisType const& x, thisType const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", make_callable([](thisType const& x, thisType const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", make_callable([](thisType const& x, thisType const& y) -> bool { return x >= y; }));

					// Operators
					classPtr->AddFunction("+", make_callable([](thisType const& x, thisType const& y) -> thisType { return x + y; }));
					classPtr->AddFunction("-", make_callable([](thisType const& x, thisType const& y) -> thisType { return x - y; }));
					classPtr->AddFunction("*", make_callable([](thisType const& x, thisType const& y) -> thisType { return x * y; }));
					classPtr->AddFunction("/", make_callable([](thisType const& x, thisType const& y) -> thisType { if (y == 0) return (Units::second)(std::numeric_limits<Units::second>::max()); else return x / y; }));
					classPtr->AddFunction("+=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x += b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));
					classPtr->AddFunction("-=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x -= b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));
					classPtr->AddFunction("*=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x *= b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));
					classPtr->AddFunction("/=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); if (b != 0) x /= b; else x = (Units::second)(std::numeric_limits<Units::second>::max()); return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));

					// Functions
					classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<thisType>::max(); }));
					classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<thisType>::lowest(); }));
					classPtr->AddFunction("to_string", make_callable([](thisType const& o) -> std::string { return o.c_str(); }));
					classPtr->AddFunction("to_hash", make_callable([](thisType const& o) -> size_t { return (size_t)(double)(Units::millisecond)(Units::second)o; }));

					classPtr->AddFunction("Epoch", make_callable(&DateTime::Epoch));
					classPtr->AddFunction("Now", make_callable(&DateTime::Now));
					// classPtr->AddFunction("Now", make_callable([]() -> DateTime { return DateTime::Now(); }));					
					classPtr->AddFunction("tm_fractionalsec", make_callable(&DateTime::tm_fractionalsec));
					classPtr->AddFunction("tm_sec", make_callable(&DateTime::tm_sec));
					classPtr->AddFunction("tm_min", make_callable(&DateTime::tm_min));
					classPtr->AddFunction("tm_hour", make_callable(&DateTime::tm_hour));
					classPtr->AddFunction("tm_mday", make_callable(&DateTime::tm_mday));
					classPtr->AddFunction("tm_mon", make_callable(&DateTime::tm_mon));
					classPtr->AddFunction("tm_yday", make_callable(&DateTime::tm_yday));
					classPtr->AddFunction("tm_wday", make_callable(&DateTime::tm_wday));
					classPtr->AddFunction("tm_year", make_callable(&DateTime::tm_year));
					classPtr->AddFunction("load", make_callable(&DateTime::load));
					classPtr->AddFunction("getNumDaysInSameMonth", make_callable(&DateTime::getNumDaysInSameMonth));
					//classPtr->AddFunction("getNumDaysInSameMonth", make_callable([](DateTime const& dt) -> int {
					//	return DateTime::getNumDaysInSameMonth(dt);
					//}));
					classPtr->AddFunction("make_time", make_callable([]() { return DateTime::make_time(); }));
					classPtr->AddFunction("make_time", make_callable([](int year) { return DateTime::make_time(year); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month) { return DateTime::make_time(year, month); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day) { return DateTime::make_time(year, month, day); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour) { return DateTime::make_time(year, month, day, hour); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute) { return DateTime::make_time(year, month, day, hour, minute); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute, float second) { return DateTime::make_time(year, month, day, hour, minute, second); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute, float second, bool useLocalTime) { return DateTime::make_time(year, month, day, hour, minute, second, useLocalTime); }));
					classPtr->AddFunction("ToStartOfMonth", make_callable([](DateTime from) -> DateTime { return from.ToStartOfMonth(); }));
					classPtr->AddFunction("ToEndOfMonth", make_callable([](DateTime from) -> DateTime { return from.ToEndOfMonth(); }));
					classPtr->AddFunction("ToStartOfDay", make_callable([](DateTime from) -> DateTime { return from.ToStartOfDay(); }));
					classPtr->AddFunction("ToEndOfDay", make_callable([](DateTime from) -> DateTime { return from.ToEndOfDay(); }));
					classPtr->AddFunction("ToStartOfHour", make_callable([](DateTime from) -> DateTime { return from.ToStartOfHour(); }));
					classPtr->AddFunction("ToEndOfHour", make_callable([](DateTime from) -> DateTime { return from.ToEndOfHour(); }));
					classPtr->AddFunction("ToStartOfMinute", make_callable([](DateTime from) -> DateTime { return from.ToStartOfMinute(); }));
					classPtr->AddFunction("ToEndOfMinute", make_callable([](DateTime from) -> DateTime { return from.ToEndOfMinute(); }));

					// Parameters
					classPtr->AddFunction("time", make_callable(&DateTime::time));
				}

				// Var
				if (1) {
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), "Var", user_type_shared<Var>().lock()));
					}
					classPtr->p_self = classPtr;
					this->AddChild(classPtr);

					// Constructors
					// Var()
					classPtr->AddFunction("Var", make_callable([]() -> Var {
						return Var();
					})); // explicit = cannot be used for conversion trees
					// Var(Var const&)
					classPtr->AddFunction("Var", make_callable([](Var const& obj) -> Var {
						return obj;
					})); // explicit = cannot be used for conversion trees
					// Var(Any)
					classPtr->AddFunction("Var", Function(make_callable([](Any const& obj) -> Var {
						if (obj.IsTypeOf<Var>()) {
							return obj.cast<Var&>();
						}
						else {
							return Var(obj);
						}
					}), true)); // explicit = cannot be used for conversion trees
					// Var& = Var const&
					classPtr->AddFunction("=", make_callable([](Any const& a, Var const& b) -> Any {
						Var& out = a.cast(); out = b; return a;
					}, ParamTypes({ user_type_shared<Var>().lock()->MakeRef(), user_type_shared<Var>().lock()->MakeConstRef() }), user_type_shared<Var>().lock()->MakeRef()));
					// Var& = Any
					classPtr->AddFunction("=", make_callable([](Any const& a, Any const& b) -> Any {
						Var& out = a.cast(); out = Var(b); return a;
					}, ParamTypes({ user_type_shared<Var>().lock()->MakeRef(), user_type_shared<Any>() }), user_type_shared<Var>().lock()->MakeRef())); 
					// Var().get()
					classPtr->AddFunction("get", make_callable([](Var const& b) -> Any {
						return b.p_data;
					})); 
					// template func, Any = Var const&
					classPtr->AddFunction("=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any & a, Var const& b) -> Any {
						if (auto Self = self.lock()) {
							return Self->CallFunction("=", { a, b.p_data });
						}
						else {
							a = b.p_data;
							return a;
						}
					}));

					// Returns the type of the contained object. By not specifying the type, the Any is treated like a Template
					this->AddFunction("Type_Info", make_callable([](Var const& obj) -> std::weak_ptr<Type_Info> {
						return obj.p_data->Type();
					}));
					// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
					this->AddFunction("Type", make_callable([](Var const& obj) -> std::weak_ptr<Type_Info> {
						return obj.p_data->Type();
					}));
					// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
					this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](Var const& x) -> std::string {
						if (auto Self = self.lock()) {
							return Self->Cast<std::string>(Self->CallFunction("to_string", { x.p_data }));
						}
						else {
							auto name = x.p_data->TypeName();
							return Units::printf("`%s`", name.c_str());
						}
					}));
					// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
					this->AddFunction("to_hash", make_callable([self = std::weak_ptr<Class>(classPtr)](Var const& x) -> size_t {
						if (auto Self = self.lock()) {
							return Self->Cast<size_t>(Self->CallFunction("to_hash", { x.p_data }));
						}
						else {
							throw exception::not_found_error("to_hash");
						}
					}));
				}

				// Pair
				if (1) {
					using thisType = std::pair<Var, Var>;
					std::string thisTypeName = "pair";

					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
					}
					classPtr->p_self = classPtr;
					this->AddChild(classPtr);
					auto thisTypeInfo = classPtr->ClassType;

					// Constructors
					classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
					classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
					classPtr->AddFunction(thisTypeName, make_callable([](Any const& a, Any const& b) -> thisType { return thisType{ Var(a), Var(b) }; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any { 
						thisType& out = a.cast(); out = b; return a; 
					}, 
						ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })));

					// Functions
					classPtr->AddFunction("first", make_callable([](thisType & r) -> Var& { return r.first; }));
					classPtr->AddFunction("first", make_callable([](thisType const& r) -> Var const& { return r.first; }));
					classPtr->AddFunction("second", make_callable([](thisType& r) -> Var& { return r.second; }));
					classPtr->AddFunction("second", make_callable([](thisType const& r) -> Var const& { return r.second; }));

					// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
					this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->std::string {
						if (auto Self = self.lock()) {
							std::string a = Self->Cast<std::string>(Self->CallFunction("to_string", { x.first.p_data }));
							std::string b = Self->Cast<std::string>(Self->CallFunction("to_string", { x.second.p_data }));
							return Units::printf("[%s, %s]", a.c_str(), b.c_str());
						}
						else {
							throw exception::not_found_error("to_string");
						}
					}));
					// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
					this->AddFunction("to_hash", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->size_t {
						if (auto Self = self.lock()) {
							size_t a = Self->Cast<size_t>(Self->CallFunction("to_hash", { x.first.p_data }));
							size_t b = Self->Cast<size_t>(Self->CallFunction("to_hash", { x.second.p_data }));
							Scope::hash_combine(a, b);
							return a;
						}
						else {
							throw exception::not_found_error("to_hash");
						}
					}));



				}
			}

			// Built-In static, templated functions
			if (1) {
				// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
				this->AddFunction("Type_Info", make_callable([](Any const& obj) -> std::weak_ptr<Type_Info> {
					return obj.Type();
				}));
				// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
				this->AddFunction("Type", make_callable([](Any const& obj) -> std::weak_ptr<Type_Info> {
					return obj.Type();
				}));
				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_string", make_callable([](Any const& x) -> std::string {
					auto name = x.TypeName();
					return Units::printf("`%s`", name.c_str());
				}));
			}
		};


	private:
		void GetClasses(std::unordered_map<size_t, std::weak_ptr<Class>>& out) const {
			bool DoCleanup = false;
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

			size_t hash{ 0 };
			for (auto& x : Classes) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.insert({ hash, x->second });
					}
				}
			}

			if (DoCleanup) {
				const_cast<Global*>(this)->RemoveStaleReferences();
			}
		};
		void GetAllAvailableClassesImpl(
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
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						if (auto ptr = x->second.lock()) {
							if (auto p2 = ptr->GetLibrary()) {
								p2->GetAllAvailableClassesImpl(out, uniqueLibraries);
							}
						}
					}
				}
			}
			if (DoCleanup) {
				//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global*>(this)->RemoveStaleReferences();
			}
		};

	private:
		// Searches for all classes that are defined in the current and "used" libraries. 
		std::unordered_map<size_t, std::weak_ptr<Class>> GetAllAvailableClassesImpl() const {
			thread_local static std::unordered_map<size_t, std::weak_ptr<Class>> allClasses{};
			thread_local static std::unordered_map<size_t, std::weak_ptr<Scope>> uniqueList{};
			defer(allClasses.clear());
			defer(uniqueList.clear());

			GetAllAvailableClassesImpl(allClasses, uniqueList);
			return allClasses;
		};
	public:
		std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>> GetAllAvailableClasses() const {
			auto oldVersion = CachedClassListVersion.load();
			if (oldVersion != RecordVersion) {
				auto guard{ std::scoped_lock(const_cast<Global*>(this)->CachedClassListMutex) };
				if (const_cast<Global*>(this)->CachedClassListVersion.CompareExchange(oldVersion, RecordVersion)) {
					return const_cast<Global*>(this)->CachedClassList = std::make_shared<std::unordered_map<size_t, std::weak_ptr<Class>>>(GetAllAvailableClassesImpl());
				}
			}

			if (1) {
				auto guard{ std::shared_lock(const_cast<Global*>(this)->CachedClassListMutex) };
				return CachedClassList;
			}
		};
	public:
		// Creates a tree of type-converter functions using the classes found with GetAllAvailableClasses()
		void CreateTypeConverterTree(std::shared_ptr<TypeConverter>& out) const {
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

	public:
		virtual size_t GetTypeConverterTreeVersion() const override {
			return CachedTypeConverterTreeVersion.load();
		};
		virtual std::shared_ptr<TypeConverter> GetTypeConverterTree() const override {
			auto oldVersion = CachedTypeConverterTreeVersion.load();
			if (oldVersion != RecordVersion) {
				auto guard{ std::scoped_lock(const_cast<Global*>(this)->CachedTypeConverterTreeMutex) };
				if (const_cast<Global*>(this)->CachedTypeConverterTreeVersion.CompareExchange(oldVersion, RecordVersion)) {
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

	private:
		fibers::containers::Map<size_t, std::weak_ptr<Class>> // collection of all classes that are added as "children" of this library
			Classes;
		fibers::containers::Map<size_t, std::weak_ptr<Namespace>> // collection of all namespaces that are added being "used" by this library
			Usings;
		fibers::containers::Map<size_t, std::pair<std::string, std::weak_ptr<details::Proxy_Function_Base>>> // collection of all namespaces that are added being "used" by this library
			Functions;

		std::shared_ptr<TypeConverter>
			CachedTypeConverterTree{ std::make_shared<TypeConverter>() };
		fibers::containers::number<unsigned __int64>
			CachedTypeConverterTreeVersion{ 0 };
		std::shared_mutex // fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedTypeConverterTreeMutex{};

		std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>>
			CachedClassList{ std::make_shared<std::unordered_map<size_t, std::weak_ptr<Class>>>() };
		fibers::containers::number<unsigned __int64>
			CachedClassListVersion{ 0 };
		std::shared_mutex // fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedClassListMutex{};

		//fibers::containers::number<unsigned int> 
			//CleanupRequested{ 0 };
		fibers::containers::number<unsigned __int64>
			CleanupVersion{ 0 };
		fibers::containers::number<unsigned __int64>
			RecordVersion{ 0 };
		fibers::containers::number<unsigned __int64>
			LibraryVersion{ 0 };

		virtual void RemoveStaleReferences() override {
			auto oldVersion = CleanupVersion.load();
			if (oldVersion != RecordVersion) {
				if (CleanupVersion.CompareExchange(oldVersion, RecordVersion)) {
					// p_using
					while (true) {
						size_t toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_using) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove = ref->first;
									doRemoval = true;
									break;
								}
							}
						}
						if (doRemoval) {
							p_using.erase(toRemove);
						}
						else {
							break;
						}
					}

					// p_postfixes
					while (true) {
						std::string toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_postfixes) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove = ref->first;
									doRemoval = true;
									break;
								}
							}
						}
						if (doRemoval) {
							p_postfixes.erase(toRemove);
						}
						else {
							break;
						}
					}

					// Classes
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Classes) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Classes.erase(toRemove);
						toRemove.clear();
					}

					// Usings
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Usings) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Usings.erase(toRemove);
						toRemove.clear();
					}

					// Functions
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Functions) {
							if (ref) {
								if (ref->second.second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Functions.erase(toRemove);
						toRemove.clear();
					}
				}
			}
		};

		virtual bool RecordClass(std::shared_ptr<Class> ptr, bool overrideIfExists = true) override {
			if (Classes.emplace(Scope::Hasher()(ptr), ptr, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

		virtual bool RecordUsing(std::shared_ptr<Namespace> ptr, bool overrideIfExists = true) override {
			if (Usings.emplace(Scope::Hasher()(ptr), ptr, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

		virtual bool RecordFunction(std::string const& Name, Function const& ptr, bool overrideIfExists = true) override {
			if (Functions.emplace(std::hash<Proxy_Function>()(ptr.m_function), { Name, ptr.m_function }, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

	};
};