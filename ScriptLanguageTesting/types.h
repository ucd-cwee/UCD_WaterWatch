#pragma once

#include <vector>
#include <set>
#include <map>
#include "util.h"
#include "Strings.h"
#include "atomic_shared_ptr.h"
#include <boost/type_index.hpp>
#include <concurrent_unordered_map.h>

// implimentatin of atomic type info and atomic any
namespace GL {
    class undefined {};
    class template_t {};

    class any;
    class atomic_any;
    class var;
    class dynamic_object;

    //namespace type_erasure {
    //    struct any_cast;
    //    struct fast_any_cast;
    //};

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
            constexpr static GL::type_hash_t OFFSET = (sizeof(GL::type_hash_t) * 8) - 5;
            constexpr static GL::type_hash_t MAGIC_MASK1 = (GL::type_hash_t)0xF8 << (OFFSET - 3);
            constexpr static GL::type_hash_t MAGIC_MASK2 = ~MAGIC_MASK1;
            
            GL::string // underlying name for this type
                name{ "void" };
            std::set<GL::type_hash_t> // hashes to the underlying base classes to this type
                base_classes{};
            std::vector<GL::type_hash_t> // hashes to the underlying base classes to this type
                base_classes_ordered{};
            GL::type_hash_t // without const, ref, etc. 
                base_hash{ 0 };
            size_t
                T_size{ 0 };

            bool is_cpp_type() const {
                return T_size != std::numeric_limits<size_t>::max();
            };
            // returns true if this is found to be a child of the parent type (id'd by its base hash) 
            bool is_derived_from(GL::type_hash_t base) const;
            // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
            bool is_base_of(GL::type_hash_t derived) const;
            // attempts to include the specified hash as a base of this class.
            bool add_base(GL::type_hash_t base);

            bool match_base_hash(GL::type_hash_t to_match) const;

            //bool is_any() const noexcept { return modifiers & Modifiers::Any; };
            bool is_void() const noexcept {
                static GL::type_hash_t void_type{ util::type_id<void>().hash_code() & MAGIC_MASK2 };
                return void_type == this->base_hash;
            };
            // bool is_value() const noexcept { return modifiers & Modifiers::ValueType; };

        };

        // get the unique hash for this scripted type and perform set-up. This hash will always point this object until the "return" is called. 
        GL::type_hash_t checkout_scripted_type(GL::string type_name);
        // unloads the scripted type and allows the index (or hash) to be re-used. 
        void return_scripted_type(GL::type_hash_t ticket);
        // get the cached, scripted type info based on the ticketed hash. Must have been previously "checked out" otherwise may perform an out-of-bounds index call. 
        cached_type& get_scripted_type(GL::type_hash_t hash);

        // get the cached, built-in cpp type info based on the boost::type hash
        cached_type& get_impl(GL::type_hash_t hash);

        // get the cached, built-in cpp type info
        template <typename T> cached_type& get_impl() {
            using BaseType = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
            static auto const& this_type{ util::type_id<BaseType>() };

            auto hash = this_type.hash_code() & cached_type::MAGIC_MASK2;
            auto& out = get_impl(hash);
            if (out.base_hash == 0) {
                if (InterlockedCompareExchangeNoFence(reinterpret_cast<volatile GL::type_hash_t*>(&out.base_hash), hash, 0) == 0) {
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
            , TemplateType = 16 // Distinguishes whether this is a template type.
        };

    protected:
        // combination of the base hash and the qualifiers. Limited to four qualifier types with the current magic mask set-up. 
        GL::type_hash_t hash; // e.g. int, long, std::string, or a (registered) scripted type
        static GL::type_hash_t const& any_hash_code() {
            static GL::type_hash_t out{ util::type_id<any>().hash_code() & impl::cached_type::MAGIC_MASK2 };
            return out;
        };
        static GL::type_hash_t const& void_hash_code() {
            static GL::type_hash_t out{ util::type_id<void>().hash_code() & impl::cached_type::MAGIC_MASK2 };
            return out;
        };
    public:
        static constexpr GL::type_hash_t default_hash_code = (0ull & impl::cached_type::MAGIC_MASK2) | ((GL::type_hash_t)CppType << impl::cached_type::OFFSET);
        //static GL::type_hash_t const& default_hash_code() {
        //    static GL::type_hash_t out{ (util::type_id<void>().hash_code() & impl::cached_type::MAGIC_MASK2) | ((GL::type_hash_t)CppType << impl::cached_type::OFFSET) };
        //    return out;
        //};

    public:
        type() : hash{ default_hash_code } {};
        explicit type(GL::type_hash_t _hash) : hash{ _hash } {};
        type(type const&) = default;
        type(type &&) = default;
        type& operator=(type const& rhs) {
            InterlockedExchangeNoFence(reinterpret_cast<volatile GL::type_hash_t*>(&hash), rhs.hash);
            return *this;
        };
        type& operator=(type && rhs) noexcept {
            InterlockedExchangeNoFence(reinterpret_cast<volatile GL::type_hash_t*>(&hash), rhs.hash);
            return *this;
        };
        ~type() = default;

        std::vector<type> all_base_types(bool local_only = true) const;
        // removes the qualifiers and returns only the base hash value
        GL::type_hash_t get_base_hash() const {
            return hash & impl::cached_type::MAGIC_MASK2;
        };
        // returns the qualifiers attached to this hash, shifted to easily compare with the Qualifiers enum directly. 
        GL::type_hash_t get_qualifiers() const {
            return (hash & impl::cached_type::MAGIC_MASK1) >> impl::cached_type::OFFSET;
        };
        // atomicly swaps this type with a new hash and qualifier(s)
        void set_qualifiers(GL::type_hash_t qualifiers) {
            if ((qualifiers & Temporary) > 0) {
                qualifiers &= ~Const;
                qualifiers &= ~Reference;
            }
            InterlockedExchangeNoFence(reinterpret_cast<volatile GL::type_hash_t*>(&hash), (hash & impl::cached_type::MAGIC_MASK2) | ((qualifiers << impl::cached_type::OFFSET) & impl::cached_type::MAGIC_MASK1));
        };

        bool is_temp() const noexcept { return ((hash & ((GL::type_hash_t)Temporary << impl::cached_type::OFFSET)) > 0); };
        bool is_const() const noexcept { return /*is_temp() ? false : */((hash & ((GL::type_hash_t)Const << impl::cached_type::OFFSET)) > 0); };
        bool is_ref() const noexcept { return /*is_temp() ? false : */((hash & ((GL::type_hash_t)Reference << impl::cached_type::OFFSET)) > 0); };
        bool is_const_ref() const noexcept { return /*is_temp() ? false : */((hash & (((GL::type_hash_t)Const + (GL::type_hash_t)Reference) << impl::cached_type::OFFSET)) == (((GL::type_hash_t)Const + (GL::type_hash_t)Reference) << impl::cached_type::OFFSET)); };
        bool is_base() const noexcept { 
            return 0 == ((hash & impl::cached_type::MAGIC_MASK1) & ~((GL::type_hash_t)CppType << impl::cached_type::OFFSET));
        };
        bool is_void() const noexcept {
            // thread_local GL::type_hash_t const h{ void_hash_code() };
            return get_base_hash() == void_hash_code();
        };
        bool is_template() const noexcept {
            return ((hash & ((GL::type_hash_t)TemplateType << impl::cached_type::OFFSET)) > 0);
        };
        bool is_cpp_type() const noexcept { return (hash & ((GL::type_hash_t)CppType << impl::cached_type::OFFSET)) > 0; };
        bool is_any() const noexcept;
        bool is_var() const noexcept;
        bool is_dynamic_object() const noexcept;
        // returns true if this is found to be a child of the parent type (id'd by its base hash) 
        bool is_derived_from(type const& base) const;
        // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
        bool is_base_of(type const& derived) const;
        // attempts to include the specified hash as a base of this class.
        bool add_base(type const& base) const;
        bool match_base_hash(type const& to_match) const;

        GL::string name() const;
        // this is NOT a thread-safe operation. Ideally should be done on start-up. 
        bool try_update_name(GL::string const& new_name) const;

        GL::type_hash_t get_hash() const {
            return hash;
        };

        // Operators
        friend bool operator==(const type& a, const type& b) noexcept { return a.hash == b.hash; };
        friend bool operator!=(const type& a, const type& b) noexcept { return a.hash != b.hash; };
        friend bool operator<(const type& a, const type& b) noexcept { return a.hash < b.hash; };
        friend bool operator<=(const type& a, const type& b) noexcept { return a.hash <= b.hash; };
        friend bool operator>(const type& a, const type& b) noexcept { return a.hash > b.hash; };
        friend bool operator>=(const type& a, const type& b) noexcept { return a.hash >= b.hash; };
        bool operator&(GL::type_hash_t p_modifiers) const {
            return (get_qualifiers() & p_modifiers) > 0;
        };
        type operator|(GL::type_hash_t p_modifiers) const {
            type out = *this;
            out.set_qualifiers(out.get_qualifiers() | p_modifiers);
            return out;
        };
        type& operator|=(GL::type_hash_t p_modifiers) {
            set_qualifiers(get_qualifiers() | p_modifiers);
            return *this;
        };
        type operator+(GL::type_hash_t p_modifiers) const {
            type out = *this;
            out.set_qualifiers(out.get_qualifiers() | p_modifiers);
            return out;
        };
        type& operator+=(GL::type_hash_t p_modifiers) {
            set_qualifiers(get_qualifiers() | p_modifiers);
            return *this;
        };
        type operator-(GL::type_hash_t p_modifiers) const {
            type out = *this;
            out.set_qualifiers(out.get_qualifiers() & ~p_modifiers);
            return out;
        };
        type& operator-=(GL::type_hash_t p_modifiers) {
            set_qualifiers(get_qualifiers() & ~p_modifiers);
            return *this;
        };

    private:
        // Returns true if the types are similar enough to be casted for free (0 cost)
        static bool can_free_cast(type const& from, type const& to, bool allow_polymorphic = true) {
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
    static constexpr bool is_p = std::is_pod_v<type>;

    // owner for a scripted type. types can be generated from this to be shared / manipulated as normal. Checks-out and returns space on the heap that can be accessed outside of the type itself. 
    class script_type {
    private:
        GL::type_hash_t ticket;
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
    template<typename T> /*__forceinline*//*__declspec(noinline)*/ static GL::type const& type_of() noexcept {
        using BaseType = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*        

        static constexpr GL::type_hash_t template_modifier{ 
            (std::is_base_of_v< template_t, BaseType> || std::is_same_v< template_t, BaseType>) ? type::Qualifiers::TemplateType : 0
        };

        if constexpr (std::is_rvalue_reference<T>::value) {
            static GL::type Base((GL::impl::get_impl<BaseType>().base_hash & impl::cached_type::MAGIC_MASK2) | (((GL::type_hash_t)((GL::type_hash_t)type::Qualifiers::Temporary | (GL::type_hash_t)type::Qualifiers::CppType | template_modifier) << impl::cached_type::OFFSET) & impl::cached_type::MAGIC_MASK1));
            return Base;
        }
        else {
            static constexpr GL::type_hash_t const_modifier{ std::is_const<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>::value ? type::Qualifiers::Const : 0 };
            static constexpr GL::type_hash_t ref_modifier{ std::is_reference<typename std::remove_pointer<T>::type>::value ? type::Qualifiers::Reference : 0 };
            static GL::type Base((GL::impl::get_impl<BaseType>().base_hash & impl::cached_type::MAGIC_MASK2) | (((const_modifier | ref_modifier | (GL::type_hash_t)type::Qualifiers::CppType | template_modifier) << impl::cached_type::OFFSET) & impl::cached_type::MAGIC_MASK1));
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
        public:
            virtual ~any_data() = default;
            virtual bool is_pod() const = 0;
            virtual GL::type const& get_type() const = 0;
            virtual void* get() const = 0;
            virtual GL::shared_ptr<void> get(GL::shared_ptr<any_data>&&) = 0;
            virtual std::shared_ptr<void> get_std(GL::shared_ptr<any_data>&&) = 0;
            
            template <typename T> T* cast() const { return reinterpret_cast<T*>(get()); };

            // returns true if this type can easily match the requested type (e.g. int& -> const int&)
            virtual bool can_free_cast(type const& to) const = 0;
            // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
            virtual bool can_cast(type const& to) const = 0;
            virtual bool can_cast_var() const = 0;
            virtual bool can_cast_dynamic_object() const = 0;
        };

        template <typename T>
        class shared_data final : public any_data {
        public:
            shared_data(GL::shared_ptr<T>&& p_ptr) : m_ptr(std::forward<GL::shared_ptr<T>>(p_ptr)) {};
            virtual ~shared_data() = default;

            bool is_pod() const override {
                return std::is_pod_v<T>;
            };
            GL::type const& get_type() const override {
                return GL::type_of<T>();
            };
            void* get() const override {
                return m_ptr.get();
            };
            GL::shared_ptr<void> get(GL::shared_ptr<any_data>&&) override {
                return m_ptr;
            };
            std::shared_ptr<void> get_std(GL::shared_ptr<any_data>&&) override {
                return std::shared_ptr<void>(get(), [ptr = m_ptr](void* p) -> void {
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

        public:
            GL::shared_ptr< T > m_ptr;

        };

        template <typename T>
        class instanced_data final : public any_data {
        public:
            template <typename... TArgs>
            instanced_data(TArgs &&... a) noexcept {       
                if constexpr ((sizeof...(TArgs) > 0) || !std::is_pod<T>::value) {
                    new (reinterpret_cast<T*>(&m_obj[0])) T(_STD forward<TArgs>(a)...);
                }
                else {
                    std::memset(&m_obj[0], 0, sizeof(m_obj));
                }
            };
            virtual ~instanced_data() {
                if constexpr (!std::is_pod_v<T>) {
                    reinterpret_cast<T*>(&m_obj[0])->~T();
                }
            };
            
            bool is_pod() const override {
                return std::is_pod_v<T>;
            };
            GL::type const& get_type() const override {
                return GL::type_of<T>();
            };
            void* get() const override {
                return const_cast<void*>(reinterpret_cast<const void*>(&m_obj[0]));
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
                return std::shared_ptr<void>(get(), [ptr = std::forward<GL::shared_ptr<any_data>>(parent_ptr)](void* p) -> void {
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

        public:
            unsigned char
                m_obj[sizeof(T)];
        };

        template <typename T>
        class std_shared_data final : public any_data {
        public:
            std_shared_data(std::shared_ptr<T> && a) noexcept : m_obj(std::forward<std::shared_ptr<T>>(a)) {};
            virtual ~std_shared_data() = default;

            bool is_pod() const override {
                return std::is_pod_v<T>;
            };
            GL::type const& get_type() const override {
                return GL::type_of<T>();
            };
            void* get() const override {
                return m_obj.get();
            };
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

        public:
            std::shared_ptr<T>
                m_obj;
        };

        struct wrapper {
            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, GL::shared_ptr<S>>>>
            static GL::shared_ptr<any_data> get_v(H<S> obj) {
                if (obj) {
                    if constexpr (std::is_same<GL::any, S>::value) 
                        return obj->get_underlying_ptr(); // return self                    
                    else if constexpr (std::is_same<GL::atomic_any, S>::value)
                        return obj->get_underlying_ptr(); // return self                    
                    else {
                        if constexpr (std::is_same<GL::shared_ptr<S>, H<S>>::value) 
                            return GL::make_shared<shared_data<S>>(std::move(obj));
                        else if constexpr (std::is_same<std::shared_ptr<S>, H<S>>::value) 
                            return GL::static_pointer_cast<any_data>(GL::make_shared<std_shared_data<S>>(std::move(obj)));
                        else 
                            return nullptr;                        
                    }
                }
                else {
                    return nullptr; // return null if incoming is null
                }
            };

            template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>> || std::is_same_v<H<S>, GL::shared_ptr<S>>>>
            static GL::shared_ptr<any_data> get(H<S> obj) {
                if (obj) {
                    if constexpr (std::is_same<GL::any, S>::value)
                        return obj->get_underlying_ptr(); // return self                    
                    else if constexpr (std::is_same<GL::atomic_any, S>::value)
                        return obj->get_underlying_ptr(); // return self                    
                    else {
                        if constexpr (std::is_same<GL::shared_ptr<S>, H<S>>::value) 
                            return GL::make_shared<shared_data<S>>(std::move(obj));
                        else if constexpr (std::is_same<std::shared_ptr<S>, H<S>>::value) 
                            return GL::static_pointer_cast<any_data>(GL::make_shared<std_shared_data<S>>(std::move(obj)));
                        else 
                            return nullptr;                        
                    }
                }
                else {
                    return nullptr; // return null if incoming is null
                }
            };

            template<typename T>
            static GL::shared_ptr<any_data> get_v(const T& obj) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.get_underlying_ptr();
                }
                else if constexpr (std::is_same<GL::atomic_any, T>::value) {
                    // return self                    
                    return obj.get_underlying_ptr();
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(obj));
                }
            };

            template<typename T>
            static GL::shared_ptr<any_data> get(T&& obj) {
                if constexpr (std::is_same<GL::any, T>::value) {
                    // return self
                    return obj.get_underlying_ptr();
                }
                else if constexpr (std::is_same<GL::atomic_any, T>::value) {
                    // return self
                    return obj.get_underlying_ptr();
                }
                else {
                    return GL::static_pointer_cast<any_data>(GL::make_shared<instanced_data<T>>(std::forward<T>(obj)));
                }
            };

            //static GL::shared_ptr<any_data> get(const type_erasure::any_cast& obj);
            //static GL::shared_ptr<any_data> get(const type_erasure::any_cast* t);
            //static GL::shared_ptr<any_data> get(const type_erasure::fast_any_cast& obj);
            //static GL::shared_ptr<any_data> get(const type_erasure::fast_any_cast* t);
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
            if constexpr (std::is_same_v<T, GL::atomic_any> || std::is_same_v<T, GL::any>) {
                //if (rhs.can_cast(GL::type_of<GL::dynamic_object&>())) {
                    if (GL::shared_ptr<GL::any>* p = rhs.cast<GL::dynamic_object>().try_at(member_name); p) {
                        return *p;
                    }
                    else {
                        GL::string err = GL::string("Could not find object \"") + member_name + "\" within " + rhs.m_casted_type.name();
                        throw std::runtime_error(err.to_string());
                    }
                //}
                GL::string err = GL::string("Could not cast from ") + rhs.m_casted_type.name() + " to " + GL::type_of< GL::dynamic_object&>().name();
                throw std::runtime_error(err.to_string());
            }
            else {
                static_assert(std::is_same_v<T, GL::atomic_any> || std::is_same_v<T, GL::any>, "Must be an any or atomic_any");
            }
        };


    };

    // Declaration
    namespace uuid {
        static constexpr unsigned long TEMPORARY = 0x1000'0000;
        static constexpr unsigned long FLAGS = 0xF000'0000;
        static constexpr unsigned long INV_FLAGS = ~FLAGS;

        struct uuid_ticket {
            size_t count{ 0 };
            GL::shared_ptr<GL::type_erasure::any_data> data{ nullptr };
        };
        unsigned long new_uuid(GL::shared_ptr<GL::type_erasure::any_data>&& rhs) noexcept;
        void free_uuid(unsigned long uuid) noexcept;
        uuid_ticket& get_uuid(unsigned long uuid) noexcept;
    };

    class Function;
    class Function_Caller;

    // not thread-safe. Should only be accessed in a single-threaded fashion.
    class any {
        friend class atomic_any;

        void*
            m_ptr;
        GL::type
            m_type;
        unsigned long
            m_uuid;

    public:
        auto& uuid() const {
            return m_uuid;
        };
        auto& ptr() const {
            return m_ptr;
        };
        auto& type() const {
            return m_type;
        };

        any(unsigned long uuid, void* data, GL::type const& type) noexcept : m_uuid{ uuid }, m_type{ type }, m_ptr{ data } {};
        any() noexcept : m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr } {};
        any(std::nullptr_t) noexcept : m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr } {};
        any(any const& rhs) : m_uuid{ rhs.m_uuid & uuid::INV_FLAGS }, m_type{ rhs.m_type }, m_ptr{ rhs.m_ptr } { if (m_uuid > 0) GL::interlocked::increment(uuid::get_uuid(m_uuid).count); };
        // declares that the parent will NOT go out-of-scope before this child does. That guarrantee allows us to skip a increment and decrement call to the counter. 
        any(any const& rhs, bool) : m_uuid{ rhs.m_uuid | uuid::TEMPORARY }, m_type{ rhs.m_type }, m_ptr{ rhs.m_ptr } {};
        any(any&& rhs) noexcept : m_uuid{ std::move(rhs.m_uuid) }, m_type{ std::move(rhs.m_type) }, m_ptr{ rhs.m_ptr } { rhs.m_uuid = 0; };
        any const& operator=(any const& rhs) {
            if ((m_uuid & uuid::TEMPORARY) == 0)
                if ((m_uuid & uuid::INV_FLAGS) > 0)
                    if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                        uuid::free_uuid(m_uuid);

            m_uuid = rhs.m_uuid & uuid::INV_FLAGS;
            m_type = rhs.m_type;
            m_ptr = rhs.m_ptr;
            if (m_uuid > 0) GL::interlocked::increment(uuid::get_uuid(m_uuid).count);
            return *this;
        };
        any const& operator=(any&& rhs) noexcept {
            if ((m_uuid & uuid::TEMPORARY) == 0)
                if ((m_uuid & uuid::INV_FLAGS) > 0)
                    if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                        uuid::free_uuid(m_uuid);

            m_uuid = std::move(rhs.m_uuid);
            m_type = std::move(rhs.m_type);
            m_ptr = rhs.m_ptr;
            rhs.m_uuid = 0;
            return *this;
        };

        any(atomic_any const& rhs);
        any(atomic_any&& rhs) noexcept;

        ~any() {
            if ((m_uuid & uuid::TEMPORARY) == 0)
                if ((m_uuid & uuid::INV_FLAGS) > 0)
                    if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                        uuid::free_uuid(m_uuid);
        };

        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<atomic_any, std::decay_t<ValueType>>>> static any instance(const ValueType& value) noexcept {
            // std::cout << GL::string("Instancing const ") + GL::type_of<ValueType>().name() + "&\n";
            auto wrapped = GL::type_erasure::wrap(value);
            auto* ptr = wrapped->get();
            return any(uuid::new_uuid(std::move(wrapped)), ptr, GL::type_of<typename GL::type_erasure::get_type<std::decay_t<ValueType>>::type>());
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<atomic_any, std::decay_t<ValueType>>>> static any instance(ValueType&& value) noexcept {
            // std::cout << GL::string("Instancing ") + GL::type_of<ValueType>().name() + "&&\n";
            auto wrapped = GL::type_erasure::wrap(std::forward<ValueType>(value));
            auto* ptr = wrapped->get();
            return any(uuid::new_uuid(std::move(wrapped)), ptr, GL::type_of<typename GL::type_erasure::get_type<std::decay_t<ValueType>>::type>());
        };
        static any instance(any&& value) noexcept {
            return std::forward<any>(value);
        };
        static any& instance(any& value) noexcept {
            return value;
        };
        static any const& instance(const any& value) noexcept {
            return value;
        };
        static any instance(atomic_any&& value) noexcept;
        static any instance(atomic_any& value) noexcept;
        static any instance(const atomic_any& value) noexcept;

        operator bool() const noexcept {
            return m_ptr;
        };
        bool empty() const noexcept {
            return !m_ptr;
        };
        friend bool operator==(const any& a, const any& b) noexcept { return a.m_ptr == b.m_ptr; };
        friend bool operator!=(const any& a, const any& b) noexcept { return a.m_ptr != b.m_ptr; };
        friend bool operator<(const any& a, const any& b) noexcept { return a.m_ptr < b.m_ptr; };
        friend bool operator<=(const any& a, const any& b) noexcept { return a.m_ptr <= b.m_ptr; };
        friend bool operator>(const any& a, const any& b) noexcept { return a.m_ptr > b.m_ptr; };
        friend bool operator>=(const any& a, const any& b) noexcept { return a.m_ptr >= b.m_ptr; };

        bool operator&(int p_modifiers) const noexcept {
            return m_type & p_modifiers;
        };
        any operator|(int p_modifiers) const noexcept {
            any out(*this);
            out.m_type |= p_modifiers;
            return out;
        };
        any operator+(int p_modifiers) const noexcept {
            any out(*this);
            out.m_type |= p_modifiers;
            return out;
        };
        any operator-(int p_modifiers) const noexcept {
            any out(*this);
            out.m_type -= p_modifiers;
            return out;
        };
        any& operator|=(int p_modifiers) noexcept {
            m_type |= p_modifiers;
            return *this;
        };
        any& operator+=(int p_modifiers) noexcept {
            m_type += p_modifiers;
            return *this;
        };
        any& operator-=(int p_modifiers) noexcept {
            m_type -= p_modifiers;
            return *this;
        };

    protected:
        friend struct GL::type_erasure::wrapper;

        GL::shared_ptr<GL::type_erasure::any_data>& get_underlying_ptr() const noexcept {
            if ((m_uuid & uuid::INV_FLAGS) > 0)
                return uuid::get_uuid(m_uuid).data;
            else {
                static GL::shared_ptr<GL::type_erasure::any_data> out{ nullptr };
                return out;
            }
        };
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
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const GL::shared_ptr<T>> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const GL::shared_ptr<T>&> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<const GL::shared_ptr<T>*> { typedef std::true_type type; };
            template<typename T> struct is_SharedPtr_class<GL::shared_ptr<T>&&> { typedef std::true_type type; };

        private:
            template <class VType> static decltype(auto) DoCast_Shared_fast(GL::shared_ptr<GL::type_erasure::any_data>& ptr) noexcept {
                return GL::static_pointer_cast<VType>(ptr->get(GL::shared_ptr<GL::type_erasure::any_data>(ptr)));
            };
            template <class VType> static decltype(auto) DoCast_StdShared_fast(GL::shared_ptr<GL::type_erasure::any_data>& ptr) noexcept {
                return std::static_pointer_cast<VType>(ptr->get_std(GL::shared_ptr<GL::type_erasure::any_data>(ptr)));
            };
            template<typename VType> static decltype(auto) DoCast_Unshared_fast(void* container) /*noexcept*/ {
                static constexpr bool is_ptr{ std::is_pointer_v<VType> };

                if constexpr (is_ptr) {
                    return reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container);
                }
                else {
                    return *reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container);
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
                static constexpr bool is_any{ std::is_same_v<any, std::decay_t<T>> };

                if (!p) {
                    if constexpr (is_any) {
                        static any out;
                        return *&out;
                    }
                    else {
                        if constexpr (is_shared_ptr) {
                            return GL::shared_ptr<typename GL::type_erasure::get_type<T>::type>(nullptr);
                        }
                        else if constexpr (is_std_shared_ptr) {
                            return std::shared_ptr<typename GL::type_erasure::get_type<T>::type>(nullptr);
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
                }
                else {
                    if constexpr (is_any) return *p;
                    else {
                        if constexpr (is_shared_ptr) {
                            if constexpr (is_ptr) throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            else if constexpr (is_ref) throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            return DoCast_Shared_fast<typename GL::type_erasure::get_type<T>::type>(p->get_underlying_ptr());
                        }
                        else if constexpr (is_std_shared_ptr) {
                            if constexpr (is_ptr) {
                                throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            }
                            else if constexpr (is_ref) {
                                throw("Casting any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            }
                            return DoCast_StdShared_fast<typename GL::type_erasure::get_type<T>::type>(p->get_underlying_ptr());
                        }
                        else {
                            return DoCast_Unshared_fast<T>(p->m_ptr);
                        }
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

        /// \todo it is a performance increase to allow the return value to be "referenced" rather than incremented/decremented properly, however, is this safe in practice?
        template <typename T> __declspec(noinline) static any wrap_member(any const& parent, T const& ref) noexcept {
            if ((parent.m_uuid & uuid::TEMPORARY) > 0) {
                return any((unsigned long)parent.m_uuid, &const_cast<T&>(ref), GL::type_of<T const&>());
            }
            else {
                unsigned long uuid = parent.m_uuid & uuid::INV_FLAGS;
                if (uuid > 0) GL::interlocked::increment(uuid::get_uuid(uuid).count);
                return any(std::move(uuid), &const_cast<T&>(ref), GL::type_of<T const&>());
            }
        };
        /// \todo it is a performance increase to allow the return value to be "referenced" rather than incremented/decremented properly, however, is this safe in practice?
        template <typename T> __declspec(noinline) static any wrap_member(any const& parent, T& ref) noexcept {
            if ((parent.m_uuid & uuid::TEMPORARY) > 0) {
                return any((unsigned long)parent.m_uuid, &ref, GL::type_of<T&>());
            }
            else {
                unsigned long uuid = parent.m_uuid & uuid::INV_FLAGS;
                if (uuid > 0) GL::interlocked::increment(uuid::get_uuid(uuid).count);
                return any(std::move(uuid), &ref, GL::type_of<T&>());
            }
        };

    };

    // thread-safe alternative for 'any', with interop. Can be safely accessed in a multi-threaded fashion, including changing the type info.
    class atomic_any {
        friend class any;

    protected:
        GL::fast_shared_mutex
            mut;
        void*
            m_ptr;
        GL::type
            m_type;
        unsigned long
            m_uuid;

    public:
        auto uuid() const {
            std::shared_lock locked(mut);
            return m_uuid;
        };
        auto ptr() const {
            std::shared_lock locked(mut);
            return m_ptr;
        };
        auto type() const {
            return m_type;
        };

        atomic_any(unsigned long uuid, void* data, GL::type const& type) noexcept : mut(), m_uuid{ uuid }, m_type{ type }, m_ptr{ data } {};
        atomic_any() noexcept : mut(), m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr } {};
        atomic_any(std::nullptr_t) noexcept : mut(), m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr } {};
        atomic_any(atomic_any const& rhs) : mut(), m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr }
        {
            if (1) {
                std::shared_lock locked(rhs.mut);
                m_uuid = rhs.m_uuid & uuid::INV_FLAGS;
                m_type = rhs.m_type;
                m_ptr = rhs.m_ptr;
            }
            if (m_uuid > 0) GL::interlocked::increment(uuid::get_uuid(m_uuid).count);
        };
        // declares that the parent will NOT go out-of-scope before this child does. That guarrantee allows us to skip a increment and decrement call to the counter. 
        atomic_any(atomic_any const& rhs, bool) : mut(), m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr }
        {
            std::shared_lock locked(rhs.mut);
            m_uuid = rhs.m_uuid | uuid::TEMPORARY;
            m_type = rhs.m_type;
            m_ptr = rhs.m_ptr;
        };
        atomic_any(atomic_any&& rhs) noexcept : mut(), m_uuid{ std::move(rhs.m_uuid) }, m_type{ std::move(rhs.m_type) }, m_ptr{ rhs.m_ptr } { rhs.m_uuid = 0; };
        atomic_any const& operator=(atomic_any const& rhs) {
            if (this == &rhs) return *this;

            std::unique_lock locked1(mut);
            std::shared_lock locked2(rhs.mut);

            if ((m_uuid & uuid::TEMPORARY) == 0)
                if ((m_uuid & uuid::INV_FLAGS) > 0)
                    if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                        uuid::free_uuid(m_uuid);

            m_uuid = rhs.m_uuid & uuid::INV_FLAGS;
            m_type = rhs.m_type;
            m_ptr = rhs.m_ptr;
            if (m_uuid > 0) GL::interlocked::increment(uuid::get_uuid(m_uuid).count);
            return *this;
        };
        atomic_any const& operator=(atomic_any&& rhs) noexcept {
            std::unique_lock locked1(mut);

            if ((m_uuid & uuid::TEMPORARY) == 0)
                if ((m_uuid & uuid::INV_FLAGS) > 0)
                    if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                        uuid::free_uuid(m_uuid);

            m_uuid = std::move(rhs.m_uuid);
            m_type = std::move(rhs.m_type);
            m_ptr = rhs.m_ptr;
            rhs.m_uuid = 0;
            return *this;
        };
        atomic_any(any const& rhs);
        atomic_any(any&& rhs) noexcept;

        ~atomic_any() {
            if ((m_uuid & uuid::TEMPORARY) == 0)
                if ((m_uuid & uuid::INV_FLAGS) > 0)
                    if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                        uuid::free_uuid(m_uuid);
        };

        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<atomic_any, std::decay_t<ValueType>>>> static atomic_any instance(const ValueType& value) noexcept {
            auto wrapped = GL::type_erasure::wrap(value);
            auto* ptr = wrapped->get();
            return atomic_any(uuid::new_uuid(std::move(wrapped)), ptr, GL::type_of<typename GL::type_erasure::get_type<std::decay_t<ValueType>>::type>());
        };
        template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<atomic_any, std::decay_t<ValueType>>>> static atomic_any instance(ValueType&& value) noexcept {
            auto wrapped = GL::type_erasure::wrap(std::forward<ValueType>(value));
            auto* ptr = wrapped->get();
            return atomic_any(uuid::new_uuid(std::move(wrapped)), ptr, GL::type_of<typename GL::type_erasure::get_type<std::decay_t<ValueType>>::type>());
        };
        static atomic_any instance(atomic_any&& value) noexcept {
            return std::forward<atomic_any>(value);
        };
        static atomic_any& instance(atomic_any& value) noexcept {
            return value;
        };
        static atomic_any const& instance(const atomic_any& value) noexcept {
            return value;
        };
        static atomic_any instance(any&& value) noexcept;
        static atomic_any instance(any& value) noexcept;
        static atomic_any instance(const any& value) noexcept;

        operator bool() const noexcept {
            return m_ptr;
        };
        bool empty() const noexcept {
            return !m_ptr;
        };
        friend bool operator==(const atomic_any& a, const atomic_any& b) noexcept { return a.m_ptr == b.m_ptr; };
        friend bool operator!=(const atomic_any& a, const atomic_any& b) noexcept { return a.m_ptr != b.m_ptr; };
        friend bool operator<(const atomic_any& a, const atomic_any& b) noexcept { return a.m_ptr < b.m_ptr; };
        friend bool operator<=(const atomic_any& a, const atomic_any& b) noexcept { return a.m_ptr <= b.m_ptr; };
        friend bool operator>(const atomic_any& a, const atomic_any& b) noexcept { return a.m_ptr > b.m_ptr; };
        friend bool operator>=(const atomic_any& a, const atomic_any& b) noexcept { return a.m_ptr >= b.m_ptr; };

        bool operator&(int p_modifiers) const noexcept {
            return m_type & p_modifiers;
        };
        atomic_any operator|(int p_modifiers) const noexcept {
            atomic_any out(*this);
            out.m_type |= p_modifiers;
            return out;
        };
        atomic_any operator+(int p_modifiers) const noexcept {
            atomic_any out(*this);
            out.m_type |= p_modifiers;
            return out;
        };
        atomic_any operator-(int p_modifiers) const noexcept {
            atomic_any out(*this);
            out.m_type -= p_modifiers;
            return out;
        };
        atomic_any& operator|=(int p_modifiers) noexcept {
            std::unique_lock locked1(mut);
            m_type |= p_modifiers;
            return *this;
        };
        atomic_any& operator+=(int p_modifiers) noexcept {
            std::unique_lock locked1(mut);
            m_type += p_modifiers;
            return *this;
        };
        atomic_any& operator-=(int p_modifiers) noexcept {
            std::unique_lock locked1(mut);
            m_type -= p_modifiers;
            return *this;
        };

    protected:
        GL::shared_ptr<GL::type_erasure::any_data>& get_underlying_ptr() const noexcept {
            std::shared_lock locked1(mut);
            if ((m_uuid & uuid::INV_FLAGS) > 0)
                return uuid::get_uuid(m_uuid).data;
            else {
                static GL::shared_ptr<GL::type_erasure::any_data> out{ nullptr };
                return out;
            }
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
            template <class VType> static decltype(auto) DoCast_Shared_fast(GL::shared_ptr<GL::type_erasure::any_data>& ptr) noexcept {
                return GL::static_pointer_cast<VType>(ptr->get(GL::shared_ptr<GL::type_erasure::any_data>(ptr)));
            };
            template <class VType> static decltype(auto) DoCast_StdShared_fast(GL::shared_ptr<GL::type_erasure::any_data>& ptr) noexcept {
                return std::static_pointer_cast<VType>(ptr->get_std(GL::shared_ptr<GL::type_erasure::any_data>(ptr)));
            };
            template<typename VType> static decltype(auto) DoCast_Unshared_fast(void* container) /*noexcept*/ {
                static constexpr bool is_ptr{ std::is_pointer_v<VType> };

                if constexpr (is_ptr) {
                    return reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container);
                }
                else {
                    return *reinterpret_cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>(container);
                }
            };

        public:
            template<typename T> static decltype(auto) DoCast(atomic_any* p) /*noexcept*/ {
                typedef typename is_SharedPtr_class<T>::type isShared;
                typedef typename is_stdSharedPtr_class<T>::type isStdShared;

                static constexpr bool is_shared_ptr{ isShared::value };
                static constexpr bool is_std_shared_ptr{ isStdShared::value };
                static constexpr bool is_ptr{ std::is_pointer_v<T> };
                static constexpr bool is_ref{ std::is_reference_v<T> };
                static constexpr bool is_const{ std::is_const_v<T> };
                static constexpr bool is_atomic_any{ std::is_same_v<atomic_any, std::decay_t<T>> };

                if (!p) {
                    if constexpr (is_atomic_any) {
                        static atomic_any out;
                        return *&out;
                    }
                    else {
                        if constexpr (is_shared_ptr) {
                            return GL::shared_ptr<typename GL::type_erasure::get_type<T>::type>(nullptr);
                        }
                        else if constexpr (is_std_shared_ptr) {
                            return std::shared_ptr<typename GL::type_erasure::get_type<T>::type>(nullptr);
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
                }
                else {
                    if constexpr (is_atomic_any) return *p;
                    else {
                        if constexpr (is_shared_ptr) {
                            if constexpr (is_ptr) throw("Casting atomic_any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            else if constexpr (is_ref) throw("Casting atomic_any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            return DoCast_Shared_fast<typename GL::type_erasure::get_type<T>::type>(p->get_underlying_ptr());
                        }
                        else if constexpr (is_std_shared_ptr) {
                            if constexpr (is_ptr) {
                                throw("Casting atomic_any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            }
                            else if constexpr (is_ref) {
                                throw("Casting atomic_any to shared_ptr<T>* or shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to shared_ptr<T>.");
                            }
                            return DoCast_StdShared_fast<typename GL::type_erasure::get_type<T>::type>(p->get_underlying_ptr());
                        }
                        else {
                            std::shared_lock locked1(p->mut);
                            return DoCast_Unshared_fast<T>(p->m_ptr);
                        }
                    }
                }
            };

        };

    public:
        template<typename VType, typename = std::enable_if_t<!std::is_same_v<atomic_any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<atomic_any*>(this)); };

        template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<atomic_any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        atomic_any& cast() const noexcept { return *const_cast<atomic_any*>(this); };

        template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<atomic_any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
        atomic_any* cast() const noexcept { return const_cast<atomic_any*>(this); };

        /// \todo it is a performance increase to allow the return value to be "referenced" rather than incremented/decremented properly, however, is this safe in practice?
        template <typename T> __declspec(noinline) static atomic_any wrap_member(atomic_any const& parent, T const& ref) noexcept {
            std::shared_lock locked1(parent.mut);

            if ((parent.m_uuid & uuid::TEMPORARY) > 0) {
                return atomic_any((unsigned long)parent.m_uuid, &const_cast<T&>(ref), GL::type_of<T const&>());
            }
            else {
                unsigned long uuid = parent.m_uuid & uuid::INV_FLAGS;
                if (uuid > 0) GL::interlocked::increment(uuid::get_uuid(uuid).count);
                return atomic_any(std::move(uuid), &const_cast<T&>(ref), GL::type_of<T const&>());
            }
        };
        /// \todo it is a performance increase to allow the return value to be "referenced" rather than incremented/decremented properly, however, is this safe in practice?
        template <typename T> __declspec(noinline) static atomic_any wrap_member(atomic_any const& parent, T& ref) noexcept {
            std::shared_lock locked1(parent.mut);

            if ((parent.m_uuid & uuid::TEMPORARY) > 0) {
                return atomic_any((unsigned long)parent.m_uuid, &ref, GL::type_of<T&>());
            }
            else {
                unsigned long uuid = parent.m_uuid & uuid::INV_FLAGS;
                if (uuid > 0) GL::interlocked::increment(uuid::get_uuid(uuid).count);
                return atomic_any(std::move(uuid), &ref, GL::type_of<T&>());
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
        explicit var(any&& data_f) : p_data(std::move(data_f)) {
            p_type = p_data.type();
        }
        var(var const&) = default;
        var(var&&) = default;
        var& operator=(var const&) = default;
        var& operator=(var&&) = default;
        ~var() = default;

    public:
        GL::type const& get_type() const {
            return p_type;
        };
        GL::type get_actual_type() {
            return p_data.type();
        };
        GL::atomic_any& get_data() {
            return p_data;
        };
        void set_data(GL::any&& data_f) {
            p_data = std::move(data_f);
            p_type = p_data.type();
        };

    private:
        GL::atomic_any
            p_data; // may be "updated" at any time and therefore should be thread-safe. 
        GL::type
            p_type;
    };

    class is_template {
    public:
        template <GL::type_hash_t index> static GL::type type(GL::string const& name) {
            return GL::type((((GL::type_hash_t)GL::type::TemplateType + (GL::type_hash_t)GL::type::CppType) << impl::cached_type::OFFSET) | (name.hash() & GL::impl::cached_type::MAGIC_MASK2 & ~0xF) | index);
        };
        static GL::type type(GL::type_hash_t index, GL::string const& name) {
            return GL::type((((GL::type_hash_t)GL::type::TemplateType + (GL::type_hash_t)GL::type::CppType) << impl::cached_type::OFFSET) | (name.hash() & GL::impl::cached_type::MAGIC_MASK2 & ~0xF) | index);
        };
        static int index(GL::type const& what) {
            if (what & GL::type::TemplateType) {
                return (int)(what.get_base_hash() & 0xF);
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
