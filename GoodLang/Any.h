#pragma once

#include "Foundation.h"
#include <cstdarg>
#include <functional>
#include <boost/any.hpp>
#include <shared_mutex>
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>
// #include <string_view>

#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <deque>

// #include <iostream>

// typenames, function_traits, Compare-and-swap, atomic shared_ptr
namespace GoodLang {
	namespace utilities {
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<typename R> struct function_traits {
			typedef R result_type;
			typedef std::tuple<> arguments;
		};
		template<typename R> struct function_traits<std::function<R(void)>> {
			typedef R result_type;
			typedef std::tuple<> arguments;
		};
		template<typename R, typename... Args> struct function_traits<std::function<R(Args...)>> {
			typedef R result_type;
			typedef std::tuple<Args...> arguments;
		};

		template <typename T, typename U> struct helper : helper<T, decltype(&U::operator())> {};
		template <typename T, typename C, typename R, typename... A> struct helper<T, R(C::*)(A...) const> { static const bool value = std::is_convertible<T, R(*)(A...)>::value; };
		template<typename T> struct is_stateless { static const bool value = helper<T, T>::value; };

		template <typename T, typename = std::void_t<>>
		struct is_std_hashable : std::false_type { };

		template <typename T>
		struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>> : std::true_type { };

		template <typename T>
		constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

		template <typename T, typename = std::void_t<>>
		struct is_std_stringable : std::false_type { };

		template <typename T>
		struct is_std_stringable<T, std::void_t<decltype(std::to_string(std::declval<T>()))>> : std::true_type { };

		template <typename T>
		constexpr bool is_std_stringable_v = is_std_stringable<T>::value;
	};

	namespace cas_impl {
		template<typename... Args> struct UnionDetails {
		public:
			using byte = unsigned char;
			template<int N> using NthTypeOf = typename std::remove_const<typename std::remove_reference<typename std::tuple_element<N, std::tuple<Args...>>::type>::type>::type;
			static constexpr size_t num_parameters = sizeof...(Args);
			template<int N>	static constexpr size_t SizeOfFirstN() {
				size_t d(0);
				if constexpr (num_parameters >= 1 && N >= 1) {
					d += sizeof(NthTypeOf<0>);
				}
				if constexpr (num_parameters >= 2 && N >= 2) {
					d += sizeof(NthTypeOf<1>);
				}
				if constexpr (num_parameters >= 3 && N >= 3) {
					d += sizeof(NthTypeOf<2>);
				}
				if constexpr (num_parameters >= 4 && N >= 4) {
					d += sizeof(NthTypeOf<3>);
				}
				if constexpr (num_parameters >= 5 && N >= 5) {
					d += sizeof(NthTypeOf<4>);
				}
				if constexpr (num_parameters >= 6 && N >= 6) {
					d += sizeof(NthTypeOf<5>);
				}
				if constexpr (num_parameters >= 7 && N >= 7) {
					d += sizeof(NthTypeOf<6>);
				}
				if constexpr (num_parameters >= 8 && N >= 8) {
					d += sizeof(NthTypeOf<7>);
				}
				if constexpr (num_parameters >= 9 && N >= 9) {
					d += sizeof(NthTypeOf<8>);
				}
				if constexpr (num_parameters >= 10 && N >= 10) {
					d += sizeof(NthTypeOf<9>);
				}
				if constexpr (num_parameters >= 11 && N >= 11) {
					d += sizeof(NthTypeOf<10>);
				}
				if constexpr (num_parameters >= 12 && N >= 12) {
					d += sizeof(NthTypeOf<11>);
				}
				if constexpr (num_parameters >= 13 && N >= 13) {
					d += sizeof(NthTypeOf<12>);
				}
				if constexpr (num_parameters >= 13 && N >= 14) {
					d += sizeof(NthTypeOf<13>);
				}
				if constexpr (num_parameters >= 14 && N >= 15) {
					d += sizeof(NthTypeOf<14>);
				}
				if constexpr (num_parameters >= 15 && N >= 16) {
					d += sizeof(NthTypeOf<15>);
				}
				return d;
			};
			static constexpr size_t SizeOfAll() {
				return SizeOfFirstN<num_parameters>();
			};
			static constexpr size_t sizeOfArgs = SizeOfAll();
		private:
			static constexpr size_t bitOffset_0 = SizeOfFirstN<0>();
			static constexpr size_t bitOffset_1 = SizeOfFirstN<1>();
			static constexpr size_t bitOffset_2 = SizeOfFirstN<2>();
			static constexpr size_t bitOffset_3 = SizeOfFirstN<3>();
			static constexpr size_t bitOffset_4 = SizeOfFirstN<4>();
			static constexpr size_t bitOffset_5 = SizeOfFirstN<5>();
			static constexpr size_t bitOffset_6 = SizeOfFirstN<6>();
			static constexpr size_t bitOffset_7 = SizeOfFirstN<7>();
			static constexpr size_t bitOffset_8 = SizeOfFirstN<8>();
			static constexpr size_t bitOffset_9 = SizeOfFirstN<9>();
			static constexpr size_t bitOffset_10 = SizeOfFirstN<10>();
			static constexpr size_t bitOffset_11 = SizeOfFirstN<11>();
			static constexpr size_t bitOffset_12 = SizeOfFirstN<12>();
			static constexpr size_t bitOffset_13 = SizeOfFirstN<13>();
			static constexpr size_t bitOffset_14 = SizeOfFirstN<14>();
			static constexpr size_t bitOffset_15 = SizeOfFirstN<15>();
		public:
			template <int N> static NthTypeOf<N>* PtrAt(byte* data) { return static_cast<NthTypeOf<N>*>(static_cast<void*>(&data[SizeOfFirstN<N>()])); };

		};
		template <typename type1, typename type2, typename type3, typename type4> class CAS4 {
		private:
			using wrapperDetails = typename UnionDetails<type1, type2, type3, type4>;
			static constexpr size_t offset0 = 0;
			static constexpr size_t offset1 = offset0 + sizeof(type1);
			static constexpr size_t offset2 = offset1 + sizeof(type2);
			static constexpr size_t offset3 = offset2 + sizeof(type3);

		public:
			struct Data {
			protected:
				typename wrapperDetails::byte data[wrapperDetails::sizeOfArgs];

			public:
				type1& a() {
					return *reinterpret_cast<type1*>(&data[offset0]);
				};
				type2& b() {
					return *reinterpret_cast<type2*>(&data[offset1]);
				};
				type3& c() {
					return *reinterpret_cast<type3*>(&data[offset2]);
				};
				type4& d() {
					return *reinterpret_cast<type4*>(&data[offset3]);
				};

				Data() = default;
				~Data() = default;
				Data(type1 A, type2 B = {}, type3 C = {}, type4 D = {}) {
					a() = std::move(A);
					b() = std::move(B);
					c() = std::move(C);
					d() = std::move(D);
				};
			};

		public:
			CAS4(type1 a = {}, type2 b = {}, type3 c = {}, type4 d = {}) : read_write(Data(a, b, c, d)) {};
			CAS4(Data const& RHS) : read_write(RHS) {};
			CAS4(CAS4 const& RHS) : read_write(RHS.read_write.load()) {};
			CAS4(CAS4&& RHS) : read_write(RHS.read_write.load()) {};
			CAS4& operator=(CAS4 const& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			CAS4& operator=(CAS4&& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			~CAS4() = default;

			void store(Data const RHS) {
				read_write.store(std::move(RHS), std::memory_order::memory_order_relaxed);
			};
			Data load() const {
				return read_write.load(std::memory_order::memory_order_acquire);
			};
			Data exchange(Data const RHS) {
				return read_write.exchange(std::move(RHS), std::memory_order::memory_order_release);
			};
			bool compare_exchange_strong(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_strong(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};
			bool compare_exchange_weak(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_weak(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};

		private:
			std::atomic < Data > read_write;
			static_assert(decltype(read_write)::is_always_lock_free);
		public:
			static constexpr bool is_always_lock_free{ decltype(read_write)::is_always_lock_free };
		};
		template <typename type1, typename type2, typename type3> class CAS3 {
		private:
			using wrapperDetails = UnionDetails<type1, type2, type3>;
		public:
			struct Data {
			protected:
				typename wrapperDetails::byte data[wrapperDetails::sizeOfArgs];

			public:
				type1& a() {
					return *wrapperDetails::PtrAt<0>(&data[0]);
				};
				type2& b() {
					return *wrapperDetails::PtrAt<1>(&data[0]);
				};
				type3& c() {
					return *wrapperDetails::PtrAt<2>(&data[0]);
				};

				Data() = default;
				~Data() = default;
				Data(type1 A, type2 B = {}, type3 C = {}) {
					a() = A;
					b() = B;
					c() = C;
				};
			};

		public:
			CAS3(type1 a = {}, type2 b = {}, type3 c = {}) : read_write(Data(a, b, c)) {};
			CAS3(Data const& RHS) : read_write(RHS) {};
			CAS3(CAS3 const& RHS) : read_write(RHS.read_write.load()) {};
			CAS3(CAS3&& RHS) : read_write(RHS.read_write.load()) {};
			CAS3& operator=(CAS3 const& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			CAS3& operator=(CAS3&& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			~CAS3() = default;

			void store(Data const RHS) {
				read_write.store(std::move(RHS), std::memory_order::memory_order_relaxed);
			};
			Data load() const {
				return read_write.load(std::memory_order::memory_order_acquire);
			};
			Data exchange(Data const RHS) {
				return read_write.exchange(std::move(RHS), std::memory_order::memory_order_release);
			};
			bool compare_exchange_strong(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_strong(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};
			bool compare_exchange_weak(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_weak(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};

		private:
			std::atomic < Data > read_write;
			static_assert(decltype(read_write)::is_always_lock_free);
		public:
			static constexpr bool is_always_lock_free{ decltype(read_write)::is_always_lock_free };
		};
		template <typename type1, typename type2> class CAS2 {
		private:
			using wrapperDetails = UnionDetails<type1, type2>;
		public:
			struct Data {
			protected:
				typename wrapperDetails::byte data[wrapperDetails::sizeOfArgs];

			public:
				type1& a() {
					return *wrapperDetails::PtrAt<0>(&data[0]);
				};
				type2& b() {
					return *wrapperDetails::PtrAt<1>(&data[0]);
				};

				Data() = default;
				~Data() = default;
				Data(type1 A, type2 B = {}) {
					a() = A;
					b() = B;
				};
			};
		public:
			CAS2(type1 a = {}, type2 b = {}) : read_write(Data(a, b)) {};
			CAS2(Data const& RHS) : read_write(RHS) {};
			CAS2(CAS2 const& RHS) : read_write(RHS.read_write.load()) {};
			CAS2(CAS2&& RHS) : read_write(RHS.read_write.load()) {};
			CAS2& operator=(CAS2 const& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			CAS2& operator=(CAS2&& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			~CAS2() = default;

			void store(Data const RHS) {
				read_write.store(std::move(RHS), std::memory_order::memory_order_relaxed);
			};
			Data load() const {
				return read_write.load(std::memory_order::memory_order_acquire);
			};
			Data exchange(Data const RHS) {
				return read_write.exchange(std::move(RHS), std::memory_order::memory_order_release);
			};
			bool compare_exchange_strong(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_strong(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};
			bool compare_exchange_weak(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_weak(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};

		private:
			std::atomic < Data > read_write;
			static_assert(decltype(read_write)::is_always_lock_free);
		public:
			static constexpr bool is_always_lock_free{ decltype(read_write)::is_always_lock_free };
		};
		template <typename type1> class CAS1 {
		private:
			using wrapperDetails = UnionDetails<type1>;
		public:
			struct Data {
			protected:
				typename wrapperDetails::byte data[wrapperDetails::sizeOfArgs];

			public:
				type1& a() {
					return *wrapperDetails::PtrAt<0>(&data[0]);
				};

				Data() = default;
				~Data() = default;
				Data(type1 A) {
					a() = A;
				};
			};

		public:
			CAS1(type1 a = {}) : read_write(Data(a)) {};
			CAS1(Data const& RHS) : read_write(RHS) {};
			CAS1(CAS1 const& RHS) : read_write(RHS.read_write.load()) {};
			CAS1(CAS1&& RHS) : read_write(RHS.read_write.load()) {};
			CAS1& operator=(CAS1 const& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			CAS1& operator=(CAS1&& RHS) { read_write.store(RHS.read_write.load()); return *this; };
			~CAS1() = default;

			void store(Data const RHS) {
				read_write.store(std::move(RHS), std::memory_order::memory_order_relaxed);
			};
			Data load() const {
				return read_write.load(std::memory_order::memory_order_acquire);
			};
			Data exchange(Data const RHS) {
				return read_write.exchange(std::move(RHS), std::memory_order::memory_order_release);
			};
			bool compare_exchange_strong(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_strong(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};
			bool compare_exchange_weak(Data& _Expected, const Data _Desired) {
				return read_write.compare_exchange_weak(_Expected, std::move(_Desired), std::memory_order_release, std::memory_order_relaxed);
			};

		private:
			std::atomic < Data > read_write;
			static_assert(decltype(read_write)::is_always_lock_free);
		public:
			static constexpr bool is_always_lock_free{ decltype(read_write)::is_always_lock_free };
		};
	};

	/// <summary>
	/// Single-word compare-and-swap wrapper for multiple types that fit within the size of a uint64_t, like 4 shorts or 2 ints. 
	/// </summary>
	/// <typeparam name="...Args"></typeparam>
	template <typename... Args> class CAS {
	private:
		using wrapperDetails = cas_impl::UnionDetails<Args...>;
		static_assert(wrapperDetails::num_parameters >= 1);
		static_assert(wrapperDetails::num_parameters <= 4);

		static auto TypeTest() {
			if constexpr (wrapperDetails::num_parameters == 1) {
				return cas_impl::CAS1<Args...>();
			}
			else if constexpr (wrapperDetails::num_parameters == 2) {
				return cas_impl::CAS2<Args...>();
			}
			else if constexpr (wrapperDetails::num_parameters == 3) {
				return cas_impl::CAS3<Args...>();
			}
			else if constexpr (wrapperDetails::num_parameters == 4) {
				return cas_impl::CAS4<Args...>();
			}
		};
		typedef typename GoodLang::utilities::function_traits<decltype(std::function(&TypeTest)) >::result_type thisType;
		typename thisType read_write;

	public:
		using Data = typename thisType::Data;
		static constexpr bool is_always_lock_free{ decltype(read_write)::is_always_lock_free };
		static constexpr size_t TupleSize() {
			if constexpr (wrapperDetails::num_parameters == 1) {
				return 1;
			}
			else if constexpr (wrapperDetails::num_parameters == 2) {
				return 2;
			}
			else if constexpr (wrapperDetails::num_parameters == 3) {
				return 3;
			}
			else if constexpr (wrapperDetails::num_parameters == 4) {
				return 4;
			}
		};

		CAS() : read_write() {};
		CAS(Data const& RHS) : read_write(RHS) {};
		CAS(CAS const& RHS) : read_write(RHS.read_write.load()) {};
		CAS(CAS&& RHS) : read_write(RHS.read_write.load()) {};
		CAS& operator=(CAS const& RHS) { read_write.store(RHS.read_write.load()); return *this; };
		CAS& operator=(CAS&& RHS) { read_write.store(RHS.read_write.load()); return *this; };
		~CAS() = default;

		void store(Data const RHS) {
			read_write.store(std::move(RHS));
		};
		Data load() const {
			return read_write.load();
		};
		Data exchange(Data const RHS) {
			return read_write.exchange(std::move(RHS));
		};
		bool compare_exchange_strong(Data& _Expected, const Data _Desired) {
			return read_write.compare_exchange_strong(_Expected, std::move(_Desired));
		};
		bool compare_exchange_weak(Data& _Expected, const Data _Desired) {
			return read_write.compare_exchange_weak(_Expected, std::move(_Desired));
		};
	};

	/// <summary>
	/// Thread-safe and fiber-safe wrapper for atomic operations on pointers, without having to utilize std_atomic(T*)
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template< typename T> struct atomic_ptr {
	private:
		static void* Sys_InterlockedExchangePointer(void*& ptr, void* exchange) {
			return InterlockedExchangePointer(&ptr, exchange);
		};
		static void* Sys_InterlockedCompareExchangePointer(void*& ptr, void* comparand, void* exchange) {
			return InterlockedCompareExchangePointer(&ptr, exchange, comparand);
		};

	public:
		constexpr atomic_ptr() noexcept : ptr(nullptr) {}
		constexpr atomic_ptr(T* newSource) noexcept : ptr(newSource) {}
		constexpr atomic_ptr(const atomic_ptr& other) noexcept : ptr(other.ptr) {};
		atomic_ptr& operator=(const atomic_ptr& other) noexcept { Set(other.Get()); return *this; };
		atomic_ptr& operator=(T* newSource) noexcept { Set(newSource); return *this; };
		~atomic_ptr() { ptr = nullptr; };

		explicit operator bool() { return ptr; };
		explicit operator bool() const { return ptr; };

		operator T* () noexcept { return ptr; };
		operator const T* () const noexcept { return ptr; };

		/* atomically sets the pointer and returns the previous pointer value */
		T* Set(T* newPtr) noexcept {
			return static_cast<T*>(Sys_InterlockedExchangePointer((void*&)ptr, static_cast<void*>(newPtr)));
		};
		bool TrySet(T* newPtr, T*& oldPtr) noexcept {
			T* PREV_VAL = this->load();
			if (this->CompareExchange(PREV_VAL, newPtr) == PREV_VAL) {
				oldPtr = PREV_VAL;
				return true;
			}
			else {
				return false;
			}
		};
		bool TrySet(T* newPtr, atomic_ptr<T>& oldPtr) noexcept {
			T* PREV_VAL = this->load();
			if (this->CompareExchange(PREV_VAL, newPtr) == PREV_VAL) {
				oldPtr = PREV_VAL;
				return true;
			}
			else {
				return false;
			}
		};

		/* atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr' */
		T* CompareExchange(T* comparePtr, T* newPtr) noexcept {
			return static_cast<T*>(Sys_InterlockedCompareExchangePointer((void*&)ptr, static_cast<void*>(comparePtr), static_cast<void*>(newPtr)));
		};

		T* operator->() noexcept { return Get(); };
		const T* operator->() const noexcept { return Get(); };
		T* Get() noexcept { return ptr; };
		T* Get() const noexcept { return ptr; };
		T* load() noexcept { return Get(); };
		T* load() const noexcept { return Get(); };

	protected:
		T* ptr;
	};

	template<class T> class weak_ptr; // forward-decl

	class shared_ptr_base {
	public:
		struct aux {
			std::atomic<long long> // strong (first short), weak (second short), destroy flag (third short), delete flag (fourth short)
				Strong_Weak_Destroy_Delete{ 1 }; // strong = 1, weak = 0, destroy = 0, delete = 0

			aux() = default;

			virtual void* ptr() const = 0;
			virtual void destroy() = 0;
			virtual ~aux() {} //must be polymorphic
		};
		static void PreventDeletion(aux* const& ptr);
		static void AllowDeletion(aux* const& ptr);
		// requires that the ptr is NOT already locked through PreventDeletion
		static void DoDeletion(aux* const& ptr);
		// requires that the ptr is NOT already locked through PreventDeletion
		static void DoDestroyOrDelete(aux* const& ptr, bool Destroy, bool Delete);
		template<class U, class Deleter> struct auximpl : public aux {
			void* p;
			Deleter d;
			auximpl(U* pu, Deleter x) : aux(), p(static_cast<void*>(pu)), d(std::move(x)) {}
			virtual void* ptr() const override { return p; };
			virtual void destroy() override { d(static_cast<U*>(p)); }
		};
		template<class U> struct auxlocalimpl : public aux {
			unsigned char p[sizeof(U)];

			template <class... _Types>
			auxlocalimpl(_Types&&... _Args) : aux() {
				auto* ptr = reinterpret_cast<void*>(&p[0]);
				new (ptr) U(_STD forward<_Types>(_Args)...);
			};
			virtual void* ptr() const override { return reinterpret_cast<void*>(const_cast<unsigned char*>(&p[0])); };
			virtual void destroy() override {
				reinterpret_cast<U*>(&p[0])->~U();
			};
		};
		template<class U> struct default_deleter {
			void operator()(U* p) const { delete p; };
		};
		// USER MUST ALLOW DELETION AFTER RECIEVING THE PTR
		static aux* inc(GoodLang::atomic_ptr<aux> const& pa);
		// ASSUMES THAT THE PTR COMES IN LOCKED.
		static void dec(aux* pa_ptr);
		// USER MUST ALLOW DELETION AFTER RECIEVING THE PTR
		static aux* inc_weak(GoodLang::atomic_ptr<aux> const& pa);
		// ASSUMES THAT THE PTR COMES IN LOCKED.
		static void dec_weak(aux* pa_ptr);
	};

	/// <summary>
	/// Thread-safe implimentation of std::shared_ptr. Slower in single-thread cases, faster (and race-free) in multi-threaded cases. weak_ptr dereferencing is particularly slow here. 
	/// </summary>
	/// <returns></returns>
	template<class T> class shared_ptr : public shared_ptr_base {
	protected:
		friend class weak_ptr<T>;

		GoodLang::atomic_ptr<aux> pa; // pointer to shared memory block
		T* pt;

		static T* get(shared_ptr const& p) {
			T*
				out{ nullptr };
			aux
				* pa_ptr{ nullptr },
				* pa_ptr_copy{ nullptr };
			long long
				read;

			while (pa_ptr_copy = pa_ptr = p.pa.load()) {
				// prevent its deletion while we work on it. This does not access it, it simply locks the region the pointer belongs to, HOPING to prevent collisions. 
				PreventDeletion(pa_ptr_copy);
				if (pa_ptr_copy == (pa_ptr = p.pa.load())) {
					if (pa_ptr) {
						read = pa_ptr->Strong_Weak_Destroy_Delete.load();
						if (!reinterpret_cast<short*>(&read)[2] && !reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed or deleted...
							out = static_cast<T*>(pa_ptr->ptr());
							AllowDeletion(pa_ptr_copy);
							return out;
						}
					}
				}
				AllowDeletion(pa_ptr_copy);
			}
			return out;
		};

		explicit shared_ptr(aux* p) : pa(p), pt(nullptr) {
			pt = get(*this);
		};

	public:
		const auto& GetPa() const {
			return pa;
		};

		shared_ptr() :pa(nullptr), pt(nullptr) {}
		shared_ptr(std::nullptr_t) : pa(nullptr), pt(nullptr) {}
		explicit shared_ptr(aux* pa_p, T* pt_p, bool) :pa(pa_p), pt(pt_p) {} // used to initialize a new ptr from custom aux-type.

		template<class U, class Deleter> shared_ptr(U* pu, Deleter d) : pa(new auximpl<U, Deleter>(pu, d)), pt(reinterpret_cast<T*>(pu)) {}
		template<class U> explicit shared_ptr(U* pu) : pa(new auximpl<U, default_deleter<U> >(pu, default_deleter<U>())), pt(reinterpret_cast<T*>(pu)) {}

		template<class U> shared_ptr(shared_ptr<U> const& s) : pa(nullptr), pt(nullptr) {
			auto* new_pa = shared_ptr_base::inc(s.GetPa()); // this is locked! 
			if (new_pa) {
				pt = static_cast<T*>(new_pa->ptr());
			}
			pa = new_pa;
			shared_ptr_base::AllowDeletion(new_pa);
		};
		shared_ptr(shared_ptr<T> const& s) : pa(nullptr), pt(nullptr) {
			auto* new_pa = shared_ptr_base::inc(s.GetPa()); // this is locked! 
			if (new_pa) {
				pt = static_cast<T*>(new_pa->ptr());
			}
			pa = new_pa;
			shared_ptr_base::AllowDeletion(new_pa);
		};

		~shared_ptr() {
			auto* ptr = pa.load();
			shared_ptr_base::PreventDeletion(ptr);
			shared_ptr_base::dec(ptr);
		}

		shared_ptr& operator=(const shared_ptr& s) {
			if (this != &s) {
				InterlockedExchangePointer(reinterpret_cast<void**>(&pt), nullptr);
				auto* new_ptr = shared_ptr_base::inc(s.pa); // this is locked! 
				auto* old_ptr = pa.Set(new_ptr);
				shared_ptr_base::AllowDeletion(new_ptr);
				shared_ptr_base::PreventDeletion(old_ptr);
				shared_ptr_base::dec(old_ptr);
			}
			return *this;
		};
		template<class U> shared_ptr& operator=(const shared_ptr<U>& s) {
			InterlockedExchangePointer(reinterpret_cast<void**>(&pt), nullptr);
			auto* new_ptr = shared_ptr_base::inc(s.GetPa()); // this is locked! 
			auto* old_ptr = pa.Set(new_ptr);
			shared_ptr_base::AllowDeletion(new_ptr);
			shared_ptr_base::PreventDeletion(old_ptr);
			shared_ptr_base::dec(old_ptr);
			return *this;
		};
		shared_ptr& operator=(std::nullptr_t) {
			InterlockedExchangePointer(reinterpret_cast<void**>(&pt), nullptr);
			auto* old_ptr = pa.Set(nullptr);
			shared_ptr_base::PreventDeletion(old_ptr);
			shared_ptr_base::dec(old_ptr);
			return *this;
		};

		T* get() const {
			if (pt) return pt;
			return get(*this);
		};
		operator bool() const {
			return get();
		};
		T* operator->() const {
			return get();
		};
		T& operator*() const {
			return *get();
		};

		friend bool operator==(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() == b.get(); };
		friend bool operator!=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() != b.get(); };
		friend bool operator<(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() < b.get(); };
		friend bool operator<=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() <= b.get(); };
		friend bool operator>(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() > b.get(); };
		friend bool operator>=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() >= b.get(); };
		friend bool operator==(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() == nullptr; };
		friend bool operator!=(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() != nullptr; };
		friend bool operator<(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() < nullptr; };
		friend bool operator<=(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() <= nullptr; };
		friend bool operator>(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() > nullptr; };
		friend bool operator>=(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() >= nullptr; };
		friend bool operator==(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr == a.get(); };
		friend bool operator!=(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr != a.get(); };
		friend bool operator<(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr < a.get(); };
		friend bool operator<=(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr <= a.get(); };
		friend bool operator>(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr > a.get(); };
		friend bool operator>=(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr >= a.get(); };
	};

	/// <summary>
	/// Thread-safe implimentation of std::weak_ptr. Slower in single-thread cases, faster (and race-free) in multi-threaded cases. weak_ptr dereferencing is particularly slow here if locks are not needed. 
	/// </summary>
	/// <returns></returns>
	template<class T> class weak_ptr {
		GoodLang::atomic_ptr<shared_ptr_base::aux> pa; // pointer to shared memory block

	public:
		weak_ptr() : pa(nullptr) {}
		weak_ptr(std::nullptr_t) : pa(nullptr) {}
		weak_ptr(shared_ptr<T> const& r) : pa(nullptr) {
			auto* new_pa = shared_ptr_base::inc_weak(r.pa); // this is locked! 
			pa = new_pa;
			shared_ptr_base::AllowDeletion(new_pa);
		};
		weak_ptr(const weak_ptr& r) : pa(nullptr) {
			auto* new_pa = shared_ptr_base::inc_weak(r.pa); // this is locked! 
			pa = new_pa;
			shared_ptr_base::AllowDeletion(new_pa);
		};
		~weak_ptr() {
			auto* ptr = pa.load();
			shared_ptr_base::PreventDeletion(ptr);
			shared_ptr_base::dec_weak(ptr);
		}

		operator bool() const {
			return !expired();
		};

		weak_ptr& operator=(const weak_ptr& s) {
			if (this != &s) {
				auto* new_ptr = shared_ptr_base::inc_weak(s.pa); // this is locked! 
				auto* old_ptr = pa.Set(new_ptr);
				shared_ptr_base::AllowDeletion(new_ptr);
				shared_ptr_base::PreventDeletion(old_ptr);
				shared_ptr_base::dec_weak(old_ptr);
			}
			return *this;
		};
		weak_ptr& operator=(std::nullptr_t) {
			auto* old_ptr = pa.Set(nullptr);
			shared_ptr_base::PreventDeletion(old_ptr);
			shared_ptr_base::dec_weak(old_ptr);
			return *this;
		};

		shared_ptr<T> lock() const {
			auto* new_ptr = shared_ptr_base::inc(pa); // this is locked! 
			auto out{ shared_ptr<T>(new_ptr) };
			shared_ptr_base::AllowDeletion(new_ptr);
			return out;
		};
		bool expired() {
			if (auto* pa_ptr = pa.load()) {
				shared_ptr_base::PreventDeletion(pa_ptr);
				auto read = pa_ptr->Strong_Weak_Destroy_Delete.load();
				if ((reinterpret_cast<short*>(&read)[0] < std::numeric_limits<short>::max()) && !reinterpret_cast<short*>(&read)[2] && !reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed or deleted...
					shared_ptr_base::AllowDeletion(pa_ptr);
					return false;
				}
				else {
					shared_ptr_base::AllowDeletion(pa_ptr);
					return true;
				}
			}
			return true;
		};
	};

	template <class _Ty, class... _Types>
	_NODISCARD shared_ptr<_Ty> make_shared(_Types&&... _Args) { // make a shared_ptr to non-array object
		if constexpr (sizeof(_Ty) > (sizeof(uint64_t) * 16)) {
			// large objects should be deleted / free-d seperately from the memory block, otherwise there's a risk that a weak_ptr could hold that memory for a long time
			return shared_ptr<_Ty>(new _Ty(_STD forward<_Types>(_Args)...));
		}
		else {
			// small objects should be included in the memory block
			auto* aux = new shared_ptr_base::auxlocalimpl<_Ty>(_STD forward<_Types>(_Args)...);
			return shared_ptr<_Ty>(aux, reinterpret_cast<_Ty*>(&aux->p[0]), true);
		}
	};
};

// printf
namespace GoodLang {
	__forceinline size_t		vsnPrintf(char* dest, size_t size, const char* fmt, va_list argptr) {
		size_t ret;
#undef _vsnprintf
		ret = ::_vsnprintf(dest, size - 1, fmt, argptr);
		dest[size - 1] = '\0';
		if (ret < 0 || ret >= size)  ret = -1;
		return ret;
	};
	__forceinline std::string	printf(const char* fmt, ...) {
		thread_local static std::shared_ptr<char> sp{ nullptr };
		if (!sp) {
			sp = std::shared_ptr<char>(new char[128000], std::default_delete<char[]>());
		}

		va_list argptr;

		char* buffer = sp.get();
		buffer[128000 - 1] = '\0';

		va_start(argptr, fmt);
		vsnPrintf(buffer, 128000 - 1, fmt, argptr);
		va_end(argptr);
		buffer[128000 - 1] = '\0';

		std::string out(buffer);
		return out;
	};
};

// std::hash for double type
//namespace std {
//	template <> struct hash<double> {
//		std::size_t operator()(double k) const {
//			return *(uint64_t*)(void*)(&k);
//		};
//	};
//};

// forward decl
namespace GoodLang {
	class Any;
	class AnyData;
	template <typename T> class AnyData_Shared;
};

// Type_Info
namespace GoodLang {
	namespace impl {
		template<typename T> static const auto& TypeId() {
			static auto typeIdOfT{ boost::typeindex::type_id<T>() };
			return typeIdOfT.type_info();
		};
		typedef decltype(TypeId<void>()) underlying_type_info;
	};
	namespace details {
		template<typename T>
		struct Bare_Type {
			typedef typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type type;
		};

		inline static void hash_combine(std::size_t& seed) { };
		template <typename T, typename... Rest> inline static void hash_combine(std::size_t& seed, T const& v, Rest const&... rest) {
			if constexpr (std::is_same_v<double, typename std::remove_reference_t<typename std::decay<T>>>) {
				seed ^= *(uint64_t*)(void*)(&v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			else {
				std::hash<T> hasher{};
				seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			hash_combine(seed, rest...);
		};
	};

	// Type_Info records the type of either built-in or scripted, runtime types
	//class BuiltIn_Type_Info; class Scripted_Type_Info;
	class Type_Info {
	protected:
		virtual size_t GetHashImpl() const;
		void CacheHash() noexcept;

	public:
		size_t GetHash() const;

		Type_Info() noexcept
			: uniqueHash(GetHashImpl())
			, underlyingHash(0)
			, isConst(false)
			, isVoid(true)
			, isRef(false)
		{};
		Type_Info(bool t_is_const, bool t_is_void, bool t_is_ref, bool t_is_any) noexcept
			: uniqueHash(GetHashImpl())
			, underlyingHash(0)
			, isConst(t_is_const)
			, isVoid(t_is_void)
			, isRef(t_is_ref)
			, isAny(t_is_any)
		{};
		Type_Info(Type_Info const&) = delete;
		Type_Info(Type_Info&&) = delete;
		Type_Info& operator=(Type_Info const&) = delete;
		Type_Info& operator=(Type_Info&&) = delete;
		virtual ~Type_Info() = default;

		// Returns true if the types are similar enough to be casted
		static bool CanCast(Type_Info const& from, Type_Info const& to);
		// Returns true if the types are similar enough to be casted
		bool CanCast(Type_Info const& to) const;

		//// Operators
		friend bool operator==(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() == b.GetHash();
		};
		friend bool operator!=(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() != b.GetHash();
		};
		friend bool operator<(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() < b.GetHash();
		};
		friend bool operator<=(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() <= b.GetHash();
		};
		friend bool operator>(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() > b.GetHash();
		};
		friend bool operator>=(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() >= b.GetHash();
		};

		// Query
		bool is_const() const noexcept;
		bool is_void() const noexcept;
		bool is_ref() const noexcept;
		bool is_any() const noexcept;
		virtual std::string name() const noexcept;
		virtual std::weak_ptr<Type_Info> MakeBase() const;
		virtual std::weak_ptr<Type_Info> MakeConst() const;
		virtual std::weak_ptr<Type_Info> MakeRef() const;
		virtual std::weak_ptr<Type_Info> MakeConstRef() const;
		virtual std::weak_ptr<Type_Info> RemoveConst() const;
		virtual std::weak_ptr<Type_Info> RemoveRef() const;

		virtual std::function<Any(Any const&)>& GetCopyConstructor() const; 
		virtual bool IsBuiltInType() const;

		const size_t uniqueHash;
		const size_t underlyingHash;
	private:
		bool isConst;
		bool isVoid;
		bool isRef;
		bool isAny;

	};

	template <typename T>
	class BuiltIn_Type_Info final : public Type_Info {
	protected:
		virtual size_t GetHashImpl() const override {
			return this->m_type_info.hash_code();
		};

	public:
		BuiltIn_Type_Info() noexcept
			: Type_Info(false, true, false, false)
			, m_type_info(impl::TypeId<T>())
		{
			this->CacheHash();
		};
		BuiltIn_Type_Info(impl::underlying_type_info t_ti, bool t_is_const = false, bool t_is_ref = false) noexcept
			: Type_Info(t_is_const, std::is_same<typename std::decay_t<T>, void>::value, t_is_ref, std::is_same<typename std::decay_t<T>, Any>::value)
			, m_type_info(t_ti)
		{
			this->CacheHash();
		};
		BuiltIn_Type_Info(BuiltIn_Type_Info const&) = delete;
		BuiltIn_Type_Info(BuiltIn_Type_Info&&) = delete;
		BuiltIn_Type_Info& operator=(BuiltIn_Type_Info const&) = delete;
		BuiltIn_Type_Info& operator=(BuiltIn_Type_Info&&) = delete;
		virtual ~BuiltIn_Type_Info() = default;

		virtual std::string name() const noexcept override {
			if (is_const()) {
				if (is_ref()) {
					return std::string("const ") + std::string(m_type_info.name()) + "&";
				}
				else {
					return std::string("const ") + std::string(m_type_info.name());
				}
			}
			else {
				if (is_ref()) {
					return std::string(m_type_info.name()) + "&";
				}
				else {
					return m_type_info.name();
				}
			}
		};
		impl::underlying_type_info type_info() const noexcept {
			return m_type_info;
		};
		virtual std::weak_ptr<Type_Info> MakeBase() const override {
			typedef typename std::decay_t<T> baseType;
			static auto out{ std::make_shared<BuiltIn_Type_Info<baseType>>(
				impl::TypeId<baseType>(),
				false,
				false
			) };
			return out;
		};;
		virtual std::weak_ptr<Type_Info> MakeConst() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			typedef typename std::decay_t<T> baseType;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
						impl::TypeId<void>(),
						false,
						false
				) };
				return out;
			}
			else {
				if constexpr (thisIsRef) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType&>>(
							impl::TypeId<const baseType&>(),
							true,
							thisIsRef
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType>>(
							impl::TypeId<const baseType>(),
							true,
							thisIsRef
					) };
					return out;
				}
			}
		};;
		virtual std::weak_ptr<Type_Info> MakeRef() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			typedef typename std::decay_t<T> baseType;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
						impl::TypeId<void>(),
						false,
						false
				) };
				return out;
			}
			else {
				if constexpr (thisIsConst) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType&>>(
							impl::TypeId<const baseType&>(),
							thisIsConst,
							true
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType&>>(
							impl::TypeId<baseType&>(),
							thisIsConst,
							true
					) };
					return out;
				}
			}
		};;
		virtual std::weak_ptr<Type_Info> MakeConstRef() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			typedef typename std::decay_t<T> baseType;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
						impl::TypeId<void>(),
						false,
						false
				) };
				return out;
			}
			else {
				static auto out{ std::make_shared<BuiltIn_Type_Info<baseType&>>(
						impl::TypeId<baseType&>(),
						true,
						true
				) };
				return out;
			}
		};;
		virtual std::weak_ptr<Type_Info> RemoveConst() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			typedef typename std::decay_t<T> baseType;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
						impl::TypeId<void>(),
						false,
						false
				) };
				return out;
			}
			else {
				if constexpr (thisIsRef) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType&>>(
							impl::TypeId<baseType&>(),
							false,
							thisIsRef
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType>>(
							impl::TypeId<baseType>(),
							false,
							thisIsRef
					) };
					return out;
				}
			}
		};;
		virtual std::weak_ptr<Type_Info> RemoveRef() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			typedef typename std::decay_t<T> baseType;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
						impl::TypeId<void>(),
						false,
						false
				) };
				return out;
			}
			else {
				if constexpr (thisIsConst) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType>>(
							impl::TypeId<const baseType>(),
							thisIsConst,
							false
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType>>(
							impl::TypeId<baseType>(),
							thisIsConst,
							false
					) };
					return out;
				}
			}
		};
		virtual std::function<Any(Any const&)>& GetCopyConstructor() const override; 

	private:
		impl::underlying_type_info m_type_info;

	};

	class Scripted_Type_Info final : public Type_Info {
	protected:
		virtual size_t GetHashImpl() const override;

	public:
		Scripted_Type_Info() noexcept
			: Type_Info(false, true, false, false)
			, m_full_name("")
			, m_qualified_namespace("")
			, m_name("")
			, m_uniqueHash(impl::TypeId<void>().hash_code())
		{
			this->CacheHash();
		};
		Scripted_Type_Info(const std::string& t_namespace, const std::string& t_name, bool t_is_const = false, bool t_is_ref = false) noexcept
			: Type_Info(t_is_const, false, t_is_ref, false)
			, m_full_name(t_namespace + "::" + t_name)
			, m_qualified_namespace(t_namespace)
			, m_name(t_name)
			, m_uniqueHash(std::hash<std::string>()(t_namespace + "::" + t_name))
		{
			this->CacheHash();
		};
		Scripted_Type_Info(Scripted_Type_Info const&) = delete;
		Scripted_Type_Info(Scripted_Type_Info&&) = delete;
		Scripted_Type_Info& operator=(Scripted_Type_Info const&) = delete;
		Scripted_Type_Info& operator=(Scripted_Type_Info&&) = delete;
		virtual ~Scripted_Type_Info() = default;

		virtual std::string name() const noexcept;
		void SetSelf(std::shared_ptr<Scripted_Type_Info>& t_self);
		virtual std::weak_ptr<Type_Info> MakeBase() const override;
		virtual std::weak_ptr<Type_Info> MakeConst() const override;
		virtual std::weak_ptr<Type_Info> MakeRef() const override;
		virtual std::weak_ptr<Type_Info> MakeConstRef() const override;
		virtual std::weak_ptr<Type_Info> RemoveConst() const override;
		virtual std::weak_ptr<Type_Info> RemoveRef() const override;
		virtual std::function<Any(Any const&)>& GetCopyConstructor() const override;
		virtual bool IsBuiltInType() const override;

	protected:
		std::string m_full_name; // namespace::name
		std::string m_qualified_namespace; // namespace
		std::string m_name; // name
		size_t m_uniqueHash; // std::hash<std::string>()(m_full_name)

		std::weak_ptr<Scripted_Type_Info> m_self;
		std::weak_ptr<Scripted_Type_Info> m_parent;
		mutable GoodLang::fast_shared_mutex m_children_mut;
		mutable std::unordered_map<size_t, std::shared_ptr<Scripted_Type_Info>> m_children;
		std::weak_ptr<Type_Info> MakeDuplicate(bool targetConst, bool targetRef) const;

	};
};

// Type_Info std::hash
namespace std {
	template <> struct hash<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& k) const {
			return k.GetHash();
		};
	};
	template <> struct hash<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& k) const {
			static auto voidHash{ GoodLang::impl::TypeId<void>().hash_code() };
			if (k) {
				return k->GetHash();
			}
			else {
				return voidHash;
			}
		};
	};
	template <> struct hash<std::weak_ptr<GoodLang::Type_Info>> {	
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& k) const {
			static auto voidHash{ GoodLang::impl::TypeId<void>().hash_code() };
			if (auto p = k.lock()) {
				return p->GetHash();
			}
			else {
				return voidHash;
			}
		};
	};
};

// GetHash
namespace GoodLang {
	template <typename T> std::hash<typename details::Bare_Type<T>::type>& GetHash() {
		static auto hasher{ std::hash<typename details::Bare_Type<T>::type>{} };
		return hasher;
	};
	template <typename T> size_t GetHash(const T& a) {
		return GetHash<T>()(a);
	};
	template<> size_t GetHash<Type_Info>(Type_Info const& r);
	template<> size_t GetHash<std::shared_ptr<Type_Info>>(std::shared_ptr<Type_Info> const& r);
	template<> size_t GetHash<std::weak_ptr<Type_Info>>(std::weak_ptr<Type_Info> const& r);
};

// Type_Info std::less, std::greater, std::equal_to
namespace std {
	template <> struct less<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& lhs, const GoodLang::Type_Info& rhs) const {
			return GoodLang::GetHash(lhs) < GoodLang::GetHash(rhs);
		};
	};
	template <> struct less<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& lhs, const std::shared_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) < GoodLang::GetHash(rhs);
		};
	};
	template <> struct less<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& lhs, const std::weak_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) < GoodLang::GetHash(rhs);
		};
	};

	template <> struct greater<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& lhs, const GoodLang::Type_Info& rhs) const {
			return GoodLang::GetHash(lhs) > GoodLang::GetHash(rhs);
		};
	};
	template <> struct greater<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& lhs, const std::shared_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) > GoodLang::GetHash(rhs);
		};
	};
	template <> struct greater<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& lhs, const std::weak_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) > GoodLang::GetHash(rhs);
		};
	};

	template <> struct equal_to<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& lhs, const GoodLang::Type_Info& rhs) const {
			return GoodLang::GetHash(lhs) == GoodLang::GetHash(rhs);
		};
	};
	template <> struct equal_to<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& lhs, const std::shared_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) == GoodLang::GetHash(rhs);
		};
	};
	template <> struct equal_to<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& lhs, const std::weak_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) == GoodLang::GetHash(rhs);
		};
	};
};

// user_type & user_type_shared
namespace GoodLang {
	namespace details {
		/// Helper used to create a Type_Info object
		template<typename T>
		struct Get_Type_Info {
			static std::shared_ptr<BuiltIn_Type_Info<T>> get() noexcept {
				return std::make_shared<BuiltIn_Type_Info<T>>(
					impl::TypeId<T>(),
					std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value,
					std::is_reference<typename std::remove_pointer<T>::type>::value
				);
			};
		};

		template<typename T> struct Get_Type_Info<std::shared_ptr<T>> : Get_Type_Info<T> {};

		template<typename T> struct Get_Type_Info<std::shared_ptr<T>&> : Get_Type_Info<std::shared_ptr<T>> {};

		template<typename T> struct Get_Type_Info<const std::shared_ptr<T>&> : Get_Type_Info<T> {};
	} // namespace detail

	/// \brief Creates a Type_Info object representing the templated type
	/// \tparam T Type of object to get a Type_Info for
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// chaiscript::Type_Info ti = chaiscript::user_type<int>();
	/// \endcode
	template<typename T> std::shared_ptr<Type_Info> const& user_type_shared_ptr() noexcept {
		static std::shared_ptr<Type_Info> out{ std::dynamic_pointer_cast<Type_Info>(details::Get_Type_Info<T>::get()) };
		return out;
	};

	/// \brief Creates a Type_Info object representing the templated type
	/// \tparam T Type of object to get a Type_Info for
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// chaiscript::Type_Info ti = chaiscript::user_type<int>();
	/// \endcode
	template<typename T> std::weak_ptr<Type_Info> const& user_type_shared() noexcept {
		static std::weak_ptr<Type_Info> out2{ user_type_shared_ptr<T>() };
		return out2;
	};

	/// \brief Creates a Type_Info object representing the templated type
	/// \tparam T Type of object to get a Type_Info for
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// chaiscript::Type_Info ti = chaiscript::user_type<int>();
	/// \endcode
	template<typename T> const Type_Info& user_type() noexcept {
		static std::shared_ptr<Type_Info> out{ user_type_shared<T>().lock() };
		static const Type_Info& toReturn{ *out };
		return toReturn;
	};

	/// \brief Creates a Type_Info object representing the type passed in
	/// \tparam T Type of object to get a Type_Info for, derived from the passed in parameter
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// int i;
	/// chaiscript::Type_Info ti = chaiscript::user_type(i);
	/// \endcode
	template<typename T> const Type_Info& user_type(const T& /*t*/) noexcept {
		return user_type<T>();
	};
};

__forceinline bool operator==(std::weak_ptr<GoodLang::Type_Info> const& a, std::weak_ptr<GoodLang::Type_Info> const& b) {
	return GetHash(a) == GetHash(b);
};
__forceinline bool operator!=(std::weak_ptr<GoodLang::Type_Info> const& a, std::weak_ptr<GoodLang::Type_Info> const& b) {
	return !operator==(a, b);
};
__forceinline bool operator==(std::weak_ptr<GoodLang::Type_Info> const& a, GoodLang::Type_Info const& b) {
	return GetHash(a) == GetHash(b);
};
__forceinline bool operator!=(std::weak_ptr<GoodLang::Type_Info> const& a, GoodLang::Type_Info const& b) {
	return !operator==(a, b);
};
__forceinline bool operator==(GoodLang::Type_Info const& b, std::weak_ptr<GoodLang::Type_Info> const& a) {
	return GetHash(a) == GetHash(b);
};
__forceinline bool operator!=(GoodLang::Type_Info const& b, std::weak_ptr<GoodLang::Type_Info> const& a) {
	return !operator==(a, b);
};

namespace GoodLang::Impl {
	struct UniqueNode {
		const void* address;
		const GoodLang::Type_Info* type;
	};
	template <typename T> UniqueNode GetNode(const T& r) {
		return {
			static_cast<const void*>(&r),
			&user_type<T>()
		};
	};

	class NodeCache {
	public:
		std::vector< NodeCache > children{};
		UniqueNode self{};
		std::shared_ptr<AnyData> data{ nullptr };
		// std::shared_ptr<AnyData> recursive_source{ nullptr };
		int refCount{ 0 };
		long long recursiveFlag{ 0 };
	};
};

__forceinline bool operator==(GoodLang::Impl::UniqueNode const& b, GoodLang::Impl::UniqueNode const& a) {
	return (a.address == b.address) && (a.type == b.type);
};
__forceinline bool operator!=(GoodLang::Impl::UniqueNode const& b, GoodLang::Impl::UniqueNode const& a) {
	return !operator==(a, b);
};
namespace std {
	template <> struct less<GoodLang::Impl::UniqueNode> {
		std::size_t operator()(const GoodLang::Impl::UniqueNode& lhs, const GoodLang::Impl::UniqueNode& rhs) const {
			size_t hash1{ 0 };
			GoodLang::details::hash_combine(hash1, lhs.address, lhs.type);
			size_t hash2{ 0 };
			GoodLang::details::hash_combine(hash2, rhs.address, rhs.type);
			return hash1 < hash2;
		};
	};
};

// ToString
namespace GoodLang {
	namespace exception {
		struct recursive_function_error : std::runtime_error {
			recursive_function_error()
				: std::runtime_error("")
			{}
			recursive_function_error(const recursive_function_error&) = default;
			~recursive_function_error() noexcept override = default;
		};
	}; // namespace exception
	
	namespace Impl {
		template <typename T> struct Tag {}; // Necessary to correctly coordinate creation of functions in correct order.

		static const bool find_recursion(GoodLang::Impl::NodeCache const& current_cache, std::deque<const GoodLang::Impl::NodeCache*>& path) {			
			if (path.size() > 0) {
				if (current_cache.recursiveFlag == 1) { // we started the final recursion search. We are looking for -1 from here on out.
					const GoodLang::Impl::NodeCache* this_cache{ &current_cache };
					// int max_iter = 100;
					while (this_cache) { //this_cache && ((max_iter--) >= 0)) {
						auto& children = this_cache->children;
						this_cache = nullptr;
						for (int i = 0; i < children.size(); i++) {
							if (children[i].recursiveFlag < 0) {
								path.push_back(this_cache = &children[i]);
								break;
							}
						}
					}
					return true; // regardless, return true. 
				}
				else {
					for (auto& child : current_cache.children) {
						path.push_back(&child);
						if (find_recursion(child, path/*, FoundRecursion*/)) {
							return true;
						}
						path.pop_back();
					}
				}
			}
			else {
				path.push_back(&current_cache);
				for (auto& child : current_cache.children) {
					path.push_back(&child);
					if (find_recursion(child, path/*, FoundRecursion*/)) {
						return true;
					}
					path.pop_back();
				}
				path.pop_back();
			}
			return false;
		};

		class ImplClass {
		private:
			static size_t& GetCurrentDepth() {
				thread_local static size_t currentDepth{ 0 };
				return currentDepth;
			};
			static size_t& GetCurrentDepthLimit() {
				thread_local static size_t currentDepthLimit{ std::numeric_limits<size_t>::max() };
				return currentDepthLimit;
			};

		public:
			template <typename T> static std::string ToStringImpl(T const& value) {
				thread_local static size_t recursion_detection{ 0 };
				constexpr int maxNumReEntry{ 50 };

				std::string out;
				size_t entryNumber = recursion_detection++;
				defer(recursion_detection--);

				if (entryNumber <= 1) {
					try { ToString(Tag<T>(), value, out); }
					catch (exception::recursive_function_error const& e) { return "..."; }					
				}
				else if (entryNumber < maxNumReEntry) ToString(Tag<T>(), value, out);
				else throw exception::recursive_function_error();
				return out;
			};
			
			template <typename T> static Impl::NodeCache GetChildrenImpl(T const& parent) {
				thread_local static size_t recursion_detection{ 0 };
				constexpr int maxNumReEntry{ 50 }; // maximum times we are allowed to see this type re-entered before it becomes unlikely to be intentional.
				
				// This approach is a cheap alternative to tracking the actually-visitied pointers.
				// Issue with tracking visited pointers is that what method is used during iteration matters -- for example:
				//      for (auto& x : std::map<int, int>{...}){ GetChildren(x); }
				// Resulted in an incorrect recursion detection, because the iterator during the loop was being shared between iterations.
				// This cheap approach is immune to that type of issue (as far as I've tested at least). 

				Impl::NodeCache out{}; {
					out.self = GetNode(parent);
					out.refCount = 0;
					out.data = std::dynamic_pointer_cast<AnyData>(std::make_shared<AnyData_Shared<T>>(std::shared_ptr<T>(const_cast<T*>(&parent), [](T*) { /* do nothing */ })));
				}
				size_t entryNumber = recursion_detection++;
				size_t depthNumber = GetCurrentDepth()++;
				if (depthNumber == 0) GetCurrentDepthLimit() = std::numeric_limits<size_t>::max(); // reset
				defer(recursion_detection--; GetCurrentDepth()--);

				if (entryNumber < GetCurrentDepthLimit()) {
					if (entryNumber <= 1) {
						GetChildren(Tag<T>(), parent, out.children);
						for (auto& child : out.children) {
							if (child.recursiveFlag < 0) {
								out.recursiveFlag = 1;
								break;
							}
							else if (child.recursiveFlag > 0) {
								out.recursiveFlag = std::max(out.recursiveFlag, child.recursiveFlag + 1);
							}
						}
					}
					else if (entryNumber < maxNumReEntry) {
						try {
							GetChildren(Tag<T>(), parent, out.children);
							for (auto& child : out.children) {
								if (child.recursiveFlag < 0) {
									out.recursiveFlag = child.recursiveFlag - 1;
									// out.children = {};
									break;
								}
								else if (child.recursiveFlag > 0) {
									out.recursiveFlag = std::max(out.recursiveFlag, child.recursiveFlag + 1);
								}
							}
						}
						catch (exception::recursive_function_error const&) {
							out.recursiveFlag = -1;
							// out.children = {};
						}
					}
					else {
						// out.children = {};
						out.recursiveFlag = -1; // negative one indicates it is the source of the recursion
						GetCurrentDepthLimit() = std::min<size_t>(GetCurrentDepthLimit(), 3);
						throw exception::recursive_function_error();
					}
				}
				else {
					// loop must end
				}

#if 1
				
#else
				if (entryNumber >= maxNumReEntry) {
					out.recursiveFlag = 1;
					out.children = {};
				}
				else if (entryNumber == 1) {
					GetChildren(Tag<T>(), parent, out.children);
					for (auto& child : out.children) {
						if (child.recursiveFlag > 0) {
							out.children = {}; // clear the children from the list.
							out.recursiveFlag = 1; // child.recursiveFlag + 1;
							break;
						}
					}
				}
				else {
					GetChildren(Tag<T>(), parent, out.children);
					for (auto& child : out.children) {
						if (child.recursiveFlag > 0) {
							out.recursiveFlag = child.recursiveFlag + 1;
							break;
						}
					}
				}
#endif
				return out;
			};
			template <typename T> static bool TryDisconnectChildImpl(T const& parent) {
				bool out{ false };
				TryDisconnectChild(Tag<T>(), parent, out);
				return out;
			};
		};
	}; // namespace Read

	// ToString
	template <typename T> __forceinline std::string ToString(T const& value) { 
		return Impl::ImplClass::ToStringImpl<T>(value);
	};
	template <size_t N> __forceinline std::string ToString(const char(&r)[N]) { return r; };
	namespace Impl {
		// ultimate fall-back
		template <typename T> __forceinline void ToString(Tag<T>, T const& r, std::string& out) {
			if constexpr (GoodLang::utilities::is_std_stringable<T>::value) {
				out = std::to_string(r);
			}
			else {
				out = GoodLang::user_type<T>().name();
			}			
		};
		// specializations
		__forceinline void ToString(Tag<Impl::UniqueNode>, Impl::UniqueNode const& r, std::string& out) {
			out = r.type->name();
		};
		namespace NodeCachedetails {
			static void recursive(int indentLevel, GoodLang::Impl::NodeCache const& cache, std::string& out) {
				if (cache.recursiveFlag == 1) {
					out += GoodLang::ToString(cache.self);
					out += " { ...recursive... };\n";
				}
				else {
					out += GoodLang::ToString(cache.self);
					if (cache.recursiveFlag > 0) out += std::string("*") + std::to_string(cache.recursiveFlag) + "*";

					if (cache.children.size() == 0) {						
						out += ";\n";
					}
					else if (cache.children.size() == 1) {
						out += " -> ";
						recursive(indentLevel, cache.children[0], out);
					}
					else {
						out += " -> ";
						out += std::to_string(cache.children.size());
						out += " children: { \n";

						for (auto& child : cache.children) {
							for (int i = 0; i < indentLevel + 1; i++) out += "\t";
							recursive(indentLevel + 1, child, out);
						}

						for (int i = 0; i < indentLevel; i++) out += "\t";

						out += "}\n";
					}
				}
			};
		};
		__forceinline void ToString(Tag<Impl::NodeCache>, Impl::NodeCache const& r, std::string& out) {
			NodeCachedetails::recursive(0, r, out);
		};
		__forceinline void ToString(Tag<std::nullptr_t>, nullptr_t const&, std::string& out) {
			out = "nullptr";
		};
		__forceinline void ToString(Tag<GoodLang::Type_Info>, GoodLang::Type_Info const& r, std::string& out) {
			out = r.name();
		};
		__forceinline void ToString(Tag<bool>, bool const& r, std::string& out) {
			if (r) out = "true";
			else out = "false";
		};
		__forceinline void ToString(Tag<char>, char const& r, std::string& out) {
			out = std::string(1, r);
		};
		__forceinline void ToString(Tag<unsigned char>, unsigned char const& r, std::string& out) {
			out = std::string(1, r);
		};
		__forceinline void ToString(Tag<signed char>, signed char const& r, std::string& out) {
			out = std::string(1, r);
		};
		__forceinline void ToString(Tag<std::string>, std::string const& r, std::string& out) {
			out = r;
		};
		__forceinline void ToString(Tag<const char*>, const char* const& r, std::string& out) {
			out = r;
		};
		__forceinline void ToString(Tag<const void*>, const void* const& r, std::string& out) {
			out = GoodLang::ToString((unsigned long long)r);
		};
		template <typename T> __forceinline void ToString(Tag<std::shared_ptr<T>>, std::shared_ptr<T> const& r, std::string& out) {
			if (r) out = GoodLang::ToString(*r);
			else out = "nullptr";
		};
		template <typename T> __forceinline void ToString(Tag<std::weak_ptr<T>>, std::weak_ptr<T> const& r, std::string& out) {
			out = GoodLang::ToString(r.lock());
		};
		template <typename... Args> __forceinline void ToString(Tag<std::unique_ptr<Args...>>, std::unique_ptr<Args...> const& r, std::string& out) {
			if (r) out = GoodLang::ToString(*r);
			else out = "nullptr";
		};
		template <typename... Args> __forceinline void ToString(Tag<std::pair<Args...>>, std::pair<Args...> const& r, std::string& out) {
			out = std::string("<") + GoodLang::ToString(r.first) + ", " + GoodLang::ToString(r.second) + ">";
		};
		template <typename... Args> __forceinline void ToString(Tag<std::vector<Args...>>, std::vector<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
		template <typename... Args> __forceinline void ToString(Tag<std::set<Args...>>, std::set<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
		template <typename... Args> __forceinline void ToString(Tag<std::unordered_set<Args...>>, std::unordered_set<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
		template <typename... Args> __forceinline void ToString(Tag<std::map<Args...>>, std::map<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
		template <typename... Args> __forceinline void ToString(Tag<std::unordered_map<Args...>>, std::unordered_map<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
		template <typename... Args> __forceinline void ToString(Tag<concurrency::concurrent_unordered_map<Args...>>, concurrency::concurrent_unordered_map<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
		template <typename... Args> __forceinline void ToString(Tag<concurrency::concurrent_unordered_set<Args...>>, concurrency::concurrent_unordered_set<Args...> const& r, std::string& out) {
			for (auto& x : r) {
				if (out.empty())
					out += GoodLang::ToString(x);
				else
					out += std::string(", ") + GoodLang::ToString(x);
			}
			out = std::string("[") + out + std::string("]");
		};
    };

	// GetChildren
	template <typename T> __forceinline Impl::NodeCache GetChildren(T const& value) {
		return Impl::ImplClass::GetChildrenImpl<T>(value);
	};
	namespace Impl {
		// ultimate fall-back
		template <typename T> __forceinline void GetChildren(Tag<T>, T const& r, std::vector< NodeCache >& out) {};
		// specializations
		template <typename T> __forceinline void GetChildren(Tag<std::shared_ptr<T>>, std::shared_ptr<T> const& r, std::vector< NodeCache >& out) {
			if (r) out.push_back(GoodLang::GetChildren(*r));
		};
		template <typename T> __forceinline void GetChildren(Tag<std::unique_ptr<T>>, std::unique_ptr<T> const& r, std::vector< NodeCache >& out) {
			if (r) out.push_back(GoodLang::GetChildren(*r));
		};
		template <typename T> __forceinline void GetChildren(Tag<std::weak_ptr<T>>, std::weak_ptr<T> const& r, std::vector< NodeCache >& out) {	};
		template <typename... Args> __forceinline void GetChildren(Tag<std::pair<Args...>>, std::pair<Args...> const& r, std::vector< NodeCache >& out) {
			out.push_back(GoodLang::GetChildren(r.first));
			out.push_back(GoodLang::GetChildren(r.second));
		};
		template <typename... Args> __forceinline void GetChildren(Tag<std::vector<Args...>>, std::vector<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};
		template <typename... Args> __forceinline void GetChildren(Tag<std::set<Args...>>, std::set<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};
		template <typename... Args> __forceinline void GetChildren(Tag<std::unordered_set<Args...>>, std::unordered_set<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};
		template <typename... Args> __forceinline void GetChildren(Tag<std::map<Args...>>, std::map<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};
		template <typename... Args> __forceinline void GetChildren(Tag<std::unordered_map<Args...>>, std::unordered_map<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};
		template <typename... Args> __forceinline void GetChildren(Tag<concurrency::concurrent_unordered_map<Args...>>, concurrency::concurrent_unordered_map<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};
		template <typename... Args> __forceinline void GetChildren(Tag<concurrency::concurrent_unordered_set<Args...>>, concurrency::concurrent_unordered_set<Args...> const& r, std::vector< NodeCache >& out) {
			for (auto& x : r) {
				out.push_back(GoodLang::GetChildren(x));
			}
		};

	};

	// TryDisconnectChild
	template <typename T> __forceinline bool TryDisconnectChild(T const& value) {
		return Impl::ImplClass::TryDisconnectChildImpl<T>(value);
	};
	namespace Impl {
		// ultimate fall-back
		template <typename T> __forceinline void TryDisconnectChild(Tag<T>, T const& r, bool& out) {};
		// specializations
		template <typename T> __forceinline void TryDisconnectChild(Tag<std::shared_ptr<T>>, std::shared_ptr<T> const& r, bool& out) {
			const_cast<std::shared_ptr<T>&>(r) = nullptr;
			out = true;
		};
	};

};

// DynamicObject && Var
namespace GoodLang {
	// serves as an instance of a customizable class
	class DynamicObject {
	public:
		DynamicObject()
			: m_actualType(std::weak_ptr< Type_Info >())
			, m_classType(std::weak_ptr< Type_Info >())
			, m_objects(std::make_shared<concurrency::concurrent_unordered_map<std::string, std::shared_ptr<Any>>>())
		{};
		DynamicObject(std::weak_ptr< Type_Info > const& type)
			: m_actualType(type)
			, m_classType(type)
			, m_objects(std::make_shared<concurrency::concurrent_unordered_map<std::string, std::shared_ptr<Any>>>())
		{};
		// Upcast from a child_class to a parent_class (e.g. from class C : public A {} to class A {})
		DynamicObject(std::weak_ptr< Type_Info > const& castedType, DynamicObject const& parent)
			: m_actualType(parent.m_actualType)
			, m_classType(castedType)
			, m_objects(parent.m_objects)
		{};
		DynamicObject(DynamicObject const&) = default;
		DynamicObject(DynamicObject&&) = default;
		DynamicObject& operator=(DynamicObject const&) = default;
		DynamicObject& operator=(DynamicObject&&) = default;
		~DynamicObject() = default;

		std::weak_ptr< Type_Info >
			m_actualType;
		std::weak_ptr< Type_Info >
			m_classType;
		std::shared_ptr<concurrency::concurrent_unordered_map<std::string, std::shared_ptr<Any>>>
			m_objects;
	};
	namespace Impl {
		__forceinline void ToString(Tag<DynamicObject>, DynamicObject const& r, std::string& out) {
			out = GoodLang::ToString(r.m_actualType) + "{ " + GoodLang::ToString(r.m_objects) + " }";
		};
		__forceinline void GetChildren(Tag<DynamicObject>, DynamicObject const& r, std::vector< NodeCache >& out) {
			out = { GoodLang::GetChildren(r.m_objects) };
		};
	};

#define AllowInlineVarTyping
	/* class "Var" is a generic container for dynamically typed objects for use in the scripting language.
	It defers from "Any" because Any objects are for use in C++ to contain statically typed objects.
	"Var" objects are wrappers for Anys that allow the scripting language to process them as
	empty & assignable, or filled and implimented */
	class Var {
	public:
		Var() : p_data(std::make_shared<Any>()) {}
		explicit Var(Any const& data_f) : p_data(std::make_shared<Any>(data_f)) {};
		Var(Var const&) = default;
		Var(Var&&) = default;
		Var& operator=(Var const&) = default;
		Var& operator=(Var&&) = default;
		~Var() = default;

	public:
		std::shared_ptr<Any> p_data;

	public:
		friend bool operator==(Var const& _Left, Var const& _Right) {
			return _Left.p_data == _Right.p_data;
		};
		friend bool operator!=(Var const& _Left, Var const& _Right) {
			return !operator==(_Left, _Right);
		};
	};
	namespace Impl {
		__forceinline void ToString(Tag<Var>, Var const& r, std::string& out) {
			if (r.p_data) {
				out = GoodLang::ToString(*r.p_data);
			}
			else {
				out = GoodLang::ToString(r.p_data);
			}
		};
		__forceinline void GetChildren(Tag<Var>, Var const& r, std::vector< NodeCache >& out) {
			if (r.p_data) {
				out.push_back(GoodLang::GetChildren(*r.p_data));
			}
		};
		__forceinline void TryDisconnectChild(Tag<Var>, Var const& r, bool& out) {
			const_cast<Var&>(r).p_data = nullptr;
			out = true;
		};
	};
};

// Any, AnyAutoCast, DynamicObject, exceptions
namespace GoodLang {
	namespace exception {
		/// \brief Thrown in the event that an Any cannot be cast to the desired type
		/// It is used internally during function dispatch.
		class bad_any_cast : public std::bad_cast {
		public:
			/// \brief Description of what error occurred
			const char* what() const noexcept override { return exc.c_str(); }

			bad_any_cast() :
				bad_cast(std::bad_cast::__construct_from_string_literal("Bad Any Cast")),
				from(user_type<void>()),
				to(user_type<void>()),
				exc("Bad Any Cast")
			{};
			bad_any_cast(GoodLang::Type_Info const& from_m, GoodLang::Type_Info const& to_m, int lineNumber) :
				bad_cast(std::bad_cast::__construct_from_string_literal((std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber)).c_str())),
				from(from_m),
				to(to_m),
				exc(std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber))
			{};
			bad_any_cast(std::weak_ptr<GoodLang::Type_Info> from_m, std::weak_ptr<GoodLang::Type_Info> to_m, int lineNumber) :
				bad_cast(std::bad_cast::__construct_from_string_literal((std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber)).c_str())),
				from(TypeFromPtr(from_m)),
				to(TypeFromPtr(to_m)),
				exc(std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber))
			{};
		private:
			std::string exc;
			GoodLang::Type_Info const& from;
			GoodLang::Type_Info const& to;

			static std::string NameFromType(GoodLang::Type_Info const& x) { return x.name(); };
			static std::string NameFromType(std::weak_ptr<GoodLang::Type_Info> const& x) { if (auto y = x.lock()) return y->name(); else return user_type<void>().name(); };
			static GoodLang::Type_Info const& TypeFromPtr(std::weak_ptr<GoodLang::Type_Info> const& x) { if (auto y = x.lock()) return *y; else return user_type<void>(); };
		};

		/**
		* Exception thrown when there is a mismatch in number of
		* parameters during Proxy_Function execution
		*/
		struct arity_error : std::range_error {
			arity_error(int t_got, int t_expected, long t_lineN)
				: std::range_error(
					t_expected >= 0 ?
					GoodLang::printf("{line %i} Arity mismatch: function requires %i parameters, but only %i were provided", t_lineN, t_expected, t_got)
					: std::string("Function was not found")
				)
				, got(t_got)
				, expected(t_expected) {
			}

			arity_error(const arity_error&) = default;

			~arity_error() noexcept override = default;

			int got;
			int expected;
		};

		/**
		* Exception thrown when there is a mismatch in number of
		* parameters during Proxy_Function execution
		*/
		struct not_found_error : std::runtime_error {
			not_found_error(const std::string& triedToFind)
				: std::runtime_error(
					GoodLang::printf("Could not find \"%s\"", triedToFind.c_str())
				), m_triedToFind(triedToFind)
			{}
			not_found_error(const not_found_error&) = default;
			~not_found_error() noexcept override = default;

			std::string m_triedToFind;
		};

	}; // namespace exception

	namespace details {
		template<class T> struct get_type { typedef T type; };
		template<class T> struct get_type<std::shared_ptr<T>> { typedef typename get_type<T>::type type; };
		template<class T> struct get_type<std::shared_ptr<T>&> { typedef typename get_type<T>::type type; };
		template<class T> struct get_type<std::shared_ptr<T>*> { typedef typename get_type<T>::type type; };
		template<class T> struct get_type<const std::shared_ptr<T>> { typedef typename get_type<T>::type type; };
		template<class T> struct get_type<const std::shared_ptr<T>&> { typedef typename get_type<T>::type type; };
		template<class T> struct get_type<const std::shared_ptr<T>*> { typedef typename get_type<T>::type type; };

	};

	class AnyData {
	public:
		enum class Flag {
			constant = 0
		};

		AnyData() noexcept = default;
		AnyData(AnyData const&) = delete;
		AnyData(AnyData&&) = delete;
		AnyData& operator=(AnyData const&) = delete;
		AnyData& operator=(AnyData&&) = delete;
		virtual ~AnyData() = default;

	public:
		// user must set the ptr to the AnyData object, so that it is aware of itself
		void SetSelf(std::shared_ptr< AnyData>& t_self);

	public:
		size_t GetTypeHash() const;
		virtual bool CanCast(Type_Info const& to_type) const;
		virtual Type_Info const& GetType() const;
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const;
		virtual std::shared_ptr<Type_Info> const& GetTypeSharedPtr() const;
		virtual void* ptr() const;
		virtual std::shared_ptr<void> shared_ptr() const;
		template<typename ToType> std::shared_ptr<ToType> cast_shared() const {
			Type_Info const& to_type{ user_type<ToType>() };

			if (CanCast(to_type)) {
				return std::static_pointer_cast<ToType>(shared_ptr());
			}
			else {
				return nullptr;
			}
		};
		template<typename ToType> ToType* cast() const {
			Type_Info const& to_type{ user_type<ToType>() };

			if (CanCast(to_type)) {
				return static_cast<ToType*>(ptr());
			}
			else {
				return nullptr;
			}
		};
		void ThrowIfNot(Type_Info const& type) const;
		template<typename T> static std::shared_ptr<void> get_data(const std::shared_ptr<T>& data) {
			if constexpr (std::is_const< T >::value) {
				return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(data));
			}
			else {
				return std::static_pointer_cast<void>(data);
			}
		};
		template<typename T> static std::shared_ptr<void> get_data(std::shared_ptr<T>&& data) {
			if constexpr (std::is_const< T >::value) {
				return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(std::forward<std::shared_ptr<T>>(data)));
			}
			else {
				return std::static_pointer_cast<void>(std::forward<std::shared_ptr<T>>(data));
			}
			// return std::static_pointer_cast<void>(std::const_pointer_cast<std::remove_const_t<T>>(std::forward<std::shared_ptr<T>>(data)));
		};
		virtual std::string ToString() const { return ""; };
		virtual std::vector< Impl::NodeCache > GetChildren() const { return std::vector< Impl::NodeCache >{}; };
		virtual bool TryDisconnectChild() const { return false; };
		bool GetFlag(Flag which) const;
		void SetFlag(Flag which, bool newV);

	protected:
		std::weak_ptr< AnyData> m_self;
		size_t typeHash;
		bool m_flags[1];
	};	

	template <typename T>
	class AnyData_Instanced final : public AnyData {
	public:
		AnyData_Instanced() noexcept 
			: AnyData()
			, m_obj()
		{}
		AnyData_Instanced(T const& t_obj) noexcept
			: AnyData()
			, m_obj(t_obj)
		{ };
		AnyData_Instanced(T && t_obj) noexcept
			: AnyData()
			, m_obj(std::forward<T>(t_obj))
		{ };
		AnyData_Instanced(AnyData_Instanced const&) = delete;
		AnyData_Instanced(AnyData_Instanced&&) = delete;
		AnyData_Instanced& operator=(AnyData_Instanced const&) = delete;
		AnyData_Instanced& operator=(AnyData_Instanced&&) = delete;
		virtual ~AnyData_Instanced() = default;

		virtual bool CanCast(Type_Info const& to_type) const override { return GetType().CanCast(to_type); };
		virtual Type_Info const& GetType() const override { return user_type<T>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const override { 
			return user_type_shared<T>();
		};
		virtual std::shared_ptr<Type_Info> const& GetTypeSharedPtr() const override {
			return user_type_shared_ptr<T>();
		};
		virtual void* ptr() const override { return static_cast<void*>(&const_cast<std::remove_const_t<T>&>(m_obj)); };
		virtual std::shared_ptr<void> shared_ptr() const override {
			if (auto p = m_self.lock()) {
				auto P = std::shared_ptr<T>(&const_cast<std::remove_const_t<T>&>(m_obj), [PTR = p](T* toDelete) { (void)PTR->ptr(); /* ThrowIfNot(user_type<T>());*/ });
				return std::static_pointer_cast<void>(std::const_pointer_cast<std::remove_const_t<T>>(P));
			}
			return nullptr;
		};
		virtual std::string ToString() const override {
			return GoodLang::ToString(m_obj);
		};
		virtual std::vector< Impl::NodeCache > GetChildren() const override {
			return { GoodLang::GetChildren(m_obj) };
		};
		//virtual bool TryDisconnectChild() const override { 
		//	return GoodLang::TryDisconnectChild(m_obj);
		//};

	private:
		T m_obj;

	};

	template <typename T>
	class AnyData_Shared final : public AnyData {
	public:
		AnyData_Shared() noexcept
			: AnyData()
			, m_obj()
		{ };
		AnyData_Shared(std::shared_ptr<T> const& t_obj) noexcept
			: AnyData()
			, m_obj(t_obj)
		{ };
		AnyData_Shared(std::shared_ptr<T>&& t_obj) noexcept
			: AnyData()
			, m_obj(std::forward<std::shared_ptr<T>>(t_obj))
		{ };
		AnyData_Shared(AnyData_Shared const&) = delete;
		AnyData_Shared(AnyData_Shared&&) = delete;
		AnyData_Shared& operator=(AnyData_Shared const&) = delete;
		AnyData_Shared& operator=(AnyData_Shared&&) = delete;
		virtual ~AnyData_Shared() = default;

		virtual bool CanCast(Type_Info const& to_type) const override { return GetType().CanCast(to_type); };
		virtual Type_Info const& GetType() const override { return user_type<T>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const override {
			return user_type_shared<T>();
		};
		virtual std::shared_ptr<Type_Info> const& GetTypeSharedPtr() const override {
			return user_type_shared_ptr<T>();
		};
		virtual void* ptr() const override { return const_cast<void*>((const void*)(m_obj.get())); };
		virtual std::shared_ptr<void> shared_ptr() const override { return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(m_obj)); };
		virtual std::string ToString() const override {
			return GoodLang::ToString(*m_obj);
		};
		virtual std::vector< Impl::NodeCache > GetChildren() const override {
			if (m_obj) {
				return { GoodLang::GetChildren(*m_obj) };
			}
			else {
				return {};
			}
		};
		virtual bool TryDisconnectChild() const override {
			if (m_obj) {
				return { GoodLang::TryDisconnectChild(*m_obj) };
			}
			else {
				return false;
			}
		};

	private:
		std::shared_ptr<T> m_obj;

	};

	namespace Impl {
		__forceinline void ToString(Tag<AnyData>, AnyData const& r, std::string& out) {
			out = r.ToString();
		};
		__forceinline void GetChildren(Tag<AnyData>, AnyData const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		//__forceinline void TryDisconnectChild(Tag<AnyData>, AnyData const& r, bool& out) {
		//	out = r.TryDisconnectChild();
		//};
	};

	namespace details {
		class AnyAutoCast; /* forward decl */
	};

	/*! Generic container that enables the containment and sharing of any data type to/from std::shared_ptrs */
	class Any {
	public:
		struct Object_Data {
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static std::shared_ptr<AnyData> get(const H<S>* obj) { return get(*obj); };
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static std::shared_ptr<AnyData> get(H<S> obj) {
				if (obj) {
					if constexpr (std::is_same<Any, S>::value) {
						return obj->container;
					}
					else {
						std::shared_ptr<AnyData> instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Shared<S>>(std::move(obj)));
						instanced_any->SetSelf(instanced_any);
						return instanced_any;
					}
				}
				else {
					return std::shared_ptr<AnyData>();
				}
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static std::shared_ptr<AnyData> get(T* t) { return get(std::make_shared<T>(t)); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static std::shared_ptr<AnyData> get(const T* t) { return get(*t); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static std::shared_ptr<AnyData> get(const T& obj) {
				// return get((std::decay_t<T>)obj); // makes a copy

				std::shared_ptr<AnyData> instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::decay_t<T>>>(obj));
				instanced_any->SetSelf(instanced_any);
				return instanced_any;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static std::shared_ptr<AnyData> get(T&& obj) {
				std::shared_ptr<AnyData> instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::decay_t<T>>>(std::forward<T>(obj)));
				instanced_any->SetSelf(instanced_any);
				return instanced_any;
			};

			static std::shared_ptr<AnyData> get(const GoodLang::details::AnyAutoCast& obj);
			static std::shared_ptr<AnyData> get(const GoodLang::details::AnyAutoCast* t);
		};
		template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(const ValueType& r) { return Object_Data::get(r); };
		template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(ValueType&& r) { return Object_Data::get(std::forward<ValueType>(r)); };

	public: /*! Init */
		Any() noexcept
			: container(nullptr)
			, mut()
		{};
		Any(std::nullptr_t) noexcept
			: container(nullptr)
			, mut()
		{};
		Any(const Any& rhs) noexcept
			: container()
			, mut()
		{
			rhs.mut.lock_shared();
			container = rhs.container;
			rhs.mut.unlock_shared();
		};
		Any(Any&& rhs) noexcept
			: container(std::move(rhs.container))
			, mut()
		{};

	public: /*! Init w/ DATA ASSIGNMENT */
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType& value) noexcept
			: container(CreateContainer(value))
			, mut()
		{};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType* value) noexcept
			: container(CreateContainer(value))
			, mut()
		{};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType* value) noexcept
			: container(CreateContainer(value))
			, mut()
		{};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType&& value) noexcept
			: container(CreateContainer(std::forward<ValueType>(value)))
			, mut()
		{};

	public: /*! Destroy */
		~Any() = default; // { Clear(); };

	public: /*! Data Assignment AFTER INIT */
		Any& swap(Any& rhs) noexcept;
		Any& swap(Any&& rhs) noexcept;
		Any& operator=(const Any& rhs) noexcept;
		Any& operator=(Any&& rhs) noexcept;
		Any& operator=(std::nullptr_t) noexcept;

		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(const ValueType& rhs) noexcept {
			auto newContainer = CreateContainer(rhs);
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};
		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(const ValueType* rhs) noexcept {
			auto newContainer = CreateContainer(rhs);
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};
		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(ValueType* rhs) noexcept {
			auto newContainer = CreateContainer(rhs);
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};
		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(ValueType&& rhs) noexcept {
			auto newContainer = CreateContainer(std::forward<ValueType>(rhs));
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};

	public:
		/*! Checks if the Any has been assigned something */
		bool IsEmpty() const noexcept;
		/*! Empties the Any and frees the memory. */
		void Clear() noexcept;

		template <typename ValueT> static const char* TypeNameOf() { return TypeOf<ValueT>().name(); };
		template <typename ValueT> static const Type_Info& TypeOf() { return user_type<ValueT>(); };

		std::string TypeName() const noexcept;
		// DynamicObjects can "present" as one class but actually be another. This checks the ACTUAL class, not the presenting class.
		std::weak_ptr<Type_Info> ActualType() const noexcept;
		std::weak_ptr<Type_Info> Type() const noexcept;
		std::shared_ptr<Type_Info> TypePtr() const noexcept;
		size_t TypeHash() const noexcept;
		bool IsTypeOf(std::weak_ptr<Type_Info> const& targetType) const noexcept;
		bool IsTypeOf(Type_Info const& targetType) const noexcept;
		template<typename VType> bool IsTypeOf() const noexcept {
			return IsTypeOf(user_type<typename std::decay_t<VType>>());
		};
		bool GetFlag(AnyData::Flag which) const;
		void SetFlag(AnyData::Flag which, bool newV);

#pragma region Boolean Operators
	public:
		explicit operator bool() const { auto locked{ std::shared_lock(mut) }; return (bool)container; };
		friend bool operator==(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container == b.container; };
		friend bool operator!=(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container != b.container; };
		friend bool operator<(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container < b.container; };
		friend bool operator<=(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container <= b.container; };
		friend bool operator>(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container > b.container; };
		friend bool operator>=(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container >= b.container; };
		friend bool operator==(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container == nullptr; };
		friend bool operator!=(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container != nullptr; };
		friend bool operator<(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container < nullptr; };
		friend bool operator<=(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container <= nullptr; };
		friend bool operator>(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container > nullptr; };
		friend bool operator>=(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container >= nullptr; };
		friend bool operator==(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr == a.container; };
		friend bool operator!=(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr != a.container; };
		friend bool operator<(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr < a.container; };
		friend bool operator<=(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr <= a.container; };
		friend bool operator>(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr > a.container; };
		friend bool operator>=(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr >= a.container; };
#pragma endregion

	public:
		class DataCaster {
		public:
			template<typename T> struct is_SharedPtr_class { typedef std::false_type type; };
			template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>> { typedef std::true_type type; };
			template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>&> { typedef std::true_type type; };
			template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>*> { typedef std::true_type type; };
			template<typename T> struct is_SharedPtr_class<const std::shared_ptr<T>> { typedef std::true_type type; };
			template<typename T> struct is_SharedPtr_class<const std::shared_ptr<T>&> { typedef std::true_type type; };
			template<typename T> struct is_SharedPtr_class<const std::shared_ptr<T>*> { typedef std::true_type type; };
			template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>&&> { typedef std::true_type type; };

		private:
			template <class VType> static decltype(auto) DoCast_Shared(Any* p) noexcept {
				auto locked{ std::shared_lock(p->mut) };
				if (p->container) {
					return std::shared_ptr<VType>(static_cast<VType*>(p->container->ptr()), [P = p->container](VType*) -> void {});
					// return p->container->cast_shared<VType>();
				}
				else {
					return std::shared_ptr<VType>{ nullptr };
				}
			};
			template <class VType> static decltype(auto) DoCast_Shared_Sentinel(Any* p) noexcept {
				throw("Casting Any to  std::shared_ptr<T>* or  std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
			};
			template<typename VType> static decltype(auto) DoCast_Unshared(Any* p) noexcept {
				constexpr bool is_ptr = std::is_pointer_v<VType>;

				auto locked{ std::shared_lock(p->mut) };
				if (p->container) {
					if constexpr (is_ptr) {
						return p->container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
					}
					else {
						return *p->container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
					}
				}
				else {
					if constexpr (is_ptr) {
						return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*)nullptr;
					}
					else {
						throw exception::bad_any_cast(p->Type(), user_type_shared<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>(), __LINE__);
					}
				}
			};

		public:
			template<typename T> static decltype(auto) DoCast(Any* p) noexcept {
				while (true) {
					static auto VarHash{ GetHash(user_type<Var>()) };
					typedef typename is_SharedPtr_class<T>::type isShared;
					constexpr bool is_shared_ptr = isShared::value;
					constexpr bool is_ptr = std::is_pointer_v<T>;
					constexpr bool is_ref = std::is_reference_v<T>;
					if constexpr (is_shared_ptr) {
						typedef typename details::get_type<T>::type innertype;
						if constexpr (is_ptr) {
							throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
						}
						else if constexpr (is_ref) {
							throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
						}
						else {
#ifdef AllowInlineVarTyping
							if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, Var>) {
								return DoCast_Shared<innertype>(p);
							}
							else {
								if (std::shared_ptr<AnyData>& m = p->container) {
									//if (m->GetTypeHash() == VarHash) {
									if (auto p2 = m->cast<Var>()) {
										p = &*p2->p_data;
										continue;
									}
									//}
								}
							}
#endif
							return DoCast_Shared<innertype>(p);
						}
					}
					else {
#ifdef AllowInlineVarTyping
						if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, Var>) {
							return DoCast_Unshared<T>(p);
						}
						else {
							if (std::shared_ptr<AnyData>& m = p->container) {
								//if (m->GetTypeHash() == VarHash) {
								if (auto p2 = m->cast<Var>()) {
									p = &*p2->p_data;
									continue;
								}
								//}
							}
						}
#endif
						return DoCast_Unshared<T>(p);
					}
				}
			};
		};

		template<typename VType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
		decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<Any*>(this)); };

		template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
		Any& cast() const noexcept { return *const_cast<Any*>(this); };

		template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
		Any* cast() const noexcept { return const_cast<Any*>(this); };

		details::AnyAutoCast cast() const noexcept;
		std::shared_ptr<AnyData> impl() const;

	public:
		mutable std::shared_ptr<AnyData> container;
		mutable GoodLang::fast_shared_mutex mut;
	};
	namespace Impl {
		__forceinline void ToString(Tag<Any>, Any const& r, std::string& out) {
			if (r.container)
				out = r.container->ToString();
			else 
				out = GoodLang::ToString(r.container);
		};
		__forceinline void GetChildren(Tag<Any>, Any const& r, std::vector< NodeCache >& out) {
			if (r.container) {
				out = r.container->GetChildren();
			}
		};
		__forceinline void TryDisconnectChild(Tag<Any>, Any const& r, bool& out) {
			const_cast<Any&>(r) = nullptr;
			out = true;
		};
	};

	namespace details {
		/*! Supports forward-declaring a "cast" from an Any to the desired destination type. e.g: int& ref_int = any_obj.cast(); ... std::string str = any_obj.cast(); */
		class AnyAutoCast {
		public:
			AnyAutoCast(const Any* _parent) : parent(const_cast<Any*>(_parent)) {};
			AnyAutoCast(AnyAutoCast&& other) : parent(std::move(other.parent)) {};
			AnyAutoCast() = delete;
			AnyAutoCast(const AnyAutoCast&) = delete;
			AnyAutoCast& operator=(const AnyAutoCast&) = delete;
			AnyAutoCast& operator=(AnyAutoCast&&) = delete;
			~AnyAutoCast() {};

			explicit operator Any& () const noexcept { return *parent; };
			explicit operator Any* () const noexcept { return parent; };
			template <typename T> operator std::shared_ptr<T>() const noexcept { return parent->cast<std::shared_ptr<T>>(); };
			template <typename T> operator std::shared_ptr<T>* () const noexcept { return parent->cast<std::shared_ptr<T>*>(); };
			template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
			operator ValueTypeT& () const noexcept { return parent->cast<ValueTypeT&>(); };
			template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
			operator ValueTypeT* () const noexcept { return parent->cast<ValueTypeT*>(); };
			
			Any* parent;
		};
	};

	/*! Casts to whatever is on the left-hand-side, with specializations for references, pointers, values, and std::shared_ptrs. References and pointers are lifetime-sensitive. */
	__forceinline details::AnyAutoCast Any::cast() const noexcept { return details::AnyAutoCast(this); };
	__forceinline std::shared_ptr<AnyData> Any::Object_Data::get(const details::AnyAutoCast& obj) {
		Any* t = const_cast<Any*>(obj.parent);
		if (t) {
			auto locked{ std::shared_lock(t->mut) };
			return t->container;
		}
		return std::shared_ptr<AnyData>{ nullptr };
	};
	__forceinline std::shared_ptr<AnyData> Any::Object_Data::get(const details::AnyAutoCast* t) { return get(*t); };
};

// GetCopyConstructor
namespace GoodLang {
	__forceinline std::function<Any(Any const&)>& Type_Info::GetCopyConstructor() const {
		static auto out{ std::function<Any(Any const&)>([](Any const& from) -> Any {
			return from;
		}) };
		return out;
	};
	__forceinline std::function<Any(Any const&)>& Scripted_Type_Info::GetCopyConstructor() const {
		static auto out{ std::function<Any(Any const&)>([](Any const& from) -> Any {
			auto& dynObj = from.cast<DynamicObject>();
			return DynamicObject(dynObj);
		}) };
		return out;
	};
	template <typename T> __forceinline std::function<Any(Any const&)>& BuiltIn_Type_Info<T>::GetCopyConstructor() const {
		static auto out{ std::function<Any(Any const&)>([](Any const& from) -> Any {
			if constexpr (std::is_same_v<T, void>) {
				return from;
			}
			else {
				if constexpr (std::is_copy_constructible_v<typename std::decay_t<T>>) {
					return typename std::decay_t<T>{ from.cast<T>() };
				}
				else {
					// we cannot copy construct -- we must simply pass through, since anything we do would have to be moved / copied to the Any regardless.
					return from;
				}
			}
		}) };
		return out;
	};
};

namespace GoodLang {
	template <typename T> static bool try_remove_recursions(T const& obj) {
		bool result{ false };
		while (true) {
			auto children = GoodLang::GetChildren(obj);
			std::deque<const GoodLang::Impl::NodeCache*> queue;
			bool doDisconnection = false;
			std::set<void*> data;
			if (Impl::find_recursion(children, queue)) {
				while (queue.size() > 0) {
					if (queue.back()->data) {
						if (data.find(queue.back()->data->ptr()) != data.end()) {
							doDisconnection = true;
						}
						else {
							data.emplace(queue.back()->data->ptr());
						}

						if (doDisconnection) {
							if (queue.back()->data->TryDisconnectChild()) {
								result = true;
								break;
							}
						}
					}
					queue.pop_back();
				}
				if (!doDisconnection) break; // something went wrong!
			}
			else break;
		}
		return result;
	};
};