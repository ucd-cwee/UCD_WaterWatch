#include "types.h"
#include "atomic_maps.h"
#include "ticket_dispensor.h"
#include "atomic_vector.h"

namespace GL {
	static concurrency::concurrent_unordered_map<size_t, builtin_type_info::cpp_builtin_type_info> cpp_builtin_type_infos;
	
    namespace builtin_type_info {
        cpp_builtin_type_info& get_builtin_type_info(size_t hash) {
            return cpp_builtin_type_infos[hash];
        };
        bool cpp_builtin_type_info::is_derived_from(size_t base) const {
            if (this->base_hash == base) {
                return true;
            }
            else {
                if (this->base_classes.find(base) != this->base_classes.end()) return true;
                for (auto& x : this->base_classes) {
                    auto& Base = get_builtin_type_info(x);
                    if (Base.is_derived_from(base)) {
                        return true;
                    }
                }
                return false;
            }
        };
        bool cpp_builtin_type_info::is_base_of(size_t derived) const {
            auto& base = get_builtin_type_info(derived);
            return base.is_derived_from(this->base_hash);
        };
        bool cpp_builtin_type_info::add_base(size_t base) {
            auto& Base = get_builtin_type_info(base);
            if (this->is_derived_from(base) || Base.is_derived_from(this->base_hash))
                return false;
            else {
                this->base_classes.insert(base);
                return true;
            }
        };
        bool cpp_builtin_type_info::match_base_hash(size_t to_match) const {
            if (base_hash == to_match) return true;
            if (this->base_classes.find(to_match) != this->base_classes.end()) return true;
            for (auto& x : base_classes) {
                auto& base = get_builtin_type_info(x);
                if (base.match_base_hash(to_match)) return true;
            }
            return false;
        };
    };

    GL::string type_info::name() const {
        auto& Base = builtin_type_info::get_builtin_type_info(get_base_hash());
        if (is_temp()) return Base.name + "&&";
        else if (is_const() && is_ref()) return "const " + Base.name + "&";
        else if (is_const() && !is_ref()) return "const " + Base.name;
        else if (!is_const() && is_ref()) return Base.name + "&";
        else return Base.name;
    };




};
