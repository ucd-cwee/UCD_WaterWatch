#pragma once
#include "types.h"
#include "Parallel.h"
#include "units.h"
#include "../GpuProgramming/matrix.h"
#include "constexpr_math.h"

// GL::Proxy_Functions, which wrap other functions into a shareable interface
namespace GL {
    // function name, function qualifiers (e.g. static), return type, argument types, and (optionally) argument default values. 
    class function_signature {
    private:
        size_t 
            hash_m;

    public:
        enum function_state {
            Normal = 0, // default -- meaningless. 
            Static = 1, // whether the function is a static function or not. Non-static implies it is a member-function. 
            Constant = 2, // whether this function commits to making no changes to the underlying object. Often, const should also be async, but not always. 
            Async = 4, // whether this function can be safely called asynchronously or not
            Template = 8, // whether the function is a template 
            Explicit = 16, // whether the function is explicit and the input params must exactly match (does not allow conversion)
            Cached = 32, // whether the function is a cache from another function, for performance reasons. 
            NoCost = 64,
            Constructor = 128,
            MemberObject = 256,
            Volatile = 512,
            Object = 1024
            // TemplateConstructor = 512 // indicates that the name and return type for the function should be updated once template-initialization is performed. 
        };

    public:
        size_t
            state_m;
        std::vector<GL::any::fast_any>
            argument_defaults_m;
        std::vector<GL::string>
            argument_names_m;
        std::vector<GL::type>
            argument_types_m;
        GL::string
            name_m;
        GL::type
            returns_m;
        short
            numConversions;

    public:
        static size_t eval_hash(std::vector<GL::type> const& types) {
            size_t new_hash = 0;
            for (auto& x : types)
                GL::util::hash(new_hash, x.get_hash());
            return new_hash;
        };

    private:
        void eval_hash() {
            InterlockedExchange(reinterpret_cast<volatile size_t*>(&hash_m), eval_hash(argument_types_m));
        };

    public:
        void reevaluate_hash() {
            eval_hash();
        };
        size_t get_base_hash() const {
            if (hash_m == 0) const_cast<function_signature*>(this)->eval_hash();            
            return hash_m;
        };
        size_t get_hash() const {
            return get_base_hash() | state_m;
        };
        friend bool operator==(function_signature const& lhs, function_signature const& rhs) {
            return lhs.get_hash() == rhs.get_hash();
        };
        friend bool operator!=(function_signature const& lhs, function_signature const& rhs) {
            return !operator==(lhs, rhs);
        };
        friend bool operator>(function_signature const& lhs, function_signature const& rhs) {
            return lhs.get_hash() > rhs.get_hash();
        };
        friend bool operator>=(function_signature const& lhs, function_signature const& rhs) {
            return lhs.get_hash() >= rhs.get_hash();
        };
        friend bool operator<(function_signature const& lhs, function_signature const& rhs) {
            return !operator>=(lhs, rhs);
        };
        friend bool operator<=(function_signature const& lhs, function_signature const& rhs) {
            return !operator>(lhs, rhs);
        };

    public:
        function_signature() = default;
        // The provided defaults are scooted to the end of the argument list, such as:
           //  argA, argB, argC = default1, argD = default2;        
        function_signature(GL::string const& name, GL::type returns, std::vector<std::pair<GL::string, GL::type>> const& args, std::vector<GL::any> const& defaults)
            : name_m{ name }
            , returns_m{ returns }
            , state_m(function_state::Normal)
            , hash_m{ 0 }
            , numConversions{ 0 }
        {
            argument_defaults_m.reserve(defaults.size() + 1);
            for (auto& x : defaults)
                argument_defaults_m.push_back(x.fast());

            argument_names_m.reserve(args.size() + 1);
            argument_types_m.reserve(args.size() + 1);
            for (auto& arg : args) {
                argument_names_m.push_back(arg.first);
                argument_types_m.push_back(arg.second);
            }

            bool found_non_void = false;
            for (size_t index = 0; index < argument_defaults_m.size(); index++) {
                if (!argument_defaults_m[index].empty()) {
                    found_non_void = true;
                }
                else if (found_non_void) {
                    argument_defaults_m.resize(index);
                }
            }
            while (argument_defaults_m.size() < argument_types_m.size()) argument_defaults_m.insert(argument_defaults_m.begin(), GL::any::fast_any{});
            // all non-void defaults are guarranteed to be at the end. 

            evaluate_if_template_function();
        };
        function_signature(function_signature const&) = default;
        function_signature(function_signature&&) = default;
        function_signature& operator=(function_signature const&) = default;
        function_signature& operator=(function_signature&&) = default;
        ~function_signature() = default;

        // reviews the arguments to see if any are defined as "any". If so, sets the state of the function as "template". Otherwise, unsets the template state. 
        void evaluate_if_template_function() {
            // state_m &= ~function_state::Constructor; // unsets the constructor flag
            if ((state_m & Static) > 0) {
                if (argument_types_m.size() <= 1) {
                    if ((returns_m.name() == name_m) || (returns_m.name() == (name_m + "&&"))) {
                        this->state_m |= Constructor;
                        this->returns_m |= GL::type::Temporary;
                    }
                }
            }

            for (auto& t : argument_types_m) {
                if (t.is_any()) {
                    state_m |= function_state::Template;
                    return;
                }
            }     
            state_m &= ~function_state::Template; // unsets the template flag
        }
        template<typename iter_type> int degrees_of_error_with_free_cast(iter_type iter, iter_type const& end) const {
            int out = 0;
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            return out;
        };
        template<typename iter_type> int degrees_of_error_with_cast(iter_type iter, iter_type const& end) const {
            int out = 0;
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return std::numeric_limits<int>::max();
                    }
                    else {
                        if (iter->m_casted_type.is_const() != argument_types_m[i].is_const()) out += 1;
                        if (iter->m_casted_type.is_ref() != argument_types_m[i].is_ref()) out += 1;
                        if (iter->m_casted_type.is_temp() != argument_types_m[i].is_temp()) out += 1;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return std::numeric_limits<int>::max();
                    }
                }
                if (iter != end) return std::numeric_limits<int>::max();
            }
            return out;
        };
        template<typename iter_type> bool can_call_with_free_cast(iter_type iter, iter_type const& end) const {

            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_free_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
        };
        template<typename iter_type> bool can_call_with_cast(iter_type iter, iter_type const& end) const {
            if constexpr (std::is_same_v<iter_type, GL::type*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any::fast_any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type, GL::any*>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::type>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any::fast_any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
            else if constexpr (std::is_same_v<iter_type::value_type, GL::any>) {
                size_t i = 0;
                for (; (iter != end) && (i < argument_types_m.size()); ++i, ++iter) {
                    if (!iter->can_cast(argument_types_m[i])) {
                        return false;
                    }
                }
                for (; i < argument_defaults_m.size(); ++i) {
                    if (argument_defaults_m[i].m_casted_type.is_void()) {
                        return false;
                    }
                }
                return iter == end;
            }
        };
        bool can_call_with_free_cast(std::vector<GL::type> const& from) const {
            return can_call_with_free_cast(from.begin(), from.end());
        };
        bool can_call_with_cast(std::vector<GL::type> const& from) const {
            return can_call_with_cast(from.begin(), from.end());
        };
        GL::string display() const {
            GL::string out;
            if ((this->state_m & Template) > 0) {
                out = out.add_to_delim("[[template]]", " ");
            }
            if ((this->state_m & Constructor) > 0) {
                out = out.add_to_delim("[[constructor]]", " ");
            }
            if ((this->state_m & MemberObject) > 0) {
                out = out.add_to_delim("[[member]]", " ");
            }
            if ((this->state_m & Cached) > 0) {
                out = out.add_to_delim("[[cached]]", " ");
            }
            if ((this->state_m & NoCost) > 0) {
                out = out.add_to_delim("[[nocost]]", " ");
            }
            if ((this->state_m & Static) > 0) {
                out = out.add_to_delim("static", " ");
            }

            if ((this->state_m & Constructor) > 0) {
                out = out.add_to_delim(returns_m.name(), " ");
                out = out.add_to_delim(name_m, " ");
            }
            else {
                out = out.add_to_delim(returns_m.name(), " ");
                out = out.add_to_delim(name_m, " ");
            }
            {
                GL::string args; 
                for (int i = 0; i < argument_types_m.size(); ++i) {
                    GL::string arg = argument_types_m[i].name();
                    arg = arg.add_to_delim(argument_names_m[i], " ");
                    if (!argument_defaults_m[i].empty()) {
                        arg = arg + " = {}";
                    }                    
                    args = args.add_to_delim(arg, ", ");
                }
                out = out + "(" + args + ")";
            }

            if ((this->state_m & Constant) > 0) {
                out = out.add_to_delim("const", " ");
            }
            if ((this->state_m & Async) > 0) {
                out = out.add_to_delim("async", " ");
            }
            if ((this->state_m & Explicit) > 0) {
                out = out.add_to_delim("explicit", " ");
            }

            out = out.add_to_delim("{ ... }", " ");

            return out;
        };

    };

    namespace details {
        /* Pure virtual base class for all Proxy_Function implementations
        Proxy_Functions are a type erasure of type-safe C++ function calls.
        At runtime parameter types are expected to be tested against passed in types.
        Dispatch_Engine only knows how to work with Proxy_Function, no other
        function classes */
        class Proxy_Function_Base {
        public:
            Proxy_Function_Base() = default;
            Proxy_Function_Base(Proxy_Function_Base const&) = default;
            Proxy_Function_Base(Proxy_Function_Base &&) = default;
            Proxy_Function_Base& operator=(Proxy_Function_Base const&) = default;
            Proxy_Function_Base& operator=(Proxy_Function_Base&&) = default;
            virtual ~Proxy_Function_Base() = default;

            function_signature 
                m_signature;

            // fast path
            GL::any::fast_any operator()(GL::any::fast_any** inputs_rhs, size_t num_inputs) const {
                static thread_local std::array<any::fast_any*, 16> inputs;
                // std::memset(&inputs[0], 0, sizeof(inputs));
                short pos{ 0 };
                for (; (pos < 16) && (pos < num_inputs); ++pos) {
                    inputs[pos] = inputs_rhs[pos];
                }
                for (; (pos < 16) && (pos < m_signature.argument_defaults_m.size()); ++pos) {
                    inputs[pos] = const_cast<any::fast_any*>(&m_signature.argument_defaults_m[pos]);
                }

                try {
                    return do_call(&inputs[0]);
                }
                catch (std::runtime_error const& e) {
                    auto err = GL::string("Error with function call: ") + this->m_signature.display() + "\n\t" + std::string(e.what());
                    throw std::runtime_error(err.to_string());
                }
                catch (std::exception const& e) {
                    auto err = GL::string("Error with function call: ") + this->m_signature.display() + "\n\t" + std::string(e.what());
                    throw std::runtime_error(err.to_string());
                }
                /*catch (GL::any const& return_val) {
                    throw return_val;
                }
                catch (GL::any::fast_any const& return_val) {
                    throw return_val;
                }*/
                catch (...) {
                    std::rethrow_exception(std::current_exception());
                }
            }
            template <typename iter_type> GL::any::fast_any operator()(iter_type begin, iter_type const& end) const {
                if constexpr (!std::is_same_v< iter_type, GL::any::fast_any*>) {
                    static_assert(std::is_same_v<iter_type::value_type, GL::any::fast_any>, "iterator must be for a GL::any::fast_any class");
                }

                static thread_local std::array<any::fast_any*, 16> inputs;
                std::memset(&inputs[0], 0, sizeof(inputs));
                short pos{ 0 };
                for (; (begin != end) && (pos < 16); ++begin, ++pos) {
                    inputs[pos] = const_cast<any::fast_any*>(&*begin);
                }
                for (; (pos < 16) && (pos < m_signature.argument_defaults_m.size()); ++pos) {
                    inputs[pos] = const_cast<any::fast_any*>(&m_signature.argument_defaults_m[pos]);
                }

                try {
                    return do_call(&inputs[0]);
                }
                catch (std::runtime_error const& e) {
                    auto err = GL::string("Error with function call: ") + this->m_signature.display() + "\n\t" + std::string(e.what());
                    throw std::runtime_error(err.to_string());
                }
                catch (std::exception const& e) {
                    auto err = GL::string("Error with function call: ") + this->m_signature.display() + "\n\t" + std::string(e.what());
                    throw std::runtime_error(err.to_string());
                }
                /*catch (GL::any const& return_val) {
                    throw return_val;
                }
                catch (GL::any::fast_any const& return_val) {
                    throw return_val;
                }*/
                catch (...) {
                    std::rethrow_exception(std::current_exception());
                }
            };
            // fastest path
            GL::any::fast_any operator()() const {
                if (m_signature.argument_types_m.size() > 0) {
                    static thread_local std::array<any::fast_any*, 16> inputs;
                    std::memset(&inputs[0], 0, sizeof(inputs));
                    short pos{ 0 };
                    for (; (pos < 16) && (pos < m_signature.argument_defaults_m.size()); ++pos) {
                        inputs[pos] = const_cast<any::fast_any*>(&m_signature.argument_defaults_m[pos]);
                    }
                    return do_call(&inputs[0]);
                }
                else {
                    return do_call(nullptr);
                }
            };
            // fast path
            GL::any::fast_any operator()(std::vector<any::fast_any>& params) const {
                return operator()(params.begin(), params.end());
            };
            // convenience path, requires casting to any::fast_any
            GL::any::fast_any operator()(const std::vector<any>& params) const {
                std::vector<any::fast_any> Params;
                Params.resize(params.size());
                std::transform(params.begin(), params.end(), Params.begin(), [](any const& from) { return from.fast(); });
                return operator()(Params.begin(), Params.end());
            };
            // convenience path, requires casting to any::fast_any
            GL::any::fast_any operator()(any& param) const {
                any::fast_any p = param.fast();
                return operator()(&p, &p + 1);
            };
            // fast path
            GL::any::fast_any operator()(any::fast_any& param) const {
                return operator()(&param, &param + 1);
            };
            // fast path
            GL::any::fast_any operator()(any::fast_any&& param) const {
                return operator()(&param, &param + 1);
            };

            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const = 0;

        protected:
            virtual GL::any::fast_any do_call(any::fast_any** begin) const {
                return {};
            };
            Proxy_Function_Base(function_signature&& p_signature) : m_signature(std::move(p_signature)) {}

        };
    };

    typedef GL::shared_ptr<details::Proxy_Function_Base> Proxy_Function;

    namespace details {        
        __forceinline auto cast_any(any::fast_any* const& p) { return p->cast(); }
        template <typename F, std::size_t... Is> auto unpack_and_call_helper_returns(F const& func, any::fast_any** arr_ptr, std::index_sequence<Is...>) {
            return func(cast_any(arr_ptr[Is])...);
        };
        template <typename F, std::size_t... Is> auto unpack_and_call_helper(F const& func, any::fast_any** arr_ptr, std::index_sequence<Is...>) {
            func(cast_any(arr_ptr[Is])...);
        };
        template <typename F, bool returns, std::size_t N> auto unpack_and_call(F const& func, any::fast_any** arr_ptr) {
            if constexpr (returns) {
                return unpack_and_call_helper_returns(func, arr_ptr, std::make_index_sequence<N>{});
            }
            else {
                unpack_and_call_helper(func, arr_ptr, std::make_index_sequence<N>{});
            }
        }

        /* Use to call function objects */
        template <class Callable>
        class Explicit_Function_Impl final : public Proxy_Function_Base {
        public:
            using argType = typename parallel::impl::function_traits<decltype(std::function(std::declval<Callable>()))>::arguments;
            using returnType = typename parallel::impl::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>> 
                    args;
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature(GL::string::empty_string(), GL::type_of<returnType>(), args, std::move(defaults));
            };
        public:
            Explicit_Function_Impl(Explicit_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , F_m(from.F_m)
            {};

        public:
            Explicit_Function_Impl(Callable F_p, std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , F_m(std::move(F_p))
            {};
            virtual ~Explicit_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Explicit_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        protected:
            virtual GL::any::fast_any do_call(any::fast_any** begin) const override {
                if constexpr (std::is_same_v<returnType, void>) {
                    unpack_and_call<Callable, false, numArgs>(F_m, begin);
                    return {};
                }
                else {
                    return any::fast_any::instance(unpack_and_call<Callable, true, numArgs>(F_m, begin));
                }
            };
            Callable F_m;
        };

        /* Use to call function objects */
        template <typename T, class Class>
        class Attribute_Access_Impl final : public Proxy_Function_Base {
        public:
            using actualT = typename std::decay_t<typename GL::type_erasure::get_type<T>::type>;

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
                args.push_back({ "parent", GL::type_of<const Class&>() });
                return function_signature(GL::string::empty_string(), GL::type_of<actualT&>(), args, std::move(defaults));
            };

        public:
            Attribute_Access_Impl(Attribute_Access_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , m_attr(from.m_attr)
            {};

        public:
            Attribute_Access_Impl(T Class::* t_attr, std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , m_attr(t_attr)
            {};
            virtual ~Attribute_Access_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Attribute_Access_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        protected:
            __declspec(noinline) virtual GL::any::fast_any do_call(any::fast_any** begin) const override {
                if constexpr (std::is_same_v<T, void>) {
                    return {};
                }
                else if constexpr (std::is_same_v<any, T>) {
                    if (begin[0]->m_casted_type.is_const()) {
                        return any::fast_any::instance(begin[0]->cast<Class*>()->*m_attr) + (GL::type::Const | GL::type::Reference);
                    }
                    else {
                        return any::fast_any::instance(begin[0]->cast<Class*>()->*m_attr) + GL::type::Reference;
                    }                    
                }
                else if constexpr (std::is_pointer<T>::value) {
                    GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();
                    auto out = GL::shared_ptr<actualT>(GL::shared_ptr<void>(ptr));
                    out.set_pointer_without_modifying_control_block((*ptr).*m_attr);
                    if (begin[0]->m_casted_type.is_const()) {
                        return any::fast_any::instance(out) + (GL::type::Const | GL::type::Reference);
                    }
                    else {
                        return any::fast_any::instance(out) + GL::type::Reference;
                    }
                }
                else {
                    GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();         
                    auto out = GL::shared_ptr<actualT>(GL::shared_ptr<void>(ptr));
                    out.set_pointer_without_modifying_control_block(&(ptr.get()->*m_attr));
                    if (begin[0]->m_casted_type.is_const()) {
                        return any::fast_any::instance(out) + (GL::type::Const | GL::type::Reference);
                    }
                    else {
                        return any::fast_any::instance(out) + GL::type::Reference;
                    }
                }
            };
            T Class::* m_attr;
        };

        /**
         * Use to call member functions:
         * struct Test{ public: std::string attr(){ return "TEST"; }; }
         * var& func = details::Attribute_Access_Impl(&Test::attr);
         * assert(func(Test{}).cast<std::string>() == "TEST");
        */
        template <typename R, typename... T>
        class Static_Function_Impl final : public Proxy_Function_Base {
        public:
            using argType = std::tuple<T...>;
            using returnType = typename std::decay_t<typename GL::type_erasure::get_type<R>::type>;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature(GL::string::empty_string(), GL::type_of<returnType>(), args, std::move(defaults));
            };
        public:
            Static_Function_Impl(Static_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , F_m(from.F_m)
            {};

        public:
            Static_Function_Impl(R(*f)(T...), std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , F_m(std::move(f)) {};
            virtual ~Static_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Static_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        protected:
            virtual GL::any::fast_any do_call(any::fast_any** begin) const override {
                if constexpr (std::is_same_v<returnType, void>) {
                    unpack_and_call<R(*)(T...), false, numArgs>(F_m, begin);
                    return {};
                }
                else {
                    return any::fast_any::instance(unpack_and_call<R(*)(T...), true, numArgs>(F_m, begin));
                }
            };
            R(*F_m)(T...);
        };

        template <typename R, typename Class, typename... T>
        class Default_Member_Function_Impl : public Proxy_Function_Base {
        public:
            using argType = std::tuple<T...>;
            using returnType = typename std::decay_t<typename GL::type_erasure::get_type<R>::type>;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
                args.push_back({ "parent", GL::type_of<Class&>() });
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature(GL::string::empty_string(), GL::type_of<returnType>(), args, std::move(defaults));
            };
        public:
            Default_Member_Function_Impl(Default_Member_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , m_attr(from.m_attr)
            {};

        public:
            Default_Member_Function_Impl(R(Class::* f)(T...), std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , m_attr(std::move(f)) {};
            virtual ~Default_Member_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Default_Member_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        private:
            decltype(auto) do_call_impl(any::fast_any** begin) const {
                if (Class* parent = begin[0]->cast<Class*>()) {
                    if constexpr (numArgs == 15) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                            );
                    }
                    else if constexpr (numArgs == 14) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                            );
                    }
                    else if constexpr (numArgs == 13) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                            );
                    }
                    else if constexpr (numArgs == 12) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                            );
                    }
                    else if constexpr (numArgs == 11) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                            );
                    }
                    else if constexpr (numArgs == 10) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                            );
                    }
                    else if constexpr (numArgs == 9) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                            );
                    }
                    else if constexpr (numArgs == 8) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                            );
                    }
                    else if constexpr (numArgs == 7) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                            );
                    }
                    else if constexpr (numArgs == 6) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                            );
                    }
                    else if constexpr (numArgs == 5) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                            );
                    }
                    else if constexpr (numArgs == 4) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                            );
                    }
                    else if constexpr (numArgs == 3) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                            );
                    }
                    else if constexpr (numArgs == 2) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                            );
                    }
                    else if constexpr (numArgs == 1) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast()
                            );
                    }
                    else if constexpr (numArgs == 0) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)();
                        else return (parent->*m_attr)();
                    }
                }
                throw std::runtime_error("Cannot call member function on a null object.");
            };

            virtual GL::any::fast_any do_call(any::fast_any** begin) const override {
                if (Class* parent = begin[0]->cast<Class*>()) {
                    if constexpr (std::is_same_v<returnType, void>)
                        do_call_impl(begin);
                    else {
                        if constexpr (std::is_same_v<any, returnType>) {
                            return any::fast_any::instance(do_call_impl(begin));
                        }
                        else if constexpr (std::is_reference_v<R>) {
                            decltype(auto) ref = do_call_impl(begin);

                            GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();
                            auto out = GL::shared_ptr<returnType>(GL::shared_ptr<void>(ptr));
                            out.set_pointer_without_modifying_control_block(const_cast<returnType*>(&ref));
                            if (begin[0]->m_casted_type.is_const()) {
                                return any::fast_any::instance(out) + (GL::type::Const | GL::type::Reference);
                            }
                            else {
                                return any::fast_any::instance(out) + GL::type::Reference;
                            }
                        }
                        else if constexpr (std::is_pointer_v<R>) {
                            decltype(auto) ref = do_call_impl(begin);

                            GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();
                            auto out = GL::shared_ptr<returnType>(GL::shared_ptr<void>(ptr));
                            out.set_pointer_without_modifying_control_block(const_cast<returnType*>(ref));
                            if (begin[0]->m_casted_type.is_const()) {
                                return any::fast_any::instance(out) + (GL::type::Const | GL::type::Reference);
                            }
                            else {
                                return any::fast_any::instance(out) + GL::type::Reference;
                            }
                        }
                        else {
                            return any::fast_any::instance(do_call_impl(begin));
                        }
                    }
                }
                return {};
            };

            R(Class::* m_attr)(T...);
        };

        template <typename R, typename Class, typename... T>
        class Const_Member_Function_Impl : public Proxy_Function_Base {
        public:
            using argType = std::tuple<T...>;
            using returnType = typename std::decay_t<typename GL::type_erasure::get_type<R>::type>;
            static constexpr auto numArgs{ std::tuple_size_v< argType > };

        protected:
            static function_signature CreateSignature(std::vector<any>&& defaults = {}) {
                std::vector<std::pair<GL::string, GL::type>>
                    args;
                args.push_back({ "parent", GL::type_of<const Class&>() });
#define argT(NN) if constexpr (numArgs > NN) { args.push_back({ GL::printf("param%i", NN), GL::type_of<typename std::tuple_element_t<NN, argType>>() }); }
                argT(0);
                argT(1);
                argT(2);
                argT(3);
                argT(4);
                argT(5);
                argT(6);
                argT(7);
                argT(8);
                argT(9);
                argT(10);
                argT(11);
                argT(12);
                argT(13);
                argT(14);
                argT(15);
#undef argT
                return function_signature(GL::string::empty_string(), GL::type_of<returnType>(), args, std::move(defaults));
            };
        public:
            Const_Member_Function_Impl(Const_Member_Function_Impl const& from)
                : Proxy_Function_Base((function_signature)from.m_signature)
                , m_attr(from.m_attr)
            {};

        public:
            Const_Member_Function_Impl(R(Class::* f)(T...) const, std::vector<any>&& defaults = {})
                : Proxy_Function_Base(CreateSignature(std::move(defaults)))
                , m_attr(std::move(f)) {};
            virtual ~Const_Member_Function_Impl() = default;
            virtual GL::shared_ptr<details::Proxy_Function_Base> duplicate() const override {
                auto* function_impl = new Const_Member_Function_Impl(*this);
                return GL::static_pointer_cast<details::Proxy_Function_Base>(GL::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl));
            };

        private:
            decltype(auto) do_call_impl(any::fast_any** begin) const {
                if (const Class* parent = begin[0]->cast<const Class*>()) {
                    if constexpr (numArgs == 15) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast(), begin[15]->cast()
                            );
                    }
                    else if constexpr (numArgs == 14) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast(), begin[14]->cast()
                            );
                    }
                    else if constexpr (numArgs == 13) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast(), begin[13]->cast()
                            );
                    }
                    else if constexpr (numArgs == 12) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast(), begin[12]->cast()
                            );
                    }
                    else if constexpr (numArgs == 11) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast(), begin[11]->cast()
                            );
                    }
                    else if constexpr (numArgs == 10) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast(), begin[10]->cast()
                            );
                    }
                    else if constexpr (numArgs == 9) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast(),
                            begin[9]->cast()
                            );
                    }
                    else if constexpr (numArgs == 8) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast(), begin[8]->cast()
                            );
                    }
                    else if constexpr (numArgs == 7) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast(), begin[7]->cast()
                            );
                    }
                    else if constexpr (numArgs == 6) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast(), begin[6]->cast()
                            );
                    }
                    else if constexpr (numArgs == 5) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast(), begin[5]->cast()
                            );
                    }
                    else if constexpr (numArgs == 4) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast(), begin[4]->cast()
                            );
                    }
                    else if constexpr (numArgs == 3) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast(), begin[3]->cast()
                            );
                    }
                    else if constexpr (numArgs == 2) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast(), begin[2]->cast()
                            );
                    }
                    else if constexpr (numArgs == 1) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)(
                            begin[1]->cast()
                            );
                        else return (parent->*m_attr)(
                            begin[1]->cast()
                            );
                    }
                    else if constexpr (numArgs == 0) {
                        if constexpr (std::is_same_v<returnType, void>) (parent->*m_attr)();
                        else return (parent->*m_attr)();
                    }
                }
                throw std::runtime_error("Cannot call member function on a null object.");
            };

            virtual GL::any::fast_any do_call(any::fast_any** begin) const override {
                if (const Class* parent = begin[0]->cast<const Class*>()) {
                    if constexpr (std::is_same_v<returnType, void>)
                        do_call_impl(begin);
                    else {
                        if constexpr (std::is_same_v<any, returnType>) {
                            return any::fast_any::instance(do_call_impl(begin));
                        }
                        else if constexpr (std::is_reference_v<R>) {
                            decltype(auto) ref = do_call_impl(begin);

                            GL::shared_ptr<const Class> ptr = begin[0]->cast< GL::shared_ptr<const Class> >();
                            auto out = GL::shared_ptr<returnType>(GL::shared_ptr<void>(ptr));
                            out.set_pointer_without_modifying_control_block(const_cast<returnType*>(&ref));
                            if (begin[0]->m_casted_type.is_const()) {
                                return any::fast_any::instance(out) + (GL::type::Const | GL::type::Reference);
                            }
                            else {
                                return any::fast_any::instance(out) + GL::type::Reference;
                            }
                        }
                        else if constexpr (std::is_pointer_v<R>) {
                            decltype(auto) ref = do_call_impl(begin);

                            GL::shared_ptr<Class> ptr = begin[0]->cast< GL::shared_ptr<Class> >();
                            auto out = GL::shared_ptr<returnType>(GL::shared_ptr<void>(ptr));
                            out.set_pointer_without_modifying_control_block(const_cast<returnType*>(ref));
                            if (begin[0]->m_casted_type.is_const()) {
                                return any::fast_any::instance(out) + (GL::type::Const | GL::type::Reference);
                            }
                            else {
                                return any::fast_any::instance(out) + GL::type::Reference;
                            }
                        }
                        else {
                            return any::fast_any::instance(do_call_impl(begin));
                        }
                    }
                }
                return {};
            };

            R(Class::* m_attr)(T...) const;
        };

        namespace detail {
            template <typename T>
            struct is_static_member_function : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...)> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const volatile> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) volatile> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...)&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...)&&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const&&> : std::false_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) noexcept> : std::true_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const noexcept> : std::true_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) volatile noexcept> : std::true_type {};

            template <typename R, typename C, typename... Args>
            struct is_static_member_function<R(C::*)(Args...) const volatile noexcept> : std::true_type {};


            template<typename... Param> struct Function_Params {
                typedef std::tuple<Param...> argType;
                static constexpr auto numArgs = std::tuple_size_v<argType>;

                // Specialization to unpack the tuple's internal parameter pack
                template <typename Ret>
                struct to_function_pointer {
                    using type = Ret(*)(Param...);
                };
            };

            template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false/*, bool IsStatelessObject = false*/>
            struct Function_Signature {
                typedef Params Param_Types;
                typedef Class Class_Type;
                typedef Ret Return_Type;

                // constexpr static const bool is_stateless_object = IsStatelessObject; // e.g. lambda object that does not capture anything
                constexpr static const bool is_object = IsObject; // e.g. lambda object
                constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
                constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
                constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
                constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

                template<typename T> constexpr Function_Signature(T&&) noexcept { };
                constexpr Function_Signature() noexcept = default;
            };

            // Free functions
            template<typename Ret, typename... Param>
            Function_Signature(Ret(*f)(Param...))
                ->Function_Signature<Ret, void, Function_Params<Param...>, false, false, false>; // static function

            // no reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...))
                ->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const)
                ->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

            // & reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile&)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const&)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...)&)
                ->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const&)
                ->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

            // && reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile&&)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...)&&)
                ->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const&&)
                ->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true, false, false>; // member function

            // Free functions
            template<typename Ret, typename... Param>
            Function_Signature(Ret(*f)(Param...) noexcept)
                ->Function_Signature<Ret, void, Function_Params<Param...>, false, false, false>; // static function

            // no reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile noexcept)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const noexcept)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) noexcept)
                ->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const noexcept)
                ->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

            // & reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...)& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

            // && reference specifier
            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile&& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) volatile const&& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...)&& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class, typename... Param>
            Function_Signature(Ret(Class::* f)(Param...) const&& noexcept)
                ->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true, false, false>; // member function

            template<typename Ret, typename Class>
            Function_Signature(Ret Class::* f)
                ->Function_Signature<Ret, Class, Function_Params<Class&>, true, true, false>; // member object

#if 1
            namespace impl {

                template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false/*, bool IsStatelessObject = false*/>
                struct Function_Signature_m1 {
                    typedef Params Param_Types;
                    typedef Class Class_Type;
                    typedef Ret Return_Type;

                    // constexpr static const bool is_stateless_object = IsStatelessObject; // e.g. lambda object that does not capture anything
                    constexpr static const bool is_object = IsObject; // e.g. lambda object
                    constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
                    constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
                    constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
                    constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

                    template<typename T> constexpr Function_Signature_m1(T&&) noexcept { };
                    constexpr Function_Signature_m1() noexcept = default;
                };

                // Free functions
                template<typename Ret, typename... Param>
                Function_Signature_m1(Ret(*f)(Param...))
                    ->Function_Signature_m1<Ret, void, Function_Params<Param...>, false, false, false>; // static function

                // no reference specifier
                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile const)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...))
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) const)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                // & reference specifier
                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile const&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...)&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) const&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                // && reference specifier
                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile&&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile const&&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...)&&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) const&&)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                // Free functions
                template<typename Ret, typename... Param>
                Function_Signature_m1(Ret(*f)(Param...) noexcept)
                    ->Function_Signature_m1<Ret, void, Function_Params<Param...>, false, false, false>; // static function

                // no reference specifier
                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile const noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) const noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                // & reference specifier
                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile& noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile const& noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) & noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) const& noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                // && reference specifier
                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile&& noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) volatile const&& noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) && noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

                template<typename Ret, typename Class, typename... Param>
                Function_Signature_m1(Ret(Class::* f)(Param...) const&& noexcept)
                    ->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

            }
#endif


            // primary template handles types that have no nested ::type member:
            template<class, class = std::void_t<>>
            struct has_call_operator : std::false_type {};

            // specialization recognizes types that do have a nested ::type member:
            template<class T>
            struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

            template<typename Func>
            auto function_signature(Func const& f) {
                if constexpr (has_call_operator<Func>::value) {
                    return Function_Signature<
                        typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
                        void,
                        typename decltype(impl::Function_Signature_m1{ &std::decay_t<Func>::operator() })::Param_Types,
                        false,
                        false,
                        true 
                        // , std::is_empty_v<Func>
                    > {};
                }
                else {
                    return Function_Signature{ f };
                }
            };

            template<typename Obj, typename Param1, typename... Rest>
            Param1 get_first_param(Function_Params<Param1, Rest...>, Obj&& obj) {
                return static_cast<Param1>(std::forward<Obj>(obj));
            };

        } // namespace chaiscript::dispatch::detail

        template<typename Ret, typename Class, typename... Param>
        Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) const, std::vector<any>&& defaults) {
            auto out = GL::static_pointer_cast<details::Proxy_Function_Base>(GL::make_shared_forwarded(Const_Member_Function_Impl(f, std::move(defaults))));
            out->m_signature.state_m |= function_signature::Constant;
            // out->m_signature.state_m |= function_signature::Async; // const member functions (e.g. std::string::length) are assumed to be async-friendly. 
            return out;
        };
        
        template<typename Ret, typename Class, typename... Param>
        Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...), std::vector<any>&& defaults) {
            auto out = GL::static_pointer_cast<details::Proxy_Function_Base>(GL::make_shared_forwarded(Default_Member_Function_Impl(f, std::move(defaults))));
            return out;
        };
    };

    // Convert nearly any function or function pointer to a callable, generic proxy function. 
    template<typename Func> __declspec(noinline) Proxy_Function make_callable(
        GL::string const& name,
        Func&& func,
        std::vector<any>&& defaults = {}
    ) {
        static bool update_name{ []() -> bool {
            return GL::type_of<details::Proxy_Function_Base>().try_update_name("function");
        }() };

        Proxy_Function out;
        typedef decltype(details::detail::function_signature(func)) function_header;
        if constexpr (function_header::is_object) { // function objects, e.g. auto x = [](){};
            constexpr bool is_static_s = std::is_convertible_v<std::decay_t<decltype(func)>, typename function_header::Param_Types::template to_function_pointer<typename function_header::Return_Type>::type>;

            out = GL::static_pointer_cast<details::Proxy_Function_Base>(GL::make_shared< details::Explicit_Function_Impl<Func> >(std::move(func), std::move(defaults)));
            if constexpr (is_static_s){
                out->m_signature.state_m |= function_signature::Static;
                out->m_signature.state_m |= function_signature::Constant;
                // out->m_signature.state |= function_signature::Async; // static functions are assumed to be async-friendly. 
            }            
        }
        else if constexpr (function_header::is_member_object) { // member objects, e.g. return object.member;    
            out = GL::static_pointer_cast<details::Proxy_Function_Base>(GL::make_shared_forwarded(details::Attribute_Access_Impl(std::move(func), std::move(defaults))));
            out->m_signature.state_m |= function_signature::Constant; // accessing a member object is assumed to be constant -- it does not necessarily change anything just to "look".
            out->m_signature.state_m |= function_signature::Async; // accessing a member object is assumed to be async-friendly.            
            out->m_signature.state_m |= function_signature::MemberObject; // accessing a member object is assumed to be async-friendly.            
        }
        else if constexpr (function_header::is_member && !function_header::is_member_object) { // member functions, e.g. return object.member();
            out = details::Member_Function_Impl(std::move(func), std::move(defaults));
        }
        else if constexpr (function_header::is_static_member_function) { // static function pointers, e.g. static foo(){};        
            out = GL::static_pointer_cast<details::Proxy_Function_Base>(GL::make_shared_forwarded(details::Static_Function_Impl(std::move(func), std::move(defaults))));
            out->m_signature.state_m |= function_signature::Static;
            out->m_signature.state_m |= function_signature::Constant;
            //out->m_signature.state_m |= function_signature::Async; // static functions are assumed to be async-friendly. 
        }
        else {
            throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
        }

        out->m_signature.name_m = name;

        if ((out->m_signature.state_m & function_signature::Static) > 0) {
            if (out->m_signature.argument_types_m.size() <= 1) {
                if (
                    (out->m_signature.returns_m.name() == out->m_signature.name_m)
                    || 
                    (out->m_signature.returns_m.name() == out->m_signature.name_m + "&&")
                ) {
                    out->m_signature.returns_m |= GL::type::Temporary;
                    out->m_signature.state_m |= function_signature::Constructor;
                }
            }
        }

        return out;
    };

    // Convert nearly any function or function pointer to a callable, generic proxy function. 
    template<typename Func> __declspec(noinline) Proxy_Function make_callable(
        GL::string const& name,
        Func&& func, 
        size_t stateModifier, 
        std::vector<any>&& defaults = {}
    ) {
        auto out = make_callable(name, std::move(func), std::move(defaults));
        out->m_signature.state_m |= stateModifier;
        return out;
    };

    template<typename Func> __declspec(noinline) Proxy_Function make_callable(
        GL::string const& name,
        Func&& func,
        size_t stateModifier,
        std::vector<any>&& defaults,
        std::vector<std::pair<GL::string, GL::type>> const& arguments
    ) {
        auto out = make_callable(name, std::move(func), stateModifier, std::move(defaults));

        if (arguments.size() > 0) {
            for (int i = 0; (i < arguments.size()) && (i < out->m_signature.argument_names_m.size()); ++i) {
                out->m_signature.argument_names_m[i] = arguments[i].first;
                //if (out->m_signature.argument_types_m[i].is_any()) {
                    out->m_signature.argument_types_m[i] = arguments[i].second;
                //}
            }            
        }
        out->m_signature.evaluate_if_template_function();
        return out;
    };

    template<typename Func> __declspec(noinline) Proxy_Function make_callable(
        GL::string const& name,
        Func&& func,
        size_t stateModifier,
        std::vector<any>&& defaults,
        std::vector<std::pair<GL::string, GL::type>> const& arguments,
        GL::type returnType
    ) {
        auto out = make_callable(name, std::move(func), stateModifier, std::move(defaults));

        if (arguments.size() > 0) {
            for (int i = 0; (i < arguments.size()) && (i < out->m_signature.argument_names_m.size()); ++i) {
                out->m_signature.argument_names_m[i] = arguments[i].first;
                //if (out->m_signature.argument_types_m[i].is_any()) {
                out->m_signature.argument_types_m[i] = arguments[i].second;
                //}
            }
        }
        out->m_signature.returns_m = returnType;
        out->m_signature.evaluate_if_template_function();

        return out;
    };

    template<typename Func> __declspec(noinline) Proxy_Function make_callable(
        GL::string const& name,
        Func&& func,
        std::vector<any>&& defaults,
        std::vector<std::pair<GL::string, GL::type>> const& arguments
    ) {
        auto out = make_callable(name, std::move(func), std::move(defaults));

        if (arguments.size() > 0) {
            for (int i = 0; (i < arguments.size()) && (i < out->m_signature.argument_names_m.size()); ++i) {
                out->m_signature.argument_names_m[i] = arguments[i].first;
               // if (out->m_signature.argument_types_m[i].is_any()) {
                    out->m_signature.argument_types_m[i] = arguments[i].second;
               // }
            }            
        }
        out->m_signature.evaluate_if_template_function();
        return out;
    };

    template<typename Func> __declspec(noinline) Proxy_Function make_callable(
        GL::string const& name,
        Func&& func,
        std::vector<any>&& defaults,
        std::vector<std::pair<GL::string, GL::type>> const& arguments,
        GL::type returnType
    ) {
        auto out = make_callable(name, std::move(func), std::move(defaults));

        if (arguments.size() > 0) {
            for (int i = 0; (i < arguments.size()) && (i < out->m_signature.argument_names_m.size()); ++i) {
                out->m_signature.argument_names_m[i] = arguments[i].first;
                // if (out->m_signature.argument_types_m[i].is_any()) {
                out->m_signature.argument_types_m[i] = arguments[i].second;
                // }
            }
        }
        out->m_signature.returns_m = returnType;
        out->m_signature.evaluate_if_template_function();
        return out;
    };

#define decl_func(F, ...) make_callable(GL::string(#F).right_of_last("::"), F, __VA_ARGS__)

    namespace details {
        template <class From, class To, class = void>
        struct is_explicitly_convertible_to_impl : std::false_type {};

        template <class From, class To>
        struct is_explicitly_convertible_to_impl<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>> : std::true_type {};

        template <class From, class To>
        struct is_explicitly_convertible_to : is_explicitly_convertible_to_impl<From, To> {};

        template <class From, class To>
        inline constexpr bool is_explicitly_convertible_to_v = is_explicitly_convertible_to<From, To>::value;

        template <typename T>
        static constexpr bool is_numeric_type() {
            return
                std::is_same_v<T, double> ||
                std::is_same_v<T, float> ||
                std::is_same_v<T, long> ||
                std::is_same_v<T, int> ||
                std::is_same_v<T, char> ||
                std::is_same_v<T, long double> ||
                std::is_same_v<T, long long> ||
                std::is_same_v<T, unsigned long> ||
                std::is_same_v<T, unsigned long long> ||
                std::is_same_v<T, unsigned char> ||
                std::is_same_v<T, unsigned int> ||
                std::is_same_v<T, GL::value> ||
                std::is_base_of<GL::value, T>::value;
        };
    };

#pragma warning(push)
#pragma warning(disable : 4244) // suppressing warning on casting size_t to double, etc.
    // Creates a Proxy_Function whose job is to convert from "From" types to "To" types. 
    // Supports static conversions (e.g. double to int) and polymorphic conversions (e.g. GL::foot& to GL::value&)
    template<typename From_Requested, typename To_Requested>
    __declspec(noinline) Proxy_Function make_converter() {
        using From = typename std::decay< From_Requested >::type;
        using To = typename std::decay< To_Requested >::type;

        constexpr static bool is_convertable = details::is_explicitly_convertible_to<From, To>::value;
        constexpr static bool is_bidir_convertable = details::is_explicitly_convertible_to<To, From>::value;
        constexpr static bool is_polymorphic = std::is_base_of<To, From>::value;

        if constexpr (is_polymorphic) {
            // memorize the connection, regardless of outcome
            static const bool Added{ GL::type_of<To>().add_base(GL::type_of<From>()) };
        }
        
        Proxy_Function out;
        if constexpr (is_polymorphic && ((std::is_reference_v<To_Requested> || std::is_pointer_v<To_Requested> || std::is_same_v<GL::shared_ptr<To>, To_Requested> || std::is_same_v<std::shared_ptr<To>, To_Requested>) || (!is_convertable))) {
            out = GL::make_callable(GL::type_of<To>().name(), [](GL::shared_ptr<From> from) -> GL::shared_ptr<To> {
                return GL::shared_ptr<To>(from);
            }, {}, { { "From", GL::type_of<From>() | GL::type::Reference } }, GL::type_of<To>() | GL::type::Reference);
            out->m_signature.state_m |= (GL::function_signature::Async | GL::function_signature::Static);
            out->m_signature.state_m |= GL::function_signature::NoCost;
            out->m_signature.numConversions = 0;
        }
        else {
            if constexpr (is_convertable) {
                out = GL::make_callable(GL::type_of<To>().name(), [](From const& from) -> To {
                    return static_cast<To>(from);
                }, {}, { { "From", GL::type_of< From const&>() } }, GL::type_of<To>() | GL::type::Temporary);
                out->m_signature.state_m |= (/*GL::function_signature::Async | */GL::function_signature::Static);
                out->m_signature.state_m |= GL::function_signature::NoCost;
                if constexpr ((std::is_pod<From>::value || details::is_numeric_type<From>()) && (std::is_pod<To>::value || details::is_numeric_type<To>())) {
                    constexpr short sz_diff = cx::abs((float)(short)sizeof(From) - (float)(short)sizeof(To));
                    if constexpr (sz_diff >= 6) {
                        out->m_signature.numConversions = 3;
                    }
                    else if constexpr (sz_diff >= 4) {
                        out->m_signature.numConversions = 2;
                    }
                    else {
                        out->m_signature.numConversions = 1;
                    }
                }
                else {
                    out->m_signature.numConversions = 1;
                }                
            }
            else {
                if constexpr (details::is_numeric_type<From>() && details::is_numeric_type<To>()) {
                    if constexpr (std::is_same_v<From, GL::value> || std::is_base_of<GL::value, From>::value) {
                        if constexpr (std::is_same_v<To, GL::value> || std::is_base_of<GL::value, To>::value) {
                            out = GL::make_callable(GL::type_of<To>().name(), [](From const& from) -> To {
                                return To(from);
                            }, {}, { { "From", GL::type_of< From>() | GL::type::Const | GL::type::Reference } }, GL::type_of<To>() | GL::type::Temporary);
                            out->m_signature.state_m |= (GL::function_signature::Async | GL::function_signature::Static);
                            out->m_signature.state_m |= GL::function_signature::NoCost;
                        }
                        else {
                            out = GL::make_callable(GL::type_of<To>().name(), [](From const& from) -> To {
                                return static_cast<To>((float)from);
                            }, {}, { { "From", GL::type_of< From>() | GL::type::Const | GL::type::Reference } }, GL::type_of<To>() | GL::type::Temporary);
                            out->m_signature.state_m |= (GL::function_signature::Async | GL::function_signature::Static);
                            out->m_signature.state_m |= GL::function_signature::NoCost;
                        }
                    }
                    else if constexpr (std::is_same_v<To, GL::value> || std::is_base_of<GL::value, To>::value) {
                        out = GL::make_callable(GL::type_of<To>().name(), [](From const& from) -> To {
                            return To((float)from);
                        }, {}, { { "From", GL::type_of< From>() | GL::type::Const | GL::type::Reference } }, GL::type_of<To>() | GL::type::Temporary);
                        out->m_signature.state_m |= (/*GL::function_signature::Async | */GL::function_signature::Static);
                        out->m_signature.state_m |= GL::function_signature::NoCost;
                    }
                    else {
                        out = GL::make_callable(GL::type_of<To>().name(), [](From const& from) -> To {
                            return static_cast<To>(from);
                        }, {}, { { "From", GL::type_of<From const&>() }}, GL::type_of<To>() | GL::type::Temporary);
                        out->m_signature.state_m |= (/*GL::function_signature::Async | */GL::function_signature::Static);
                        out->m_signature.state_m |= GL::function_signature::NoCost;
                    }
                    out->m_signature.numConversions = 1;
                }
                else {
                    out = GL::make_callable(GL::type_of<To>().name(), [](From const& from) -> To {
                        return To(from);
                    }, {}, { { "From", GL::type_of< From>() | GL::type::Const | GL::type::Reference } }, GL::type_of<To>() | GL::type::Temporary);
                    out->m_signature.state_m |= GL::function_signature::Static;
                    out->m_signature.state_m |= GL::function_signature::NoCost;
                    out->m_signature.numConversions = 1;
                }
            }
        }
        return out;
    };
#pragma warning(pop)
};

// std::hash< GL::function_signature > 
namespace std {
    template <> struct hash<GL::function_signature> {
        std::size_t operator()(const GL::function_signature& k) const {
            return k.get_base_hash();
        };
    };
};

#if 0
// Sorted Proxy_Functions with support for caching
namespace GL {

    // quick hash of functions that are sorted by 
    class FunctionCache {
    public:
        using cacheT = concurrency::concurrent_unordered_map<GL::string, concurrency::concurrent_unordered_map<size_t/*GL::function_signature*/, GL::Proxy_Function> >;
        GL::atomic_shared_ptr < cacheT >
            cache;

        FunctionCache()
            : cache(GL::make_shared<cacheT>())
        {};
        void invalidate() {
            cache.store(GL::make_shared<cacheT>());
        };
        void emplace(GL::function_signature const& key, Proxy_Function const& value) {
            auto f = cache.load_fast();
            f->operator[](key.name_m).insert({ key.get_base_hash(), value });
        };
        Proxy_Function find(GL::function_signature const& key) {
            auto f = cache.load_fast();
            auto& M = f->operator[](key.name_m);
            if (auto F = M.find(key.get_base_hash()); F != M.end()) {
                return F->second;
            }
            else {
                return nullptr;
            }            
        };
        Proxy_Function find(GL::string const& name, size_t types_hash) {
            auto f = cache.load_fast();
            auto& M = f->operator[](name);
            if (auto F = M.find(types_hash); F != M.end()) {
                return F->second;
            }
            else {
                return nullptr;
            }
        };
    };
    class Functions {
    public:
        using cacheT = concurrency::concurrent_unordered_map<GL::string, concurrency::concurrent_unordered_map<size_t, std::pair< GL::function_signature, GL::Proxy_Function>> >;
        FunctionCache 
            cache;        
        cacheT
            originals;

        void emplace(Proxy_Function const& value) {
            if (value) {
                originals[value->m_signature.name_m].insert({ 
                    value->m_signature.get_base_hash(), 
                    { value->m_signature, value }
                });
            }
        };
        Proxy_Function find(GL::string const& name, size_t types_hash) {
            auto& M = originals[name];
            if (auto F = M.find(types_hash); F != M.end()) {
                return F->second.second;
            }
            return nullptr;
        };
        auto& at(GL::string const& name) {
            return originals[name];
        };



    };

};
#endif

