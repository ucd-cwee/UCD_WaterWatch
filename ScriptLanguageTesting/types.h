#pragma once

#include <set>
#include "Strings.h"
#include "atomic_shared_ptr.h"
#include <boost/type_index.hpp>
#include <concurrent_unordered_map.h>

// implimentatin of atomic type info and atomic any
namespace GL {
    class any;
    namespace type_erasure {
        class any_cast;
    };

    namespace util {
        template<typename T> static const auto& type_id() {
            static auto const& typeIdOfT{ boost::typeindex::type_id<T>().type_info() };
            return typeIdOfT;
        };
    };

    namespace impl {
        template <typename T>
        struct instance_funcs {
            // const value_t& to This&&
            static GL::any instance_by_value(GL::any const&);
            // const This& to This&&
            static GL::any instance_by_copy(GL::any const&);
            // This&&
            static GL::any instance();
            // p->~T();
            static void destroy(void*);
        };

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
            size_t
                T_size{ 0 };

            GL::any(*instance_by_value)(GL::any const&); // const value_t& to This&&
            GL::any(*instance_by_copy)(GL::any const&); // const This& to This&&
            GL::any(*instance)(); // This&&
            void(*destroy)(void*); // // p->~T();

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
                    out.instance_by_value = &instance_funcs<T>::instance_by_value;
                    out.instance_by_copy = &instance_funcs<T>::instance_by_copy;
                    out.instance = &instance_funcs<T>::instance;
                    out.destroy = &instance_funcs<T>::destroy;
                    if constexpr (!std::is_void_v<T>) out.T_size = sizeof(BaseType);
                    else out.T_size = 0;
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
        size_t size() const;

        GL::any instance_by_value(GL::any const&) const; // const value_t& to This&&
        GL::any instance_by_copy(GL::any const&) const; // const This& to This&&
        GL::any instance() const; // This&&
        void destroy(void*) const; // p->~T();
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

            template<typename T, typename = std::enable_if_t<!std::is_same_v<GL::type_erasure::any_cast, T>>>
            static GL::shared_ptr<any_data> get(const T& obj) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.container;
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(obj));
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<GL::type_erasure::any_cast, T>>>
            static GL::shared_ptr<any_data> get(T&& obj, int modifier) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.container;
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(std::move(obj)));
                }
            };

            static GL::shared_ptr<any_data> get(const type_erasure::any_cast& obj);
            static GL::shared_ptr<any_data> get(const type_erasure::any_cast* t);
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
        explicit var(GL::shared_ptr<any> && data_f) : p_data(std::move(data_f)) {};
        var(var const&) = default;
        var(var&&) = default;
        var& operator=(var const&) = default;
        var& operator=(var&&) = default;
        ~var() = default;

    public:
        GL::atomic_shared_ptr<any> 
            p_data; // may be "updated" at any time and therefore should be thread-safe. 

    };

    // atomic wrapper for any object that can store and share shared_ptrs. Handles automatic casting to nearly all variants of qualified types. 
    class any {
    protected:
        mutable GL::atomic_shared_ptr< type_erasure::any_data >
            m_ptr; // atomic shared-ptr for the type-erased underlying data. 
    public:
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

    private:
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
            friend class type_erasure::any_cast;
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
            template <class VType> static decltype(auto) DoCast_Shared(GL::fast_shared_ptr<type_erasure::any_data> & ptr, any* p) noexcept {
                if (p) {
                    return GL::static_pointer_cast<VType>(ptr->get(p->m_ptr));
                }
                return GL::shared_ptr<VType>{ nullptr };
            };
            template <class VType> static decltype(auto) DoCast_StdShared(GL::fast_shared_ptr<type_erasure::any_data> & ptr, any* p) noexcept {
                if (p) {
                    return std::static_pointer_cast<VType>(ptr->get_std(p->m_ptr));
                }
                return std::shared_ptr<VType>{ nullptr };
            };
            template<typename VType> static decltype(auto) DoCast_Unshared(GL::fast_shared_ptr<type_erasure::any_data> & container) /*noexcept*/ {
                static constexpr bool is_ptr = std::is_pointer_v<VType>;
                //if (container) {
                    if constexpr (is_ptr) {
                        //if (container->can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                            // return container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
                            return reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container->m_data);
                        //}
                        //else {                                
                        //    return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*)nullptr;
                        //}
                    }
                    else {
                        return *reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container->m_data);
                        // return *container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
                    }                    
                //}
                //if constexpr (is_ptr) {
                //    return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*)nullptr;
                //}
                //else {
                //    auto err = "Cannot cast from void-type to " + GL::type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>().name();
                //    throw std::runtime_error(err.to_string());
                //}
            };

        public:
            template<typename T> static decltype(auto) DoCast(any* p) /*noexcept*/ {
                typedef typename is_SharedPtr_class<T>::type isShared;
                typedef typename is_stdSharedPtr_class<T>::type isStdShared;

                constexpr bool is_shared_ptr = isShared::value;
                constexpr bool is_std_shared_ptr = isStdShared::value;
                constexpr bool is_ptr = std::is_pointer_v<T>;
                constexpr bool is_ref = std::is_reference_v<T>;
                constexpr bool is_const = std::is_const_v<T>;

                if (p) {
                    GL::fast_shared_ptr<type_erasure::any_data> container{ p->m_ptr.load_fast() };
                    while (container) {
                        if constexpr (is_shared_ptr) {
                            // casting to utilities::shared_ptr
                            typedef typename type_erasure::get_type<T>::type innertype;
                            if constexpr (is_ptr) {
                                throw("Casting Any to utilities::shared_ptr<T>* or utilities::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to stutilitiesd::shared_ptr<T>.");
                            }
                            else if constexpr (is_ref && !is_const) {
                                throw("Casting Any to utilities::shared_ptr<T>* or utilities::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to utilities::shared_ptr<T>.");
                            }

                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_Shared<innertype>(container, p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (auto f = ptr->p_data.load_fast()) {
                                        container = f->m_ptr.load_fast();
                                        continue;
                                    }
                                }
                                return DoCast_Shared<innertype>(container, p);
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
                                return DoCast_StdShared<innertype>(container, p);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (auto f = ptr->p_data.load_fast()) {
                                        container = f->m_ptr.load_fast();
                                        continue;
                                    }
                                }
                                return DoCast_StdShared<innertype>(container, p);
                            }
                        }
                        else {
                            // casting to a reference or a pointer
                            if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                return DoCast_Unshared<T>(container);
                            }
                            else {
                                if (container->can_cast(type_of<var>())) {
                                    var* ptr = container->cast<var>();
                                    if (auto f = ptr->p_data.load_fast()) {
                                        container = f->m_ptr.load_fast();
                                        continue;
                                    }
                                }
                                return DoCast_Unshared<T>(container);
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

    public:
        template<typename VType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<any*>(this)); };

        template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value && std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any& cast() const noexcept { return *const_cast<any*>(this); };

        template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value && std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any* cast() const noexcept { return const_cast<any*>(this); };

        type_erasure::any_cast cast() const noexcept;

        friend class type_erasure::any_cast;
        friend struct type_erasure::wrapper;
    };

    namespace type_erasure {
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
    };

    __forceinline type_erasure::any_cast any::cast() const noexcept {
        return type_erasure::any_cast(this);
    };

    namespace type_erasure {
        __forceinline GL::shared_ptr<any_data> wrapper::get(const any_cast& obj) {
            any* t = const_cast<any*>(obj.parent);
            if (t) {
                return t->m_ptr.load();
            }
            return nullptr;
        };
        __forceinline GL::shared_ptr<any_data> wrapper::get(const any_cast* t) {
            return get(*t); 
        };
    };

    // serves as an instance of a customizable class
    class dynamic_object {
    public:
        dynamic_object() = default;
        dynamic_object(GL::type const& type)
            : m_type(type)
            , m_objects()
        {
            //if (type.is_cpp_type()) {
            //    throw std::runtime_error("Should not use a cpp-type with a dynamic object.");
            //}
        };
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

    namespace impl {
        // const value_t& to This&&
        template <typename T> __forceinline GL::any instance_funcs<T>::instance_by_value(GL::any const& from) {
            // try to cast to this object from a numeric value. 
            using base_type = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
            if constexpr (std::is_same_v<base_type, GL::any>) {
                return from;
            }

            size_t base_hash = from.m_casted_type.get_base_hash();
            static size_t double_hash = GL::util::type_id<double>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t float_hash = GL::util::type_id<float>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t long_hash = GL::util::type_id<long>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t int_hash = GL::util::type_id<int>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t char_hash = GL::util::type_id<char>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t ldouble_hash = GL::util::type_id<long double>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t llong_hash = GL::util::type_id<long long>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t ulong_hash = GL::util::type_id<unsigned long>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t size_t_hash = GL::util::type_id<unsigned long long>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t uchar_hash = GL::util::type_id<unsigned char>().hash_code() & cached_type::MAGIC_MASK2;
            static size_t uint_hash = GL::util::type_id<unsigned int>().hash_code() & cached_type::MAGIC_MASK2;

            if (base_hash == double_hash) {
                if constexpr (std::is_constructible_v<base_type, double&>) {
                    GL::any out(static_cast<base_type>(from.cast<double>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == float_hash) {
                if constexpr (std::is_constructible_v<base_type, float&>) {
                    GL::any out(static_cast<base_type>(from.cast<float>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == long_hash) {
                if constexpr (std::is_constructible_v<base_type, long&>) {
                    GL::any out(static_cast<base_type>(from.cast<long>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == int_hash) {
                if constexpr (std::is_constructible_v<base_type, int&>) {
                    GL::any out(static_cast<base_type>(from.cast<int>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == char_hash) {
                if constexpr (std::is_constructible_v<base_type, char&>) {
                    GL::any out(static_cast<base_type>(from.cast<char>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == ldouble_hash) {
                if constexpr (std::is_constructible_v<base_type, long double&>) {
                    GL::any out(static_cast<base_type>(from.cast<long double>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == llong_hash) {
                if constexpr (std::is_constructible_v<base_type, long long&>) {
                    GL::any out(static_cast<base_type>(from.cast<long long>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == ulong_hash) {
                if constexpr (std::is_constructible_v<base_type, unsigned long&>) {
                    GL::any out(static_cast<base_type>(from.cast<unsigned long>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == size_t_hash) {
                if constexpr (std::is_constructible_v<base_type, size_t&>) {
                    GL::any out(static_cast<base_type>(from.cast<size_t>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == uchar_hash) {
                if constexpr (std::is_constructible_v<base_type, unsigned char&>) {
                    GL::any out(static_cast<base_type>(from.cast<unsigned char>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            else if (base_hash == uint_hash) {
                if constexpr (std::is_constructible_v<base_type, unsigned int&>) {
                    GL::any out(static_cast<base_type>(from.cast<unsigned int>()));
                    out |= GL::type::Qualifiers::Temporary;
                    return out;
                }
            }
            return {};
        };
        // const This& to This&&
        template <typename T> __forceinline GL::any instance_funcs<T>::instance_by_copy(GL::any const& from) {
            using base_type = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
            if constexpr (std::is_same_v<base_type, GL::any>) {
                return from;
            }
            else if constexpr (std::is_copy_constructible_v<base_type>) {
                GL::any out(base_type{ from.cast<T>() });
                out |= GL::type::Qualifiers::Temporary;
                return out;
            }
            else if constexpr (std::is_copy_assignable_v<base_type>) {
                GL::any out(base_type{  });
                out.cast<T>() = from.cast<T>();
                out |= GL::type::Qualifiers::Temporary;
                return out;
            }
            else if constexpr (std::is_constructible_v< base_type>) {
                GL::any out(base_type{  });
                out |= GL::type::Qualifiers::Temporary;
                return out;
            }
            return {};
        };
        // This&&
        template <typename T> __forceinline GL::any instance_funcs<T>::instance() {
            using base_type = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
            if constexpr (std::is_constructible_v< base_type>) {
                GL::any out(base_type{  });
                out |= GL::type::Qualifiers::Temporary;
                return out;
            }
            return {};
        };
        // This&&
        template <typename T> __forceinline void instance_funcs<T>::destroy(void* p) {
            using base_type = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*            
            reinterpret_cast<T*>(p)->~T();
        };
    }

};

