#include "types.h"
#include "atomic_maps.h"
#include "ticket_dispensor.h"
#include "atomic_vector.h"
#include "atomic_tree.h"
#include <queue>

namespace GL {
    namespace impl {
#define use_btree_for_cpp_types

        static 
#ifdef use_btree_for_cpp_types
        GL::binary_search_tree< impl::cached_type, size_t, 32>
#else
        std::unordered_map< size_t, impl::cached_type> 
#endif
            builtin_cpp_types;
        
        static GL::atomic_vector< impl::cached_type > scripted_types; // atomic_vector because ticket system will prefer small values.         
        static GL::ticket_dispensor scripted_types_ticket_dispensor; // ticket system helps ensure values remain small. 

        size_t checkout_scripted_type(GL::string type_name) {
            size_t ticket = scripted_types_ticket_dispensor.get_ticket() - 1;
            scripted_types.grow_to_at_least(ticket + 1); // desired size

            auto& out = scripted_types[ticket];
            if (out.base_hash == 0) {
                if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&out.base_hash), ticket, 0) == 0) {
                    out.name = type_name;
                    out.T_size = std::numeric_limits<size_t>::max();
                    out.base_classes = {};
                    out.instance_by_copy = [](GL::any const& orig) -> GL::any {
                        return orig;
                    };
                    out.instance_by_value = [](GL::any const& orig) -> GL::any {
                        return orig;
                    };
                    out.instance = []() -> GL::any {
                        return GL::any{};
                    };
                }
            }
            return ticket;
        };
        void return_scripted_type(size_t ticket) {
            auto& out = scripted_types[ticket];            
            out.name = "";
            out.T_size = std::numeric_limits<size_t>::max();            
            out.base_classes = {};
            out.base_hash = 0;
            scripted_types_ticket_dispensor.return_ticket(ticket + 1);
        };
        cached_type& get_scripted_type(size_t hash) {
            if (hash < scripted_types.size()) {
                return scripted_types[hash];
            } 
            else {
#ifndef use_btree_for_cpp_types
                return builtin_cpp_types[hash];
#else
                if (auto* p = builtin_cpp_types.NodeFind(hash); p) {
                    return *p->object();
                }
                else {
                    return *builtin_cpp_types.Add(new impl::cached_type(), hash)->object();
                }
#endif
            }
        };
        cached_type& get_impl(size_t hash) {
#ifndef use_btree_for_cpp_types
            return builtin_cpp_types[hash];
#else
            if (auto* p = builtin_cpp_types.NodeFind(hash); p) {
                return *p->object();
            }
            else {
                return *builtin_cpp_types.Add(new impl::cached_type(), hash)->object();
            }
#endif
        };
#undef use_btree_for_cpp_types
        bool cached_type::is_derived_from(size_t base) const {
            if (this->base_hash == base) {
                return true;
            }
            else {
                if (this->base_classes.find(base) != this->base_classes.end()) return true;
                for (auto& x : this->base_classes) {
                    if (this->is_cpp_type()) {
                        auto& Base = get_impl(x);
                        if (Base.is_derived_from(base)) {
                            return true;
                        }
                    }
                    else {
                        auto& Base = get_scripted_type(x);
                        if (Base.is_derived_from(base)) {
                            return true;
                        }
                    }
                }
                return false;
            }
        };
        bool cached_type::is_base_of(size_t derived) const {
            if (this->is_cpp_type()) {
                auto& base = get_impl(derived);
                return base.is_derived_from(this->base_hash);
            }
            else {
                auto& base = get_scripted_type(derived);
                return base.is_derived_from(this->base_hash);
            }  
        };
        bool cached_type::add_base(size_t base) {
            if (this->is_cpp_type()) {
                auto& Base = get_impl(base);
                if (Base.is_cpp_type() && Base.T_size != 0) {
                    if (this->is_derived_from(base) || Base.is_derived_from(this->base_hash))
                        return false;
                    else {
                        this->base_classes.insert(base);
                        return true;
                    }
                }
            }
            else {
                auto& Base = get_scripted_type(base);
                if (!Base.is_cpp_type()) {
                    if (this->is_derived_from(base) || Base.is_derived_from(this->base_hash))
                        return false;
                    else {
                        this->base_classes.insert(base);
                        return true;
                    }
                }
            }
            return false;
        };
        bool cached_type::match_base_hash(size_t to_match) const {
            if (base_hash == to_match) return true;
            if (this->base_classes.find(to_match) != this->base_classes.end()) return true;
            for (auto& x : base_classes) {
                if (this->is_cpp_type()) {
                    auto& base = get_impl(x);
                    if (base.match_base_hash(to_match)) return true;
                }
                else {
                    auto& base = get_scripted_type(x);
                    if (base.match_base_hash(to_match)) return true;
                }
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
        GL::string out = Base.name;
        if (is_const()) out = "const " + out;
        if (is_ref()) out = out + "&";
        if (is_temp()) out = out + "&&";
        return out;

        //auto& Base = get_base(*this);
        //if (is_temp()) return Base.name + "&&";
        //else if (is_const() && is_ref()) return "const " + Base.name + "&";
        //else if (is_const() && !is_ref()) return "const " + Base.name;
        //else if (!is_const() && is_ref()) return Base.name + "&";
        //else return Base.name;
    };
    bool type::try_update_name(GL::string const& new_name) {
        //if (this->is_cpp_type()) {
            auto& Base = get_base(*this);
            Base.name = new_name;
            return true;
        //}
        //else {
        //    return false;
        //}
    };
    std::set<type> type::all_base_types(bool local_only) const {
        if (local_only) {
            std::set<type> out;
            if (this->is_cpp_type()) {
                for (auto& x : get_base(*this).base_classes) {
                    auto& Base = impl::get_impl(x);
                    out.insert(type(Base.base_hash) + GL::type::CppType);
                }
            }
            else {
                for (auto& x : get_base(*this).base_classes) {
                    auto& Base = impl::get_scripted_type(x);
                    out.insert(type(Base.base_hash));
                }
            }
            return out;
        }
        else {
            std::queue< GL::type > types_to_try;
            types_to_try.push(*this - GL::type::Reference - GL::type::Const - GL::type::Temporary);
            std::set<GL::type> attempted_types;
            while (types_to_try.size() > 0) {
                GL::type this_t = types_to_try.front();
                types_to_try.pop();
                if (attempted_types.find(this_t) == attempted_types.end()) {
                    attempted_types.insert(this_t);
                    for (GL::type const& base_type : this_t.all_base_types()) { types_to_try.push(base_type); }
                }
            }
            return attempted_types;

            //std::queue< GL::type > types_to_try;
            //types_to_try.push(*this);
            //std::set<GL::type> attempted_types;
            //while (types_to_try.size() > 0) {
            //    GL::type this_t = types_to_try.front();
            //    types_to_try.pop();
            //    if (attempted_types.find(this_t) == attempted_types.end()) {
            //        attempted_types.insert(this_t);
            //        for (GL::type const& base_type : this_t.all_base_types(true)) { types_to_try.push(base_type); }
            //    }
            //}
            //attempted_types.erase(*this);
            //return attempted_types;
        }
    };
    // returns true if this is found to be a child of the parent type (id'd by its base hash) 
    bool type::is_derived_from(type const& base) const {
        return get_base(*this).is_derived_from(get_base(base).base_hash);
    };
    // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
    bool type::is_base_of(type const& derived) const {
        //if (this->is_cpp_type() == derived.is_cpp_type()) {
            return get_base(*this).is_base_of(get_base(derived).base_hash);
        //}
        //else {
        //    return false;
        //}
    };
    // attempts to include the specified hash as a base of this class.
    bool type::add_base(type const& base) {
        if (this->is_cpp_type() == base.is_cpp_type()) {
            return get_base(*this).add_base(get_base(base).base_hash);
        }
        else {
            return false;
        }
    };

    bool type::match_base_hash(type const& to_match) const {
        auto& this_base = get_base(*this);
        auto& that_base = get_base(to_match);
        return this_base.match_base_hash(that_base.base_hash);
    };
    size_t type::size() const {
        auto& this_base = get_base(*this);
        return this_base.T_size;
    };
    // const value_t& to This&&
    GL::any type::instance_by_value(GL::any const& rhs) const {
        auto& this_base = get_base(*this);
        return this_base.instance_by_value(rhs);
    };
    // const This& to This&&
    GL::any type::instance_by_copy(GL::any const& rhs) const {
        auto& this_base = get_base(*this);
        return this_base.instance_by_copy(rhs);
    };
    // This&&
    GL::any type::instance() const {
        auto& this_base = get_base(*this);
        return this_base.instance();
    };
    // This&&
    void type::destroy(void* p) const {
        auto& this_base = get_base(*this);
        this_base.destroy(p);
    };

    static auto precompiled_cpp_names = []() -> bool {
        GL::type_of<void>().try_update_name("void");
        GL::type_of<bool>().try_update_name("bool");
        GL::type_of<char>().try_update_name("char");
        GL::type_of<unsigned char>().try_update_name("uchar");
        GL::type_of<short>().try_update_name("short");
        GL::type_of<unsigned short>().try_update_name("ushort");
        GL::type_of<int>().try_update_name("int");
        GL::type_of<unsigned int>().try_update_name("uint");
        GL::type_of<long>().try_update_name("long");
        GL::type_of<unsigned long>().try_update_name("ulong");
        GL::type_of<long long>().try_update_name("llong");
        GL::type_of<unsigned long long>().try_update_name("size_t");
        GL::type_of<float>().try_update_name("float");
        GL::type_of<double>().try_update_name("double");
        GL::type_of<long double>().try_update_name("ldouble");

        GL::type_of<std::string>().try_update_name("std_string");
        GL::type_of<GL::string>().try_update_name("string");        
        GL::type_of<GL::any>().try_update_name("any");
        GL::type_of<GL::any::fast_any>().try_update_name("fast_any");
        GL::type_of<GL::type>().try_update_name("type");
        GL::type_of<GL::value>().try_update_name("value"); // the implimentations of units (meter, foot, etc) each correct their own name during definition. 

        return true;
    }();
};
