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
#include <concurrent_vector.h>
#include <set>
#include <concurrent_unordered_set.h>
#include <stack>
#include "../FiberTasks/Concurrent_Queue.h"
#include "veque.hpp"
#include <intrin.h>

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

	/// <summary>
	/// extremely high precision number for scientific experiments or analysis
	/// </summary>
	// using number = boost::multiprecision::number<boost::multiprecision::cpp_dec_float< std::numeric_limits<double>::digits10 * 4 > >; 

	/// <summary>
	/// Implimentation of std_tuple, with built-in get() functions. It is only as thread-safe as the inner types.
	/// </summary>
	/// <typeparam name="...Args"></typeparam>
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
		std::string ToString() const {
			std::string out;
			if constexpr (num_parameters >= 1) { out = GoodLang::ToString(get<0>()); }
			if constexpr (num_parameters >= 2) { out += std::string(", ") + GoodLang::ToString(get<1>()); }
			if constexpr (num_parameters >= 3) { out += std::string(", ") + GoodLang::ToString(get<2>()); }
			if constexpr (num_parameters >= 4) { out += std::string(", ") + GoodLang::ToString(get<3>()); }
			if constexpr (num_parameters >= 5) { out += std::string(", ") + GoodLang::ToString(get<4>()); }
			if constexpr (num_parameters >= 6) { out += std::string(", ") + GoodLang::ToString(get<5>()); }
			if constexpr (num_parameters >= 7) { out += std::string(", ") + GoodLang::ToString(get<6>()); }
			if constexpr (num_parameters >= 8) { out += std::string(", ") + GoodLang::ToString(get<7>()); }
			if constexpr (num_parameters >= 9) { out += std::string(", ") + GoodLang::ToString(get<8>()); }
			if constexpr (num_parameters >= 10) { out += std::string(", ") + GoodLang::ToString(get<9>()); }
			if constexpr (num_parameters >= 11) { out += std::string(", ") + GoodLang::ToString(get<10>()); }
			if constexpr (num_parameters >= 12) { out += std::string(", ") + GoodLang::ToString(get<11>()); }
			if constexpr (num_parameters >= 13) { out += std::string(", ") + GoodLang::ToString(get<12>()); }
			if constexpr (num_parameters >= 14) { out += std::string(", ") + GoodLang::ToString(get<13>()); }
			if constexpr (num_parameters >= 15) { out += std::string(", ") + GoodLang::ToString(get<14>()); }
			if constexpr (num_parameters >= 16) { out += std::string(", ") + GoodLang::ToString(get<15>()); }
			return std::string("<") + out + ">";
		};
		std::vector< Impl::NodeCache > GetChildren() const {
			std::vector< Impl::NodeCache > out;
			if constexpr (num_parameters >= 1) { out.push_back(GoodLang::GetChildren(get<0>())); }
			if constexpr (num_parameters >= 2) { out.push_back(GoodLang::GetChildren(get<1>())); }
			if constexpr (num_parameters >= 3) { out.push_back(GoodLang::GetChildren(get<2>())); }
			if constexpr (num_parameters >= 4) { out.push_back(GoodLang::GetChildren(get<3>())); }
			if constexpr (num_parameters >= 5) { out.push_back(GoodLang::GetChildren(get<4>())); }
			if constexpr (num_parameters >= 6) { out.push_back(GoodLang::GetChildren(get<5>())); }
			if constexpr (num_parameters >= 7) { out.push_back(GoodLang::GetChildren(get<6>())); }
			if constexpr (num_parameters >= 8) { out.push_back(GoodLang::GetChildren(get<7>())); }
			if constexpr (num_parameters >= 9) { out.push_back(GoodLang::GetChildren(get<8>())); }
			if constexpr (num_parameters >= 10) { out.push_back(GoodLang::GetChildren(get<9>())); }
			if constexpr (num_parameters >= 11) { out.push_back(GoodLang::GetChildren(get<10>())); }
			if constexpr (num_parameters >= 12) { out.push_back(GoodLang::GetChildren(get<11>())); }
			if constexpr (num_parameters >= 13) { out.push_back(GoodLang::GetChildren(get<12>())); }
			if constexpr (num_parameters >= 14) { out.push_back(GoodLang::GetChildren(get<13>())); }
			if constexpr (num_parameters >= 15) { out.push_back(GoodLang::GetChildren(get<14>())); }
			if constexpr (num_parameters >= 16) { out.push_back(GoodLang::GetChildren(get<15>())); }
			return out;
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
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< Union<Args...> >, Union<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<Union<Args...>>, Union<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};

	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< atomic_ptr<Args...> >, atomic_ptr<Args...> const& r, std::string& out) {
			if (auto* p = r.Get()) out = GoodLang::ToString(*p); else out = GoodLang::ToString(nullptr);
		};
		template <typename... Args> __forceinline void GetChildren(Tag<atomic_ptr<Args...>>, atomic_ptr<Args...> const& r, std::vector< NodeCache >& out) {
			if (auto* p = r.Get()) out.push_back(GoodLang::GetChildren(*p));
		};
		//template <typename... Args> __forceinline void TryDisconnectChild(Tag<atomic_ptr<Args...>>, atomic_ptr<Args...> const& r, bool& out) {
		//	const_cast<atomic_ptr<Args...>&>(r) = nullptr;
		//	out = true;
		//};
	};

	/// <summary>
	/// thread-safe and fiber-safe integer (atomic swapping of integers)
	/// </summary>
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
	namespace Impl {
		__forceinline void ToString(Tag< InterlockedLong >, InterlockedLong const& r, std::string& out) {
			out = GoodLang::ToString(r.load());
		};
		__forceinline void GetChildren(Tag<InterlockedLong>, InterlockedLong const& r, std::vector< NodeCache >& out) {
			out.push_back(GoodLang::GetChildren(r.load()));
		};
	};

	/// <summary>
	/// allows any copiable object to be thread-safe by wrapping it in a locking container.
	/// </summary>
	/// <typeparam name="Arg"></typeparam>
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
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< Lockable<Args...> >, Lockable<Args...> const& r, std::string& out) {
			auto read = r.Read();
			out = GoodLang::ToString(*read);
		};
		template <typename... Args> __forceinline void GetChildren(Tag<Lockable<Args...>>, Lockable<Args...> const& r, std::vector< NodeCache >& out) {
			auto read = r.Read();
			out.push_back(GoodLang::GetChildren(*read));
		};
	};

	/// <summary>
	/// allows any copiable object to be thread-safe by wrapping it in a locking container.
	/// </summary>
	/// <typeparam name="Arg"></typeparam>
	template <typename Arg, typename Deleter = std::default_delete<Arg>> class SharedLockable {
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

	public:
		std::unique_ptr<Arg, Deleter> data;
		mutable std::shared_mutex lock{};
		void EnsureDataExists() const {
			if (!data) {
				lock.lock();
				if (!data) {
					const_cast<std::unique_ptr<Arg, Deleter>&>(data) = TryInstance();
				}
				lock.unlock();
			}
		};
		bool DataExists() const {
			return (bool)data;
		};

	public:
		auto& GetLock() const { return lock; };

		[[nodiscard]] ConstObj Unique() const { EnsureDataExists(); return ConstObj(*data.get(), lock); };
		[[nodiscard]] Obj Unique() { EnsureDataExists(); return Obj(*data.get(), lock); };
		[[nodiscard]] SharedConstObj Shared() const { EnsureDataExists(); return SharedConstObj(*data.get(), lock); };
		[[nodiscard]] SharedObj Shared() { EnsureDataExists(); return SharedObj(*data.get(), lock); };
		
		auto ForwardSharedLock() { 
			return std::make_shared<std::shared_lock< decltype(lock) >>(lock);
		};
		auto ForwardUniqueLock() {
			return std::make_shared<std::unique_lock< decltype(lock) >>(lock);
		};

	public:
		SharedLockable Copy() const {
			return SharedLockable(load());
		};

	public:
		operator Arg() const {
			return load();
		};

	private:
		static std::unique_ptr<Arg, Deleter> TryInstance() {
			if constexpr (std::is_constructible_v<Arg>) {
				return std::make_unique<Arg>();
			}
			else {
				return nullptr;
			}
		};
		static std::unique_ptr<Arg, Deleter> TryInstance(Arg const& a) {
			if constexpr (std::is_copy_constructible_v<Arg>) {
				return std::make_unique<Arg>(a);
			}
			else {
				return nullptr;
			}
		};

	public:
		SharedLockable() : data{ /*TryInstance()*/ } {};
		SharedLockable(Arg const& a) : data{ TryInstance(a) } {};
		SharedLockable(std::unique_ptr<Arg, Deleter> && a) : data{ std::move(a) } {};
		SharedLockable(const SharedLockable& r) : data{} {
			r.lock.lock_shared();
			if (r.data) {
				data = TryInstance(*r.data);
			}
			r.lock.unlock_shared();
		};
		SharedLockable& operator=(const SharedLockable& r) {
			*Unique() = *r.Shared();
			return *this;
		};
		SharedLockable(SharedLockable&& r) : data{ std::move(r.data) } {};
		SharedLockable& operator=(SharedLockable&& r) {
			*Unique() = *r.Shared();
			return *this;
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

		void unsafe_set_ptr(std::unique_ptr<Arg, Deleter>&& input) {
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
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< SharedLockable<Args...> >, SharedLockable<Args...> const& r, std::string& out) {
			auto read = r.Shared();
			out = GoodLang::ToString(*read);
		};
		template <typename... Args> __forceinline void GetChildren(Tag<SharedLockable<Args...>>, SharedLockable<Args...> const& r, std::vector< NodeCache >& out) {
			auto read = r.Shared();
			out.push_back(GoodLang::GetChildren(*read));
		};
	};

	namespace impl {
		struct Interlocked {
			Interlocked& operator=(Interlocked const& i) {
				SetValue(i.load());
				return *this;
			};
			Interlocked& operator=(long i) {
				SetValue(i);
				return *this;
			};

			explicit operator long() { return GetValue(); };
			explicit operator long() const { return GetValue(); };

			explicit operator bool() { if (GetValue() == 0) return false; else return true; };
			explicit operator bool() const { if (GetValue() == 0) return false; else return true; };

			friend Interlocked operator+(long i, const Interlocked& b) { Interlocked out(b); out.Add(i); return out; };
			friend Interlocked operator+(const Interlocked& b, long i) { Interlocked out(b); out.Add(i); return out; };
			friend Interlocked operator-(long i, const Interlocked& b) { Interlocked out{ i }; out.Add(-b.GetValue()); return out; };
			friend Interlocked operator-(const Interlocked& b, long i) { Interlocked out(b); out.Add(-i); return out; };
			friend Interlocked operator/(long i, const Interlocked& b) { return Interlocked{ i } / b.GetValue(); };
			friend Interlocked operator/(const Interlocked& b, long i) { return b.GetValue() / Interlocked{ i }; };

			friend bool operator<=(long i, const Interlocked& b) { return i <= b.GetValue(); };
			friend bool operator<=(const Interlocked& b, long i) { return i > b.GetValue(); };
			friend bool operator>=(long i, const Interlocked& b) { return i >= b.GetValue(); };
			friend bool operator>=(const Interlocked& b, long i) { return i < b.GetValue(); };
			friend bool operator>(long i, const Interlocked& b) { return i > b.GetValue(); };
			friend bool operator>(const Interlocked& b, long i) { return i <= b.GetValue(); };
			friend bool operator<(long i, const Interlocked& b) { return i < b.GetValue(); };
			friend bool operator<(const Interlocked& b, long i) { return i >= b.GetValue(); };

			friend bool operator<=(const Interlocked& i, const Interlocked& b) { return i.GetValue() <= b.GetValue(); };
			friend bool operator>=(const Interlocked& i, const Interlocked& b) { return i.GetValue() >= b.GetValue(); };
			friend bool operator>(const Interlocked& i, const Interlocked& b) { return i.GetValue() > b.GetValue(); };
			friend bool operator<(const Interlocked& i, const Interlocked& b) { return i.GetValue() < b.GetValue(); };

			long operator++(int) { return Increment() - 1; };
			long operator++() { return Increment(); };
			long operator--(int) { return Decrement() - 1; };
			long operator--() { return Decrement(); };

			Interlocked& operator+=(long i) {
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
			Interlocked& operator-=(long i) {
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

			Interlocked& operator+=(const Interlocked& i) { return operator+=(i.GetValue()); };
			Interlocked& operator-=(const Interlocked& i) { return operator-=(i.GetValue()); };

			friend bool operator==(const Interlocked& i, const Interlocked& j) { return i.GetValue() == j.GetValue(); };
			friend bool operator!=(const Interlocked& i, const Interlocked& j) { return i.GetValue() != j.GetValue(); };

			long Increment() { return InterlockedIncrementAcquire(&value); } // atomically increments the integer and returns the new value
			long Decrement() { return InterlockedDecrementRelease(&value); } // atomically decrements the integer and returns the new value
			long Add(long v) { return InterlockedExchangeAdd(&value, v) + v; } // atomically adds a value to the integer and returns the new value
			long Sub(long v) { return InterlockedExchangeAdd(&value, -v) - v; } // atomically subtracts a value from the integer and returns the new value
			long GetValue() const { return value; } // returns the current value of the integer
			void SetValue(long v) { InterlockedExchange(&value, v); };
			bool SetValueIfEqual(long desired, long compare) { return InterlockedCompareExchange(&value, desired, compare) == compare; };
			bool TryIncrementTo(long n) {
				if (Increment() == n) {
					return true;
				}
				Decrement();
				return false;
			};
			void lock() {
				while (!TryIncrementTo(1)) {};
			};
			void unlock() {
				Decrement();
			};
			long load() const { return GetValue(); };
			bool CompareExchange(long oldVersion, long RecordVersion) {
				return SetValueIfEqual(RecordVersion, oldVersion);
			};

			long	value;
		};
	};
	namespace Impl {
		__forceinline void ToString(Tag< impl::Interlocked >, impl::Interlocked const& r, std::string& out) {
			out = GoodLang::ToString(r.load());
		};
		__forceinline void GetChildren(Tag<impl::Interlocked>, impl::Interlocked const& r, std::vector< NodeCache >& out) {
			out.push_back(GoodLang::GetChildren(r.load()));
		};
	};

	/* Thread- and fiber-safe queue which utilizes a fixed-sized buffer of size *maxCapacity*
	Can optionally lock-up once the buffer is full, to support some atomic operations like memory allocators. */
	template<typename Type, short capacity_m, bool FailWhenFull = false> struct RingBuffer {
	private:
		static constexpr bool isPod() { return std::is_pod<Type>::value; };
		static constexpr bool preventOverflow{ false }; // about 200% slower if using preventOverflow

		std::array< Type, capacity_m> elements;
		impl::Interlocked Write_Reservation; // 		
		impl::Interlocked Read_Reservation; // 
		impl::Interlocked Write_Position; // 
		impl::Interlocked Read_Position; // 
		impl::Interlocked entry; // only used when preventOverflow is set to true.
		impl::Interlocked locked; // only used when preventOverflow is set to true.			

	public:
		bool push_back(Type val) {
			if constexpr (preventOverflow) {
				while (locked > 0) {};
				(void)entry++;
			}

			// If full, return failed
			if (((Write_Reservation.value + 1) - Read_Reservation.value) > capacity_m) {
				if constexpr (preventOverflow) {
					entry--;
				}
				return false;
			}

			auto write_position = Write_Reservation++;

			// do the write
			elements[(write_position) % capacity_m] = std::move(val);

			(void)++Write_Position;

			if constexpr (preventOverflow) {
				constexpr static long long threshold{ (long long)capacity_m * (long long)10000 };
				if (write_position > threshold) {
					if (Write_Reservation.value > threshold) {
						if (Read_Reservation.value > threshold) {
							if (Write_Position.value > threshold) {
								if (Read_Position.value > threshold) {
									if (++locked == 1) {
										while (entry > 1) {}

										Write_Reservation -= threshold;
										Read_Reservation -= threshold;
										Write_Position -= threshold;
										Read_Position -= threshold;
									}
									(void)locked--;
								}
							}
						}
					}
				}
				(void)entry--;
			}

			return true;
		};
		bool try_pop(Type& val) {
			if constexpr (preventOverflow) {
				while (locked > 0) {};
				(void)entry++;
			}

			// If empty, return failed.
			if ((Read_Position.value + 1) > Write_Position.value) {
				if constexpr (preventOverflow) {
					(void)entry--;
				}
				return false;
			}

			// If full, return failed
			if constexpr (FailWhenFull) {
				if (((Write_Reservation.value + 1) - Read_Reservation.value) > capacity_m) {
					if constexpr (preventOverflow) {
						(void)entry--;
					}
					return false;
				}
			}

			// Do extraction
			val = std::move(elements[Read_Position++ % capacity_m]);
			(void)++Read_Reservation;
			if constexpr (preventOverflow) {
				(void)entry--;
			}
			return true;
		};
		size_t size() {
			if constexpr (preventOverflow) {
				while (locked > 0) {};
				(void)entry++;
			}
			long out = Write_Position.value - Read_Position.value;
			if constexpr (preventOverflow) {
				(void)entry--;
			}
			return out;
		};
		constexpr size_t capacity() const {
			return capacity_m;
		};
		void clear() {
			if constexpr (preventOverflow) {
				// guarranteed to be thread-safe
				while (true) {
					while (locked > 0) {};
					(void)entry++;

					if (++locked == 1) {
						while (entry > 1) {}
						Write_Reservation = Read_Reservation = Write_Position = Read_Position = 0;
						(void)locked--;
						(void)entry--;
						return;
					}
					else {
						(void)locked--;
						(void)entry--;
					}
				}
			}
			else {
				// not guarranteed to be thread-safe
				Write_Reservation = Read_Reservation = Write_Position = Read_Position = 0;
			}
		};
	};

#if 0
	/// <summary>
	/// Thread-safe allocator. 
	/// When the allocator goes out-of-scope, all children are destroyed. Children may not out-live the Allocator, at risk of a crash.
	/// User is allowed to allocate more memory per-element than necessary, to support custom data signatures. 
	/// </summary>
	template <typename _type_, size_t _blockSize_ = sizeof(_type_) << 4, unsigned int forcedSize = sizeof(_type_)> class BlockAllocator {
	private:
		struct element_item {
			char data[forcedSize]; // actual underlying data (must be first in this list -- do not re-order)
			size_t header_index; // 0 to INF
		};
		struct header {
			size_t parentBlock; // what block do I belong to
			bool initialized; // whether my assigned data was initialized
			element_item* data; // my data, pointing to a slot within the header_block.data
		};
		struct header_block {
			std::array<element_item, _blockSize_> data; // the data for the whole block, allocated all at once. 
			std::array<header, _blockSize_> headers; // headers related to the data
		};

		concurrency::concurrent_vector< header_block >
			header_blocks{}; // blocks of _blockSize_ headers each
		moodycamel::ConcurrentQueue<size_t>
			available_header_indexes{}; // 0..INF to individual headers

		static _type_* Decode(element_item* ptr) { return static_cast<_type_*>(static_cast<void*>(ptr)); };
		static element_item* Encode(const _type_* ptr) { return static_cast<element_item*>(static_cast<void*>(const_cast<_type_*>(ptr))); };
		static constexpr bool isPod() { return std::is_pod<_type_>::value; };

	public:
		BlockAllocator() = default;
		~BlockAllocator() {
			// go through and de-allocate the blocks
			if constexpr (!isPod()) {
				for (header_block& block : header_blocks) {
					for (header& Header : block.headers) {
						if (Header.initialized) {
							static_cast<_type_*>(static_cast<void*>(&Header.data->data[0]))->~_type_();
							Header.initialized = false;
						}
					}
				}
			}
		};

		template <typename... TArgs> _type_* Alloc(TArgs&&... a) noexcept {
			// get (or make) an available header
			size_t header_index{ 0 };

			moodycamel::ConsumerToken consumer(available_header_indexes);

			if (!available_header_indexes.try_pop(consumer, header_index)) {
				// add a new header_block and then move those to this queue
				auto header_block_iterator = header_blocks.grow_by(1); // .push_back({});
				size_t parent_block_index = std::distance(header_blocks.begin(), header_block_iterator);

				moodycamel::ProducerToken producer(available_header_indexes);

				size_t inner_index = 0;
				if (1) {
					header& Header = header_block_iterator->headers[inner_index];

					::memset(static_cast<void*>(&Header), 0, sizeof(header));
					Header.parentBlock = parent_block_index;
					Header.initialized = false;
					Header.data = &header_block_iterator->data[inner_index];
					Header.data->header_index = (parent_block_index * _blockSize_) + inner_index;

					header_index = Header.data->header_index;
				}
				for (inner_index = 1; inner_index < _blockSize_; inner_index++) {
					header& Header = header_block_iterator->headers[inner_index];

					::memset(static_cast<void*>(&Header), 0, sizeof(header));
					Header.parentBlock = parent_block_index;
					Header.initialized = false;
					Header.data = &header_block_iterator->data[inner_index];
					Header.data->header_index = (parent_block_index * _blockSize_) + inner_index;
				}

				auto seq = GoodLang::Sequence<size_t>((parent_block_index * _blockSize_) + 1, (parent_block_index * _blockSize_) + _blockSize_);
				available_header_indexes.push_bulk(producer, seq.begin(), _blockSize_ - 1);
			}

			// Find the data from the header
			header& thisHeader = header_blocks[header_index / _blockSize_].headers[header_index % _blockSize_];
			_type_* out = static_cast<_type_*>(static_cast<void*>(&thisHeader.data->data[0]));

			// Initialize the data
			if constexpr (std::is_default_constructible<_type_>::value) {
				new (static_cast<void*>(&thisHeader.data->data[0])) _type_(std::forward<TArgs>(a)...);
			}
			else {
				std::memset(static_cast<void*>(&thisHeader.data->data[0]), 0, forcedSize);
			}
			thisHeader.initialized = true;

			// Return the data
			return out;
		};
		void Free(const _type_* t) noexcept {
			moodycamel::ProducerToken producer(available_header_indexes);

			element_item* element{ static_cast<element_item*>(static_cast<void*>(const_cast<_type_*>(t))) };
			if constexpr (!isPod()) t->~_type_(); // destroy
			header_blocks[element->header_index / _blockSize_].headers[element->header_index % _blockSize_].initialized = false; // indicate it was destroyed
			available_header_indexes.push(producer, element->header_index); // free the header_index
		};
		template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) noexcept {
			return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
		};
	};
#else
	// Fast lock-free arena allocator. If the current arena is exceeded, will allocate an additional block. 
	template<class T, size_t BlockSize = 128, size_t objectsize = sizeof(T)> class BlockAllocator {
	private:
		// element_t is either the data for T or a pointer to the next free element_t
		struct element_t {
			unsigned char data[((((objectsize + sizeof(bool) + sizeof(element_t*)) + (16 - 1)) & ~(16 - 1)) - sizeof(bool)) - sizeof(element_t*)];
			bool initialized;
			element_t* next;
		};

		// block_t is a contiguous block of elements
		struct block_t {
			element_t elements[BlockSize];
		};

	private:
		concurrency::concurrent_vector<block_t>
			blocks;
		std::atomic<element_t*>
			free;

	private:
		// Allocate one new block of contiguous elements
		void AllocBlock() {
			auto block = blocks.grow_by(1);

			// each element in the block points to the previous element...
			for (int i = 1; i < BlockSize; ++i) block->elements[i].next = &block->elements[i - 1];

			// free should point to the final element in the block, 
			// while the first element of the block points to free's old location.
			while (!free.compare_exchange_weak(block->elements[0].next = free.load(), &block->elements[BlockSize - 1], std::memory_order::memory_order_relaxed)) {}
		};

		// Release all memory held by all blocks
		void ReleaseBlocks() {
			if (!std::is_pod<T>::value)
				for (auto& block : blocks)
					for (auto& element : block.elements)
						if (element.initialized)
							reinterpret_cast<T*>(&element.data[0])->~T();
		};

	public:
		BlockAllocator() : blocks{}, free(nullptr) { /*AllocBlock();*/ };
		~BlockAllocator() { ReleaseBlocks(); };

		// Acquire a new element from the free list and construct it.
		template <typename... TArgs> T* Alloc(TArgs &&... a) {
			T* data{ nullptr };
			element_t* element{ nullptr };

			// Grab the free element and set free to the next item in that list
			// Update made with compare-and-swap loop:
			while (true) {
				if (element = free.load()) {
					// print(GoodLang::printf("element: %p, element->next: %p", element, element->next));
					if (free.compare_exchange_weak(element, element->next, std::memory_order::memory_order_relaxed)) {
						data = reinterpret_cast<T*>(&element->data[0]);
						new (data) T(std::forward<TArgs>(a)...);
						element->initialized = true;
						return data;
					}
				}
				else {
					// if free is empty, grow the list
					AllocBlock();
				}
			}
		};

		// Destroys the element and return its memory to the free list
		void Free(const T* element) {
			if (element == nullptr) { return; }
			if (!std::is_pod<T>::value) element->~T();
			element_t* t = (element_t*)const_cast<T*>(element);
			t->initialized = false;
			while (!free.compare_exchange_weak(t->next = free.load(), t, std::memory_order::memory_order_relaxed)) {}
		};
		template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) noexcept {
			return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
		};

	};
#endif

	class EpochGarbageCollectorImpl {
	public:
		EpochGarbageCollectorImpl() = default;
		virtual ~EpochGarbageCollectorImpl() = default;
		virtual void RunGC() {};

		struct clock
		{
			typedef unsigned long long                 rep;
			typedef std::ratio<1, 2'800'000'000>       period; // My machine is 2.8 GHz
			typedef std::chrono::duration<rep, period> duration;
			typedef std::chrono::time_point<clock>     time_point;
			static const bool is_steady = true;

			static time_point now() noexcept
			{
				return time_point(duration(__rdtsc()));
			}
		};

		class ThreadManager {
		public:
			static const size_t kMaxThreadNum{ 128 };
			static std::shared_ptr<std::atomic_bool[kMaxThreadNum]> id_vec() {
				static std::shared_ptr<std::atomic_bool[kMaxThreadNum]> vec{ new std::atomic_bool[kMaxThreadNum] };
				return vec;
			}; // should be extern
			static auto GetCurrentEpoch() {
				//return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now().time_since_epoch()).count();
				// return clock_ms();
				return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			};

		};
		class HeartBeater {
		public:
			constexpr HeartBeater() = default;
			HeartBeater(const HeartBeater&) = delete;
			HeartBeater(HeartBeater&&) noexcept = delete;
			auto operator=(const HeartBeater& obj)->HeartBeater & = delete;
			auto operator=(HeartBeater&&) noexcept -> HeartBeater & = delete;

			/**
			  * @brief Destroy the object.
			  *
			  * This destructor removes the heart beat and thread reservation flags.
			  */
			~HeartBeater() {
				ThreadManager::id_vec()[*id_].store(false, std::memory_order_relaxed);
			};

			/**
			  * @retval true if this object has a unique thread ID.
			  * @retval false otherwise.
			  */
			[[nodiscard]] bool HasID() const {
				return id_.use_count() > 0;
			};

			/**
			 * @return The assigned ID for this object.
			 */
			[[nodiscard]] size_t GetID() const {
				return *id_;
			};

			/**
			 * @return A weak pointer object to check heart beats of this object.
			 */
			[[nodiscard]] std::weak_ptr<size_t> GetHeartBeat() const {
				return std::weak_ptr<size_t>{id_};
			};

			/**
			  * @brief Set a unique thread ID to this object.
			  *
			  * @param id The thread ID to be assigned.
			  */
			void SetID(const size_t id) {
				id_ = std::make_shared<size_t>(id);
			};

		private:
			/// @brief The assigned ID for this object.
			std::shared_ptr<size_t> id_{};

		};
		class IDManager {
		public:
			IDManager() = delete;
			~IDManager() = delete;

			IDManager(const IDManager&) = delete;
			IDManager(IDManager&&) noexcept = delete;

			auto operator=(const IDManager&)->IDManager & = delete;
			auto operator=(IDManager&&) noexcept -> IDManager & = delete;

			/**
			 * @return The unique thread ID in [0, DBGROUP_MAX_THREAD_NUM).
			 */
			[[nodiscard]] static size_t GetThreadID() {
				return GetHeartBeater().GetID();
			};

			/**
			 * @return A weak pointer object to check heart beats of the current thread.
			 */
			[[nodiscard]] static std::weak_ptr<size_t> GetHeartBeat() {
				return GetHeartBeater().GetHeartBeat();
			};

		private:
			/**
			 * @brief Get the reference to a heart beater.
			 *
			 * When a thread calls this function for the first time, it prepares its
			 * unique ID and heart beat manager.
			 *
			 * @return The reference to a heart beater.
			 */
			[[nodiscard]] static const HeartBeater& GetHeartBeater() {
				thread_local HeartBeater hb{};
				if (!hb.HasID()) {
					auto id = std::hash<std::thread::id>{}(std::this_thread::get_id()) % ThreadManager::kMaxThreadNum;
					while (auto ptr = ThreadManager::id_vec()) {
						auto& dst = ptr[id];
						auto reserved = dst.load(std::memory_order_relaxed);
						if (!reserved && dst.compare_exchange_strong(reserved, true, std::memory_order_relaxed)) {
							hb.SetID(id);
							break;
						}
						if (++id >= ThreadManager::kMaxThreadNum) {
							id = 0;
						}
					}
				}
				return hb;
			};

		};
		class ThreadLocalStorage {
		public:
			using EpochStorageType = long long;

		public:
			ThreadLocalStorage() = default;
			~ThreadLocalStorage() = default;
			ThreadLocalStorage(ThreadLocalStorage const&) = delete;
			ThreadLocalStorage(ThreadLocalStorage&&) = delete;
			ThreadLocalStorage& operator=(ThreadLocalStorage const&) = delete;
			ThreadLocalStorage& operator=(ThreadLocalStorage&&) = delete;

			std::atomic<EpochStorageType> EpochLimit{ 0 }; // to be reclaimed if a pointer is older than this 
			std::atomic<unsigned long long> Active{ 0 }; // incremented when a TLS is utilized. Should not (but is allowed to) exceed a value of 1. 
			EpochGarbageCollectorImpl* parent{ nullptr }; // set once without race condition, and never changed thereafter. 

		private:
			std::atomic<EpochStorageType> Epoch_3{ 0 }; // oldest Epoch
			std::atomic<EpochStorageType> Epoch_2{ 0 }; // middle Epoch
			std::atomic<EpochStorageType> Epoch_1{ 0 }; // youngest Epoch
			std::atomic<long> StackLevel{ 0 };

		public:
			class EpochGuard {
			private:
				ThreadLocalStorage* _parent;
				EpochStorageType _CurrentEpoch;

			public:
				EpochGuard() : _parent{ nullptr }, _CurrentEpoch{} {};
				EpochGuard(ThreadLocalStorage* parent, EpochStorageType CurrentEpoch) : _parent{ parent }, _CurrentEpoch{ CurrentEpoch } {};
				EpochGuard(EpochGuard const&) = delete;
				EpochGuard(EpochGuard&& rhs) : _parent{ std::move(rhs._parent) }, _CurrentEpoch{ std::move(rhs._CurrentEpoch) } {
					rhs._parent = nullptr;
				};
				EpochGuard& operator=(EpochGuard const&) = delete;
				EpochGuard& operator=(EpochGuard&& rhs) {
					if (_parent) {
						if (--_parent->StackLevel == 0) {
							EpochStorageType safeToDelete = _parent->ForwardEpoch(_CurrentEpoch);
							_parent->parent->RunGC();
						}
					}
					_parent = std::move(rhs._parent);
					_CurrentEpoch = std::move(rhs._CurrentEpoch);
					rhs._parent = nullptr;
				}
				~EpochGuard() {
					if (_parent) {
						if (--_parent->StackLevel == 0) {
							EpochStorageType safeToDelete = _parent->ForwardEpoch(_CurrentEpoch);
							_parent->parent->RunGC();
						}
					}
				};
			};
		private:
			// forwards the current Epoch, and returns the epoch for which it is 100% safe to delete for (previous EpochLimit).
			EpochStorageType ForwardEpoch(EpochStorageType CurrentEpoch) {
				--Active;
				return EpochLimit.exchange(
					Epoch_3.exchange(
						Epoch_2.exchange(
							Epoch_1.exchange(
								CurrentEpoch, std::memory_order_relaxed
							), std::memory_order_relaxed
						), std::memory_order_relaxed
					), std::memory_order_relaxed
				);
			};

		public:
			[[nodiscard]] const auto ProtectCurrentEpoch() {
				if (++StackLevel == 1 /*&& Active == 0*/) { ++Active; }
				return EpochGuard(this, ThreadManager::GetCurrentEpoch());
			};

		};

	};

	/// <summary>
	/// Fastest allocator to-date, leveraging a block-allocator per-thread, significantly reducing contention, to the degree that this is now the fastest way to allocate memory!
	/// Plus, it is thread-safe and garbage-collected on end-of-scope. These features are effectively free now. 
	/// </summary>
	/// <typeparam name="_type_"></typeparam>
	template <typename _type_, size_t num_parallel_allocators = 4>
	class Allocator final : public EpochGarbageCollectorImpl {
	private:
		std::array<BlockAllocator<_type_, sizeof(_type_) << 4, sizeof(_type_) + sizeof(size_t)>, num_parallel_allocators> TLS_arr{};
		static auto GetThreadID() { return IDManager::GetThreadID(); };

		struct innerType {
			_type_ T;
			size_t threadID;
		};

	public:
		template <typename... TArgs> _type_* Alloc(TArgs&&... a) {
			size_t thisThreadIndex = GetThreadID() % num_parallel_allocators;
			auto& TLS = TLS_arr[thisThreadIndex];
			_type_* out{ TLS.Alloc(std::forward<TArgs>(a)...) };
			innerType* impl = static_cast<innerType*>(static_cast<void*>(out));
			impl->threadID = thisThreadIndex;
			return out;
		};
		void Free(const _type_* t) {
			innerType* impl = static_cast<innerType*>(static_cast<void*>(const_cast<_type_*>(t)));
			auto& TLS = TLS_arr[impl->threadID];
			TLS.Free(t);
		};
		template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) {
			return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
		};
	};

	/// <summary>
	/// Allocator that can prevent deletion of memory until after it is safe to do so, leveraging Epoch (e.g. time of deletion) to determine safety. 
	/// Typically, three epochs (or iterations) within a thread must pass before a memory recollection is valid. 
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template <typename T, uint64_t CleanupFrequencyMilliseconds = 1> class EpochProtectedAllocator final : public EpochGarbageCollectorImpl {
	private:
		using EpochStorageType = typename ThreadLocalStorage::EpochStorageType;
		using EpochQueueType = std::pair<EpochStorageType, T*>;

		std::array<ThreadLocalStorage, ThreadManager::kMaxThreadNum> TLS_arr;
		std::atomic<EpochStorageType> lastGC;
		Allocator<T> _alloc;

		static auto GetThreadID() { return IDManager::GetThreadID(); };
		auto& GetTLS() { return TLS_arr[GetThreadID()]; };

		moodycamel::ConcurrentQueue< EpochQueueType > DeleteList;

	public:
		// Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
		void RunGC() override {
			static constexpr auto duration{ std::chrono::milliseconds(CleanupFrequencyMilliseconds) };
			static thread_local EpochQueueType out{};
			EpochStorageType _EpochLimit{ std::numeric_limits<EpochStorageType>::max() };
			auto currentGC{ std::chrono::milliseconds(ThreadManager::GetCurrentEpoch()) };

			if ((currentGC - std::chrono::milliseconds(lastGC.load())) > duration) {
				lastGC.store(currentGC.count());

				for (auto& tls : TLS_arr)
					if (tls.Active > 0)
						_EpochLimit = std::min< EpochStorageType>(_EpochLimit, tls.EpochLimit.load());

				if (_EpochLimit > 0)
					while (DeleteList.try_pop(out)) // while jobs are available...
						if (out.first < _EpochLimit) // ...if the data is in the correct time period for reclamation...
							_alloc.Free(out.second); // ... then do the clean-up, and try again (hoping that the list is semi-sorted).
						else
						{
							DeleteList.push(out);
							return;
						}// ... otherwise push to the end of the queue, which is a form of lazy sorting (also prevents endless looping without additional checks / handles). 
			}
		};

	public:
		// Request a new memory pointer
		template <typename... TArgs> T* Alloc(TArgs &&... a) {
			return _alloc.Alloc(std::forward<TArgs>(a)...);
		};

		// Frees the memory pointer
		void				Free(const T* element) {
			DeleteList.push({ ThreadManager::GetCurrentEpoch(), const_cast<T*>(element) });
			RunGC();
		};

	public:
		EpochProtectedAllocator() : EpochGarbageCollectorImpl(), TLS_arr{}, lastGC{ 0 }, DeleteList{} { for (auto& tls : TLS_arr) tls.parent = this; };
		EpochProtectedAllocator(EpochProtectedAllocator const&) = delete;
		EpochProtectedAllocator(EpochProtectedAllocator&&) = delete;
		EpochProtectedAllocator& operator=(EpochProtectedAllocator const&) = delete;
		EpochProtectedAllocator& operator=(EpochProtectedAllocator&&) = delete;
		virtual ~EpochProtectedAllocator() = default;

	public:
		// Stalls deallocation / free calls made after this guard until at least after this guard expires.
		[[nodiscard]] const auto CreateEpochGuard() {
			return GetTLS().ProtectCurrentEpoch();
		};

	};


	template <typename T> class ThreadLocalInstance final : public EpochGarbageCollectorImpl {
	private:
		std::array<T, ThreadManager::kMaxThreadNum> TLS_arr;
		static auto GetThreadID() { return IDManager::GetThreadID(); };
		auto& GetTLS() { return TLS_arr[GetThreadID()]; };
		auto& GetTLS() const { return TLS_arr[GetThreadID()]; };

	public:
		T* operator->() { return &GetTLS(); };
		const T* operator->() const { return &GetTLS(); };
		T& operator*() { return GetTLS(); };
		const T& operator*() const { return GetTLS(); };

		ThreadLocalInstance& operator=(T const& val) {
			for (auto& x : TLS_arr) {
				x = val;
			}
			return *this;
		};
	};


	/// <summary>
	/// thread-safe sorted std::map.
	/// </summary>
	/// <typeparam name="key_type"></typeparam>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="Cmp"></typeparam>
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
		
		/// <summary>
		/// Similar to at(), except if the object does not exist, it will instantiate it with the provided content, and then return the referenced object.
		/// Is less efficienct due to the redundant checks, so recommend only doing this when necessary.
		/// </summary>
		/// <param name="_Keyval"></param>
		/// <param name="_Mapval"></param>
		/// <returns></returns>
		typename SharedLockable<T>::SharedObj get_or_insert(const key_type& _Keyval, T _Mapval = T()) {
			while (true) {
				// try to get it straight-away
				if (auto shared = data.Shared()) {
					try {
						T& result = shared->at(_Keyval);
						return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
					}
					catch (std::out_of_range&) {}
				}
				// failed -- try to create it.
				if (auto shared = data.Unique()) {
					try {
						(void)shared->at(_Keyval);						
					}
					catch (std::out_of_range&) {
						// does not exist -- create it and assign
						shared->insert_or_assign(_Keyval, std::move(_Mapval));
					}
				}
			}
		};

		typename SharedLockable<T>::SharedObj operator[](const key_type& _Keyval) {
			while (true) {
				// try to get it straight-away
				if (auto shared = data.Shared()) {
					try {
						T& result = shared->at(_Keyval);
						return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
					}
					catch (std::out_of_range&) {}
				}
				// failed -- try to create it.
				if (auto shared = data.Unique()) {
					(void)shared->operator[](_Keyval);
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
				ref->data.EnsureDataExists();
				lifetime = ref->data.ForwardSharedLock();
			};
			void ToBeginning(thisType* ref) {
				if (auto* p = ref->data.data.get()) {
					_ptr = p->begin();
				}
			};
			void ToEnd(thisType* ref) {
				if (auto* p = ref->data.data.get()) {
					_ptr = p->end();
				}
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
		std::string ToString() const{ return GoodLang::ToString(data); };
		std::vector< Impl::NodeCache > GetChildren() const { return { GoodLang::GetChildren(data) }; };
	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< Map<Args...> >, Map<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<Map<Args...>>, Map<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};

	/// <summary>
	/// thread-safe unsorted concurrency::unordered_map. Higher performance than the sorted map.
	/// </summary>
	/// <typeparam name="key_type"></typeparam>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="Hasher"></typeparam>
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
			auto V{ underlying::value_type(_Keyval, std::forward<_Mappedty>(_Mapval)...) };
			data.EnsureDataExists();
			data.lock.lock_shared();
			if (data.data->insert(V).second) {
				data.lock.unlock_shared();
				return true;
			}
			else {
				data.data->operator[](_Keyval) = std::move(V.second);
				data.lock.unlock_shared();
				return true;
			}

			//auto shared = data.Shared();			
			//if (shared->insert(V).second) {
			//	return true;
			//}
			//else {
			//	shared->operator[](_Keyval) = std::move(V.second);
			//	return true;
			//}
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
			size_t out{ 0 };
			data.lock.lock_shared();
			if (auto* p = data.data.get()) {
				out = p->size();
			}
			data.lock.unlock_shared();
			return out;
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
			bool out{ false };
			data.lock.lock_shared();
			if (auto* p = data.data.get()) {
				out = p->empty();
			}
			data.lock.unlock_shared();
			return out;
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
				ref->data.EnsureDataExists();
				lifetime = ref->data.ForwardSharedLock();
			};
			void ToBeginning(thisType* ref) {
				if (auto* p = ref->data.data.get()) {
					_ptr = p->begin();
				}
			};
			void ToEnd(thisType* ref) {
				if (auto* p = ref->data.data.get()) {
					_ptr = p->end();
				}
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
			if (auto shared = data.Shared()) {
				typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;
				return iterator(const_cast<thisType*>(this), it_state{ shared->find(_Keyval), shared.ForwardLock() });
			}
			else {
				return this->end();
			}
		};

	public:
		friend bool operator==(UnorderedMap const& _Left, UnorderedMap const& _Right) {
			return (_Left.size() == _Right.size())
				&& (*_Left.data.Shared() == *_Right.data.Shared());
		};
		friend bool operator!=(UnorderedMap const& _Left, UnorderedMap const& _Right) {
			return !operator==(_Left, _Right);
		};
		std::string ToString() const { return GoodLang::ToString(data); };
		std::vector< Impl::NodeCache > GetChildren() const { return { GoodLang::GetChildren(data) }; };
	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< UnorderedMap<Args...> >, UnorderedMap<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<UnorderedMap<Args...>>, UnorderedMap<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};

	/// <summary>
	/// thread-safe std::vector
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template <typename T> class Vector {
	protected:
		typedef std::vector<T> underlying;
		mutable SharedLockable<underlying> data;

	public:
		Vector() = default;
		explicit Vector(size_t count, T defaultObj) : data(underlying(count, std::move(defaultObj))) {};
		Vector(Vector const&) = default;
		Vector(Vector &&) = default;
		Vector& operator=(Vector const&) = default;
		Vector& operator=(Vector&&) = default;
		~Vector() = default;

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
			if (shared->size() <= _Keyval) throw std::out_of_range("Out of range of Vector");
			T& result = shared->operator[](_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj operator[](size_t _Keyval) const {
			auto shared = data.Shared();
			if (shared->size() <= _Keyval) throw std::out_of_range("Out of range of Vector");
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj at(size_t _Keyval) {
			auto shared = data.Shared();
			if (shared->size() <= _Keyval) throw std::out_of_range("Out of range of Vector");
			T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj at(size_t _Keyval) const {
			auto shared = data.Shared();
			if (shared->size() <= _Keyval) throw std::out_of_range("Out of range of Vector");
			const T& result = shared->at(_Keyval);
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj front() {
			auto shared = data.Shared();
			if (shared->size() <= 0) throw std::out_of_range("Out of range of Vector");
			T& result = shared->front();
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj front() const {
			auto shared = data.Shared();
			if (shared->size() <= 0) throw std::out_of_range("Out of range of Vector");
			const T& result = shared->front();
			return typename SharedLockable<T>::SharedConstObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedObj back() {
			auto shared = data.Shared();
			if (shared->size() <= 0) throw std::out_of_range("Out of range of Vector");
			T& result = shared->back();
			return typename SharedLockable<T>::SharedObj(result, shared.ForwardLock());
		};
		typename SharedLockable<T>::SharedConstObj back() const {
			auto shared = data.Shared();
			if (shared->size() <= 0) throw std::out_of_range("Out of range of Vector");
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
		void reserve(size_t N) {
			auto shared = data.Unique();
			return shared->reserve(N);
		};
		void resize(size_t N) {
			auto shared = data.Unique();
			return shared->resize(N);
		};
		void resize(size_t N, const T& V) {
			auto shared = data.Unique();
			return shared->resize(N, V);
		};
		size_t max_size() const {
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
			if (index >= shared->size()) return;
			shared->erase(shared->begin() + index);
		};
		// removes element, swapping with the last element, and does NOT maintain order of the list.
		void erase_fast(size_t index) {
			auto shared = data.Unique();
			if (index >= shared->size()) return;
			if ((shared->size() - 1) != index) { // no point in moving data if the index would be identical
				shared->operator[](index) = shared->operator[](shared->size() - 1);
			}
			shared->pop_back();
		};
		// removes element, swapping with the last element, and does NOT maintain order of the list.
		void pop_back() {
			auto shared = data.Unique();
			shared->pop_back();
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
				ref->data.EnsureDataExists();
				lifetime = ref->data.ForwardSharedLock();
			};
			void ToBeginning(thisType* ref) {
				if (auto* p = ref->data.data.get()) {
					_ptr = p->begin();
				}
			};
			void ToEnd(thisType* ref) {
				if (auto* p = ref->data.data.get()) {
					_ptr = p->end();
				}
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

		std::string ToString() const { return GoodLang::ToString(data); };
		std::vector< Impl::NodeCache > GetChildren() const { return { GoodLang::GetChildren(data) }; };
	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< Vector<Args...> >, Vector<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<Vector<Args...>>, Vector<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};

	/// <summary>
	/// thread-safe concurrent queue, with First-In-First-Out (FIFO) functionality.
	/// </summary>
	/// <typeparam name="T"></typeparam>
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

	/// <summary>
	/// thread-safe queue, with Last-In-First-Out (LIFO) functionality. 
	/// FIFO Queue is higher-performance under contention, utilizing a concurrent queue.
	/// </summary>
	/// <typeparam name="T"></typeparam>
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

	/// <summary>
	/// thread-safe sorted std::set
	/// </summary>
	/// <typeparam name="key_type"></typeparam>
	/// <typeparam name="Cmp"></typeparam>
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
				ref->data.EnsureDataExists();
				lifetime = ref->data.ForwardSharedLock();
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
		std::string ToString() const { return GoodLang::ToString(data); };
		std::vector< Impl::NodeCache > GetChildren() const { return { GoodLang::GetChildren(data) }; };
	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< Set<Args...> >, Set<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<Set<Args...>>, Set<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};

	/// <summary>
	/// thread-safe unsorted concurrency::unordered_set. Higher performance than the sorted set.
	/// </summary>
	/// <typeparam name="key_type"></typeparam>
	/// <typeparam name="Cmp"></typeparam>
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
		std::string ToString() const { return GoodLang::ToString(data); };
		std::vector< Impl::NodeCache > GetChildren() const { return { GoodLang::GetChildren(data) }; };
	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< UnorderedSet<Args...> >, UnorderedSet<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<UnorderedSet<Args...>>, UnorderedSet<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};

	namespace details {
		// single-threaded flat set
		template <typename T> class flat_set {
		private:
			veque::veque<T> queue;
		public:
			auto emplace(T const& item) {
				return queue.insert(
					std::upper_bound(queue.begin(), queue.end(), item),
					item
				);
			};
			void clear() {
				queue.clear();
			};
			void emplace_fast(T&& val) {
				queue.push_back(std::move(val));
			};
			void emplace_fast(T const& val) {
				queue.push_back(val);
			};
			bool contains_fast(T const& find) {
				for (auto& x : queue) {
					if (x == find) {
						return true;
					}
				}
				return false;
			};
			bool contains(T const& v) {
				return std::binary_search(queue.begin(), queue.end(), v);
			};
			bool contains(T&& v) {
				return std::binary_search(queue.begin(), queue.end(), std::move(v));
			};
		};
	};

	namespace details {
		/*
		================================================
		cweeBlockAlloc is a block-based allocator for fixed-size objects.
		All objects are properly constructed and destructed.
		================================================
		*/
#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )
#define BLOCK_ALLOC_ALIGNMENT 16

		template<class _type_, size_t BlockSize = 128, size_t objectsize = sizeof(_type_)>
		class SingleThreadedAllocator {
		private:
			static constexpr bool isPod() { return std::is_pod<_type_>::value; };

		public:
			class BlockAlloc {
			public:
				BlockAlloc(bool clear = true) : // = false
					blocks(NULL),
					free(NULL),
					total(0),
					active(0),
					allowAllocs(true),
					clearAllocs(clear)
				{};
				BlockAlloc(int toReserve) :
					blocks(NULL),
					free(NULL),
					total(0),
					active(0),
					allowAllocs(true),
					clearAllocs(true)
				{
					Reserve(toReserve);
				};
				~BlockAlloc() {
					Shutdown();
				};

				// returns total size of allocated memory
				size_t				Allocated() const { return total * objectsize; }

				// returns total size of allocated memory including size of (*this)
				size_t				Size() const { return sizeof(*this) + Allocated(); }

				void				Shutdown() {
					while (blocks != NULL) {
						cweeBlock* block = blocks;
						blocks = blocks->next;
						delete block;
					}
					blocks = NULL;
					free = NULL;
					total = active = 0;
				};
				void			SetFixedBlocks(long long numBlocks) {
					long long currentNumBlocks = 0;
					for (cweeBlock* block = blocks; block != NULL; block = block->next) {
						currentNumBlocks++;
					}
					for (long long i = currentNumBlocks; i < numBlocks; i++) {
						AllocNewBlock();
					}
					allowAllocs = false;
				};
				void			FreeEmptyBlocks() {
					// first count how many free elements are in each block and build up a free chain per block
					for (cweeBlock* block = blocks; block != NULL; block = block->next) {
						block->free = NULL;
						block->freeCount = 0;
					}
					for (element_t* element = free; element != NULL; ) {
						element_t* next = element->next;
						for (cweeBlock* block = blocks; block != NULL; block = block->next) {
							if (element >= block->elements && element < block->elements + BlockSize) {
								element->next = block->free;
								block->free = element;
								block->freeCount++;
								break;
							}
						}
						// if this assert fires, we couldn't find the element in any block
						assert(element->next != next);
						element = next;
					}
					// now free all blocks whose free count == BlockSize
					cweeBlock* prevBlock = NULL;
					for (cweeBlock* block = blocks; block != NULL; ) {
						cweeBlock* next = block->next;
						if (block->freeCount == BlockSize) {
							if (prevBlock == NULL) {
								assert(blocks == block);
								blocks = block->next;
							}
							else {
								assert(prevBlock->next == block);
								prevBlock->next = block->next;
							}
							delete block;
							total -= BlockSize;
						}
						else {
							prevBlock = block;
						}
						block = next;
					}
					// now rebuild the free chain
					free = NULL;
					for (cweeBlock* block = blocks; block != NULL; block = block->next) {
						for (element_t* element = block->free; element != NULL; ) {
							element_t* next = element->next;
							element->next = free;
							free = element;
							element = next;
						}
					}
				};

				static constexpr bool isPod() { return std::is_pod<_type_>::value; };

				template <typename... TArgs> _type_* Alloc(TArgs &&... a) {
					if (free == NULL) {
						if (!allowAllocs) {
							return NULL;
						}
						AllocNewBlock();
					}

					active++;
					element_t* element = free;
					free = free->next;
					element->next = NULL;

					_type_* t = (_type_*)element->buffer;

					if constexpr (isPod() && (sizeof...(a) == 0)) {
						memset(t, 0, objectsize);
					}
					else {
						if (clearAllocs) memset(t, 0, objectsize);
						new (t) _type_(std::forward<TArgs>(a)...);
					}

					return t;
				};
				void				Free(_type_* element) {
					if (element == nullptr) {
						return;
					}

					if constexpr (!isPod()) {
						element->~_type_();
					}

					element_t* t = (element_t*)(element);
					t->next = free;
					free = t;
					active--;
				};
				void			    Reserve(long long num) {
					if (total < num) {
						std::vector< _type_* > arr; arr.reserve(2 * (num - total));
						while (total < num) {
							arr.push_back(Alloc());
						}
						for (_type_* p : arr) {
							Free(p);
						}
					}
				};
				long long			GetTotalCount() const { return total; }
				long long			GetAllocCount() const { return active; }
				long long			GetFreeCount() const { return total - active; }

			private:
				union element_t {
					_type_* data;
					element_t* next;
					::byte			buffer[(CONST_MAX(objectsize, sizeof(element_t*)) + (BLOCK_ALLOC_ALIGNMENT - 1)) & ~(BLOCK_ALLOC_ALIGNMENT - 1)];
				};

				class cweeBlock {
				public:
					element_t		elements[BlockSize];
					cweeBlock* next;
					element_t* free;		// list with free elements in this block (temp used only by FreeEmptyBlocks)
					long long		freeCount;	// number of free elements in this block (temp used only by FreeEmptyBlocks)
				};

				cweeBlock* blocks;
				element_t* free;
				long long			total;
				long long			active;
				bool				allowAllocs;
				bool				clearAllocs;

				void			AllocNewBlock() {
					cweeBlock* block = new cweeBlock(); // (cweeBlock*)Mem_Alloc((size_t)(sizeof(cweeBlock)));
					block->next = blocks;
					blocks = block;
					for (int i = 0; i < BlockSize; i++) {
						block->elements[i].next = free;
						free = &block->elements[i];
						assert((((UINT_PTR)free) & (BLOCK_ALLOC_ALIGNMENT - 1)) == 0);
					}
					total += BlockSize;
				};
			};

		public:
			SingleThreadedAllocator() : ptrs(), alloc() {};
			SingleThreadedAllocator(int toReserve) : ptrs(), alloc(toReserve) {};
			~SingleThreadedAllocator() { Clear(true); };

			template <typename... TArgs> _type_* Alloc(TArgs &&... a) {
				auto p = alloc.Alloc(std::forward<TArgs>(a)...);
				if constexpr (!isPod()) {
					ptrs.insert(p);
				}
				return p;
			};
			void	Free(_type_* element) {
				if constexpr (!isPod()) {
					ptrs.erase(element);
				}
				alloc.Free(element);
			};
			void	Clean() {
				alloc.FreeEmptyBlocks();
			};
			long long	GetTotalCount() const {
				return alloc.GetTotalCount();
			};
			long long	GetAllocCount() const {
				if constexpr (!isPod()) {
					return ptrs.size();
				}
				else {
					return alloc.GetAllocCount();
				}
			};
			void	Clear(bool destroyAllocator = false) {
				if constexpr (!isPod()) {
					for (auto& x : ptrs) {
						if (x != nullptr) {
							alloc.Free(x);
						}
					}
					ptrs.clear();
					alloc.Shutdown();
					if (!destroyAllocator) alloc.Free(alloc.Alloc());
				}
				else {
					alloc.Shutdown();
					if (!destroyAllocator) alloc.Free(alloc.Alloc());
				}
			};
			void	Reserve(long long n) {
				alloc.Reserve(n);
			};

		private:
			std::set<_type_*> ptrs;
			BlockAlloc alloc;
		};

		template <typename _type_, size_t num_parallel_allocators = 4, size_t BlockSize = 128>
		class BTreeAllocator final : public EpochGarbageCollectorImpl {
		private:
			std::array<std::pair<GoodLang::mutex, SingleThreadedAllocator<_type_, BlockSize, sizeof(_type_) + sizeof(size_t)>>, num_parallel_allocators> TLS_arr{};
			static auto GetThreadID() { return IDManager::GetThreadID(); };

			struct innerType {
				_type_ T;
				size_t threadID;
			};

		public:
			template <typename... TArgs> _type_* Alloc(TArgs&&... a) {
				static thread_local size_t thisThreadIndex{ GetThreadID() % num_parallel_allocators };
				auto& TLS = TLS_arr[thisThreadIndex];
				_type_* out;
				TLS.first.lock();
				out = TLS.second.Alloc(std::forward<TArgs>(a)...);
				TLS.first.unlock();
				innerType* impl = static_cast<innerType*>(static_cast<void*>(out));
				impl->threadID = thisThreadIndex;
				return out;
			};
			void Free(const _type_* t) {
				innerType* impl = static_cast<innerType*>(static_cast<void*>(const_cast<_type_*>(t)));
				auto& TLS = TLS_arr[impl->threadID];
				TLS.first.lock();
				TLS.second.Free(const_cast<_type_*>(t));
				TLS.first.unlock();
			};
			template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) {
				return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
			};
		};

		template<class T, size_t BlockSize = 128, uint64_t CleanupFrequencyMilliseconds = 1>
		class FastEpochAllocator final : public EpochGarbageCollectorImpl {
		private:
			using EpochStorageType = typename ThreadLocalStorage::EpochStorageType;
			using EpochQueueType = std::pair<EpochStorageType, T*>;

			std::array<ThreadLocalStorage, ThreadManager::kMaxThreadNum> 
				TLS_arr;
			std::atomic<EpochStorageType> 
				lastGC;
			BTreeAllocator<T, 4, BlockSize> 
				_alloc;
			moodycamel::ConcurrentQueue< EpochQueueType >
				DeleteList;

			static auto GetThreadID() { return IDManager::GetThreadID(); };
			auto& GetTLS() { return TLS_arr[GetThreadID()]; };

		public:
			// Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
			void RunGC() override {
				static constexpr auto duration{ std::chrono::milliseconds(CleanupFrequencyMilliseconds) };
				static thread_local EpochQueueType out{};
				EpochStorageType _EpochLimit{ std::numeric_limits<EpochStorageType>::max() };
				auto currentGC{ std::chrono::milliseconds(ThreadManager::GetCurrentEpoch()) };
				thread_local std::array< EpochQueueType, 10> iter;
				size_t count_popped;
				bool anyFailure;

				if ((currentGC - std::chrono::milliseconds(lastGC.load())) > duration) {
					lastGC.store(currentGC.count());

					for (auto& tls : TLS_arr)
						if (tls.Active > 0)
							_EpochLimit = std::min<EpochStorageType>(_EpochLimit, tls.EpochLimit.load());

					if (_EpochLimit > 0) {						
						
						while ((count_popped = DeleteList.try_pop_bulk(/*token, */iter.begin(), iter.size())) > 0) {
							anyFailure = false;
							for (size_t i = 0; i < count_popped; ++i) {
								if (iter[i].first <= _EpochLimit) { // ...if the data is in the correct time period for reclamation...
									_alloc.Free(iter[i].second); // ... then do the clean-up, and try again (hoping that the list is semi-sorted).
								}
								else {
									DeleteList.push(iter[i]);
									anyFailure = true;
								}// ... otherwise push to the end of the queue, which is a form of lazy sorting (also prevents endless looping without additional checks / handles). 
							}							
							if (anyFailure) break;
						}
					}
				}
			};

		public:
			// Request a new memory pointer
			template <typename... TArgs> T* Alloc(TArgs &&... a) {
				return _alloc.Alloc(std::forward<TArgs>(a)...);
			};

			// Frees the memory pointer
			void				Free(const T* element) {
				DeleteList.push(/*token, */{ ThreadManager::GetCurrentEpoch(), const_cast<T*>(element) });
				RunGC();
			};

		public:
			FastEpochAllocator() : EpochGarbageCollectorImpl(), TLS_arr{}, lastGC{ 0 }, _alloc{}, DeleteList{} { for (auto& tls : TLS_arr) tls.parent = this; };
			FastEpochAllocator(FastEpochAllocator const&) = delete;
			FastEpochAllocator(FastEpochAllocator&&) = delete;
			FastEpochAllocator& operator=(FastEpochAllocator const&) = delete;
			FastEpochAllocator& operator=(FastEpochAllocator&&) = delete;
			virtual ~FastEpochAllocator() = default;

		public:
			// Stalls deallocation / free calls made after this guard until at least after this guard expires.
			[[nodiscard]] const auto CreateEpochGuard() {
				return GetTLS().ProtectCurrentEpoch();
			};

		};

#undef BLOCK_ALLOC_ALIGNMENT
#undef CONST_MAX

		template< class objType, class keyType, int maxChildrenPerNode = 10 >
		class BalancedTree {
		public:
			struct TreeNode {
				keyType				  key;							// key used for sorting
				objType* object;						            // if != NULL pointer to object stored in leaf node
				TreeNode* parent;						// parent node
				TreeNode* next;							// next sibling
				TreeNode* prev;							// prev sibling
				long long			  numChildren;					// number of children
				TreeNode* firstChild;					// first child
				TreeNode* lastChild;					// last child
			};
			typedef TreeNode _iterType;

			_iterType* InitNode(_iterType* p) {
				p->key = {};
				p->object = nullptr;
				p->parent = nullptr;
				p->next = nullptr;
				p->prev = nullptr;
				p->numChildren = 0;
				p->firstChild = nullptr;
				p->lastChild = nullptr;
				return p;
			};
				
		private:
			long long
				Num;
			_iterType
				* root,
				* first,
				* last;
			SingleThreadedAllocator<objType, maxChildrenPerNode>
				objAllocator;
			SingleThreadedAllocator<_iterType, maxChildrenPerNode>
				nodeAllocator;

		public:
			BalancedTree& operator=(const BalancedTree& obj) {
				Clear(); // empty out whatever this container had 
				for (auto* x = obj.GetFirst(); x != nullptr; x = obj.GetNextLeaf(x)) {
					Add(*x->object, x->key, false);
				}

				return *this;
			};
			bool operator==(const BalancedTree& obj) {
				return GetFirst() == obj.GetFirst() && GetLast() == obj.GetLast();
			};
			bool operator!=(const BalancedTree& obj) { return !operator==(obj); };

			BalancedTree() : Num(0), root(nullptr), first(nullptr), last(nullptr), objAllocator(), nodeAllocator() {
				static_assert(maxChildrenPerNode >= 4);
				Init();
			};
			BalancedTree(int toReserve) :
				Num(0),
				root(nullptr),
				first(nullptr),
				last(nullptr),
				objAllocator(toReserve),
				nodeAllocator(toReserve * 1.25)
			{
				static_assert(maxChildrenPerNode >= 4);
				Init();
			};
			~BalancedTree() {
				// Clear(true);
			};

			void									Reserve(long long num) {
				objAllocator.Reserve(num);
				nodeAllocator.Reserve(num * 1.25); // approximately 25% more for 'overage'
			};

			_iterType* Add(objType const& object, keyType const& key, bool addUnique = true, bool* AlreadyExisted = nullptr) {
				_iterType* node, * child, * newNode; objType* OBJ;

				if (root == nullptr) {
					root = AllocNode();
				}

				// check that the key does not already exist		
				if (addUnique) {
					node = NodeFind(key);
					if (node && node->object) {
						*node->object = const_cast<objType&>(object);
						if (AlreadyExisted) *AlreadyExisted = true;
						return CheckLastNode(CheckFirstNode(node));
					}
				}

				if (AlreadyExisted) *AlreadyExisted = false;

				if (root->numChildren >= maxChildrenPerNode) {
					newNode = AllocNode();
					newNode->key = root->key;
					newNode->firstChild = root;
					newNode->lastChild = root;
					newNode->numChildren = 1;
					root->parent = newNode;
					SplitNode(root);
					root = newNode;
				}

				newNode = AllocNode();
				newNode->key = key;

				OBJ = nullptr;
				{
					OBJ = objAllocator.Alloc();
					*OBJ = const_cast<objType&>(object);
					Num++;
				}

				newNode->object = OBJ;

				for (node = root; node->firstChild != nullptr; node = child) {

					if (key > node->key) {
						node->key = key;
					}

					// find the first child with a key larger equal to the key of the new node
					for (child = node->firstChild; child->next; child = child->next) {
						if (key <= child->key) {
							break;
						}
					}

					if (child->object) {

						if (key <= child->key) {
							// insert new node before child
							if (child->prev) {
								child->prev->next = newNode;
							}
							else {
								node->firstChild = newNode;
							}
							newNode->prev = child->prev;
							newNode->next = child;
							child->prev = newNode;
						}
						else {
							// insert new node after child
							if (child->next) {
								child->next->prev = newNode;
							}
							else {
								node->lastChild = newNode;
							}
							newNode->prev = child;
							newNode->next = child->next;
							child->next = newNode;
						}

						newNode->parent = node;
						node->numChildren++;

						return CheckLastNode(CheckFirstNode(newNode));
					}

					// make sure the child has room to store another node
					if (child->numChildren >= maxChildrenPerNode) {
						SplitNode(child);
						if (key <= child->prev->key) {
							child = child->prev;
						}
					}
				}

				// we only end up here if the root node is empty
				newNode->parent = root;
				root->key = key;
				root->firstChild = newNode;
				root->lastChild = newNode;
				root->numChildren++;

				return CheckLastNode(CheckFirstNode(newNode));
			};

			void									Remove(_iterType* node) {
				if (!node) return;

				if (first == node) {
					first = this->GetNextLeaf(node);
				}

				if (last == node) {
					last = this->GetPrevLeaf(node);
				}

				_iterType* parent, * oldRoot;

				// unlink the node from it's parent
				if (node->prev) {
					node->prev->next = node->next;
				}
				else {
					node->parent->firstChild = node->next;
				}
				if (node->next) {
					node->next->prev = node->prev;
				}
				else {
					node->parent->lastChild = node->prev;
				}
				node->parent->numChildren--;

				// make sure there are no parent nodes with a single child
				for (parent = node->parent; parent != root && parent->numChildren <= 1; parent = parent->parent) {

					if (parent->next) {
						parent = MergeNodes(parent, parent->next);
					}
					else if (parent->prev) {
						parent = MergeNodes(parent->prev, parent);
					}

					// a parent may not use a key higher than the key of it's last child
					if (parent->key > parent->lastChild->key) {
						parent->key = parent->lastChild->key;
					}

					if (parent->numChildren > maxChildrenPerNode) {
						SplitNode(parent);
						break;
					}
				}
				for (; parent != nullptr && parent->lastChild != nullptr; parent = parent->parent) {
					// a parent may not use a key higher than the key of it's last child
					if (parent->key > parent->lastChild->key) {
						parent->key = parent->lastChild->key;
					}
				}

				// free the node
				FreeNode(node);

				// remove the root node if it has a single internal node as child
				if (root->numChildren == 1 && root->firstChild->object == nullptr) {
					oldRoot = root;
					root->firstChild->parent = nullptr;
					root = root->firstChild;
					FreeNode(oldRoot);
				}
			};				// remove an object node from the tree
			void									Clear(bool destroyAllocator = false) {
				// remove all
				nodeAllocator.Clear(destroyAllocator);
				objAllocator.Clear(destroyAllocator);
				root = nullptr;
				first = nullptr;
				last = nullptr;
				Num = 0;
				if (!destroyAllocator) Init();
			};
			_iterType* NodeFindByIndex(int index) const {
				if (index <= 0) return first;
				else if (index >= (Num - 1)) return last;
				else return NodeFindByIndex(index, root);
			};
			_iterType* NodeFind(keyType  const& key) const {
				return NodeFind(key, root);
			};								// find an object using the given key;
			_iterType* NodeFindSmallestLargerEqual(keyType const& key) const {
				return NodeFindSmallestLargerEqual(key, root);
			};			// find an object with the smallest key larger equal the given key;
			_iterType* NodeFindLargestSmallerEqual(keyType const& key) const {
				return NodeFindLargestSmallerEqual(key, root);
			};			// find an object with the largest key smaller equal the given key;

			static _iterType* NodeFind(keyType  const& key, _iterType* root) {
				_iterType* node = NodeFindLargestSmallerEqual(key, root);
				if (node && node->object && node->key == key) return node;
				return nullptr;
			};								// find an object using the given key;
			static _iterType* NodeFindByIndex(int index, _iterType* Root) {
				int startIndex{ 0 };

				if (Root == nullptr) {
					return nullptr;
				}

				while (Root) {
					if (index == startIndex && Root->object) { return Root; }

					if (startIndex <= index && (startIndex + Root->numChildren) > index) {
						// one of my children has this index				
						Root = Root->firstChild;
					}
					else {
						// one of my neighbors has this index				
						if (Root->object) ++startIndex;
						else startIndex += Root->numChildren;

						Root = Root->next;
					}
				}

				return Root;
			};			// find an object with the largest key smaller equal the given key;
			static _iterType* NodeFindSmallestLargerEqual(keyType const& key, _iterType* Root) {
				_iterType* node, * smaller;

				if (Root == nullptr) {
					return nullptr;
				}

				smaller = nullptr;
				for (node = Root->lastChild; node != nullptr; node = node->lastChild) {
					while (node->prev) {
						if (node->key <= key) {
							if (!smaller) {
								smaller = GetPrevLeaf(Root);
							}
							break;
						}
						smaller = node;
						node = node->prev;
					}
					if (node->object) {
						if (node->key >= key) {
							break;
						}
						else if (smaller == nullptr) {
							return nullptr;
						}
						else {
							node = smaller;
							if (node->object) {
								break;
							}
						}
					}
				}

				return node;
			};			// find an object with the smallest key larger equal the given key;
			static _iterType* NodeFindLargestSmallerEqual(keyType const& key, _iterType* Root) {
				_iterType* node, * smaller;

				if (Root == nullptr) {
					return nullptr;
				}

				smaller = nullptr;
				for (node = Root->firstChild; node != nullptr; node = node->firstChild) {
					while (node->next) {
						if (node->key >= key) {
							if (!smaller) {
								smaller = GetNextLeaf(Root);
							}
							break;
						}
						smaller = node;
						node = node->next;
					}
					if (node->object) {
						if (node->key <= key) {
							break;
						}
						else if (smaller == nullptr) {
							return nullptr;
						}
						else {
							node = smaller;
							if (node->object) {
								break;
							}
						}
					}
				}
				return node;
			};			// find an object with the largest key smaller equal the given key;
			objType* Find(keyType  const& key) const {
				_iterType* node = NodeFind(key, root);
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};									// find an object using the given key;
			objType* FindSmallestLargerEqual(keyType const& key) const {
				_iterType* node = NodeFindSmallestLargerEqual(key, root);
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};				// find an object with the smallest key larger equal the given key;
			objType* FindLargestSmallerEqual(keyType const& key) const {
				_iterType* node = NodeFindLargestSmallerEqual(key, root);
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};				// find an object with the largest key smaller equal the given key;

			_iterType* GetFirst() const { return first; };
			_iterType* GetLast() const { return last; };
			_iterType* GetRoot() const { return root; };
			long long								GetNodeCount() const {
				return Num;
			};										// returns the total number of nodes in the tree;
			long long								GetReservedCount() const {
				return objAllocator.GetTotalCount();  // .Num(); //  
			};
			static _iterType* GetNext(_iterType* node) {
				if (node) {
					if (node->firstChild) {
						node = node->firstChild;
					}
					else {
						while (node && node->next == nullptr) {
							node = node->parent;
						}
					}
				}
				return node;
			};		// goes through all nodes of the tree;

		public:
			static _iterType* GetNextLeaf(_iterType* node) {
				if (node) {
					if (node->firstChild) {
						while (node->firstChild) {
							node = node->firstChild;
						}
					}
					else {
						while (node && !node->next) {
							node = node->parent;
						}
						if (node) {
							node = node->next;
							while (node->firstChild) {
								node = node->firstChild;
							}
						}
						else {
							node = nullptr;
						}
					}
				}
				return node;
			};	// goes through all leaf nodes of the tree;
			static _iterType* GetPrevLeaf(_iterType* node) {
				if (!node) return nullptr;
				if (node->lastChild) {
					while (node->lastChild) {
						node = node->lastChild;
					}
					return node;
				}
				else {
					while (node && node->prev == nullptr) {
						node = node->parent;
					}
					if (node) {
						node = node->prev;
						while (node->lastChild) {
							node = node->lastChild;
						}
						return node;
					}
					else {
						return nullptr;
					}
				}
			};	// goes through all leaf nodes of the tree;

		private:
			_iterType* CheckFirstNode(_iterType* newNode) {
				if (newNode && first) {
					if (newNode->key < first->key) {
						first = newNode;
					}
				}
				else {
					first = newNode;
				}
				return newNode;
			};
			_iterType* CheckLastNode(_iterType* newNode) {
				if (newNode && last) {
					if (newNode->key > last->key) {
						last = newNode;
					}
				}
				else {
					last = newNode;
				}
				return newNode;
			};
			void									Init() {
				root = AllocNode();
				{ // helps init the objAllocator
					// auto x = objAllocator.Alloc();
					// objAllocator.Free(x);
				}
			};
			void									Shutdown() {
				nodeAllocator.Clear();

				objAllocator.Clear();
				root = nullptr;
				first = nullptr;
				last = nullptr;
				Num = 0;
			};
			_iterType* AllocNode() {
				_iterType* node;

				node = nodeAllocator.Alloc();
				return InitNode(node);

				//node->key = 0;
				//node->parent = nullptr;
				//node->next = nullptr;
				//node->prev = nullptr;
				//node->numChildren = 0;
				//node->firstChild = nullptr;
				//node->lastChild = nullptr;
				//node->object = nullptr;

				//return node;
			};
			void									FreeNode(_iterType* node) {
				if (node && node->object) {
					objAllocator.Free(node->object);  // RemoveFast(node->object); // 
					Num--;
				}
				nodeAllocator.Free(node); // RemoveFast(node); //  
			};
			void									SplitNode(_iterType* node) {
				long long i;
				_iterType* child, * newNode;

				// allocate a new node
				newNode = AllocNode();
				newNode->parent = node->parent;

				// divide the children over the two nodes
				child = node->firstChild;
				child->parent = newNode;
				for (i = 3; i < node->numChildren; i += 2) {
					child = child->next;
					child->parent = newNode;
				}

				newNode->key = child->key;
				newNode->numChildren = node->numChildren / 2;
				newNode->firstChild = node->firstChild;
				newNode->lastChild = child;

				node->numChildren -= newNode->numChildren;
				node->firstChild = child->next;

				child->next->prev = nullptr;
				child->next = nullptr;

				// add the new child to the parent before the split node
				assert(node->parent->numChildren < maxChildrenPerNode);

				if (node->prev) {
					node->prev->next = newNode;
				}
				else {
					node->parent->firstChild = newNode;
				}
				newNode->prev = node->prev;
				newNode->next = node;
				node->prev = newNode;

				node->parent->numChildren++;
			};
			_iterType* MergeNodes(_iterType* node1, _iterType* node2) {
				_iterType* child;

				assert(node1->parent == node2->parent);
				assert(node1->next == node2 && node2->prev == node1);
				assert(node1->object == nullptr && node2->object == nullptr);
				assert(node1->numChildren >= 1 && node2->numChildren >= 1);

				for (child = node1->firstChild; child->next; child = child->next) {
					child->parent = node2;
				}
				child->parent = node2;
				child->next = node2->firstChild;
				node2->firstChild->prev = child;
				node2->firstChild = node1->firstChild;
				node2->numChildren += node1->numChildren;

				// unlink the first node from the parent
				if (node1->prev) {
					node1->prev->next = node2;
				}
				else {
					node1->parent->firstChild = node2;
				}
				node2->prev = node1->prev;
				node2->parent->numChildren--;

				FreeNode(node1);

				return node2;
			};

		};
	};

	namespace details {
		// fast, thread-safe sorted map. Fast for single-threaded inserts, and fast for multi-threaded reading. Slower for simultaneous reading/inserting.
		template<class KeyType, class ValueType> class flat_map {
			friend class it_state;
		protected:
			mutable BalancedTree<ValueType, KeyType, 10> 
				tree;
			mutable fast_shared_mutex
				lock;
			mutable size_t
				count;
		public:
			flat_map()
				: tree{}
				, lock{}
				, count{ 0 }
			{};
			flat_map(flat_map const& rhs)
				: tree{}
				, lock{}
				, count{ 0 }
			{
				std::shared_lock locked{ rhs.lock };
				tree = rhs.tree;
				InterlockedExchangeNoFence(&count, rhs.count);
			};
			flat_map(flat_map&& rhs)
				: tree{ std::move(rhs.tree) }
				, lock{}
				, count{ rhs.count }
			{};
			flat_map& operator=(flat_map const& rhs) {
				if (this == &rhs) return *this;
				std::unique_lock locked1{ lock };
				std::shared_lock locked2{ rhs.lock };
				tree = rhs.tree;
				InterlockedExchangeNoFence(&count, rhs.count);
				return *this;
			};
			flat_map& operator=(flat_map&& rhs) {
				if (this == &rhs) return *this;
				std::unique_lock locked1{ lock };
				std::shared_lock locked2{ rhs.lock };
				tree = rhs.tree;
				InterlockedExchangeNoFence(&count, rhs.count);
				return *this;
			};
			~flat_map() = default;

			size_t const& size() const {
				return count;
			};			
			std::pair<std::reference_wrapper<KeyType>, std::reference_wrapper<ValueType>>				
				insert(const KeyType& time, ValueType&& value) {
				std::unique_lock locked{ lock };
				bool alreadyExisted = false;
				auto* p = tree.Add(std::move(value), time, true, &alreadyExisted);
				if (!alreadyExisted)
					InterlockedIncrementNoFence(&count);
				return std::pair<std::reference_wrapper<KeyType>, std::reference_wrapper<ValueType>>{ std::ref(p->key), std::ref(*p->object) };
			};
			std::pair<std::reference_wrapper<KeyType>, std::reference_wrapper<ValueType>>
				emplace(const KeyType& time, ValueType&& value) {
				std::unique_lock locked{ lock };
				bool alreadyExisted = false; 				
				auto* p = tree.Add(std::move(value), time, true, &alreadyExisted);
				if (!alreadyExisted)
					InterlockedIncrementNoFence(&count);
				return std::pair<std::reference_wrapper<KeyType>, std::reference_wrapper<ValueType>>{ std::ref(p->key), std::ref(*p->object) };
			};

			ValueType&
				at(const KeyType& time) const {
				std::shared_lock locked{ lock };
				if (ValueType* p = tree.Find(time)) {
					return *p;
				}
				else {
					throw std::range_error("Could not find " + GoodLang::ToString(time));
				}
			};
			ValueType*
				try_at(const KeyType& time, long& hint) const {
				std::shared_lock locked{ lock };
				if (ValueType* p = tree.Find(time)) {
					return p;
				}
				else {
					return nullptr;
				}
			};
			template <typename Func> bool
				do_at_beginning(Func const& func) const {
				std::shared_lock locked{ lock };
				if (auto* p = tree.GetFirst()) {
					func(p->key, *p->object);
					return true;
				}
				return false;
			};
			template <typename Func> bool
				do_at_end(Func const& func) const {
				std::shared_lock locked{ lock };
				if (auto* p = tree.GetLast()) {
					func(p->key, *p->object);
					return true;
				}
				return false;
			};
			bool 
				pop_front() const {
				std::unique_lock locked{ lock };
				if (auto* p = tree.GetFirst()) {
					InterlockedDecrementNoFence(&count);
					tree.Remove(p);					
					return true;
				}
				return false;
			};
			bool
				pop_back() const {
				std::unique_lock locked{ lock };
				if (auto* p = tree.GetLast()) {
					InterlockedDecrementNoFence(&count);
					tree.Remove(p);					
					return true;
				}
				return false;
			};
			ValueType&
				operator[](const KeyType& time) {
				lock.lock_shared();
				if (ValueType* p = tree.Find(time)) {
					lock.unlock_shared();
					return *p;
				}
				else {
					(void)lock.upgrade_lock();
					if (auto* p = tree.Add({}, time, true)) {
						lock.unlock();
						return *p->object;
					}
					else {
						lock.unlock();
						throw std::range_error("Could not find " + GoodLang::ToString(time));
					}
				}
			};
			template <typename Func> ValueType&
				get_or_make(const KeyType& time, Func const& func, bool* ExistedAlready = nullptr) {
				lock.lock_shared();
				if (ValueType* p = tree.Find(time)) {
					lock.unlock_shared();
					return *p;
				}
				else {
					bool ExistedAlready = false;
					(void)lock.upgrade_lock();
					if (auto* p = tree.Add(func(), time, true, &ExistedAlready)) {
						lock.unlock();
						if (!ExistedAlready) InterlockedIncrementNoFence(&count);
						return *p->object;
					}
					else {
						lock.unlock();
						throw std::range_error("Could not find " + GoodLang::ToString(time));
					}
				}
			};
			bool
				erase(const KeyType& time, ValueType* out = nullptr) {
				lock.lock_shared();
				if (auto* p = tree.NodeFind(time)) {					
					InterlockedDecrementNoFence(&count);
					if (out) *out = *p->object;
					(void)lock.upgrade_lock();
					tree.Remove(p);
					lock.unlock();
					return true;
				}
				lock.unlock_shared();
				return false;			
			};
			void
				clear() {
				std::unique_lock locked{ lock };
				InterlockedExchangeNoFence(&count, 0);
				tree.Clear();
			};

		private:
			class it_state {
			public:
				using thisType = flat_map;
				using value_type = std::pair<KeyType*, ValueType*>;
				using iterator_category = std::forward_iterator_tag;
				using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

				// data
				mutable typename BalancedTree<ValueType, KeyType, 10> ::_iterType*
					_ptr{};
				std::shared_ptr<std::shared_lock<fast_shared_mutex>>
					lifetime{ nullptr };
				mutable value_type
					_out;

				// functions
				void Initialize(thisType* ref) {};
				void ToBeginning(thisType* ref) {
					lifetime = std::make_shared<std::shared_lock<fast_shared_mutex>>(ref->lock);
					_ptr = ref->tree.GetFirst();
				};
				void ToEnd(thisType* ref) {
					_ptr = nullptr;
				};
				void Next(thisType* ref) {
					this->_ptr = ref->tree.GetNextLeaf(this->_ptr);
				};
				void Prev(thisType* ref) {
					this->_ptr = ref->tree.GetPrevLeaf(this->_ptr);
				};
				value_type& Get(thisType* ref) const {
					_out.first = &_ptr->key;
					_out.second = _ptr->object;
					return _out;
				};
				bool operator==(it_state const& rhs) const {
					return _ptr == rhs._ptr;
				};
				difference_type Distance(it_state const& other) const { 
					return _ptr - other._ptr; 
				};
			};

		public:
			SETUP_ITERATOR(flat_map, it_state);
			iterator find(const KeyType& _Keyval) const {
				std::shared_lock locked{ lock };

				auto iter = this->end();
				if (auto* p = this->tree.NodeFind(_Keyval)) {
					iter.state._ptr = p;
				}
				return iter;
			};
			std::string ToString() const {
				std::string out;
				
				return std::string("[") + out + std::string("]");
			};
			std::vector< Impl::NodeCache > GetChildren() const {
				std::vector< Impl::NodeCache > out;
				
				return out;
			};
		};
	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< details::flat_map<Args...> >, details::flat_map<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<details::flat_map<Args...>>, details::flat_map<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
	};


	/// <summary>
	/// Splines are wrappers for a dictionary of 2D values, which can interpolate between the values smoothly based on your position. 
	/// </summary>
	namespace spline {
		struct linear {};
		struct left_snap {};
		struct right_snap {};
		struct spline {};

		/// <summary>
		/// thread-safe concurrent spline, interpolating between values
		/// </summary>
		/// <typeparam name="Key"></typeparam>
		/// <typeparam name="Value"></typeparam>
		template <typename Key, typename Value> class Spline {
		protected:
			Map<Key, Value> data; // underlying data container

		public:
			Spline() = default;
			Spline(Spline const&) = default;
			Spline(Spline&&) = default;
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
				return interpolate(x, spline::spline{});
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
			template<typename interpType = spline> auto GetTimeSeries(Key const& start, Key const& end, Key const& step) const {
				return CustomizedSequence<std::pair<Key, Value>, Key>(
					std::function([this](Key const& x) -> std::pair<Key, Value> {
						return std::pair<Key, Value>{ x, this->interpolate(x, interpType{}) };
					})
					, start
					, end
					, step
				);
			};

			std::string ToString() const { return GoodLang::ToString(data); };
			std::vector< Impl::NodeCache > GetChildren() const { return { GoodLang::GetChildren(data) }; };
		};

		/// <summary>
		/// Special type of spline that crosses through its control points utilizing the centripetal catmull-rom algorithm (alpha = 0.5). Generally a good spline for natural data such as water consumption flowrates, tank levels, air wind speed, etc. 
		/// </summary>
		/// <typeparam name="Key"></typeparam>
		/// <typeparam name="Value"></typeparam>
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
				// if all the data is available, do the catmull-rom spline
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
					out = ::fma(Y0, ((2.0 - s) * s - 1.0) * s * 0.5,                   // -0.5f s * s * s + s * s - 0.5f * s
						::fma(Y1, (((3.0 * s - 5.0) * s) * s + 2.0) * 0.5,            // 1.5f * s * s * s - 2.5f * s * s + 1.0f
							::fma(Y2, ((-3.0 * s + 4.0) * s + 1.0) * s * 0.5,        // -1.5f * s * s * s - 2.0f * s * s + 0.5f s
								::fma(Y3, ((s - 1.0) * s * s) * 0.5,                // 0.5f * s * s * s - 0.5f * s * s
									0.0))));

					return out;
				}
				// otherwise, default to the linear spline
				else {
					return this->interpolate(x, linear{});
				}
			};

		};

	};
	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< spline::Spline<Args...> >, spline::Spline<Args...> const& r, std::string& out) {
			out = r.ToString();
		};
		template <typename... Args> __forceinline void ToString(Tag< spline::CatmullRomSpline<Args...> >, spline::CatmullRomSpline<Args...> const& r, std::string& out) {
			out = r.ToString();
		};

		template <typename... Args> __forceinline void GetChildren(Tag<spline::Spline<Args...>>, spline::Spline<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};
		template <typename... Args> __forceinline void GetChildren(Tag<spline::CatmullRomSpline<Args...>>, spline::CatmullRomSpline<Args...> const& r, std::vector< NodeCache >& out) {
			out = r.GetChildren();
		};

	};


};
