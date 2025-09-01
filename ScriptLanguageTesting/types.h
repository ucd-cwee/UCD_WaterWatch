#pragma once

#include <map>
#include <set>
#include "Strings.h"
#include "atomic_shared_ptr.h"
#include "atomic_maps.h"
#include <boost/type_index.hpp>


namespace GL {
    namespace util {
        template<typename T> static const auto& type_id() {
            static auto typeIdOfT{ boost::typeindex::type_id<T>() };
            return typeIdOfT.type_info();
        };
    };


    // base class type, intended to be derived from.
    class base_type {
    public:
        enum Modifiers {
            Const = 1
            , Reference = 2
            , Temporary = 4
            , Any = 8
            , Void = 16
            , Static = 32
            , ValueType = 64
        };

        size_t
            base_hash;
        const GL::string
            name; // not an atomic_string, since the name is meant to be read-only. 
        std::map<size_t, std::shared_ptr<base_type>> // link to the base classes for this type.
            base_classes; // underlying_hash to type

        base_type(size_t base_hash_p = 0, GL::string const& name_p = "") noexcept
            : base_hash(base_hash_p)
            , name(name_p)
        {};
        base_type(std::vector<std::shared_ptr<base_type>> const& bases, GL::string const& name_p = "") noexcept
            : base_hash(0)
            , name(name_p)
        {
            base_hash = bases[0]->base_hash;
            for (int i = 1; i < bases.size(); ++i)
                if (auto x = bases[i]) {
                    base_classes[x->base_hash] = x;
                    base_hash ^= (x->base_hash + 0x9e3779b9 + (base_hash << 6) + (base_hash >> 2));
                }
        };

        bool is_child_of(base_type const& parent) const {
            if (this->base_hash == parent.base_hash) {
                return true;
            }
            else {
                for (auto& x : this->base_classes) {
                    if (x.first == parent.base_hash) {
                        return true;
                    }
                }
                for (auto& x : this->base_classes) {
                    if (x.second->is_child_of(parent)) {
                        return true;
                    }
                }
                return false;
            }
        };
        bool is_parent_of(base_type const& child) const {
            return child.is_child_of(*this);
        };
        bool add_parent(std::shared_ptr<base_type> const& parent) {
            if (this->is_child_of(*parent) || parent->is_child_of(*this))
                return false;
            else {
                this->base_classes[parent->base_hash] = parent;
                return true;
            }
        };
        bool match_base_hash(size_t to_match) const {
            if (base_hash == to_match) return true;
            for (auto& x : base_classes) {
                if (x.first == to_match) return true;
            }
            for (auto& x : base_classes) {
                if (x.second->match_base_hash(to_match)) return true;
            }
            return false;
        };        
    };

    // derived or base class type. 
    class type : public base_type {
    private:
        unsigned short
            modifiers = 0;
        size_t
            actual_hash = 0;
    public:

        type(size_t base_hash_p = 0, unsigned short modifiers_p = Modifiers::Void, GL::string const& name_p = "") noexcept
            : base_type(base_hash_p, name_p)
            , modifiers(modifiers_p)
        {
            if (modifiers != 0) actual_hash = (this->base_hash ^ (modifiers_p + 0x9e3779b9 + (this->base_hash << 6) + (this->base_hash >> 2)));
            else actual_hash = this->base_hash;
            if (is_void()) actual_hash = 0;
        };
        type(std::vector<std::shared_ptr<base_type>> const& bases, unsigned short modifiers_p, GL::string const& name_p) noexcept
            : base_type(bases, name_p)
            , modifiers(modifiers_p)
        {
            if (modifiers != 0) actual_hash = (this->base_hash ^ (modifiers_p + 0x9e3779b9 + (this->base_hash << 6) + (this->base_hash >> 2)));
            else actual_hash = this->base_hash;
            if (is_void()) actual_hash = 0;
        };


        int const& get_modifiers() const { return modifiers; };
        size_t const& get_hash() const { return actual_hash; };

        // Returns true if the types are similar enough to be casted for free (0 cost)
        static bool can_free_cast(type const& from, type const& to) {
            if (from.actual_hash == to.actual_hash) return true;
            if (from.match_base_hash(to.base_hash)) {
                // anything can convert into const T&
                if (to.is_const_ref()) return true;

                // cannot cast-away the const-ness
                if (from.is_const() && !to.is_const()) return false;

                // temporary (T&&) can be used for a base
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
        // Returns true if the types are similar enough to be casted for free (0 cost)
        bool can_free_cast(type const& to) const { return can_free_cast(*this, to); };
        // Returns true if the types are the same foundational type (may not be zero cost to convert)
        bool can_cast(type const& to) const {
            return this->match_base_hash(to.base_hash);
        };

        bool is_temp() const noexcept { return modifiers & Modifiers::Temporary; };
        bool is_const() const noexcept { return is_temp() ? false : (modifiers & Modifiers::Const); };
        bool is_ref() const noexcept { return is_temp() ? false : (modifiers & Modifiers::Reference); };
        bool is_const_ref() const noexcept { return is_temp() ? false : (modifiers & (Modifiers::Const | Modifiers::Reference)); };
        bool is_base() const noexcept { return modifiers == 0; };
        bool is_any() const noexcept { return modifiers & Modifiers::Any; };
        bool is_void() const noexcept { return modifiers & Modifiers::Void; };
        bool is_static() const noexcept { return modifiers & Modifiers::Static; };
        bool is_value() const noexcept { return modifiers & Modifiers::ValueType; };

        GL::string get_name() const {
            auto out{ name.remove_leading_and_trailing(':').remove_suffix(" __cdecl(void)") };
            if (is_temp()) return out + "&&";
            else if (is_const() && is_ref()) return "const " + out + "&";
            else if (is_const() && !is_ref()) return "const " + out;
            else if (!is_const() && is_ref()) return out + "&";
            else return out;
        };

        //// Operators
        friend bool operator==(const type& a, const type& b) noexcept { return a.get_hash() == b.get_hash(); };
        friend bool operator!=(const type& a, const type& b) noexcept { return a.get_hash() != b.get_hash(); };
        friend bool operator<(const type& a, const type& b) noexcept { return a.get_hash() < b.get_hash(); };
        friend bool operator<=(const type& a, const type& b) noexcept { return a.get_hash() <= b.get_hash(); };
        friend bool operator>(const type& a, const type& b) noexcept { return a.get_hash() > b.get_hash(); };
        friend bool operator>=(const type& a, const type& b) noexcept { return a.get_hash() >= b.get_hash(); };

        bool operator&(int p_modifiers) const {
            return modifiers & p_modifiers;
        };
        type operator|(int p_modifiers) const {
            type out = *this;
            out.modifiers = this->modifiers | p_modifiers;
            out.actual_hash = base_hash ^ (out.modifiers + 0x9e3779b9 + (base_hash << 6) + (base_hash >> 2));
            return out;
        };
        type operator+(int modifier) const {
            type out = *this;
            out.modifiers = this->modifiers | modifier;
            out.actual_hash = base_hash ^ (out.modifiers + 0x9e3779b9 + (base_hash << 6) + (base_hash >> 2));
            return out;
        };
        type operator-(int modifier) const {
            type out = *this;
            out.modifiers = this->modifiers & ~modifier;
            out.actual_hash = base_hash ^ (out.modifiers + 0x9e3779b9 + (base_hash << 6) + (base_hash >> 2));
            return out;
        };

    };

    namespace impl {        
        static GL::atomic_map<size_t, std::shared_ptr<base_type>>& base_types() noexcept {
            static GL::atomic_map<size_t, std::shared_ptr<base_type>> out;
            return out;
        };
        template<typename BaseType> __forceinline static std::shared_ptr<base_type>& base_type_ptr() noexcept {
            static auto const& underlying_type = util::type_id<BaseType>();
            static auto const& void_type = util::type_id<void>();
            //static auto const& any_type = util::type_id<utilities::any>();
            static auto const const_modifier = std::is_const<typename std::remove_pointer_t<typename std::remove_reference_t<BaseType>>>::value ? type::Modifiers::Const : 0;
            static auto const ref_modifier = std::is_reference<typename std::remove_pointer<BaseType>::type>::value ? type::Modifiers::Reference : 0;
            static auto const void_modifier = (void_type.hash_code() == underlying_type.hash_code()) ? type::Modifiers::Void : 0;
            static auto const any_modifier = 0; //(any_type.hash_code() == underlying_type.hash_code()) ? type::Modifiers::Any : 0;
            static auto const value_type = 0; //  std::is_base_of_v<GoodLang::Units::value, BaseType> || std::is_same_v<BaseType, GoodLang::Units::value>;

            static std::shared_ptr<type> base_as_type{
                std::make_shared<type>(void_modifier ? 0 : underlying_type.hash_code(), const_modifier | ref_modifier | void_modifier | any_modifier | value_type, GL::string(std::string_view(underlying_type.name())))
            };
            static std::shared_ptr<base_type> base{
                std::dynamic_pointer_cast<base_type>(base_as_type)
            };
            static size_t ref{ base_types().insert(base->base_hash, std::shared_ptr<base_type>(base)).first };

            return base;
        }
    }

    template<typename T> __forceinline static type& type_of() noexcept {
        using BaseType = typename std::remove_const_t<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
        static auto const& underlying_type = util::type_id<BaseType>();
        static auto const& void_type = util::type_id<void>();
        //static auto const& any_type = util::type_id<utilities::any>();
        static auto const const_modifier = std::is_const<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>::value ? type::Modifiers::Const : 0;
        static auto const ref_modifier = std::is_reference<typename std::remove_pointer<T>::type>::value ? type::Modifiers::Reference : 0;
        static auto const void_modifier = (void_type.hash_code() == underlying_type.hash_code()) ? type::Modifiers::Void : 0;
        static auto const any_modifier = 0; // (any_type.hash_code() == underlying_type.hash_code()) ? type::Modifiers::Any : 0;
        static auto const value_type = 0; //  std::is_base_of_v<GoodLang::Units::value, BaseType> || std::is_same_v<BaseType, GoodLang::Units::value>;

#if 0
        static std::function<utilities::any(utilities::any const&)> copy_constructor = [](utilities::any const& from) -> utilities::any {
            if constexpr (std::is_same_v<base_type, utilities::any>) {
                return from;
            }
            else if constexpr (std::is_copy_constructible_v<base_type>) {
                if (from.current_type().get_underlying_hash() == util::type_id<base_type>().hash_code()) {
                    return utilities::any(base_type{ *static_cast<base_type*>(from.ptr()) }, utilities::type::Temporary);
                }
                else {
                    return from;
                }
            }
            else {
                return from;
            }

            // To-Do, the return type should be set to "temporary" to improve type engine

            /* scripted objects -->
            auto& dynObj = from.cast<DynamicObject>();
            return DynamicObject(dynObj);
            <-- scripted objects */
        };
        static std::function<utilities::any(utilities::any const&)> constructor_from_value = [](utilities::any const& from) -> utilities::any {
            if constexpr (std::is_same_v<base_type, utilities::any>) {
                return from;
            }
            else if constexpr (std::is_copy_constructible_v<base_type>
                && (std::is_constructible_v<base_type, GoodLang::Units::value&> || std::is_assignable_v<base_type, GoodLang::Units::value&>)) {
                if (from.current_type().is_value()) {
                    return utilities::any(base_type{ *static_cast<GoodLang::Units::value*>(from.ptr()) }, utilities::type::Temporary);
                }
                return from;
                //if (from.current_type().get_underlying_hash() == util::type_id<GoodLang::Units::value>().hash_code()) {
                //    return base_type{ *static_cast<GoodLang::Units::value*>(from.ptr()) };
                //}
            }
            else {
                return from;
            }
        };
#endif
     
        static std::shared_ptr<GL::base_type>& base_ref{ impl::base_type_ptr<BaseType>() };
        if constexpr (const_modifier == ref_modifier) {
            return *reinterpret_cast<type*>(base_ref.get());
        }
        else {
            static type out({ base_ref }, const_modifier | ref_modifier | void_modifier | any_modifier | value_type, GL::string(std::string_view(underlying_type.name())));
            return out;
        }
    };







};