
#ifndef __CWEETHREADEDMAP_H__
#define __CWEETHREADEDMAP_H__
#pragma hdrstop
/*!
Thread-safe list that performs garbage collection and manages read/write/create/delete operations on data. Intended to act as a multi-threaded database.
*/
template< typename Key, typename Value>
class cweeThreadedMap {
public:
	typedef Value						Type;
	typedef cweeSharedPtr < Type >		PtrType;
	typedef std::pair<Key, PtrType>		_iterType;
	typedef long long					IdxType;

	class cweeThreadedMap_Impl {
	public:
		/*! prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope. */
		auto			Guard(void) const { // assumes unlocked!
			return lock.Guard();
		}
		/*! Prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock. A "Unlock" must be called to re-enable access to the list. */
		void			Lock(void) const { // assumes unlocked!
			lock.Lock();
		}
		/*! Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior. */
		void			Unlock(void) const { // assumes already locked! 
			lock.Unlock();
		};
		/*! After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping. */
		PtrType			UnsafeRead(Key index) const { // was non-const			
			auto it = list.find(index);
			if (it != list.end()) {
				return it->second;
			}
			return nullptr;			
		}
		/*! After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list. */
		cweeThreadedList<Key> UnsafeList(void) const {
			cweeThreadedList<Key> out;

			if (lastCreatedListVersion == CreatedListVersion) {
				indexList.Lock();
				out = *indexList.UnsafeRead();
				indexList.Unlock();

				return out;
			}
			int size = list.size();

			indexList.Lock();
			indexList.UnsafeRead()->Clear();
			indexList.UnsafeRead()->SetGranularity(size + 16);
			for (auto& kv : list) indexList.UnsafeRead()->Append(kv.first);
			lastCreatedListVersion = CreatedListVersion;
			indexList.Unlock();

			indexList.Lock();
			out = *indexList.UnsafeRead();
			indexList.Unlock();

			return out;
		};
		AUTO			UnsafeIndexForKey(Key index) const {
			return std::distance(list.begin(), list.find(index));
		};

		/* Clear the list */
		void Clear() {
			AUTO g = Guard();

			list_version = 0;
			CreatedListVersion = 0;
			
			for (auto& kv : list) {
				PtrType& p = const_cast<PtrType&>(kv.second); // need to access the underlying thing
				p = nullptr;
			}
			list.clear();

			indexList = cweeThreadedList<Key>();
			lastSearchID = Key();
			lastResult = list.end();
			lastVersion = -1;
			lastCreatedListVersion = -1;
		};

		cweeThreadedMap_Impl() : lock(), list(), indexList(), list_version(0), lastSearchID(), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1) {
			list.reserve(16);
			indexList.Lock();
			indexList.UnsafeRead()->SetGranularity(16);
			indexList.Unlock();
		};
		~cweeThreadedMap_Impl() {
			Clear();
		};

		/*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
		mutable cweeSysMutex														lock;
		/* Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions. */
		mutable tsl::robin_map<Key, PtrType, robin_hood::hash<Key>, std::equal_to<Key>, std::allocator<_iterType>, true>	list;
		/* Optimized search parameters */
		mutable cweeUnpooledInterlocked < cweeThreadedList<Key>	>					indexList;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											list_version;
		/* Optimized search parameters */
		mutable cweeUnpooledInterlocked<Key>										lastSearchID;
		/* Optimized search parameters */
		mutable typename tsl::robin_map<Key, PtrType, robin_hood::hash<Key>, std::equal_to<Key>, std::allocator<_iterType>, true>::iterator		lastResult; //  = list.end();
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											lastVersion;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											CreatedListVersion;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											lastCreatedListVersion;
	};

	typedef cweeSharedPtr<cweeThreadedMap_Impl>		ImplType;

	struct it_state {
		inline void at(const cweeThreadedMap* ref, Key index) {
			ref->Lock();
			AUTO h = ref->UnsafeIndexForKey(index);
			ref->Unlock();
			at(ref, index, h);
		};
		inline void at(const cweeThreadedMap* ref, Key index, const IdxType t_hint) {
			listOfIndexes = ref->GetList();
			idx = 0;
			IdxType n = listOfIndexes.Num();


			//idx = t_hint <= n ? t_hint : n;
			//return;

			for (idx = t_hint; idx < n; ++idx) {
				if (listOfIndexes[idx] == index) {
					return;
				}
			}	
			for (idx = 0; idx < n && idx < t_hint; ++idx) {
				if (listOfIndexes[idx] == index) {
					return;
				}
			}
			idx = n;
		};
		mutable cweeThreadedList<Key>	listOfIndexes;
		mutable IdxType idx;
		mutable _iterType iter;
		inline void begin(const cweeThreadedMap* ref) { 
			listOfIndexes = ref->GetList(); 
			idx = 0;
		}
		inline void next(const cweeThreadedMap* ref) { 
			++idx;
			while (idx < listOfIndexes.Num() && !ref->Check(listOfIndexes[idx])) {
				++idx;
			}	
		}
		inline void end(const cweeThreadedMap* ref) { 
			listOfIndexes = ref->GetList();
			idx = listOfIndexes.Num();
		}
		inline _iterType& get(cweeThreadedMap* ref) {
			iter = ref->GetIterator(listOfIndexes[idx]);
			return iter; 
		}
		inline bool cmp(const it_state& s) const { return (idx == s.idx) ? false : true; }
		inline IdxType distance(const it_state& s) const { return idx - s.idx; }

		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeThreadedMap* ref) { 
			--idx;
			while (idx >= 0 && !ref->Check(listOfIndexes[idx])) {
				--idx;
			}
		}
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const cweeThreadedMap* ref) const { iter = ref->GetIterator(listOfIndexes[idx]); return iter; }
	};	

	SETUP_STL_ITERATOR(cweeThreadedMap, _iterType, it_state);

	/*! Default constructor */
	cweeThreadedMap() : impl(new cweeThreadedMap_Impl()) {};
	/*! Default copy */
	cweeThreadedMap(const cweeThreadedMap& other) : impl(new cweeThreadedMap_Impl()) { this->Copy(other); };
	/*! Take reference to underlying data */
	cweeThreadedMap(cweeThreadedMap&& other) : impl(other.impl) {};
	/*! Default copy */
	cweeThreadedMap& operator=(const cweeThreadedMap& other) {
		this->Copy(other);
		return *this;
	};
	/*! Default destructor */
	~cweeThreadedMap() {
		impl = nullptr;
	};
	/*! Convert UnorderedMap to a list of keys */
	operator cweeThreadedList<Key>() const {
		return GetList();
	};
	/*! Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack. */
	PtrType			operator[](Key index) const { return GetPtr(index); };
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			Emplace(Key index, const Value& obj) {
		bool out;
		Lock();
		out = UnsafeEmplace(index, obj);
		Unlock();
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			Emplace(Key index, Value&& obj) {
		bool out;
		Lock();
		out = UnsafeEmplace(index, std::forward<Value>(obj));
		Unlock();
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			UnsafeEmplace(Key index, const Value& obj) {
		bool out;
		auto ptr = UnsafeRead(index);
		if (ptr) {
			// already exists
			*ptr = obj;
			out = false;
		}
		else {
			// does not yet exist
			ptr = UnsafeAppendAt(index);
			if (ptr) {
				*ptr = obj;
			}
			out = true;
		}
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			UnsafeEmplace(Key index, Value&& obj) {
		bool out;
		auto ptr = UnsafeRead(index);
		if (ptr) {
			// already exists
			*ptr = std::forward<Value>(obj);
			out = false;
		}
		else {
			// does not yet exist
			ptr = UnsafeAppendAt(index);
			if (ptr) {
				*ptr = std::forward<Value>(obj);
			}
			out = true;
		}
		return out;
	};
	/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeThreadedMap lifetime does.   */
	PtrType			UnsafeGetPtr(Key index) const {
		if (impl->list.count(index) <= 0) 
			return UnsafeAppendAt(index);		
		else 
			return impl->list[index];		
	};
	/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeThreadedMap lifetime does.   */
	PtrType			GetPtr(Key index) const {
		PtrType out;
		Lock();
		out = UnsafeGetPtr(std::move(index));
		Unlock();
		return out;
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeThreadedMap lifetime does. */
	PtrType			TryGetPtr(Key index) const {
		PtrType out;
		Lock();
		if (impl->list.count(index) > 0) {
			out = impl->list[index];
		}
		Unlock();
		return out;
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeThreadedMap lifetime does. */
	_iterType		GetIterator(Key index) const {
		PtrType out;
		Lock();
		if (impl->list.count(index) <= 0) {
			out = UnsafeAppendAt(index);
		}
		else
		{
			out = impl->list[index];
		}
		Unlock();
		return _iterType(index, out);
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeThreadedMap lifetime does. */
	_iterType		TryGetIterator(Key index) const {
		_iterType out;
		Lock();
		if (impl->list.count(index) > 0) {
			out = _iterType(index, impl->list[index]);
		}
		Unlock();
		return out;
	};
	/*! True/False if the index exists on the heap */
	bool			Check(Key index) const {
		bool result = true;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			impl->indexList.Lock();
			result = (impl->indexList.UnsafeRead()->FindIndex(index) != -1);
			impl->indexList.Unlock();
			SharedUnlock();
			return result;
		}
		SharedLock();
		result = (impl->list.count(index) != 0);
		SharedUnlock();
		return result;
	};
	/*! True/False if the index exists on the heap */
	bool			UnsafeCheck(Key index) const {
		bool result = true;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			impl->indexList.Lock();
			result = (impl->indexList.UnsafeRead()->FindIndex(index) != -1);
			impl->indexList.Unlock();
			return result;
		}
		result = (impl->list.count(index) != 0);
		return result;
	};
	/*! Clear the current list and create a copy of the incoming list. */
	void			Copy(const cweeThreadedMap& copy, const bool threadSafePtr = false) {
		Clear();

		AUTO List = copy.GetList();

		if (threadSafePtr) {
			// Go through and perform swaps					
			for (int i = 0; i < List.Num(); i++) {
				copy.Lock();
				auto readCopy = copy.UnsafeRead(List[i]);
				copy.Unlock();

				if (readCopy) {
					Lock();
					UnsafeAppendAt(List[i]);
					auto access = UnsafeRead(List[i]);
					Unlock();

					if (access) *access = *readCopy;
				}
			}
		}
		else {
			// Go through and perform swaps		
			for (int i = 0; i < List.Num(); i++) {
				copy.Lock();
				auto readCopy = copy.UnsafeRead(List[i]);
				if (readCopy) {
					Lock();
					UnsafeAppendAt(List[i]);
					auto access = UnsafeRead(List[i]);
					if (access) *access = *readCopy;
					Unlock();
				}
				copy.Unlock();
			}
		}
	};
	/*! Copy the Value object from the stack into the heap at the index specified. If the index does not exist on the heap, nothing happens. */
	void			Swap(Key index, const Value& replacement) {
		if (Check(index)) {
			while (!CheckCanDeleteIndex(index)) {};
			Lock();
			AUTO it = impl->lastResult;
			if (impl->lastSearchID == index && impl->list.count(impl->lastSearchID.GetValue()) != 0 && impl->lastVersion == impl->list_version)	it = impl->lastResult;
			else { it = impl->list.find(index); impl->lastSearchID = index; impl->lastResult = it; impl->lastVersion = impl->list_version; }
			impl->list_version.Increment();
			if (it != impl->list.end()) *it->second = replacement;
			Unlock();
		}
	};
	/*! get the number of objects on the heap managed by this list */
	int				Num(void) const {
		int i = 0;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			impl->indexList.Lock();
			i = impl->indexList.UnsafeRead()->Num();
			impl->indexList.Unlock();
			return i;
		}

		SharedLock();
		i = impl->list.size();
		SharedUnlock();
		return i;
	};
	/*! get the number of objects on the heap managed by this list */
	int				UnsafeNum(void) const {
		int i = 0;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			impl->indexList.Lock();
			i = impl->indexList.UnsafeRead()->Num();
			impl->indexList.Unlock();
			return i;
		}
		i = impl->list.size();
		return i;
	};
	/*! get the index position of the key */
	AUTO			UnsafeIndexForKey(Key index) const {
		return impl->UnsafeIndexForKey(std::move(index));
	};
	/*! get a list of all indexes currently on the heap that are currently valid */
	cweeThreadedList<Key> GetList(void) const {
		cweeThreadedList<Key> out;

		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			impl->indexList.Lock();
			out = *impl->indexList.UnsafeRead();
			impl->indexList.Unlock();
			return out;
		}
		int size = Num();

		SharedLock();
		impl->indexList.Lock();
		impl->indexList.UnsafeRead()->Clear();
		impl->indexList.UnsafeRead()->SetGranularity(size + 16);
		for (auto& kv : impl->list) {
			impl->indexList.UnsafeRead()->Append(kv.first);
		}
		impl->lastCreatedListVersion = impl->CreatedListVersion;
		impl->indexList.Unlock();

		impl->indexList.Lock();
		out = *impl->indexList.UnsafeRead();
		impl->indexList.Unlock();

		SharedUnlock();
		return out;
	};
	/*! Erase the indexed object from the heap and free the memory for the list to re-use. */
	void			Erase(Key index) {
		Lock();
		if (impl->list.count(index) > 0) {
			impl->list[index] = nullptr;
			impl->list.erase(index);
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		Unlock();
	};
	/*! Clear the current list and free all memory back to the operating system */
	void			Clear(void) {
		impl->Clear();
	};
	/*!
	prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock.
	A "Unlock" must be called to re-enable access to the list.
	*/
	void			Lock(void) const {
		impl->Lock();
	};
	/*!
	Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior.
	*/
	void			Unlock(void) const {
		impl->Unlock();
	};
	/*!
	prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope.
	*/
	[[nodiscard]] auto	Guard(void) const { // assumes unlocked!
		return impl->Guard();
	}
	/*!
	After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping.
	*/
	PtrType			UnsafeRead(Key index) const {
		return impl->UnsafeRead(index);
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeThreadedMap lifetime does. */
	_iterType		UnsafeIterator(Key index) const {
		PtrType out;
		if (impl->list.count(index) <= 0) {
			out = UnsafeAppendAt(index);
		}
		else
		{
			out = impl->list[index];
		}
		return _iterType(index, out);
	};
	/*!
	After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list.
	*/
	cweeThreadedList<Key> UnsafeList(void) const {
		return impl->UnsafeList();
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<Key> SelectIndexes(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<Key> out;
		_iterType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(i);
			}
			Unlock();
		}
		return out;
	};
	/*!
	After calling "Lock", Append new data at a specified key location, regardless of the counter.
	*/
	PtrType			UnsafeAppendAt(Key index) const {
		if (impl->list.count(index) != 0) { return impl->list[index]; }

		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		/* Allocate a _type_ and store at specified index */
		AUTO newPtr = PtrType(new Value());
		impl->list.emplace(index, newPtr);
		return newPtr;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing.
	*/
	void			UnsafeErase(Key index) {
		if (impl->list.count(index) > 0) {
			impl->list.erase(index);
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtract(Key index, PtrType& out) {
		bool result = false;
		if (impl->list.count(index) > 0) {
			out = impl->list[index];
			impl->list.erase(index);
			result = true;
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		return result;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	Extract(Key index, PtrType& out) {
		bool result = false;
		AUTO G = Guard();
		result = UnsafeExtract(std::move(index), out);
		return result;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtractIndex(IdxType idx, PtrType& out) {
		_iterType p = unsafe_pair_at_index(idx);
		if (p.second) {
			UnsafeErase(p.first);
			out = p.second;
			return true;
		}
		return false;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	ExtractIndex(IdxType idx, PtrType& out) {
		bool result = false;
		AUTO G = Guard();
		result = UnsafeExtractIndex(std::move(idx), out);
		return result;
	};
public: // Compatability functions
	PtrType			at(Key index) const { return GetPtr(index); };
	PtrType			unsafe_at_index(IdxType idx) const {
		return unsafe_pair_at_index(idx).second;
	};
	_iterType		unsafe_pair_at_index(IdxType idx) const {
		_iterType out(-1, nullptr); 
		bool foundKey = false;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			impl->indexList.Lock();
			if (impl->indexList.UnsafeRead()->Num() > idx) {
				out.first = impl->indexList.UnsafeRead()->operator[](idx);
				foundKey = true;
			}
			impl->indexList.Unlock();
		}
		if (!foundKey) {
			auto _keys = UnsafeList();
			if (_keys.Num() > idx) {
				out.first = _keys[idx];
				foundKey = true;
			}
		}
		if (foundKey) {
			out.second = UnsafeGetPtr(out.first);
		}
		return out;
	};
	PtrType			at_index(IdxType idx) const {
		PtrType out;
		Lock();
		out = unsafe_pair_at_index(std::move(idx)).second;
		Unlock();
		return out;
	};
	_iterType		pair_at_index(IdxType idx) const {
		_iterType out; 
		Lock();
		out = unsafe_pair_at_index(std::move(idx));
		Unlock();
		return out;
	};
	void			erase_at(IdxType idx) {
		AUTO p = pair_at_index(idx);
		if (p.second) {
			Erase(p.first);
		}
	};
	void			erase(Key index) {		
		Erase(index);
	};
	bool			empty() const {
		return (size() == 0);
	};
	void			clear() {
		Clear();
	};
	_iterType		back() const {
		return pair_at_index(Num() - 1);
	};
	iterator		find(Key s) noexcept {
		iterator itr = iterator(this);
		itr.state.at(this, s);
		return itr;
	}
	iterator		find(Key s, const IdxType t_hint) noexcept {
		iterator itr = iterator(this);
		itr.state.at(this, s, t_hint);
		return itr;
	}
	const_iterator	find(Key s) const noexcept {
		const_iterator itr = const_iterator(this);
		itr.state.at(this, s);
		return itr;
	}
	const_iterator	find(Key s, const IdxType t_hint) const noexcept {
		const_iterator itr = const_iterator(this);
		itr.state.at(this, s, t_hint);
		return itr;
	}
	AUTO			size() const noexcept { return Num(); }
	template<typename M> AUTO insert_or_assign(Key&& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	template<typename M> AUTO insert_or_assign(const Key& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	IdxType count(const Key& s) const noexcept {
		return Check(s) ? 1 : 0;
		// return (find(s) != end()) ? 1 : 0;
	}
	std::pair<iterator, bool> insert(std::pair<Key, Value>&& value) {
		iterator out = iterator(this);
		Lock();
		bool didNotExist = UnsafeEmplace(value.first, value.second);		
		Unlock();
		out.state.at(this, value.first);
		return std::pair(out, didNotExist);
	};
	template<typename M> AUTO insert(Key&& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	template<typename M> AUTO insert(const Key& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};

private:
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedLock(void) const {
		impl->lock.Lock();
	};
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedUnlock(void) const {
		impl->lock.Unlock();
	};

	ImplType impl;
};

#endif