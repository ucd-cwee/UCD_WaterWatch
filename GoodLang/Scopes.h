#pragma once
#include "Foundation.h"
#include "Any"
#include "Proxy_Function.h"
#include "ThreadSafeContainers.h"
#include "Units_Base.h"
#include <unordered_set>

#define useCachedData

namespace GoodLang {
	// std::string to a collection of maps with given params. (E.g. first sorted by strings)
	class FunctionsMap {
		friend class it_state;
	public:
		using TupleType = GoodLang::Union<
			std::string, // name of object or function
			GoodLang::ParamTypes, // params may be empty
			GoodLang::Function, // function
			std::shared_ptr<GoodLang::Any>, // object
			bool // is_function (true if function, false if object)
		>;

	private:
		// Allocators allow for pointers to be made which do not need to be explicitely deleted -- they will be deleted in the future. 
		// Additionally, if we want to support deleting functions from the map, these allocators can switched to Epoch style and defer deletion until it is safe to do so. 
		GoodLang::Allocator<std::pair<GoodLang::details::flat_map<size_t, TupleType*>, long>>
			alloc1;
		GoodLang::Allocator<TupleType>
			alloc2;

	public:
		using MapType =
			std::pair <
			    GoodLang::details::flat_map<size_t, 
			        std::pair<GoodLang::details::flat_map<size_t, TupleType*>, long>*
				>,
			    long // hint
			>;

	protected:
		static constexpr size_t numV = ((int)('Z') - (int)('A') + 1) + ((int)('z') - (int)('a') + 1) + 1;
		static constexpr size_t CharToIndex(char firstChar) {
			if (firstChar >= 'a' && firstChar <= 'z') {
				return ((int)firstChar - (int)('a')) + 27;
			}
			else if (firstChar >= 'A' && firstChar <= 'Z') {
				return ((int)firstChar - (int)('A')) + 1;
			}
			else {
				return 0;
			}
		};
		std::array< MapType, numV> FirstCharToFunctionNameMap;

	public:
		// emplace a function, if not already exists, using the provided params
		TupleType* emplace(std::string_view const& name, GoodLang::ParamTypes const& params, GoodLang::Function const& func);
		// emplace a function, if not already exists
		TupleType* emplace(std::string_view const& name, GoodLang::Function const& func);
		// emplace an object, if not already exists
		TupleType* emplace(std::string_view const& name, std::shared_ptr<GoodLang::Any> const& obj);
		// Get the tuple (function, likely) that exists at this name and param types
		TupleType* at(std::string_view const& name, GoodLang::ParamTypes const& params) const;
		// Get the tuple (function, likely) that exists at this name and param types
		TupleType* operator()(std::string_view const& name, GoodLang::ParamTypes const& params) const;
		// Get the tuple (object, likely) that exists at this name and with empty params
		TupleType* at(std::string_view const& name) const;
		// Get the tuple (object, likely) that exists at this name and with empty params
		TupleType* operator()(std::string_view const& name) const;

	private:
		using value_type = TupleType;
		class it_state {
		public:
			using thisType = FunctionsMap;
			using value_type = typename thisType::value_type;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

			// data
			mutable size_t
				outtermost_index{ 0 };
			mutable size_t
				outtermost_index_max{ 0 };

			mutable typename thisType::MapType::first_type::iterator
				middle_iter{};
			mutable size_t
				middle_index{ 0 };
			mutable size_t
				middle_index_max{ 0 };

			mutable GoodLang::details::flat_map<size_t, value_type*>::iterator
				final_iter{};
			mutable size_t
				final_index{ 0 };
			mutable size_t
				final_index_max{ 0 };

			// functions
			void Initialize(thisType* ref) {
				outtermost_index_max = ref->numV;
			};
			void ToBeginning(thisType* ref) {
				outtermost_index = 0;
				while (outtermost_index < outtermost_index_max) {
					middle_iter = ref->FirstCharToFunctionNameMap[outtermost_index].first.begin();
					middle_index = 0;
					if (0 == (middle_index_max = ref->FirstCharToFunctionNameMap[outtermost_index].first.size())) {
						++outtermost_index;
					}
					else {
						// we got one
						while (middle_index < middle_index_max) {
							final_iter = (*middle_iter->second)->first.begin();
							final_index = 0;
							if (0 == (final_index_max = (*middle_iter->second)->first.size())) {
								++middle_iter;
								++middle_index;
							}
							else {
								// we got one
								return;
							}
						}
					}
				}
			};
			void ToEnd(thisType* ref) {
				final_index_max = final_index = middle_index_max = middle_index = outtermost_index = outtermost_index_max = 0;
			};
			void Next(thisType* ref) {
				while (outtermost_index < outtermost_index_max) {
					if (final_index < (final_index_max - 1)) {
						++final_iter;
						++final_index;
						return;
					}
					else {
						if (middle_index < (middle_index_max - 1)) {
							++middle_iter;
							++middle_index;

							while (middle_index < middle_index_max) {
								final_iter = (*middle_iter->second)->first.begin();
								final_index = 0;
								if (0 == (final_index_max = (*middle_iter->second)->first.size())) {
									++middle_iter;
									++middle_index;
								}
								else {
									// we got one
									return;
								}
							}
						}
						else {
							++outtermost_index;
							while (outtermost_index < outtermost_index_max) {
								middle_iter = ref->FirstCharToFunctionNameMap[outtermost_index].first.begin();
								middle_index = 0;
								if (0 == (middle_index_max = ref->FirstCharToFunctionNameMap[outtermost_index].first.size())) {
									++outtermost_index;
								}
								else {
									// we got one
									while (middle_index < middle_index_max) {
										final_iter = (*middle_iter->second)->first.begin();
										final_index = 0;
										if (0 == (final_index_max = (*middle_iter->second)->first.size())) {
											++middle_iter;
											++middle_index;
										}
										else {
											// we got one
											return;
										}
									}
								}
							}
						}
					}
				}

				// failure!
				final_index_max = final_index = middle_index_max = middle_index = outtermost_index = outtermost_index_max = 0;
			};
			void Prev(thisType* ref) {
				throw std::runtime_error("Cannot iterate in reverse with the FunctionsMap iterator");
			};
			value_type& Get(thisType* ref) const {
				return **final_iter->second;
			};
			bool operator==(it_state const& rhs) const {
				return (outtermost_index == rhs.outtermost_index)
					&& (middle_index == rhs.middle_index)
					&& (final_index == rhs.final_index);
			};
			difference_type Distance(it_state const& other) const {
				throw std::runtime_error("Cannot calculate distance with the FunctionsMap iterator");
			};
		};

	public:
		SETUP_ITERATOR(FunctionsMap, it_state);

		// Allows looping over all contained Tuples with the requested name. 
		iterator find(std::string_view const& name);

	public:
		TupleType* BuildMatch(std::string_view const& functionName, ParamTypes const& params, TypeConverter& m_typeConverters, bool AllowTemplateInstantiation = true, bool AllowTypeConversion = true);
		Any Call(std::string_view const& functionName, std::vector<Any> const& params, TypeConverter& m_typeConverters);

	};

};

// forward decl
namespace GoodLang {
	class Scope;
	class FunctionScope;
	class Namespace;
	class Class;	
	class Global;

};

// Scope, Namespace, Class
namespace GoodLang {
	class Scope {
	public:
		friend class FunctionScope;
		friend class Namespace;
		friend class Class;
		friend class Global;

		Scope(std::shared_ptr<Scope> const& parent, bool fromScope = true)
			: p_UniqueName(" _ _ _ _") //  _ _
			, p_self()
			, p_parent(parent)
			, p_namespace()
			, p_library()
			, p_using()
		{
			// for a speed-up, attempt to convert a single random number into our desired pseudo-random string. 
			void* pos = const_cast<void*>((const void*)p_UniqueName.c_str());
			double* double_part = (double*)pos;
			*double_part = std::rand();

			if (parent) { p_namespace = parent->GetNamespaceImpl(); }
			else { p_namespace = std::dynamic_pointer_cast<Namespace>(p_self.lock()); }

			if (parent) { p_library = parent->GetLibraryImpl(); }
			else { p_library = std::dynamic_pointer_cast<Global>(p_self.lock()); }

			if (!parent || !fromScope) {
				qualifiedNamespaceWithQualifiers = GetQualifiedNamespaceImpl(true);
				qualifiedNamespaceWithoutQualifiers = GetQualifiedNamespaceImpl();
			}
			else {
				qualifiedNamespaceWithQualifiers = parent->qualifiedNamespaceWithQualifiers;
				qualifiedNamespaceWithoutQualifiers = parent->qualifiedNamespaceWithoutQualifiers;
			}
		};
		Scope(std::shared_ptr<FunctionScope> const& parent, bool fromScope = true) : Scope(std::dynamic_pointer_cast<Scope>(parent), fromScope) {};
		Scope(std::shared_ptr<Namespace> const& parent, bool fromScope = true) : Scope(std::dynamic_pointer_cast<Scope>(parent), fromScope) {};
		Scope(std::shared_ptr<Class> const& parent, bool fromScope = true) : Scope(std::dynamic_pointer_cast<Scope>(parent), fromScope) {};
		Scope(std::shared_ptr<Global> const& parent, bool fromScope = true) : Scope(std::dynamic_pointer_cast<Scope>(parent), fromScope) {};

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

			if (parent) {
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
					if (GetUniqueQualifier)
						path = parent->qualifiedNamespaceWithQualifiers + name + "::";
					else 
						path = parent->qualifiedNamespaceWithoutQualifiers + name + "::";
				}
				else {
					if (GetUniqueQualifier)
						path = parent->qualifiedNamespaceWithQualifiers;
					else
						path = parent->qualifiedNamespaceWithoutQualifiers;
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

	public:
		std::shared_ptr<Scope> GetSelf() const { return p_self.lock(); };
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
		void SetParent_Unsafe(std::weak_ptr<Scope> const& parent) {
			p_parent = parent;
		};

	private:
		concurrency::concurrent_unordered_map< std::string, std::shared_ptr<Any>>
			// UnorderedMap<std::string, std::shared_ptr<Any>>
			p_objects; // scopes of all types may declare objects. Namespace objects may be global objects, but still. 

	public:
		// try and find the object with the requested key.
		std::shared_ptr<Any> GetObj(std::string const& name) const;
		// Returns true if successful. Returns false is replaceIfExisting==false and the object already existed on the Scope.
		bool AddObj(std::string const& name, std::shared_ptr<Any> const& obj, bool updateObjectTree = true);

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
		GoodLang::details::flat_map<size_t, std::weak_ptr<Namespace>> // allows this scope to use the children of other scopes as if they were their own.
			p_using;
		// the Library should know about our "using" list
		virtual bool RecordUsing(std::shared_ptr<Namespace> ptr) {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->RecordUsing(ptr);
			}
			return false;
		};
		virtual bool RecordObject(std::string const& Name, std::shared_ptr<Any> const& ptr) {
			this->CachedObjectVersion.Increment();
			return true;
		};

	public:
		// allows this scope to use the children of other scopes as if they were their own.
		bool AddUsing(std::weak_ptr<Namespace> namespacePtr);

	private:
		UnorderedMap<std::string, std::shared_ptr<Namespace>>
	    // children namespaces - may be classes or namespaces. By this design, imported namespaces may be "unloaded" on scope unloading, which is on purpose.
			p_children{};

		// the Library should know about our "Class" list
		virtual bool RecordClass(std::shared_ptr<Class> ptr) {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->RecordClass(ptr);
			}
			return false;
		};

		std::atomic<bool>
			is_basic_scope{ true };
    public:
		bool IsBasicScope() const {
			return is_basic_scope.load();
		};
	public:
		bool AddChild(std::shared_ptr<Namespace> NamespacePtr);

	private:
		virtual void RemoveStaleReferences() {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref.second->expired()) {
						toRemove = *ref.first;
						doRemoval = true;
						break;
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

		struct custom_hash {
			static uint64_t splitmix64(uint64_t x) {
				// http://xorshift.di.unimi.it/splitmix64.c
				x += 0x9e3779b97f4a7c15;
				x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
				x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
				return x ^ (x >> 31);
			}

			size_t operator()(Scope* x) const {
				static const uint64_t FIXED_RANDOM = std::chrono::steady_clock::now().time_since_epoch().count();
				return splitmix64(reinterpret_cast<uint64_t&>(x) + FIXED_RANDOM);
			}
		};


		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const;
		virtual bool TryFindNearestScopeWhere_2(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&, bool, bool)> const& func,
			bool isExporingParent,
			bool allowFindObject,
			GoodLang::details::flat_set< Scope* > const& CheckedSelf = {},
			GoodLang::details::flat_set< Scope* > const& CheckedAll = {}
		) const;

		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const;

	protected:
		virtual std::weak_ptr<Type_Info> GetClassType() const { return user_type_shared<void>(); };

	public:
		virtual bool AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists = true);

	public:
		virtual std::shared_ptr< Functions > GetFunctions() const;
		virtual std::shared_ptr< Functions::FunctionSort > GetFunctions(std::string const& name) const;
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params);
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree);
		virtual Proxy_Function GetFunction(std::string const& name, ParamTypes& params, TypeConverter& tree);

	public:
		std::shared_ptr<Scope> FindNearestScopeWhere(std::function<bool(std::shared_ptr<Scope> const&)> const& func) const;
		std::shared_ptr<Namespace> FindNearestNamespaceWhere(std::function<bool(std::shared_ptr<Namespace> const&)> const& func) const;

	public:
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
		using CacheContainer
			= std::pair < GoodLang::fast_shared_mutex, std::unordered_map<size_t, std::weak_ptr<void>>>;
		using VersionedCacheContainer // use this mutex to delete entire caches once the version is out-of-date
			= std::map<size_t, std::shared_ptr<CacheContainer>>;
		using TemplatedCacheContainer // organizes multiple caches for several purposes...
			= std::vector<SharedLockable<VersionedCacheContainer>>;
		TemplatedCacheContainer
			SearchCache{ 6, SharedLockable<VersionedCacheContainer>() };

		template<size_t CacheID> SharedLockable<VersionedCacheContainer>& GetVersionedCacheContainer() const {
			return const_cast<SharedLockable<VersionedCacheContainer>&>(SearchCache[CacheID]);
		};
		template<size_t CacheID> std::shared_ptr<CacheContainer> GetCacheContainer(size_t version) const {
			SharedLockable<VersionedCacheContainer>& version_container = GetVersionedCacheContainer<CacheID>();
			version_container.EnsureDataExists();

			// test if exists
			if (1) {
				version_container.lock.lock_shared();
				if (version_container.data->size() > 0) {
					if (1) {
						auto f = version_container.data->rbegin();
						if (f->first >= version) {
							std::shared_ptr<CacheContainer> out{ f->second };
							version_container.lock.unlock_shared();
							return out;
						}
					}
				}
				version_container.lock.unlock_shared();
			}

			// didn't exist yet -- delete old version(s) and create new version.
			if (1) {
				auto locked = version_container.Unique();
				auto f = locked->find(version); // someone beat us to it
				if (f != locked->end()) {
					return f->second;
				}
				else {
					// allow some caching of versions, in case of multithreading
					if (locked->size() > 64) {
						// locked->erase(locked->begin());
						while (locked->size() > 8) {
							locked->erase(locked->begin());
						}
					}
					auto out{ std::make_shared<CacheContainer>() };
					locked->insert(std::pair<size_t, std::shared_ptr<CacheContainer>>{ version, out });
					return out;
				}
			}
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
		std::shared_ptr<Namespace> FindNamespace(std::string QualifiedOrUnqualifiedNamespaceName) const;
		std::shared_ptr<Class> FindClass(std::string const& QualifiedOrUnqualifiedNamespaceName) const;

	private:
		virtual std::shared_ptr<Scope>  FindScopeWithObjImpl(std::string const& objName, std::shared_ptr<Any>* found_obj) const;
		std::shared_ptr<Scope> FindScopeWithObj(std::string const& objName, std::shared_ptr<Any>* found_obj = nullptr) const;

		GoodLang::InterlockedLong
			CachedObjectVersion{ 0 };
	public:
		std::shared_ptr<Class> FindClass(std::weak_ptr<Type_Info> typeInfo) const;

		std::shared_ptr<Any> FindObj(std::string objName) const;

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName) const;
		std::shared_ptr< Functions::FunctionSort > FindFunctions(std::string functionName) const;

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree);
		Proxy_Function FindFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree);

		std::shared_ptr<Scope> FindScopeWithObjOrFunction(std::string objName, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree, std::shared_ptr<Any>* found_obj, Proxy_Function* found_function);
		std::shared_ptr<Scope> FindScopeWithObjOrFunction(std::string objName, ParamTypes const& Params, TypeConverter& tree, std::shared_ptr<Any>* found_obj, Proxy_Function* found_function);
		bool FindObjOrFunction(std::string const& objName, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree, std::shared_ptr<Any>* found_obj, Proxy_Function* found_function);
		bool FindObjOrFunction(std::string const& objName, ParamTypes const& Params, TypeConverter& tree, std::shared_ptr<Any>* found_obj, Proxy_Function* found_function);

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, ParamTypes& params, TypeConverter& tree);
		Proxy_Function FindFunction(std::string functionName, ParamTypes& params, TypeConverter& tree);

		virtual size_t GetObjectCacheVersion() const;
		virtual size_t GetTypeConverterTreeVersion() const;
		virtual GoodLang::shared_ptr<TypeConverter>& GetTypeConverterTree() const;

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, ParamTypes const& Params);
		Proxy_Function FindFunction(std::string functionName, std::vector<Any> const& params);



		std::vector<std::shared_ptr<Scope>> GetScopesForObjectSearch() const;



	public: // private:
		bool TryFindFunctionImpl(std::string const& functionName, std::vector<Any>  const& params, ParamTypes const& Params, GoodLang::shared_ptr<TypeConverter> const& m_conversionTree, Proxy_Function& out, size_t paramsHash = 0) const;

	public:
		std::pair<Proxy_Function, std::reference_wrapper<GoodLang::shared_ptr<TypeConverter>>> BuildFunction(std::string const& functionName, std::vector<Any> const& params, ParamTypes const& Params, size_t paramsHash = 0) const;
		Any CallFunction(std::string const& functionName, std::vector<Any> const& params) const;
		Any CallFunction(Proxy_Function const& function, std::vector<Any> const& params) const;
		Any CallFunction(std::string const& functionName, Any& params) const;
		Any CallFunction(Proxy_Function const& function, Any& params) const;

		template <typename T>
		T Cast(Any const& from) const {
			// see if it already matches (best option)
			if (from.IsTypeOf<T>()) {
				return from.cast<T>();
			}

			auto ToType = user_type_shared<T>();
			auto FromType = from.Type();

			// see if we can convert (fastest option)
			if (auto Tree = this->GetTypeConverterTree()) {
				if (Tree->Converts<T>(FromType)) {
					try {
						return Tree->Convert<T>(from);
					}
					catch (exception::bad_any_cast&) {}
				}
			}

			auto ToClass = std::dynamic_pointer_cast<Scope>(this->FindClass(ToType));
			if (ToClass) {
				// see if he can convert (fastest option)
				if (auto Tree2 = ToClass->GetTypeConverterTree()) {
					if (Tree2->Converts<T>(FromType)) {
						try {
							return Tree2->Convert<T>(from);
						}
						catch (exception::bad_any_cast&) {}
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

				// Failure to cast From -> To
				throw exception::bad_any_cast(FromType, ToType, __LINE__);
			}

			// Failure
			throw exception::not_found_error(GetTypeName(ToType));
		};
		Any Cast(Any const& from, std::weak_ptr<Type_Info> const& To) const {
			auto ToType = To.lock();
			auto FromType = from.Type();

			// see if it already matches (best option)
			if (from.IsTypeOf(ToType)) {
				return from;
			}

			// see if we can convert (fastest option)
			if (auto Tree = this->GetTypeConverterTree()) {
				if (Tree->Converts(FromType, ToType)) {
					try {
						return Tree->Convert(from, ToType);
					}
					catch (exception::bad_any_cast&) {}
				}
			}

			auto ToClass = std::dynamic_pointer_cast<Scope>(this->FindClass(ToType));
			if (ToClass) {
				// see if he can convert (fastest option)
				if (auto Tree2 = ToClass->GetTypeConverterTree()) {
					if (Tree2->Converts(FromType, ToType)) {
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
						return this->CallFunction(ToClass->GetName(), params);
					}
					catch (exception::not_found_error) {}

					// call a functor from their scope
					try {
						return ToClass->CallFunction(ToClass->GetName(), params);
					}
					catch (exception::not_found_error) {}
				}

				// Failure to cast From -> To
				throw exception::bad_any_cast(FromType, ToType, __LINE__);
			}

			// Failure
			throw exception::not_found_error(GetTypeName(ToType));
		};

	public:
		virtual std::string ToString() const;
		virtual std::vector< Impl::NodeCache > GetChildren() const;
		virtual bool TryDisconnectChild() const;

	};

	class FunctionScope : public Scope {
	public:
		friend class Namespace;
		friend class Class;
		friend class Global;

		FunctionScope(std::shared_ptr<Scope> const& parent) : Scope(parent, true) {};
		FunctionScope(std::shared_ptr<FunctionScope> const& parent) : FunctionScope(std::dynamic_pointer_cast<Scope>(parent)) {};
		FunctionScope(std::shared_ptr<Namespace> const& parent) : FunctionScope(std::dynamic_pointer_cast<Scope>(parent)) {};
		FunctionScope(std::shared_ptr<Class> const& parent) : FunctionScope(std::dynamic_pointer_cast<Scope>(parent)) {};
		FunctionScope(std::shared_ptr<Global> const& parent) : FunctionScope(std::dynamic_pointer_cast<Scope>(parent)) {};
		virtual ~FunctionScope() {};
		void SetSelf(std::shared_ptr<FunctionScope>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };

		virtual std::shared_ptr<Scope>  FindScopeWithObjImpl(std::string const& objName, std::shared_ptr<Any>* found_obj) const override;
		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const override;

		virtual bool TryFindNearestScopeWhere_2(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&, bool, bool)> const& func,
			bool isExporingParent,
			bool allowFindObject,
			GoodLang::details::flat_set< Scope* > const& CheckedSelf = {},
			GoodLang::details::flat_set< Scope* > const& CheckedAll = {}
		) const override;

	public:
		virtual std::string ToString() const override;
		virtual std::vector< Impl::NodeCache > GetChildren() const override;
		virtual bool TryDisconnectChild() const override;

	};

	class Namespace : public Scope {
	public:
		friend class Class;
		friend class Global;

		Namespace(std::shared_ptr<Scope> const& parent, std::string const& Name)
			: Scope(parent, false)
			, p_Name(Name)
		{
			this->is_basic_scope = false;
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		Namespace(std::shared_ptr<FunctionScope> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};
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
		void SetName_Unsafe(std::string const& namespaceName) {
			p_Name = this->p_UniqueName + namespaceName;
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};

	private:
		UnorderedMap<std::string, std::weak_ptr<Class>> // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate are not allowed.
			p_postfixes;

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
					if (ref.second->expired()) {
						toRemove = *ref.first;
						doRemoval = true;
						break;
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
					if (ref.second.expired()) {
						toRemove = ref.first;
						doRemoval = true;
						break;
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
		virtual bool RecordFunction(std::string const& Name, Function const& ptr) {
			if (auto p = std::dynamic_pointer_cast<Namespace>(GetLibrary())) {
				return p->RecordFunction(Name, ptr);
			}
			return false;
		};

	public:
		virtual bool AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists = true) override;

	public:
		virtual std::shared_ptr< Functions > GetFunctions() const override;
		virtual std::shared_ptr< Functions::FunctionSort > GetFunctions(std::string const& name) const override;
		virtual Proxy_Function GetFunction(std::string const& name, ParamTypes& params, TypeConverter& tree) override;
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& tree) override;
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params) override;

	public:
		virtual std::string ToString() const override;
		virtual std::vector< Impl::NodeCache > GetChildren() const override;
		virtual bool TryDisconnectChild() const override;
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
		Class(std::shared_ptr<FunctionScope> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Namespace> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Class> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Global> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type, std::vector<std::weak_ptr<Class>> inheritance)
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};

		Class(std::shared_ptr<Scope> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, std::vector<std::weak_ptr<Class>>{ inheritance }) {};
		Class(std::shared_ptr<FunctionScope> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
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
		UnorderedMap<std::string, std::pair<std::weak_ptr<Type_Info>, std::shared_ptr<Any>>>
			p_declared_member_objects; // declared member objects for the custom, scripted class which will be instantiated upon construction of the scripted class

	public:
		void ConstructMemberObjects(DynamicObject& obj) const;
		void ConstructMemberObjects(DynamicObject& obj, DynamicObject const& CopyFrom) const;
		// Gets the member objects of just this class
		std::map<std::string, std::weak_ptr<Type_Info>> GetMemberObjects() const;
		// Gets the member objects of this class all all inherited classes (recursively)
		std::map<std::string, std::weak_ptr<Type_Info>> GetAllMemberObjects() const;

	public:
		void AddDefaultConstructors();

	public:
		virtual std::weak_ptr<Type_Info> GetClassType() const override { return ClassType; };
		void DeclareMemberObject(std::string const& name, std::weak_ptr<Type_Info> type, std::shared_ptr<Any> defaultValue = nullptr);

	private:
		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const override;

		virtual bool TryFindNearestScopeWhere_2(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&, bool, bool)> const& func,
			bool isExporingParent,
			bool allowFindObject,
			GoodLang::details::flat_set< Scope* > const& CheckedSelf = {},
			GoodLang::details::flat_set< Scope* > const& CheckedAll = {}
		) const override;

		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const override;

	public:
		virtual std::string ToString() const override;
		virtual std::vector< Impl::NodeCache > GetChildren() const override;
		virtual bool TryDisconnectChild() const override;

	};

	// Support for Units
	class UnitsLibrary {
	public:
		template <typename T>
		__forceinline static Any CastToValue(std::shared_ptr<T> from) {
			return Any(std::dynamic_pointer_cast<Units::value>(std::move(from)));
		};

		template<typename T>
		__forceinline static void AddUnit(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
			std::string UnitName = T().UnitName().data();
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
				foot_namespace->AddFunction("=", Function(make_callable([](Any const& a, Units::value const& b) -> Any {
					T& out = a.cast(); out = b; return a;
					}, ParamTypes({ foot_namespace->GetClassType().lock()->MakeRef(), user_type_shared<Units::value>().lock()->MakeConstRef() }), foot_namespace->GetClassType().lock()->MakeRef()), false));

				// value(foot)
				value_namespace->AddFunction(
					value_namespace->GetName(),
					make_callable(
						&CastToValue<T>,
						ParamTypes({ foot_namespace->GetClassType().lock()->MakeConstRef() }),
						GoodLang::user_type_shared<Units::value>().lock()->MakeConstRef()
					)
				);
				value_namespace->AddFunction(
					value_namespace->GetName(),
					make_callable(
						&CastToValue<T>,
						ParamTypes({ foot_namespace->GetClassType().lock()->MakeRef() }),
						GoodLang::user_type_shared<Units::value>().lock()->MakeRef()
					)
				);
			}
		};

		static void Part1(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		static void Part2(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		static void Part3(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		static void Part4(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		static void Part5(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		static void Part6(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
	};

};

// <Any, Scope> hash
namespace std {
	template <> struct hash<std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>> {
		std::size_t operator()(const std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>& k) const {
			if (auto p = k.second.lock()) {
				return p->Cast<size_t>(p->CallFunction("to_hash", { k.first }));
			}
			return 0;
		};
	};
	template <> struct less<std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>> {
		std::size_t operator()(const std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>& lhs, const std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>& rhs) const {
			static std::hash< std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>> > hasher{};
			return hasher(lhs) < hasher(rhs);
		};
	};
};

namespace GoodLang {
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

		std::vector<std::weak_ptr<Class>> GetClasses() const;
		std::vector<std::weak_ptr<Namespace>> GetUsing() const;

	public:
		void AddBuiltIns();

	private:
		void GetClasses(std::unordered_map<size_t, std::weak_ptr<Class>>& out) const;
		void GetAllAvailableClassesImpl(
			std::unordered_map<size_t, std::weak_ptr<Class>>& out,
			std::unordered_map<size_t, std::weak_ptr<Scope>>& uniqueLibraries
		) const;

	private:
		// Searches for all classes that are defined in the current and "used" libraries. 
		std::unordered_map<size_t, std::weak_ptr<Class>> GetAllAvailableClassesImpl() const;
	public:
		std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>> GetAllAvailableClasses() const;

	public:
		// Creates a tree of type-converter functions using the classes found with GetAllAvailableClasses()
		void CreateTypeConverterTree(GoodLang::shared_ptr<TypeConverter>& out) const;

	public:
		virtual size_t GetTypeConverterTreeVersion() const override;
		virtual GoodLang::shared_ptr<TypeConverter>& GetTypeConverterTree() const override;

	private:
		UnorderedMap<size_t, std::weak_ptr<Class>> // collection of all classes that are added as "children" of this library
			Classes;
		UnorderedMap<size_t, std::weak_ptr<Namespace>> // collection of all namespaces that are added being "used" by this library
			Usings;
		UnorderedMap<size_t, std::pair<std::string, std::weak_ptr<details::Proxy_Function_Base>>> // collection of all namespaces that are added being "used" by this library
			Functions;

		GoodLang::shared_ptr<TypeConverter>
			CachedTypeConverterTree{ GoodLang::make_shared<TypeConverter>() };

		GoodLang::InterlockedLong
			CachedTypeConverterTreeVersion{ -1 };

		std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>>
			CachedClassList{ std::make_shared<std::unordered_map<size_t, std::weak_ptr<Class>>>() };
		GoodLang::InterlockedLong
			CachedClassListVersion{ 0 };
		GoodLang::fast_shared_mutex // fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedClassListMutex{};

		//fibers::containers::number<unsigned int> 
			//CleanupRequested{ 0 };
		GoodLang::InterlockedLong
			CleanupVersion{ 0 };
		GoodLang::InterlockedLong
			RecordVersion{ 0 };
		GoodLang::InterlockedLong
			LibraryVersion{ 0 };

		virtual void RemoveStaleReferences() override {
			auto oldVersion = CleanupVersion.load();
			if (oldVersion != RecordVersion) {
				if (CleanupVersion.CompareExchange(oldVersion, RecordVersion.GetValue())) {
					// p_using
					while (true) {
						size_t toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_using) {
							if (ref.second->expired()) {
								toRemove = *ref.first;
								doRemoval = true;
								break;
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
							if (ref.second.expired()) {
								toRemove = ref.first;
								doRemoval = true;
								break;
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
							if (ref.second.expired()) {
								toRemove.push_back(ref.first);
							}
						}
						for (auto& x : toRemove) Classes.erase(x);
						toRemove.clear();
					}

					// Usings
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Usings) {
							if (ref.second.expired()) {
								toRemove.push_back(ref.first);
							}
						}
						for (auto& x : toRemove) Usings.erase(x);
						toRemove.clear();
					}

					// Functions
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Functions) {
							if (ref.second.second.expired()) {
								toRemove.push_back(ref.first);
							}
						}
						for (auto& x : toRemove) Functions.erase(x);
						toRemove.clear();
					}
				}
			}
		};

		virtual bool RecordClass(std::shared_ptr<Class> ptr) override;
		virtual bool RecordUsing(std::shared_ptr<Namespace> ptr) override;
		virtual bool RecordFunction(std::string const& Name, Function const& ptr) override;

	public:
		// Creates a temporary "fake" scope that will act as if it is a global scope, but whose changes will never effect it.
		// Benefits from being able to share the real parent's cached functions and type conversions, which should be a significant performance boost. 
		static std::shared_ptr<Global> CreateTemporaryGlobalChild(std::shared_ptr<Global> const& parent);

	public:
		virtual std::string ToString() const override;
		virtual std::vector< Impl::NodeCache > GetChildren() const override;
		virtual bool TryDisconnectChild() const override;
	};

	std::shared_ptr<Global> StartScope(std::shared_ptr<Scope> const& parent = nullptr);
};

namespace GoodLang {
	namespace Impl {
		void ToString(Tag<Scope>, Scope const& r, std::string& out);
		void GetChildren(Tag<Scope>, Scope const& r, std::vector< NodeCache >& out);
		void TryDisconnectChild(Tag<Scope>, Scope const& r, bool& out);

		void ToString(Tag<FunctionScope>, FunctionScope const& r, std::string& out);
		void GetChildren(Tag<FunctionScope>, FunctionScope const& r, std::vector< NodeCache >& out);
		void TryDisconnectChild(Tag<FunctionScope>, FunctionScope const& r, bool& out);

		void ToString(Tag<Namespace>, Namespace const& r, std::string& out);
		void GetChildren(Tag<Namespace>, Namespace const& r, std::vector< NodeCache >& out);
		void TryDisconnectChild(Tag<Namespace>, Namespace const& r, bool& out);

		void ToString(Tag<Class>, Class const& r, std::string& out);
		void GetChildren(Tag<Class>, Class const& r, std::vector< NodeCache >& out);
		void TryDisconnectChild(Tag<Class>, Class const& r, bool& out);

		void ToString(Tag<Global>, Global const& r, std::string& out);
		void GetChildren(Tag<Global>, Global const& r, std::vector< NodeCache >& out);
		void TryDisconnectChild(Tag<Global>, Global const& r, bool& out);
	};
};