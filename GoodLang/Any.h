#pragma once

#include "Foundation.h"
#include <cstdarg>
#include <functional>
#include <boost/any.hpp>
#include <shared_mutex>
#include <concurrent_unordered_map.h>
// #include <string_view>

#include <vector>
#include <map>
#include <set>


// typenames, function_traits
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
			, isConst(false)
			, isVoid(true)
			, isRef(false)
		{};
		Type_Info(bool t_is_const, bool t_is_void, bool t_is_ref, bool t_is_any) noexcept
			: uniqueHash(GetHashImpl())
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
			, m_type_info(impl::TypeId<void>())
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
		mutable std::shared_mutex m_children_mut;
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
			if (k) {
				return k->GetHash();
			}
			else {
				return GoodLang::impl::TypeId<void>().hash_code();
			}
		};
	};
	template <> struct hash<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& k) const {
			if (auto p = k.lock()) {
				return p->GetHash();
			}
			else {
				return GoodLang::impl::TypeId<void>().hash_code();
			}
		};
	};
};

// GetHash
namespace GoodLang {
	template <typename T> auto& GetHash() {
		static auto hasher{ std::hash<typename details::Bare_Type<T>::type>{} };
		return hasher;
	};
	template <typename T> size_t GetHash(const T& a) {
		return GetHash<T>()(a);
	};
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
	template<typename T> std::weak_ptr<Type_Info> user_type_shared() noexcept {
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

// ToString
namespace GoodLang {
	namespace details {
		template <template<class, class, class, class, class, class> class H, class S1, class S2, class S3, class S4, class S5, class S6> __forceinline std::string ToString_templated() {
			return user_type<H<S1, S2, S3, S4, S5, S6>>().name();
		};
		template <template<class, class, class, class, class> class H, class S1, class S2, class S3, class S4, class S5> __forceinline std::string ToString_templated() {
			return user_type<H<S1, S2, S3, S4, S5>>().name();
		};
		template <template<class, class, class, class> class H, class S1, class S2, class S3, class S4> __forceinline std::string ToString_templated() {
			return user_type<H<S1, S2, S3, S4>>().name();
		};
		template <template<class, class, class> class H, class S1, class S2, class S3> __forceinline std::string ToString_templated() {
			return user_type<H<S1, S2, S3>>().name();
		};
		template <template<class, class> class H, class S1, class S2> __forceinline std::string ToString_templated() {
			return user_type<H<S1, S2>>().name();
		};
		template <template<class> class H, class S1> __forceinline std::string ToString_templated() {
			return user_type<H<S1>>().name();
		};
		template <typename T> __forceinline std::string ToString_templated() {
			return user_type<T>().name();
		};
	};
	template <typename T> __forceinline std::string ToString(T const&) { return details::ToString_templated<T>(); };
	template <> __forceinline std::string ToString(std::nullptr_t const&) { return ""; };
	template <> __forceinline std::string ToString(char const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(unsigned char const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(short const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(unsigned short const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(int const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(unsigned int const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(long const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(unsigned long const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(long long const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(unsigned long long const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(float const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(double const& r) { return std::to_string(r); };
	template <> __forceinline std::string ToString(std::string const& r) { return r; };
	template <> __forceinline std::string ToString(Type_Info const& r) { return r.name(); };
	template <typename T> __forceinline std::string ToString(std::shared_ptr<T> const& r) { if (r) return ToString(*r); else return ToString(nullptr); };
	template <typename T> __forceinline std::string ToString(std::weak_ptr<T> const& r) { return ToString(r.lock()); };
	template <typename T, typename... Args> __forceinline std::string ToString(std::unique_ptr<T, Args...> const& r) { if (r) return ToString(*r); else return ToString(nullptr); };
	template <typename T, typename... Args> __forceinline std::string ToString(std::pair<T, Args...> const& r) {
		return std::string("<") + ToString(r.first) + ", " + ToString(r.second) + ">";
	};
	template <typename T, typename... Args> __forceinline std::string ToString(std::vector<T, Args...> const& r) {
		std::string out{};
		for (auto& x : r) {
			if (out.empty())
				out += ToString(x);
			else
				out += std::string(", ") + ToString(x);
		}
		return std::string("[") + out + std::string("]");
	};
	template <typename T, typename... Args> __forceinline std::string ToString(std::set<T, Args...> const& r) {
		std::string out{};
		for (auto& x : r) {
			if (out.empty())
				out += ToString(x);
			else
				out += std::string(", ") + ToString(x);
		}
		return std::string("[") + out + std::string("]");
	};
	template <typename T, typename... Args> __forceinline std::string ToString(std::map<T, Args...> const& r) {
		std::string out{};
		for (auto& x : r) {
			if (out.empty())
				out += ToString(x);
			else
				out += std::string(", ") + ToString(x);
		}
		return std::string("[") + out + std::string("]");
	};
	template <typename T, typename... Args> __forceinline std::string ToString(concurrency::concurrent_unordered_map<Args...> const& r) {
		std::string out{};
		for (auto& x : r) {
			if (out.empty())
				out += ToString(x);
			else
				out += std::string(", ") + ToString(x);
		}
		return std::string("[") + out + std::string("]");
	};
	template <typename T> __forceinline std::string ToStringImpl(T const& r) { return ToString(r); };

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
	template <> __forceinline std::string ToString(DynamicObject const& r) { return ToString(r.m_actualType) + "{ " + ToString(r.m_objects) + " }"; };

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
	template <> __forceinline std::string ToString(Var const& r) { return ToString(r.p_data); };
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
			arity_error(int t_got, int t_expected)
				: std::range_error(
					t_expected >= 0 ?
					GoodLang::printf("Arity mismatch: function requires %i parameters, but only %i were provided", t_expected, t_got)
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
		AnyData() noexcept = default;
		AnyData(AnyData const&) = default;
		AnyData(AnyData&&) = default;
		AnyData& operator=(AnyData const&) = default;
		AnyData& operator=(AnyData&&) = default;
		virtual ~AnyData() = default;

	public:
		// user must set the ptr to the AnyData object, so that it is aware of itself
		void SetSelf(std::shared_ptr< AnyData>& t_self);

	public:
		size_t GetTypeHash() const;
		virtual bool CanCast(Type_Info const& to_type) const;
		virtual Type_Info const& GetType() const;
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const;
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
		virtual size_t Constexpr_Type_Hash() const { return 0; };
		// virtual std::vector<std::weak_ptr<AnyData>> get_children() const;

	protected:
		std::weak_ptr< AnyData> m_self;
		size_t typeHash;

	};	

	template <typename T>
	class AnyData_Instanced final : public AnyData {
	public:
		AnyData_Instanced() noexcept = default;
		AnyData_Instanced(T t_obj) noexcept
			: AnyData()
			, m_obj{ std::move(t_obj) }
		{ };
		AnyData_Instanced(AnyData_Instanced const&) = default;
		AnyData_Instanced(AnyData_Instanced&&) = default;
		AnyData_Instanced& operator=(AnyData_Instanced const&) = default;
		AnyData_Instanced& operator=(AnyData_Instanced&&) = default;
		virtual ~AnyData_Instanced() = default;

		virtual bool CanCast(Type_Info const& to_type) const override { return GetType().CanCast(to_type); };
		virtual Type_Info const& GetType() const override { return user_type<T>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const override { return user_type_shared<T>(); };
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
		virtual size_t Constexpr_Type_Hash() const override { 
			return GoodLang::utilities::constexpr_type_info<T>::hash;
		};
		// virtual std::vector<std::weak_ptr<AnyData>> get_children() const override { /* depending on the type, it'll have children. */ };

	private:
		T m_obj;

	};

	template <typename T>
	class AnyData_Shared final : public AnyData {
	public:
		AnyData_Shared() noexcept = default;
		AnyData_Shared(std::shared_ptr<T> const& t_obj) noexcept
			: AnyData()
			, m_obj(t_obj)
		{ };
		AnyData_Shared(std::shared_ptr<T>&& t_obj) noexcept
			: AnyData()
			, m_obj(std::forward<std::shared_ptr<T>>(t_obj))
		{ };
		AnyData_Shared(AnyData_Shared const&) = default;
		AnyData_Shared(AnyData_Shared&&) = default;
		AnyData_Shared& operator=(AnyData_Shared const&) = default;
		AnyData_Shared& operator=(AnyData_Shared&&) = default;
		virtual ~AnyData_Shared() = default;

		virtual bool CanCast(Type_Info const& to_type) const override { return GetType().CanCast(to_type); };
		virtual Type_Info const& GetType() const override { return user_type<T>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const override { return user_type_shared<T>(); };
		virtual void* ptr() const override { return const_cast<void*>((const void*)(m_obj.get())); };
		virtual std::shared_ptr<void> shared_ptr() const override { return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(m_obj)); };
		virtual std::string ToString() const override {
			return GoodLang::ToString(*m_obj);
		};
		virtual size_t Constexpr_Type_Hash() const override {
			return GoodLang::utilities::constexpr_type_info<T>::hash;
		};
		// virtual std::vector<std::weak_ptr<AnyData>> get_children() const override { /* depending on the type, it'll have children. */ };

	private:
		std::shared_ptr<T> m_obj;

	};
	template <> __forceinline std::string ToString(AnyData const& r) { return r.ToString(); };

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
				return get((std::decay_t<T>)obj);
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
			auto locked2{ std::shared_lock(rhs.mut) };
			container = rhs.container;
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
		size_t TypeHash() const noexcept;
		bool IsTypeOf(std::weak_ptr<Type_Info> const& targetType) const noexcept;
		bool IsTypeOf(Type_Info const& targetType) const noexcept;
		template<typename VType> bool IsTypeOf() const noexcept {
			return IsTypeOf(user_type<typename std::decay_t<VType>>());
		};

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
					return p->container->cast_shared<VType>();
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

		size_t Constexpr_Type_Hash() const {
			if (std::shared_ptr<AnyData>& m = container) {
				return m->Constexpr_Type_Hash();
			}
			else {
				return 0;
			}
		};

	public:
		mutable std::shared_ptr<AnyData> container;
		mutable std::shared_mutex mut;
	};
	template <> __forceinline std::string ToString(Any const& r) { return ToString(r.container); };

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