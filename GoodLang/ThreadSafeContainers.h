#pragma once
#include "Foundation.h"
#include "Any"
#include "Proxy_Function.h"

#include <complex>
#include <array>
#include <chrono>
#include <thread>
#include <emmintrin.h> // _mm_pause()
#include <functional>
#include <cassert>
#include <ShlDisp.h> // InterlockedExchangePointer
#include <map>
#include <concurrent_queue.h>
#include <set>
#include <concurrent_unordered_set.h>
#include <stack>

#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace GoodLang {
	namespace utilities {
		namespace HasDefaultConstructor
		{
			template <class... T> struct Friend;
			struct testing_tag;

			// specialisation simply to check if default constructible
			template <class T> struct Friend<T, testing_tag> {
				// sfinae trick has to be nested in the Friend class
				// this candidate will be ignored if X does not have a default constructor
				template <class X, class = decltype(X())>
				static std::true_type Get(X*);

				template <class X>
				static std::false_type Get(...);

				static constexpr bool value = decltype(Get<T>(0))::value;
			};
			template <class T> using has_any_default_constructor = Friend<T, testing_tag>;
			template <class T> constexpr bool HasDefaultConstructor_v() {
				return has_any_default_constructor < T >::value;
			};
		}

		// ATOMIC
		#define MWCAS_CAPACITY 14
		#define MWCAS_RETRY_THRESHOLD 10
		#define MWCAS_SLEEP_TIME 10
		#define BZTREE_PAGE_SIZE 1024
		#define BZTREE_MAX_DELTA_RECORD_NUM 64
		#define BZTREE_MAX_DELETED_SPACE_SIZE (BZTREE_PAGE_SIZE / 8)
		#define BZTREE_MIN_FREE_SPACE_SIZE (BZTREE_PAGE_SIZE / 8)
		#define BZTREE_MIN_NODE_SIZE (BZTREE_PAGE_SIZE / 16)
		#define BZTREE_MAX_MERGED_SIZE (BZTREE_PAGE_SIZE / 2)
		#define BZTREE_MAX_VARIABLE_DATA_SIZE 128
		#define DBGROUP_MAX_THREAD_NUM 128
		#define CPP_UTILITY_SPINLOCK_RETRY_NUM 10
		#define CPP_UTILITY_BACKOFF_TIME 10
		#define BW_TREE_PAGE_SIZE 1024
		static __forceinline constexpr size_t LOG2(size_t n) { return ((n < 2) ? 1 : 1 + LOG2(n / 2)); };
		#define BW_TREE_DELTA_RECORD_NUM_THRESHOLD (2 * LOG2(BW_TREE_PAGE_SIZE / 256))
		#define BW_TREE_MAX_DELTA_RECORD_NUM 64
		#define BW_TREE_MIN_NODE_SIZE (BW_TREE_PAGE_SIZE / 16)
		#define BW_TREE_MAX_VARIABLE_DATA_SIZE 128
		#define BW_TREE_RETRY_THRESHOLD 10
		#define BW_TREE_SLEEP_TIME 10
		// utility
		namespace dbgroup::atomic::mwcas {
			/*######################################################################################
			 * Global enum and constants
			 *####################################################################################*/

			 /// The maximum number of retries for preventing busy loops.
			constexpr size_t kRetryNum = MWCAS_RETRY_THRESHOLD;

			/// A sleep time for preventing busy loops [us].
			static constexpr auto kShortSleep = std::chrono::microseconds{ MWCAS_SLEEP_TIME };

			/*######################################################################################
			 * Global utility functions
			 *####################################################################################*/

			 /**
			  * @tparam T a MwCAS target class.
			  * @retval true if a target class can be updated by MwCAS.
			  * @retval false otherwise.
			  */
			template <class T> constexpr auto CanMwCAS() -> bool {
				if constexpr (sizeof(uint64_t) == sizeof(T)) {
					return true;
				}
				else {
					if constexpr (std::is_same_v<T, uint64_t> || std::is_pointer_v<T>) {
						return true;
					}
					else {
						return false;
					}
				}
			};

		}  // namespace dbgroup::atomic::mwcas
		// common
		namespace dbgroup::atomic::mwcas::component
		{
			/*######################################################################################
			 * Global enum and constants
			 *####################################################################################*/

			 /// Assumes that the length of one word is 8 bytes
			constexpr size_t kWordSize = 8;

			/// Assumes that the size of one cache line is 64 bytes
			constexpr size_t kCacheLineSize = 64; // e.g. maximum of 8 words simultaneously? 

			/*######################################################################################
			 * Global utility structs
			 *####################################################################################*/

			 /**
			  * @brief An union to convert MwCAS target data into uint64_t.
			  *
			  * @tparam T a type of target data
			  */
			template <class T>
			union CASTargetConverter {
				const T target_data;
				const uint64_t converted_data;

				explicit constexpr CASTargetConverter(const uint64_t converted) : converted_data{ converted } {}

				explicit constexpr CASTargetConverter(const T target) : target_data{ target } {}
			};

			/**
			 * @brief Specialization for unsigned long type.
			 *
			 */
			template <>
			union CASTargetConverter<uint64_t> {
				const uint64_t target_data;
				const uint64_t converted_data;

				explicit constexpr CASTargetConverter(const uint64_t target) : target_data{ target } {}
			};

		}  // namespace dbgroup::atomic::mwcas::component
		// field 
		namespace dbgroup::atomic::mwcas::component
		{
			/**
			 * @brief A class to represent a MwCAS target field.
			 *
			 */
			class MwCASField
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an empty field for MwCAS.
				  *
				  */
				constexpr MwCASField() : target_bit_arr_{}, mwcas_flag_{ 0 } {}

				/**
				 * @brief Construct a MwCAS field with given data.
				 *
				 * @tparam T a target class to be embedded.
				 * @param target_data target data to be embedded.
				 * @param is_mwcas_descriptor a flag to indicate this field contains a descriptor.
				 */
				template <class T>
				explicit constexpr MwCASField(  //
					T target_data,
					bool is_mwcas_descriptor = false)
					: target_bit_arr_{ ConvertToUint64(target_data) }, mwcas_flag_{ is_mwcas_descriptor }
				{
					// static check to validate MwCAS targets
					static_assert(sizeof(T) == kWordSize);  // NOLINT
					static_assert(std::is_trivially_copyable_v<T>);
					static_assert(std::is_copy_constructible_v<T>);
					static_assert(std::is_move_constructible_v<T>);
					static_assert(std::is_copy_assignable_v<T>);
					static_assert(std::is_move_assignable_v<T>);
					static_assert(CanMwCAS<T>());
				}

				constexpr MwCASField(const MwCASField&) = default;
				constexpr MwCASField(MwCASField&&) = default;

				constexpr auto operator=(const MwCASField& obj)->MwCASField & = default;
				constexpr auto operator=(MwCASField&&)->MwCASField & = default;

				/*####################################################################################
				 * Public destructor
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MwCASField object.
				  *
				  */
				~MwCASField() = default;

				/*####################################################################################
				 * Public operators
				 *##################################################################################*/

				auto
					operator==(const MwCASField& obj) const  //
					-> bool
				{
					return memcmp(this, &obj, sizeof(MwCASField)) == 0;
				}

				auto
					operator!=(const MwCASField& obj) const  //
					-> bool
				{
					return memcmp(this, &obj, sizeof(MwCASField)) != 0;
				}

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @retval true if this field contains a descriptor.
				  * @retval false otherwise.
				  */
				[[nodiscard]] constexpr auto
					IsMwCASDescriptor() const  //
					-> bool
				{
					return mwcas_flag_;
				}

				/**
				 * @tparam T an expected class of data.
				 * @return data retained in this field.
				 */
				template <class T>
				[[nodiscard]] constexpr auto
					GetTargetData() const  //
					-> T
				{
					if constexpr (std::is_same_v<T, uint64_t>) {
						return target_bit_arr_;
					}
					else if constexpr (std::is_pointer_v<T>) {
						return reinterpret_cast<T>(target_bit_arr_);  // NOLINT
					}
					else {
						return CASTargetConverter<T>{target_bit_arr_}.target_data;  // NOLINT
					}
				}

			private:
				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @brief Conver given data into uint64_t.
				  *
				  * @tparam T a class of given data.
				  * @param data data to be converted.
				  * @return data converted to uint64_t.
				  */
				template <class T>
				constexpr auto
					ConvertToUint64(const T data)  //
					-> uint64_t
				{
					if constexpr (std::is_same_v<T, uint64_t>) {
						return data;
					}
					else if constexpr (std::is_pointer_v<T>) {
						return reinterpret_cast<uint64_t>(data);  // NOLINT
					}
					else {
						return CASTargetConverter<T>{data}.converted_data;  // NOLINT
					}
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// An actual target data
				uint64_t target_bit_arr_ : 63;

				/// Representing whether this field contains a MwCAS descriptor
				uint64_t mwcas_flag_ : 1;
			};

			// CAS target words must be one word
			static_assert(sizeof(MwCASField) == kWordSize);

		}  // namespace dbgroup::atomic::mwcas::component
		// target
		namespace dbgroup::atomic::mwcas::component
		{
			/**
			 * @brief A class to represent a MwCAS target.
			 *
			 */
			class MwCASTarget
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an empty MwCAS target.
				  *
				  */
				constexpr MwCASTarget() = default;

				/**
				 * @brief Construct a new MwCAS target based on given information.
				 *
				 * @tparam T a class of MwCAS targets.
				 * @param addr a target memory address.
				 * @param old_val an expected value of the target address.
				 * @param new_val an desired value of the target address.
				 */
				template <class T>
				constexpr MwCASTarget(  //
					void* addr,
					const T old_val,
					const T new_val,
					const std::memory_order fence)
					: addr_{ static_cast<std::atomic<MwCASField> *>(addr) },
					old_val_{ old_val },
					new_val_{ new_val },
					fence_{ fence }
				{
				}

				constexpr MwCASTarget(const MwCASTarget&) = default;
				constexpr MwCASTarget(MwCASTarget&&) = default;

				constexpr auto operator=(const MwCASTarget& obj)->MwCASTarget & = default;
				constexpr auto operator=(MwCASTarget&&)->MwCASTarget & = default;

				/*####################################################################################
				 * Public destructor
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MwCASTarget object.
				  *
				  */
				~MwCASTarget() = default;

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Embed a descriptor into this target address to linearlize MwCAS operations.
				  *
				  * @param desc_addr a memory address of a target descriptor.
				  * @retval true if the descriptor address is successfully embedded.
				  * @retval false otherwise.
				  */
				auto
					EmbedDescriptor(const MwCASField desc_addr)  //
					-> bool
				{
					for (size_t i = 1; true; ++i) {
						// try to embed a MwCAS decriptor
						auto expected = addr_->load(std::memory_order_relaxed);
						if (expected == old_val_
							&& addr_->compare_exchange_strong(expected, desc_addr, std::memory_order_relaxed)) {
							return true;
						}
						if (!expected.IsMwCASDescriptor() || i >= kRetryNum) return false;

						// retry if another desctiptor is embedded
					}
				}

				/**
				 * @brief Update a value of this target address.
				 *
				 */
				void
					RedoMwCAS()
				{
					addr_->store(new_val_, fence_);
				}

				/**
				 * @brief Revert a value of this target address.
				 *
				 */
				void
					UndoMwCAS()
				{
					addr_->store(old_val_, std::memory_order_relaxed);
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A target memory address
				std::atomic<MwCASField>* addr_{};

				/// An expected value of a target field
				MwCASField old_val_{};

				/// An inserting value into a target field
				MwCASField new_val_{};

				/// A fence to be inserted when embedding a new value.
				std::memory_order fence_{ std::memory_order_seq_cst };
			};

		}  // namespace dbgroup::atomic::mwcas::component
		// descriptor
		namespace dbgroup::atomic::mwcas
		{
			/**
			 * @brief A class to manage a MwCAS (multi-words compare-and-swap) operation.
			 *
			 */
			template <int kMwCASCapacity>
			class alignas(component::kCacheLineSize) MwCASDescriptor
			{
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using MwCASTarget = component::MwCASTarget;
				using MwCASField = component::MwCASField;

			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an empty descriptor for MwCAS operations.
				  *
				  */
				constexpr MwCASDescriptor() = default;

				constexpr MwCASDescriptor(const MwCASDescriptor&) = default;
				constexpr MwCASDescriptor(MwCASDescriptor&&) = default;

				constexpr auto operator=(const MwCASDescriptor& obj)->MwCASDescriptor & = default;
				constexpr auto operator=(MwCASDescriptor&&)->MwCASDescriptor & = default;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MwCASDescriptor object.
				  *
				  */
				~MwCASDescriptor() = default;

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @return the number of registered MwCAS targets
				  */
				[[nodiscard]] constexpr auto
					Size() const  //
					-> size_t
				{
					return target_count_;
				}

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Read a value from a given memory address.
				  * \e NOTE: if a memory address is included in MwCAS target fields, it must be read via this function.
				  *
				  * @tparam T an expected class of a target field
				  * @param addr a target memory address to read
				  * @param fence a flag for controling std::memory_order.
				  * @return a read value
				  */
				template <class T>
				static auto
					Read(  //
						const void* addr,
						const std::memory_order fence = std::memory_order_seq_cst)  //
					-> T
				{
					const auto* target_addr = static_cast<const std::atomic<MwCASField> *>(addr);
					size_t i;
					MwCASField target_word{};
					while (true) {
						for (i = 0; i < kRetryNum; ++i) {
							target_word = target_addr->load(fence);
							if (!target_word.IsMwCASDescriptor()) return target_word.GetTargetData<T>();
						}
						std::this_thread::sleep_for(kShortSleep); // wait to prevent busy loop
					}
				}

				/**
				 * @brief Add a new MwCAS target to this descriptor.
				 *
				 * @tparam T a class of a target
				 * @param addr a target memory address
				 * @param old_val an expected value of a target field
				 * @param new_val an inserting value into a target field
				 * @param fence a flag for controling std::memory_order.
				 */
				template <class T>
				constexpr void
					AddMwCASTarget(  //
						void* addr,
						const T old_val,
						const T new_val,
						const std::memory_order fence = std::memory_order_seq_cst)
				{

					assert(target_count_ < kMwCASCapacity);

					targets_[target_count_++] = MwCASTarget{ addr, old_val, new_val, fence };
				}

				/**
				 * @brief Perform a MwCAS operation by using registered targets.
				 *
				 * @retval true if a MwCAS operation succeeds
				 * @retval false if a MwCAS operation fails
				 */
				auto
					MwCAS()  //
					-> bool
				{
					const MwCASField desc_addr{ this, true };

					// serialize MwCAS operations by embedding a descriptor
					auto mwcas_success = true;
					size_t embedded_count = 0;
					for (size_t i = 0; i < target_count_; ++i, ++embedded_count) {
						if (!targets_[i].EmbedDescriptor(desc_addr)) {
							// if a target field has been already updated, MwCAS fails
							mwcas_success = false;
							break;
						}
					}

					// complete MwCAS
					if (mwcas_success) {
						for (size_t i = 0; i < embedded_count; ++i) {
							targets_[i].RedoMwCAS();
						}
					}
					else {
						for (size_t i = 0; i < embedded_count; ++i) {
							targets_[i].UndoMwCAS();
						}
					}

					return mwcas_success;
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// Target entries of MwCAS
				MwCASTarget targets_[kMwCASCapacity];

				/// The number of registered MwCAS targets
				size_t target_count_{ 0 };
			};

		}  // namespace dbgroup::atomic::mwcas

	};

	using number = boost::multiprecision::number<boost::multiprecision::cpp_dec_float< std::numeric_limits<double>::digits10 * 4 > >; // extremely high precision number for scientific experiments or analysis

	/* Implimentation of std::tuple, with built-in get<n>() functions. It is only as thread-safe as the inner types */
	template<typename... Args> class Union {
	private:
#pragma region IMPLIMENTATION DETAILS
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

#pragma endregion
	public:
#pragma region INTENDED PUBLIC FUCNTIONS AND USES
		/* ALLOC THE UNIONED DATA IN STACK */
		Union() noexcept : data{ 0 } { Alloc(&data[0]); };
		/* ALLOC THE UNIONED DATA FROM PARAMETERS */ template <typename T, typename = std::enable_if_t<!std::is_same_v<typename std::remove_reference<T>::type, Union>>, typename... TArgs>
		explicit Union(T&& b, TArgs&&... a) noexcept : data{ 0 } {
			InitAndSet(std::forward<T>(b), std::forward<TArgs>(a)...);
		};

		/* INIT AND COPY THE UNIONED DATA FROM ANOTHER UNION */
		Union(Union const& a) noexcept : data{ 0 } { InitAndCopy(a); };
		/* INIT AND TAKE THE UNIONED DATA FROM ANOTHER UNION */
		Union(Union&& a) noexcept : data{ 0 } { InitAndTake(std::forward<Union>(a)); };
		/* COPY THE UNIONED DATA FROM ANOTHER UNION */
		Union& operator=(Union const& a) { Copy(a); return *this; };
		/* TAKE THE UNIONED DATA FROM ANOTHER UNION */
		Union& operator=(Union&& a) { Take(std::forward<Union>(a)); return *this; };
		/* DESTROY THE UNION ONCE OUT-OF-SCOPE */
		~Union() { Delete(); };

		/* GET A REFERENCE TO THE N'th ITEM */  template<int N>
		constexpr NthTypeOf<N>& get() const noexcept {
			static_assert(N < num_parameters&& N >= 0, "Cannot access parameters beyond the allocated buffer.");
			constexpr size_t index = SizeOfFirstN<N>();
			return *static_cast<NthTypeOf<N>*>(static_cast<void*>(&data[index]));

			//constexpr NthTypeOf<N>* out = static_cast<NthTypeOf<N>*>(static_cast<void*>((static_cast<byte*>(&const_cast<byte&>(data[SizeOfFirstN<N>()])))));
			//return *out;
		};
		/* GET THE SIZE (in bytes) OF THE ENTIRE UNION */
		static constexpr size_t size() noexcept { return SizeOfAll(); };
		/* GET THE SIZE (in bytes) OF THE N'th ITEM */ template<int N>
		static constexpr size_t size() noexcept { return sizeof(NthTypeOf<N>); };

		/* COMPARE WITH ANOTHER UNION */
		friend bool operator==(Union const& a, Union const& b) {
			return Equals(a, b);
		};
		friend bool operator!=(Union const& a, Union const& b) {
			return !operator==(a, b);
		};
#pragma endregion
	private:
#pragma region DATA ARRAY (BYTES)
		mutable byte data[sizeOfArgs];
#pragma endregion
	private:
#pragma region STATIC UTILITY FUNCTIONS
		template<typename _type_> static bool is_empty(_type_* d, size_t size) { byte* buf = static_cast<byte*>(static_cast<void*>(d)); return buf[0] == 0 && 0 == ::memcmp(buf, buf + 1, size - 1); };
		template<typename _type_> static constexpr bool isPod() { return std::is_pod<_type_>::value; };
		template<typename _type_> static void InstantiateData(_type_* ptr) {
			if constexpr (isPod<_type_>()) {
				/* already cleared during instantiation */
			}
			else {
				if constexpr (utilities::HasDefaultConstructor::HasDefaultConstructor_v<_type_>()) {
					new (&ptr[0]) _type_;
				}
			}
		};
		template<typename _type_> static void InstantiateData(_type_* ptr, _type_&& srce) {
			if constexpr (isPod<_type_>()) {
				*ptr = std::forward<_type_>(srce);
			}
			else {
				if constexpr (std::is_move_constructible<_type_>::value) {
					new (&ptr[0]) _type_(std::forward<_type_>(srce));
				}
				else if constexpr (std::is_copy_constructible<_type_>::value) {
					new (&ptr[0]) _type_(srce);
				}
			}
		};
		template<typename _type_> static void InstantiateData(_type_* ptr, _type_ const& srce) {
			if constexpr (isPod<_type_>()) {
				*ptr = srce;
			}
			else {
				if constexpr (std::is_copy_constructible<_type_>::value) {
					new (&ptr[0]) _type_(srce);
				}
			}
		};
		template<typename _type_, typename _type2_, typename = std::enable_if_t<!std::is_same_v<_type2_, _type_>>>
		static void InstantiateData(_type_* ptr, _type2_&& srce) {
			if constexpr (isPod<_type_>()) {
				*ptr = static_cast<_type_>(srce);
			}
			else {
				new (&ptr[0]) _type_(std::forward<_type2_>(srce));
			}
		};
		template<typename _type_, typename _type2_, typename = std::enable_if_t<!std::is_same_v<_type2_, _type_>>>
		static void InstantiateData(_type_* ptr, _type2_ const& srce) {
			if constexpr (isPod<_type_>()) {
				*ptr = static_cast<_type_>(srce);
			}
			else {
				new (&ptr[0]) _type_(srce);
			}
		};
		template<typename _type_> static void DestroyData(_type_* ptr) {
			if constexpr (isPod<_type_>()) { /* does not require clearing */ }
			else {
				if (!is_empty(ptr, sizeof(_type_))) {
					ptr[0].~_type_();
				}
			}
		};
		template <int N> static NthTypeOf<N>* PtrAt(byte* data) { return static_cast<NthTypeOf<N>*>(static_cast<void*>(&data[SizeOfFirstN<N>()])); };
		static void Alloc(byte* data) {
			if constexpr (num_parameters >= 1) { InstantiateData(PtrAt<0>(data)); }
			if constexpr (num_parameters >= 2) { InstantiateData(PtrAt<1>(data)); }
			if constexpr (num_parameters >= 3) { InstantiateData(PtrAt<2>(data)); }
			if constexpr (num_parameters >= 4) { InstantiateData(PtrAt<3>(data)); }
			if constexpr (num_parameters >= 5) { InstantiateData(PtrAt<4>(data)); }
			if constexpr (num_parameters >= 6) { InstantiateData(PtrAt<5>(data)); }
			if constexpr (num_parameters >= 7) { InstantiateData(PtrAt<6>(data)); }
			if constexpr (num_parameters >= 8) { InstantiateData(PtrAt<7>(data)); }
			if constexpr (num_parameters >= 9) { InstantiateData(PtrAt<8>(data)); }
			if constexpr (num_parameters >= 10) { InstantiateData(PtrAt<9>(data)); }
			if constexpr (num_parameters >= 11) { InstantiateData(PtrAt<10>(data)); }
			if constexpr (num_parameters >= 12) { InstantiateData(PtrAt<11>(data)); }
			if constexpr (num_parameters >= 13) { InstantiateData(PtrAt<12>(data)); }
			if constexpr (num_parameters >= 14) { InstantiateData(PtrAt<13>(data)); }
			if constexpr (num_parameters >= 15) { InstantiateData(PtrAt<14>(data)); }
			if constexpr (num_parameters >= 16) { InstantiateData(PtrAt<15>(data)); }
		};
#pragma endregion
	private:
#pragma region UTILITY FUNCTIONS
		template <int N> bool ElementIsZero() { return is_empty(PtrAt<N>(&data[0]), sizeof(NthTypeOf<N>)); };
		bool IsAllZero() { return is_empty(&data[0], sizeOfArgs); };
		void Clear() { ::memset(&data[0], 0, sizeOfArgs); };
		template <int N> void Clear() { ::memset(&get<N>(), 0, sizeof(NthTypeOf<N>)); };
		void Delete() {
			if (IsAllZero()) return; // sign that this item was not initialized, is all POD, or was recently "taken" over
			if constexpr (num_parameters >= 1) { DestroyData(PtrAt<0>(data)); }
			if constexpr (num_parameters >= 2) { DestroyData(PtrAt<1>(data)); }
			if constexpr (num_parameters >= 3) { DestroyData(PtrAt<2>(data)); }
			if constexpr (num_parameters >= 4) { DestroyData(PtrAt<3>(data)); }
			if constexpr (num_parameters >= 5) { DestroyData(PtrAt<4>(data)); }
			if constexpr (num_parameters >= 6) { DestroyData(PtrAt<5>(data)); }
			if constexpr (num_parameters >= 7) { DestroyData(PtrAt<6>(data)); }
			if constexpr (num_parameters >= 8) { DestroyData(PtrAt<7>(data)); }
			if constexpr (num_parameters >= 9) { DestroyData(PtrAt<8>(data)); }
			if constexpr (num_parameters >= 10) { DestroyData(PtrAt<9>(data)); }
			if constexpr (num_parameters >= 11) { DestroyData(PtrAt<10>(data)); }
			if constexpr (num_parameters >= 12) { DestroyData(PtrAt<11>(data)); }
			if constexpr (num_parameters >= 13) { DestroyData(PtrAt<12>(data)); }
			if constexpr (num_parameters >= 14) { DestroyData(PtrAt<13>(data)); }
			if constexpr (num_parameters >= 15) { DestroyData(PtrAt<14>(data)); }
			if constexpr (num_parameters >= 16) { DestroyData(PtrAt<15>(data)); }
			Clear();
		};

#pragma endregion
	private:
#pragma region SET WITH PARAMETER ARGS
		/* INIT DATA USING PARAMETER */ template <int N> void InitAndSetAt(NthTypeOf<N> const& a) {
			InstantiateData(PtrAt<N>(&data[0]), a);
		};
		/* INIT DATA USING PARAMETER */ template <int N> void InitAndSetAt(NthTypeOf<N>&& a) {
			InstantiateData(PtrAt<N>(&data[0]), std::forward<NthTypeOf<N>>(a));
		};
		/* INIT DATA USING PARAMETER */ template <int N, typename T, typename = std::enable_if_t<!std::is_same_v<T, NthTypeOf<N>>>> void InitAndSetAt(T&& a) {
			InstantiateData(PtrAt<N>(&data[0]), std::forward<T>(a));
		};
		/* EMPTY PARAMETER PACK -> END RECURSION */ void InitAndSetDataWith() { return; };
		/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void InitAndSetDataWith(const T& value, Targs&&... Fargs) {
			InitAndSetAt<num_parameters - (1 + sizeof...(Fargs))>(value);
			InitAndSetDataWith(std::forward<Targs>(Fargs)...);
		};
		/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void InitAndSetDataWith(T&& value, Targs&&... Fargs) {
			InitAndSetAt<num_parameters - (1 + sizeof...(Fargs))>(std::forward<T>(value));
			InitAndSetDataWith(std::forward<Targs>(Fargs)...);
		};
		/* SET WITH PARAMETER PACK */ template <typename... TArgs> void InitAndSet(TArgs const&... a) {
			static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
			InitAndSetDataWith(a...);
		};
		/* SET WITH PARAMETER PACK */ template <typename... TArgs> void InitAndSet(TArgs&&... a) {
			static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
			InitAndSetDataWith(std::forward<TArgs>(a)...);
		};

		/* SET DATA USING PARAMETER */ template <int N> void SetAt(NthTypeOf<N> const& a) { this->get<N>() = a; };
		/* SET DATA USING PARAMETER */ template <int N> void SetAt(NthTypeOf<N>&& a) { this->get<N>() = std::forward<NthTypeOf<N>>(a); };
		/* EMPTY PARAMETER PACK -> END RECURSION */ void SetDataWith() { return; };
		/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void SetDataWith(const T& value, Targs&&... Fargs) { SetAt<num_parameters - (1 + sizeof...(Fargs))>(value); SetDataWith(std::forward<Targs>(Fargs)...); };
		/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void SetDataWith(T&& value, Targs&&... Fargs) { SetAt<num_parameters - (1 + sizeof...(Fargs))>(std::forward<T>(value)); SetDataWith(std::forward<Targs>(Fargs)...); };
		/* SET WITH PARAMETER PACK */ template <typename... TArgs> void Set(TArgs const&... a) {
			static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
			SetDataWith(a...);
		};
		/* SET WITH PARAMETER PACK */ template <typename... TArgs> void Set(TArgs&&... a) {
			static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
			SetDataWith(std::forward<TArgs>(a)...);
		};
#pragma endregion
#pragma region COMPARE WITH CONST&
		template <int N> static bool EqualsAt(Union const& a, Union const& b) {
			return a.get<N>() == b.get<N>();
		};
		static bool Equals(Union const& a, Union const& b) {
			bool out = true;
			if constexpr (num_parameters >= 1) { out = out && EqualsAt<0>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 2) { out = out && EqualsAt<1>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 3) { out = out && EqualsAt<2>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 4) { out = out && EqualsAt<3>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 5) { out = out && EqualsAt<4>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 6) { out = out && EqualsAt<5>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 7) { out = out && EqualsAt<6>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 8) { out = out && EqualsAt<7>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 9) { out = out && EqualsAt<8>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 10) { out = out && EqualsAt<9>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 11) { out = out && EqualsAt<10>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 12) { out = out && EqualsAt<11>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 13) { out = out && EqualsAt<12>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 14) { out = out && EqualsAt<13>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 15) { out = out && EqualsAt<14>(a, b); if (!out) return out; }
			if constexpr (num_parameters >= 16) { out = out && EqualsAt<15>(a, b); if (!out) return out; }
			return out;
		};
#pragma endregion
#pragma region COPY FROM CONST&
		template <int N> void InitAndCopyAt(Union const& a) { InstantiateData(PtrAt<N>(&data[0]), a.get<N>()); };
		void InitAndCopy(Union const& a) {
			if constexpr (num_parameters >= 1) { InitAndCopyAt<0>(a); }
			if constexpr (num_parameters >= 2) { InitAndCopyAt<1>(a); }
			if constexpr (num_parameters >= 3) { InitAndCopyAt<2>(a); }
			if constexpr (num_parameters >= 4) { InitAndCopyAt<3>(a); }
			if constexpr (num_parameters >= 5) { InitAndCopyAt<4>(a); }
			if constexpr (num_parameters >= 6) { InitAndCopyAt<5>(a); }
			if constexpr (num_parameters >= 7) { InitAndCopyAt<6>(a); }
			if constexpr (num_parameters >= 8) { InitAndCopyAt<7>(a); }
			if constexpr (num_parameters >= 9) { InitAndCopyAt<8>(a); }
			if constexpr (num_parameters >= 10) { InitAndCopyAt<9>(a); }
			if constexpr (num_parameters >= 11) { InitAndCopyAt<10>(a); }
			if constexpr (num_parameters >= 12) { InitAndCopyAt<11>(a); }
			if constexpr (num_parameters >= 13) { InitAndCopyAt<12>(a); }
			if constexpr (num_parameters >= 14) { InitAndCopyAt<13>(a); }
			if constexpr (num_parameters >= 15) { InitAndCopyAt<14>(a); }
			if constexpr (num_parameters >= 16) { InitAndCopyAt<15>(a); }
		};

		template <int N> void CopyAt(Union const& a) { this->get<N>() = a.get<N>(); };
		void Copy(Union const& a) {
			if constexpr (num_parameters >= 1) { CopyAt<0>(a); }
			if constexpr (num_parameters >= 2) { CopyAt<1>(a); }
			if constexpr (num_parameters >= 3) { CopyAt<2>(a); }
			if constexpr (num_parameters >= 4) { CopyAt<3>(a); }
			if constexpr (num_parameters >= 5) { CopyAt<4>(a); }
			if constexpr (num_parameters >= 6) { CopyAt<5>(a); }
			if constexpr (num_parameters >= 7) { CopyAt<6>(a); }
			if constexpr (num_parameters >= 8) { CopyAt<7>(a); }
			if constexpr (num_parameters >= 9) { CopyAt<8>(a); }
			if constexpr (num_parameters >= 10) { CopyAt<9>(a); }
			if constexpr (num_parameters >= 11) { CopyAt<10>(a); }
			if constexpr (num_parameters >= 12) { CopyAt<11>(a); }
			if constexpr (num_parameters >= 13) { CopyAt<12>(a); }
			if constexpr (num_parameters >= 14) { CopyAt<13>(a); }
			if constexpr (num_parameters >= 15) { CopyAt<14>(a); }
			if constexpr (num_parameters >= 16) { CopyAt<15>(a); }
		};
#pragma endregion
#pragma region TAKE FROM &&
		template <int N> void InitAndTakeAt(Union&& a) {
			InstantiateData(PtrAt<N>(&data[0]), std::move(a.get<N>()));
			if constexpr (!isPod<NthTypeOf<N>>()) { a.Clear<N>(); }
		};
		void InitAndTake(Union&& a) {
			if constexpr (num_parameters >= 1) { InitAndTakeAt<0>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 2) { InitAndTakeAt<1>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 3) { InitAndTakeAt<2>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 4) { InitAndTakeAt<3>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 5) { InitAndTakeAt<4>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 6) { InitAndTakeAt<5>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 7) { InitAndTakeAt<6>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 8) { InitAndTakeAt<7>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 9) { InitAndTakeAt<8>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 10) { InitAndTakeAt<9>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 11) { InitAndTakeAt<10>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 12) { InitAndTakeAt<11>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 13) { InitAndTakeAt<12>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 14) { InitAndTakeAt<13>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 15) { InitAndTakeAt<14>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 16) { InitAndTakeAt<15>(std::forward<Union>(a)); }
			a.Clear();
		};

		template <int N> void TakeAt(Union&& a) {
			get<N>() = std::move(a.get<N>());
			if constexpr (!isPod<NthTypeOf<N>>()) { a.Clear<N>(); }
		};
		void Take(Union&& a) {
			if constexpr (num_parameters >= 1) { TakeAt<0>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 2) { TakeAt<1>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 3) { TakeAt<2>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 4) { TakeAt<3>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 5) { TakeAt<4>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 6) { TakeAt<5>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 7) { TakeAt<6>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 8) { TakeAt<7>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 9) { TakeAt<8>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 10) { TakeAt<9>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 11) { TakeAt<10>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 12) { TakeAt<11>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 13) { TakeAt<12>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 14) { TakeAt<13>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 15) { TakeAt<14>(std::forward<Union>(a)); }
			if constexpr (num_parameters >= 16) { TakeAt<15>(std::forward<Union>(a)); }
			a.Clear();
		};
#pragma endregion
	};

	/* *THREAD SAFE* Thread-safe and fiber-safe wrapper for atomic operations on pointers, without having to utilize std::atomic<T*> */
	template< typename T>
	struct atomic_ptr {
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

	/* thread-safe and fiber-safe integer (atomic swapping of integers) */
	class InterlockedLong {
	public:
		constexpr InterlockedLong() noexcept : value(0) {};
		constexpr InterlockedLong(long a) noexcept : value(a) {};
		InterlockedLong(const InterlockedLong& other) : value(other.GetValue()) {};
		InterlockedLong& operator=(const InterlockedLong& other) { SetValue(other.GetValue()); return *this; };
		InterlockedLong& operator=(long newSource) { SetValue(newSource); return *this; };

		explicit operator long() { return GetValue(); };
		explicit operator long() const { return GetValue(); };

		explicit operator bool() { if (GetValue() == 0) return false; else return true; };
		explicit operator bool() const { if (GetValue() == 0) return false; else return true; };

		friend InterlockedLong operator+(long i, const InterlockedLong& b) { InterlockedLong out(b); out.Add(i); return out; };
		friend InterlockedLong operator+(const InterlockedLong& b, long i) { InterlockedLong out(b); out.Add(i); return out; };
		friend InterlockedLong operator-(long i, const InterlockedLong& b) { InterlockedLong out(i); out.Add(-b.GetValue()); return out; };
		friend InterlockedLong operator-(const InterlockedLong& b, long i) { InterlockedLong out(b); out.Add(-i); return out; };
		friend InterlockedLong operator/(long i, const InterlockedLong& b) { return i / b.GetValue(); };
		friend InterlockedLong operator/(const InterlockedLong& b, long i) { return b.GetValue() / i; };

		friend bool operator<=(long i, const InterlockedLong& b) { return i <= b.GetValue(); };
		friend bool operator<=(const InterlockedLong& b, long i) { return i > b.GetValue(); };
		friend bool operator>=(long i, const InterlockedLong& b) { return i >= b.GetValue(); };
		friend bool operator>=(const InterlockedLong& b, long i) { return i < b.GetValue(); };
		friend bool operator>(long i, const InterlockedLong& b) { return i > b.GetValue(); };
		friend bool operator>(const InterlockedLong& b, long i) { return i <= b.GetValue(); };
		friend bool operator<(long i, const InterlockedLong& b) { return i < b.GetValue(); };
		friend bool operator<(const InterlockedLong& b, long i) { return i >= b.GetValue(); };

		friend bool operator<=(const InterlockedLong& i, const InterlockedLong& b) { return i.GetValue() <= b.GetValue(); };
		friend bool operator>=(const InterlockedLong& i, const InterlockedLong& b) { return i.GetValue() >= b.GetValue(); };
		friend bool operator>(const InterlockedLong& i, const InterlockedLong& b) { return i.GetValue() > b.GetValue(); };
		friend bool operator<(const InterlockedLong& i, const InterlockedLong& b) { return i.GetValue() < b.GetValue(); };

		InterlockedLong operator++(int) { InterlockedLong out{ *this }; Increment(); return out; };
		InterlockedLong& operator++() { Increment(); return *this; };

		InterlockedLong operator--(int) { InterlockedLong out{ *this }; Decrement(); return out; };
		InterlockedLong& operator--() { Decrement(); return *this; };

		InterlockedLong& operator+=(long i) {
			if (i == 1) {
				Increment();
			}
			else if (i == -1) {
				Decrement();
			}
			else {
				Add(i);
			}
			return *this;
		};
		InterlockedLong& operator-=(long i) {
			if (i == 1) {
				Decrement();
			}
			else if (i == -1) {
				Increment();
			}
			else {
				Add(-i);
			}
			return *this;
		};

		InterlockedLong& operator+=(const InterlockedLong& i) { return operator+=(i.GetValue()); };
		InterlockedLong& operator-=(const InterlockedLong& i) { return operator-=(i.GetValue()); };

		friend bool operator==(const InterlockedLong& i, const InterlockedLong& j) { return i.GetValue() == j.GetValue(); };
		friend bool operator!=(const InterlockedLong& i, const InterlockedLong& j) { return i.GetValue() != j.GetValue(); };

		long Increment(); // atomically increments the integer and returns the new value
		long Decrement(); // atomically decrements the integer and returns the new value
		long Add(long v); // atomically adds a value to the integer and returns the new value
		long Sub(long v); // atomically subtracts a value from the integer and returns the new value
		long GetValue() const; // returns the current value of the integer
		void SetValue(long v);
		bool SetValueIfEqual(long desired, long compare);
		bool TryIncrementTo(long n);
		void lock();
		void unlock();

		long load() const;
		bool CompareExchange(long oldVersion, long RecordVersion);



	private:
		long	value;

	};

	/* allows any copiable object to be thread-safe by wrapping it in a locking container. */
	template <typename Arg> class Lockable {
	public:
		class SharedObj {
		public:
			Arg& obj;
			std::scoped_lock< decltype(Lockable::lock) > locked;
			SharedObj(Arg& object, decltype(Lockable::lock)& lock) : obj{ object }, locked{ lock } {};
			Arg* operator->() { return &obj; };
			Arg& operator*() { return obj; };
		};
		class SharedConstObj {
		public:
			const Arg& obj;
			std::scoped_lock< decltype(Lockable::lock) > locked;
			SharedConstObj(const Arg& object, decltype(Lockable::lock)& lock) : obj{ object }, locked{ lock } {};
			const Arg* operator->() { return &obj; };
			const Arg& operator*() { return obj; };
		};

	protected:
		Arg data;
		// mutable GoodLang::InterlockedLong lock{ 0 };
		mutable std::mutex lock{}; // GoodLang::

	public:
		[[nodiscard]] SharedConstObj Read() const { return SharedConstObj(data, lock); };
		[[nodiscard]] SharedObj Read() { return SharedObj(data, lock); };

	public:
		Lockable Copy() const {
			return Lockable(load());
		};

	public:
		operator Arg() const {
			return load();
		};

	public:
		Lockable() : data{ Arg{} } {};
		Lockable(Arg const& a) : data{ a } {};
		Lockable(const Lockable& r) : data{ *r.Read() } {};
		Lockable& operator=(const Lockable& r) {
			*Read() = *r.Read();
		};
		Lockable(Lockable&& r) : data{ *r.Read() } {};
		Lockable& operator=(Lockable&& r) {
			*Read() = *r.Read();
		};
		~Lockable() = default;

		template <typename T> bool operator==(T b) const {
			return *Read() == b;
		};
		template <typename T> bool operator!=(T b) const {
			return !operator==(b);
		};

	public:
		bool CompareSwap(Arg const& compare, Arg const& input) {
			auto x = Read();
			if (*x == compare) {
				*x = input;
				return true;
			}
			else {
				return false;
			}
		}; // returns the previous value while changing the underlying value
		Arg Swap(Arg const& input) {
			auto x = Read();
			Arg out = *x;
			*x = input;
			return out;
		}; // returns the previous value while changing the underlying value
		template <typename F> // std::function<Arg(Arg)>
		Arg Update(F const& updateFunction) {
			auto x = Read();
			Arg out;
			*x = updateFunction(out = *x);
			return out;
		}; // returns the previous value while incrementing the actual counter

	public: // std::atomic compatability
		Arg exchange(Arg const& v) {
			return Swap(v);
		}; // returns the previous value while setting the value to the input
		Arg load() const {
			return *Read();
		}; // gets the value
		void store(Arg const& v) {
			Swap(v);
			return;
		}; // sets the value to the input
	};

	/* allows any copiable object to be thread-safe by wrapping it in a locking container. */
	template <typename Arg> class SharedLockable {
	public:
		class SharedObj {
			typedef std::shared_lock< decltype(SharedLockable::lock) > lockType;
			Arg& obj;
			std::shared_ptr<lockType> locked;

		public:
			SharedObj(Arg& object, decltype(SharedLockable::lock)& lock) : obj{ object }, locked{ std::make_shared<lockType>(lock) } {};
			SharedObj(Arg& object, decltype(locked) lock) : obj{ object }, locked{ std::move(lock) } {};
			Arg* operator->() { return &obj; };
			Arg& operator*() { return obj; };
			std::shared_ptr<lockType> ForwardLock() { return locked; };
			operator bool() const { return (bool)locked; };
		};
		class SharedConstObj {
			typedef std::shared_lock< decltype(SharedLockable::lock) > lockType;
			const Arg& obj;
			std::shared_ptr<lockType> locked;

		public:
			SharedConstObj(const Arg& object, decltype(SharedLockable::lock)& lock) : obj{ object }, locked{ std::make_shared<lockType>(lock) } {};
			SharedConstObj(const Arg& object, decltype(locked) lock) : obj{ object }, locked{ std::move(lock) } {};
			const Arg* operator->() { return &obj; };
			const Arg& operator*() { return obj; };
			std::shared_ptr<lockType> ForwardLock() { return locked; };
			operator bool() const { return (bool)locked; };
		};
		class Obj {
			typedef std::unique_lock< decltype(SharedLockable::lock) > lockType;
			Arg& obj;
			std::shared_ptr<lockType> locked;

		public:
			Obj(Arg& object, decltype(SharedLockable::lock)& lock) : obj{ object }, locked{ std::make_shared<lockType>(lock) } {};
			Obj(Arg& object, decltype(locked) lock) : obj{ object }, locked{ std::move(lock) } {};
			Arg* operator->() { return &obj; };
			Arg& operator*() { return obj; };
			std::shared_ptr<lockType> ForwardLock() { return locked; };
			operator bool() const { return (bool)locked; };
		};
		class ConstObj {
			typedef std::unique_lock< decltype(SharedLockable::lock) > lockType;
			const Arg& obj;
			std::shared_ptr<lockType> locked;

		public:
			ConstObj(const Arg& object, decltype(SharedLockable::lock)& lock) : obj{ object }, locked{ std::make_shared<lockType>(lock) } {};
			ConstObj(const Arg& object, decltype(locked) lock) : obj{ object }, locked{ std::move(lock) } {};
			const Arg* operator->() { return &obj; };
			const Arg& operator*() { return obj; };
			std::shared_ptr<lockType> ForwardLock() { return locked; };
			operator bool() const { return (bool)locked; };
		};

	protected:
		std::unique_ptr<Arg> data;
		mutable std::shared_mutex lock{};

	public:
		auto& GetLock() const { return lock; };

		[[nodiscard]] ConstObj Unique() const { if (!data) throw std::runtime_error("Bad call to Unique()"); return ConstObj(*data.get(), lock); };
		[[nodiscard]] Obj Unique() { if (!data) throw std::runtime_error("Bad call to Unique()"); return Obj(*data.get(), lock); };
		[[nodiscard]] SharedConstObj Shared() const { if (!data) throw std::runtime_error("Bad call to Shared()"); return SharedConstObj(*data.get(), lock); };
		[[nodiscard]] SharedObj Shared() { if (!data) throw std::runtime_error("Bad call to Shared()"); return SharedObj(*data.get(), lock); };

	public:
		SharedLockable Copy() const {
			return SharedLockable(load());
		};

	public:
		operator Arg() const {
			return load();
		};

	private:
		static std::unique_ptr<Arg> TryInstance() {
			if constexpr (std::is_constructible_v<Arg>) {
				return std::make_unique<Arg>();
			}
			else {
				return nullptr;
			}
		};
		static std::unique_ptr<Arg> TryInstance(Arg const& a) {
			if constexpr (std::is_copy_constructible_v<Arg>) {
				return std::make_unique<Arg>(a);
			}
			else {
				return nullptr;
			}
		};

	public:
		SharedLockable() : data{ TryInstance() } {};
		SharedLockable(Arg const& a) : data{ TryInstance(a) } {};
		SharedLockable(std::unique_ptr<Arg> && a) : data{ std::move(a) } {};
		SharedLockable(const SharedLockable& r) : data{ TryInstance(*r.Shared()) } {};
		SharedLockable& operator=(const SharedLockable& r) {
			*Unique() = *r.Shared();
		};
		SharedLockable(SharedLockable&& r) : data{ TryInstance(*r.Shared()) } {};
		SharedLockable& operator=(SharedLockable&& r) {
			*Unique() = *r.Shared();
		};
		~SharedLockable() = default;

		template <typename T> bool operator==(T b) const {
			return *Shared() == b;
		};
		template <typename T> bool operator!=(T b) const {
			return !operator==(b);
		};

	public:
		bool CompareSwap(Arg const& compare, Arg const& input) {
			auto x = Unique();
			if (*x == compare) {
				*x = input;
				return true;
			}
			else {
				return false;
			}
		}; // returns the previous value while changing the underlying value
		Arg Swap(Arg const& input) {
			auto x = Unique();
			Arg out = *x;
			*x = input;
			return out;
		}; // returns the previous value while changing the underlying value
		template <typename F> // std::function<Arg(Arg)>
		Arg Update(F const& updateFunction) {
			auto x = Unique();
			Arg out;
			*x = updateFunction(out = *x);
			return out;
		}; // returns the previous value while incrementing the actual counter

		void unsafe_set_ptr(std::unique_ptr<Arg>&& input) {
			data = std::move(input);
		}; // returns the previous value while changing the underlying value

	public: // std::atomic compatability
		Arg exchange(Arg const& v) {
			return Swap(v);
		}; // returns the previous value while setting the value to the input
		Arg load() const {
			return *Shared();
		}; // gets the value
		void store(Arg const& v) {
			Swap(v);
			return;
		}; // sets the value to the input
	};

	// thread-safe sorted std::map.
	template <typename key_type, typename T, typename Cmp = std::less<key_type>> class Map {		
	protected:
		mutable SharedLockable<std::map<key_type, T, Cmp>> data;
		typedef std::map<key_type, T, Cmp> underlying;

	public:
		void try_emplace(const key_type& _Keyval, T _Mapval) {
			auto shared = data.Unique();
			shared->try_emplace(_Keyval, std::move(_Mapval));
		};
		void insert_or_assign(const key_type& _Keyval, T _Mapval) {
			auto shared = data.Unique();
			shared->insert_or_assign(_Keyval, std::move(_Mapval));
		};
		// Equal to operator[], but returns a unique lock
		typename SharedLockable<T>::Obj UniqueAt(const key_type& _Keyval) {
			auto shared = data.Unique();
			T& result = shared->operator[](_Keyval);
			return typename SharedLockable<T>::Obj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj operator[](const key_type& _Keyval) {
			while (true) {
				if (auto shared = data.Unique()) {
					(void)shared->operator[](_Keyval);
				}
				if (auto shared = data.Shared()) {
					try {
						T& result = shared->at(_Keyval);
						return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
					}
					catch (std::out_of_range&) {}
				}
			}
		};
		typename SharedLockable<T>::SharedConstObj operator[](const key_type& _Keyval) const {
			auto shared = data.Shared();
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj at(const key_type& _Keyval) {
			auto shared = data.Shared();
			T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj at(const key_type& _Keyval) const {
			auto shared = data.Shared();
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		size_t size() const {
			auto shared = data.Shared();
			return shared->size();
		};
		size_t erase(const key_type& _Keyval) {
			auto shared = data.Unique();
			return shared->erase(_Keyval);
		};
		void clear() {
			auto shared = data.Unique();
			shared->clear();
		};
		size_t count(const key_type& _Keyval) const {
			auto shared = data.Shared();
			return shared->count(_Keyval);
		};
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};
		
	private:
		class it_state {			
		public:
			using thisType = Map;
			using value_type = typename underlying::value_type;
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

			// data
			mutable typename underlying::iterator 
				_ptr{};
			std::shared_ptr<std::shared_lock<std::shared_mutex>> 
				lifetime{ nullptr };

			// functions
			void Initialize(thisType* ref) {
				auto shared = ref->data.Shared();
				lifetime = shared.ForwardLock();
			};
			void ToBeginning(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->begin();
			};
			void ToEnd(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->end();
			};
			void Next(thisType* ref) {
				++_ptr;
			};
			void Prev(thisType* ref) {
				--_ptr;
			};
			value_type& Get(thisType* ref) const {
				return *_ptr;
			};
			bool operator==(it_state const& rhs) const {
				return _ptr == rhs._ptr;
			};
			difference_type Distance(it_state const& other) const { return _ptr - other._ptr; };
		};
	public:
        SETUP_ITERATOR(Map, it_state);

		iterator find(const key_type& _Keyval) const {
			auto iter = this->end();
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->find(_Keyval);
			}
			return iter;
		};
		iterator lower_bound(const key_type& _Keyval) const {
			iterator iter{ this->end() };
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->lower_bound(_Keyval);
			}
			return iter;
		};
		iterator upper_bound(const key_type& _Keyval) const {
			auto iter{ this->end() };
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->upper_bound(_Keyval);
			}
			return iter;
		};
		iterator FindLargestSmallerEqual(const key_type& pos) const {
			iterator iter{ this->end() };
			if (auto f = data.Shared()) {
				auto ptr = f->lower_bound(pos); // equal to or larger than pos
				if (ptr != f->end()) {
					// map is not empty, AND we either found the actual key, or the one JUST larger than it 
					if (ptr->first == pos) { // consider converting this to using the std::less (e.g. Cmp) comparator, rather than the == comparitor.
						// DONE
					}
					else {
						// we are beyond our goal -- move back one. 
						ptr = std::prev(ptr);

						// what if this was actually the first position? 
						if (ptr == f->end()) {
							ptr = f->begin();
						}
					}
				}
				else {
					if (!f->empty()) {
						ptr = std::prev(f->end()); // get what you get
					}
				}
				iter.state._ptr = ptr;
			}
			return iter;
		};
		iterator FindSmallestLargerEqual(const key_type& pos) const {
			return lower_bound(pos); // equal to or larger than pos
		};

	public:
		friend bool operator==(Map const& _Left, Map const& _Right) {
			return (_Left.size() == _Right.size())
				&& (*_Left.data.Shared() == *_Right.data.Shared());
		};
		friend bool operator!=(Map const& _Left, Map const& _Right) {
			return !operator==(_Left, _Right);
		};

	};
	
	// thread-safe unsorted concurrency::unordered_map. Higher performance than the sorted map.
	template <typename key_type, typename T, typename Hasher = std::hash<key_type>> class UnorderedMap {
	protected:
		typedef concurrency::concurrent_unordered_map<key_type, T, Hasher> underlying;
		mutable SharedLockable<underlying> data;
	
	public:
		template <class... _Mappedty> bool insert(const key_type& _Keyval, _Mappedty&&... _Mapval) {
			auto shared = data.Shared();
			return shared->insert(underlying::value_type(_Keyval, std::forward<_Mappedty>(_Mapval)...)).second;
		};
		template <class... _Mappedty> bool insert(key_type&& _Keyval, _Mappedty&&... _Mapval) {
			auto shared = data.Shared();
			return shared->insert(underlying::value_type(std::move(_Keyval), std::forward<_Mappedty>(_Mapval)...)).second;
		};
		template <class... _Mappedty> bool emplace(const key_type& _Keyval, _Mappedty&&... _Mapval) {
			auto shared = data.Shared();
			auto V{ underlying::value_type(_Keyval, std::forward<_Mappedty>(_Mapval)...) };
			if (shared->insert(V).second) {
				return true;
			}
			else {
				shared->operator[](_Keyval) = std::move(V.second);
				return true;
			}
		};
		typename SharedLockable<T>::SharedObj operator[](const key_type& _Keyval) {
			auto shared = data.Shared();
			T& result = shared->operator[](_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj operator[](const key_type& _Keyval) const {
			auto shared = data.Shared();
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj at(const key_type& _Keyval) {
			auto shared = data.Shared();			
			T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj at(const key_type& _Keyval) const {
			auto shared = data.Shared();
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		
		template <class... _Mappedty> typename SharedLockable<T>::SharedObj get_or_insert(const key_type& _Keyval, _Mappedty&&... _Mapval) {
			auto shared = data.Shared();
			T& result = shared->insert(underlying::value_type(std::move(_Keyval), std::forward<_Mappedty>(_Mapval)...)).first->second;
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};

		size_t size() const {
			auto shared = data.Shared();
			return shared->size();
		};
		bool erase(const key_type& _Keyval) {
			auto shared = data.Unique();
			return shared->unsafe_erase(_Keyval) != 0;
		};
		void clear() {
			auto shared = data.Unique();
			shared->clear();
		};
		size_t count(const key_type& _Keyval) const {
			auto shared = data.Shared();
			return shared->count(_Keyval);
		};
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};

	private:
		class it_state {
		public:
			using thisType = UnorderedMap;
			using value_type = typename underlying::value_type;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

			// data
			mutable typename underlying::iterator
				_ptr{};
			std::shared_ptr<std::shared_lock<std::shared_mutex>>
				lifetime{ nullptr };

			// functions
			void Initialize(thisType* ref) {
				auto shared = ref->data.Shared();
				lifetime = shared.ForwardLock();
			};
			void ToBeginning(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->begin();
			};
			void ToEnd(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->end();
			};
			void Next(thisType* ref) {
				++_ptr;
			};
			void Prev(thisType* ref) {
				--_ptr;
			};
			value_type& Get(thisType* ref) const {
				return *_ptr;
			};
			bool operator==(it_state const& rhs) const {
				return _ptr == rhs._ptr;
			};
			difference_type Distance(it_state const& other) const { return _ptr - other._ptr; };
		};
	public:
		SETUP_ITERATOR(UnorderedMap, it_state);
		iterator find(const key_type& _Keyval) const {
			auto iter = this->end();
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->find(_Keyval);
			}
			return iter;
		};

	public:
		friend bool operator==(UnorderedMap const& _Left, UnorderedMap const& _Right) {
			return (_Left.size() == _Right.size())
				&& (*_Left.data.Shared() == *_Right.data.Shared());
		};
		friend bool operator!=(UnorderedMap const& _Left, UnorderedMap const& _Right) {
			return !operator==(_Left, _Right);
		};

	};

	// thread-safe std::vector
	template <typename T> class Vector {
	protected:
		typedef std::vector<T> underlying;
		mutable SharedLockable<underlying> data;

	public:
		void push_back(T&& V) {
			auto shared = data.Unique();
			shared->push_back(std::move(V));
		};
		void push_back(const T& V) {
			auto shared = data.Unique();
			shared->push_back(V);
		};
		typename SharedLockable<T>::SharedObj operator[](size_t _Keyval) {
			auto shared = data.Shared();
			T& result = shared->operator[](_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj operator[](size_t _Keyval) const {
			auto shared = data.Shared();
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj at(size_t _Keyval) {
			auto shared = data.Shared();
			T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj at(size_t _Keyval) const {
			auto shared = data.Shared();
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj front() {
			auto shared = data.Shared();
			T& result = shared->front();
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj front() const {
			auto shared = data.Shared();
			const T& result = shared->front();
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj back() {
			auto shared = data.Shared();
			T& result = shared->back();
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj back() const {
			auto shared = data.Shared();
			const T& result = shared->back();
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		size_t size() const {
			auto shared = data.Shared();
			return shared->size();
		};
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};
		size_t capacity() const {
			auto shared = data.Shared();
			return shared->capacity();
		};
		void reserve(size_t N) const {
			auto shared = data.Unique();
			return shared->reserve(N);
		};
		void resize(size_t N) const {
			auto shared = data.Unique();
			return shared->resize(N);
		};
		void resize(size_t N, const T& V) const {
			auto shared = data.Unique();
			return shared->resize(N, V);
		};
		size_t max_size() {
			auto shared = data.Shared();
			return shared->max_size();
		};
		void clear() {
			auto shared = data.Unique();
			shared->clear();
		};
		// removes element, shifts all other elements to maintain order
		void erase(size_t index) {
			auto shared = data.Unique();
			shared->erase(shared->begin() + index);
		};
		// removes element, swapping with the last element, and does NOT maintain order of the list.
		void erase_fast(size_t index) {
			auto shared = data.Unique();
			shared->operator[](index) = shared->operator[](shared->size()-1);
			shared->resize(shared->size() - 1);
		};
		friend bool operator==(Vector const& _Left, Vector const& _Right) {
			return (_Left.size() == _Right.size())
				&& (*_Left.data.Shared() == *_Right.data.Shared());
		};
		friend bool operator!=(Vector const& _Left, Vector const& _Right) {
			return !operator==(_Left, _Right);
		};

	private:
		class it_state {
		public:
			using thisType = Vector;
			using value_type = typename underlying::value_type;
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

			// data
			mutable typename underlying::iterator
				_ptr{};
			std::shared_ptr<std::shared_lock<std::shared_mutex>>
				lifetime{ nullptr };

			// functions
			void Initialize(thisType* ref) {
				auto shared = ref->data.Shared();
				lifetime = shared.ForwardLock();
			};
			void ToBeginning(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->begin();
			};
			void ToEnd(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->end();
			};
			void Next(thisType* ref) {
				++_ptr;
			};
			void Prev(thisType* ref) {
				--_ptr;
			};
			value_type& Get(thisType* ref) const {
				return *_ptr;
			};
			bool operator==(it_state const& rhs) const {
				return _ptr == rhs._ptr;
			};
			difference_type Distance(it_state const& other) const { return _ptr - other._ptr; };
		};
	public:
		SETUP_ITERATOR(Vector, it_state);

	};

	// thread-safe concurrent queue, with First-In-First-Out (FIFO) functionality.
	template <typename T> class Queue {
	protected:
		typedef concurrency::concurrent_queue<T> underlying;
		mutable SharedLockable<underlying> data;

	public:
		void push(T&& V) {
			auto shared = data.Shared();
			shared->push(std::move(V));
		};
		void push(const T& V) {
			auto shared = data.Shared();
			shared->push(V);
		};
		bool try_pop(T& V) {
			auto shared = data.Shared();
			return shared->try_pop(V);
		};
		[[nodiscard]] std::optional<T> pop() {
			auto shared = data.Shared();
			T out;
			if (shared->try_pop(out)) {
				return std::move(out);
			}
			else {
				return std::nullopt;
			}
		};
		size_t size() const {
			auto shared = data.Unique();
			return shared->unsafe_size();
		};
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};
		void clear() {
			auto shared = data.Unique();
			shared->clear();
		};

	};

	// thread-safe queue, with Last-In-First-Out (LIFO) functionality. FIFO Queue is higher-performance under contention, utilizing a concurrent queue.
	template <typename T> class Stack {
	protected:
		typedef std::stack<T> underlying;
		mutable SharedLockable<underlying> data;

	public:
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};
		size_t size() const {
			auto shared = data.Shared();
			return shared->size();
		};
		template <class... _Valty> void emplace(_Valty&&... _Val) {
			auto shared = data.Unique();
			return shared->emplace(std::forward<_Valty>(_Val)...);
		};
		void push(T&& _Val) {
			auto shared = data.Unique();
			return shared->push(std::forward<T>(_Val));
		};
		void push(T const& _Val) {
			auto shared = data.Unique();
			return shared->push(_Val);
		};
		bool try_pop(T& _Val) {
			auto shared = data.Unique();
			if (shared->size() > 0) {
				_Val = shared->top();
				shared->pop();
				return true;
			}
			else {
				return false;
			}
		};
		[[nodiscard]] std::optional<T> pop() {
			auto shared = data.Unique();
			if (shared->size() > 0) {
				defer(shared->pop());
				return shared->top();
			}
			else {
				return std::nullopt;
			}
		};
	};

	// thread-safe sorted std::set
	template <typename key_type, typename Cmp = std::less<key_type>> class Set {
	protected:
		typedef std::set<key_type, Cmp> underlying;
		mutable SharedLockable<underlying> data;		

	public:
		bool emplace(const key_type& _Keyval) {
			auto shared = data.Unique();
			return shared->emplace(_Keyval).second;
		};
		bool emplace(key_type&& _Keyval) {
			auto shared = data.Unique();
			return shared->emplace(std::move(_Keyval)).second;
		};
		bool contains(key_type const& _Keyval) {
			auto shared = data.Shared();
			return shared->count(_Keyval) > 0;
		};
		size_t count(key_type const& _Keyval) {
			auto shared = data.Shared();
			return shared->count(_Keyval);
		};
		size_t size() const {
			auto shared = data.Shared();
			return shared->size();
		};
		size_t erase(const key_type& _Keyval) {
			auto shared = data.Unique();
			return shared->erase(_Keyval);
		};
		void clear() {
			auto shared = data.Unique();
			shared->clear();
		};
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};

	private:
		class it_state {
		public:
			using thisType = Set;
			using value_type = typename underlying::value_type;
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

			// data
			mutable typename underlying::iterator
				_ptr{};
			std::shared_ptr<std::shared_lock<std::shared_mutex>>
				lifetime{ nullptr };

			// functions
			void Initialize(thisType* ref) {
				auto shared = ref->data.Shared();
				lifetime = shared.ForwardLock();
			};
			void ToBeginning(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->begin();
			};
			void ToEnd(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->end();
			};
			void Next(thisType* ref) {
				++_ptr;
			};
			void Prev(thisType* ref) {
				--_ptr;
			};
			value_type Get(thisType* ref) const {
				return *_ptr;
			};
			bool operator==(it_state const& rhs) const {
				return _ptr == rhs._ptr;
			};
			difference_type Distance(it_state const& other) const { return _ptr - other._ptr; };
		};
	public:
		SETUP_ITERATOR(Set, it_state);

		iterator find(const key_type& _Keyval) const {
			auto iter = this->end();
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->find(_Keyval);
			}
			return iter;
		};
		iterator lower_bound(const key_type& _Keyval) const {
			iterator iter{ this->end() };
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->lower_bound(_Keyval);
			}
			return iter;
		};
		iterator upper_bound(const key_type& _Keyval) const {
			auto iter{ this->end() };
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->upper_bound(_Keyval);
			}
			return iter;
		};
		iterator FindLargestSmallerEqual(const key_type& pos) const {
			iterator iter{ this->end() };
			if (auto f = data.Shared()) {
				auto ptr = f->lower_bound(pos); // equal to or larger than pos
				if (ptr != f->end()) {
					// map is not empty, AND we either found the actual key, or the one JUST larger than it 
					if (*ptr == pos) { // consider converting this to using the std::less (e.g. Cmp) comparator, rather than the == comparitor.
						// DONE
					}
					else {
						// we are beyond our goal -- move back one. 
						ptr = std::prev(ptr);

						// what if this was actually the first position? 
						if (ptr == f->end()) {
							ptr = f->begin();
						}
					}
				}
				else {
					if (!f->empty()) {
						ptr = std::prev(f->end()); // get what you get
					}
				}
				iter.state._ptr = ptr;
			}
			return iter;
		};
		iterator FindSmallestLargerEqual(const key_type& pos) const {
			return lower_bound(pos); // equal to or larger than pos
		};

	public:
		friend bool operator==(Set const& _Left, Set const& _Right) {
			return (_Left.size() == _Right.size())
				&& (*_Left.data.Shared() == *_Right.data.Shared());
		};
		friend bool operator!=(Set const& _Left, Set const& _Right) {
			return !operator==(_Left, _Right);
		};

	};

	// thread-safe unsorted concurrency::unordered_set. Higher performance than the sorted set.
	template <typename key_type, typename Cmp = std::hash<key_type>> class UnorderedSet {
	protected:
		typedef concurrency::concurrent_unordered_set<key_type, Cmp> underlying;
		mutable SharedLockable<underlying> data;

	public:
		bool emplace(const key_type& _Keyval) {
			auto shared = data.Shared();
			return shared->insert(_Keyval).second;
		};
		bool emplace(key_type&& _Keyval) {
			auto shared = data.Shared();
			return shared->insert(std::move(_Keyval)).second;
		};
		bool contains(key_type const& _Keyval) {
			auto shared = data.Shared();
			return shared->count(_Keyval) > 0;
		};
		size_t count(key_type const& _Keyval) {
			auto shared = data.Shared();
			return shared->count(_Keyval);
		};
		size_t size() const {
			auto shared = data.Shared();
			return shared->size();
		};
		size_t erase(const key_type& _Keyval) {
			auto shared = data.Unique();
			return shared->unsafe_erase(_Keyval);
		};
		void clear() {
			auto shared = data.Unique();
			shared->clear();
		};
		bool empty() const {
			auto shared = data.Shared();
			return shared->empty();
		};

	private:
		class it_state {
		public:
			using thisType = UnorderedSet;
			using value_type = typename underlying::value_type;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

			// data
			mutable typename underlying::iterator
				_ptr{};
			std::shared_ptr<std::shared_lock<std::shared_mutex>>
				lifetime{ nullptr };

			// functions
			void Initialize(thisType* ref) {
				auto shared = ref->data.Shared();
				lifetime = shared.ForwardLock();
			};
			void ToBeginning(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->begin();
			};
			void ToEnd(thisType* ref) {
				auto shared = ref->data.Shared();
				_ptr = shared->end();
			};
			void Next(thisType* ref) {
				++_ptr;
			};
			void Prev(thisType* ref) {
				--_ptr;
			};
			value_type Get(thisType* ref) const {
				return *_ptr;
			};
			bool operator==(it_state const& rhs) const {
				return _ptr == rhs._ptr;
			};
			difference_type Distance(it_state const& other) const { return _ptr - other._ptr; };
		};
	public:
		SETUP_ITERATOR(UnorderedSet, it_state);
		iterator find(const key_type& _Keyval) const {
			auto iter = this->end();
			if (auto shared = data.Shared()) {
				iter.state._ptr = shared->find(_Keyval);
			}
			return iter;
		};

	public:
		friend bool operator==(UnorderedSet const& _Left, UnorderedSet const& _Right) {
			return (_Left.size() == _Right.size())
				&& (*_Left.data.Shared() == *_Right.data.Shared());
		};
		friend bool operator!=(UnorderedSet const& _Left, UnorderedSet const& _Right) {
			return !operator==(_Left, _Right);
		};

	};


	struct linear {};
	struct left_snap {};
	struct right_snap {};
	struct spline {};
	// thread-safe concurrent spline, interpolating between values
	template <typename Key, typename Value> class Spline {
	protected:
		Map<Key, Value> data;

	public:
		Spline() = default;
		Spline(Spline const&) = default;
		Spline(Spline &&) = default;
		Spline& operator=(Spline const&) = default;
		Spline& operator=(Spline&&) = default;
		virtual ~Spline() = default;

		using iterator = typename decltype(data)::iterator;
		using const_iterator = iterator;
		iterator begin() const {
			return data.begin();
		};
		iterator end() const { return begin().end(); };
		iterator cbegin() const { return begin(); };
		iterator cend() const { return end(); };

		iterator FindLargestSmallerEqual(Key const& pos) const {
			return data.FindLargestSmallerEqual(pos);
		};
		iterator FindSmallestLargerEqual(Key const& pos) const {
			return data.FindSmallestLargerEqual(pos);
		};
		void emplace(Key const& x, Value const& y) {
			data.insert_or_assign(x, y);
		};
		size_t erase(Key const& x) {
			return data.erase(x);
		};
		bool contains(Key const& x) {
			return data.count(x) != 0;
		};
		bool empty() const {
			return data.empty();
		};
		size_t size() const {
			return data.size();
		};
		iterator at_index(size_t index) const {
			auto iter{ data.begin() };
			std::advance(iter, index);
			return iter;
		};
		iterator find(Key const& x) const {
			return data.find(x);
		};
		
	private:
		virtual Value spline_interpolate(Key const& x) const {
			return interpolate(x, linear{});
		};

	public:
		Value interpolate(Key const& x, left_snap) const {
			iterator p1 = FindLargestSmallerEqual(x);
			// p0 ... p1 ... * ...  p2 ... p3

			if (p1 != end()) 
				return p1->second;			
			else 
				return Value{};
		};
		Value interpolate(Key const& x, right_snap) const {
			iterator p2 = FindSmallestLargerEqual(x);
			// p0 ... p1 ... * ...  p2 ... p3

			if (p2 != end()) 
				return p2->second;			
			else 
				return Value{};
		};
		Value interpolate(Key const& x, linear) const {
			iterator
				p1{ FindLargestSmallerEqual(x) },
				p2{ FindSmallestLargerEqual(x) };
			// p0 ... p1 ... * ...  p2 ... p3

			auto End = end();
			if (p1 != End) {
				if (p2 != End) {
					if (p1 != p2) {
						// All golden
						double t = (double)((x - p1->first) / (p2->first - p1->first));
						Value copy = p2->second;
						copy = ::fma(t, (double)p2->second, ::fma(-t, (double)p1->second, (double)p1->second));
						return copy;
					}
					else {
						// we have landed ontop of a datapoint
						return p1->second;
					}
				}
				else {
					// Snap Left
					return p1->second;
				}
			}
			else {
				if (p2 != End) {
					// Snap Right
					return p2->second;
				}
				else {
					// No Data
					return Value{};
				}
			}
		};
		Value interpolate(Key const& x) const { // assumes spline interpolation
			return interpolate(x, spline{});
		};
		Value interpolate(Key const& x, spline) const {
			return spline_interpolate(x);
		};

		/// <summary>
		/// Creates an iterator that will step through from Start to End at interval Step, sampling the pattern using the InterpolationType. 
		/// </summary>
		/// <param name="start"></param>
		/// <param name="end"></param>
		/// <param name="step"></param>
		/// <param name="interpolationType"></param>
		/// <returns>Iterator that will sample at the requested interval using the interpolationType</returns>
		template<typename interpType = spline>
		auto GetTimeSeries(Key const& start, Key const& end, Key const& step) const {
			return CustomizedSequence<std::pair<Key, Value>, Key>(
				std::function([this](Key const& x) -> std::pair<Key, Value> {
					return std::pair<Key, Value>{ x, this->interpolate(x, interpType{}) };
				})
				, start
				, end
				, step
			);
		};

	};
	// Special type of spline that crosses through its control points utilizing a catmull-rom algorithm. Generally a good spline for natural data, like water flowrates. 
	template <typename Key, typename Value> class CatmullRomSpline final : public Spline<Key, Value> {
	public:
		CatmullRomSpline() = default;
		CatmullRomSpline(CatmullRomSpline const&) = default;
		CatmullRomSpline(CatmullRomSpline&&) = default;
		CatmullRomSpline& operator=(CatmullRomSpline const&) = default;
		CatmullRomSpline& operator=(CatmullRomSpline&&) = default;
		virtual ~CatmullRomSpline() = default;
	
	private:
		virtual Value spline_interpolate(Key const& x) const override {
			auto
				p1{ this->FindLargestSmallerEqual(x) },
				p2{ this->FindSmallestLargerEqual(x) };
			auto
				p0{ std::prev(p1) },
				p3{ std::next(p2) };
			// p0 ... p1 ... * ...  p2 ... p3

			auto End = this->end();
			if ((p1 != End) && (p2 != End) && (p0 != End) && (p3 != End) && (p1 != p2)) {
				double Y0 = (double)p0->second;
				double Y1 = (double)p1->second;
				double Y2 = (double)p2->second;
				double Y3 = (double)p3->second;

				double X0 = (double)p0->first;
				double X1 = (double)p1->first;
				double X2 = (double)p2->first;
				double X3 = (double)p3->first;

				double s = ((double)x - X1) / (X2 - X1);
				if (!::isfinite(s)) s = 0;

				Value out = p1->second;
				out = ::fma(Y0, ((2.0f - s) * s - 1.0f) * s * 0.5f,                    // -0.5f s * s * s + s * s - 0.5f * s
						::fma(Y1, (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f,         // 1.5f * s * s * s - 2.5f * s * s + 1.0f
							::fma(Y2, ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f,      // -1.5f * s * s * s - 2.0f * s * s + 0.5f s
								::fma(Y3, ((s - 1.0f) * s * s) * 0.5f,                 // 0.5f * s * s * s - 0.5f * s * s
									0))));

				return out;
			}
			else {
				return this->interpolate(x, linear{});
			}
		};
		

	};

};

