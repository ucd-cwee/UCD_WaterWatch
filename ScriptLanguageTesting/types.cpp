#include "types.h"
#include "atomic_maps.h"
#include "ticket_dispensor.h"
#include "atomic_vector.h"

namespace GL {
    namespace impl {
        // boost::type_info hashes are unpredictable and therefore we must use a map.
        static concurrency::concurrent_unordered_map<size_t, impl::cached_type> builtin_cpp_types; 
        // atomic_vector because ticket system will prefer small values. 
        static GL::atomic_vector< impl::cached_type > scripted_types; 
        // ticket system helps ensure values remain small. 
        static GL::ticket_dispensor scripted_types_ticket_dispensor; 

        size_t checkout_scripted_type(GL::string type_name) {
            size_t ticket = scripted_types_ticket_dispensor.get_ticket();
            scripted_types.grow_to_at_least(ticket);

            auto& out = scripted_types[ticket];
            if (out.base_hash == 0) {
                if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&out.base_hash), ticket, 0) == 0) {
                    out.name = type_name;
                }
            }
            return ticket;
        };
        void return_scripted_type(size_t ticket) {
            auto& out = scripted_types[ticket];
            out.name = "";
            out.base_hash = 0;
            scripted_types_ticket_dispensor.return_ticket(ticket);
        };
        cached_type& get_scripted_type(size_t hash) {
            return scripted_types[hash];
        };

        cached_type& get_impl(size_t hash) {
            return builtin_cpp_types[hash];
        };
        bool cached_type::is_derived_from(size_t base) const {
            if (this->base_hash == base) {
                return true;
            }
            else {
                if (this->base_classes.find(base) != this->base_classes.end()) return true;
                for (auto& x : this->base_classes) {
                    auto& Base = get_impl(x);
                    if (Base.is_derived_from(base)) {
                        return true;
                    }
                }
                return false;
            }
        };
        bool cached_type::is_base_of(size_t derived) const {
            auto& base = get_impl(derived);
            return base.is_derived_from(this->base_hash);
        };
        bool cached_type::add_base(size_t base) {
            auto& Base = get_impl(base);
            if (this->is_derived_from(base) || Base.is_derived_from(this->base_hash))
                return false;
            else {
                this->base_classes.insert(base);
                return true;
            }
        };
        bool cached_type::match_base_hash(size_t to_match) const {
            if (base_hash == to_match) return true;
            if (this->base_classes.find(to_match) != this->base_classes.end()) return true;
            for (auto& x : base_classes) {
                auto& base = get_impl(x);
                if (base.match_base_hash(to_match)) return true;
            }
            return false;
        };

    };

    auto& get_base(type const& from) {
        if (from.is_cpp_type()) {
            return impl::get_impl(from.get_base_hash());
        }
        else {
            return impl::get_scripted_type(from.get_base_hash());
        }
    };

    GL::string type::name() const {
        auto& Base = get_base(*this);
        if (is_temp()) return Base.name + "&&";
        else if (is_const() && is_ref()) return "const " + Base.name + "&";
        else if (is_const() && !is_ref()) return "const " + Base.name;
        else if (!is_const() && is_ref()) return Base.name + "&";
        else return Base.name;
    };

    // returns true if this is found to be a child of the parent type (id'd by its base hash) 
    bool type::is_derived_from(type const& base) const {
        return get_base(*this).is_derived_from(get_base(base).base_hash);
    };
    // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
    bool type::is_base_of(type const& derived) const {
        return get_base(*this).is_base_of(get_base(derived).base_hash);
    };
    // attempts to include the specified hash as a base of this class.
    bool type::add_base(type const& base) {
        return get_base(*this).add_base(get_base(base).base_hash);
    };

    bool type::match_base_hash(type const& to_match) const {
        return get_base(*this).match_base_hash(get_base(to_match).base_hash);
    };


};
