#include "types.h"
#include "ticket_dispensor.h"
#include "atomic_vector.h"
#include <queue>
#include <map>

namespace GL {
    type_erasure::any_cast any::cast() const noexcept {
        std::cout << "";
        return type_erasure::any_cast{ const_cast<any*>(this) };
    };
    type_erasure::fast_any_cast any::fast_any::cast() const noexcept {
        std::cout << "";
        return type_erasure::fast_any_cast{ const_cast<fast_any*>(this) };
    };

    bool type::is_any() const noexcept {
        static GL::type_hash_t const h{ util::type_id<any>().hash_code() };
        static GL::type_hash_t const h2{ util::type_id<any::fast_any>().hash_code() };
        return (((hash ^ h) & impl::cached_type::MAGIC_MASK2) == 0) || (((hash ^ h2) & impl::cached_type::MAGIC_MASK2) == 0);

        //static GL::type_hash_t const h{ util::type_id<any>().hash_code() & impl::cached_type::MAGIC_MASK2 };
        //static GL::type_hash_t const h2{ util::type_id<any::fast_any>().hash_code() & impl::cached_type::MAGIC_MASK2 };
        //GL::type_hash_t this_h{ get_base_hash() };
        //return (this_h == h) || (this_h == h2);
    };
    bool type::is_var() const noexcept {
        static GL::type_hash_t const h{ util::type_id<var>().hash_code() };
        return ((hash ^ h) & impl::cached_type::MAGIC_MASK2) == 0;

        // static GL::type_hash_t const h{ util::type_id<var>().hash_code() & impl::cached_type::MAGIC_MASK2 };
        // return (hash & impl::cached_type::MAGIC_MASK2) == h;
    };
    bool type::is_dynamic_object() const noexcept {
        static GL::type_hash_t const h{ util::type_id<dynamic_object>().hash_code() };
        return ((hash ^ h) & impl::cached_type::MAGIC_MASK2) == 0;

        //static GL::type_hash_t const h{ util::type_id<dynamic_object>().hash_code() & impl::cached_type::MAGIC_MASK2 };
        //GL::type_hash_t this_h{ get_base_hash() };
        //return this_h == h;
    };

    namespace impl {       
        static GL::atomic_vector< impl::cached_type > builtin_cpp_types; // atomic_vector because ticket system will prefer small values.         
        static GL::atomic_vector< impl::cached_type > scripted_types; // atomic_vector because ticket system will prefer small values.         
        static GL::ticket_dispensor scripted_types_ticket_dispensor; // ticket system helps ensure values remain small. 

        GL::type_hash_t checkout_scripted_type(GL::string type_name) {
            GL::type_hash_t ticket = (GL::type_hash_t)(scripted_types_ticket_dispensor.get_ticket() - 1);
            scripted_types.grow_to_at_least((size_t)(ticket + 1)); // desired size

            auto& out = scripted_types[ticket];
            if (out.base_hash == 0) {
                if (InterlockedCompareExchangeNoFence(reinterpret_cast<volatile GL::type_hash_t*>(&out.base_hash), ticket, 0) == 0) {
                    out.name = type_name;
                    out.T_size = std::numeric_limits<GL::type_hash_t>::max();
                    out.base_classes = {};
                    out.base_classes_ordered = {};
                }
            }
            return ticket;
        };
        void return_scripted_type(GL::type_hash_t ticket) {
            auto& out = scripted_types[ticket];            
            out.name = "";
            out.T_size = std::numeric_limits<GL::type_hash_t>::max();            
            out.base_classes = {};
            out.base_classes_ordered = {};
            out.base_hash = 0;
            scripted_types_ticket_dispensor.return_ticket(ticket + 1);
        };
        cached_type& get_scripted_type(GL::type_hash_t hash) {
            return scripted_types[hash];
        };
        cached_type& get_impl(GL::type_hash_t hash) {
            if (hash >= (1 << 20)) {
                return builtin_cpp_types.get_or_make((hash - (1 << 20)));
            }
            else {
                return builtin_cpp_types.get_or_make(hash);
            }
        };
#undef use_btree_for_cpp_types
        bool cached_type::is_derived_from(GL::type_hash_t base) const {
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
        bool cached_type::is_base_of(GL::type_hash_t derived) const {
            if (this->is_cpp_type()) {
                auto& base = get_impl(derived);
                return base.is_derived_from(this->base_hash);
            }
            else {
                auto& base = get_scripted_type(derived);
                return base.is_derived_from(this->base_hash);
            }  
        };
        bool cached_type::add_base(GL::type_hash_t base) {
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
        bool cached_type::match_base_hash(GL::type_hash_t to_match) const {
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
        //if (from.get_hash() == 0) {
        //    return impl::get_impl(GL::type_of<void>().get_base_hash());
        //}
        if (from.is_cpp_type()) {
            return impl::get_impl(from.get_base_hash());
        }
        else {
            return impl::get_scripted_type(from.get_base_hash());
        }
    };

    GL::string type::name() const {
        if (this->is_template()) {
            return GL::printf("{%i}", GL::is_template::index(*this));
        }
        else {
            GL::string out = get_base(*this).name;
            if (is_const()) out = "const " + out;
            if (is_ref()) out = out + "&";
            if (is_temp()) out = out + "&&";
            return out;
        }
        //auto& Base = get_base(*this);
        //if (is_temp()) return Base.name + "&&";
        //else if (is_const() && is_ref()) return "const " + Base.name + "&";
        //else if (is_const() && !is_ref()) return "const " + Base.name;
        //else if (!is_const() && is_ref()) return Base.name + "&";
        //else return Base.name;
    };
    bool type::try_update_name(GL::string const& new_name) const {
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
            GL::atomic_vector< std::vector<type> > collection;
            std::deque< std::pair<int, GL::type> > types_to_try;
            std::set<GL::type> attempted_types;
            types_to_try.push_back({ 0, *this - GL::type::Reference - GL::type::Const - GL::type::Temporary });
            while (types_to_try.size() > 0) {
                std::pair<int, GL::type> this_t = types_to_try.front();
                types_to_try.pop_front();
                if (auto [iter, success] = attempted_types.insert(this_t.second); success) {
                    collection.get_or_make(this_t.first).push_back(this_t.second);
                    if (this_t.second.is_cpp_type()) {
                        for (auto& x : get_base(this_t.second).base_classes_ordered) {
                            types_to_try.push_back({ this_t.first + 1, type(impl::get_impl(x).base_hash) + GL::type::CppType });
                        }
                    }
                    else {
                        for (auto& x : get_base(this_t.second).base_classes_ordered) {
                            types_to_try.push_back({ this_t.first + 1, type(impl::get_scripted_type(x).base_hash) });
                        }
                    }                    
                }
            }
            std::vector<GL::type> out;
            out.reserve(attempted_types.size() + 16);
            for (auto& x : collection) {
                out.insert(out.end(), x.begin(), x.end());
            }
            return out;

            //std::map<int, std::vector<type>> collection;
            //std::queue< std::pair<int, GL::type> > types_to_try;
            //std::set<GL::type> attempted_types;
            //types_to_try.push({ 0, *this - GL::type::Reference - GL::type::Const - GL::type::Temporary });
            //while (types_to_try.size() > 0) {
            //    std::pair<int, GL::type> this_t = types_to_try.front();
            //    types_to_try.pop();
            //    if (attempted_types.find(this_t.second) == attempted_types.end()) {
            //        attempted_types.insert(this_t.second);
            //        collection[this_t.first].push_back(this_t.second);
            //        for (GL::type const& base_type : this_t.second.all_base_types()) {
            //            types_to_try.push({ this_t.first + 1, base_type });
            //        }
            //    }
            //}
            //std::vector<GL::type> out;
            //out.reserve(attempted_types.size() + 16);
            //for (auto& x : collection) {
            //    out.insert(out.end(), x.second.begin(), x.second.end());
            //}
            //return out;
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
    bool type::add_base(type const& base) const {
        if (this->is_cpp_type() == base.is_cpp_type()) {
            return get_base(*this).add_base(base.get_base_hash());
        }
        else {
            return false;
        }
    };

    bool type::match_base_hash(type const& to_match) const {
        if (this->get_base_hash() == to_match.get_base_hash())
            return true;
        else 
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

        return true;
    }();
};