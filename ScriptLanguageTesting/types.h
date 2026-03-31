#pragma once

#include <vector>
#include <set>
#include <map>
#include "Strings.h"
#include "atomic_shared_ptr.h"
#include <boost/type_index.hpp>
#include <concurrent_unordered_map.h>

// implimentatin of atomic type info and atomic any
namespace GL {
    class any;
    class var;
    class dynamic_object;

    namespace type_erasure {
        struct any_cast;
        struct fast_any_cast;
    };

    namespace util {
        template<typename T> static const auto& type_id() {
            // static auto const& typeIdOfT{ boost::typeindex::type_id<T>().type_info() };
            static auto typeIdOfT{ GL::util::type_hash<T>() };
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
            std::vector<size_t> // hashes to the underlying base classes to this type
                base_classes_ordered{};
            size_t // without const, ref, etc. 
                base_hash{ 0 };
            size_t
                T_size{ 0 };

            bool is_cpp_type() const {
                return T_size != std::numeric_limits<size_t>::max();
            };
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

    // atomic type information about a C++ or scripted class 
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
        size_t hash; // e.g. int, long, std::string, or a (registered) scripted type
        static size_t const& any_hash_code() {
            static size_t out{ util::type_id<any>().hash_code() & impl::cached_type::MAGIC_MASK2 };
            return out;
        };
        static size_t const& void_hash_code() {
            static size_t out{ util::type_id<void>().hash_code() & impl::cached_type::MAGIC_MASK2 };
            return out;
        };
        static size_t const& default_hash_code() {
            static size_t out{ (util::type_id<void>().hash_code() & impl::cached_type::MAGIC_MASK2) | 0x8000000000000000 };
            return out;
        };

    public:
        type() : hash(default_hash_code()) {};
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

        std::vector<type> all_base_types(bool local_only = true) const;
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
            if ((qualifiers & Temporary) > 0) {
                qualifiers &= ~Const;
                qualifiers &= ~Reference;
            }
            InterlockedExchange(reinterpret_cast<volatile size_t*>(&hash), (hash & impl::cached_type::MAGIC_MASK2) | ((qualifiers << 60) & impl::cached_type::MAGIC_MASK1));
        };

        bool is_temp() const noexcept { return ((hash & 0x4000000000000000) > 0); };
        bool is_const() const noexcept { return /*is_temp() ? false : */((hash & 0x1000000000000000) > 0); };
        bool is_ref() const noexcept { return /*is_temp() ? false : */((hash & 0x2000000000000000) > 0); };
        bool is_const_ref() const noexcept { return /*is_temp() ? false : */((hash & 0x3000000000000000) == 0x3000000000000000); };
        bool is_base() const noexcept {  return 0 == (hash & ~0x8FFFFFFFFFFFFFFF); };
        bool is_void() const noexcept {
            thread_local size_t const h{ void_hash_code() };
            return get_base_hash() == h;
        };
        bool is_cpp_type() const noexcept { return (hash & 0x8000000000000000) > 0; };
        bool is_any() const noexcept;
        // returns true if this is found to be a child of the parent type (id'd by its base hash) 
        bool is_derived_from(type const& base) const;
        // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
        bool is_base_of(type const& derived) const;
        // attempts to include the specified hash as a base of this class.
        bool add_base(type const& base);
        bool match_base_hash(type const& to_match) const;

        GL::string name() const;
        // this is NOT a thread-safe operation. Ideally should be done on start-up. 
        bool try_update_name(GL::string const& new_name);

        size_t get_hash() const {
            return hash;
        };

        // Operators
        friend bool operator==(const type& a, const type& b) noexcept { return a.hash == b.hash; };
        friend bool operator!=(const type& a, const type& b) noexcept { return a.hash != b.hash; };
        friend bool operator<(const type& a, const type& b) noexcept { return a.hash < b.hash; };
        friend bool operator<=(const type& a, const type& b) noexcept { return a.hash <= b.hash; };
        friend bool operator>(const type& a, const type& b) noexcept { return a.hash > b.hash; };
        friend bool operator>=(const type& a, const type& b) noexcept { return a.hash >= b.hash; };
        bool operator&(size_t p_modifiers) const {
            // return hash & ((p_modifiers << 60ull) & impl::cached_type::MAGIC_MASK1);
            return (get_qualifiers() & p_modifiers) > 0;
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
        __declspec(noinline) static bool can_free_cast(type const& from, type const& to, bool allow_polymorphic = true) {
            if (from.hash == to.hash) return true;

            if (from.get_base_hash() == to.get_base_hash()) {
                // conversion is possible and does not use polymorphism. 
                if (to.is_const_ref()) return true;

                // cannot cast-away the const-ness
                if (from.is_const() && !to.is_const()) return false;

                // temporary (T&&) cannot be used as const-less references (T&)
                if (from.is_temp() && to.is_ref()) return false;

                // temporary (T&&) cannot be used for a base cast (requires a copy)
                if (from.is_temp() && to.is_base()) return false;

                // T& cannot cast to T or T&& without a conversion function
                if (from.is_ref() && (to.is_temp() || to.is_base())) return false;

                // Otherwise OK
                return true;
            }

            if (allow_polymorphic) {
                if (from.match_base_hash(to)) {
                    // conversion uses polymorphism. 
                    // cannot be used for a base cast (requires a copy in some way)
                    if (to.is_base()) return false;

                    if (from.is_temp()) {
                        return to.is_temp() || to.is_const_ref();
                    }

                    // cannot cast-away the const-ness
                    if (from.is_const() && !to.is_const()) return false;

                    // conversion is possible
                    if (to.is_const_ref()) return true;

                    // T& cannot cast to T or T&& without a conversion function
                    if (from.is_ref() && to.is_temp()) return false;

                    // Otherwise OK
                    return true;
                }
            }

            if (from.is_any() || to.is_any()) {
                return !from.is_void() && !to.is_void();
            }

            return from.is_void() && to.is_void();
        };
    public:
        // Returns true if the types are similar enough to be casted for free (0 cost)
        bool can_free_cast(type const& to, bool allow_polymorphic = true) const {
            return can_free_cast(*this, to, allow_polymorphic);
        };
        // Returns true if the types are the same foundational type (may not be zero cost to convert)
        bool can_cast(type const& to) const {
            if (this->match_base_hash(to)) {
                return true;
            }
            if (is_any() || to.is_any()) {
                return !is_void() && !to.is_void();
            }
            return is_void() && to.is_void();
        };
        size_t size() const;

    };

    // owner for a scripted type. types can be generated from this to be shared / manipulated as normal. Checks-out and returns space on the heap that can be accessed outside of the type itself. 
    class script_type {
    private:
        size_t ticket;
    public:
        script_type(GL::string name) : ticket{ impl::checkout_scripted_type(name) } {};
        script_type(script_type const&) = delete;
        script_type(script_type &&) = delete;
        script_type& operator=(script_type const&) = delete;
        script_type& operator=(script_type&&) = delete;
        ~script_type() {
            impl::return_scripted_type(ticket);
        };

        operator type() const {
            return GL::type(ticket);
        };
        type load() const {
            return GL::type(ticket);
        }
    };

    // get the type information for a c++ type.
    template<typename T> /*__forceinline*/__declspec(noinline) static GL::type type_of() noexcept {
        using BaseType = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*        
        if constexpr (std::is_rvalue_reference<T>::value) {
            static GL::type Base((GL::impl::get_impl<BaseType>().base_hash & impl::cached_type::MAGIC_MASK2) | (((size_t)((size_t)type::Qualifiers::Temporary | (size_t)type::Qualifiers::CppType) << 60ull) & impl::cached_type::MAGIC_MASK1));
            return Base;
        }
        else {
            static constexpr size_t const_modifier{ std::is_const<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>::value ? type::Qualifiers::Const : 0 };
            static constexpr size_t ref_modifier{ std::is_reference<typename std::remove_pointer<T>::type>::value ? type::Qualifiers::Reference : 0 };
            static GL::type Base((GL::impl::get_impl<BaseType>().base_hash & impl::cached_type::MAGIC_MASK2) | (((const_modifier | ref_modifier | (size_t)type::Qualifiers::CppType) << 60ull) & impl::cached_type::MAGIC_MASK1));
            return Base;
        }
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
                : m_actual_type{ p_actual_type }
                , m_data{ nullptr } 
            {};
        public:
            any_data() = delete;
            any_data(any_data const&) = delete;
            any_data(any_data &&) = delete;
            any_data& operator=(any_data const&) = delete;
            any_data& operator=(any_data&&) = delete;
            virtual ~any_data() = default;

            virtual GL::shared_ptr<void> get(GL::shared_ptr<any_data>&&) = 0;
            virtual std::shared_ptr<void> get_std(GL::shared_ptr<any_data>&&) = 0;
            
            template <typename T>
            T* cast() const {
                return reinterpret_cast<T*>(m_data);
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            virtual bool can_free_cast(type const& to) const = 0;
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            virtual bool can_cast(type const& to) const = 0;
            virtual bool can_cast_var() const = 0;
            virtual bool can_cast_dynamic_object() const = 0;
            virtual GL::type get_type() const = 0;

        public:
            GL::type m_actual_type; // atomic type information. May be updated to include information such as the const-ness or temporary type. This should (usually) be the base type. 
            void* m_data; // pointer to the actual data, for quicker access. 

        };

        template <typename T>
        class shared_data final : public any_data {
        public:
            shared_data(GL::shared_ptr<T>&& p_ptr) 
                : m_ptr(GL::static_pointer_cast<void>(std::forward<GL::shared_ptr<T>>(p_ptr)))
                , any_data(GL::type_of<T>()) 
            {
                this->m_data = m_ptr.get();
            };
            virtual ~shared_data() = default;

            GL::shared_ptr<void> get(GL::shared_ptr<any_data>&&) override {
                return m_ptr;
            };
            std::shared_ptr<void> get_std(GL::shared_ptr<any_data>&&) override {
                return std::shared_ptr<void>(this->m_data, [ptr = m_ptr](void* p) -> void {
                    if (p != ptr.get()) {
                        std::cout << "ERROR1\n";
                    }
                });
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const override {
                static auto var_hash_code = GL::util::type_id<var>().hash_code();
                static auto dynamic_object_hash_code = GL::util::type_id<dynamic_object>().hash_code();
                if constexpr (std::is_same_v<T, var>) {
                    if (to.get_base_hash() == var_hash_code) {
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<T, dynamic_object>) {
                    if (to.get_base_hash() == dynamic_object_hash_code) {
                        return true;
                    }
                }
                return GL::type_of<T>().can_free_cast(to);                
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const override {
                static auto var_hash_code = GL::util::type_id<var>().hash_code();
                static auto dynamic_object_hash_code = GL::util::type_id<dynamic_object>().hash_code();

                if constexpr (std::is_same_v<T, var>) {
                    if (to.get_base_hash() == var_hash_code) {
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<T, dynamic_object>) {
                    if (to.get_base_hash() == dynamic_object_hash_code) {
                        return true;
                    }
                }                
                return GL::type_of<T>().can_cast(to);                
            };
            // returns true if this type is of the same foundational type as a var
            bool can_cast_var() const override {
                if constexpr (std::is_same_v<T, var>) {
                    return true;
                }
                else {
                    return false;
                }
            };
            bool can_cast_dynamic_object() const override {
                if constexpr (std::is_same_v<T, dynamic_object>) {
                    return true;
                }
                else {
                    return false;
                }
            };
            GL::type get_type() const override {
                return GL::type_of<T>();
            };

        public:
            GL::shared_ptr< void > m_ptr;

        };
        
        template <typename T>
        class instanced_data final : public any_data {
        public:
            template <typename... TArgs>
            instanced_data(TArgs &&... a) noexcept
                : any_data(GL::type_of<T>())
            {       
                if constexpr ((sizeof...(TArgs) > 0) || !std::is_pod<T>::value) {
                    new (reinterpret_cast<T*>(&m_obj[0])) T(_STD forward<TArgs>(a)...);
                }
                else {
                    std::memset(&m_obj[0], 0, sizeof(m_obj));
                }
                this->m_data = reinterpret_cast<T*>(&m_obj[0]);
            };
            virtual ~instanced_data() {
                if constexpr (!std::is_pod_v<T>) {
                    reinterpret_cast<T*>(&m_obj[0])->~T();
                }
            };

            GL::shared_ptr<void> get(GL::shared_ptr<any_data>&& parent_ptr) override {
                if (parent_ptr) {
                    GL::shared_ptr<T> out(parent_ptr.release_control_block(), true);
                    out.set_pointer_without_modifying_control_block(reinterpret_cast<T*>(&m_obj[0]));
                    return out;
                }
                return nullptr;
            };
            std::shared_ptr<void> get_std(GL::shared_ptr<any_data>&& parent_ptr) override {
                return std::shared_ptr<void>(this->m_data, [ptr = std::forward<GL::shared_ptr<any_data>>(parent_ptr)](void* p) -> void {
                    if (p != ptr.get()) {
                        std::cout << "ERROR2\n";
                    }
                });
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const override {
                static auto var_hash_code = GL::util::type_id<var>().hash_code();
                static auto dynamic_object_hash_code = GL::util::type_id<dynamic_object>().hash_code();
                if constexpr (std::is_same_v<T, var>) {
                    if (to.get_base_hash() == var_hash_code) {
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<T, dynamic_object>) {
                    if (to.get_base_hash() == dynamic_object_hash_code) {
                        return true;
                    }
                }
                return GL::type_of<T>().can_free_cast(to);
                            
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const override {
                static auto var_hash_code = GL::util::type_id<var>().hash_code();
                static auto dynamic_object_hash_code = GL::util::type_id<dynamic_object>().hash_code();
                if constexpr (std::is_same_v<T, var>) {
                    if (to.get_base_hash() == var_hash_code) {
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<T, dynamic_object>) {
                    if (to.get_base_hash() == dynamic_object_hash_code) {
                        return true;
                    }
                }
                return GL::type_of<T>().can_cast(to);                
            };
            // returns true if this type is of the same foundational type as a var
            bool can_cast_var() const override {
                if constexpr (std::is_same_v<T, var>) {
                    return true;
                }
                else {
                    return false;
                }
            };
            bool can_cast_dynamic_object() const override {
                if constexpr (std::is_same_v<T, dynamic_object>) {
                    return true;
                }
                else {
                    return false;
                }
            };
            GL::type get_type() const override {
                return GL::type_of<T>();
            };

        public:
            unsigned char
                m_obj[sizeof(T)];
        };

        template <typename T>
        class std_shared_data final : public any_data {
        public:
            std_shared_data(std::shared_ptr<T> && a) noexcept
                : m_obj(std::forward<std::shared_ptr<T>>(a))
                , any_data(GL::type_of<T>())
            {
                this->m_data = m_obj.get();
            };
            virtual ~std_shared_data() = default;

            GL::shared_ptr<void> get(GL::shared_ptr<any_data>&& parent_ptr) override {
                if (parent_ptr) {
                    auto* control_block = parent_ptr.release_control_block();
                    GL::shared_ptr<T> out(control_block, true);
                    out.set_pointer_without_modifying_control_block(m_obj.get());
                    return out;
                }
                return nullptr;
            };
            std::shared_ptr<void> get_std(GL::shared_ptr<any_data>&&) override {
                return std::static_pointer_cast<void>(m_obj);
            };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            bool can_free_cast(type const& to) const override {
                static auto var_hash_code = GL::util::type_id<var>().hash_code();
                static auto dynamic_object_hash_code = GL::util::type_id<dynamic_object>().hash_code();
                if constexpr (std::is_same_v<T, var>) {
                    if (to.get_base_hash() == var_hash_code) {
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<T, dynamic_object>) {
                    if (to.get_base_hash() == dynamic_object_hash_code) {
                        return true;
                    }
                }
                return GL::type_of<T>().can_free_cast(to);                
            };
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            bool can_cast(type const& to) const override {
                static auto var_hash_code = GL::util::type_id<var>().hash_code();
                static auto dynamic_object_hash_code = GL::util::type_id<dynamic_object>().hash_code();
                if constexpr (std::is_same_v<T, var>) {
                    if (to.get_base_hash() == var_hash_code) {
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<T, dynamic_object>) {
                    if (to.get_base_hash() == dynamic_object_hash_code) {
                        return true;
                    }
                }
                return GL::type_of<T>().can_cast(to);                
            };
            // returns true if this type is of the same foundational type as a var
            bool can_cast_var() const override {
                if constexpr (std::is_same_v<T, var>) {
                    return true;
                }
                else {
                    return false;
                }
            };
            bool can_cast_dynamic_object() const override {
                if constexpr (std::is_same_v<T, dynamic_object>) {
                    return true;
                }
                else {
                    return false;
                }
            };
            GL::type get_type() const override {
                return GL::type_of<T>();
            };

        public:
            std::shared_ptr<T>
                m_obj;
        };

        struct wrapper {
            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, GL::shared_ptr<S>>>>
            static GL::shared_ptr<any_data> get_v(H<S> obj) {
                if (obj) {
                    if constexpr (std::is_same<GL::any, S>::value) {
                        // return self
                        return obj->m_ptr.load();
                    }
                    else if constexpr (std::is_same<GL::any::fast_any, S>::value) {
                        // return self
                        return obj->m_ptr;
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

            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, GL::shared_ptr<S>>>>
            static GL::shared_ptr<any_data> get(H<S> obj) {
                if (obj) {
                    if constexpr (std::is_same<GL::any, S>::value) {
                        // return self
                        return obj->m_ptr.load();
                    }
                    else if constexpr (std::is_same<GL::any::fast_any, S>::value) {
                        // return self
                        return obj->m_ptr;
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

            template<typename T, typename = std::enable_if_t<!std::is_same_v<GL::type_erasure::any_cast, T> && !std::is_same_v<GL::type_erasure::fast_any_cast, T>>>
            static GL::shared_ptr<any_data> get_v(const T& obj) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.m_ptr.load();
                }
                else if constexpr (std::is_same<GL::any::fast_any, T>::value) {
                    // return self
                    return obj.m_ptr;
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(obj));
                }
            };

            template<typename T, typename = std::enable_if_t<!std::is_same_v<GL::type_erasure::any_cast, T> && !std::is_same_v<GL::type_erasure::fast_any_cast, T>>>
            static GL::shared_ptr<any_data> get(T&& obj) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.m_ptr.load();
                }
                else if constexpr (std::is_same<GL::any::fast_any, T>::value) {
                    // return self
                    return obj.m_ptr;
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(std::forward<T>(obj)));
                }
            };

            static GL::shared_ptr<any_data> get(const type_erasure::any_cast& obj);
            static GL::shared_ptr<any_data> get(const type_erasure::any_cast* t);
            static GL::shared_ptr<any_data> get(const type_erasure::fast_any_cast& obj);
            static GL::shared_ptr<any_data> get(const type_erasure::fast_any_cast* t);
        };
        template<typename T> static GL::shared_ptr<any_data> wrap(const T& r) { return wrapper::get_v(r); };
        template<typename T> static GL::shared_ptr<any_data> wrap(T&& r) { 
            if constexpr (std::is_reference<T>::value) {
                return wrapper::get_v(r);
            }
            else {
                return wrapper::get(std::forward<T>(r));
            }
        };

    };

    /* class "dynamic_object" is a generic container for any custom, scripting-language class or struct, which may itself contain objects that it carrys with it. */
    class dynamic_object {
    public:
        dynamic_object() = default;
        dynamic_object(GL::type const& type)
            : m_type(type)
            , m_objects()
        {};
        dynamic_object(dynamic_object const&) = default;
        dynamic_object(dynamic_object&&) = default;
        dynamic_object& operator=(dynamic_object const&) = default;
        dynamic_object& operator=(dynamic_object&&) = default;
        ~dynamic_object() = default;

        GL::type
            m_type;
        concurrency::concurrent_unordered_map<GL::string, GL::shared_ptr<GL::any>> // GL::shared_ptr<GL::any> instead of GL::any simply due to compilation order
            m_objects;

        GL::shared_ptr<GL::any>& operator[](GL::string const& sv) {
            return m_objects[sv];
        };
        GL::shared_ptr<GL::any> const& operator[](GL::string const& sv) const {
            return m_objects.at(sv);
        };
        GL::shared_ptr<GL::any>* try_at(GL::string const& sv) {
            if (m_objects.count(sv) > 0) {
                return &m_objects.at(sv);
            }
            else {
                return nullptr;
            }
        };
        const GL::shared_ptr<GL::any>* try_at(GL::string const& sv) const {
            if (m_objects.count(sv) > 0) {
                return &m_objects.at(sv);
            }
            else {
                return nullptr;
            }
        };

        template <typename T>
        static GL::shared_ptr<GL::any> object_access(GL::string const& member_name, T const& rhs) {
            if constexpr (std::is_same_v<T, GL::any::fast_any> || std::is_same_v<T, GL::any>) {
                if (rhs.can_cast(GL::type_of<GL::dynamic_object&>())) {
                    if (GL::shared_ptr<GL::any>* p = rhs.cast<GL::dynamic_object>().try_at(member_name); p) {
                        return *p;
                    }
                    else {
                        GL::string err = GL::string("Could not find object \"") + member_name + "\" within " + rhs.m_casted_type.name();
                        throw std::runtime_error(err.to_string());
                    }
                }
                GL::string err = GL::string("Could not cast from ") + rhs.m_casted_type.name() + " to " + GL::type_of< GL::dynamic_object&>().name();
                throw std::runtime_error(err.to_string());
            }
            else {
                static_assert(std::is_same_v<T, GL::any::fast_any> || std::is_same_v<T, GL::any>, "Must be an Any or Any::Fast_Any");
            }
        };


    };

    /* class "Var" is a generic container for dynamically typed objects for use in the scripting language.
    It defers from "Any" because Any objects are for use in C++ to contain statically typed objects.
    "Var" objects are wrappers for Anys that allow the scripting language to process them as
    empty & assignable, or filled and implimented */
    class var {
    public:
        var() = default;
        explicit var(GL::shared_ptr<any>&& data_f);
        var(var const&) = default;
        var(var&&) = default;
        var& operator=(var const&) = default;
        var& operator=(var&&) = default;
        ~var() = default;

    public:
        GL::type const& get_type() const;
        GL::type const& get_actual_type();
        GL::fast_shared_ptr<GL::any> get_data() {
            return p_data.load_fast();
        };
        void set_data(GL::shared_ptr<GL::any>&& rhs);
    
    private:
        GL::atomic_shared_ptr<any> 
            p_data; // may be "updated" at any time and therefore should be thread-safe. 
        GL::type
            p_type;
    };




    // atomic wrapper for any object that can store and share shared_ptrs. Handles automatic casting to nearly all variants of qualified types. 
    class /*alignas(CACHE_LINE_SIZE)*/ any {
    public:
        class fast_any;
        friend class dynamic_object;
    protected:
        mutable GL::atomic_shared_ptr< type_erasure::any_data >
            m_ptr; // atomic shared-ptr for the type-erased underlying data. 
    public:
        GL::type
            m_casted_type; // atomic type information. May be updated to include information such as the const-ness or temporary type. 

    private:
        void correct_type_information() {
            if (auto f = m_ptr.load_fast(); f) {
                if (f->can_cast_var()) {
                    if (auto* V = f->cast<var>()) {
                        m_casted_type = V->get_type();
                        if (m_casted_type.is_void()) {
                            m_casted_type = GL::type_of<GL::var>();
                        }
                    }
                }
                if (f->can_cast_dynamic_object()) {
                    if (auto* V = f->cast<dynamic_object>()) {
                        m_casted_type = V->m_type;
                    }
                }
            }
        }
        explicit any(GL::atomic_shared_ptr< type_erasure::any_data > const& p_ptr, GL::type&& p_type)
            : m_ptr{ p_ptr }, m_casted_type{ std::forward<GL::type>(p_type) }
        {
            correct_type_information();
        }

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

        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> any(const ValueType& value) noexcept
            : any(type_erasure::wrap(value), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>())
        {};
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> any(ValueType&& value) noexcept
            : any(type_erasure::wrap(std::forward<ValueType>(value)), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>())
        {};
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> any& operator=(const ValueType& rhs) noexcept {
            m_ptr = type_erasure::wrap(rhs);
            m_casted_type = type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>();
            correct_type_information();
            return *this;
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> any& operator=(ValueType&& rhs) noexcept {
            m_ptr = type_erasure::wrap(std::forward<ValueType>(rhs));
            m_casted_type = type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>();
            correct_type_information();
            return *this;
        };
        
        bool operator&(int p_modifiers) const {
            return m_casted_type & p_modifiers;
        };
        any operator|(int p_modifiers) const {
            return any(m_ptr, m_casted_type | p_modifiers);
        };
        any operator+(int p_modifiers) const {
            return any(m_ptr, m_casted_type + p_modifiers);
        };
        any operator-(int p_modifiers) const {
            return any(m_ptr, m_casted_type - p_modifiers);
        };
        any& operator|=(int p_modifiers) {
            m_casted_type |= p_modifiers;
            return *this;
        };
        any& operator+=(int p_modifiers) {
            m_casted_type += p_modifiers;
            return *this;
        };
        any& operator-=(int p_modifiers) {
            m_casted_type -= p_modifiers;
            return *this;
        };

        operator bool() const noexcept {
            if (auto f = m_ptr.load_fast(); f) return true;
            return false;
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> static any ref(ValueType& value) {
            auto base_t = type_of<ValueType>();
            any out(GL::shared_ptr<std::decay_t< ValueType >>((std::decay_t< ValueType >*)(&value), [](std::decay_t< ValueType >*) {}));
            out.m_casted_type = base_t;
            if constexpr (std::is_const_v< ValueType>) {                
                out.m_casted_type |= GL::type::Reference;
                out.m_casted_type |= GL::type::Const;
            }
            else {
                out.m_casted_type |= GL::type::Reference;                
            }            
            return out;
        };

        template <typename T> __declspec(noinline) static any wrap_member(any const& parent, T const& ref) {
            GL::shared_ptr<T> out = GL::shared_ptr<T>(parent.shared_ptr());
            out.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
            return any(out) + (GL::type::Const | GL::type::Reference);
        };
        template <typename T> __declspec(noinline) static any wrap_member(any const& parent, T& ref) {
            GL::shared_ptr<T> out = GL::shared_ptr<T>(parent.shared_ptr());
            out.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
            if (parent.m_casted_type.is_const()) {
                return any(out) + (GL::type::Const | GL::type::Reference);
            }
            else {
                return any(out) + GL::type::Reference;
            }
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
            if (auto ptr = m_ptr.load_fast()) {
                if (ptr->m_actual_type.can_cast(to)) return true;
                if (ptr->can_cast_dynamic_object() && (to.get_base_hash() == GL::type_of<GL::dynamic_object>().get_base_hash())) { return true; }
                if (ptr->can_cast_var()) {
                    if (to.get_base_hash() == GL::type_of<GL::var>().get_base_hash()) return true;
                    if (auto f = ptr->cast<GL::var>()->get_data(); f) {
                        return f->can_cast(to);
                    }
                }
            }
            return false;
        };
        GL::type const& get_actual_type() const {
            if (auto ptr = m_ptr.load_fast()) return ptr->m_actual_type;
            else {
                static GL::type out{};
                return out;
            }
        };
        GL::shared_ptr<type_erasure::any_data> get_underlying_ptr() const {
            return m_ptr.load();
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
                return p->get(this->m_ptr.load());
            }
            return nullptr;
        };
        std::shared_ptr<void> std_shared_ptr() const {
            if (auto p = m_ptr.load_fast()) {
                return p->get_std(this->m_ptr.load());
            }
            return nullptr;
        };

    protected:
        class DataCaster {
            friend struct type_erasure::any_cast;
            friend struct type_erasure::fast_any_cast;
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
            template <class VType> static decltype(auto) DoCast_Shared(GL::shared_ptr<type_erasure::any_data>&& ptr) noexcept {
                auto& any_p = ptr.operator*();
                return GL::static_pointer_cast<VType>(any_p.get(std::move(ptr)));
            };
            template <class VType> static decltype(auto) DoCast_StdShared(GL::shared_ptr<type_erasure::any_data>&& ptr) noexcept {
                auto& any_p = ptr.operator*();
                return std::static_pointer_cast<VType>(any_p.get_std(std::forward<GL::shared_ptr<type_erasure::any_data>>(ptr)));
            };
            template<typename VType> static decltype(auto) DoCast_Unshared(GL::shared_ptr<type_erasure::any_data>&& container) /*noexcept*/ {
                static constexpr bool is_ptr{ std::is_pointer_v<VType> };
                auto& any_p = container.operator*();
                if constexpr (is_ptr) {
                    if (any_p.can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                        return reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(any_p.m_data);
                    }
                    else {                                
                        return static_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(nullptr);
                    }
                }
                else {
                    if (any_p.can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                        return *any_p.cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>();
                    }
                    else {
                        auto err = "Cannot cast from `" + any_p.m_actual_type.name() + "` to `" + type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>().name() + "`";
                        throw std::runtime_error(err.to_string());
                    }
                }
            };
            
            template <class VType> static decltype(auto) DoCast_Shared_fast(GL::shared_ptr<type_erasure::any_data>& ptr) noexcept {
                return GL::static_pointer_cast<VType>(ptr->get(GL::shared_ptr<type_erasure::any_data>(ptr)));
            };
            template <class VType> static decltype(auto) DoCast_StdShared_fast(GL::shared_ptr<type_erasure::any_data>& ptr) noexcept {
                return std::static_pointer_cast<VType>(ptr->get_std(GL::shared_ptr<type_erasure::any_data>(ptr)));
            };
            template<typename VType> static decltype(auto) DoCast_Unshared_fast(GL::shared_ptr<type_erasure::any_data>& container) /*noexcept*/ {
                static constexpr bool is_ptr{ std::is_pointer_v<VType> };
                
                if constexpr (is_ptr) {
                    if (container->can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                        return reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container->m_data);
                    }
                    else {
                        return static_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(nullptr);
                    }
                }
                else {
                    if (container->can_cast(type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>())) {
                        return *container->cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>();
                    }
                    else {
                        auto err = "Cannot cast from `" + container->m_actual_type.name() + "` to `" + type_of<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>().name() + "`";
                        throw std::runtime_error(err.to_string());
                    }
                }
            };

        public:
            template<typename T> static decltype(auto) DoCast(any* p) /*noexcept*/ {
                typedef typename is_SharedPtr_class<T>::type isShared;
                typedef typename is_stdSharedPtr_class<T>::type isStdShared;

                static constexpr bool is_shared_ptr{ isShared::value };
                static constexpr bool is_std_shared_ptr{ isStdShared::value };
                static constexpr bool is_ptr{ std::is_pointer_v<T> };
                static constexpr bool is_ref{ std::is_reference_v<T> };
                static constexpr bool is_const{ std::is_const_v<T> };
                static constexpr bool is_any{ std::is_same_v<GL::any, T> || std::is_same_v<GL::any::fast_any, T> || std::is_same_v<GL::any const&, T> || std::is_same_v<GL::any::fast_any const&, T> || std::is_same_v<GL::any::fast_any&, T> };

                if constexpr (is_any) {
                    return p->fast();
                }
                else {
                    if (p) {
                        decltype(auto) container{ p->m_ptr.load() };
                        while (container) {
                            if constexpr (is_shared_ptr) {
                                // casting to GL::shared_ptr
                                if constexpr (is_ptr) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }
                                else if constexpr (is_ref && !is_const) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }

                                if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                    return DoCast_Shared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                                else {
                                    if (container->can_cast_var()) {
                                        var* ptr = container->cast<var>();
                                        if (auto f = ptr->get_data()) {
                                            container = f->m_ptr.load();
                                            continue;
                                        }
                                    }
                                    return DoCast_Shared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                            }
                            else if constexpr (is_std_shared_ptr) {
                                // casting to std::shared_ptr
                                if constexpr (is_ptr) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }
                                else if constexpr (is_ref) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }

                                if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                    return DoCast_StdShared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                                else {
                                    if (container->can_cast_var()) {
                                        var* ptr = container->cast<var>();
                                        if (auto f = ptr->get_data()) {
                                            container = f->m_ptr.load();
                                            continue;
                                        }
                                    }
                                    return DoCast_StdShared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                            }
                            else {
                                // casting to a reference or a pointer
                                if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                    return DoCast_Unshared<T>(std::move(container));
                                }
                                else {
                                    if (container->can_cast_var()) {
                                        var* ptr = container->cast<var>();
                                        if (auto f = ptr->get_data()) {
                                            container = f->m_ptr.load();
                                            continue;
                                        }
                                    }
                                    return DoCast_Unshared<T>(std::move(container));
                                }
                            }
                        }
                    }

                    if constexpr (is_shared_ptr) {
                        return GL::shared_ptr<typename type_erasure::get_type<T>::type>(nullptr);
                    }
                    else if constexpr (is_std_shared_ptr) {
                        return std::shared_ptr<typename type_erasure::get_type<T>::type>(nullptr);
                    }
                    else {
                        if constexpr (is_ptr) {
                            return static_cast<typename std::remove_reference<typename std::remove_pointer<T>::type>::type*>(nullptr);
                        }
                        else {
                            auto err = "Cannot cast from `void` to `" + GL::type_of<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>().name() + "`";
                            throw std::runtime_error(err.to_string());
                        }
                    }
                }
            };
            template<typename T> static decltype(auto) DoCast(fast_any* p) /*noexcept*/ {
                typedef typename is_SharedPtr_class<T>::type isShared;
                typedef typename is_stdSharedPtr_class<T>::type isStdShared;

                static constexpr bool is_shared_ptr{ isShared::value };
                static constexpr bool is_std_shared_ptr{ isStdShared::value };
                static constexpr bool is_ptr{ std::is_pointer_v<T> };
                static constexpr bool is_ref{ std::is_reference_v<T> };
                static constexpr bool is_const{ std::is_const_v<T> };
                static constexpr bool is_any{ std::is_same_v<GL::any, T> || std::is_same_v<GL::any::fast_any, T> || std::is_same_v<GL::any const&, T> || std::is_same_v<GL::any::fast_any const&, T> || std::is_same_v<GL::any::fast_any&, T> };

                if constexpr (is_any) {
                    return *p;                    
                }
                else {
                    // fast path
                    if (p && p->m_ptr) {
                        if (!p->m_ptr->can_cast_var()) {
                            if constexpr (is_shared_ptr) {
                                if constexpr (is_ptr) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }
                                else if constexpr (is_ref) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }

                                return DoCast_Shared_fast<typename type_erasure::get_type<T>::type>(p->m_ptr);
                            }
                            else if constexpr (is_std_shared_ptr) {
                                if constexpr (is_ptr) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }
                                else if constexpr (is_ref) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }

                                return DoCast_StdShared_fast<typename type_erasure::get_type<T>::type>(p->m_ptr);
                            }
                            else {
                                return DoCast_Unshared_fast<T>(p->m_ptr);
                            }
                        }
                    }

                    // slow path
                    if (p) {
                        GL::shared_ptr<type_erasure::any_data> container{ p->m_ptr };
                        while (container) {
                            if constexpr (is_shared_ptr) {
                                // casting to GL::shared_ptr
                                if constexpr (is_ptr) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }
                                else if constexpr (is_ref && !is_const) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }

                                if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                    return DoCast_Shared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                                else {
                                    if (container->can_cast_var()) {
                                        var* ptr = container->cast<var>();
                                        if (auto f = ptr->get_data()) {
                                            container = f->m_ptr.load();
                                            continue;
                                        }
                                    }
                                    return DoCast_Shared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                            }
                            else if constexpr (is_std_shared_ptr) {
                                // casting to std::shared_ptr
                                if constexpr (is_ptr) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }
                                else if constexpr (is_ref) {
                                    throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                                }

                                if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                    return DoCast_StdShared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                                else {
                                    if (container->can_cast_var()) {
                                        var* ptr = container->cast<var>();
                                        if (auto f = ptr->get_data()) {
                                            container = f->m_ptr.load();
                                            continue;
                                        }
                                    }
                                    return DoCast_StdShared<typename type_erasure::get_type<T>::type>(std::move(container));
                                }
                            }
                            else {
                                // casting to a reference or a pointer
                                if constexpr (std::is_same_v<typename std::remove_pointer_t<typename std::decay_t<T>>, var>) {
                                    return DoCast_Unshared<T>(std::move(container));
                                }
                                else {
                                    if (container->can_cast_var()) {
                                        var* ptr = container->cast<var>();
                                        if (auto f = ptr->get_data()) {
                                            container = f->m_ptr.load();
                                            continue;
                                        }
                                    }
                                    return DoCast_Unshared<T>(std::move(container));
                                }
                            }
                        }
                    }

                    if constexpr (is_shared_ptr) {
                        return GL::shared_ptr<typename type_erasure::get_type<T>::type>(nullptr);
                    }
                    else if constexpr (is_std_shared_ptr) {
                        return std::shared_ptr<typename type_erasure::get_type<T>::type>(nullptr);
                    }
                    else {
                        if constexpr (is_ptr) {
                            return static_cast<typename std::remove_reference<typename std::remove_pointer<T>::type>::type*>(nullptr);
                        }
                        else {
                            auto err = "Cannot cast from `void` to `" + GL::type_of<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>().name() + "`";
                            throw std::runtime_error(err.to_string());
                        }
                    }
                }
            };

        };

    public:
        template<typename VType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        decltype(auto) cast() const { return DataCaster::DoCast<VType>(const_cast<any*>(this)); };

        template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value && std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any& cast() const noexcept { return *const_cast<any*>(this); };

        template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value && std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        any* cast() const noexcept { return const_cast<any*>(this); };

        type_erasure::any_cast cast() const noexcept;

        friend struct type_erasure::any_cast;
        friend struct type_erasure::fast_any_cast;
        friend struct type_erasure::wrapper;

        // version of 'any' that is NOT thread-safe, but is faster as a result.
        class fast_any {
            friend class dynamic_object;
        protected:
            mutable GL::shared_ptr< type_erasure::any_data >
                m_ptr; // atomic shared-ptr for the type-erased underlying data. 
        public:
            GL::type
                m_casted_type; // atomic type information. May be updated to include information such as the const-ness or temporary type. 

        private:
            void correct_type_information() {
                if (m_ptr) {
                    if (m_ptr->can_cast_var()) {
                        if (auto* V = m_ptr->cast<var>()) {
                            m_casted_type = V->get_type();
                            if (m_casted_type.is_void()) {
                                m_casted_type = GL::type_of<GL::var>();
                            }
                        }
                    }
                    if (m_ptr->can_cast_dynamic_object()) {
                        if (auto* V = m_ptr->cast<dynamic_object>()) {
                            m_casted_type = V->m_type;
                        }
                    }
                }
            }
        public:
            explicit fast_any(GL::shared_ptr< type_erasure::any_data >&& p_ptr, GL::type const& p_type)
                : m_ptr{ std::move(p_ptr) }, m_casted_type{ p_type }
            {
                correct_type_information();
            }

        public:
            fast_any() = default;
            fast_any(fast_any const&) = default;
            fast_any(fast_any&&) noexcept = default;
            fast_any(std::nullptr_t) noexcept : m_ptr{}, m_casted_type{} { };
            fast_any& operator=(fast_any const&) = default;
            fast_any& operator=(fast_any&&) noexcept = default;
            ~fast_any() = default;

            template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> static fast_any instance(const ValueType& value) noexcept {
                return fast_any(type_erasure::wrap(value), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>());
            };
            template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<fast_any, std::decay_t<ValueType>>>> static fast_any instance(ValueType&& value) noexcept {
                return fast_any(type_erasure::wrap(std::forward<ValueType>(value)), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>());
            };    
            static fast_any instance(any&& value) noexcept {
                return value.fast();
            };
            static fast_any instance(const fast_any& value) noexcept {
                return value;
            };
            GL::shared_ptr<type_erasure::any_data> get_underlying_ptr() const {
                return m_ptr;
            };
            operator bool() const noexcept {
                return m_ptr.operator bool();
            };
            bool empty() const noexcept {
                return !operator bool();
            };
            friend bool operator==(const fast_any& a, const fast_any& b) noexcept { return a.m_ptr == b.m_ptr; };
            friend bool operator!=(const fast_any& a, const fast_any& b) noexcept { return a.m_ptr != b.m_ptr; };
            friend bool operator<(const fast_any& a, const fast_any& b) noexcept { return a.m_ptr < b.m_ptr; };
            friend bool operator<=(const fast_any& a, const fast_any& b) noexcept { return a.m_ptr <= b.m_ptr; };
            friend bool operator>(const fast_any& a, const fast_any& b) noexcept { return a.m_ptr > b.m_ptr; };
            friend bool operator>=(const fast_any& a, const fast_any& b) noexcept { return a.m_ptr >= b.m_ptr; };

            bool operator&(int p_modifiers) const {
                return m_casted_type & p_modifiers;
            };
            fast_any operator|(int p_modifiers) const {
                return fast_any((GL::shared_ptr< type_erasure::any_data >)m_ptr, m_casted_type | p_modifiers);
            };
            fast_any operator+(int p_modifiers) const {
                return fast_any((GL::shared_ptr< type_erasure::any_data >)m_ptr, m_casted_type + p_modifiers);
            };
            fast_any operator-(int p_modifiers) const {
                return fast_any((GL::shared_ptr< type_erasure::any_data >)m_ptr, m_casted_type - p_modifiers);
            };
            fast_any& operator|=(int p_modifiers) {
                m_casted_type |= p_modifiers;
                return *this;
            };
            fast_any& operator+=(int p_modifiers) {
                m_casted_type += p_modifiers;
                return *this;
            };
            fast_any& operator-=(int p_modifiers) {
                m_casted_type -= p_modifiers;
                return *this;
            };

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
                if (m_ptr) {
                    if (m_ptr->m_actual_type.can_cast(to)) return true;
                    if (m_ptr->can_cast_dynamic_object() && (to.get_base_hash() == GL::type_of<GL::dynamic_object>().get_base_hash())) { return true; }
                    if (m_ptr->can_cast_var()) {
                        if (to.get_base_hash() == GL::type_of<GL::var>().get_base_hash()) return true;
                        if (auto f = m_ptr->cast<GL::var>()->get_data(); f) {
                            return f->can_cast(to);
                        }
                    }
                }
                return false;
            };
            GL::type const& get_actual_type() const {
                if (m_ptr) return m_ptr->m_actual_type;
                else {
                    static GL::type out{};
                    return out;
                }
            };

            fast_any fast() const { return *this; };

        private:
            void* ptr() const {
                if (m_ptr)
                    return m_ptr->m_data;
                return nullptr;
            };
            GL::shared_ptr<void> shared_ptr() const {
                if (m_ptr) {
                    return m_ptr->get(GL::shared_ptr< type_erasure::any_data >(this->m_ptr));
                }
                return nullptr;
            };
            std::shared_ptr<void> std_shared_ptr() const {
                if (m_ptr) {
                    return m_ptr->get_std(GL::shared_ptr< type_erasure::any_data >(this->m_ptr));
                }
                return nullptr;
            };

        public:
            template<typename VType, typename = std::enable_if_t<!std::is_same_v<fast_any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
            decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<fast_any*>(this)); };

            template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<fast_any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
            any& cast() const noexcept { return *const_cast<fast_any*>(this); };

            template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<fast_any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
            any* cast() const noexcept { return const_cast<fast_any*>(this); };

            type_erasure::fast_any_cast cast() const noexcept;

            friend struct type_erasure::any_cast;
            friend struct type_erasure::fast_any_cast;
            friend struct type_erasure::wrapper;
            friend class any;

            template <typename T> __declspec(noinline) static any::fast_any wrap_member(any::fast_any const& parent, T const& ref) {
                GL::shared_ptr<T> out = GL::shared_ptr<T>(parent.shared_ptr());
                out.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
                return any(out).fast() + (GL::type::Const | GL::type::Reference);
            };
            template <typename T> __declspec(noinline) static any::fast_any wrap_member(any::fast_any const& parent, T& ref) {
                GL::shared_ptr<T> out = GL::shared_ptr<T>(parent.shared_ptr());
                out.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
                if (parent.m_casted_type.is_const()) {
                    return any(out).fast() + (GL::type::Const | GL::type::Reference);
                }
                else {
                    return any(out).fast() + GL::type::Reference;
                }
            };
        };

        fast_any fast() const { return fast_any(this->m_ptr.load(), this->m_casted_type); };

        any(fast_any const& rhs) noexcept : m_ptr{ GL::shared_ptr< type_erasure::any_data >{ rhs.m_ptr } }, m_casted_type{ rhs.m_casted_type } { };
        any& operator=(fast_any const& rhs) noexcept {
            m_ptr = GL::shared_ptr< type_erasure::any_data >{ rhs.m_ptr };
            m_casted_type = rhs.m_casted_type;
            return *this;
        };
        any(fast_any&& rhs) noexcept : m_ptr{ std::move(rhs.m_ptr) }, m_casted_type{ std::move(rhs.m_casted_type) } { };
        any& operator=(fast_any && rhs) noexcept {
            m_ptr = std::move(rhs.m_ptr);
            m_casted_type = std::move(rhs.m_casted_type);
            return *this;
        };
    };

    __forceinline var::var(GL::shared_ptr<any>&& data_f) : p_data(std::move(data_f)) {
        if (auto f = p_data.load_fast(); f)
            p_type = f->m_casted_type;
    };
    __forceinline GL::type const& var::get_type() const {
        return p_type;
    };
    __forceinline GL::type const& var::get_actual_type() {
        if (auto f = p_data.load_fast(); f)
            return f->get_actual_type();
        return p_type;
    };
    __forceinline void var::set_data(GL::shared_ptr<GL::any>&& rhs) {
        if (rhs) {
            p_type = rhs->m_casted_type;
            p_data.store(std::move(rhs));
        }
        else {
            p_type = GL::type{};
            p_data.store(nullptr);
        }
    };

    namespace type_erasure {
        struct any_cast {
            any_cast() = default;
            any_cast(const any_cast&) = delete;
            any_cast(any_cast&&) = default;
            any_cast& operator=(const any_cast&) = delete;
            any_cast& operator=(any_cast&&) = delete;
            ~any_cast() = default;

            explicit operator any& () const noexcept { return *parent; };
            explicit operator any* () const noexcept { return parent; };
            template <typename T> operator std::shared_ptr<T>() const noexcept { 
                return any::DataCaster::DoCast<std::shared_ptr<T>>(parent);
            };
            template <typename T> operator GL::shared_ptr<T>() const noexcept { 
                return any::DataCaster::DoCast<GL::shared_ptr<T>>(parent);
            };
            template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
            operator ValueTypeT& () const { 
                return any::DataCaster::DoCast<ValueTypeT&>(parent);
            };
            template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
            operator ValueTypeT* () const noexcept { 
                return any::DataCaster::DoCast<ValueTypeT*>(parent);
            };
        
            any* parent;
        };
        struct fast_any_cast {
            fast_any_cast() = default;
            fast_any_cast(const fast_any_cast&) = delete;
            fast_any_cast(fast_any_cast&&) = default;
            fast_any_cast& operator=(const fast_any_cast&) = delete;
            fast_any_cast& operator=(fast_any_cast&&) = delete;
            ~fast_any_cast() = default;

            explicit operator any::fast_any& () const noexcept { return *parent; };
            explicit operator any::fast_any* () const noexcept { return parent; };
            template <typename T> operator std::shared_ptr<T>() const noexcept {
                return any::DataCaster::DoCast<std::shared_ptr<T>>(parent);
            };
            template <typename T> operator GL::shared_ptr<T>() const noexcept {
                return any::DataCaster::DoCast<GL::shared_ptr<T>>(parent);
            };
#pragma warning(push)
#pragma warning(disable : 4172)
            template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
            operator ValueTypeT& () const {
                return any::DataCaster::DoCast<ValueTypeT&>(parent);
            };
#pragma warning(pop)
            template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value && !any::DataCaster::is_stdSharedPtr_class<ValueTypeT>::type::value> >
            operator ValueTypeT* () const noexcept {
                return any::DataCaster::DoCast<ValueTypeT*>(parent);
            };

            any::fast_any* parent;
        };
    };

    __forceinline type_erasure::any_cast any::cast() const noexcept {
        return type_erasure::any_cast{ const_cast<any*>(this) };
    };
    __forceinline type_erasure::fast_any_cast any::fast_any::cast() const noexcept {
        return type_erasure::fast_any_cast{ const_cast<fast_any*>(this) };
    };
    __forceinline bool GL::type::is_any() const noexcept {
        static size_t const h{ util::type_id<any>().hash_code() & impl::cached_type::MAGIC_MASK2 };
        static size_t const h2{ util::type_id<any::fast_any>().hash_code() & impl::cached_type::MAGIC_MASK2 };
        size_t this_h{ get_base_hash() };
        return (this_h == h) || (this_h == h2);
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
        __forceinline GL::shared_ptr<any_data> wrapper::get(const fast_any_cast& obj) {
            any::fast_any* t = const_cast<any::fast_any*>(obj.parent);
            if (t) {
                return t->m_ptr;
            }
            return nullptr;
        };
        __forceinline GL::shared_ptr<any_data> wrapper::get(const fast_any_cast* t) {
            return get(*t);
        };
    };

    class undefined {};
    template<int index> class template_parameter {};    
    class is_template {
        static std::map<GL::type, int>& list_of_template_parameter() {
            static std::map<GL::type, int> out{ 
                { GL::type_of<template_parameter<0>>(), 0}, { GL::type_of<template_parameter<1>>(), 1}, 
                { GL::type_of<template_parameter<2>>(), 2}, { GL::type_of<template_parameter<3>>(), 3}, 
                { GL::type_of<template_parameter<4>>(), 4}, { GL::type_of<template_parameter<5>>(), 5},
                { GL::type_of<template_parameter<6>>(), 6}, { GL::type_of<template_parameter<7>>(), 7},
                { GL::type_of<template_parameter<8>>(), 8}, { GL::type_of<template_parameter<9>>(), 9},
                { GL::type_of<template_parameter<10>>(), 10}, { GL::type_of<template_parameter<11>>(), 11},
                { GL::type_of<template_parameter<12>>(), 12}, { GL::type_of<template_parameter<13>>(), 13},
                { GL::type_of<template_parameter<14>>(), 14}, { GL::type_of<template_parameter<15>>(), 15}
            };
            return out;
        };
    public:
        static int index(GL::type const& what) {
            auto& out = list_of_template_parameter();
            if (auto f = out.find(what - GL::type::Reference - GL::type::Const - GL::type::Temporary), e = out.end(); f != e) {
                return f->second;
            }
            else {
                return -1;
            }
        };
    };

};

namespace std {
    _NODISCARD inline std::string to_string(GL::type const& _Val) { // convert string to string
        return _Val.name().to_string();
    };
    template <> struct hash<GL::type> {
        std::size_t operator()(const GL::type& k) const {
            return k.get_hash();
        };
    };
};

#include "units.h"
