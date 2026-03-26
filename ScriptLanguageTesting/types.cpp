#include "types.h"
#include "atomic_maps.h"
#include "ticket_dispensor.h"
#include "atomic_vector.h"
#include "atomic_tree.h"
#include <queue>
#include <map>

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
                    out.base_classes_ordered = {};
                }
            }
            return ticket;
        };
        void return_scripted_type(size_t ticket) {
            auto& out = scripted_types[ticket];            
            out.name = "";
            out.T_size = std::numeric_limits<size_t>::max();            
            out.base_classes = {};
            out.base_classes_ordered = {};
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
                for (auto& x : this->base_classes_ordered) {
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
                        if (this->base_classes.insert(base).second) {
                            this->base_classes_ordered.push_back(base);
                        }
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
                        if (this->base_classes.insert(base).second) {
                            this->base_classes_ordered.push_back(base);
                        }
                        return true;
                    }
                }
            }
            return false;
        };
        bool cached_type::match_base_hash(size_t to_match) const {
            if (base_hash == to_match) return true;
            if (this->base_classes.find(to_match) != this->base_classes.end()) return true;
            for (auto& x : base_classes_ordered) {
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
        GL::string out = get_base(*this).name;
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
        get_base(*this).name = new_name;
        return true;
    };
    std::vector<type> type::all_base_types(bool local_only) const {
        if (local_only) {
            std::vector<type> out;
            if (this->is_cpp_type()) {
                for (auto& x : get_base(*this).base_classes_ordered) {
                    auto& Base = impl::get_impl(x);
                    out.push_back(type(Base.base_hash) + GL::type::CppType);
                }
            }
            else {
                for (auto& x : get_base(*this).base_classes_ordered) {
                    auto& Base = impl::get_scripted_type(x);
                    out.push_back(type(Base.base_hash));
                }
            }
            return out;
        }
        else {
            std::map<int, std::vector<type>> collection;
            std::queue< std::pair<int, GL::type> > types_to_try;
            std::set<GL::type> attempted_types;
            types_to_try.push({ 0, *this - GL::type::Reference - GL::type::Const - GL::type::Temporary });
            while (types_to_try.size() > 0) {
                std::pair<int, GL::type> this_t = types_to_try.front();
                types_to_try.pop();
                if (attempted_types.find(this_t.second) == attempted_types.end()) {
                    attempted_types.insert(this_t.second);
                    collection[this_t.first].push_back(this_t.second);
                    for (GL::type const& base_type : this_t.second.all_base_types()) {
                        types_to_try.push({ this_t.first + 1, base_type });
                    }
                }
            }
            std::vector<GL::type> out;
            out.reserve(attempted_types.size() + 16);
            for (auto& x : collection) {
                out.insert(out.end(), x.second.begin(), x.second.end());
            }
            return out;
        }
    };
    // returns true if this is found to be a child of the parent type (id'd by its base hash) 
    bool type::is_derived_from(type const& base) const {
        return get_base(*this).is_derived_from(base.get_base_hash());        

    };
    // returns true if this is found to be a parent of the derived type (id'd by its base hash) 
    bool type::is_base_of(type const& derived) const {
        return get_base(*this).is_base_of(derived.get_base_hash());
    };
    // attempts to include the specified hash as a base of this class.
    bool type::add_base(type const& base) {
        if (this->is_cpp_type() == base.is_cpp_type()) {
            return get_base(*this).add_base(base.get_base_hash());
        }
        else {
            return false;
        }
    };

    bool type::match_base_hash(type const& to_match) const {
        return get_base(*this).match_base_hash(to_match.get_base_hash());
    };
    size_t type::size() const {
        return get_base(*this).T_size;
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

        GL::type_of<std::string>().try_update_name("string"); // on-purpose, this is a stress test for the system to handle classes that share the same name in alternative namespaces. 
        GL::type_of<GL::string>().try_update_name("string");        
        GL::type_of<GL::any>().try_update_name("any");
        GL::type_of<GL::any::fast_any>().try_update_name("fast_any");
        GL::type_of<GL::type>().try_update_name("type");
        GL::type_of<GL::var>().try_update_name("var");
        GL::type_of<GL::value>().try_update_name("value"); // the implimentations of units (meter, foot, etc) each correct their own name during definition. 
        GL::type_of<GL::undefined>().try_update_name("undefined");
        GL::type_of<GL::template_parameter<0>>().try_update_name("{0}");
        GL::type_of<GL::template_parameter<1>>().try_update_name("{1}");
        GL::type_of<GL::template_parameter<2>>().try_update_name("{2}");
        GL::type_of<GL::template_parameter<3>>().try_update_name("{3}");
        GL::type_of<GL::template_parameter<4>>().try_update_name("{4}");
        GL::type_of<GL::template_parameter<5>>().try_update_name("{5}");
        GL::type_of<GL::template_parameter<6>>().try_update_name("{6}");
        GL::type_of<GL::template_parameter<7>>().try_update_name("{7}");
        GL::type_of<GL::template_parameter<8>>().try_update_name("{8}");
        GL::type_of<GL::template_parameter<9>>().try_update_name("{9}");
        GL::type_of<GL::template_parameter<10>>().try_update_name("{10}");
        GL::type_of<GL::template_parameter<11>>().try_update_name("{11}");
        GL::type_of<GL::template_parameter<12>>().try_update_name("{12}");
        GL::type_of<GL::template_parameter<13>>().try_update_name("{13}");
        GL::type_of<GL::template_parameter<14>>().try_update_name("{14}");
        GL::type_of<GL::template_parameter<15>>().try_update_name("{15}");

        return true;
    }();
};
