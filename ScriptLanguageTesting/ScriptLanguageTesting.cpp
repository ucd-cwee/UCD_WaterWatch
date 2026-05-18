#pragma once
//#include <math.h>
//#include <stdio.h>
//#include <algorithm>
//#include <iterator>
//#include <fstream>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <map>
//#include <iostream>
//#include <string>
//#include <string_view>
//#include <regex>
//#include <list>
//#include <thread>
//#include <concurrent_unordered_map.h>
//#include <stdlib.h>
#include "aba_problem.h"
#include "util.h"
#include "Parallel.h"
#include "units.h"
#include "Stopwatch.h"
//#include "stopwatch.h"
//#include "strings.h"
//#include "types.h"
#include "scripting.h"

#if 0
// version of 'any' that is NOT thread-safe, but is faster as a result.
class any {
    friend class dynamic_object;
protected:
    mutable GL::shared_ptr< GL::type_erasure::any_data >
        m_ptr; // atomic shared-ptr for the type-erased underlying data. 
public:
    GL::type
        m_casted_type; // atomic type information. May be updated to include information such as the const-ness or temporary type. 
    size_t
        offset{ 0 };

private:
    void correct_type_information() {
        if (m_ptr) {
            if (m_ptr->m_actual_type.is_var() /*can_cast_var()*/) {
                if (auto* V = m_ptr->cast<GL::var>()) {
                    m_casted_type = V->get_type();
                    if (m_casted_type.is_void()) {
                        m_casted_type = GL::type_of<GL::var>();
                    }
                }
            }
            if (m_ptr->can_cast_dynamic_object()) {
                if (auto* V = m_ptr->cast<GL::dynamic_object>()) {
                    m_casted_type = V->m_type;
                }
            }
        }
    }

public:
    explicit any(GL::shared_ptr< GL::type_erasure::any_data >&& p_ptr, GL::type const& p_type, size_t attr_offset)
        : m_ptr{ std::move(p_ptr) }, m_casted_type{ p_type }, offset{ attr_offset }
    {
        correct_type_information();
    }

public:
    any() = default;
    any(any const&) = default;
    any(any&&) noexcept = default;
    any(std::nullptr_t) noexcept : m_ptr{}, m_casted_type{}, offset{ 0 } { };
    any& operator=(any const&) = default;
    any& operator=(any&&) noexcept = default;
    ~any() = default;

    template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<any, std::decay_t<ValueType>>>> static any instance(const ValueType& value) noexcept {
        return any(type_erasure::wrap(value), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>());
    };
    template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>> && !std::is_same_v<any, std::decay_t<ValueType>>>> static any instance(ValueType&& value) noexcept {
        return any(type_erasure::wrap(std::forward<ValueType>(value)), type_of<typename type_erasure::get_type<std::decay_t<ValueType>>::type>());
    };
    static any instance(any&& value) noexcept {
        return std::forward<any>(value);
    };
    static any instance(const any& value) noexcept {
        return value;
    };
    GL::shared_ptr<GL::type_erasure::any_data> get_underlying_ptr() const {
        return m_ptr;
    };
    operator bool() const noexcept {
        return m_ptr.operator bool();
    };
    bool empty() const noexcept {
        return !operator bool();
    };
    friend bool operator==(const any& a, const any& b) noexcept { return a.m_ptr == b.m_ptr; };
    friend bool operator!=(const any& a, const any& b) noexcept { return a.m_ptr != b.m_ptr; };
    friend bool operator<(const any& a, const any& b) noexcept { return a.m_ptr < b.m_ptr; };
    friend bool operator<=(const any& a, const any& b) noexcept { return a.m_ptr <= b.m_ptr; };
    friend bool operator>(const any& a, const any& b) noexcept { return a.m_ptr > b.m_ptr; };
    friend bool operator>=(const any& a, const any& b) noexcept { return a.m_ptr >= b.m_ptr; };

    bool operator&(int p_modifiers) const {
        return m_casted_type & p_modifiers;
    };
    any operator|(int p_modifiers) const {
        any out(*this);
        out.m_casted_type |= p_modifiers;
        return out;
    };
    any operator+(int p_modifiers) const {
        any out(*this);
        out.m_casted_type |= p_modifiers;
        return out;
    };
    any operator-(int p_modifiers) const {
        any out(*this);
        out.m_casted_type -= p_modifiers;
        return out;
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

    // returns true if this type can easily match the requested type (e.g. int& -> const int&)
    bool can_free_cast(GL::type const& to) const {
        if (m_casted_type.can_free_cast(to)) return true;
        //if (auto ptr = m_ptr.load_fast()) {                    
        //    return ptr->m_actual_type.can_free_cast(to);
        //}
        return false;
    };
    // returns true if this type is the same foundational type at the requested type (e.g. int&& -> const int)
    bool can_cast(GL::type const& to) const {
        if (m_casted_type.can_cast(to)) return true;
        if (m_ptr) {
            if (m_ptr->m_actual_type.can_cast(to)) return true;
            if (m_ptr->m_actual_type.is_dynamic_object() /*can_cast_dynamic_object()*/ && (to.get_base_hash() == GL::type_of<GL::dynamic_object>().get_base_hash())) { return true; }
            if (m_ptr->m_actual_type.is_var() /*can_cast_var()*/) {
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

private:
    void* ptr() const {
        if (m_ptr)
            return (void*)((char*)m_ptr->m_data + offset);
        return nullptr;
    };
    GL::shared_ptr<void> shared_ptr() const {
        if (m_ptr) {
            auto out = m_ptr->get(GL::shared_ptr< GL::type_erasure::any_data >(this->m_ptr));
            out.set_pointer_without_modifying_control_block((void*)((char*)out.get() + offset));
            return out;
        }
        return nullptr;
    };
    std::shared_ptr<void> std_shared_ptr() const {
        if (m_ptr) {
            return m_ptr->get_std(GL::shared_ptr< GL::type_erasure::any_data >(this->m_ptr), offset);
        }
        return nullptr;
    };

public:
    template<typename VType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
    decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<any*>(this)); };

    template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
    any& cast() const noexcept { return *const_cast<any*>(this); };

    template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
    any* cast() const noexcept { return const_cast<any*>(this); };

    type_erasure::any_cast cast() const noexcept;

    friend struct type_erasure::any_cast;
    friend struct type_erasure::any_cast;
    friend struct type_erasure::wrapper;
    friend class any;

    template <typename T> __declspec(noinline) static any::any wrap_member(any::any const& parent, T const& ref) {
        return any::any(GL::make_shared < GL::type_erasure::referenced_data<T> >(parent.m_ptr, GL::type_of<T const&>(), const_cast<T*>(&ref)), GL::type_of<T const&>());

        //auto outP = GL::static_pointer_cast<T>(parent.shared_ptr());
        //outP.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
        //return any::any(GL::make_shared< GL::type_erasure::shared_data<T> >(std::move(outP)), GL::type_of<T const&>());


        //auto outP = GL::static_pointer_cast<T>(parent.shared_ptr());
        //outP.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
        //auto out = GL::any::any::instance(outP);
        //out |= (GL::type::Const | GL::type::Reference);
        //return out;
    };
    template <typename T> __declspec(noinline) static any::any wrap_member(any::any const& parent, T& ref) {
        return any::any(GL::make_shared < GL::type_erasure::referenced_data<T> >(parent.m_ptr, GL::type_of<T&>(), &ref), GL::type_of<T&>());

        /*auto outP = GL::static_pointer_cast<T>(parent.shared_ptr());
        outP.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
        return any::any(GL::make_shared< GL::type_erasure::shared_data<T> >(std::move(outP)), GL::type_of<T&>());*/

        //auto outP = GL::static_pointer_cast<T>(parent.shared_ptr());
        //outP.set_pointer_without_modifying_control_block(&const_cast<T&>(ref));
        //auto out = GL::any::any::instance(std::move(outP));
        //out |= (GL::type::Reference);
        //return std::move(out);
    };
};
#endif

















template <typename R, typename Class, typename... T> class Const_Member_Function_Traits {
public:
    using argType = std::tuple<T...>;
    using classType = Class const&;
    using returnType = R;
    static constexpr auto numArgs{ std::tuple_size_v< argType > };
    Const_Member_Function_Traits(R(Class::* f)(T...) const) {
        if (!m_attr) m_attr = f;        
    };
    ~Const_Member_Function_Traits() = default;

private:
    inline static R(Class::* m_attr)(T...) const = nullptr;

public:    
    static R call(const GL::any::fast_any* begin) {
        if constexpr (numArgs >= 15) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>(), begin[15].cast<std::tuple_element_t<14, argType>>()
                );
        }
        else if constexpr (numArgs == 14) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>()
                );
        }
        else if constexpr (numArgs == 13) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                begin[13].cast<std::tuple_element_t<12, argType>>()
                );
        }
        else if constexpr (numArgs == 12) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>()
                );
        }

        else if constexpr (numArgs == 11) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>()
                );
        }
        else if constexpr (numArgs == 10) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>()
                );
        }
        else if constexpr (numArgs == 9) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>()
                );
        }
        else if constexpr (numArgs == 8) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>()
                );
        }

        else if constexpr (numArgs == 7) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>()
                );
        }
        else if constexpr (numArgs == 6) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>()
                );
        }
        else if constexpr (numArgs == 5) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>()
                );
        }
        else if constexpr (numArgs == 4) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>()
                );
        }

        else if constexpr (numArgs == 3) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>()
                );
        }
        else if constexpr (numArgs == 2) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>()
                );
        }
        else if constexpr (numArgs == 1) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>()
                );
        }
        else if constexpr (numArgs == 0) {
            return (begin[0].cast<classType>().*m_attr)(
                );
        }
    }; 
};
template <typename R, typename Class, typename... T> class Member_Function_Traits {
public:
    using argType = std::tuple<T...>;
    using classType = Class&;
    using returnType = R;
    static constexpr auto numArgs{ std::tuple_size_v< argType > };
    Member_Function_Traits(R(Class::* f)(T...)) {
        if (!m_attr) m_attr = f;
    };
    ~Member_Function_Traits() = default;

private:
    inline static R(Class::* m_attr)(T...) = nullptr;

public:
    static R call(const GL::any::fast_any* begin) {
        if constexpr (numArgs >= 15) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>(), begin[15].cast<std::tuple_element_t<14, argType>>()
                );
        }
        else if constexpr (numArgs == 14) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>()
                );
        }
        else if constexpr (numArgs == 13) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                begin[13].cast<std::tuple_element_t<12, argType>>()
                );
        }
        else if constexpr (numArgs == 12) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>()
                );
        }

        else if constexpr (numArgs == 11) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>()
                );
        }
        else if constexpr (numArgs == 10) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>()
                );
        }
        else if constexpr (numArgs == 9) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                begin[9].cast<std::tuple_element_t<8, argType>>()
                );
        }
        else if constexpr (numArgs == 8) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>()
                );
        }

        else if constexpr (numArgs == 7) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>()
                );
        }
        else if constexpr (numArgs == 6) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>()
                );
        }
        else if constexpr (numArgs == 5) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                begin[5].cast<std::tuple_element_t<4, argType>>()
                );
        }
        else if constexpr (numArgs == 4) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>()
                );
        }

        else if constexpr (numArgs == 3) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>()
                );
        }
        else if constexpr (numArgs == 2) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>()
                );
        }
        else if constexpr (numArgs == 1) {
            return (begin[0].cast<classType>().*m_attr)(
                begin[1].cast<std::tuple_element_t<0, argType>>()
                );
        }
        else if constexpr (numArgs == 0) {
            return (begin[0].cast<classType>().*m_attr)(
                );
        }
    };
};
template <typename R, typename Class> class Attribute_Function_Traits {
public:
    using classType = Class;
    using returnType = R;
    static_assert(!std::is_same_v<void, returnType>);

    Attribute_Function_Traits(R Class::* t_attr) {        
        if (!m_attr) m_attr = t_attr;
    };
    ~Attribute_Function_Traits() = default;

private:
    inline static R Class::* m_attr = nullptr;

public:
    static returnType& call(const GL::any::fast_any* begin) {
        return begin->cast<Class>().*m_attr;
    };
};
template <typename R, typename... T> class Static_Function_Traits {
public:
    using argType = std::tuple<T...>;
    using returnType = R;
    static constexpr auto numArgs{ std::tuple_size_v< argType > };
    Static_Function_Traits(R(*f)(T...)) {
        if (!m_attr) m_attr = f;
    };
    ~Static_Function_Traits() = default;

private:
    inline static R(*m_attr)(T...) = nullptr;

public:
    static R call(const GL::any::fast_any* begin) {
        if constexpr (numArgs >= 16) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>(), begin[16 - 1].cast<std::tuple_element_t<15, argType>>()
                );
        }
        else if constexpr (numArgs == 15) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>()
                );
        }
        else if constexpr (numArgs == 14) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>()
                );
        }
        else if constexpr (numArgs == 13) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>()
                );
        }
        else if constexpr (numArgs == 12) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>()
                );
        }

        else if constexpr (numArgs == 11) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>()
                );
        }
        else if constexpr (numArgs == 10) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>()
                );
        }
        else if constexpr (numArgs == 9) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>()
                );
        }
        else if constexpr (numArgs == 8) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>()
                );
        }

        else if constexpr (numArgs == 7) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>()
                );
        }
        else if constexpr (numArgs == 6) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>()
                );
        }
        else if constexpr (numArgs == 5) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>()
                );
        }
        else if constexpr (numArgs == 4) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>()
                );
        }

        else if constexpr (numArgs == 3) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>()
                );
        }
        else if constexpr (numArgs == 2) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>()
                );
        }
        else if constexpr (numArgs == 1) {
            return (*m_attr)(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>()
                );
        }
        else if constexpr (numArgs == 0) {
            return (*m_attr)(
                );
        }
    };
};
template <typename LambdaFunction> class Lambda_Function_Traits {
private:
    using function_traits = GL::parallel::impl::function_traits< decltype(std::function(std::declval<LambdaFunction>())) >;
public:
    using argType = typename function_traits::arguments;
    using returnType = typename function_traits::result_type;
    static constexpr auto numArgs{ std::tuple_size_v< argType > };
    Lambda_Function_Traits() = default;
    ~Lambda_Function_Traits() = default;
public:
    static returnType call(const GL::any::fast_any* begin, LambdaFunction const& m_attr) {
        if constexpr (numArgs >= 16) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>(), begin[16 - 1].cast<std::tuple_element_t<15, argType>>()
                );
        }
        else if constexpr (numArgs == 15) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>()
                );
        }
        else if constexpr (numArgs == 14) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>()
                );
        }
        else if constexpr (numArgs == 13) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                begin[13 - 1].cast<std::tuple_element_t<12, argType>>()
                );
        }
        else if constexpr (numArgs == 12) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>()
                );
        }

        else if constexpr (numArgs == 11) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>()
                );
        }
        else if constexpr (numArgs == 10) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>()
                );
        }
        else if constexpr (numArgs == 9) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                begin[9 - 1].cast<std::tuple_element_t<8, argType>>()
                );
        }
        else if constexpr (numArgs == 8) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>()
                );
        }

        else if constexpr (numArgs == 7) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>()
                );
        }
        else if constexpr (numArgs == 6) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>()
                );
        }
        else if constexpr (numArgs == 5) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                begin[5 - 1].cast<std::tuple_element_t<4, argType>>()
                );
        }
        else if constexpr (numArgs == 4) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>()
                );
        }

        else if constexpr (numArgs == 3) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>()
                );
        }
        else if constexpr (numArgs == 2) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>()
                );
        }
        else if constexpr (numArgs == 1) {
            return m_attr(
                begin[1 - 1].cast<std::tuple_element_t<0, argType>>()
                );
        }
        else if constexpr (numArgs == 0) {
            return m_attr(
                );
        }
    };
};

class Function_Caller {
public:
    virtual ~Function_Caller() = default;    
    virtual void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const = 0;    
};

template <typename Function> class Const_Member_Function_Caller final : public Function_Caller {
public:
    using traits = decltype(Const_Member_Function_Traits(std::declval< Function>()));
    Const_Member_Function_Caller(Function&& Func) {
        (void)Const_Member_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
    };
    virtual ~Const_Member_Function_Caller() = default;
    static typename traits::returnType call(const GL::any::fast_any* begin) {
        return traits::call(std::move(begin));
    };
    void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const override {
        if constexpr (std::is_same_v<void, typename traits::returnType>) {
            call(std::move(begin));
        }
        else {
            if (out) {
                *out = GL::any::fast_any::instance(call(std::move(begin)));
            }
            else {
                call(std::move(begin));
            }
        }
    };
};
template <typename Function> class Member_Function_Caller final : public Function_Caller {
public:
    using traits = decltype(Member_Function_Traits(std::declval< Function>()));
    Member_Function_Caller(Function&& Func) {
        (void)Member_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
    };
    virtual ~Member_Function_Caller() = default;
    static typename traits::returnType call(const GL::any::fast_any* begin) {
        return traits::call(std::move(begin));
    };
    void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const override {
        if constexpr (std::is_same_v<void, typename traits::returnType>) {
            call(std::move(begin));
        }
        else {
            if (out) {
                *out = GL::any::fast_any::instance(call(std::move(begin)));
            }
            else {
                call(std::move(begin));
            }
        }
    };
};
template<typename Ret, typename Class, typename... Param> auto Member_Function_Caller_Impl(Ret(Class::* f)(Param...) const) {
    return Const_Member_Function_Caller(std::move(f));
};
template<typename Ret, typename Class, typename... Param> auto Member_Function_Caller_Impl(Ret(Class::* f)(Param...)) {
    return Member_Function_Caller(std::move(f));
};
template <typename Function> class Attribute_Function_Caller final : public Function_Caller {
public:
    using traits = decltype(Attribute_Function_Traits(std::declval< Function>()));
    Attribute_Function_Caller(Function&& Func) {
        (void)Attribute_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
    };
    virtual ~Attribute_Function_Caller() = default;
    static typename traits::returnType& call(const GL::any::fast_any* begin) {
        return traits::call(std::move(begin));
    };
    __declspec(noinline) void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const override {
        if (out) {
            if constexpr (std::is_same_v<GL::any, std::decay_t<typename traits::returnType>> || std::is_same_v<GL::any::fast_any, std::decay_t<typename traits::returnType>>) {
                if (begin->m_casted_type.is_const()) {
                    *out = call(std::move(begin)) | (GL::type::Const | GL::type::Reference);
                }
                else {
                    *out = call(std::move(begin)) | GL::type::Reference;
                }
            }
            else if constexpr (std::is_pointer<typename traits::returnType>::value) {
                if (begin->m_casted_type.is_const()) {
                    *out = GL::any::fast_any::wrap_member(*begin, *call(std::move(begin)));
                    *out |= GL::type::Const;
                }
                else {
                    *out = GL::any::fast_any::wrap_member(*begin, *call(std::move(begin)));
                }
            }
            else {
                if (begin->m_casted_type.is_const()) {
                    *out = GL::any::fast_any::wrap_member(*begin, call(std::move(begin)));
                    *out |= GL::type::Const;
                }
                else {
                    *out = GL::any::fast_any::wrap_member(*begin, call(std::move(begin)));
                }
            }
        }     
    };
};
template <typename Function> class Unsafe_Attribute_Function_Caller final : public Function_Caller {
public:
    using traits = decltype(Attribute_Function_Traits(std::declval< Function>()));
    Unsafe_Attribute_Function_Caller(Function&& Func) {
        (void)Attribute_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
    };
    virtual ~Unsafe_Attribute_Function_Caller() = default;
    static typename traits::returnType& call(const GL::any::fast_any* begin) {
        return traits::call(std::move(begin));
    };
    __declspec(noinline) void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const override {
        if (out) {
            if constexpr (std::is_same_v<GL::any, std::decay_t<typename traits::returnType>> || std::is_same_v<GL::any::fast_any, std::decay_t<typename traits::returnType>>) {
                if (begin->m_casted_type.is_const()) {
                    *out = call(std::move(begin)) | (GL::type::Const | GL::type::Reference);
                }
                else {
                    *out = call(std::move(begin)) | GL::type::Reference;
                }
            }
            else if constexpr (std::is_pointer<typename traits::returnType>::value) {
                if (begin->m_casted_type.is_const()) {
                    // *out = GL::any::fast_any::instance(GL::shared_ptr<typename traits::returnType>((typename traits::returnType*)(call(std::move(begin))), [parent = *begin](typename traits::returnType*) {}));
                    *out = GL::any::fast_any::wrap_member(*begin, *call(std::move(begin)));
                    *out |= GL::type::Const;
                }
                else {
                    // *out = GL::any::fast_any::instance(GL::shared_ptr<typename traits::returnType>((typename traits::returnType*)(call(std::move(begin))), [parent = *begin](typename traits::returnType*) {}));
                    *out = GL::any::fast_any::wrap_member(*begin, *call(std::move(begin)));
                }
            }
            else {
                if (begin->m_casted_type.is_const()) {
                    // *out = GL::any::fast_any::instance(GL::shared_ptr<typename traits::returnType>((typename traits::returnType*)(&call(std::move(begin))), [parent = *begin](typename traits::returnType*) {}));
                    *out = GL::any::fast_any::wrap_member(*begin, call(std::move(begin)));
                    *out |= GL::type::Const;
                }
                else {
                    // *out = GL::any::fast_any::instance(GL::shared_ptr<typename traits::returnType>((typename traits::returnType*)(&call(std::move(begin))), [parent = *begin](typename traits::returnType*) {}));
                    *out = GL::any::fast_any::wrap_member(*begin, call(std::move(begin)));
                }
            }
        }
    };
};
template <typename Function> class Static_Function_Caller final : public Function_Caller {
public:
    using traits = decltype(Static_Function_Traits(std::declval<Function>()));
    Static_Function_Caller(Function && Func) {
        (void)Static_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
    };
    virtual ~Static_Function_Caller() = default;

    static typename traits::returnType call(const GL::any::fast_any* begin) {
        return traits::call(std::move(begin));
    };
    void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const override {
        if constexpr (std::is_same_v<void, typename traits::returnType>) {
            call(std::move(begin));
        }
        else {
            if (out) {
                *out = GL::any::fast_any::instance(call(std::move(begin)));
            }
            else {
                call(std::move(begin));
            }
        }
    };
};
/* Converts to a static function caller if the lambda does not capture. Otherwise, uses the local function copy. */
template <typename Function> class Lambda_Function_Caller final : public Function_Caller {
private:
    Function func;
public:
    using traits = typename Lambda_Function_Traits< Function >;
    Lambda_Function_Caller(Function && Func) : func(std::forward<Function>(Func)) {
        if constexpr (std::is_empty_v< Function >) {
            (void)Static_Function_Traits(+Func); // instantiate the static function pointer
        }
    };
    virtual ~Lambda_Function_Caller() = default;
    typename traits::returnType call(const GL::any::fast_any* begin) const {
        if constexpr (std::is_empty_v< Function >) {
            using static_traits = decltype(Static_Function_Traits(+std::declval<Function>()));
            return static_traits::call(std::move(begin));
        }
        else {
            return Lambda_Function_Traits< Function >::call(std::move(begin), func);
        }        
    };
    void call(const GL::any::fast_any* begin, GL::any::fast_any* out) const override {
        if constexpr (std::is_same_v<void, typename traits::returnType>) {
            call(std::move(begin));
        }
        else {
            if (out) {
                *out = GL::any::fast_any::instance(call(std::move(begin)));
            }
            else {
                call(std::move(begin));
            }
        }
    };
};

template <typename Function> auto make_callable(Function&& func) {
    typedef decltype(GL::details::detail::function_signature(func)) function_header;
    if constexpr (function_header::is_object) { // function objects, e.g. auto x = [](){};
        if constexpr (std::is_empty_v< Function >) {
            return Static_Function_Caller(+std::move(func));
        }
        else {
            return Lambda_Function_Caller(std::move(func));
        }
    }
    else if constexpr (function_header::is_member_object) { // member objects, e.g. return object.member;    
        return Attribute_Function_Caller(std::move(func));         
    }
    else if constexpr (function_header::is_member && !function_header::is_member_object) { // member functions, e.g. return object.member();
        return Member_Function_Caller_Impl(std::move(func));
    }
    else if constexpr (function_header::is_static_member_function) { // static function pointers, e.g. static foo(){};   
        return Static_Function_Caller(std::move(func));
    }
    else {
        throw std::runtime_error("Did not handle conversion of provided function.");
    }
};


int main() {
#if 1
    while (1) {    
        switch ((int)std::floor(GL::util::rand(0, 11.999))) {
        case 0: {
                GL::string ref = "this";
                if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (unboxed value)\t")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)ref.begins_with("this");
                    }
                }
            } break;
        case 1: {
                GL::string ref = "this";
                if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (unboxed ref)\t")) {
                    for (int i = 0; i < 1000000; ++i) {
                        GL::string& Ref = ref;
                        (void)Ref.begins_with("this");
                    }
                }
            } break;
        case 2: {
                auto boxed = GL::any::fast_any::instance(GL::make_shared<GL::string>("this"));
                if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed GL::shared_ptr)\t")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)boxed.cast<GL::string&>().begins_with("this");
                    }
                }
            } break;
        case 3: {
                auto boxed = GL::any::fast_any::instance(GL::string("this"));
                if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed value)\t")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)boxed.cast<GL::string&>().begins_with("this");
                    }
                }
            } break;
        case 4: if (0) {
            auto boxed = GL::any::fast_any::instance(GL::string("this"));
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed cast)\t")) {
                for (int i = 0; i < 1000000; ++i) {
                    GL::string& Ref = boxed.cast();
                    (void)Ref.begins_with("this");
                }
            }
        } break;
        case 5: {            
            auto callable = GL::make_callable("begins_with", &GL::string::begins_with);
            std::array<GL::any::fast_any, 2> example{
                GL::any::fast_any::instance(GL::string("this")),
                GL::any::fast_any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with callable and w/o converters, no conversion needed, w/o return")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable->operator()(&example[0], &example[0] + example.size(), false);
                }
            }            
        } break;
        case 6: {
            auto callable{ make_callable(&GL::string::begins_with) };

            std::array<GL::any::fast_any, 2> example {
                GL::any::fast_any::instance(GL::string("this")),
                GL::any::fast_any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with templatized callable and w/o converters, no conversion needed, w/ return")) {
                for (int i = 0; i < 1000000; ++i) {
                   (void)callable.call(&example[0]);
                }
            }
        } break;
        case 7: {
            auto callable{ make_callable(&GL::string::begins_with) };
            std::array<GL::any::fast_any, 2> example{
                GL::any::fast_any::instance(GL::string("this")),
                GL::any::fast_any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with generalized callable and w/o converters, no conversion needed, w/o return")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable.call(&example[0], nullptr);
                }
            }
        } break;
        case 8: {
            auto callable{ make_callable(&GL::string::empty_string) };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::empty_string() with generalized callable and w/o converters, no conversion needed, w/o return")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable.call(nullptr, nullptr);
                }
            }
        } break;
        case 9: {
            auto callable{ make_callable([]() -> auto { return GL::string::empty_string(); }) };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::empty_string() with non-capturing lambda function and w/o converters, no conversion needed, w/o return")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable.call(nullptr, nullptr);
                }
            }
        } break;
        case 10: {
            struct TEST {
                GL::string obj;
            };
            std::array<GL::any::fast_any, 1> example{
                GL::any::fast_any::instance(TEST{ GL::string("this") })
            };
            auto callable{ make_callable(&TEST::obj) };
            if (auto timer = GL::stopwatch().debug_timer("TEST::obj, w/o return")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable.call(&example[0], nullptr);
                }
            }
        } break;
        case 11: {
            struct TEST {
                GL::string obj;
            };
            std::array<GL::any::fast_any, 1> example{
                GL::any::fast_any::instance(TEST{ GL::string("this") })
            };
            GL::any::fast_any out = example[0];
            auto callable{ make_callable(&TEST::obj) };
            
            if (auto timer = GL::stopwatch().debug_timer("TEST::obj, w/ return")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable.call(&example[0], &out);
                }
            }
        } break;
        }




        if (0) {
            GL::scope::impl::RootScope root;
            root.perform_builtins();
            root.try_get_converter(GL::type_of<GL::string>(), GL::type_of<GL::string>());
            auto converters = root.get_converters();
            auto callable = GL::make_callable("size", &GL::string::size);
            if (1) {
                std::array<GL::any::fast_any, 1> example{
                     GL::any::fast_any::instance(GL::string("this"))
                };
                if (auto timer = GL::stopwatch().debug_timer("operator() with callable and w/converters, no conversion needed")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)callable->operator()(&example[0], &example[0] + example.size());
                    }
                }
            }
            if (1) {
                std::array<GL::any::fast_any, 2> example{
                     GL::any::fast_any::instance(std::string("this"))
                };
                if (auto timer = GL::stopwatch().debug_timer("operator() with callable and w/converters, conversion needed")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)callable->operator()(&example[0], &example[0] + example.size());
                    }
                }
            }
        }
    }   
#endif
    using namespace GL::literals;
    struct F {
        static void ToDo(GL::foot const& i) {
            if (i > 0ull) throw std::runtime_error("e");
        };
        static GL::meter AsyncTest(GL::meter const& i) {
            return i;
        };
    };

    // in parallel, each thread attempts this parallel-tasking test suite
    GL::parallel::While(
        []() -> bool { return true; }, 
        []() {
            // casting is automatic for all of these
            auto task_sequence = GL::parallel::task([]() -> GL::millisecond {
                std::cout << "First...\n";
                return GL::millisecond(0);
            })->and_then([](GL::second t0) -> GL::millisecond {
                std::cout << "Second...\n";
                return t0;
            })->and_then(0, 10'000, [](size_t i, GL::millisecond& t0, GL::job_base& parent) -> GL::any::fast_any {
                t0 += 1;
                return parent.result;
            })->and_then([](GL::any::fast_any const& t0) {
                if (t0.cast<GL::millisecond>() != 10'000_ms) throw "SHOULD HAVE MATCHED";
                std::cout << "Third and done.\n";
            });
            
            GL::value progress = 0;
            auto task_1 = GL::parallel::task(0, 10'000, [&progress]() {
                if ((++progress).mod(1000) == 0) {
                    std::cout << GL::printf("%i percent\n", (int)(100.0f * ((float)progress / 10'000.0f)));
                }
                GL::stopwatch sw; sw.reset();
                while (sw.check() < 0.001) {}
            });

            // casting is automatic for this
            GL::parallel::For(0, 1'000'000, [](size_t i) {});
            // casting is automatic for this
            (void)GL::parallel::async([](double i) { return i; }, 100);
            // no casting required
            GL::parallel::Until(
                []() {},
                []() -> bool { return true; }
            );
            // no casting required
            GL::parallel::While(
                []() -> bool { return false; },
                []() {}
            );
            std::cout << "All jobs submitted.\n";
        }
    );

    for (int j = 0; j < 1'000'000; ++j) {
        auto future1 = GL::parallel::async(&F::AsyncTest, 10_ft);
        if (GL::type_of<GL::meter>() != future1.as_promise().Type()) throw "SHOULD HAVE MATCHED";
        if (future1.get_ref() != 10_ft) throw "SHOULD HAVE MATCHED";

        auto future2 = GL::parallel::async([](GL::millisecond const& t) {
            ::Sleep((long long)t.operator float());
        }, 0.01_s);

        GL::parallel::For(0, -1'000'000, &F::ToDo);
    }
    return 0;
};

