#pragma once

#pragma region Precompiled STL Headers
#pragma warning(disable : 4005)				// macro redefinition
#pragma warning(disable : 4010)				// single-line comment contains line-continuation character
#pragma warning(disable : 4018)				// singed / unsigned mismatch
#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4101)				// unreferenced local variable
#pragma warning(disable : 4127)				// conditional expression is constant
#pragma warning(disable : 4172)				// returning address of local variable or temporary
#pragma warning(disable : 4189)				// local variable is initialized but not referenced
#pragma warning(disable : 4238)				// nonstandard extension used: class rvalue used as lvalue
#pragma warning(disable : 4239)				// conversion from 'T' to 'T&'
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4251)				// needs to have dll-interface
#pragma warning(disable : 4267)				// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4273)				// inconsistent DLL linkage
#pragma warning(disable : 4297)				// function assumed not to throw but does
#pragma warning(disable : 4302)				// truncation from 'void *' to 'int'
#pragma warning(disable : 4305)				// truncating a literal from double to float
#pragma warning(disable : 4311)				// pointer truncation from 'void *' to 'int'
#pragma warning(disable : 4312)				// conversion from 'int' to 'void*' of greater size
#pragma warning(disable : 4390)				// ';' empty controlled statement
#pragma warning(disable : 4456)				// declaration hides previous local declaration
#pragma warning(disable : 4458)				// hides class member
#pragma warning(disable : 4459)				// hides global declaration
#pragma warning(disable : 4499)				// 'static': an explicit specialization cannot have a storage class
#pragma warning(disable : 4505)				// unreferenced local function has been removed
#pragma warning(disable : 4595)				// non-member operator new or delete functions may not be declared inline
#pragma warning(disable : 4701)				// potentially uninitialized local variable
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4715)				// not all control paths return a value
#pragma warning(disable : 4996)				// unsafe string operations
#pragma warning(disable : 6011)				// Dereferencing NULL ptr
#pragma warning(disable : 6385)				// Reading invalid data from buf
#pragma warning(disable : 26110)			// Caller failing to hold lock
#pragma warning(disable : 26439)			// This kind of function may not throw
#pragma warning(disable : 26450)			// Arithmetic overflow: using '<<'
#pragma warning(disable : 26451)			// Arithmetic overflow: using '*' on a 4 byte variable and casting to 8 bytes
#pragma warning(disable : 26495)			// uninitialized member variable type 6
#pragma warning(disable : 26498)			// Mark function constexpr if compile-time evaluation is desired
#pragma warning(disable : 26812)			// prefer enum class to enum
#pragma warning(disable : 28182)			// Dereferencing NULL pointer
#pragma warning(disable : 28251)			// Inconsistent annotation for 'new'
#define NOMINMAX
#define _CRT_FUNCTIONS_REQUIRED 1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include <string>
#include <vector>
#include <ShlDisp.h>
#include <condition_variable>
#include <Windows.h>
#include <winnt.h>
#include <functional>
#include <typeinfo>
#include <atomic>

#pragma endregion

#pragma region iterator_definition
#define SETUP_ITERATOR(parentClassType, it_state)   \
		class Iterator : public std::iterator<typename it_state::iterator_category, typename it_state::value_type> {   \
		public:   \
			using thisType = typename it_state::thisType;   \
            friend class parentClassType; \
			using value_type = typename it_state::value_type;   \
			using difference_type = typename std::iterator<typename it_state::iterator_category, value_type>::difference_type;   \
		protected:   \
			thisType* parent;   \
			it_state state;   \
		private:   \
			void Initialize() { state.Initialize(parent); };   \
			void ToBeginning() { state.ToBeginning(parent); };   \
			void ToEnd() { state.ToEnd(parent); };   \
			void Next() { state.Next(parent); };   \
			void Prev() { state.Prev(parent); };   \
			decltype(auto) Get() const { return state.Get(parent); };   \
			difference_type Distance(Iterator const& other) const { return state.Distance(other.state); };   \
		public:   \
			Iterator() = default;   \
            Iterator(thisType* _parent, bool toBeginning = true) : parent{ _parent }, state{} { Initialize(); if (toBeginning) ToBeginning(); else ToEnd(); };   \
			Iterator(thisType* _parent, it_state&& _state) : parent{ _parent }, state{ std::forward<it_state>(_state) } {  };   \
			Iterator(const Iterator& rhs) = default;   \
			Iterator(Iterator&& rhs) = default;   \
			Iterator& operator=(const Iterator& rhs) = default;   \
			Iterator& operator=(Iterator&& rhs) = default;   \
			~Iterator() = default;   \
            /*explicit operator bool() const { return state.Valid(parent); }; */  \
			bool operator==(const Iterator& rhs) const { return state == rhs.state; };   \
			bool operator!=(const Iterator& rhs) const { return !operator==(rhs); };   \
			Iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) Next(); return *this; };   \
			Iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) Prev(); return *this; };   \
			difference_type operator-(Iterator const& other) const { return Distance(other); };   \
			Iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) Prev(); return *this; };   \
			Iterator& operator++() { Next(); return *this; };   \
			Iterator& operator--() { Prev(); return *this; };   \
			Iterator operator++(int) { Iterator out(*this); Next(); return out; };   \
			Iterator operator--(int) { Iterator out(*this); Prev(); return out; };   \
			Iterator begin() const { Iterator out(*this); out.ToBeginning(); return out; };   \
			Iterator end() const { Iterator out(*this); out.ToEnd(); return out; };   \
			decltype(auto) operator*() { return Get(); };   \
			decltype(auto) operator*() const { return Get(); };   \
			decltype(auto) operator->() { return &Get(); };   \
			decltype(auto) operator->() const { return &Get(); };   \
		};   \
		using iterator = Iterator;   \
		using const_iterator = Iterator;   \
		Iterator begin() const {   \
			typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;   \
			return Iterator(const_cast<thisType*>(this), true);   \
		};   \
		Iterator end() const {   \
			typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;   \
			return Iterator(const_cast<thisType*>(this), false);   \
		};   \
		Iterator cbegin() const { return begin(); };   \
		Iterator cend() const { return end(); };
#pragma endregion 

namespace GoodLang {
	namespace utilities {
		/// <summary>
		/// Finally is a pure virtual base class, implemented by the templated FinallyImpl.
		/// </summary>
		class Finally {
		public:
			virtual ~Finally() = default;
		};

		/// <summary>
		/// FinallyImpl implements a Finally. The template parameter F is the function type to be called when the finally is destructed. F must have the signature void().
		/// </summary>
		/// <typeparam name="F"></typeparam>
		template <typename F> class FinallyImpl : public Finally {
		public:
			inline FinallyImpl(const F& func_) : func(func_) {};
			inline FinallyImpl(F&& func_) : func(std::move(func_)) {};
			inline FinallyImpl(FinallyImpl<F>&& other) : func(std::move(other.func)) { other.valid = false; };
			inline ~FinallyImpl() { if (valid) { func(); } };

		private:
			FinallyImpl(const FinallyImpl<F>& other) = delete;
			FinallyImpl<F>& operator=(const FinallyImpl<F>& other) = delete;
			FinallyImpl<F>& operator=(FinallyImpl<F>&&) = delete;
			F func;
			bool valid = true;
		};

		template <typename F> __forceinline [[nodiscard]] FinallyImpl<F> make_finally(F&& f) { return FinallyImpl<F>(std::forward<F>(f)); };
	};

	/// <summary>
	/// Iterator that steps through a list, without needing to instance the whole list. 
	/// </summary>
	/// <typeparam name="Type"></typeparam>
	template<typename Type = size_t> class Sequence {
	private:
		Type min;
		Type max;
		Type step;

		static std::tuple<Type, Type, Type> DetermineSteps(Type N0, Type N1, Type Step) {
			if (Step >= 0) {
				// want to go from small to large
				if (N1 >= N0) {
					return { N0, N1, Step };
				}
				else {
					return { N1, N0, Step };
				}
			}
			else {
				// want to go from large to small
				if (N1 >= N0) {
					return { N1, N0, Step }; 
				}
				else {
					return { N0, N1, Step };
				}
			}			
		};

	public:
		Sequence(Type N0, Type N1, Type Step) {
			std::tie(min, max, step) = DetermineSteps(std::move(N0), std::move(N1), std::move(Step));
		};
		Sequence() : Sequence(0, 0, 1) {};
		Sequence(Type N) : Sequence(0, N, 1) {};
		Sequence(Type N0, Type N1) : Sequence(N0, N1, 1) {};		

		class Iterator : public std::iterator<std::random_access_iterator_tag, Type> {
		public:
			using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;

			Iterator() : _ptr(0), _min(0), _step(1) {}
			Iterator(Type rhs, Type min, Type step) : _ptr(rhs), _min(min), _step(step) {}
			Iterator(const Iterator& rhs) : _ptr(rhs._ptr), _min(rhs._min), _step(rhs._step) {}

			inline Iterator& operator+=(difference_type rhs) { _ptr += rhs * _step; return *this; }
			inline Iterator& operator-=(difference_type rhs) { _ptr -= rhs * _step; return *this; }
			inline Type& operator*() { return _ptr; }
			inline Type* operator->() { return &_ptr; }
			inline Type operator[](difference_type rhs) { return static_cast<Type>(_min + rhs * _step); }
			inline const Type& operator*() const { return _ptr; }
			inline const Type* operator->() const { return &_ptr; }
			inline const Type operator[](difference_type rhs) const { return static_cast<Type>(_min + rhs * _step); }

			inline Iterator& operator++() { _ptr += _step; return *this; }
			inline Iterator& operator--() { _ptr -= _step; return *this; }
			inline Iterator operator++(int) { Iterator tmp(*this); _ptr += _step; return tmp; }
			inline Iterator operator--(int) { Iterator tmp(*this); _ptr -= _step; return tmp; }
			inline difference_type operator-(const Iterator& rhs) const { return (_ptr - rhs._ptr) / _step; }
			inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + rhs * _step, _min, _step); }
			inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - rhs * _step, _min, _step); }
			friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator((lhs * rhs._step) + rhs._ptr, rhs._min, rhs._step); }
			friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator((lhs * rhs._step) - rhs._ptr, rhs._min, rhs._step); }

			inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
			inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
			inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
			inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
			inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
			inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

		protected:
			Type _min;
			Type _ptr;
			Type _step;
		};

		using iterator = Iterator;
		using const_iterator = iterator;

		auto begin() { return Iterator(min, min, step); };
		auto end() { return Iterator(max, min, step); };
		auto cbegin() const { return iterator(min, min, step); };
		auto cend() const { return iterator(max, min, step); };
		auto begin() const { return iterator(min, min, step); };
		auto end() const { return iterator(max, min, step); };
	};

	template<typename Type> class IteratorSequence {
	private:
		using iterator_tag = typename Type::iterator_category;
		using target_value_type = typename std::remove_reference_t<typename Type::reference>;
		using value_type = std::pair<int, target_value_type*>;

		Type min;
		Type max;
		typename std::iterator<iterator_tag, value_type>::difference_type count;

	public:
		IteratorSequence() : min{}, max{}, count{ 0 } {};
		IteratorSequence(Type begin, Type end) : min{ begin }, max{ end }, count{ std::distance(begin, end) } {};

		class Iterator : public std::iterator<iterator_tag, value_type> {
		public:
			using difference_type = typename std::iterator<iterator_tag, value_type>::difference_type;

			Iterator() : _ptr{}, _index{ 0 }, _result{ 0,nullptr }  {};
			Iterator(Type rhs, difference_type index) : _ptr{ rhs }, _index{ index }, _result{ 0,nullptr } {};
			Iterator(const Iterator& rhs) : _ptr{ rhs._ptr }, _index{ rhs._index }, _result{ 0,nullptr } {};

			inline Iterator& operator+=(difference_type rhs) { _ptr += rhs; _index += rhs; return *this; };
			inline Iterator& operator-=(difference_type rhs) { _ptr -= rhs; _index -= rhs; return *this; };
			inline value_type& operator*() { _result.first = _index; _result.second = &*_ptr; return _result; };
			inline value_type* operator->() { _result.first = _index; _result.second = &*_ptr; return &_result; };
			inline value_type operator[](difference_type rhs) { _temp = *_ptr[rhs]; return value_type(rhs, &_temp); };
			inline const value_type& operator*() const { _result.first = _index; _result.second = &*_ptr; return _result; };
			inline const value_type* operator->() const { _result.first = _index; _result.second = &*_ptr; return &_result; };
			inline const value_type operator[](difference_type rhs) const { _temp = *_ptr[rhs]; return value_type(rhs, &_temp); };

			inline Iterator& operator++() { ++_ptr; ++_index; return *this; }
			inline Iterator& operator--() { --_ptr; --_index; return *this; }
			inline Iterator operator++(int) { Iterator tmp(*this); ++_ptr; ++_index; return tmp; }
			inline Iterator operator--(int) { Iterator tmp(*this); --_ptr; --_index; return tmp; }
			inline difference_type operator-(const Iterator& rhs) const { return _index - rhs._index; }
			inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + rhs, _index + rhs); }
			inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - rhs, _index - rhs); }
			friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator(lhs + rhs._ptr, lhs + rhs._index); }
			friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator(lhs - rhs._ptr, lhs - rhs._index); }

			inline bool operator==(const Iterator& rhs) const { return _index == rhs._index; }
			inline bool operator!=(const Iterator& rhs) const { return _index != rhs._index; }
			inline bool operator>(const Iterator& rhs) const { return _index > rhs._index; }
			inline bool operator<(const Iterator& rhs) const { return _index < rhs._index; }
			inline bool operator>=(const Iterator& rhs) const { return _index >= rhs._index; }
			inline bool operator<=(const Iterator& rhs) const { return _index <= rhs._index; }

		private:
			Type _ptr;
			difference_type _index;
			mutable value_type _result;
			mutable std::remove_const_t<target_value_type> _temp;
		};

		using iterator = Iterator;

		auto begin() { return Iterator(min, 0); };
		auto end() { return Iterator(max, count); };
	};

	/// <summary>
	/// Iterator
	/// </summary>
	/// <typeparam name="Type"></typeparam>
	/// <typeparam name="returnType"></typeparam>
	template<class returnType, typename Type = size_t> class CustomizedSequence {
	private:
		Type min;
		Type max;
		Type step;
		std::function<returnType(Type)> Converter;

		static std::tuple<Type, Type, Type> DetermineSteps(Type N0, Type N1, Type Step) {
			if (Step >= 0) {
				// want to go from small to large
				if (N1 >= N0) {
					return { N0, N1, Step };
				}
				else {
					return { N1, N0, Step };
				}
			}
			else {
				// want to go from large to small
				if (N1 >= N0) {
					return { N1, N0, Step };
				}
				else {
					return { N0, N1, Step };
				}
			}
		};

	public:
		CustomizedSequence(std::function<returnType(Type)>&& toDo, Type N0, Type N1, Type Step) : Converter(std::forward<std::function<returnType(Type)>>(toDo)) {
			std::tie(min, max, step) = DetermineSteps(std::move(N0), std::move(N1), std::move(Step));
		};
		CustomizedSequence() : CustomizedSequence({}, 0, 0, 1) {};
		CustomizedSequence(std::function<returnType(Type)>&& toDo, Type N) : CustomizedSequence(std::forward<std::function<returnType(Type)>>(toDo), 0, N, 1) {};
		CustomizedSequence(std::function<returnType(Type)>&& toDo, Type N0, Type N1) : CustomizedSequence(std::forward<std::function<returnType(Type)>>(toDo), N0, N1, 1) {};

		class Iterator : public std::iterator<std::random_access_iterator_tag, Type> {
		public:
			using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;

			Iterator()
				: _out(), Converter(nullptr), _ptr(0), _min(0), _step(1) {};
			Iterator(std::function<returnType(Type)>* converter, Type position, Type min, Type step)
				: _out(), Converter(converter), _ptr((size_t)(double)((position - min) / step)), _min(min), _step(step) {};
			Iterator(const Iterator& rhs)
				: _out(rhs._out), Converter(rhs.Converter), _ptr(rhs._ptr), _min(rhs._min), _step(rhs._step) {};

			inline Iterator& operator+=(difference_type rhs) { _ptr += rhs; return *this; }
			inline Iterator& operator-=(difference_type rhs) { _ptr -= rhs; return *this; }
			inline returnType& operator*() { DoConversion(((double)_ptr * _step) + _min); return _out; }
			inline returnType* operator->() { DoConversion(((double)_ptr * _step) + _min); return &_out; }
			inline returnType operator[](difference_type rhs) { DoConversion(static_cast<Type>(_min + ((double)rhs * _step))); return _out; }
			inline const returnType& operator*() const { DoConversion(((double)_ptr * _step) + _min); return _out; }
			inline const returnType* operator->() const { DoConversion(((double)_ptr * _step) + _min); return &_out; }
			inline const returnType operator[](difference_type rhs) const { DoConversion(static_cast<Type>(_min + ((double)rhs * _step))); return _out; }

			inline Iterator& operator++() { _ptr += 1; return *this; }
			inline Iterator& operator--() { _ptr -= 1; return *this; }
			inline Iterator operator++(int) { Iterator tmp(*this); _ptr += 1; return tmp; }
			inline Iterator operator--(int) { Iterator tmp(*this); _ptr -= 1; return tmp; }
			inline difference_type operator-(const Iterator& rhs) const { return _ptr - rhs._ptr; }
			inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + rhs, _min, _step); }
			inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - rhs, _min, _step); }
			friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator(lhs + rhs._ptr, rhs._min, rhs._step); }
			friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator(lhs - rhs._ptr, rhs._min, rhs._step); }

			inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
			inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
			inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
			inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
			inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
			inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

		protected:
			mutable returnType _out;
			void DoConversion(Type pos) const {
				_out = (*Converter)(pos);
			};

			std::function<returnType(Type)>* Converter;
			Type _min;
			size_t _ptr;
			Type _step;
		};

		using iterator = Iterator;
		using const_iterator = iterator;

		auto begin() { return Iterator(&Converter, min, min, step); };
		auto end() { return Iterator(&Converter, max, min, step); };
		auto cbegin() const { return iterator(&Converter, min, min, step); };
		auto cend() const { return iterator(&Converter, max, min, step); };
		auto begin() const { return iterator(&Converter, min, min, step); };
		auto end() const { return iterator(&Converter, max, min, step); };
	};

};

#define FINALLY_CONCAT_(a, b) a##b
#define FINALLY_CONCAT(a, b) FINALLY_CONCAT_(a, b)

// defer() is a macro to defer execution of a statement until the surrounding scope is closed and is typically used to perform cleanup logic once a function returns.
// . .
// Note: Unlike golang's defer(), the defer statement is executed when the surrounding *scope* is closed, not necessarily the function.
// . .
// Example usage:
// . .
// void sayHelloWorld() {
//		defer(printf("world\n"));
//      printf("hello ");
// }
#define defer(x) decltype(auto) FINALLY_CONCAT(defer_, __LINE__) { GoodLang::utilities::make_finally([&] { x; }) }

namespace GoodLang {
	namespace utilities {
		/* Allocates *_blockSize_* number of elements at a time. Blocks are free'd once the entire allocator goes out-of-scope. Not thread-safe. */
		template <typename _type_, size_t _blockSize_ = std::max<size_t>(128, (sizeof(_type_) << 4)), bool ForcePOD = false>
		class FastBlockAllocator {
		private: // data members

			std::allocator<_type_> alloc{};
			std::vector< _type_* > blocks{};
			size_t capacity{ 0 };
			size_t count{ 0 };

			// returns whether the constructor/destructor needs to be called for each element. (Constructor is always called if args are provided on initialization)
			static constexpr bool isPod() { return std::is_pod<_type_>::value || ForcePOD; };

		public: // public API
			// Request a new memory pointer. May be recovered from a previously-used location. Will be cleared and correctly initialized, if appropriate.
			template <typename... TArgs> _type_* Alloc(TArgs&&... a) {
				_type_* out;

				long long index = count++;
				if (index >= capacity) {
					blocks.push_back(alloc.allocate(_blockSize_));
					capacity += _blockSize_;
				}
				auto pos = std::div(index, _blockSize_);
				out = &blocks[pos.quot][pos.rem];
				alloc.construct(out, std::forward<TArgs>(a)...);

				return out;
			};
			// Does nothing -- is now handled automatically during Free(...) calls.
			void			FreeEmptyBlocks() {
				return;
			};
			// Attempts to cleanup unused memory
			auto TryCleanupUnusedMemory() ->void {};
			// Returns the maximum number of blocks the allocator has reserved -- does not mean all of these blocks are in active use or even alive.
			size_t          MaxBlockCapacity() const {
				return 0;
			};
			// Approximate current (alive) block count. Not all elements in thse blocks are alive or allocated, but usually at least one is.
			size_t          CurrentBlockCount() const {
				return 0;
			};
			// Approximate current capacity for elements, based on the alive blocks.
			size_t          TotalCapacity() const {
				return 0;
			};
			// Approximate current alive element count
			size_t          TotalAlive() const {
				return 0;
			};
			// Report on the statistics for the allocator
			std::string     ReportStatistics(bool includeEndLine = false) const {
				std::string out;
				return out;
			};

		public: // constructors and destructors
			FastBlockAllocator() = default;
			~FastBlockAllocator() {
				if constexpr (!isPod()) {
					_type_* p;
					for (int i = 0; i < count; i++) {
						auto pos = std::div(i, _blockSize_);
						p = &blocks[pos.quot][pos.rem];
						alloc.destroy(p);
					}
				}
				for (auto& block : blocks)
					alloc.deallocate(block, _blockSize_);
			};
		};

		template<class _type_, int _blockSize_ = std::max<size_t>(128, (sizeof(_type_) << 4)), bool ForcePOD = false>
		using FastAllocator = FastBlockAllocator<_type_, _blockSize_, ForcePOD>;

		template<typename T>
		class constexpr_type_info {
		private:
			constexpr static std::string_view fullname_intern() {
				return __FUNCTION__;
			}
			constexpr static std::string_view name_impl() {
				size_t length = fullname_intern().size();
				return fullname_intern().substr(20 + 8 + 9 + 4, (length - (19 + 8 + 9 + 4)) - 19);
			}
			constexpr static size_t constexpr_hash(std::string_view str) {
				size_t seed = 31;
				for (char c : str) {
					seed ^= static_cast<uint64_t>(c) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				}
				return seed;
			};

		public:
			using type = T;
			constexpr static std::string_view name{ name_impl() };
			constexpr static size_t hash{ constexpr_hash(name_impl()) };
		};
	};

	/* *THREAD SAFE* Windows-specific high-performance lock that only locks the OS (slow) when contention actually happens. When there is no contention, this is very fast.
	Generally speaking, out-performs std::mutex under most conditions. */
	class mutex {
	private:
		using mutexHandle_t = RTL_CRITICAL_SECTION;;
		static void				Sys_MutexCreate(mutexHandle_t& handle) noexcept { InitializeCriticalSection(&handle); };
		static void				Sys_MutexDestroy(mutexHandle_t& handle) noexcept { DeleteCriticalSection(&handle); };
		static void				Sys_MutexLock(mutexHandle_t& handle) noexcept { EnterCriticalSection(&handle); };
		static bool				Sys_MutexTryLock(mutexHandle_t& handle) noexcept { return TryEnterCriticalSection(&handle) != 0; };
		static void				Sys_MutexUnlock(mutexHandle_t& handle) noexcept { LeaveCriticalSection(&handle); };

	public:
		mutex() noexcept { Sys_MutexCreate(Handle); };
		~mutex() noexcept { Sys_MutexDestroy(Handle); };

		void lock() {
			Sys_MutexLock(Handle);
		};
		bool try_lock() {
			return Sys_MutexTryLock(Handle);
		};
		void unlock() {
			Sys_MutexUnlock(Handle);
		};

		mutex(const mutex&) = delete;
		mutex(mutex&&) = delete;
		mutex& operator=(mutex const&) = delete;
		mutex& operator=(mutex&&) = delete;

	protected:
		mutexHandle_t Handle;

	};

	// a fast alternative to the std::shared_mutex when prioritizing readers over writers. 
	class fast_shared_mutex {
	private:
		mutable std::atomic<long long> mut{ 0 }; // Read, Write

	public:
		bool try_lock() const {
			thread_local long long read, planned;
			read = planned = mut.load(std::memory_order::memory_order_relaxed);
			if (reinterpret_cast<short*>(&planned)[0] == 0) { // no readers...
				if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...
					if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
						return true; // success!
					}
				}
			}
			return false;
		};
		void unlock() const {
			thread_local long long read, planned;
			int i = 0;
			while (true) {
				if (++i > 40) std::this_thread::yield();
				read = planned = mut.load(std::memory_order::memory_order_relaxed);
				--reinterpret_cast<short*>(&planned)[1];
				if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
					break; // success!
				}
			}
		};
		void lock() const {
			int i = 0;
			while (!try_lock()) {
				if (++i > 40) std::this_thread::yield();
			}
		};

		bool try_lock_shared() const {
			thread_local long long read;
			read = mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // immediately increments the Read count, leaves the writer count alone
			if (
				(reinterpret_cast<short*>(&read)[0] >= 1) // we are allowed to read with other readers...
				&& (reinterpret_cast<long*>(&read)[1] == 0) // so long as there are no writers...
				) {
				return true;
			}
			else {
				mut.fetch_add(-1, std::memory_order::memory_order_acq_rel); // failure -- undo our mistake.
				return false;
			}
		};
		void unlock_shared() const {
			mut.fetch_add(-1, std::memory_order::memory_order_acq_rel);
		};
		void lock_shared() const {
			int i = 0;
			while (!try_lock_shared()) {
				if (++i > 40) std::this_thread::yield();
			}
		};

		// if you already hold a shared_lock and want to upgrade to a hard lock without releasing.
		// Returns true if this ideal scenario was successful. Returns false otherwise.
		bool upgrade_lock() const {
			thread_local long long read, planned;
			// increment the write count and decrement our read count...
			for (int i = 0; i < 40; ++i) {
				planned = read = mut.load(std::memory_order::memory_order_relaxed);
				if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...					
					if (--reinterpret_cast<short*>(&planned)[0] == 0) { // we're the only reader...		
						if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
							return true;
						}
					}
				}
				else {
					break;
				}
			}

			unlock_shared();
			lock();

			return false;
		};

	};

	// Allows readers or writers exclusive access to a critical section, but not both at the same time. 
	class fast_boolean_mutex {
	private:
		mutable std::atomic<long long> mut{ 0 }; // Read, Write

	public:
		bool try_lock() const {
			thread_local long long read, planned;

			read = planned = mut.load(std::memory_order::memory_order_relaxed);
			if (reinterpret_cast<short*>(&planned)[0] == 0) { // no readers...
				if (++reinterpret_cast<short*>(&planned)[1] >= 1) { //  we are allowed to write with other writers...
					if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
						return true; // success!
					}
				}
			}
			return false;
		};
		void unlock() const {
			thread_local long long read, planned;
			while (true) {
				read = planned = mut.load(std::memory_order::memory_order_relaxed);
				--reinterpret_cast<short*>(&planned)[1];
				if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
					break; // success!
				}
			}
		};
		void lock() const {
			while (!try_lock()) {}
		};

		bool try_lock_shared() const {
			thread_local long long read;
			read = mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // immediately increments the Read count, leaves the writer count alone
			if (
				(reinterpret_cast<short*>(&read)[0] >= 1) // we are allowed to read with other readers...
				&& (reinterpret_cast<long*>(&read)[1] == 0) // so long as there are no writers...
				) {
				return true;
			}
			else {
				mut.fetch_add(-1, std::memory_order::memory_order_acq_rel); // failure -- undo our mistake.
				return false;
			}
		};
		void unlock_shared() const {
			mut.fetch_add(-1, std::memory_order::memory_order_acq_rel);
		};
		void lock_shared() const {
			while (!try_lock_shared()) {}
		};
	};
#if 1
	using shared_mutex = fast_shared_mutex;
#else
	/* mutex which allows multiple readers OR one writer to access a critical section at the same time. */
	class shared_mutex {
	public:
		using cond_var = std::condition_variable_any;

	private:
		mutex    mut_;
		cond_var gate1_;
		cond_var gate2_;
		unsigned state_;

		static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
		static const unsigned n_readers_ = ~write_entered_;

	public:
		shared_mutex() : mut_(), gate1_(), gate2_(), state_(0) {}

		// Exclusive ownership
		void lock() {
			std::unique_lock<mutex> lk(mut_);
			while (state_ & write_entered_) gate1_.wait(lk);
			state_ |= write_entered_;
			while (state_ & n_readers_) gate2_.wait(lk);
		};
		bool try_lock() {
			std::unique_lock<mutex> lk(mut_, std::try_to_lock_t{});
			if (lk.owns_lock() && state_ == 0) {
				state_ = write_entered_;
				return true;
			}
			return false;
		};
		void unlock() {
			{
				std::unique_lock<mutex> _(mut_);
				state_ = 0;
			}
			gate1_.notify_all();
		};

		// Shared ownership
		void lock_shared() {
			std::unique_lock<mutex> lk(mut_);
			while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_) gate1_.wait(lk);
			unsigned num_readers = (state_ & n_readers_) + 1;
			state_ &= ~n_readers_;
			state_ |= num_readers;
		};
		bool try_lock_shared() {
			std::unique_lock<mutex> lk(mut_, std::try_to_lock_t{});
			unsigned num_readers = state_ & n_readers_;
			if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_) {
				++num_readers;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				return true;
			}
			return false;
		};
		void unlock_shared() {
			std::unique_lock<mutex> _(mut_);
			unsigned num_readers = (state_ & n_readers_) - 1;
			state_ &= ~n_readers_;
			state_ |= num_readers;
			if (state_ & write_entered_) {
				if (num_readers == 0) gate2_.notify_one();
			}
			else {
				if (num_readers == n_readers_ - 1) gate1_.notify_one();
			}
		};

	};
#endif

	class ExceptionHandling {
	public:
		template <typename T> static std::exception_ptr get_nested(const T& e) {
			try
			{
				auto& nested = dynamic_cast<const std::nested_exception&>(e);
				return nested.nested_ptr();
			}
			catch (const std::bad_cast&)
			{
				return nullptr;
			}
		};
		static std::string what(std::exception_ptr eptr = std::current_exception()) {
			if (!eptr) { throw std::bad_exception(); }

			std::string whaaat;
			std::size_t num_nested = 0;
		next_exception_ptr:
			{
				try
				{
					std::exception_ptr yeptr;
					std::swap(eptr, yeptr);
					std::rethrow_exception(yeptr);
				}
				catch (const std::exception& e) { whaaat += e.what(); eptr = get_nested(e); }
				catch (const std::string& e) { whaaat += e; }
				catch (const char* e) { whaaat += e; }
				catch (...) { whaaat += "who knows"; }

				if (eptr) { whaaat += " ("; num_nested++; goto next_exception_ptr; }
			}
			whaaat += std::string(num_nested, ')');
			return whaaat;
		};
	};

};

