/*
################################################################################################################
#################### Energy Demand Management System for Water Distribution Systems ############################
#################### Developed by the Center for Water-Energy Efficiency, 2019 - 2020 ##########################
################################################################################################################
*/

#ifndef __THREAD_H__
#define __THREAD_H__

#define cweeMultithreading_UseCriticalSection
#ifndef cweeMultithreading_UseCriticalSection
#include <mutex> // #include "boost/thread/mutex.hpp"
#endif









/*
ATOMIC INTEGER COLLECTION
*/
template <typename type>
class cweeSysInterlockedValue {
public:
	cweeSysInterlockedValue() : value((type)0) {};
	cweeSysInterlockedValue(const type& a) : value(a) {};
	cweeSysInterlockedValue(const cweeSysInterlockedValue& other) : value(other.GetValue()) {};
	cweeSysInterlockedValue& operator=(const cweeSysInterlockedValue& other) { SetValue(other.GetValue()); return *this; };
	cweeSysInterlockedValue& operator=(const type& newSource) { SetValue(newSource); return *this; };

	operator type() { return GetValue(); };
	operator type() const { return GetValue(); };

	friend cweeSysInterlockedValue operator+(const type& i, const cweeSysInterlockedValue& b) { cweeSysInterlockedValue out(b); out.Add(i); return out; };
	friend cweeSysInterlockedValue operator+(const cweeSysInterlockedValue& b, const type& i) { cweeSysInterlockedValue out(b); out.Add(i); return out; };
	friend cweeSysInterlockedValue operator-(const type& i, const cweeSysInterlockedValue& b) { cweeSysInterlockedValue out(i); out.Add(-b.GetValue()); return out; };
	friend cweeSysInterlockedValue operator-(const cweeSysInterlockedValue& b, const type& i) { cweeSysInterlockedValue out(b); out.Add(-i); return out; };
	friend cweeSysInterlockedValue operator/(const type& i, const cweeSysInterlockedValue& b) { return i / b.GetValue(); };
	friend cweeSysInterlockedValue operator/(const cweeSysInterlockedValue& b, const type& i) { return b.GetValue() / i; };

	cweeSysInterlockedValue& operator+=(int i) { Add(i); return *this; };
	cweeSysInterlockedValue& operator-=(int i) { Add(-i); return *this; };

	cweeSysInterlockedValue& operator+=(const cweeSysInterlockedValue& i) { Add(i.GetValue()); return *this; };
	cweeSysInterlockedValue& operator-=(const cweeSysInterlockedValue& i) { Add(-i.GetValue()); return *this; };

	bool operator==(const type& i) { return i == GetValue(); };
	bool operator!=(const type& i) { return i != GetValue(); };
	bool operator==(const cweeSysInterlockedValue& i) { return i.GetValue() == GetValue(); };
	bool operator!=(const cweeSysInterlockedValue& i) { return i.GetValue() != GetValue(); };

	friend bool operator<=(const type& i, const cweeSysInterlockedValue& b) { return i <= b.GetValue(); };
	friend bool operator<=(const cweeSysInterlockedValue& b, const type& i) { return i > b.GetValue(); };
	friend bool operator>=(const type& i, const cweeSysInterlockedValue& b) { return i >= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedValue& b, const type& i) { return i < b.GetValue(); };
	friend bool operator>(const type& i, const cweeSysInterlockedValue& b) { return i > b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedValue& b, const type& i) { return i <= b.GetValue(); };
	friend bool operator<(const type& i, const cweeSysInterlockedValue& b) { return i < b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedValue& b, const type& i) { return i >= b.GetValue(); };

	friend bool operator<=(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() <= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() >= b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() > b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() < b.GetValue(); };

	type					Increment() { return Sys_InterlockedIncrementV(value); } // atomically increments the integer and returns the new value
	type					Decrement() { return Sys_InterlockedDecrementV(value); } // atomically decrements the integer and returns the new value
	type					Add(const type& v) { return Sys_InterlockedAddV(value, (type)v); } // atomically adds a value to the integer and returns the new value
	type					Sub(const type& v) { return Sys_InterlockedSubV(value, (type)v); } // atomically subtracts a value from the integer and returns the new value
	type					GetValue() const { return value; } // returns the current value of the integer
	void				    SetValue(const type& v) { value = v; };

private:
	type	            value;
};

class cweeSysInterlockedInteger {
public:
	constexpr cweeSysInterlockedInteger() noexcept : value(0) {};
	constexpr cweeSysInterlockedInteger(int a) noexcept : value(a) {};
	cweeSysInterlockedInteger(const cweeSysInterlockedInteger& other) : value(other.GetValue()) {};
	cweeSysInterlockedInteger& operator=(const cweeSysInterlockedInteger& other) { SetValue(other.GetValue()); return *this; };
	cweeSysInterlockedInteger& operator=(int newSource) { SetValue(newSource); return *this; };

	explicit operator int() { return GetValue(); };
	explicit operator int() const { return GetValue(); };

	explicit operator bool() { if (GetValue() == 0) return false; else return true; };
	explicit operator bool() const { if (GetValue() == 0) return false; else return true; };

	friend cweeSysInterlockedInteger operator+(int i, const cweeSysInterlockedInteger& b) { cweeSysInterlockedInteger out(b); out.Add(i); return out; };
	friend cweeSysInterlockedInteger operator+(const cweeSysInterlockedInteger& b, int i) { cweeSysInterlockedInteger out(b); out.Add(i); return out; };
	friend cweeSysInterlockedInteger operator-(int i, const cweeSysInterlockedInteger& b) { cweeSysInterlockedInteger out(i); out.Add(-b.GetValue()); return out; };
	friend cweeSysInterlockedInteger operator-(const cweeSysInterlockedInteger& b, int i) { cweeSysInterlockedInteger out(b); out.Add(-i); return out; };
	friend cweeSysInterlockedInteger operator/(int i, const cweeSysInterlockedInteger& b) { return i / b.GetValue(); };
	friend cweeSysInterlockedInteger operator/(const cweeSysInterlockedInteger& b, int i) { return b.GetValue() / i; };

	friend bool operator<=(int i, const cweeSysInterlockedInteger& b) { return i <= b.GetValue(); };
	friend bool operator<=(const cweeSysInterlockedInteger& b, int i) { return i > b.GetValue(); };
	friend bool operator>=(int i, const cweeSysInterlockedInteger& b) { return i >= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedInteger& b, int i) { return i < b.GetValue(); };
	friend bool operator>(int i, const cweeSysInterlockedInteger& b) { return i > b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedInteger& b, int i) { return i <= b.GetValue(); };
	friend bool operator<(int i, const cweeSysInterlockedInteger& b) { return i < b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedInteger& b, int i) { return i >= b.GetValue(); };

	friend bool operator<=(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() <= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() >= b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() > b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() < b.GetValue(); };

	cweeSysInterlockedInteger& operator+=(int i) { Add(i); return *this; };
	cweeSysInterlockedInteger& operator-=(int i) { Add(-i); return *this; };

	cweeSysInterlockedInteger& operator+=(const cweeSysInterlockedInteger& i) { Add(i.GetValue()); return *this; };
	cweeSysInterlockedInteger& operator-=(const cweeSysInterlockedInteger& i) { Add(-i.GetValue()); return *this; };

	bool operator==(int i) { return i == GetValue(); };
	bool operator!=(int i) { return i != GetValue(); };
	bool operator==(const cweeSysInterlockedInteger& i) { return i.GetValue() == GetValue(); };
	bool operator!=(const cweeSysInterlockedInteger& i) { return i.GetValue() != GetValue(); };

	int					Increment() { return Sys_InterlockedIncrement(value); } // atomically increments the integer and returns the new value
	int					Decrement() { return Sys_InterlockedDecrement(value); } // atomically decrements the integer and returns the new value
	int					Add(int v) { return Sys_InterlockedAdd(value, (interlockedInt_t)v); } // atomically adds a value to the integer and returns the new value
	int					Sub(int v) { return Sys_InterlockedSub(value, (interlockedInt_t)v); } // atomically subtracts a value from the integer and returns the new value
	int					GetValue() const { return Sys_InterlockedAdd(const_cast<interlockedInt_t&>(value), 0); } // returns the current value of the integer
	void				SetValue(int v) {
		Sys_InterlockedAdd(value, (interlockedInt_t)(v - GetValue()));
	};

	bool				TryIncrementTo(int n) {
		if (Increment() == n) {
			return true;
		}
		Decrement();
		return false;
	};

private:
	interlockedInt_t	value;
};

/*
ATOMIC POINTER COLLECTION
*/
template< typename T >
class cweeSysInterlockedPointer {
public:
	constexpr cweeSysInterlockedPointer() noexcept : ptr(nullptr) {}
	constexpr cweeSysInterlockedPointer(std::nullptr_t) noexcept : ptr(nullptr) {}
	constexpr cweeSysInterlockedPointer(T* newSource) noexcept : ptr(newSource) {}
	constexpr cweeSysInterlockedPointer(const cweeSysInterlockedPointer& other) : ptr(other.Get()) {};
	cweeSysInterlockedPointer& operator=(const cweeSysInterlockedPointer& other) { Set(other.Get()); return *this; };
	cweeSysInterlockedPointer& operator=(T* newSource) { Set(newSource); return *this; };
	~cweeSysInterlockedPointer() { 
		ptr = nullptr; 
	};

	explicit operator bool() { return ptr; };
	explicit operator bool() const { return ptr; };

	constexpr operator T* () noexcept { return ptr; };
	constexpr operator const T* () const noexcept { return ptr; };

	T* Set(T* newPtr) { // atomically sets the pointer and returns the previous pointer value
		return (T*)Sys_InterlockedExchangePointer((void*&)ptr, (void*)newPtr);
	}
	T* CompareExchange(T* comparePtr, T* newPtr) { // atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr'
		return (T*)Sys_InterlockedCompareExchangePointer((void*&)ptr, (void*)comparePtr, (void*)newPtr); // ptr = ( ptr == comparePtr ) ? newPtr : ptr
	}

	constexpr T* operator->() noexcept {
		return Get();
	};
	constexpr const T* operator->() const noexcept {
		return Get();
	};
	constexpr T* Get() noexcept { return ptr; } // returns the current value of the pointer
	constexpr T* Get() const noexcept { return ptr; } // returns the current value of the pointer

private:
	T* ptr;
};


class cweeSysMutexImpl {
public:
	cweeSysMutexImpl() noexcept { Sys_MutexCreate(Handle); };
	~cweeSysMutexImpl()	noexcept { Sys_MutexDestroy(Handle); };
	bool Lock(bool blocking = true) noexcept { return Sys_MutexLock(Handle, blocking); };
	void Unlock() noexcept { Sys_MutexUnlock(Handle); };

	cweeSysMutexImpl(const cweeSysMutexImpl&) = delete;
	cweeSysMutexImpl(cweeSysMutexImpl&&) = delete;
	cweeSysMutexImpl& operator=(cweeSysMutexImpl const&) = delete;
	cweeSysMutexImpl& operator=(cweeSysMutexImpl&&) = delete;

protected:
	mutexHandle_t Handle;
};

#if 0
#define usecweeMutexForSharedPtr
template <typename type>
class cweeSharedPtr {
public:
#pragma region Typedefs
#ifdef usecweeMutexForSharedPtr
	typedef cweeSysMutexImpl			MutexType;
#else
	typedef cweeSysInterlockedInteger	MutexType;
#endif
	typedef type						Type;
	typedef type*						PtrType;
	typedef std::remove_extent<type>	element_type;
#pragma endregion 

#pragma region Boolean Operators
	constexpr explicit operator bool() const {
		return (bool)Get(); 
	};

	friend bool operator==(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept {
		return a.Data().Get() == b.Data().Get();
	};
	friend bool operator!=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept {
		return a.Data().Get() != b.Data().Get();
	};
	friend bool operator<(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept {
		return a.Data().Get() < b.Data().Get();
	};
	friend bool operator<=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept {
		return a.Data().Get() <= b.Data().Get();
	};
	friend bool operator>(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept {
		return a.Data().Get() > b.Data().Get();
	};
	friend bool operator>=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept {
		return a.Data().Get() >= b.Data().Get();
	};

	friend bool operator==(const cweeSharedPtr& a, std::nullptr_t) noexcept {
		return a.Data().Get() == nullptr;
	};
	friend bool operator!=(const cweeSharedPtr& a, std::nullptr_t) noexcept {
		return a.Data().Get() != nullptr;
	};
	friend bool operator<(const cweeSharedPtr& a, std::nullptr_t) noexcept {
		return a.Data().Get() < nullptr;
	};
	friend bool operator<=(const cweeSharedPtr& a, std::nullptr_t) noexcept {
		return a.Data().Get() <= nullptr;
	};
	friend bool operator>(const cweeSharedPtr& a, std::nullptr_t) noexcept {
		return a.Data().Get() > nullptr;
	};
	friend bool operator>=(const cweeSharedPtr& a, std::nullptr_t) noexcept {
		return a.Data().Get() >= nullptr;
	};

	friend bool operator==(std::nullptr_t, const cweeSharedPtr& a) noexcept {
		return nullptr == a.Data().Get();
	};
	friend bool operator!=(std::nullptr_t, const cweeSharedPtr& a) noexcept {
		return nullptr != a.Data().Get();
	};
	friend bool operator<(std::nullptr_t, const cweeSharedPtr& a) noexcept {
		return nullptr < a.Data().Get();
	};
	friend bool operator<=(std::nullptr_t, const cweeSharedPtr& a) noexcept {
		return nullptr <= a.Data().Get();
	};
	friend bool operator>(std::nullptr_t, const cweeSharedPtr& a) noexcept {
		return nullptr > a.Data().Get();
	};
	friend bool operator>=(std::nullptr_t, const cweeSharedPtr& a) noexcept {
		return nullptr >= a.Data().Get();
	};
#pragma endregion

#pragma region Class Methods Without Parameters
	/*! Get Pointer to Managed Data */
	constexpr cweeSysInterlockedPointer<type>& Data() const noexcept {
		return _ptr; 
	};
	/*! Get Pointer to Managed Data */
	cweeSysInterlockedPointer<std::pair<cweeSysInterlockedInteger, MutexType*>>& Counter() const noexcept {
		return _count_and_lock; 
	};
	/*! Get Pointer to Managed Data */
	MutexType* GetLock() const noexcept {
		AUTO c = Counter().Get();
		if (c) {
			if (c->second) {
				return c->second;
			}
		}
		return &SharedPtrLock;
	};

	/*! Lock Entire Managed Pointer */
	MutexType* Lock() const {
		AUTO lock = GetLock();
#ifdef usecweeMutexForSharedPtr
		lock->Lock();
#else
		while (!lock->TryIncrementTo(1)) {};
#endif
		return lock;			
	};
	static void Lock(MutexType* a) {	
#ifdef usecweeMutexForSharedPtr
		a->Unlock();
#else
		while (!a->TryIncrementTo(1)) {};
#endif
	};
	static int CompareAndLock(MutexType* a, MutexType* b) {
		if (a == b && a == nullptr) {
			return 0;
		} 
		else if (a == b) {
			Lock(a);
			return 1;
		}
		else {
			Lock(a);
			Lock(b);
			return 2;
		}
	
	}
	static void CompareAndUnlock(MutexType* a, MutexType* b, int mode) {
		switch (mode) {
		case 2: 
			Unlock(b);
		case 1: 
			Unlock(a);
		default: 
			return;
		}
	}

	/*! Unlock Entire Managed Pointer */
	static void Unlock(MutexType* lock) {
		if (lock) {
#ifdef usecweeMutexForSharedPtr
			lock->Unlock();
#else
			lock->Decrement();
#endif
		}
	};
	/*! Increase Managed Pointer Count */
	void IncrementCount() const noexcept {
		AUTO L1 = Lock();
		{
			AUTO ptr = Counter().Get();
			if (ptr) {
				
				ptr->first.Increment();
			}
		}
		Unlock(L1);
	};
	/*! Decrease Managed Pointer Count */
	void DecrementCount() const noexcept {
		AUTO L1 = Lock();
		{
			AUTO ptr = Counter().Get();
			if (ptr) {
				if (ptr->first.Decrement() == 0) {
					// time to delete the underlying data
					AUTO p1 = Counter().Set(nullptr);
					if (p1) {
						delete p1;
					}
					AUTO p2 = Data().Set(nullptr);
					if (p2) {
						delete p2;
					}
				}
			}
		}
		Unlock(L1);
	};

	/*! Create a shared pointer from this one with an added const component to the underlying managed object type */
	cweeSharedPtr<typename std::add_const<type>::type> ConstReference() const noexcept {
		cweeSharedPtr<typename std::add_const<type>::type> out;
		AUTO L1 = Lock();
		{
			out.Counter() = Counter();
			out.Data() = (typename std::add_pointer<typename std::add_const<type>::type>::type)(Data().Get());
		}
		Unlock(L1);

		IncrementCount();

		return out;
	};
	/*! Create a shared pointer from this one with an removed const component to the underlying managed object type */
	cweeSharedPtr<typename std::remove_const<type>::type> RemoveConstReference() const noexcept {
		cweeSharedPtr<typename std::remove_const<type>::type> out;
		AUTO L1 = Lock();
		{
			out.Counter() = Counter();
			out.Data() = const_cast<typename std::add_pointer<typename std::remove_const<type>::type>::type>(Data().Get());
		}
		Unlock(L1);

		IncrementCount();

		return out;
	};
	/*! Create a shared pointer from this one that casts the underlying type to another underlying type (i.e. for derived types) */
	template<typename astype>  cweeSharedPtr<astype> CastReference() const noexcept {
		cweeSharedPtr<astype> out;
		AUTO L1 = Lock();
		{
			out.Counter() = Counter();
			out.Data() = dynamic_cast<typename std::remove_const<astype>::type*>(const_cast<typename std::remove_const<type>::type*>(Data().Get()));
		}
		Unlock(L1);

		IncrementCount();

		return out;
	};
#pragma endregion

#pragma region Data Access
	/*! Pointer Access (never throws) (no race conditions) */
	constexpr type* operator->() const noexcept {
		return Data().Get();
	};
	/*! Pointer Access (never throws) (no race conditions) */
	constexpr type* Get() const noexcept {
		return Data().Get();
	};
	/*! Array Access (can throw) (no race conditions) */
	type& operator[](std::ptrdiff_t idx) const {
		AUTO x = Get();
		if (x) return x[idx];
		throw(std::exception("cweeSharedPtr was null with indexed access"));
	};
	/*! Reference Access (can throw) (no race conditions) */
	type& operator*() const {
		AUTO x = Get();
		if (x) return *x;
		throw(std::exception("cweeSharedPtr was null with reference access"));
	};
#pragma endregion

#pragma region Create Shared Pointer From Nothing
	/*! Create an empty pointer (nullptr) */
	constexpr cweeSharedPtr() noexcept : _count_and_lock(nullptr), _ptr(nullptr) {};
	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtr(type* source) noexcept : _count_and_lock(new std::pair<cweeSysInterlockedInteger, MutexType*>(cweeSysInterlockedInteger(1), &SharedPtrLock)), _ptr(source) {};
	/*! Instantiate a shared pointer by handing over a data source to be copied. */
	template<typename = std::enable_if<!std::is_same<Type, std::decay<Type>>::value>>
	cweeSharedPtr(const type& source) noexcept : _count_and_lock(new std::pair<cweeSysInterlockedInteger, MutexType*>(cweeSysInterlockedInteger(1), &SharedPtrLock)), _ptr(new type(source)) {};
	/*! Instantiate a shared pointer by handing over a data source to be copied. */
	template<typename = std::enable_if<!std::is_same<Type, std::decay<Type>>::value>>
	cweeSharedPtr(type&& source) noexcept : _count_and_lock(new std::pair<cweeSysInterlockedInteger, MutexType*>(cweeSysInterlockedInteger(1), &SharedPtrLock)), _ptr(new type(std::forward<type>(source))) {};
	/*! Destroy this instance of the shared pointer and potentially delete the data */
	~cweeSharedPtr() noexcept { this->DecrementCount(); };
#pragma endregion

#pragma region Sharing the Shared Pointer
	/*! Share ownership */
	cweeSharedPtr(const cweeSharedPtr<type>& other) noexcept : _count_and_lock(other.Counter()), _ptr(other.Data())
	{
		IncrementCount();
	};

	/*! Share ownership while casting to a new type*/
	template <typename T, typename = std::enable_if_t<!std::is_same_v<type, T>> >
	cweeSharedPtr(const cweeSharedPtr<T>& other) noexcept : _count_and_lock(nullptr), _ptr(nullptr)
	{
		AUTO x = other.CastReference<type>();
		Swap(x);
	};

	/*! Share ownership */
	cweeSharedPtr<type>& operator=(const cweeSharedPtr<type>& other) noexcept {
		other.IncrementCount();
		this->DecrementCount();

		AUTO c1 = GetLock(); AUTO c2 = other.GetLock();
		AUTO lockMode = CompareAndLock(c1, c2);
		{
			Counter() = other.Counter();
			Data() = other.Data();
		}
		CompareAndUnlock(c1, c2, lockMode);

		return *this;
	};

	/*! Share ownership while casting to a new type */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<type, T>> >
	cweeSharedPtr<type>& operator=(const cweeSharedPtr<T>& other) noexcept {
		AUTO x = other.CastReference<type>();
		Swap(x);
		return *this;
	};

	/*! Take ownership */
	cweeSharedPtr(cweeSharedPtr<type>&& other)  noexcept : _count_and_lock(nullptr), _ptr(nullptr)
	{
		AUTO L1 = other.Lock();
		{
			Counter().Set(other.Counter().Set(nullptr));
			Data().Set(other.Data().Set(nullptr));
		}
		Unlock(L1);
	};

	/*! Take ownership while casting to a new type*/
	template <typename T, typename = std::enable_if_t<!std::is_same_v<type, T>> >
	cweeSharedPtr(cweeSharedPtr<T>&& other)  noexcept : _count_and_lock(nullptr), _ptr(nullptr)
	{
		cweeSharedPtr<T> y(other);
		AUTO x = y.CastReference<type>();
		Swap(x);
	};

	/*! Take ownership */
	cweeSharedPtr<type>& operator=(cweeSharedPtr<type>&& other) noexcept {
		reset();

		AUTO L1 = Lock();
		{
			Counter().Set(other.Counter().Set(nullptr));
			Data().Set(other.Data().Set(nullptr));
		}
		Unlock(L1);

		return *this;
	};
#pragma endregion

#pragma region Swap with Pointers
	void Swap(cweeSharedPtr<type>& other) {
		AUTO c1 = GetLock(); AUTO c2 = other.GetLock();
		AUTO lockMode = CompareAndLock(c1, c2);
		{
			other.Counter().Set(Counter().Set(other.Counter().Get()));
			other.Data().Set(Data().Set(other.Data().Get()));
		}
		CompareAndUnlock(c1, c2, lockMode);
	};
	type* SwapPtr(type* newptr) {
		type* out;
		AUTO L1 = Lock();
		{
			out = Data().Set(newptr);
		}
		Unlock(L1);
		return out;
	};
#pragma endregion

#pragma region "std::shared_ptr" Compatability Methods
	/*! Get access to the underlying data in a thread-safe way. (The underlying data isn't necessarily thread-safe, but accessing the pointer itself has no data race.) */
	constexpr type* get() const noexcept {
		return Data().Get();
	};
	/*! Empty the current data pointer and free the data. */
	void reset() {
		AUTO x = cweeSharedPtr();
		Swap(x);
	};
	/*! Exchange the current data with the provided new data. */
	template< class Y > void reset(Y* ptr) {
		AUTO x = cweeSharedPtr(ptr);
		Swap(x);
	};
	/*! Swap shared pointers */
	void swap(cweeSharedPtr<type>& r) {
		Swap(r);
	};
	/*! Number of shared pointers with access to the underlying data, including this one. */
	long use_count() const noexcept {
		long out;
		AUTO L1 = Lock();
		{
			AUTO x = Counter().Get();
			if (x) {
				out = x->first.GetValue();
			}
			else {
				out = 0;
			}
		}
		Unlock(L1);
		return out;
	};
	/*! Releases the held pointer, stops counting, and will not delete the pointer on destruction */
	type* release() {
		type* out;
		std::pair<cweeSysInterlockedInteger, MutexType*>* lock;
		AUTO L1 = Lock();
		{
			lock = Counter().Set(nullptr);
			out = Data().Set(nullptr);
		}
		Unlock(L1);
		if (lock) { delete lock; }

		return out;
	};
#pragma endregion

private:
#pragma region Data
	static MutexType SharedPtrLock;
	mutable cweeSysInterlockedPointer<type> _ptr;
	mutable cweeSysInterlockedPointer<std::pair<cweeSysInterlockedInteger, MutexType*>> _count_and_lock;
#pragma endregion

};
template <typename type> typename cweeSharedPtr<type>::MutexType cweeSharedPtr<type>::SharedPtrLock = cweeSharedPtr<type>::MutexType();
#else
#define allowCweeSharedPtrCaptureByValue
class Details_Interface {
protected:
	Details_Interface() noexcept {};	
	explicit Details_Interface(Details_Interface const&) = delete;
	explicit Details_Interface(Details_Interface&&) = delete;
	Details_Interface& operator=(Details_Interface const&) = delete;
	Details_Interface& operator=(Details_Interface&&) = delete;
public:
	virtual ~Details_Interface() noexcept {};
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept = 0;
};
/* Interface for simply allowing the sharing of a virtual pointer */
class cweeSharedPtr_Data_Interface {
protected:
	cweeSharedPtr_Data_Interface() noexcept {};	
	virtual ~cweeSharedPtr_Data_Interface() noexcept {};
	explicit cweeSharedPtr_Data_Interface(cweeSharedPtr_Data_Interface const& other) noexcept = delete;
	explicit cweeSharedPtr_Data_Interface(cweeSharedPtr_Data_Interface&& other) noexcept = delete;
	cweeSharedPtr_Data_Interface& operator=(cweeSharedPtr_Data_Interface const& other) = delete;
	cweeSharedPtr_Data_Interface& operator=(cweeSharedPtr_Data_Interface&& other) = delete;	
public:
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual cweeSysInterlockedPointer<Details_Interface>& ptr() const noexcept = 0;
};
template <typename type> class cweeSharedPtr {
#pragma region Class Defs
public:
	using swappablePtr = cweeSysInterlockedPointer<type>;
#ifdef allowCweeSharedPtrCaptureByValue
	class details_withData final : public Details_Interface {
	public:
		explicit details_withData() = delete;
		~details_withData() noexcept {};
		explicit details_withData(details_withData const&) = delete;
		explicit details_withData(details_withData&&) = delete;
		details_withData& operator=(details_withData const&) = delete;
		details_withData& operator=(details_withData&&) = delete;

		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
		explicit details_withData(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& obj) noexcept :
			count(1), d(std::forward<type>(obj)), p(&d) {};
		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
		explicit details_withData(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>const& obj) noexcept :
			count(1), d(obj), p(&d) {};

		void* source() const noexcept final { return (void*)(type*)(p.Get()); };
		int	use_count() const noexcept final { return count.GetValue(); };
		void increment() const noexcept final { count.Increment(); };
		int decrement() const noexcept final { 
			int i = count.Decrement();
			if (i == 0) {
				p.Set(nullptr);
			}
			return i;
		};

		std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept final {
			return [](Details_Interface* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtr<type>::details_withData*>(p);
					if (q) {
						delete q;
					}
					else {
						delete p; // likely a  leak
					}
				}
			};
		};
	private:
		mutable cweeSysInterlockedInteger count;
		type d;
		mutable swappablePtr p;
	};
#endif
	class details final : public Details_Interface {
	public:
		explicit details() = delete;
		~details() noexcept {}; 
		explicit details(details const&) = delete;
		explicit details(details&&) = delete;
		details& operator=(details const&) = delete;
		details& operator=(details&&) = delete;
		explicit details(type* _source, std::function<void(type*)> _destroy) noexcept :
			p(_source), count(1), deleter(std::move(_destroy)) {};
		void* source() const noexcept final { return static_cast<void*>(p); };
		int	use_count() const noexcept final {
			return count.GetValue();
		};
		void increment() const noexcept final {
			count.Increment();
		};
		int decrement() const noexcept final {
			int i = count.Decrement();
			if (i == 0) {
				deleter(p.Set(nullptr));
			}
			return i;
		};
		std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept final {
			return [](Details_Interface* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtr<type>::details*>(p);
					if (q) {
						delete q;
					}
					else {
						delete p; // likely a  leak
					}
				}
			};
		};
	private:
		mutable swappablePtr p;
		mutable cweeSysInterlockedInteger count;
		mutable std::function<void(type*)> deleter;
	};
	class details_no_destructor final : public Details_Interface {
	public:
		explicit details_no_destructor() = delete;
		~details_no_destructor() noexcept {}; 
		explicit details_no_destructor(details_no_destructor const&) = delete;
		explicit details_no_destructor(details_no_destructor&&) = delete;
		details_no_destructor& operator=(details_no_destructor const&) = delete;
		details_no_destructor& operator=(details_no_destructor&&) = delete;
		explicit details_no_destructor(type* _source) noexcept :
			p(_source), count(1) {};
		void* source() const noexcept final {
			return (void*)p.Get();
		};
		int	use_count() const noexcept final {
			return count.GetValue();
		};
		void increment() const noexcept final {
			count.Increment();
		};
		int decrement() const noexcept final {
			int i = count.Decrement();
			if (i == 0) {
				AUTO P = p.Set(nullptr);
				if (P) {
					delete P;
				}
			}
			return i;
		};
		std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept final { 
			return [](Details_Interface* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtr<type>::details_no_destructor*>(p);
					if (q) {
						delete q;
					}
					else {
						delete p; // likely a  leak
					}
				}
			};
		};
	private:
		mutable swappablePtr p;
		mutable cweeSysInterlockedInteger count;
	};
	class cweeSharedPtr_DataImpl final : public cweeSharedPtr_Data_Interface {
	public:
		explicit cweeSharedPtr_DataImpl() = delete;
		~cweeSharedPtr_DataImpl() noexcept { decrement(); };
		explicit cweeSharedPtr_DataImpl(cweeSharedPtr_DataImpl const&) = delete;
		explicit cweeSharedPtr_DataImpl(cweeSharedPtr_DataImpl&&) = delete;
		cweeSharedPtr_DataImpl& operator=(cweeSharedPtr_DataImpl const&) = delete;
		cweeSharedPtr_DataImpl& operator=(cweeSharedPtr_DataImpl&&) = delete;

		explicit cweeSharedPtr_DataImpl(const cweeSharedPtr_Data_Interface& other, std::function<type* (void*)> _getter) noexcept : 
			det(other.ptr()), m_getter(std::move(_getter)) { increment(); };

		explicit cweeSharedPtr_DataImpl(type* _source, std::function<void(type*)> _on_destroy, std::function<type* (void*)> _getter) noexcept : 
			det(new details(_source, std::move(_on_destroy))), m_getter(std::move(_getter)) {};
		explicit cweeSharedPtr_DataImpl(type* _source, std::function<type* (void*)> _getter) noexcept : 
			det(new details_no_destructor(_source)), m_getter(std::move(_getter)) {};

#ifdef allowCweeSharedPtrCaptureByValue
		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> > 
		explicit cweeSharedPtr_DataImpl(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& _obj, std::function<type* (void*)> _getter) noexcept : 
			det(new details_withData(std::forward<type>(_obj))), m_getter(std::move(_getter)) {};

		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> > 
		explicit cweeSharedPtr_DataImpl(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>const& _obj, std::function<type* (void*)> _getter) noexcept : 
			det(new details_withData(_obj)), m_getter(std::move(_getter)) {};
#endif

		void* source() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				return p->source();
			}
			return nullptr;
		};
		int	use_count() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				return p->use_count();
			}
			return 0;
		};
		void increment() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				p->increment();
			}
		};
		int decrement() const noexcept final {
			int i = -1;
			AUTO p = det.Get();
			if (p) {
				i = p->decrement();
				if (i == 0) {
					{
						AUTO p2 = det.Set(nullptr);
						if (p2 != nullptr) {
							AUTO func = p2->Details_Interface_Deleter();
							func(p2);			
						}
						else {
							delete p; // likely a leak
						}
					}
				}
			}
			return i;
		};
		cweeSysInterlockedPointer<Details_Interface>& ptr() const noexcept final { return det; };

	public:
		std::function<type* (void*)>							m_getter;
		mutable cweeSysInterlockedPointer<Details_Interface>	det;
	};
#pragma endregion 
#pragma region Type Defs
public:
	typedef type						Type;
	typedef type* PtrType;
#pragma endregion 
#pragma region Data Members
public:
	mutable cweeSysInterlockedInteger									mutex;
	mutable cweeSysInterlockedPointer<cweeSharedPtr_DataImpl>			m_data;
#pragma endregion 
#pragma region Create or Destroy
public:
	/*! Create an empty pointer (nullptr) */
	constexpr cweeSharedPtr() noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a nullptr */
	constexpr cweeSharedPtr(std::nullptr_t) noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtr(PtrType source) noexcept : mutex(0), m_data(InitData(source)) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtr(PtrType source, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(source, std::move(destroy))) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtr(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitData(source, std::move(destroy), std::move(_getter))) {};

	/*! instantiate with a constructor and destructor */
	cweeSharedPtr(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(std::move(create), std::move(destroy))) {};

	/*! instantiate from another ptr */
	cweeSharedPtr(cweeSharedPtr const& samePtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr)) {};
	cweeSharedPtr(cweeSharedPtr&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr>(other))) {};

	/*! instantiate from another ptr with complex "get" instructions */
	cweeSharedPtr(cweeSharedPtr const& samePtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr, std::move(_getter))) {};
	cweeSharedPtr(cweeSharedPtr&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr>(other), std::move(_getter))) {};

	/*! instantiate from another ptr with a different Type, using basic cast operations */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtr(cweeSharedPtr<T> const& similarPtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, [](void* p) constexpr -> PtrType {
		return static_cast<PtrType>((T*)p); // dynamic_cast
	})) {};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtr(cweeSharedPtr<T>&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr<T>>(other), [](void* p) constexpr -> PtrType {
		return static_cast<PtrType>((T*)p); // dynamic_cast
	})) {};

	/*! instantiate from another ptr with a different Type with complex "get" instructions */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtr(cweeSharedPtr<T> const& similarPtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, std::move(_getter))) {};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtr(cweeSharedPtr<T>&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr<T>>(other), std::move(_getter))) {};

	template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
	cweeSharedPtr(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type> const& source) noexcept : mutex(0), m_data(
#ifdef allowCweeSharedPtrCaptureByValue
		new cweeSharedPtr_DataImpl(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; }))
#else
		InitData(new Type(source))
#endif
	) {};

	template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
	cweeSharedPtr(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& source) noexcept : mutex(0), m_data(
#ifdef allowCweeSharedPtrCaptureByValue
		new cweeSharedPtr_DataImpl(std::forward<std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>>(source), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; }))
#else
		InitData(new Type(std::forward<Type>(source)))
#endif
	) {};

	/*! Destroy this instance of the shared pointer and potentially delete the data */
	~cweeSharedPtr() noexcept {
		ClearData();
	};
#pragma endregion
#pragma region Internal Support Functions
public:
	PtrType Get() const {
		PtrType out(nullptr);

		Lock();
		auto* d = m_data.Get();
		if (d) {
			out = d->m_getter(d->source());
		}
		Unlock();
		return out;
	};

public:
	void Lock() const noexcept {
		while (mutex.Increment() != 1) {
			mutex.Decrement();
		}
	};
	void Unlock() const noexcept {
		mutex.Decrement();
	};
	PtrType UnsafeGet() const noexcept {
		PtrType out(nullptr);
		auto* d = m_data.Get();
		if (d) {
			out = d->m_getter(d->source());
		}
		return out;
	};
	template <typename... Args> void UnsafeSet(Args... Fargs) {
		UnsafeSetData(cweeSharedPtr<type>::InitData(Fargs...));
	};
protected:
	template <typename T> static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T> const& Ptr) noexcept {
		return InitDataFromAnotherPtr(Ptr, [](void* p) constexpr -> PtrType { return (PtrType)p; });		
	};
	template <typename T> static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T> const& Ptr, std::function<PtrType(void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtr<T>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Get();
		if (p) {
			AUTO d = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T>&& Ptr) noexcept {
		return InitDataFromAnotherPtr(std::forward<cweeSharedPtr<T>>(Ptr), [](void* p) constexpr -> PtrType { return (PtrType)p; });
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T>&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtr<T>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr&& Ptr) noexcept {
		cweeSharedPtr_DataImpl* p;
		Ptr.Lock();
		p = Ptr.m_data.Set(nullptr);
		Ptr.Unlock();
		return p;
	};
	static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};

	static AUTO InitData(PtrType source, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtr_DataImpl(source, std::move(from));
	};
	static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtr_DataImpl(source, std::move(destroy), std::move(from));
	};
	static AUTO InitData(PtrType source) noexcept { return InitData(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy) noexcept { return InitData(source, std::move(destroy), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	static AUTO InitData(std::function<PtrType()> create) noexcept { return InitData(create()); };
	static AUTO InitData(std::function<PtrType()> create, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(from)); };	
	static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept { return InitData(create(), std::move(destroy)); };
	static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(destroy), std::move(from)); };

	void SetData(cweeSharedPtr_DataImpl* p) const noexcept {
		Lock();
		AUTO d = m_data.Set(p);
		Unlock();
		if (d) {
			delete d;
		}
	};
	void UnsafeSetData(cweeSharedPtr_DataImpl* p) const noexcept {
		AUTO d = m_data.Set(p);
		if (d) {
			delete d;
		}
	};
	void ClearData() const noexcept { SetData(nullptr); };

#pragma endregion 
#pragma region Boolean Operators
public:
	constexpr explicit operator bool() const { return m_data.Get(); };
	friend bool operator==(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() == b.Get(); };
	friend bool operator!=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() != b.Get(); };
	friend bool operator<(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() < b.Get(); };
	friend bool operator<=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() <= b.Get(); };
	friend bool operator>(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() > b.Get(); };
	friend bool operator>=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() >= b.Get(); };
	friend bool operator==(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() == nullptr; };
	friend bool operator!=(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() != nullptr; };
	friend bool operator<(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() < nullptr; };
	friend bool operator<=(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() <= nullptr; };
	friend bool operator>(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() > nullptr; };
	friend bool operator>=(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() >= nullptr; };
	friend bool operator==(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr == a.m_data.Get(); };
	friend bool operator!=(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr != a.m_data.Get(); };
	friend bool operator<(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr < a.m_data.Get(); };
	friend bool operator<=(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr <= a.m_data.Get(); };
	friend bool operator>(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr > a.m_data.Get(); };
	friend bool operator>=(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr >= a.m_data.Get(); };
#pragma endregion
#pragma region Ptr Casting
public:
	/*! Create a shared pointer from this one with an added const component to the underlying managed object type */
	cweeSharedPtr<typename std::add_const<Type>::type> ConstReference() const noexcept {
		typedef typename std::add_const<Type>::type newT;
		cweeSharedPtr<newT> out;
		typedef typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl oldDI;
		typedef typename cweeSharedPtr<newT>::cweeSharedPtr_DataImpl newDI;
		typedef typename cweeSharedPtr<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return (newP)(p);
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
	/*! Create a shared pointer from this one with an removed const component to the underlying managed object type */
	cweeSharedPtr<typename std::remove_const<Type>::type> RemoveConstReference() const noexcept {
		typedef typename std::remove_const<Type>::type newT;
		cweeSharedPtr<newT> out;
		typedef typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl oldDI;
		typedef typename cweeSharedPtr<newT>::cweeSharedPtr_DataImpl newDI;
		typedef typename cweeSharedPtr<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return (newP)(p);
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
	/*! Create a shared pointer from this one that casts the underlying type to another underlying type (i.e. for derived types) */
	template<typename astype>  cweeSharedPtr<typename std::remove_const<astype>::type> CastReference() const noexcept {
		typedef typename std::remove_const<astype>::type newT;
		cweeSharedPtr<newT> out;
		typedef typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl oldDI;
		typedef typename cweeSharedPtr<newT>::cweeSharedPtr_DataImpl newDI;
		typedef typename cweeSharedPtr<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return dynamic_cast<newP>((typename std::remove_const<PtrType>::type)(p));
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
#pragma endregion
#pragma region Data Access
public:
	/*! Pointer Access (never throws) (no race conditions) */
	PtrType operator->() const noexcept { return Get(); };

	template<typename Q = type> typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator[](std::ptrdiff_t idx) const
	{
		AUTO x = Get();
		if (x) return x[idx];
		throw(std::exception("cweeSharedPtr was null with indexed access"));
	};

	template<typename Q = type> typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator*() const
	{
		AUTO x = Get();
		if (x) return *x;
		throw(std::exception("cweeSharedPtr was null with reference access"));
	};

	template<typename Q = type> typename std::enable_if<std::is_same<Q, void>::value, void>::type operator[](std::ptrdiff_t idx) const { throw(std::exception("cweeSharedPtr was null with indexed access")); };

	template<typename Q = type> typename std::enable_if<std::is_same<Q, void>::value, void>::type operator*() const { throw(std::exception("cweeSharedPtr was null with reference access")); };

#pragma endregion
#pragma region Assignment Operators
public:
	/*! Share ownership */
	cweeSharedPtr& operator=(const cweeSharedPtr& other) noexcept {
		if (other == *this) { return *this; }

		AUTO p = InitDataFromAnotherPtr(other);
		SetData(p);

		return *this;
	};

	/*! Take ownership */
	cweeSharedPtr& operator=(cweeSharedPtr&& other) noexcept {
		if (other == *this) { return *this; }

		other.Lock();
		AUTO p = other.m_data.Set(nullptr); // InitDataFromAnotherPtr(std::forward<cweeSharedPtr>(other));
		other.Unlock();
		SetData(p);

		return *this;
	};
#pragma endregion
#pragma region Swap Operators
public:
	cweeSharedPtr& Swap(cweeSharedPtr& other) {
		cweeSharedPtr t = *this;
		*this = other;
		other = t;
		return *this;
	};
#pragma endregion
#pragma region "std::shared_ptr" Compatability Methods
public:
	/*! Get access to the underlying data in a thread-safe way. (The underlying data isn't necessarily thread-safe, but accessing the pointer itself has no data race.) */
	PtrType get() const noexcept { return Get(); };
	/*! Empty the current data pointer and free the data. */
	void reset() { cweeSharedPtr().Swap(*this); };
	/*! Exchange the current data with the provided new data. */
	template< class Y > void reset(Y* ptr) { cweeSharedPtr(ptr).Swap(*this); };
	/*! Swap shared pointers */
	void swap(cweeSharedPtr& r) { Swap(r); };
	/*! Number of shared pointers with access to the underlying data, including this one. */
	long use_count() const noexcept {
		long out(0);
		Lock();
		AUTO d = m_data.Get();
		if (d) {
			out = d->use_count();
		}
		Unlock();
		return out;
	};
#pragma endregion
#pragma region Guard
public:
	class cweeSharedPtrGuard {
	public:
		constexpr explicit operator bool() const { return m_p; };
		cweeSharedPtrGuard() = delete;
		cweeSharedPtrGuard(cweeSharedPtrGuard&&) = delete;
		cweeSharedPtrGuard(cweeSharedPtrGuard const&) = delete;
		cweeSharedPtrGuard& operator=(cweeSharedPtrGuard const&) = delete;
		cweeSharedPtrGuard& operator=(cweeSharedPtrGuard&&) = delete;
		explicit cweeSharedPtrGuard(cweeSharedPtr* p_p) noexcept : m_p(p_p) { p_p->Lock(); }
		~cweeSharedPtrGuard() noexcept { m_p->Unlock(); };
	private:
		cweeSharedPtr* m_p;
	};
	[[nodiscard]] cweeSharedPtrGuard Guard() { return cweeSharedPtrGuard(this); };
#pragma endregion
};

#if 1
template <typename T>
class LockedObject {
public:
	LockedObject() noexcept : obj(), lock(0) {};
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject(T&& o) noexcept : obj(std::forward<T>(o)), lock(0) {};
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject(T const& o) noexcept : obj(o), lock(0) {};
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject& operator=(T const& o) { Lock(); obj = o; Unlock(); };
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject& operator=(T&& o) { Lock(); obj = std::forward<T>(o); Unlock(); };

	LockedObject(LockedObject&& o) noexcept : obj(o.obj), lock(o.lock) {};
	LockedObject(LockedObject const& o) noexcept : obj(o.obj), lock(o.lock) {};
	LockedObject& operator=(LockedObject const& o) { if (this != &o) { Lock(); o.Lock(); obj = o.obj; o.Unlock(); Unlock(); } return *this; };
	LockedObject& operator=(LockedObject&& o) { if (this != &o) { Lock(); o.Lock(); obj = o.obj; o.Unlock(); Unlock(); } return *this; };

	~LockedObject() noexcept {};

	T* get() const noexcept { return &obj; };
	T* operator->() const noexcept { return get(); };
	T& operator*() const noexcept { return *get(); };
	void Lock() const noexcept { while (lock.Increment() != 1) { lock.Decrement(); } };
	void Unlock() const noexcept { lock.Decrement(); };

private:
	mutable T	obj;
	mutable cweeSysInterlockedInteger lock;
};

typedef LockedObject<std::vector<std::function<void()>>> cweeSharedPtrWithSentinelDestroyerListType;
class cweeSharedPtrWithSentinel_Details_Interface {
public:
	cweeSharedPtrWithSentinel_Details_Interface() = delete;
	explicit cweeSharedPtrWithSentinel_Details_Interface(
		std::shared_ptr<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>> on_destroy		
	) noexcept : m_destroy_det(on_destroy), m_destroyers() {};
	explicit cweeSharedPtrWithSentinel_Details_Interface(cweeSharedPtrWithSentinel_Details_Interface&) = delete;
	explicit cweeSharedPtrWithSentinel_Details_Interface(cweeSharedPtrWithSentinel_Details_Interface&&) = delete;
	cweeSharedPtrWithSentinel_Details_Interface& operator=(cweeSharedPtrWithSentinel_Details_Interface&) = delete;
	cweeSharedPtrWithSentinel_Details_Interface& operator=(cweeSharedPtrWithSentinel_Details_Interface&&) = delete;
	virtual ~cweeSharedPtrWithSentinel_Details_Interface() noexcept {};
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual void add_on_destroy(const std::function<void()>& func) const noexcept = 0;
	std::shared_ptr<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>>	m_destroy_det;
	cweeSharedPtrWithSentinelDestroyerListType	m_destroyers;
};
class cweeSharedPtrWithSentinel_Data_Interface {
public:
	explicit cweeSharedPtrWithSentinel_Data_Interface() noexcept : det(nullptr) {};
	explicit cweeSharedPtrWithSentinel_Data_Interface(cweeSharedPtrWithSentinel_Details_Interface* p) noexcept : det(p) {};
	explicit cweeSharedPtrWithSentinel_Data_Interface(cweeSharedPtrWithSentinel_Data_Interface const& other) noexcept : det(other.det) {};
	virtual ~cweeSharedPtrWithSentinel_Data_Interface() noexcept {};
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual void add_on_destroy(const std::function<void()>& func) const noexcept = 0;
	mutable cweeSysInterlockedPointer<cweeSharedPtrWithSentinel_Details_Interface>	det;
};
template <typename type> class cweeSharedPtrWithSentinel {
#pragma region Class Defs
public:
	class details final : public cweeSharedPtrWithSentinel_Details_Interface {
	public:
		details() = delete;
		explicit details(type* _source, std::function<void(type*)> _destroy) noexcept : cweeSharedPtrWithSentinel_Details_Interface(m_self_destroy), m_source(_source), m_count(1), m_on_destroy(std::move(_destroy)) {};
		~details() noexcept {};
		void* source() const noexcept final {
			return (void*)m_source.Get();
		};
		int	use_count() const noexcept final {
			return m_count.GetValue();
		};
		void increment() const noexcept final {
			m_count.Increment();
		};
		int decrement() const noexcept final {
			int i = m_count.Decrement();
			if (i == 0) {
				m_on_destroy(m_source.Set(nullptr));
			}
			return i;
		};
		void add_on_destroy(const std::function<void()>& func) const noexcept  final {
			auto& ptr = this->m_destroyers;
			ptr.Lock();
			ptr->push_back(func);
			ptr.Unlock();
		};
	public:
		mutable cweeSysInterlockedPointer<type>	m_source;
		mutable cweeSysInterlockedInteger		m_count;
		std::function<void(type*)>				m_on_destroy;
		static std::shared_ptr<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>> m_self_destroy;
	};
	class details_no_destructor final : public cweeSharedPtrWithSentinel_Details_Interface {
	public:
		details_no_destructor() = delete;
		explicit details_no_destructor(type* _source) noexcept : cweeSharedPtrWithSentinel_Details_Interface(m_self_destroy), m_source(_source), m_count(1) {};
		~details_no_destructor() noexcept {};
		void* source() const noexcept final {
			return (void*)m_source.Get();
		};
		int	use_count() const noexcept final {
			return m_count.GetValue();
		};
		void increment() const noexcept final {
			m_count.Increment();
		};
		int decrement() const noexcept final {
			int i = m_count.Decrement();
			if (i == 0) {
				AUTO p = m_source.Set(nullptr);
				if (p) {
					delete p;
				}
			}
			return i;
		};
		void add_on_destroy(const std::function<void()>& func) const noexcept  final {
			auto& ptr = this->m_destroyers;
			ptr.Lock();
			ptr->push_back(func);
			ptr.Unlock();
		};
	public:
		mutable cweeSysInterlockedPointer<type>	m_source;
		mutable cweeSysInterlockedInteger		m_count;
		static std::shared_ptr <std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>> m_self_destroy;
	};
	class cweeSharedPtrWithSentinel_DataImpl final : public cweeSharedPtrWithSentinel_Data_Interface {
	public:
		explicit cweeSharedPtrWithSentinel_DataImpl() = default;
		explicit cweeSharedPtrWithSentinel_DataImpl(type* _source, std::function<void(type*)> _on_destroy, std::function<type* (void*)> _getter) noexcept : cweeSharedPtrWithSentinel_Data_Interface(new details(_source, std::move(_on_destroy))), m_getter(std::move(_getter)) {};
		explicit cweeSharedPtrWithSentinel_DataImpl(type* _source, std::function<type* (void*)> _getter) noexcept : cweeSharedPtrWithSentinel_Data_Interface(new details_no_destructor(_source)), m_getter(std::move(_getter)) {};
		explicit cweeSharedPtrWithSentinel_DataImpl(const cweeSharedPtrWithSentinel_Data_Interface& other, std::function<type* (void*)> _getter) noexcept : cweeSharedPtrWithSentinel_Data_Interface(other), m_getter(std::move(_getter)) {	increment(); };
		~cweeSharedPtrWithSentinel_DataImpl() noexcept { decrement(); };
		void* source() const noexcept final {
			AUTO p = this->det.Get();
			if (p) {
				return p->source();
			}
			return nullptr;
		};
		int	use_count() const noexcept final {
			AUTO p = this->det.Get();
			if (p) {
				return p->use_count();
			}
			return 0;
		};
		void increment() const noexcept final {
			AUTO p = this->det.Get();
			if (p) {
				p->increment();
			}
		};
		int decrement() const noexcept final {
			int i = -1;
			AUTO p = this->det.Get();
			if (p) {
				i = p->decrement();
				if (i == 0) {					
					AUTO p2 = this->det.Set(nullptr);
					if (p2) {
						{
							p2->m_destroyers.Lock();
							for (auto& dest : *p2->m_destroyers) {
								dest();
							}
							p2->m_destroyers.Unlock();
						}
						if (p2->m_destroy_det; AUTO destroyerP = p2->m_destroy_det.get()) {
							if (destroyerP) {
								destroyerP->operator()(p2);
								return i;
							}
							else {
								delete p2;
							}
						}
						else {
							delete p2;
						}
					}					
				}
			}
			return i;
		};
		void add_on_destroy(const std::function<void()>& func) const noexcept final {
			AUTO p = this->det.Get();
			if (p) {
				p->add_on_destroy(func);
			}
			else {
				func();
			}
		};

	public:
		std::function<type* (void*)>				m_getter;
	};
#pragma endregion 
#pragma region Type Defs
public:
	typedef type						Type;
	typedef type*						PtrType;
#pragma endregion 
#pragma region Data Members
public:
	mutable cweeSysInterlockedInteger									mutex;
	mutable cweeSysInterlockedPointer<cweeSharedPtrWithSentinel_DataImpl>			m_data;
#pragma endregion 
#pragma region Create or Destroy
public:
	/*! Create an empty pointer (nullptr) */
	constexpr cweeSharedPtrWithSentinel() noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a nullptr */
	constexpr cweeSharedPtrWithSentinel(std::nullptr_t) noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtrWithSentinel(PtrType source) noexcept : mutex(0), m_data(InitData(source)) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtrWithSentinel(PtrType source, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(source, std::move(destroy))) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtrWithSentinel(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitData(source, std::move(destroy), std::move(_getter))) {};

	/*! instantiate with a constructor and destructor */
	cweeSharedPtrWithSentinel(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(std::move(create), std::move(destroy))) {};

	/*! instantiate from another ptr */
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel const& samePtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr)) {};
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel>(other))) {};

	/*! instantiate from another ptr with complex "get" instructions */
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel const& samePtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr, std::move(_getter))) {};
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel>(other), std::move(_getter))) {};

	/*! instantiate from another ptr with a different Type, using basic cast operations */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T> const& similarPtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, [](void* p) {
		return static_cast<PtrType>((T*)p); // dynamic_cast
	})) {};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T>&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel<T>>(other), [](void* p) {
		return static_cast<PtrType>((T*)p); // dynamic_cast
	})) {};

	/*! instantiate from another ptr with a different Type with complex "get" instructions */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T> const& similarPtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, std::move(_getter))) {};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T>&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel<T>>(other), std::move(_getter))) {};

	//template<typename = std::enable_if<!std::is_same<Type, void>::value && !std::is_same<Type, std::decay<Type>>::value>>
	//cweeSharedPtrWithSentinel(Type const& source) noexcept : mutex(0), m_data(InitData(new Type(source))) {};

	/*! Instantiate a shared pointer by handing over a data source to be copied. */
	//template<typename = std::enable_if<!std::is_same<Type, void>::value && !std::is_same<Type, std::decay<Type>>::value>>
	//cweeSharedPtrWithSentinel(Type&& source) noexcept : mutex(0), m_data(InitData(new Type(std::forward<Type>(source)))) {};

	/*! Destroy this instance of the shared pointer and potentially delete the data */
	~cweeSharedPtrWithSentinel() noexcept {
		ClearData();
	};
#pragma endregion
#pragma region Internal Support Functions
public:
	PtrType Get() const {
		PtrType out(nullptr);

		Lock();
		auto* d = m_data.Get();
		if (d) {
			out = d->m_getter(d->source());
		}
		Unlock();
		return out;
	};
	template <typename T> cweeSharedPtrWithSentinel<T>* UnsafeSentinel(const cweeSharedPtrWithSentinel<T>& reference) const noexcept {
		AUTO p = new cweeSharedPtrWithSentinel<T>(reference);
		this->UnsafeDoOnThisScopeLoss([p]() {
			delete p;
		});
		return p;
	};
	template <typename T> cweeSharedPtr<T>* UnsafeSentinel(const cweeSharedPtr<T>& reference) const noexcept {
		AUTO p = new cweeSharedPtr<T>(reference);
		this->UnsafeDoOnThisScopeLoss([p]() { 
			delete p;	
		});
		return p;
	};

private:
	void UnsafeDoOnThisScopeLoss(std::function<void()> todo_after_delete) const noexcept {
		auto* d = m_data.Get();
		if (d) {
			d->add_on_destroy(std::move(todo_after_delete));
		}
		else {
			todo_after_delete();
		}
	};

public:
	void Lock() const noexcept { while (mutex.Increment() != 1) { mutex.Decrement(); } };
	void Unlock() const noexcept { mutex.Decrement(); };
	PtrType UnsafeGet() const noexcept { PtrType out(nullptr); auto* d = m_data.Get(); if (d) { out = d->m_getter(d->source()); } return out; };
	template <typename... Args> void UnsafeSet(Args... Fargs) {	UnsafeSetData(cweeSharedPtrWithSentinel<type>::InitData(Fargs...));	};

protected:
	template <typename T> static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T> const& Ptr) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<T>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Get();
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, [](void* p) constexpr ->  PtrType {return (PtrType)p; });
			Ptr.Unlock();
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	template <typename T> static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T> const& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<T>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Get();
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T>&& Ptr) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<T>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, [](void* p) constexpr ->  PtrType { return (PtrType)p; });
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T>&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<T>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel&& Ptr) noexcept {
		cweeSharedPtrWithSentinel_DataImpl* p;
		Ptr.Lock();
		p = Ptr.m_data.Set(nullptr);
		Ptr.Unlock();
		return p;
	};
	static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};

	static AUTO InitData(PtrType source, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtrWithSentinel_DataImpl(source, std::move(from));
	};
	static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtrWithSentinel_DataImpl(source, std::move(destroy), std::move(from));
	};
	static AUTO InitData(PtrType source) noexcept { return InitData(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy) noexcept { return InitData(source, std::move(destroy), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	static AUTO InitData(std::function<PtrType()> create) noexcept { return InitData(create()); };
	static AUTO InitData(std::function<PtrType()> create, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(from)); };
	static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept { return InitData(create(), std::move(destroy)); };
	static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(destroy), std::move(from)); };

	void SetData(cweeSharedPtrWithSentinel_DataImpl* p) const { Lock(); AUTO d = m_data.Set(p); Unlock(); if (d) { delete d; } };
	void UnsafeSetData(cweeSharedPtrWithSentinel_DataImpl* p) const { AUTO d = m_data.Set(p); if (d) { delete d; } };
	void ClearData() const { SetData(nullptr); };

#pragma endregion 
#pragma region Boolean Operators
public:
	constexpr explicit operator bool() const { return m_data.Get(); };
	friend bool operator==(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() == b.Get(); };
	friend bool operator!=(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() != b.Get(); };
	friend bool operator<(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() < b.Get(); };
	friend bool operator<=(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() <= b.Get(); };
	friend bool operator>(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() > b.Get(); };
	friend bool operator>=(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() >= b.Get(); };
	friend bool operator==(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() == nullptr; };
	friend bool operator!=(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() != nullptr; };
	friend bool operator<(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() < nullptr; };
	friend bool operator<=(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() <= nullptr; };
	friend bool operator>(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() > nullptr; };
	friend bool operator>=(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() >= nullptr; };
	friend bool operator==(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr == a.m_data.Get(); };
	friend bool operator!=(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr != a.m_data.Get(); };
	friend bool operator<(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr < a.m_data.Get(); };
	friend bool operator<=(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr <= a.m_data.Get(); };
	friend bool operator>(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr > a.m_data.Get(); };
	friend bool operator>=(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr >= a.m_data.Get(); };
#pragma endregion
#pragma region Ptr Casting
public:
	/*! Create a shared pointer from this one with an added const component to the underlying managed object type */
	cweeSharedPtrWithSentinel<typename std::add_const<Type>::type> ConstReference() const noexcept {
		typedef typename std::add_const<Type>::type newT;
		cweeSharedPtrWithSentinel<newT> out;
		typedef typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl oldDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::cweeSharedPtrWithSentinel_DataImpl newDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return (newP)(p);
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
	/*! Create a shared pointer from this one with an removed const component to the underlying managed object type */
	cweeSharedPtrWithSentinel<typename std::remove_const<Type>::type> RemoveConstReference() const noexcept {
		typedef typename std::remove_const<Type>::type newT;
		cweeSharedPtrWithSentinel<newT> out;
		typedef typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl oldDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::cweeSharedPtrWithSentinel_DataImpl newDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return (newP)(p);
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
	/*! Create a shared pointer from this one that casts the underlying type to another underlying type (i.e. for derived types) */
	template<typename astype>  cweeSharedPtrWithSentinel<typename std::remove_const<astype>::type> CastReference() const noexcept {
		typedef typename std::remove_const<astype>::type newT;
		cweeSharedPtrWithSentinel<newT> out;
		typedef typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl oldDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::cweeSharedPtrWithSentinel_DataImpl newDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return dynamic_cast<newP>((typename std::remove_const<PtrType>::type)(p));
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
#pragma endregion
#pragma region Data Access
public:
	/*! Pointer Access (never throws) (no race conditions) */
	PtrType operator->() const noexcept {
		return Get();
	};

	template<typename Q = type> typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator[](std::ptrdiff_t idx) const
	{
		AUTO x = Get();
		if (x) return x[idx];
		throw(std::exception("cweeSharedPtrWithSentinel was null with indexed access"));
	};

	template<typename Q = type> typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator*() const
	{
		AUTO x = Get();
		if (x) return *x;
		throw(std::exception("cweeSharedPtrWithSentinel was null with reference access"));
	};

	template<typename Q = type> typename std::enable_if<std::is_same<Q, void>::value, void>::type operator[](std::ptrdiff_t idx) const
	{
		throw(std::exception("cweeSharedPtrWithSentinel was null with indexed access"));
	};

	template<typename Q = type> typename std::enable_if<std::is_same<Q, void>::value, void>::type operator*() const
	{
		throw(std::exception("cweeSharedPtrWithSentinel was null with reference access"));
	};

#pragma endregion
#pragma region Assignment Operators
public:
	/*! Share ownership */
	cweeSharedPtrWithSentinel& operator=(const cweeSharedPtrWithSentinel& other) noexcept {
		if (other == *this) { return *this; }

		AUTO p = InitDataFromAnotherPtr(other);
		SetData(p);

		return *this;
	};

	/*! Take ownership */
	cweeSharedPtrWithSentinel& operator=(cweeSharedPtrWithSentinel&& other) noexcept {
		if (other == *this) { return *this; }

		other.Lock();
		AUTO p = other.m_data.Set(nullptr); // InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel>(other));
		other.Unlock();
		SetData(p);

		return *this;
	};
#pragma endregion
#pragma region Swap Operators
public:
	cweeSharedPtrWithSentinel& Swap(cweeSharedPtrWithSentinel& other) {
		cweeSharedPtrWithSentinel t = *this;
		*this = other;
		other = t;
		return *this;
	};
#pragma endregion
#pragma region "std::shared_ptr" Compatability Methods
public:
	/*! Get access to the underlying data in a thread-safe way. (The underlying data isn't necessarily thread-safe, but accessing the pointer itself has no data race.) */
	PtrType get() const noexcept { return Get(); };
	/*! Empty the current data pointer and free the data. */
	void reset() { cweeSharedPtrWithSentinel().Swap(*this); };
	/*! Exchange the current data with the provided new data. */
	template< class Y > void reset(Y* ptr) { cweeSharedPtrWithSentinel(ptr).Swap(*this); };
	/*! Swap shared pointers */
	void swap(cweeSharedPtrWithSentinel& r) { Swap(r); };
	/*! Number of shared pointers with access to the underlying data, including this one. */
	long use_count() const noexcept { long out(0); Lock(); AUTO d = m_data.Get(); if (d) { out = d->use_count(); } Unlock(); return out; };
#pragma endregion
};
template <typename type> std::shared_ptr<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>> cweeSharedPtrWithSentinel<type>::details::m_self_destroy = std::make_shared<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>>([](cweeSharedPtrWithSentinel_Details_Interface* p) {
	if (p) {
		AUTO q = dynamic_cast<cweeSharedPtrWithSentinel<type>::details*>(p);
		if (q) {
			delete q;
		}
		else {
			delete p;
		}
	}
});
template <typename type> std::shared_ptr<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>> cweeSharedPtrWithSentinel<type>::details_no_destructor::m_self_destroy = std::make_shared<std::function<void(cweeSharedPtrWithSentinel_Details_Interface*)>>([](cweeSharedPtrWithSentinel_Details_Interface* p) {
	if (p) {
		AUTO q = dynamic_cast<cweeSharedPtrWithSentinel<type>::details_no_destructor*>(p);
		if (q) {
			delete q;
		}
		else {
			delete p;
		}
	}
});
#endif

#endif

template <typename type> INLINE cweeSharedPtr<type> make_cwee_shared() { 
	return cweeSharedPtr<type>(
#ifdef allowCweeSharedPtrCaptureByValue
		type()
#else
		new type()
#endif
	); 
};
template <typename type> INLINE cweeSharedPtr<type> make_cwee_shared(type* p) { return cweeSharedPtr<type>(p); };
template<typename type, typename t1, typename = std::enable_if_t<!std::is_same_v<type*, t1>>> INLINE cweeSharedPtr<type> make_cwee_shared(t1&& d) { return cweeSharedPtr<type>(new type(std::forward<t1>(d))); };
template <typename type, typename t1, typename = std::enable_if_t<!std::is_same_v<type*, t1>>, class ...Args> INLINE cweeSharedPtr<type> make_cwee_shared(t1&& d, Args&&... Fargs) { return cweeSharedPtr<type>(new type(std::forward<t1>(d), std::forward<Args>(Fargs)...)); };


/* MUTEX COLLECTION */
class cweeSysMutex {
public:
	class cweeSysMutexLifetimeGuard {
	public:
		constexpr explicit operator bool() const { return (bool)owner; };
		explicit cweeSysMutexLifetimeGuard(const cweeSharedPtr<cweeSysMutexImpl>& mut) noexcept : owner(mut) {
			owner->Lock();
		};
		~cweeSysMutexLifetimeGuard() noexcept {
			owner->Unlock();
		};
		explicit cweeSysMutexLifetimeGuard() = delete;
		explicit cweeSysMutexLifetimeGuard(const cweeSysMutexLifetimeGuard& other) = delete;
		explicit cweeSysMutexLifetimeGuard(cweeSysMutexLifetimeGuard&& other) = delete;
		cweeSysMutexLifetimeGuard& operator=(const cweeSysMutexLifetimeGuard&) = delete;
		cweeSysMutexLifetimeGuard& operator=(cweeSysMutexLifetimeGuard&&) = delete;
	protected:
		cweeSharedPtr<cweeSysMutexImpl> owner;
	};

public:	
	cweeSysMutex() : Handle(new cweeSysMutexImpl()) {};
	cweeSysMutex(const cweeSysMutex& other) : Handle(new cweeSysMutexImpl()) {};
	cweeSysMutex(cweeSysMutex&& other) : Handle(new cweeSysMutexImpl()) {};
	cweeSysMutex& operator=(const cweeSysMutex& s) { return *this; };
	cweeSysMutex& operator=(cweeSysMutex&& s) {	return *this; };
	~cweeSysMutex() {};

	[[nodiscard]] cweeSysMutexLifetimeGuard	Guard() const {
		return cweeSysMutexLifetimeGuard(Handle); 
	};
	bool			Lock(bool blocking = true) const {
		return Handle->Lock(blocking);
	};
	void			Unlock() const { 
		Handle->Unlock();		
	};

protected:
	mutable cweeSharedPtr<cweeSysMutexImpl> Handle;
};
class cweeConstexprLock {
public:
	class cweeConstexprLockLifetimeGuard {
	public:
		constexpr explicit operator bool() const noexcept { return true; };
		explicit cweeConstexprLockLifetimeGuard(cweeSysInterlockedInteger& mut) noexcept : owner(mut) {
			while (owner.Increment() != 1) {
				owner.Decrement();
			}
		};
		~cweeConstexprLockLifetimeGuard() noexcept {
			owner.Decrement();
		};
		explicit cweeConstexprLockLifetimeGuard() = delete;
		explicit cweeConstexprLockLifetimeGuard(const cweeSysInterlockedInteger& other) = delete;
		explicit cweeConstexprLockLifetimeGuard(cweeSysInterlockedInteger&& other) = delete;
		cweeConstexprLockLifetimeGuard& operator=(const cweeSysInterlockedInteger&) = delete;
		cweeConstexprLockLifetimeGuard& operator=(cweeSysInterlockedInteger&&) = delete;
	protected:
		cweeSysInterlockedInteger& owner;
	};

public:
	constexpr cweeConstexprLock() noexcept : Handle(0) {};
	constexpr cweeConstexprLock(const cweeSysMutex& other) noexcept : Handle(0) {};
	constexpr cweeConstexprLock(cweeSysMutex&& other) noexcept : Handle(0) {};
	constexpr cweeConstexprLock& operator=(const cweeSysMutex& s) noexcept { return *this; };
	constexpr cweeConstexprLock& operator=(cweeSysMutex&& s) noexcept { return *this; };
	~cweeConstexprLock() {};

	[[nodiscard]] cweeConstexprLockLifetimeGuard	Guard() const noexcept {	return cweeConstexprLockLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const noexcept {
		if (blocking) {
			while (Handle.Increment() != 1) Handle.Decrement(); 
			
			return true;
		}
		else {
			if (Handle.Increment() != 1) {
				Handle.Decrement();
				return false;
			}
			return true;
		}
	};
	void			Unlock() const noexcept {
		Handle.Decrement();
	};

protected:
	mutable cweeSysInterlockedInteger Handle;

};


#if 1
class cweeSysSignalImpl {
public:
	static constexpr int	WAIT_INFINITE = -1;
	cweeSysSignalImpl(bool manualReset = false) noexcept { Sys_SignalCreate(Handle, manualReset); };
	~cweeSysSignalImpl()	noexcept { Sys_SignalDestroy(Handle); };
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() noexcept { Sys_SignalRaise(Handle); }
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept { Sys_SignalClear(Handle); }
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread.
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = cweeSysSignalImpl::WAIT_INFINITE) noexcept { return Sys_SignalWait(Handle, timeout); }

	cweeSysSignalImpl(const cweeSysSignalImpl&) = delete;
	cweeSysSignalImpl(cweeSysSignalImpl&&) = delete;
	cweeSysSignalImpl& operator=(cweeSysSignalImpl const&) = delete;
	cweeSysSignalImpl& operator=(cweeSysSignalImpl&&) = delete;

protected:
	signalHandle_t		Handle;
};
class cweeSysSignal {
public:
	static constexpr int	WAIT_INFINITE = -1;
	cweeSysSignal(bool manualReset = false) : Handle(new cweeSysSignalImpl(manualReset)) {};
	cweeSysSignal(const cweeSysSignal& other) : Handle(other.Handle) {};
	cweeSysSignal(cweeSysSignal&& other) : Handle(other.Handle) {};
	cweeSysSignal& operator=(const cweeSysSignal& s) { Handle = s.Handle; return *this; };
	cweeSysSignal& operator=(cweeSysSignal&& s) { Handle = s.Handle; return *this; };
	~cweeSysSignal() {};
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		return h->Raise();
	};
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		return h->Clear();
	};
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread.
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = cweeSysSignal::WAIT_INFINITE) noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		AUTO p = h.Get();
		return p->Wait(timeout);
	};
protected:
	mutable cweeSharedPtr<cweeSysSignalImpl> Handle;
};

#else
/* SIGNAL SECTION */
class cweeSysSignal {
public:
	static const int	WAIT_INFINITE = -1;
	cweeSysSignal(bool manualReset = false) { Sys_SignalCreate(Handle, manualReset); }
	~cweeSysSignal() { Sys_SignalDestroy(Handle); }
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() { Sys_SignalRaise(Handle); }
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() { Sys_SignalClear(Handle); }
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread. 
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = WAIT_INFINITE) { return Sys_SignalWait(Handle, timeout); } 
private:
	signalHandle_t		Handle;
	cweeSysSignal(const cweeSysSignal& s) {}
	void				operator=(const cweeSysSignal& s) {}
};
#endif


/*
MUTEX COLLECTION
*/
class cweeMutex {
private:
	
	cweeSysInterlockedInteger Handle;
	int Increment() { return Handle.Increment(); }
	int Decrement() { return Handle.Decrement(); }
	cweeSysInterlockedInteger locked;

public:
	cweeMutex() { Handle.SetValue(0); locked.SetValue(0); };
	~cweeMutex() {};
	void			Lock() {
		do {
			if (Increment() == 1) {
				// achieved the lock.
				locked.Increment();
				break;
			} else {
				Decrement();
			}
		} while (1);
	};
	void			Unlock() {
		if (locked.GetValue() == 0) return; // allow repeated attempts at unlocking
		do {
			if (Decrement() == 0) {
				// achieved the unlock
				locked.Decrement();
				break;
			} else {
				Increment();
			}
		} while (1);
	};
};

/*
MULTITHREADING COLLECTION
*/
class cweeSysThread {
public:
						cweeSysThread();
	virtual				~cweeSysThread();
	const char*			GetName() const { return name; }
	uintptr_t			GetThreadHandle() const { return threadHandle; }
	bool				IsRunning() const { return isRunning; }
	bool				IsTerminating() const { return isTerminating; }
	bool				StartThread(const char* name, core_t core,	xthreadPriority priority = THREAD_NORMAL, int stackSize = DEFAULT_THREAD_STACK_SIZE);
	bool				StartWorkerThread(const char* name, core_t core, xthreadPriority priority = THREAD_NORMAL,	int stackSize = DEFAULT_THREAD_STACK_SIZE);
	void				StopThread(bool wait = true);
	void				WaitForThread();
	void				SignalWork();
	bool				IsWorkDone();
protected:
	virtual int			Run(); 	// The routine that performs the work.
private:
	const char*			name;
	uintptr_t			threadHandle;
	bool				isWorker;
	bool				isRunning;
	volatile bool		isTerminating;
	volatile bool		moreWorkToDo;
	cweeSysSignal		signalWorkerDone;
	cweeSysSignal		signalMoreWorkToDo;
	cweeSysMutex		signalMutex;
	static int			ThreadProc(cweeSysThread* thread);
						cweeSysThread(const cweeSysThread& s) {}
	void				operator=(const cweeSysThread& s) {}
};

#endif