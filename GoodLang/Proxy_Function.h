#pragma once

#include "Foundation.h"
#include "Any.h"
#include <optional>

// Type_Conversion_Base, its impl's, & TypeConverter wrapper
namespace GoodLang {
	namespace details {
		// Tuning parameter. Should be larger than the slowest conversion time. Large values encourages fewer conversions. Smaller values encourages faster conversions.
		static constexpr auto TypeConversionBaselineCost = 1000.0;
		static constexpr auto TypeConversionWorstCaseCost = 1000000000000.0;
		
		class Type_Conversion_Base {
		public:
			// Converts From -> To, and places the result into the "From" container. Useful for faster conversions.
			virtual void convert_in_place(Any& from) const = 0;
			// From -> To
			virtual Any convert(const Any& from) const = 0;
			// To -> From
			virtual Any convert_down(const Any& to) const = 0;

			// returns the actual time (in nanoseconds) to perform the conversion
			virtual double cost() const noexcept { return 0; };

			// to type
			const std::weak_ptr<Type_Info>& to() const noexcept { return m_to; }

			// from type
			const std::weak_ptr<Type_Info>& from() const noexcept { return m_from; }

			// is bidirectional?
			virtual bool bidir() const noexcept { return true; }

			// is polymorphic conversion?
			virtual bool polymorphic() const noexcept { return false; }

			virtual std::string print() const noexcept { return std::string(" ... ") + this->to().lock()->name(); };

			virtual ~Type_Conversion_Base() = default;

			virtual bool IsDaisyChained() const { return false; };

			virtual size_t NumConversions() const { return 1; }
		protected:
			Type_Conversion_Base() : m_to{}, m_from{} {}
			Type_Conversion_Base(std::weak_ptr<Type_Info> t_to, std::weak_ptr<Type_Info> t_from) : m_to(t_to), m_from(t_from) {}

		protected:
			std::weak_ptr<Type_Info> m_to;
			std::weak_ptr<Type_Info> m_from;
		};

		template<class Callable>
		class Custom_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			typedef typename utilities::function_traits< typename decltype(std::function(std::declval<Callable>())) >::result_type ReturnType;
			typedef typename std::tuple_element_t<0, typename utilities::function_traits< typename decltype(std::function(std::declval<Callable>())) >::arguments> InputType;

		public:
			Custom_Type_Conversion_Impl(Callable t_func)
				: Type_Conversion_Base(
					user_type_shared<ReturnType>(),
					user_type_shared<InputType>()
				)
				, m_func(std::move(t_func))
				, m_cost(std::nullopt)
			{};
			Custom_Type_Conversion_Impl(Callable t_func, std::weak_ptr<Type_Info> inboundType, std::weak_ptr<Type_Info> outboundType, std::optional<double> Cost = std::nullopt)
				: Type_Conversion_Base(
					outboundType,
					inboundType
				)
				, m_func(std::move(t_func))
				, m_cost(std::move(Cost))
			{};

			// To -> From
			Any convert_down(const Any&) const override {
				throw std::runtime_error("Custom_Type_Conversion_Impl is not bidirectional.");
			};

			// From -> To
			void convert_in_place(Any& t_from) const override {
				if constexpr (std::is_convertible<decltype(t_from), InputType>::value) {
					t_from = m_func(t_from);
				}
				else {
					t_from = m_func(t_from.cast());
				}
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				if constexpr (std::is_convertible<decltype(t_from), InputType>::value) {
					return m_func(t_from);
				}
				else {
					return m_func(t_from.cast());
				}
			};

			bool bidir() const noexcept override { return false; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				if (m_cost.has_value()) {
					if (m_cost.value() == 0) {
						return 0;// m_cost.value();
					}
					else {
						return TypeConversionBaselineCost;// + m_cost.value();
					}
				}
				else {
					//static double actualCost{ -1 };
					//static std::decay_t<InputType> inputObj{};
					//if (actualCost < 0) {
					//	double temp{ 0 };
					//	for (int i = 0; i < 10; i++) {
					//		auto startT = clock_ns();
					//		(void)(m_func(inputObj));
					//		temp += (double)(clock_ns() - startT) / 100.0;
					//	}
					//	actualCost = TypeConversionBaselineCost + temp / 10.0;
					//}
					//return actualCost;

					return TypeConversionBaselineCost;
				}
			};

			void SetTemplateTypes(std::weak_ptr<Type_Info> const& FromType, std::weak_ptr<Type_Info> const& ToType) {
				if (auto from = this->m_from.lock()) {
					if (from->is_any()) {
						this->m_from = FromType;
					}
				}

				if (auto to = this->m_to.lock()) {
					if (to->is_any()) {
						this->m_to = ToType;
					}
				}
			};

		private:
			Callable m_func;
			std::optional<double> m_cost;
		};

		class DaisyChained_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			DaisyChained_Type_Conversion_Impl(std::vector<std::shared_ptr<Type_Conversion_Base>>&& t_converters);

			// To -> From
			Any convert_down(const Any&) const override;

			// From -> To
			void convert_in_place(Any& t_from) const override;

			// From -> To
			Any convert(const Any& t_from) const override;

			bool bidir() const noexcept override;

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override;

			virtual std::string print() const noexcept override;
			virtual bool IsDaisyChained() const override;
			virtual size_t NumConversions() const override;
		private:
			std::vector<std::shared_ptr<Type_Conversion_Base>> m_converters;
			double m_cost;
		};

		namespace impl {
			template <class From, class To, class = void>
			struct is_explicitly_convertible_to_impl : std::false_type {};

			template <class From, class To>
			struct is_explicitly_convertible_to_impl<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>> : std::true_type {};

			template <class From, class To>
			struct is_explicitly_convertible_to : is_explicitly_convertible_to_impl<From, To> {};

			template <class From, class To>
			inline constexpr bool is_explicitly_convertible_to_v = is_explicitly_convertible_to<From, To>::value;
		};

		template<typename From, typename To>
		class Static_Type_Conversion_Impl : public Type_Conversion_Base {
		private:
			constexpr static bool is_bidir = impl::is_explicitly_convertible_to<To, From>::value;

		public:
			Static_Type_Conversion_Impl()
				: Type_Conversion_Base(user_type_shared<To>(), user_type_shared<From>())
			{};

			// To -> From
			Any convert_down(const Any& t_to) const override {
				if constexpr (is_bidir) {
					return (From)(t_to.cast<To const&>());
				}
				else {
					throw std::runtime_error("Static_Type_Conversion_Impl was not bidirectional.");
				}
			};

			// From -> To
			void convert_in_place(Any& t_from) const override {
				t_from = (To)(t_from.cast<From const&>());
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				return (To)(t_from.cast<From const&>());
			};

			bool bidir() const noexcept override { return is_bidir; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				//static double actualCost{ -1 };
				//static std::decay_t<From> inputObj{};
				//if (actualCost < 0) {
				//	double temp{ 0 };
				//	for (int i = 0; i < 10; i++) {
				//		auto startT = clock_ns();
				//		(void)((To)(inputObj));
				//		temp += (double)(clock_ns() - startT) / 100.0;
				//	}
				//	actualCost = TypeConversionBaselineCost + temp / 10.0;
				//}
				return TypeConversionBaselineCost;//  actualCost;
			};
		};

		template<typename ChildType, typename BaseType>
		class Dynamic_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			Dynamic_Type_Conversion_Impl()
				: Type_Conversion_Base(user_type_shared<BaseType>(), user_type_shared<ChildType>())
			{};

			// BaseType -> ChildType
			Any convert_down(const Any& t_to) const override {
				throw std::runtime_error("Dynamic_Type_Conversion_Impl is never bidirectional (Base -> Child). Only may cast from (Child -> Base).");
			};

			// ChildType -> BaseType
			void convert_in_place(Any& t_from) const override {
				std::shared_ptr<ChildType> ptr{ t_from.cast<std::shared_ptr<ChildType>>() };
				t_from = std::dynamic_pointer_cast<BaseType>(ptr);
			};

			// ChildType -> BaseType
			Any convert(const Any& t_from) const override {
				std::shared_ptr<ChildType> ptr{ t_from.cast<std::shared_ptr<ChildType>>() };
				return std::dynamic_pointer_cast<BaseType>(ptr);
			};

			bool bidir() const noexcept override { return false; }

			double cost() const noexcept override { return 0; /* Assumes that dynamic casts are free */ };
		};

		// Create a wrapped user-defined function to cast provided types. Must have one (and only one) argument in the function. Argument may be an Any and do wild stuff.
		template<class Callable> __forceinline std::shared_ptr<Type_Conversion_Base> MakeConversionFunc(Callable func) {
			typedef decltype(std::function(std::declval<Callable>())) CallableTypeAsStdFunc;
			typedef typename utilities::function_traits< CallableTypeAsStdFunc >::arguments CallableArguments;
			if constexpr (std::tuple_size_v< CallableArguments > != 1) {
				return nullptr;
			}
			else {
				typedef typename std::tuple_element_t<0, CallableArguments> From;
				typedef typename utilities::function_traits< CallableTypeAsStdFunc >::result_type To;
				constexpr static bool is_convertable = impl::is_explicitly_convertible_to<From, To>::value;
				constexpr static bool is_bidir_convertable = impl::is_explicitly_convertible_to<To, From>::value;
				constexpr static bool is_polymorphic = std::is_base_of<To, From>::value;

				return std::shared_ptr< Type_Conversion_Base >(new Custom_Type_Conversion_Impl(std::move(func)));
			}

		};

		// Create a function to cast from "From" to "To". Supports static or dynamic (polymorphic) casting. 
		template<typename From, typename To> __forceinline std::shared_ptr<Type_Conversion_Base> MakeConversionFunc() {
			constexpr static bool is_convertable = impl::is_explicitly_convertible_to<From, To>::value;
			constexpr static bool is_bidir_convertable = impl::is_explicitly_convertible_to<To, From>::value;
			constexpr static bool is_polymorphic = std::is_base_of<To, From>::value;

			if constexpr (is_polymorphic) {
				return std::shared_ptr<Type_Conversion_Base >(new Dynamic_Type_Conversion_Impl<From, To>());
			}
			else {
				if constexpr (is_convertable) {
					return std::shared_ptr< Type_Conversion_Base >(new Static_Type_Conversion_Impl<From, To>());
				}
				else {
					return std::shared_ptr< Type_Conversion_Base >(new Custom_Type_Conversion_Impl([](From const& i) -> To { return To(i); }));
				}
			}
		};
	};

	// Tree that manages a complex graph network of conversion opportunities. 
	// It's task is to organize those conversions, find the minimium or best conversion paths, and then cache the results. 
	// Best, most thread-safe use is to pre-populate the tree with converters before use. 
	// Performance-wise, it caches all potential conversions for each new type all at once, so beware small hick-ups in timing due to this. 
	// Can be fixed by pre-fetching all (or most) of the conversions you plan to use. 
	class TypeConverter {
	private:
		// shared lock that prioritizes uncontested shared access and uncontested write access. Contested access prioritizes readers, and fairly orders writers.
		class UncopiableSharedLock {
		public:
			UncopiableSharedLock() = default;
			UncopiableSharedLock(UncopiableSharedLock const&) {};
			UncopiableSharedLock(UncopiableSharedLock&&) {};
			UncopiableSharedLock& operator=(UncopiableSharedLock const&) {};
			UncopiableSharedLock& operator=(UncopiableSharedLock&&) {};
			~UncopiableSharedLock() = default;

			// Exclusive ownership
			void lock() {
				p_mut.lock();
			};
			//			bool try_lock() {
			//#ifndef UncopiableSharedLockAsSharedMutex
			//				long currentWriteCount = writeCount.Increment();
			//				if (currentWriteCount != 1) {
			//					writeCount.Decrement();
			//					return false;
			//				}
			//				else {
			//					// must now wait for the actual lock.
			//					auto my_ticket = write_ticket.fetch_add(1, std::memory_order::memory_order_relaxed);
			//					int spin = 0;
			//					while (my_ticket != write_serving.load(std::memory_order::memory_order_acquire)) {
			//						if (spin < 10)
			//						{
			//							_mm_pause(); // SMT thread swap can occur here
			//						}
			//						else
			//						{
			//							std::this_thread::yield(); // OS thread swap can occur here. It is important to keep it as fallback, to avoid any chance of lockup by busy wait
			//						}
			//						spin++;
			//					}
			//				}
			//
			//				// it is my turn to get the write access. Are the readers done? 
			//				while (readCount.GetValue() != 0) { std::this_thread::yield(); }
			//
			//				return true;
			//#else
			//				return p_mut.try_lock();
			//#endif
			//			};
			void unlock() {
				p_mut.unlock();
			};

			// Shared ownership
			void lock_shared() {
				p_mut.lock_shared();
			};
			//			bool try_lock_shared() {
			//#ifndef UncopiableSharedLockAsSharedMutex
			//				readCount.Increment();
			//				// we are the final read -- go ahead and wait for the write lock before we go
			//				while (writeCount.GetValue() != 0) {
			//					readCount.Decrement();
			//					return false;
			//				}
			//				return true;
			//#else
			//				return p_mut.try_lock_shared();
			//#endif
			//			};
			void unlock_shared() {
				p_mut.unlock_shared();
			};

		private:
			std::shared_mutex p_mut;
		};

	public:
		typedef std::shared_ptr< details::Type_Conversion_Base > TypeConverterFunc;

		TypeConverter() = default;
		TypeConverter(TypeConverter const& rhs) {
			AllConversions = rhs.AllConversions;
		};
		TypeConverter(TypeConverter&& rhs) {
			AllConversions = std::move(rhs.AllConversions);
		};
		TypeConverter& operator=(TypeConverter const& rhs) {
			AllConversions = rhs.AllConversions;
		};
		TypeConverter& operator=(TypeConverter&& rhs) {
			AllConversions = std::move(rhs.AllConversions);
		};
		~TypeConverter() = default;

	private:
		// All conversions, will include "real" and cached conversions.
		typedef concurrency::concurrent_unordered_map< std::shared_ptr<Type_Info>, // From
			concurrency::concurrent_unordered_map< std::shared_ptr<Type_Info>, // To
			std::pair<UncopiableSharedLock, TypeConverterFunc> // Function (lock allows for overwriting functions)
			>
		> conversionTreeType;
		conversionTreeType AllConversions;
		UncopiableSharedLock AllConversionsLock;

	public:
		std::string print();

	private:
		// may return nullptr
		TypeConverterFunc GetExistingConverter(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To);
		// may return nullptr if it could not be built
		TypeConverterFunc GetOrBuildConverter(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To, bool forceBuild = false);

	private:
		// Base -> const Base
		// Base -> Base&
		// Base -> const Base&
		// const Base -> const Base&
		// Base& -> const Base&
		void AddDefaultConverters(std::weak_ptr<Type_Info> const& Type);

	public:
		// if does not exists, will add it. If exists, overwrites if the converter is better-performance.
		template<typename From_t, typename To_t> void AddConverter() {
			// forward
			if (1) {
				auto func = details::MakeConversionFunc < const typename std::decay_t<From_t>&, typename std::decay_t<To_t> >();
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto& pair = AllConversions[From.lock()][To.lock()];
						auto locked{ std::unique_lock(pair.first) };
						pair.second = func;
					}
					AddDefaultConverters(From);
				}
			}
			// backwards (may fail, which is OK)
			if (1) {
				auto func = details::MakeConversionFunc<const typename std::decay_t<To_t>&, typename std::decay_t<From_t>>();
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto& pair = AllConversions[From.lock()][To.lock()];
						auto locked{ std::unique_lock(pair.first) };
						pair.second = func;
					}
					AddDefaultConverters(From);
				}
			}

			// Add "contructor" class for const Type& -> Type, if possible
			if constexpr (std::is_copy_constructible<typename std::decay_t<From_t>>::value) {
				auto func = details::MakeConversionFunc([](const typename std::decay_t<From_t>& f) -> typename std::decay_t<From_t> { return f; });
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto& pair = AllConversions[From.lock()][To.lock()];
						auto locked{ std::unique_lock(pair.first) };
						pair.second = func;
					}
				}
			}
			if constexpr (std::is_copy_constructible<typename std::decay_t<To_t>>::value) {
				auto func = details::MakeConversionFunc([](const typename std::decay_t<To_t>& f) -> typename std::decay_t<To_t> { return f; });
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto& pair = AllConversions[From.lock()][To.lock()];
						auto locked{ std::unique_lock(pair.first) };
						pair.second = func;
					}
				}
			}

		};
		// if does not exists, will add it. If exists, overwrites if the converter is better-performance.
		template<class Callable> void AddConverter(Callable Func) {
			typedef decltype(std::function(std::declval<Callable>())) CallableTypeAsStdFunc;
			typedef typename utilities::function_traits< CallableTypeAsStdFunc >::arguments CallableArguments;
			if constexpr (std::tuple_size_v< CallableArguments > != 1) {
				return;
			}
			else {
				auto func = details::MakeConversionFunc(std::move(Func));
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto& pair = AllConversions[From.lock()][To.lock()];
						auto locked{ std::unique_lock(pair.first) };
						pair.second = func;
					}
					AddDefaultConverters(From);
					AddDefaultConverters(To);
				}
			}
		};
		// if does not exists, will add it. If exists, overwrites if the converter is better-performance.
		template<class Callable> void AddConverter(Callable Func, std::weak_ptr<Type_Info> const& FromType, std::weak_ptr<Type_Info> const& ToType) {
			typedef decltype(std::function(std::declval<Callable>())) CallableTypeAsStdFunc;
			typedef typename utilities::function_traits< CallableTypeAsStdFunc >::arguments CallableArguments;
			if constexpr (std::tuple_size_v< CallableArguments > != 1) {
				return;
			}
			else {
				typedef typename details::get_type<typename std::tuple_element_t<0, CallableArguments> >::type From_t;
				typedef typename details::get_type<typename utilities::function_traits< CallableTypeAsStdFunc >::result_type>::type To_t;

				auto fromTypePtr = user_type_shared< From_t >().lock();
				auto toTypePtr = user_type_shared< To_t >().lock();

				if (!fromTypePtr->is_any() && !toTypePtr->is_any()) { // may as well use the default converter
					AddConverter(Func);
				}
				else {
					std::shared_ptr<details::Type_Conversion_Base> func = details::MakeConversionFunc(std::move(Func));
					if (auto func_actual = std::dynamic_pointer_cast<details::Custom_Type_Conversion_Impl<Callable>>(func)) {
						func_actual->SetTemplateTypes(FromType, ToType);

						if (func) {
							auto& From = func->from();
							auto& To = func->to();
							if (1) {
								auto& pair = AllConversions[From.lock()][To.lock()];
								auto locked{ std::unique_lock(pair.first) };
								pair.second = func;
							}
							AddDefaultConverters(From);
							AddDefaultConverters(To);
						}
					}
					else {
						// something went wrong
					}
				}
			}
		};

		// Find or make converter to accomplish the request
		TypeConverterFunc FindConverter(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To, bool forceBuild = false);
		// Find or make converter to accomplish the request
		template<typename From_t, typename To_t> TypeConverterFunc FindConverter(bool forceBuild = false) {
			return FindConverter(user_type_shared<From_t>(), user_type_shared<To_t>(), forceBuild);
		};

		static Any Static_Convert(Any const& from, std::weak_ptr<Type_Info> const& To);

		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		Any Convert(Any const& from, std::shared_ptr<Type_Info> const& To);
		// will throw an error if the conversion was impossible.
		template<typename To_t> typename std::remove_reference_t<To_t> Convert(Any const& from) {
			static auto to_type{ user_type_shared<To_t>().lock() };
			if (to_type->is_any()) {
				return from.cast();
			}
			else if (auto f = FindConverter(from.Type().lock(), to_type)) {
				Any temp = f->convert(from);
				return temp.cast();
			}
			else if (from.IsTypeOf(to_type)) {
				return from.cast();
			}
			else
				throw exception::bad_any_cast(from.Type(), user_type_shared<To_t>(), __LINE__);
		};

		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		double ConversionCost(Any const& from, std::shared_ptr<Type_Info> const& To);
		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		double ConversionCost_Fast(Any const& from, std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To);
		// will throw an error if the conversion was impossible.
		template<typename To_t> double ConversionCost(Any const& from) {
			static auto to_type{ user_type_shared<To_t>().lock() };
			return ConversionCost(from, to_type);
		};

		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		bool Converts(Any const& from, std::shared_ptr<Type_Info> const& To);
		// will throw an error if the conversion was impossible.
		template<typename To_t> bool Converts(Any const& from) {
			return ConversionCost<To_t>(from) != std::numeric_limits<double>::max();
		};
	};
};

// FunctionSignature, FunctionArgs, & ParamTypes
namespace GoodLang {
	// A collection or list of parameter types. May be a list of types for input into a function, the argument types of the function, or a simple list of types.
	// The types are hashed together to generate a unique hash for this list that can be used to quickly compare them.
	class ParamTypes {
	public:
		static size_t CalculateHash();
		static size_t CalculateHash(std::vector<std::weak_ptr<Type_Info>> const& t_types);
		static size_t CalculateHash(std::vector<Any> const& params);

	public:
		ParamTypes()
			: uniquehash{ CalculateHash() }
			, m_types{ nullptr }
		{};
		ParamTypes(std::vector<std::weak_ptr<Type_Info>> const& t_types)
			: uniquehash{ CalculateHash(t_types) }
			, m_types(std::make_shared<std::vector<std::weak_ptr<Type_Info>>>(t_types))
		{
			for (auto& type : *m_types) if (auto ptr = type.lock()) if (ptr->is_any()/*MakeBase() == user_type_shared<Any>()*/) {
				isTemplate = true;
				break;
			}
		};
		ParamTypes(std::vector<std::weak_ptr<Type_Info>>&& t_types)
			: uniquehash{}
			, m_types(std::make_shared<std::vector<std::weak_ptr<Type_Info>>>(std::forward<std::vector<std::weak_ptr<Type_Info>>>(t_types)))
		{
			uniquehash = CalculateHash(*m_types);
			for (auto& type : *m_types) if (auto ptr = type.lock()) if (ptr->is_any()/*MakeBase() == user_type_shared<Any>()*/) {
				isTemplate = true;
				break;
			}
		};
		ParamTypes(std::vector<Any> const& params)
			: uniquehash{}
			, m_types(std::make_shared<std::vector<std::weak_ptr<Type_Info>>>(params.size(), std::weak_ptr<Type_Info>()))
		{
			for (int i = params.size() - 1; i >= 0; i--) m_types->at(i) = params[i].ActualType();
			uniquehash = CalculateHash(*m_types);
			for (auto& type : *m_types) if (auto ptr = type.lock()) if (ptr->is_any()/*MakeBase() == user_type_shared<Any>()*/) {
				isTemplate = true;
				break;
			}
		};
		ParamTypes(ParamTypes const&) = default;
		ParamTypes(ParamTypes&&) = default;
		ParamTypes& operator=(ParamTypes const&) = default;
		ParamTypes& operator=(ParamTypes&&) = default;
		~ParamTypes() = default;

		const std::weak_ptr<Type_Info>& operator[](const std::size_t t_i) const noexcept { return m_types->operator[](t_i); };
		std::weak_ptr<Type_Info>& operator[](const std::size_t t_i) noexcept { return m_types->operator[](t_i); };

		friend bool operator==(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash == b.uniquehash;
		};
		friend bool operator!=(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash != b.uniquehash;
		};
		friend bool operator>(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash > b.uniquehash;
		};
		friend bool operator<(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash < b.uniquehash;
		};
		friend bool operator>=(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash >= b.uniquehash;
		};
		friend bool operator<=(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash <= b.uniquehash;
		};

		auto begin() const noexcept {
			if (m_types) return m_types->begin();
			else return std::vector<std::weak_ptr<Type_Info>>::iterator();
		}
		auto end() const noexcept {
			if (m_types) return m_types->end();
			else return std::vector<std::weak_ptr<Type_Info>>::iterator();
		}
		size_t size() const noexcept {
			if (m_types) return m_types->size();
			else return 0;
		}
		bool empty() const noexcept { return size() == 0; }
		size_t hash() const noexcept { return uniquehash; };
		bool CanCast(ParamTypes const& to) const;
		bool IsTemplate() const { return isTemplate; };

	private:
		std::shared_ptr<std::vector<std::weak_ptr<Type_Info>>> m_types;
		size_t uniquehash;
		bool isTemplate{ false };
	};

	// A combination of ParamTypes (list of types) and variable names. Variable names do NOT impact the "uniqueness" of a list of function arguments. e.g;
	// { int:"a" } == { int:"b" }
	class FunctionArgs {
	private:
		static std::vector<std::string> DefaultVariableNames(size_t n);
		static std::vector<std::string> DefaultVariableNames(size_t n, std::vector<std::string> const& paramNames);

	public:
		FunctionArgs()
			: m_names{}
			, m_types{}
		{};
		FunctionArgs(ParamTypes const& t_types)
			: m_names{ DefaultVariableNames(t_types.size()) }
			, m_types{ t_types }
		{};
		FunctionArgs(ParamTypes const& t_types, std::vector<std::string> const& paramNames)
			: m_names{ DefaultVariableNames(t_types.size(), paramNames) }
			, m_types{ t_types }
		{};
		FunctionArgs(std::vector<std::pair<std::weak_ptr<Type_Info>, std::string>> const& TypesAndNames)
			: m_names{}
			, m_types{}
		{
			std::vector<std::weak_ptr<Type_Info>> types;
			for (int i = 0; i < TypesAndNames.size(); i++) {
				m_names.push_back(TypesAndNames[i].second);
				types.push_back(TypesAndNames[i].first);
			}
			m_types = ParamTypes(types);
		};
		FunctionArgs(FunctionArgs const&) = default;
		FunctionArgs(FunctionArgs&&) = default;
		FunctionArgs& operator=(FunctionArgs const&) = default;
		FunctionArgs& operator=(FunctionArgs&&) = default;
		~FunctionArgs() = default;

		auto& Type(int i) const {
			return m_types[i];
		};
		auto& Name(int i) const {
			return m_names[i];
		};
		auto& Types() const {
			return m_types;
		};
		auto& Names() const {
			return m_names;
		};

		friend bool operator==(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types == b.m_types;
		};
		friend bool operator!=(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types != b.m_types;
		};
		friend bool operator>(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types > b.m_types;
		};
		friend bool operator<(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types < b.m_types;
		};
		friend bool operator>=(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types >= b.m_types;
		};
		friend bool operator<=(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types <= b.m_types;
		};

		size_t size() const noexcept {
			return m_types.size();
		}
		bool empty() const noexcept { return m_types.empty(); }
		size_t hash() const noexcept { return m_types.hash(); };

		auto begin() const noexcept {
			return m_types.begin();
		};
		auto end() const noexcept {
			return m_types.end();
		};

		bool CanCastTo(ParamTypes const& to) const {
			return m_types.CanCast(to);
		};
		bool CanCastFrom(ParamTypes const& from) const {
			return from.CanCast(m_types);
		};
		bool CanCastTo(FunctionArgs const& to) const {
			return m_types.CanCast(to.m_types);
		};
		bool CanCastFrom(FunctionArgs const& from) const {
			return from.m_types.CanCast(m_types);
		};
		bool IsTemplate() const { return m_types.IsTemplate(); };
	private:
		ParamTypes m_types;
		std::vector<std::string> m_names;
	};

	// A combination of FunctionArgs (argument types and names), return type, and function name. 
	// Function names DO impact the "uniqueness" of a function signature.
	// Return types do NOT impact the "uniqueness" of a function signature.
	class FunctionSignature {
	public:
		static size_t CalculateHash(FunctionArgs const& arguments, std::string const& qualified_name);
		static size_t CalculateHash(ParamTypes const& arguments, std::string const& qualified_name);

	public:
		FunctionSignature()
			: m_qualified_name("::")
		{
			uniqueHash = CalculateHash(m_arguments, m_qualified_name);
		};
		FunctionSignature(
			std::weak_ptr<Type_Info> returnType
			, FunctionArgs const& args
			, std::string const& Namespace = ""
			, std::string const& name = "")
			: m_arguments(args)
			, m_namespace(Namespace)
			, m_name(name)
			, m_qualified_name(Namespace + "::" + name)
			, m_returnType(returnType)
			, uniqueHash(CalculateHash(args, Namespace + "::" + name))
		{};
		FunctionSignature(FunctionSignature const&) = default;
		FunctionSignature(FunctionSignature&&) = default;
		FunctionSignature& operator=(FunctionSignature const&) = default;
		FunctionSignature& operator=(FunctionSignature&&) = default;
		~FunctionSignature() = default;

		const std::weak_ptr<Type_Info>& Returns() const { return m_returnType; };
		const FunctionArgs& Arguments() const { return m_arguments; };
		const std::string& Name() const { return m_name; };
		const std::string& QualifiedName() const { return m_qualified_name; };
		void Name(const std::string& input) { m_name = input; };
		void QualifiedName(const std::string& input) { m_qualified_name = input; };
		size_t hash() const noexcept { return uniqueHash; };
		bool IsTemplate() const { return m_arguments.IsTemplate(); };

		friend bool operator==(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash == b.uniqueHash;
		};
		friend bool operator!=(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash != b.uniqueHash;
		};
		friend bool operator>(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash > b.uniqueHash;
		};
		friend bool operator<(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash < b.uniqueHash;
		};
		friend bool operator>=(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash >= b.uniqueHash;
		};
		friend bool operator<=(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash <= b.uniqueHash;
		};

	private:
		FunctionArgs m_arguments; // Use this for the hash.
		std::string m_namespace;
		std::string m_name;
		std::string m_qualified_name; // m_namespace::m_name. Use this for the hash.
		std::weak_ptr<Type_Info> m_returnType; // not used for the hash.
		size_t uniqueHash;
	};
};

// FunctionSignature, FunctionArgs, & ParamTypes std::hash, std::less, std::greater, std::equal_to
namespace std {
	template <> struct hash<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& lhs, const GoodLang::ParamTypes& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& lhs, const GoodLang::ParamTypes& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& lhs, const GoodLang::ParamTypes& rhs) const {
			return lhs == rhs;
		};
	};

	template <> struct hash<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& lhs, const GoodLang::FunctionArgs& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& lhs, const GoodLang::FunctionArgs& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& lhs, const GoodLang::FunctionArgs& rhs) const {
			return lhs == rhs;
		};
	};

	template <> struct hash<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& lhs, const GoodLang::FunctionSignature& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& lhs, const GoodLang::FunctionSignature& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& lhs, const GoodLang::FunctionSignature& rhs) const {
			return lhs == rhs;
		};
	};
};

// Proxy_Function_Base 
namespace GoodLang {
	namespace details {
		/**
		 * Pure virtual base class for all Proxy_Function implementations
		 * Proxy_Functions are a type erasure of type-safe C++ function calls.
		 * At runtime parameter types are expected to be tested against passed in types.
		 * Dispatch_Engine only knows how to work with Proxy_Function, no other
		 * function classes.
		*/
		class Proxy_Function_Base {
		private:
			static double conversion_cost_fast(std::vector<Any> const& t_from, std::vector<std::shared_ptr<Type_Info>> const& t_FromTypes, ParamTypes const& t_to, TypeConverter& t_conversions);
			static double conversion_cost(std::vector<Any> const& t_from, ParamTypes const& t_to, TypeConverter& t_conversions);
			static std::vector<Any> convert(std::vector<Any> const& t_from, ParamTypes const& t_to, TypeConverter& t_conversions);
			static std::vector<Any> convert(std::vector<Any> const& t_from, ParamTypes const& t_to);
			static std::vector<Any> convert(Any& t_from, ParamTypes const& t_to);

		protected:
			GoodLang::FunctionSignature m_signature;

		public:
			virtual ~Proxy_Function_Base() = default;

			size_t hash() const;
			const GoodLang::FunctionSignature& GetSignature() const;
			GoodLang::FunctionSignature& GetSignature();
			size_t NumArguments() const;
			const auto& Argument(size_t N) const noexcept { return m_signature.Arguments().Type(N); };
			const auto& Arguments() const noexcept { return m_signature.Arguments(); };
			const auto& Returns() const noexcept { return m_signature.Returns(); };

			// Symbolic "cost" to perform the conversion. Not meant to be precise, but meant to be relative for comparison with other converters.
			double conversion_cost(std::vector<Any> const& t_params, TypeConverter& t_conversions) const;
			// Symbolic "cost" to perform the conversion. Not meant to be precise, but meant to be relative for comparison with other converters.
			double conversion_cost_fast(std::vector<Any> const& t_params, std::vector<std::shared_ptr<Type_Info>> const& t_FromTypes, TypeConverter& t_conversions) const;

			// Does want conversions -- ensure types match if possible.
			Any operator()(const std::vector<Any>& params, TypeConverter& t_conversions) const;
			// Does want conversions -- ensure types match if possible.
			Any operator()(const std::vector<Any>& params) const;
			// Does want conversions -- ensure types match if possible.
			Any operator()(Any& params) const;

			friend bool operator==(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature == rhs.m_signature; // same signature
			};
			friend bool operator!=(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature != rhs.m_signature; // same signature
			};
			friend bool operator>(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature > rhs.m_signature; // same signature
			};
			friend bool operator>=(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature >= rhs.m_signature; // same signature
			};
			friend bool operator<(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature < rhs.m_signature; // same signature
			};
			friend bool operator<=(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature <= rhs.m_signature; // same signature
			};

		protected:
			// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
			std::vector<Any> convert(std::vector<Any> const& t_params, TypeConverter& t_conversions) const;
			// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
			std::vector<Any> convert(std::vector<Any> const& t_params) const;
			// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
			std::vector<Any> convert(Any& t_params) const;

		protected:
			virtual Any do_call(std::vector<Any> const&) const = 0;
			Proxy_Function_Base(GoodLang::FunctionSignature const& p_signature) : m_signature(p_signature) {}
		};
	};
};

// Proxy_Function_Base std::hash, std::less, std::greater, std::equal_to
namespace std {
	template <> struct hash<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& lhs, const GoodLang::details::Proxy_Function_Base& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& lhs, const GoodLang::details::Proxy_Function_Base& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& lhs, const GoodLang::details::Proxy_Function_Base& rhs) const {
			return lhs == rhs;
		};
	};

	template <> struct hash<std::shared_ptr<GoodLang::details::Proxy_Function_Base>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& k) const {
			if (k) return k->hash();
			else return 0;
		};
	};
	template <> struct less<std::shared_ptr<std::shared_ptr<GoodLang::details::Proxy_Function_Base>>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& lhs, const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& rhs) const {
			if (lhs && rhs) return *lhs < *rhs;
			else return 0;
		};
	};
	template <> struct greater<std::shared_ptr<GoodLang::details::Proxy_Function_Base>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& lhs, const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& rhs) const {
			if (lhs && rhs) return *lhs > *rhs;
			else return 0;
		};
	};
	template <> struct equal_to<std::shared_ptr<GoodLang::details::Proxy_Function_Base>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& lhs, const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& rhs) const {
			if (lhs && rhs) return *lhs == *rhs;
			else return 0;
		};
	};
};

// Proxy Function typedef, make_callable(...), and call(...)
namespace GoodLang {
	/// \brief Common typedef used for passing of any registered function in ChaiScript
	typedef std::shared_ptr<details::Proxy_Function_Base> Proxy_Function;

	namespace details {
		/**
		 * Use to call function objects
		*/
		template <class Callable>
		class Explicit_Function_Impl : public Proxy_Function_Base {
		protected:
			static GoodLang::FunctionSignature CreateSignature() {
				using argType = typename utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::arguments;
				using returnType = typename utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				std::vector<std::weak_ptr<Type_Info>> types(numArgs, std::weak_ptr<Type_Info>());
#define argT(NN) if constexpr (numArgs > NN) { types[NN] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
				argT(0);
				argT(1);
				argT(2);
				argT(3);
				argT(4);
				argT(5);
				argT(6);
				argT(7);
				argT(8);
				argT(9);
				argT(10);
				argT(11);
				argT(12);
				argT(13);
				argT(14);
				argT(15);
#undef argT
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<returnType>(), args, "", "");
			};

		public:
			Explicit_Function_Impl(Callable F_p)
				: Proxy_Function_Base(CreateSignature())
				, F_m(std::move(F_p))
			{};
			virtual ~Explicit_Function_Impl() = default;

			GoodLang::FunctionSignature& GetSignature() {
				return this->m_signature;
			};
		protected:
			virtual Any do_call(std::vector<Any> const& r) const override {
				using argType = typename utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::arguments;
				using returnType = typename utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				if constexpr (std::is_reference< returnType>::value) {
					// if the return type is a reference, the parent(s) should be protected by carrying them along. 
					using refAsBaseType = typename std::remove_reference_t< returnType>;
					using ptrType = std::shared_ptr<refAsBaseType>;
					ptrType out;
					std::vector<Any> parents = r;
					if constexpr (numArgs == 16) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 15) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 14) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 13) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 12) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 11) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 10) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 9) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 8) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 7) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 6) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 5) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 4) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 3) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 2) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 1) {
						out = ptrType(&F_m(
							r[0].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs <= 0) {
						out = ptrType(&F_m(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					return out;
				}
				else {
					// best-case, normal operation
					if constexpr (std::is_same_v<returnType, void>) {
						static Any temp;
						if constexpr (numArgs == 16) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							F_m();
						}
						return temp;
					}
					else {

						if constexpr (numArgs == 16) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							return F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							return F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							return F_m();
						}
					}
				}
			};
			Callable F_m;
		};

		/**
		 * Use to call member objects:
		 * struct Test{ public: std::string attr; }
		 * var& func = details::Attribute_Access_Impl(&Test::attr);
		 * assert(func(Test{ "STR" }).cast<std::string>() == "STR");
		*/
		template <typename T, class Class>
		class Attribute_Access_Impl : public Proxy_Function_Base {
		protected:
			using ReturnType = typename std::decay_t<typename details::get_type<T>::type>;
			static GoodLang::FunctionSignature CreateSignature() {
				std::vector<std::weak_ptr<Type_Info>> types = { user_type_shared<Class&>() };
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<ReturnType&>(), args, "", "");
			};

		public:
			using actualT = typename std::decay_t<typename details::get_type<T>::type>;
			Attribute_Access_Impl(T Class::* t_attr)
				: Proxy_Function_Base(CreateSignature())
				, m_attr(t_attr)
			{};
			virtual ~Attribute_Access_Impl() = default;
			GoodLang::FunctionSignature& GetSignature() {
				return this->m_signature;
			};
		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < 1) throw(exception::arity_error(0, 1));
				return do_call_impl(r[0].cast<std::shared_ptr<Class>>());
			};

			auto& do_call_impl_impl(Class* o) const {
				return o->*m_attr;
			};

			Any do_call_impl(std::shared_ptr<Class> o) const {
				if constexpr (std::is_same_v<void, T>) {
					// void? Return void.
					return Any();
				}
				else if constexpr (std::is_same<Any, actualT>::value) { // typename std::remove_reference_t<T>
					// Any? Return reference to the underlying value, NOT a reference to the Any.
					return do_call_impl_impl(o.get());
				}
				else if constexpr (std::is_pointer<T>::value) {
					// Pointer? Wrap it as a shared pointer.
					using Type = typename std::remove_pointer<T>::type;
					decltype(auto) ptr = do_call_impl_impl(o.get());
					if (ptr) {
						return std::shared_ptr<Type>(ptr, [=](Type*) { (void)o.get(); /* do nothing */ });
					}
					else {
						return Any();
					}
				}
				else {
					// Reference? Wrap it as a shared pointer.
					using Type = typename std::remove_reference<T>::type;
					return std::shared_ptr<Type>(&(do_call_impl_impl(o.get())), [=](Type*) { (void)o.get(); /* do nothing */ });
				}
			};

			T Class::* m_attr;
		};

		template <typename T, class Class>
		class Const_Attribute_Access_Impl : public Proxy_Function_Base {
		protected:
			using ReturnType = typename std::decay_t<typename details::get_type<T>::type>;

			static GoodLang::FunctionSignature CreateSignature() {
				std::vector<std::weak_ptr<Type_Info>> types = { user_type_shared<const Class&>() };
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<const ReturnType&>(), args, "", "");
			};

		public:
			using actualT = typename std::decay_t<typename details::get_type<T>::type>;
			Const_Attribute_Access_Impl(T Class::* t_attr)
				: Proxy_Function_Base(CreateSignature())
				, m_attr(t_attr)
			{};
			virtual ~Const_Attribute_Access_Impl() = default;
			GoodLang::FunctionSignature& GetSignature() {
				return this->m_signature;
			};
		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < 1) throw(exception::arity_error(0, 1));
				return do_call_impl(r[0].cast<std::shared_ptr<Class>>());
			};

			auto& do_call_impl_impl(Class* o) const {
				return o->*m_attr;
			};

			Any do_call_impl(std::shared_ptr<Class> o) const {
				if constexpr (std::is_same_v<void, T>) {
					// void? Return void.
					return Any();
				}
				else if constexpr (std::is_same<Any, actualT>::value) { // typename std::remove_reference_t<T>
					// Any? Return reference to the underlying value, NOT a reference to the Any.
					return do_call_impl_impl(o.get());
				}
				else if constexpr (std::is_pointer<T>::value) {
					// Pointer? Wrap it as a shared pointer.
					using Type = typename std::remove_pointer<T>::type;
					decltype(auto) ptr = do_call_impl_impl(o.get());
					if (ptr) {
						return std::shared_ptr<Type>(ptr, [=](Type*) { (void)o.get(); /* do nothing */ });
					}
					else {
						return Any();
					}
				}
				else {
					// Reference? Wrap it as a shared pointer.
					using Type = typename std::remove_reference<T>::type;
					return std::shared_ptr<Type>(&(do_call_impl_impl(o.get())), [=](Type*) { (void)o.get(); /* do nothing */ });
				}
			};

			T Class::* m_attr;
		};

		namespace detail {
			/**
			 * Use to call member functions:
			 * struct Test{ public: std::string attr(){ return "TEST"; }; }
			 * var& func = details::Attribute_Access_Impl(&Test::attr);
			 * assert(func(Test{}).cast<std::string>() == "TEST");
			*/
			template <typename R, typename Class, typename... T>
			class VolatileConst_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				using actualT = typename std::decay_t<typename details::get_type<R>::type>;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<const Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN+1, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				VolatileConst_Member_Function_Impl(R(Class::* f)(T...) volatile const)
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~VolatileConst_Member_Function_Impl() = default;
				GoodLang::FunctionSignature& GetSignature() {
					return this->m_signature;
				};
			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					using returnType = R;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						using refAsBaseType = typename std::remove_reference_t< returnType>;
						using ptrType = std::shared_ptr<refAsBaseType>;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};

				R(Class::* m_attr)(T...) volatile const;
			};
			template <typename R, typename Class, typename... T>
			class Volatile_Member_Function_Impl : public Proxy_Function_Base {
			public:
				typedef std::tuple<Class, T...> argType;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				typedef typename std::decay_t<typename details::get_type<R>::type> actualT;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN+1, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				Volatile_Member_Function_Impl(R(Class::* f)(T...) volatile)
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~Volatile_Member_Function_Impl() = default;
				GoodLang::FunctionSignature& GetSignature() {
					return this->m_signature;
				};
			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					typedef R returnType;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						typedef typename std::remove_reference_t< returnType> refAsBaseType;
						typedef std::shared_ptr<refAsBaseType> ptrType;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};

				R(Class::* m_attr)(T...) volatile;
			};
			template <typename R, typename Class, typename... T>
			class Const_Member_Function_Impl : public Proxy_Function_Base {
			public:
				typedef std::tuple<Class, T...> argType;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				typedef typename std::decay_t<typename details::get_type<R>::type> actualT;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<const Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN+1, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				Const_Member_Function_Impl(R(Class::* f)(T...) const)
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~Const_Member_Function_Impl() = default;
				GoodLang::FunctionSignature& GetSignature() {
					return this->m_signature;
				};
			protected:
				virtual Any do_call(std::vector<Any> const& r) const override {
					typedef R returnType;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						typedef typename std::remove_reference_t< returnType> refAsBaseType;
						typedef std::shared_ptr<refAsBaseType> ptrType;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};
				R(Class::* m_attr)(T...) const;
			};
			template <typename R, typename Class, typename... T>
			class Default_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				using actualT = typename std::decay_t<typename details::get_type<R>::type>;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN+1, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				Default_Member_Function_Impl(R(Class::* f)(T...))
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~Default_Member_Function_Impl() = default;
				GoodLang::FunctionSignature& GetSignature() {
					return this->m_signature;
				};
			private:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					using returnType = R;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						using refAsBaseType = typename std::remove_reference_t< returnType>;
						using ptrType = std::shared_ptr<refAsBaseType>;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};

				R(Class::* m_attr)(T...);
			};

		};

		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile const) {
			auto* function_impl = new detail::VolatileConst_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile) {
			auto* function_impl = new detail::Volatile_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) const) {
			auto* function_impl = new detail::Const_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...)) {
			auto* function_impl = new detail::Default_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};

		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile const, ParamTypes const& paramTypes) {
			auto* function_impl = new detail::VolatileConst_Member_Function_Impl(f);
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile, ParamTypes const& paramTypes) {
			auto* function_impl = new detail::Volatile_Member_Function_Impl(f);
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) const, ParamTypes const& paramTypes) {
			auto* function_impl = new detail::Const_Member_Function_Impl(f);
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...), ParamTypes const& paramTypes) {
			auto* function_impl = new detail::Default_Member_Function_Impl(f);
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};

		/**
		 * Use to call member functions:
		 * struct Test{ public: std::string attr(){ return "TEST"; }; }
		 * var& func = details::Attribute_Access_Impl(&Test::attr);
		 * assert(func(Test{}).cast<std::string>() == "TEST");
		*/
		template <typename R, typename... T>
		class Static_Function_Impl : public Proxy_Function_Base {
		public:
			using argType = std::tuple<R, T...>;
			static constexpr auto numArgs = std::tuple_size_v<argType> -1;
			static GoodLang::FunctionSignature CreateSignature() {
				std::vector<std::weak_ptr<Type_Info>> types(numArgs, std::weak_ptr<Type_Info>());
#define argT(NN) if constexpr (numArgs > NN) { types[NN] = user_type_shared<typename std::tuple_element_t<NN+1, argType>>(); }
				argT(0);
				argT(1);
				argT(2);
				argT(3);
				argT(4);
				argT(5);
				argT(6);
				argT(7);
				argT(8);
				argT(9);
				argT(10);
				argT(11);
				argT(12);
				argT(13);
				argT(14);
				argT(15);
#undef argT
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<R>(), args, "", "");
			};

		public:
			Static_Function_Impl(R(*f)(T...))
				: Proxy_Function_Base(CreateSignature())
				, F_m(std::move(f)) {};
			virtual ~Static_Function_Impl() = default;
			GoodLang::FunctionSignature& GetSignature() {
				return this->m_signature;
			};
		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < numArgs) throw(exception::arity_error(r.size(), numArgs));
				return do_call_impl(r);
			};

			Any do_call_impl(std::vector<Any> const& r) const {
				if constexpr (std::is_reference< R>::value) {
					// if the return type is a reference, the parent(s) should be protected by carrying them along. 
					using refAsBaseType = typename std::remove_reference_t< R>;
					using ptrType = std::shared_ptr<refAsBaseType>;
					ptrType out;
					std::vector<Any> parents = r;
					if constexpr (numArgs == 16) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 15) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 14) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 13) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 12) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 11) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 10) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 9) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 8) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 7) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 6) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 5) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 4) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 3) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 2) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs == 1) {
						out = ptrType(&F_m(
							r[0].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					else if constexpr (numArgs <= 0) {
						out = ptrType(&F_m(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { throw exception::arity_error(parents.size(), numArgs); } });
					}
					return out;
				}
				else {
					// best-case, normal operation
					if constexpr (std::is_same_v<R, void>) {
						static Any temp;
						if constexpr (numArgs == 16) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							F_m();
						}
						return temp;
					}
					else {

						if constexpr (numArgs == 16) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							return F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							return F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							return F_m();
						}
					}
				}
			};

			R(*F_m)(T...);
		};

		namespace detail {
			template <typename T>
			struct is_static_member_function : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...)> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const volatile> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) volatile> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...)&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...)&&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const&&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) noexcept> : std::true_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const noexcept> : std::true_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) volatile noexcept> : std::true_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const volatile noexcept> : std::true_type {};


			template<typename... Param> struct Function_Params {
				typedef std::tuple<Param...> argType;
				static constexpr auto numArgs = std::tuple_size_v<argType>;
			};

			template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
			struct Function_Signature {
				typedef Params Param_Types;
				typedef Class Class_Type;
				typedef Ret Return_Type;

				constexpr static const bool is_object = IsObject; // e.g. lambda object
				constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
				constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
				constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
				constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

				template<typename T> constexpr Function_Signature(T&&) noexcept { };
				constexpr Function_Signature() noexcept = default;
			};

			// Free functions
			template<typename Ret, typename... Param>
			Function_Signature(Ret(*f)(Param...))
				->Function_Signature<Ret, void, Function_Params<Param...>>; // static function

			// no reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...))
				->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const)
				->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

			// & reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile&)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const&)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...)&)
				->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const&)
				->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

			// && reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile&&)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...)&&)
				->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const&&)
				->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class>
			Function_Signature(Ret Class::* f)
				->Function_Signature<Ret, Class, Function_Params<Class&>, true, true>; // member object

			// primary template handles types that have no nested ::type member:
			template<class, class = std::void_t<>>
			struct has_call_operator : std::false_type {};

			// specialization recognizes types that do have a nested ::type member:
			template<class T>
			struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

			template<typename Func>
			auto function_signature(Func const& f) {
				if constexpr (has_call_operator<Func>::value) {
					return Function_Signature<
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Class_Type,
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Param_Types,
						false,
						false,
						true
					> {};
				}
				else {
					return Function_Signature{ f };
				}
			};

			template<typename Obj, typename Param1, typename... Rest>
			Param1 get_first_param(Function_Params<Param1, Rest...>, Obj&& obj) {
				return static_cast<Param1>(std::forward<Obj>(obj));
			};

		} // namespace chaiscript::dispatch::detail
	};

	// Convert nearly any function or function pointer to a callable, generic proxy function. 
	template<typename Func> Proxy_Function make_callable(Func&& func) {
		typedef decltype(details::detail::function_signature(func)) function_header;

		if constexpr (function_header::is_object) {
			// function objects, e.g. auto x = [](){};
			auto* function_impl = new details::Explicit_Function_Impl(std::forward<Func>(func));
			return std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
		}
		else if constexpr (function_header::is_member_object) {
			// member objects, e.g. return object.member;
			auto* function_impl = new details::Attribute_Access_Impl(std::forward<Func>(func));
			return std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
		}
		else if constexpr (function_header::is_member && !function_header::is_member_object) {
			// member functions, e.g. return object.member();
			return details::Member_Function_Impl(std::forward<Func>(func));
		}
		else if constexpr (function_header::is_static_member_function) {
			// static function pointers, e.g. static foo(){};
			auto* function_impl = new details::Static_Function_Impl(std::forward<Func>(func));
			return std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
		}
		else {
			throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
		}
	};

	// Convert nearly any function or function pointer to a callable, generic proxy function. 
	template<typename Func> Proxy_Function make_callable(Func&& func, ParamTypes const& paramTypes) {
		typedef decltype(details::detail::function_signature(func)) function_header;

		static constexpr const bool is_static_member_function = function_header::is_static_member_function;
		static constexpr const bool is_member = function_header::is_member;
		static constexpr const bool is_object = function_header::is_object;
		static constexpr const bool is_member_object = function_header::is_member_object;

		if constexpr (is_object) {
			// function objects, e.g. auto x = [](){};
			auto* function_impl = new details::Explicit_Function_Impl(std::forward<Func>(func));
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));

			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member_object) {
			// member objects, e.g. return object.member;
			auto* function_impl = new details::Attribute_Access_Impl(std::forward<Func>(func));
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));

			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member && !is_member_object) {
			// member functions, e.g. return object.member();
			return details::Member_Function_Impl(std::forward<Func>(func), paramTypes);
		}
		else if constexpr (is_static_member_function) {
			// static function pointers, e.g. static foo(){};
			auto* function_impl = new details::Static_Function_Impl(std::forward<Func>(func));
			FunctionSignature& signature = function_impl->GetSignature();
			function_impl->GetSignature() = FunctionSignature(signature.Returns(), FunctionArgs(paramTypes));

			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else {
			throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
		}
	};

	// Convert nearly any function or function pointer to a callable, generic proxy function. 
	template<typename Func> Proxy_Function make_callable(Func&& func, ParamTypes const& paramTypes, std::weak_ptr<Type_Info> const& resultType) {
		typedef decltype(details::detail::function_signature(func)) function_header;

		static constexpr const bool is_static_member_function = function_header::is_static_member_function;
		static constexpr const bool is_member = function_header::is_member;
		static constexpr const bool is_object = function_header::is_object;
		static constexpr const bool is_member_object = function_header::is_member_object;

		if constexpr (is_object) {
			// function objects, e.g. auto x = [](){};
			auto* function_impl = new details::Explicit_Function_Impl(std::forward<Func>(func));
			function_impl->GetSignature() = FunctionSignature(resultType, FunctionArgs(paramTypes));

			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member_object) {
			// member objects, e.g. return object.member;
			auto* function_impl = new details::Attribute_Access_Impl(std::forward<Func>(func));
			function_impl->GetSignature() = FunctionSignature(resultType, FunctionArgs(paramTypes));

			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member && !is_member_object) {
			// member functions, e.g. return object.member();
			return details::Member_Function_Impl(std::forward<Func>(func), paramTypes);
		}
		else if constexpr (is_static_member_function) {
			// static function pointers, e.g. static foo(){};
			auto* function_impl = new details::Static_Function_Impl(std::forward<Func>(func));
			function_impl->GetSignature() = FunctionSignature(resultType, FunctionArgs(paramTypes));

			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else {
			throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
		}
	};

	// Call a generic, proxy function with a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
	Any call(Proxy_Function callable, std::vector<Any> const& inputs, TypeConverter& conversionTree);
};

// Functions wrapper
namespace GoodLang {
	class Function {
	public:
		Function() = default;
		Function(Proxy_Function const& a)
			: m_function(a)
		{};
		Function(Proxy_Function const& a, bool isExplicit, bool isCached = false)
			: m_function(a)
			, m_isEplicit(isExplicit)
			, m_isCached(isCached)
		{};
		Function(Function&&) = default;
		Function(Function const&) = default;
		Function& operator=(Function&&) = default;
		Function& operator=(Function const&) = default;
		~Function() = default;

		Proxy_Function m_function{ nullptr };
		bool m_isEplicit{ false };
		bool m_isCached{ false };
	};

	/*
	// If a function is namespaced in a class, that means it's a free function in the namespace of _CLASS_NAME_, whose first parameter is to be that class type.
	def _CLASS_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _FUNCTION_NAME_( _CLASS_NAME_(),  ... );
		or
		return _CLASS_NAME_()._FUNCTION_NAME_(...);

	// If a function is namespaced in a class but noted as "static", that means it is a free-function, within the namespace of _CLASS_NAME_.
	def static _CLASS_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _CLASS_NAME_::_FUNCTION_NAME_( ... );

	// If a function is declared but not namespaced, then it is assumed to be a free function declared in the current namespace
	def _FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _FUNCTION_NAME_( ... );

	// if a function is namespaced in a namespace, then it is assumed to be a free function
	def _NAMESPACE_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info {};
		e.g.
		return _NAMESPACE_NAME_::_FUNCTION_NAME_();
		or
		return (...)._FUNCTION_NAME_(...);

	namespace { // e.g. global namespace
		class _CLASS_NAME_ {
			def _FUNCTION_NAME_(...) -> void {};
				e.g.
				return ::_CLASS_NAME_()._FUNCTION_NAME_( ... );
				or
				return ::_CLASS_NAME_::_FUNCTION_NAME_( ::_CLASS_NAME_(), ...);

			def static _FUNCTION_NAME_(...) -> void {};
				e.g.
				return ::_CLASS_NAME_()._FUNCTION_NAME_( ... );
				or
				return ::_CLASS_NAME_::_FUNCTION_NAME_( ... );

			def _CLASS_NAME_() -> _CLASS_NAME_ {}; // EXCEPTION TO THE ABOVE RULES. IF THE FUNCTION NAME MATCHES THE CLASS NAME, THEN IT'S A FUNCTION ADDED TO THE PARENT NAMESPACE
				e.g.
				return ::_CLASS_NAME_();

			def _CLASS_NAME_(...) -> _CLASS_NAME_ {}; // EXCEPTION TO THE ABOVE RULES. IF THE FUNCTION NAME MATCHES THE CLASS NAME, THEN IT'S A FUNCTION ADDED TO THE PARENT NAMESPACE
				e.g.
				return ::_CLASS_NAME_(...);

			Type_Info _PARAMETER_NAME_; // parameter within the class, accessible from an instance of it.
				e.g.
				return ::_CLASS_NAME_()._PARAMETER_NAME_;
				or
				return ::_CLASS_NAME_::_PARAMETER_NAME_( ::_CLASS_NAME_() );

			static Type_Info _VARIABLE_NAME_; // static variable associated to a class, accessible from an instance of it or from the class name.
				e.g.
				return ::_CLASS_NAME_()._VARIABLE_NAME_;
				or
				return ::_CLASS_NAME_::_VARIABLE_NAME_;

			class _CLASS_NAME_ { ... } // may declare classes within a class
			namespace _NAMESPACE_NAME_ { ... } // may declare namespaces within a class

		};
	};

	namespace { // e.g. global namespace
		namespace _NAMESPACE_NAME_ {
			def _FUNCTION_NAME_() -> void {}; // IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_FUNCTION_NAME_();

			def _FUNCTION_NAME_(...) -> void {}; // IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_FUNCTION_NAME_(...);

			Type_Info _VARIABLE_NAME_; // global variable within the namespace, accessible outside of it. IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_VARIABLE_NAME_;

			class _CLASS_NAME_ { ... } // may declare classes within a namespace
			namespace _NAMESPACE_NAME_ { ... } // may declare namespaces within a namespace


		};
	};

	// If a function is delcared in a namespace, how that namespace begins determines the ownership behavior
	namespace _NAMESPACE_NAME_ { // e.g. current parent namespace
		def _FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_NAMESPACE_NAME_::_FUNCTION_NAME_(...); // incorporates the _NAMESPACE_NAME_

		def ::_FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_FUNCTION_NAME_(...); // ignores the _NAMESPACE_NAME_

		def ::_NEW_NAMESPACE_NAME_::_FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_NEW_NAMESPACE_NAME_::_FUNCTION_NAME_(...); // ignores the _NAMESPACE_NAME_
	};

	// A possible use of that feature would be extend global functions:
	namespace _CLASS_NAME_ { // e.g. current parent namespace
		def ::to_string() ->  void {}; // non-static implies the first parameter must be a _CLASS_NAME_
			e.g.
			_CLASS_NAME_().to_string();
			or
			to_string(_CLASS_NAME_());

		def static ::to_json(_CLASS_NAME_ a) -> JSON {};
			e.g.
			to_json(_CLASS_NAME_()); // as a static, global function
			or
			_CLASS_NAME_().to_json; // as a static function called on a class_name object

		def static ::`==`(_CLASS_NAME_ a, _CLASS_NAME_ b) -> bool {};
			e.g.
			_CLASS_NAME_() == _CLASS_NAME_();
			or
			`==`( _CLASS_NAME_(), _CLASS_NAME_() );
	};



	*/
	class Functions {
	public:
		Functions() = default;
		Functions(Functions const& rhs) {
			auto locked2{ std::shared_lock(rhs.m_mut) };
			m_functions = rhs.m_functions;
		};
		Functions(Functions&& rhs) {
			auto locked2{ std::unique_lock(rhs.m_mut) };
			m_functions = std::move(rhs.m_functions);
		};
		Functions& operator=(Functions const& rhs) {
			auto locked{ std::unique_lock(m_mut) };
			auto locked2{ std::shared_lock(rhs.m_mut) };
			m_functions = rhs.m_functions;
		};
		Functions& operator=(Functions&& rhs) {
			auto locked{ std::unique_lock(m_mut) };
			auto locked2{ std::unique_lock(rhs.m_mut) };
			m_functions = std::move(rhs.m_functions);
		};
		~Functions() = default;

	public:
		typedef std::shared_ptr<Function> 
			FunctionPtr;
		typedef concurrency::concurrent_unordered_map< size_t, std::pair<ParamTypes, FunctionPtr>> 
			FunctionSort; // key may NOT be the function's underlying params, but just params that were previously searched... 
		typedef concurrency::concurrent_unordered_map< size_t, std::pair<std::string, FunctionSort> > 
			FunctionMap;

		FunctionMap m_functions;
		mutable std::shared_mutex m_mut{};

	private:
		FunctionPtr at_unsafe(std::string const& key, ParamTypes const& params) const;

	public:
		FunctionPtr operator()(std::string const& key, ParamTypes const& params) const;
		FunctionPtr at(std::string const& key, ParamTypes const& params) const;

		FunctionPtr emplace(std::string const& key, ParamTypes const& params, Function const& func, bool replaceIfAlreadyExists = false);
		FunctionPtr emplace(std::string const& key, Function const& func, bool replaceIfAlreadyExists = false);

		/* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
		Proxy_Function BuildMatch(std::string const& functionName, std::vector<Any> const& params, TypeConverter& m_typeConverters, bool AllowTemplateInstantiation = true, bool AllowTypeConversion = true);
		Any Call(std::string const& functionName, std::vector<Any> const& params, TypeConverter& m_typeConverters);

	};
};