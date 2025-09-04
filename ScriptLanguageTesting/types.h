#pragma once

#include <map>
#include <set>
#include "Strings.h"
#include "atomic_shared_ptr.h"
#include "atomic_maps.h"
#include <boost/type_index.hpp>
#include <concurrent_unordered_map.h>

namespace GL {
    class any;
    class any_cast;

    namespace util {
        template<typename T> static const auto& type_id() {
            static auto const& typeIdOfT{ boost::typeindex::type_id<T>().type_info() };
            return typeIdOfT;
        };
    };

    namespace impl {
        class cached_type {
        public:
            constexpr static size_t MAGIC_MASK1 = 0xF000'0000'0000'0000;
            constexpr static size_t MAGIC_MASK2 = ~MAGIC_MASK1;

            GL::string // underlying name for this type
                name{ "void" };
            std::set<size_t> // hashes to the underlying base classes to this type
                base_classes{};
            size_t // without const, ref, etc. 
                base_hash{ 0 };

            // returns true if this is found to be a child of the parent type (id'd by its base hash) 
            bool is_derived_from(size_t base) const;
            // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
            bool is_base_of(size_t derived) const;
            // attempts to include the specified hash as a base of this class.
            bool add_base(size_t base);

            bool match_base_hash(size_t to_match) const;

            //bool is_any() const noexcept { return modifiers & Modifiers::Any; };
            bool is_void() const noexcept {
                static size_t void_type{ util::type_id<void>().hash_code() & MAGIC_MASK2 };
                return void_type == this->base_hash;
            };
            // bool is_value() const noexcept { return modifiers & Modifiers::ValueType; };

        };

        // get the unique hash for this scripted type and perform set-up. This hash will always point this object until the "return" is called. 
        size_t checkout_scripted_type(GL::string type_name);
        // unloads the scripted type and allows the index (or hash) to be re-used. 
        void return_scripted_type(size_t ticket);
        // get the cached, scripted type info based on the ticketed hash. Must have been previously "checked out" otherwise may perform an out-of-bounds index call. 
        cached_type& get_scripted_type(size_t hash);

        // get the cached, built-in cpp type info based on the boost::type hash
        cached_type& get_impl(size_t hash);
        // get the cached, built-in cpp type info
        template <typename T> cached_type& get_impl() {
            using BaseType = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
            static auto const& this_type{ util::type_id<BaseType>() };

            auto hash = this_type.hash_code() & cached_type::MAGIC_MASK2;
            auto& out = get_impl(hash);
            if (out.base_hash == 0) {
                if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&out.base_hash), hash, 0) == 0) {
                    out.name = GL::string(std::string_view(this_type.name())).remove_leading_and_trailing(':').remove_suffix(" __cdecl(void)");
                }
            }
            return out;
        };
        // declare and cache the relationship between these two types. 
        template <typename Base, typename Derived> void declare_cpp_derived() {
            static auto& base{ get_impl<Base>() };
            static auto& derived{ get_impl<Derived>() };

            if constexpr (std::is_base_of_v<Base, Derived>) {
                derived.add_base(base.base_hash);
            }
            if constexpr (std::is_base_of_v<Derived, Base>) {
                base.add_base(derived.base_hash);
            }
        };
    };

    class type {
    public:
        // these qualifiers explicitly change the type from the base. 
        enum Qualifiers {
              Const = 1 // tag as a const object, e.g: const std::string
            , Reference = 2 // tag as a reference to an object, e.g: std::string&
            , Temporary = 4 // tag as a temporary object, e.g: std::string&&
            , CppType = 8 // Distinguishes whether this is a scripted type or built-in C++ type. The look-up for the cached info changes depending on this value. 
        };

    protected:
        // combination of the base hash and the qualifiers. Limited to four qualifier types with the current magic mask set-up. 
        size_t hash{ 
            (util::type_id<void>().hash_code() & impl::cached_type::MAGIC_MASK2) | 0x8000000000000000
        }; // e.g. int, long, std::string, or a (registered) scripted type

    public:
        type() = default;
        explicit type(size_t _hash) : hash(_hash) {};
        type(type const&) = default;
        type(type &&) = default;
        type& operator=(type const& rhs) {
            InterlockedExchange(reinterpret_cast<volatile size_t*>(&hash), rhs.hash);
            return *this;
        };
        type& operator=(type && rhs) noexcept {
            InterlockedExchange(reinterpret_cast<volatile size_t*>(&hash), rhs.hash);
            return *this;
        };
        ~type() = default;

        // removes the qualifiers and returns only the base hash value
        size_t get_base_hash() const {
            return hash & impl::cached_type::MAGIC_MASK2;
        };
        // returns the qualifiers attached to this hash, shifted to easily compare with the Qualifiers enum directly. 
        size_t get_qualifiers() const {
            return (hash & impl::cached_type::MAGIC_MASK1) >> 60;
        };
        // atomicly swaps this type with a new hash and qualifier(s)
        void set_qualifiers(size_t qualifiers) {
            InterlockedExchange(reinterpret_cast<volatile size_t*>(&hash), (hash & impl::cached_type::MAGIC_MASK2) | ((qualifiers << 60) & impl::cached_type::MAGIC_MASK1));
        };

        bool is_temp() const noexcept { return hash & 0x4000000000000000; };
        bool is_const() const noexcept { return is_temp() ? false : hash & 0x1000000000000000; };
        bool is_ref() const noexcept { return is_temp() ? false : hash & 0x2000000000000000; };
        bool is_const_ref() const noexcept { return is_temp() ? false : hash & 0x3000000000000000; };
        bool is_base() const noexcept {  return 0 == (hash & ~0x8FFFFFFFFFFFFFFF); };
        bool is_void() const noexcept { return get_base_hash() == (util::type_id<void>().hash_code() & impl::cached_type::MAGIC_MASK2); };
        bool is_cpp_type() const noexcept { return hash & 0x8000000000000000; };
        // returns true if this is found to be a child of the parent type (id'd by its base hash) 
        bool is_derived_from(type const& base) const;
        // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
        bool is_base_of(type const& derived) const;
        // attempts to include the specified hash as a base of this class.
        bool add_base(type const& base);
        bool match_base_hash(type const& to_match) const;

        GL::string name() const; 

        // Operators
        friend bool operator==(const type& a, const type& b) noexcept { return a.hash == b.hash; };
        friend bool operator!=(const type& a, const type& b) noexcept { return a.hash != b.hash; };
        friend bool operator<(const type& a, const type& b) noexcept { return a.hash < b.hash; };
        friend bool operator<=(const type& a, const type& b) noexcept { return a.hash <= b.hash; };
        friend bool operator>(const type& a, const type& b) noexcept { return a.hash > b.hash; };
        friend bool operator>=(const type& a, const type& b) noexcept { return a.hash >= b.hash; };
        bool operator&(size_t p_modifiers) const {
            return get_qualifiers() & p_modifiers;
        };
        type operator|(size_t p_modifiers) const {
            type out = *this;
            out.set_qualifiers(out.get_qualifiers() | p_modifiers);
            return out;
        };
        type& operator|=(size_t p_modifiers) {
            set_qualifiers(get_qualifiers() | p_modifiers);
            return *this;
        };
        type operator+(size_t p_modifiers) const {
            type out = *this;
            out.set_qualifiers(out.get_qualifiers() | p_modifiers);
            return out;
        };
        type& operator+=(size_t p_modifiers) {
            set_qualifiers(get_qualifiers() | p_modifiers);
            return *this;
        };
        type operator-(size_t p_modifiers) const {
            type out = *this;
            out.set_qualifiers(out.get_qualifiers() & ~p_modifiers);
            return out;
        };
        type& operator-=(size_t p_modifiers) {
            set_qualifiers(get_qualifiers() & ~p_modifiers);
            return *this;
        };

    private:
        // Returns true if the types are similar enough to be casted for free (0 cost)
        static bool can_free_cast(type const& from, type const& to) {
            if ((from.get_base_hash() == to.get_base_hash()) || from.match_base_hash(to)) {
                // conversion is possible. 
                if (to.is_const_ref()) return true;

                // cannot cast-away the const-ness
                if (from.is_const() && !to.is_const()) return false;

                // temporary (T&&) can be used for a base cast.
                if (from.is_temp() && to.is_base()) return true;

                // temporary (T&&) cannot be used as const-less references (T&)
                if (from.is_temp() && to.is_ref()) return false;

                // T& cannot cast to T or T&& without a conversion function
                if (from.is_ref() && (to.is_temp() || to.is_base())) return false;

                // Otherwise OK
                return true;
            }
            return false;
        };
    public:
        // Returns true if the types are similar enough to be casted for free (0 cost)
        bool can_free_cast(type const& to) const { 
            return can_free_cast(*this, to); 
        };
        // Returns true if the types are the same foundational type (may not be zero cost to convert)
        bool can_cast(type const& to) const {
            return this->match_base_hash(to);
        };

    };

    template<typename T> __forceinline static GL::type type_of() noexcept {
        using BaseType = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
        static constexpr size_t const_modifier = std::is_const<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>::value ? type::Qualifiers::Const : 0;
        static constexpr size_t ref_modifier = std::is_reference<typename std::remove_pointer<T>::type>::value ? type::Qualifiers::Reference : 0;
        
        static GL::type Base(
            (GL::impl::get_impl<BaseType>().base_hash & impl::cached_type::MAGIC_MASK2) | (((const_modifier | ref_modifier | type::Qualifiers::CppType) << 60ull) & impl::cached_type::MAGIC_MASK1)
        );
        return Base;
    };

    namespace type_erasure {
        template<class T> struct get_type { typedef T type; };
        template<class T> struct get_type<std::shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<std::shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<std::shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const std::shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const std::shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const std::shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::fast_shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::fast_shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::fast_shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::fast_shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::fast_shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::fast_shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::atomic_shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::atomic_shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<GL::atomic_shared_ptr<T>*> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::atomic_shared_ptr<T>> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::atomic_shared_ptr<T>&> { typedef typename get_type<T>::type type; };
        template<class T> struct get_type<const GL::atomic_shared_ptr<T>*> { typedef typename get_type<T>::type type; };

        class any_data {
        protected:
            any_data(GL::type p_actual_type) 
                : m_actual_type{ std::move(p_actual_type) }
                , m_data{ nullptr } 
            {};
        public:
            any_data() = delete;
            any_data(any_data const&) = delete;
            any_data(any_data &&) = delete;
            any_data& operator=(any_data const&) = delete;
            any_data& operator=(any_data&&) = delete;
            virtual ~any_data() = default;

            virtual GL::shared_ptr<void> get(GL::atomic_shared_ptr< any_data >&) = 0;
            virtual std::shared_ptr<void> get_std(GL::atomic_shared_ptr< any_data >&) = 0;
            
            template <typename T>
            T* cast() const {
                return reinterpret_cast<T*>(m_data);
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            virtual bool can_free_cast(type const& to) const = 0;
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            virtual bool can_cast(type const& to) const = 0;

        public:
            GL::type m_actual_type; // atomic type information. May be updated to include information such as the const-ness or temporary type. This should (usually) be the base type. 
            void* m_data; // pointer to the actual data, for quicker access. 

        };

        template <typename T>
        class shared_data final : public any_data {
        public:
            shared_data(GL::shared_ptr<T> p_ptr = {}) 
                : m_ptr(GL::static_pointer_cast<void>(std::move(p_ptr)))
                , any_data(GL::type_of<T>()) 
            {
                this->m_data = m_ptr.get();
            };
            virtual ~shared_data() = default;

            GL::shared_ptr<void> get(GL::atomic_shared_ptr< any_data >&) override {
                return m_ptr;
            };
            std::shared_ptr<void> get_std(GL::atomic_shared_ptr< any_data >& parent) override {
                return std::shared_ptr<void>(this->m_data, [ptr = m_ptr](void* p) -> void {
                    if (p != ptr.get()) {
                        std::cout << "ERROR1\n";
                    }
                });
            };
            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const override {
                return GL::type_of<T>().can_free_cast(to);
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const override {
                return GL::type_of<T>().can_cast(to);
            };

        public:
            GL::shared_ptr< void > m_ptr;

        };
        
        template <typename T>
        class instanced_data final : public any_data {
        public:
            template <typename... TArgs>
            instanced_data(TArgs &&... a) noexcept
                : m_obj(std::forward<TArgs>(a)...)
                , any_data(GL::type_of<T>())
            {                
                this->m_data = &m_obj;
            };
            virtual ~instanced_data() = default;

            GL::shared_ptr<void> get(GL::atomic_shared_ptr< any_data >& parent) override {
                if (auto parent_ptr = parent.load()) {
                    GL::shared_ptr<T> out(parent_ptr.release_control_block(), true);
                    out.set_pointer_without_modifying_control_block(&m_obj);
                    return out;
                }
                return nullptr;
            };
            std::shared_ptr<void> get_std(GL::atomic_shared_ptr< any_data >& parent) override {
                return std::shared_ptr<void>(this->m_data, [ptr = get(parent)](void* p) -> void {
                    if (p != ptr.get()) {
                        std::cout << "ERROR2\n";
                    }
                });
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const override {
                return GL::type_of<T>().can_free_cast(to);
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const override {
                return GL::type_of<T>().can_cast(to);
            };

        public:
            T
                m_obj;
        };

        template <typename T>
        class std_shared_data final : public any_data {
        public:
            std_shared_data(std::shared_ptr<T> && a) noexcept
                : m_obj(std::move(a))
                , any_data(GL::type_of<T>())
            {
                this->m_data = m_obj.get();
            };
            virtual ~std_shared_data() = default;

            GL::shared_ptr<void> get(GL::atomic_shared_ptr< any_data >& parent) override {
                if (auto parent_ptr = parent.load()) {
                    auto* control_block = parent_ptr.release_control_block();
                    GL::shared_ptr<T> out(control_block, true);
                    out.set_pointer_without_modifying_control_block(m_obj.get());
                    return out;
                }
                return nullptr;
            };
            std::shared_ptr<void> get_std(GL::atomic_shared_ptr< any_data >& parent) override {
                return std::static_pointer_cast<void>(m_obj);
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const override {
                return GL::type_of<T>().can_free_cast(to);
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const override {
                return GL::type_of<T>().can_cast(to);
            };

        public:
            std::shared_ptr<T>
                m_obj;
        };

        struct wrapper {
            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, GL::shared_ptr<S>>>>
            static GL::shared_ptr<any_data> get(H<S> obj) {
                if (obj) {
                    if constexpr (std::is_same<GL::any, S>::value) {
                        // return self
                        return obj->m_ptr.load();
                    }
                    else {
                        if constexpr (std::is_same<GL::shared_ptr<S>, H<S>>::value) {
                            return GL::static_pointer_cast<any_data>(GL::make_shared<shared_data<S>>(std::move(obj)));
                        }
                        else if constexpr (std::is_same<std::shared_ptr<S>, H<S>>::value) {
                            return GL::static_pointer_cast<any_data>(GL::make_shared<std_shared_data<S>>(std::move(obj)));
                        }
                        else {
                            return nullptr;
                        }
                    }
                }
                else {
                    return nullptr; // return null if incoming is null
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<GL::any_cast, T>>>
            static GL::shared_ptr<any_data> get(const T& obj) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.container;
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(obj));
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<GL::any_cast, T>>>
            static GL::shared_ptr<any_data> get(T&& obj, int modifier) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.container;
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(std::move(obj)));
                }
            };

            static GL::shared_ptr<any_data> get(const any_cast& obj);
            static GL::shared_ptr<any_data> get(const any_cast* t);
        };

        template<typename T> static GL::shared_ptr<any_data> wrap(const T& r) { return wrapper::get(r); };
        template<typename T> static GL::shared_ptr<any_data> wrap(T&& r) { return wrapper::get(std::move(r)); };

    };

    /* class "Var" is a generic container for dynamically typed objects for use in the scripting language.
    It defers from "Any" because Any objects are for use in C++ to contain statically typed objects.
    "Var" objects are wrappers for Anys that allow the scripting language to process them as
    empty & assignable, or filled and implimented */
    class var {
    public:
        var() = default;
        explicit var(any* data_f) : p_data(data_f) {};
        var(var const&) = default;
        var(var&&) = default;
        var& operator=(var const&) = default;
        var& operator=(var&&) = default;
        ~var() = default;

    public:
        GL::atomic_shared_ptr<any> 
            p_data; // may be "updated" at any time and therefore should be thread-safe. 

    };

    class any {
    public:
        mutable GL::atomic_shared_ptr< type_erasure::any_data >
            m_ptr; // atomic shared-ptr for the type-erased underlying data. 
        GL::type
            m_casted_type; // atomic type information. May be updated to include information such as the const-ness or temporary type. 

    private:
        explicit any(GL::atomic_shared_ptr< type_erasure::any_data > const& p_ptr, GL::type&& p_type)
            : m_ptr{ p_ptr }, m_casted_type{ std::move(p_type) }
        {}

    public:
        any() = default;
        any(any const&) = default;
        any(any&&) noexcept = default;
        any(std::nullptr_t) noexcept : m_ptr{}, m_casted_type{} { };
        any& operator=(any const&) = default;
        any& operator=(any&&) noexcept = default;
        any& operator=(std::nullptr_t) noexcept {
            m_ptr = nullptr;
            m_casted_type = {};
            return *this;
        };
        ~any() = default;

        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any(const ValueType& value) noexcept
            : any(type_erasure::wrap(value), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>())
        {};
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any(ValueType&& value) noexcept
            : any(type_erasure::wrap(std::move(value)), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>())
        {};
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any& operator=(const ValueType& rhs) noexcept {
            m_ptr = type_erasure::wrap(rhs);
            m_casted_type = type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>();
            return *this;
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any& operator=(ValueType&& rhs) noexcept {
            m_ptr = type_erasure::wrap(std::move(rhs));
            m_casted_type = type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>();
            return *this;
        };

        bool operator&(size_t p_modifiers) const {
            return m_casted_type & p_modifiers;
        };
        any operator|(size_t p_modifiers) const {
            return any(m_ptr, m_casted_type | p_modifiers);
        };
        any operator+(size_t p_modifiers) const {
            return any(m_ptr, m_casted_type + p_modifiers);
        };
        any operator-(size_t p_modifiers) const {
            return any(m_ptr, m_casted_type - p_modifiers);
        };
        any& operator|=(size_t p_modifiers) {
            m_casted_type |= p_modifiers;
            return *this;
        };
        any& operator+=(size_t p_modifiers) {
            m_casted_type += p_modifiers;
            return *this;
        };
        any& operator-=(size_t p_modifiers) {
            m_casted_type -= p_modifiers;
            return *this;
        };

        operator bool() const noexcept {
            return m_ptr.operator bool();
        };
        bool empty() const noexcept {
            return !operator bool();
        };
        /*! Empties the Any and frees the memory. */
        void clear() noexcept {
            operator=(nullptr);
        };
        friend bool operator==(const any& a, const any& b) noexcept { return a.m_ptr == b.m_ptr; };
        friend bool operator!=(const any& a, const any& b) noexcept { return a.m_ptr != b.m_ptr; };
        friend bool operator<(const any& a, const any& b) noexcept { return a.m_ptr < b.m_ptr; };
        friend bool operator<=(const any& a, const any& b) noexcept { return a.m_ptr <= b.m_ptr; };
        friend bool operator>(const any& a, const any& b) noexcept { return a.m_ptr > b.m_ptr; };
        friend bool operator>=(const any& a, const any& b) noexcept { return a.m_ptr >= b.m_ptr; };

        // returns true if this type can easily match the requested type (e.g. int& -> const int&)
        bool can_free_cast(type const& to) const {
            if (m_casted_type.can_free_cast(to)) return true;
            //if (auto ptr = m_ptr.load_fast()) {                    
            //    return ptr->m_actual_type.can_free_cast(to);
            //}
            return false;
        };
        // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
        bool can_cast(type const& to) const {
            if (m_casted_type.can_cast(to)) return true;
            if (auto ptr = m_ptr.load_fast()) return ptr->m_actual_type.can_cast(to);
            return false;
        };

        void* ptr() const {
            if (auto p = m_ptr.load_fast())
                if (auto p2 = p.get())
                    return p2->m_data;
            return nullptr;
        };
        GL::shared_ptr<void> shared_ptr() const {
            if (auto p = m_ptr.load_fast()) {
                return p->get(this->m_ptr);
            }
            return nullptr;
        };
        std::shared_ptr<void> std_shared_ptr() const {
            if (auto p = m_ptr.load_fast()) {
                return p->get_std(this->m_ptr);
            }
            return nullptr;
        };

        class DataCaster {
        public:
            template<typename T> struct is_stdSharedPtr_class { typedef std::false_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<const std::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<const std::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<const std::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>&&> { typedef std::true_type type; };

            template<typename T> struct is_SharedPtr_class { typedef std::false_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const GL::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const GL::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const GL::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>&&> { typedef std::true_type type; };

        private:
            template <class VType> static decltype(auto) DoCast_Shared(any* p) noexcept {
                if (p) {
                    return GL::static_pointer_cast<VType>(p->shared_ptr());
                }
                return GL::shared_ptr<VType>{ nullptr };
            };
            template <class VType> static decltype(auto) DoCast_StdShared(any* p) noexcept {
                if (p) {
                    return std::static_pointer_cast<VType>(p->std_shared_ptr());
                }
                return std::shared_ptr<VType>{ nullptr };
            };
            template <class VType> static decltype(auto) DoCast_Shared_Sentinel(any* p) /*noexcept*/ {
                throw("Casting Any to  std::shared_ptr<T>* or  std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
            };
            template<typename VType> static decltype(auto) DoCast_Unshared(any* p) /*noexcept*/ {
                static constexpr bool is_ptr = std::is_pointer_v<VType>;
                if (p) {
                    if (auto container = p->m_ptr.load_fast()) {
                        if constexpr (is_ptr) {
                            if (container->can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                                return container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
                            }
                            else {                                
                                return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*)nullptr;
                            }
                        }
                        else {
                            return *container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
                        }
                    }
                }
                if constexpr (is_ptr) {
                    return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*)nullptr;
                }
                else {
                    auto err = "Cannot cast from void-type to " + GL::type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>().name();
                    throw std::runtime_error(err.to_string());
                }
            };

        public:
            template<typename T> static decltype(auto) DoCast(any* p) /*noexcept*/ {
                typedef typename is_SharedPtr_class<T>::type isShared;
                typedef typename is_stdSharedPtr_class<T>::type isStdShared;

                constexpr bool is_shared_ptr = isShared::value;
                constexpr bool is_std_shared_ptr = isStdShared::value;
                constexpr bool is_ptr = std::is_pointer_v<T>;
                constexpr bool is_ref = std::is_reference_v<T>;

                if (p) {
                    GL::shared_ptr<type_erasure::any_data> container = p->m_ptr.load();
                    while (container) {
                        if constexpr (is_shared_ptr) {
                            // casting to utilities::shared_ptr
                            typedef typename type_erasure::get_type<T>::type innertype;
                            if constexpr (is_ptr) {
                                throw("Casting Any to utilities::shared_ptr<T>* or utilities::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to stutilitiesd::shared_ptr<T>.");
                            }
                            else if constexpr (is_ref) {
                                throw("Casting Any to utilities::shared_ptr<T>* or utilities::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to utilities::shared_ptr<T>.");
                            }

                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_Shared<innertype>(p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (auto f = ptr->p_data.load_fast()) {
                                        container = f->m_ptr.load();
                                        continue;
                                    }
                                }
                                return DoCast_Shared<innertype>(p);
                            }
                        }
                        else if constexpr (is_std_shared_ptr) {
                            // casting to std::shared_ptr
                            typedef typename type_erasure::get_type<T>::type innertype;
                            if constexpr (is_ptr) {
                                throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
                            }
                            else if constexpr (is_ref) {
                                throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
                            }

                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_StdShared<innertype>(p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (auto f = ptr->p_data.load_fast()) {
                                        container = f->m_ptr.load();
                                        continue;
                                    }
                                }
                                return DoCast_StdShared<innertype>(p);
                            }
                        }
                        else {
                            // casting to a reference or a pointer
                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_Unshared<T>(p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (auto f = ptr->p_data.load_fast()) {
                                        container = f->m_ptr.load();
                                        continue;
                                    }
                                }
                                return DoCast_Unshared<T>(p);
                            }
                        }
                    }
                }

                if constexpr (is_shared_ptr) {
                    typedef typename type_erasure::get_type<T>::type innertype;
                    return GL::shared_ptr<innertype>(nullptr);
                }
                else if constexpr (is_std_shared_ptr) {
                    typedef typename type_erasure::get_type<T>::type innertype;
                    return std::shared_ptr<innertype>(nullptr);
                }
                else {
                    if constexpr (is_ptr) {
                        typedef typename type_erasure::get_type<T>::type innertype;
                        return (innertype*)nullptr;
                    }
                    else {
                        auto err = "Cannot cast from void-type to " + GL::type_of<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>().name();
                        throw std::runtime_error(err.to_string());
                    }
                }
            };

        };

        template<typename VType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<any*>(this)); };

        template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value && std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any& cast() const noexcept { return *const_cast<any*>(this); };

        template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value && std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any* cast() const noexcept { return const_cast<any*>(this); };

        any_cast cast() const noexcept;
    };

    class any_cast {
    public:
        any_cast(const any* _parent) : parent(const_cast<any*>(_parent)) {};
        any_cast(any_cast&& other) noexcept : parent(std::move(other.parent)) {};
        any_cast() = delete;
        any_cast(const any_cast&) = delete;
        any_cast& operator=(const any_cast&) = delete;
        any_cast& operator=(any_cast&&) = delete;
        ~any_cast() {};

        explicit operator any& () const noexcept { return *parent; };
        explicit operator any* () const noexcept { return parent; };

        template <typename T> operator std::shared_ptr<T>() const noexcept { return parent->cast<std::shared_ptr<T>>(); };
        template <typename T> operator GL::shared_ptr<T>() const noexcept { return parent->cast<GL::shared_ptr<T>>(); };
        
        template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
        operator ValueTypeT& () const noexcept { return parent->cast<ValueTypeT&>(); };
        
        template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
        operator ValueTypeT* () const noexcept { return parent->cast<ValueTypeT*>(); };

        any* parent;
    };

    __forceinline any_cast any::cast() const noexcept {
        return any_cast(this);
    };


    namespace type_erasure {
        __forceinline GL::shared_ptr<any_data> get(const any_cast& obj) {
            any* t = const_cast<any*>(obj.parent);
            if (t) {
                return t->m_ptr.load();
            }
            return nullptr;
        };
        __forceinline GL::shared_ptr<any_data> get(const any_cast* t) {
            return get(*t); 
        };
    };

#if 0
    // serves as an instance of a customizable class
    class dynamic_object {
    public:
        dynamic_object() = default;
        dynamic_object(GL::type const& type)
            : m_type(type)
            , m_objects()
        {};
        // Cast from one class to another (e.g. from class C : public A {} to class A {})
        dynamic_object(GL::type const& castedType, dynamic_object const& parent)
            : m_type(castedType)
            , m_objects(parent.m_objects)
        {
            if (castedType == parent.m_type) {
                // we are casting to the existing type, which is OK
            }
            else if (castedType.is_derived_from(parent.m_type)) {
                // we are casting from a parent (inherited) type to a derived (child) type, which is OK.                
            }
            else if (parent.m_type.is_derived_from(castedType)) {
                // we are casting from a derived (child) type to a parent (inherited) type, which is OK.
            }
            else {
                // cast was not viable!
                m_type = GL::type_of<void>();
                m_objects.clear();
            }
        };
        dynamic_object(dynamic_object const&) = default;
        dynamic_object(dynamic_object&&) = default;
        dynamic_object& operator=(dynamic_object const&) = default;
        dynamic_object& operator=(dynamic_object&&) = default;
        ~dynamic_object() = default;

        GL::type
            m_type;
        concurrency::concurrent_unordered_map<GL::string, GL::any>
            m_objects;

        any& operator[](GL::string const& sv) {
            return m_objects[sv];
        };
        any const& operator[](GL::string const& sv) const {
            return m_objects.at(sv);
        };
        any* try_at(GL::string const& sv) {
            if (m_objects.count(sv) > 0) {
                return &m_objects.at(sv);
            }
            else {
                return nullptr;
            }
        };
        const any* try_at(GL::string const& sv) const {
            if (m_objects.count(sv) > 0) {
                return &m_objects.at(sv);
            }
            else {
                return nullptr;
            }
        };

    };
#endif

};

#if 0
namespace GL {
    class any;
    class any_cast;

    // serves as an instance of a customizable class
    class dynamic_object {
    public:
        dynamic_object() = default;
        dynamic_object(GL::type const& type)
            : m_type(type)
            , m_objects()
        {};
        // Cast from one class to another (e.g. from class C : public A {} to class A {})
        dynamic_object(GL::type const& castedType, dynamic_object const& parent)
            : m_type(castedType)
            , m_objects(parent.m_objects)
        {
            if (castedType == parent.m_type) {
                // we are casting to the existing type, which is OK
            }
            else if (castedType.is_derived_from(parent.m_type)) {
                // we are casting from a parent (inherited) type to a derived (child) type, which is OK.                
            }
            else if (parent.m_type.is_derived_from(castedType)) {
                // we are casting from a derived (child) type to a parent (inherited) type, which is OK.
            }
            else {
                // cast was not viable!
                m_type = GL::type_of<void>();
                m_objects.clear();
            }
        };
        dynamic_object(dynamic_object const&) = default;
        dynamic_object(dynamic_object&&) = default;
        dynamic_object& operator=(dynamic_object const&) = default;
        dynamic_object& operator=(dynamic_object&&) = default;
        ~dynamic_object() = default;

        GL::type
            m_type;
        concurrency::concurrent_unordered_map<GL::string, GL::any>
            m_objects;

        any& operator[](GL::string const& sv) {
            return m_objects[sv];
        };
        any const& operator[](GL::string const& sv) const {
            return m_objects.at(sv);
        };
        any* try_at(GL::string const& sv) {
            if (m_objects.count(sv) > 0) {
                return &m_objects.at(sv);
            }
            else {
                return nullptr;
            }
        };
        const any* try_at(GL::string const& sv) const {
            if (m_objects.count(sv) > 0) {
                return &m_objects.at(sv);
            }
            else {
                return nullptr;
            }
        };


    };

    /* class "Var" is a generic container for dynamically typed objects for use in the scripting language.
    It defers from "Any" because Any objects are for use in C++ to contain statically typed objects.
    "Var" objects are wrappers for Anys that allow the scripting language to process them as
    empty & assignable, or filled and implimented */
    class var {
    public:
        var() : p_data() {}
        explicit var(any const& data_f) : p_data(GL::make_shared<any>(data_f)) {};
        var(var const&) = default;
        var(var&&) = default;
        var& operator=(var const&) = default;
        var& operator=(var&&) = default;
        ~var() = default;

    public:
        mutable GL::atomic_shared_ptr<any> p_data; // may be "updated" at any time and therefore should be thread-safe. 

    public:
        friend bool operator==(var const& _Left, var const& _Right) {
            auto LHS = _Left.p_data.load_fast();
            auto RHS = _Left.p_data.load_fast();
            return LHS.get() == RHS.get();
        };
        friend bool operator!=(var const& _Left, var const& _Right) {
            return !operator==(_Left, _Right);
        };
    };

    namespace type_erasure {
        class any_data {
        public:
            template<typename T> static std::shared_ptr<void> encode(const std::shared_ptr<T>& ptr_in, int& modifiers_out) {
                if constexpr (std::is_void< T >::value) {
                    modifiers_out |= type::Modifiers::Void;
                }
                if constexpr (std::is_const< T >::value) {
                    modifiers_out |= type::Modifiers::Const;
                }
                if constexpr (std::is_reference< T >::value) {
                    modifiers_out |= type::Modifiers::Reference;
                }
                if constexpr (std::is_same< T, any >::value || std::is_same< T, const any >::value) {
                    modifiers_out |= type::Modifiers::Any;
                }
                return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(ptr_in));
            };
            template<typename T> static std::shared_ptr<void> encode(std::shared_ptr<T>&& ptr_in, int& modifiers_out) {
                if constexpr (std::is_void< T >::value) {
                    modifiers_out |= type::Modifiers::Void;
                }
                if constexpr (std::is_const< T >::value) {
                    modifiers_out |= type::Modifiers::Const;
                }
                if constexpr (std::is_reference< T >::value) {
                    modifiers_out |= type::Modifiers::Reference;
                }
                if constexpr (std::is_same< T, any >::value || std::is_same< T, const any >::value) {
                    modifiers_out |= type::Modifiers::Any;
                }
                return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(std::move(ptr_in)));
            };

        public:
            any_data() noexcept = default;
            any_data(any_data const&) = delete;
            any_data(any_data&&) = delete;
            any_data& operator=(any_data const&) = delete;
            any_data& operator=(any_data&&) = delete;
            virtual ~any_data() = default;
            virtual utilities::type const& actual_type() const = 0;
            virtual utilities::type const& current_type() const = 0;
            virtual void* ptr() const = 0;
            virtual std::shared_ptr<void> const& shared_ptr() const = 0;

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const {
                return current_type().can_free_cast(to) || actual_type().can_free_cast(to);
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const {
                return current_type().can_cast(to) || actual_type().can_cast(to);
            };

            template <typename T> T* cast(bool nothrow = false) const {
                if (nothrow || can_cast(utilities::type_of<T>()))
                    return static_cast<T*>(this->ptr());
                std::string err_msg = GoodLang::printf("Could not cast from %s to %s", this->actual_type().get_name().c_str().data(), utilities::type_of<T>().get_name().c_str().data());
                throw std::runtime_error(err_msg);
            };
            template <typename T> std::shared_ptr<T> cast_shared(bool nothrow = false) const {
                if (nothrow || can_cast(utilities::type_of<T>()))
                    return std::static_pointer_cast<T>(this->shared_ptr());
                std::string err_msg = GoodLang::printf("Could not cast from %s to %s", this->actual_type().get_name().c_str().data(), utilities::type_of<T>().get_name().c_str().data());
                throw std::runtime_error(err_msg);
            };

            virtual utilities::shared_ptr<any_data> operator+(int modifier) const = 0;
            virtual utilities::shared_ptr<any_data> operator+(type const& as_type) const = 0;
        };

        template <typename T> // type-erasure which hosts std::shared_ptr<T>
        class any_data_std_shared final : public any_data {
        public:
            any_data_std_shared() noexcept
                : any_data()
                , m_obj()
                , m_current_type(utilities::type_of<void>())
            {};
            any_data_std_shared(std::shared_ptr<T> const& t_obj, int t_modifiers = 0) noexcept
                : any_data()
                , m_obj(any_data::encode<T>(t_obj, t_modifiers))
            {
                m_current_type = utilities::type_of<T>() + t_modifiers;
            };
            any_data_std_shared(std::shared_ptr<T>&& t_obj, int t_modifiers = 0) noexcept
                : any_data()
                , m_obj(any_data::encode<T>(std::move(t_obj), t_modifiers))
            {
                m_current_type = utilities::type_of<T>() + t_modifiers;
            };
            any_data_std_shared(std::shared_ptr<T>&& t_obj, type const& as_type, int t_modifiers = 0) noexcept
                : any_data()
                , m_obj(any_data::encode<T>(std::move(t_obj), t_modifiers))
            {
                m_current_type = as_type;
            };
            any_data_std_shared(any_data_std_shared const&) = delete;
            any_data_std_shared(any_data_std_shared&&) = delete;
            any_data_std_shared& operator=(any_data_std_shared const&) = delete;
            any_data_std_shared& operator=(any_data_std_shared&&) = delete;
            ~any_data_std_shared() = default;
            utilities::type const& actual_type() const override {
                return utilities::type_of<T>();
            };
            utilities::type const& current_type() const override {
                return m_current_type;
            };
            void* ptr() const override { return m_obj.get(); };
            std::shared_ptr<void> const& shared_ptr() const override {
                return m_obj;
            };
            utilities::shared_ptr<any_data> operator+(int modifier) const override {
                return utilities::make_shared<any_data_std_shared<T>>(std::static_pointer_cast<T>(m_obj), m_current_type.get_modifiers() | modifier);
            };
            utilities::shared_ptr<any_data> operator+(type const& as_type) const override {
                return utilities::make_shared<any_data_std_shared<T>>(std::static_pointer_cast<T>(m_obj), as_type);
            };

        private:
            std::shared_ptr<void> m_obj; // underlying data
            type m_current_type;

        };

        template <typename T> // type-erasure which hosts utilities::shared_ptr<T>
        class any_data_shared final : public any_data {
        public:
            any_data_shared() noexcept
                : any_data()
                , m_obj()
                , m_current_type(utilities::type_of<void>())
            {};
            any_data_shared(utilities::shared_ptr<T> const& t_obj, int t_modifiers = 0) noexcept
                : any_data()
                , m_obj(any_data::encode<T>(std::shared_ptr<T>(const_cast<T*>(t_obj.get()), [ptr = t_obj](T* p) {
                (void)ptr.get();
                    }), t_modifiers))
            {
                m_current_type = utilities::type_of<T>() + t_modifiers;
            };
                    any_data_shared(any_data_shared const&) = delete;
                    any_data_shared(any_data_shared&&) = delete;
                    any_data_shared& operator=(any_data_shared const&) = delete;
                    any_data_shared& operator=(any_data_shared&&) = delete;
                    ~any_data_shared() = default;

                    utilities::type const& actual_type() const override {
                        return utilities::type_of<T>();
                    };
                    utilities::type const& current_type() const override {
                        return m_current_type;
                    };
                    void* ptr() const override { return m_obj.get(); };
                    std::shared_ptr<void> const& shared_ptr() const override {
                        return m_obj;
                    };
                    utilities::shared_ptr<any_data> operator+(int modifier) const override {
                        return utilities::make_shared<any_data_std_shared<T>>(std::static_pointer_cast<T>(m_obj), m_current_type.get_modifiers() | modifier);
                    };
                    utilities::shared_ptr<any_data> operator+(type const& as_type) const override {
                        return utilities::make_shared<any_data_std_shared<T>>(std::static_pointer_cast<T>(m_obj), as_type);
                    };

        private:
            std::shared_ptr<void> m_obj; // underlying data
            type m_current_type;

        };

        template <typename T> // type-erasure which hosts T
        class any_data_instanced final : public any_data {
        public:
            any_data_instanced() noexcept
                : any_data()
                , m_obj()
                , m_current_type(utilities::type_of<void>())
            {};
            any_data_instanced(T const& t_obj, int t_modifiers = 0) noexcept
                : any_data()
                , m_obj(any_data::encode<T>(std::make_shared<T>(t_obj), t_modifiers))
            {
                m_current_type = utilities::type_of<T>() + t_modifiers;
            };
            any_data_instanced(T&& t_obj, int t_modifiers = 0) noexcept
                : any_data()
                , m_obj(any_data::encode<T>(std::make_shared<T>(std::move(t_obj)), t_modifiers))
            {
                m_current_type = utilities::type_of<T>() + t_modifiers;
            };
            any_data_instanced(any_data_instanced const&) = delete;
            any_data_instanced(any_data_instanced&&) = delete;
            any_data_instanced& operator=(any_data_instanced const&) = delete;
            any_data_instanced& operator=(any_data_instanced&&) = delete;
            ~any_data_instanced() = default;
            utilities::type const& actual_type() const override {
                return utilities::type_of<T>();
            };
            utilities::type const& current_type() const override {
                return m_current_type;
            };
            void* ptr() const override { return m_obj.get(); };
            std::shared_ptr<void> const& shared_ptr() const override {
                return m_obj;
            };
            utilities::shared_ptr<any_data> operator+(int modifier) const override {
                return utilities::make_shared<any_data_std_shared<T>>(std::static_pointer_cast<T>(m_obj), m_current_type.get_modifiers() | modifier);
            };
            utilities::shared_ptr<any_data> operator+(type const& as_type) const override {
                return utilities::make_shared<any_data_std_shared<T>>(std::static_pointer_cast<T>(m_obj), as_type);
            };

        private:
            std::shared_ptr<void> m_obj; // underlying data
            type m_current_type;

        };

        struct wrapper {
            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, utilities::shared_ptr<S>>>>
            static utilities::shared_ptr<any_data> get(H<S> obj, int modifier) {
                if (obj) {
                    if constexpr (std::is_same<utilities::any, S>::value) {
                        // return self
                        if (modifier == 0 || !obj->container) {
                            return obj->container;
                        }
                        else {
                            return *obj->container + modifier;
                        }
                    }
                    else {
                        if constexpr (std::is_same<utilities::shared_ptr<S>, H<S>>::value) {
                            return utilities::make_shared<any_data_shared<S>>(obj, modifier);
                        }
                        else if constexpr (std::is_same<std::shared_ptr<S>, H<S>>::value) {
                            return utilities::make_shared<any_data_std_shared<S>>(obj, modifier);
                        }
                        else {
                            return nullptr;
                        }
                    }
                }
                else {
                    return nullptr; // return null if incoming is null
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<any_cast, T>>>
            static utilities::shared_ptr<any_data> get(const T& obj, int modifier) {
                if constexpr (std::is_same<utilities::any, T>::value) {
                    // return self
                    if (modifier == 0 || !obj.container) {
                        return obj.container;
                    }
                    else {
                        return *obj.container + modifier;
                    }
                }
                else {
                    return utilities::make_shared<any_data_instanced<T>>(obj, modifier);
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<any_cast, T>>>
            static utilities::shared_ptr<any_data> get(T&& obj, int modifier) {
                if constexpr (std::is_same<utilities::any, T>::value) {
                    // return self
                    if (modifier == 0 || !obj.container) {
                        return obj.container;
                    }
                    else {
                        return *obj.container + modifier;
                    }
                }
                else {
                    return utilities::make_shared<any_data_instanced<T>>(std::move(obj), modifier);
                }
            };


            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, utilities::shared_ptr<S>>>>
            static utilities::shared_ptr<any_data> get(H<S> obj, type const& modifier) {
                if (obj) {
                    if constexpr (std::is_same<utilities::any, S>::value) {
                        // return self
                        if (!obj->container) {
                            return obj->container;
                        }
                        else {
                            return *obj->container + modifier;
                        }
                    }
                    else {
                        if constexpr (std::is_same<utilities::shared_ptr<S>, H<S>>::value) {
                            return utilities::make_shared<any_data_shared<S>>(obj, modifier);
                        }
                        else if constexpr (std::is_same<std::shared_ptr<S>, H<S>>::value) {
                            return utilities::make_shared<any_data_std_shared<S>>(obj, modifier);
                        }
                        else {
                            return nullptr;
                        }
                    }
                }
                else {
                    return nullptr; // return null if incoming is null
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<any_cast, T>>>
            static utilities::shared_ptr<any_data> get(const T& obj, type const& modifier) {
                if constexpr (std::is_same<utilities::any, T>::value) {
                    // return self
                    if (!obj.container) {
                        return obj.container;
                    }
                    else {
                        return *obj.container + modifier;
                    }
                }
                else {
                    return utilities::make_shared<any_data_instanced<T>>(obj, modifier);
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<any_cast, T>>>
            static utilities::shared_ptr<any_data> get(T&& obj, type const& modifier) {
                if constexpr (std::is_same<utilities::any, T>::value) {
                    // return self
                    if (!obj.container) {
                        return obj.container;
                    }
                    else {
                        return *obj.container + modifier;
                    }
                }
                else {
                    return utilities::make_shared<any_data_instanced<T>>(std::move(obj), modifier);
                }
            };

            static utilities::shared_ptr<any_data> get(const any_cast& obj);
            static utilities::shared_ptr<any_data> get(const any_cast* t);
        };

        template<typename T> static utilities::shared_ptr<any_data> wrap(const T& r, int modifier = 0) { return wrapper::get(r, modifier); };
        template<typename T> static utilities::shared_ptr<any_data> wrap(T&& r, int modifier = 0) { return wrapper::get(std::move(r), modifier); };

        template<typename T> static utilities::shared_ptr<any_data> wrap(const T& r, type const& current_type) { return wrapper::get(r, current_type); };
        template<typename T> static utilities::shared_ptr<any_data> wrap(T&& r, type const& current_type) { return wrapper::get(std::move(r), current_type); };
    };

    /* Type-erasure wrapper for sharing literal or shared_ptr objects while managing the intended type (e.g. const, const ref, temp) seperately from the actual object (e.g. a literal).
    Thread-safe for overwritting overwriting, clearing, casting, etc.
    This thread-safety comes at a cost, however, and is about 10x less memory- and CPU-performant than the single-threaded version. */
    class any {
    public:
        any() noexcept
            : container(nullptr)
        {};
        any(std::nullptr_t) noexcept
            : container(nullptr)
        {};
        any(const any& rhs) noexcept
            : container()
        {
            container = rhs.container;
        };
        any(any&& rhs) noexcept
            : container(std::move(rhs.container))
        {};
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any(const ValueType& value, int modifier = 0) noexcept
            : container(type_erasure::wrap(value, modifier))
        {};
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any(ValueType&& value, int modifier = 0) noexcept
            : container(type_erasure::wrap(std::move(value), modifier))
        {};
        any& operator=(std::nullptr_t) noexcept {
            container = nullptr;
            return *this;
        };
        any& operator=(const any& rhs) noexcept {
            container = rhs.container;
            return *this;
        };
        any& operator=(any&& rhs) noexcept {
            container = std::move(rhs.container);
            return *this;
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any& operator=(const ValueType& rhs) noexcept {
            container = type_erasure::wrap(rhs);
            return *this;
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> any& operator=(ValueType&& rhs) noexcept {
            container = type_erasure::wrap(std::move(rhs));
            return *this;
        };
        ~any() = default;

        any operator+(int modifier) const {
            any out;
            out.container = type_erasure::wrap(*this, current_type().get_modifiers() | modifier);
            return out;
        };
        template<typename FromType> any polymorphic_cast(type const& new_type) const {
            any out;
            if (this->actual_type().is_parent_of(new_type) || this->actual_type().is_child_of(new_type)) {
                out.container = type_erasure::wrap(*this, new_type);
            }
            else {
                // throw std::runtime_error("Could not perform the requested polymorphic cast -- the types were not connected by family tree.");
            }
            return out;
        };

    public:
        /*! Checks if the Any has been assigned something */
        bool empty() const noexcept {
            return !container.operator bool();
        };
        /*! Empties the Any and frees the memory. */
        void clear() noexcept {
            container = nullptr;
        };
        //
        utilities::type const& actual_type() const noexcept {
            if (auto* p = container.get()) {
                return p->actual_type();
            }
            else {
                return utilities::type_of<void>();
            }
        };
        utilities::type const& current_type() const noexcept {
            if (auto* p = container.get()) {
                return p->current_type();
            }
            else {
                return utilities::type_of<void>();
            }
        };
        // returns true if this type can easily match the requested type (e.g. int& -> const int&)
        bool can_free_cast(type const& to) const {
            return current_type().can_free_cast(to) || actual_type().can_free_cast(to);
        };
        // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
        bool can_cast(type const& to) const {
            return current_type().can_cast(to) || actual_type().can_cast(to);
        };

#pragma region Boolean Operators
    public:
        explicit operator bool() const { return (bool)container; };
        friend bool operator==(const any& a, const any& b) noexcept { return a.container == b.container; };
        friend bool operator!=(const any& a, const any& b) noexcept { return a.container != b.container; };
        friend bool operator<(const any& a, const any& b) noexcept { return a.container < b.container; };
        friend bool operator<=(const any& a, const any& b) noexcept { return a.container <= b.container; };
        friend bool operator>(const any& a, const any& b) noexcept { return a.container > b.container; };
        friend bool operator>=(const any& a, const any& b) noexcept { return a.container >= b.container; };
        friend bool operator==(const any& a, std::nullptr_t) noexcept { return a.container == nullptr; };
        friend bool operator!=(const any& a, std::nullptr_t) noexcept { return a.container != nullptr; };
        friend bool operator<(const any& a, std::nullptr_t) noexcept { return a.container < nullptr; };
        friend bool operator<=(const any& a, std::nullptr_t) noexcept { return a.container <= nullptr; };
        friend bool operator>(const any& a, std::nullptr_t) noexcept { return a.container > nullptr; };
        friend bool operator>=(const any& a, std::nullptr_t) noexcept { return a.container >= nullptr; };
        friend bool operator==(std::nullptr_t, const any& a) noexcept { return nullptr == a.container; };
        friend bool operator!=(std::nullptr_t, const any& a) noexcept { return nullptr != a.container; };
        friend bool operator<(std::nullptr_t, const any& a) noexcept { return nullptr < a.container; };
        friend bool operator<=(std::nullptr_t, const any& a) noexcept { return nullptr <= a.container; };
        friend bool operator>(std::nullptr_t, const any& a) noexcept { return nullptr > a.container; };
        friend bool operator>=(std::nullptr_t, const any& a) noexcept { return nullptr >= a.container; };
#pragma endregion

    public:
        class DataCaster {
        public:
            template<typename T> struct is_stdSharedPtr_class { typedef std::false_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<const std::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<const std::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<const std::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_stdSharedPtr_class<std::shared_ptr<T>&&> { typedef std::true_type type; };

            template<typename T> struct is_SharedPtr_class { typedef std::false_type type; };
            template<typename T> struct is_SharedPtr_class<utilities::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<utilities::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<utilities::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const utilities::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const utilities::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const utilities::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<utilities::shared_ptr<T>&&> { typedef std::true_type type; };

        private:
            template <class VType> static decltype(auto) DoCast_Shared(any* p) noexcept {
                if (p && p->container) {
                    std::shared_ptr<VType> data = p->container->cast_shared<VType>();
                    return utilities::shared_ptr<VType>(data.get(), [P = data](VType*) -> void {});
                }
                else {
                    return utilities::shared_ptr<VType>{ nullptr };
                }
            };
            template <class VType> static decltype(auto) DoCast_StdShared(any* p) noexcept {
                if (p && p->container) {
                    return p->container->cast_shared<VType>();
                }
                else {
                    return std::shared_ptr<VType>{ nullptr };
                }
            };
            template <class VType> static decltype(auto) DoCast_Shared_Sentinel(any* p) noexcept {
                throw("Casting Any to  std::shared_ptr<T>* or  std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
            };
            template<typename VType> static decltype(auto) DoCast_Unshared(any* p) noexcept {
                constexpr bool is_ptr = std::is_pointer_v<VType>;
                if (p && p->container) {
                    if constexpr (is_ptr) {
                        if (p->container->can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                            return p->container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
                        }
                        else {
                            return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*)nullptr;
                        }
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
                        auto str = GoodLang::printf("Cannot cast from void-type to %s-type", utilities::type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>().get_name().c_str().data());
                        throw std::runtime_error(str);
                    }
                }
            };

        public:
            template<typename T> static decltype(auto) DoCast(any* p) noexcept {
                typedef typename is_SharedPtr_class<T>::type isShared;
                typedef typename is_stdSharedPtr_class<T>::type isStdShared;

                constexpr bool is_shared_ptr = isShared::value;
                constexpr bool is_std_shared_ptr = isStdShared::value;
                constexpr bool is_ptr = std::is_pointer_v<T>;
                constexpr bool is_ref = std::is_reference_v<T>;

                if (p) {
                    while (auto container = p ? p->container : nullptr) {
                        if constexpr (is_shared_ptr) {
                            // casting to utilities::shared_ptr
                            typedef typename type_erasure::get_type<T>::type innertype;
                            if constexpr (is_ptr) {
                                throw("Casting Any to utilities::shared_ptr<T>* or utilities::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to stutilitiesd::shared_ptr<T>.");
                            }
                            else if constexpr (is_ref) {
                                throw("Casting Any to utilities::shared_ptr<T>* or utilities::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to utilities::shared_ptr<T>.");
                            }

                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_Shared<innertype>(p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (ptr->p_data) {
                                        p = &*ptr->p_data;
                                        continue;
                                    }
                                }
                                return DoCast_Shared<innertype>(p);
                            }
                        }
                        else if constexpr (is_std_shared_ptr) {
                            // casting to std::shared_ptr
                            typedef typename type_erasure::get_type<T>::type innertype;
                            if constexpr (is_ptr) {
                                throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
                            }
                            else if constexpr (is_ref) {
                                throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
                            }

                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_StdShared<innertype>(p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (ptr->p_data) {
                                        p = &*ptr->p_data;
                                        continue;
                                    }
                                }
                                return DoCast_StdShared<innertype>(p);
                            }
                        }
                        else {
                            // casting to a reference or a pointer
                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_Unshared<T>(p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (ptr->p_data) {
                                        p = &*ptr->p_data;
                                        continue;
                                    }
                                }
                                return DoCast_Unshared<T>(p);
                            }
                        }
                    }
                }
                if constexpr (is_shared_ptr) {
                    typedef typename type_erasure::get_type<T>::type innertype;
                    return DoCast_Shared<innertype>(nullptr);
                }
                else if constexpr (is_std_shared_ptr) {
                    typedef typename type_erasure::get_type<T>::type innertype;
                    return DoCast_StdShared<innertype>(nullptr);
                }
                else {
                    return DoCast_Unshared<T>(nullptr);
                }
            };

        };

        void* ptr() const noexcept {
            if (auto* p = container.get()) {
                return p->ptr();
            }
            else {
                return nullptr;
            }
        };

        template<typename VType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<any*>(this)); };

        template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any& cast() const noexcept { return *const_cast<any*>(this); };

        template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any* cast() const noexcept { return const_cast<any*>(this); };

        any_cast cast() const noexcept;

    public:
        utilities::shared_ptr<type_erasure::any_data> container;

    };
    class any_cast {
    public:
        any_cast(const any* _parent) : parent(const_cast<any*>(_parent)) {};
        any_cast(any_cast&& other) : parent(std::move(other.parent)) {};
        any_cast() = delete;
        any_cast(const any_cast&) = delete;
        any_cast& operator=(const any_cast&) = delete;
        any_cast& operator=(any_cast&&) = delete;
        ~any_cast() {};

        explicit operator any& () const noexcept { return *parent; };
        explicit operator any* () const noexcept { return parent; };
        template <typename T> operator std::shared_ptr<T>() const noexcept { return parent->cast<std::shared_ptr<T>>(); };
        template <typename T> operator utilities::shared_ptr<T>() const noexcept { return parent->cast<std::shared_ptr<T>>(); };
        template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
        operator ValueTypeT& () const noexcept { return parent->cast<ValueTypeT&>(); };
        template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
        operator ValueTypeT* () const noexcept { return parent->cast<ValueTypeT*>(); };

        any* parent;
    };

    __forceinline any_cast any::cast() const noexcept {
        return any_cast(this);
    };
    namespace type_erasure {
        __forceinline utilities::shared_ptr<any_data> get(const any_cast& obj) {
            any* t = const_cast<any*>(obj.parent);
            if (t) {
                return t->container;
            }
            return nullptr;
        };
        __forceinline utilities::shared_ptr<any_data> get(const any_cast* t) { return get(*t); };
    };

    // returns a shareable, type-erased object that will never delete the underlying object. Intended for using static objects or references to C++ objects in the scripting language. 
    template <typename T> static any ref(T& static_object) {
        return any(std::shared_ptr<T>(&static_object, [](T*) { /* do nothing */ }));
    };



};
#endif
