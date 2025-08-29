#pragma once

#include <map>
#include <set>
#include "Strings.h"
#include "atomic_shared_ptr.h"

namespace GL {





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
            base_hash = 0;
        const GL::string
            name; // not an atomic_string, since the name is meant to be read-only. 
        std::map<size_t, std::shared_ptr<base_type>> // link to the base classes for this type.
            base_classes; // underlying_hash to type

        base_type(size_t base_hash_p = 0, GL::string const& name_p = "") noexcept
            : base_hash(base_hash_p)
            , name(name_p)
        {};
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
        void add_parent(std::shared_ptr<base_type> const& parent) {
            if (is_child_of(*parent)) return;
            else this->base_classes[parent->base_hash] = parent;
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

        type(size_t base_hash_p = 0, size_t modifiers_p = Modifiers::Void, GL::string const& name_p = "") noexcept
            : base_type(base_hash_p, name_p)
            , modifiers(modifiers_p)
            , actual_hash(base_hash_p ^ (modifiers_p + 0x9e3779b9 + (base_hash_p << 6) + (base_hash_p >> 2)))
        {};

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


};