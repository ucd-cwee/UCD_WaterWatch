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
#include "datetime.h"

// Declaration
namespace uuid {
    static constexpr unsigned long FLAGS = 0xF000'0000;
    static constexpr unsigned long INV_FLAGS = ~FLAGS;
    struct uuid_ticket {
        size_t count{ 0 };
        GL::shared_ptr<GL::type_erasure::any_data> data{ nullptr };
    };
    static unsigned long new_uuid(GL::shared_ptr<GL::type_erasure::any_data>&& rhs) noexcept;
    static void free_uuid(unsigned long uuid) noexcept;
    static uuid_ticket& get_uuid(unsigned long rhs) noexcept;
};

// wrapper
class any {
public:
    void* m_ptr;    
    GL::type m_type;
    unsigned long m_uuid;

public:
    any(unsigned long&& uuid, void* data, GL::type const& type) noexcept : m_uuid{ std::forward<unsigned long>(uuid) }, m_type{ type }, m_ptr{ data } {};
    any() noexcept : m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr } {};
    any(std::nullptr_t) noexcept : m_uuid{ 0 }, m_type{ 0 }, m_ptr{ nullptr } {};
    any(any const& rhs) : m_uuid{ rhs.m_uuid & uuid::INV_FLAGS }, m_type{ rhs.m_type }, m_ptr{ rhs.m_ptr } { if (m_uuid > 0) GL::interlocked::increment(uuid::get_uuid(m_uuid).count); };
    // declares that the parent will NOT go out-of-scope before this child does. That guarrantee allows us to skip a increment and decrement call to the counter. 
    any(any const& rhs, bool) : m_uuid{ rhs.m_uuid | 0x1000'0000 }, m_type{ rhs.m_type }, m_ptr{ rhs.m_ptr } {};
    any(any && rhs) noexcept : m_uuid{ std::move(rhs.m_uuid) }, m_type{ std::move(rhs.m_type) }, m_ptr{ rhs.m_ptr } { rhs.m_uuid = 0; };
    any const& operator=(any const& rhs) {
        if ((m_uuid & 0x1000'0000) == 0)
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
        if ((m_uuid & 0x1000'0000) == 0)
            if ((m_uuid & uuid::INV_FLAGS) > 0)
                if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                    uuid::free_uuid(m_uuid);

        m_uuid = std::move(rhs.m_uuid);
        m_type = std::move(rhs.m_type);
        m_ptr = rhs.m_ptr;
        rhs.m_uuid = 0;
        return *this;
    };
    ~any() {
        if ((m_uuid & 0x1000'0000) == 0) 
            if ((m_uuid & uuid::INV_FLAGS) > 0) 
                if (GL::interlocked::decrement(uuid::get_uuid(m_uuid).count) == 0)
                    uuid::free_uuid(m_uuid);
    };

    template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> static any instance(const ValueType& value) noexcept {
        auto wrapped = GL::type_erasure::wrap(value);
        auto* ptr = wrapped->get();
        return any(uuid::new_uuid(std::move(wrapped)), ptr, GL::type_of<typename GL::type_erasure::get_type<std::decay_t<ValueType>>::type>());
    };
    template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<any, std::decay_t<ValueType>>>> static any instance(ValueType&& value) noexcept {
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
    GL::shared_ptr<GL::type_erasure::any_data>& get_underlying_ptr() const noexcept {
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
        if ((parent.m_uuid & 0x1000'0000) > 0) {
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
        if ((parent.m_uuid & 0x1000'0000) > 0) {
            return any((unsigned long)parent.m_uuid, &ref, GL::type_of<T&>());
        }
        else {
            unsigned long uuid = parent.m_uuid & uuid::INV_FLAGS;
            if (uuid > 0) GL::interlocked::increment(uuid::get_uuid(uuid).count);
            return any(std::move(uuid), &ref, GL::type_of<T&>());
        }
    };

};

// Definition
namespace uuid {
    static GL::fast_ticket_dispensor tickets;
    static GL::atomic_vector< uuid_ticket > slots; 
    unsigned long new_uuid(GL::shared_ptr<GL::type_erasure::any_data>&& rhs) noexcept {
        unsigned long uuid = (unsigned long)tickets.get_ticket();
        auto& ref = slots.get_or_make(uuid);
        ref.count = 1;
        ref.data = std::forward<GL::shared_ptr<GL::type_erasure::any_data>>(rhs);
        return uuid;
    };
    void free_uuid(unsigned long uuid) noexcept {
        uuid &= INV_FLAGS;
        if (uuid > 0) {
            auto& ref = slots[uuid];
            // if (!ref.data->is_pod()) 
                ref.data = nullptr;            
            tickets.return_ticket(uuid);
        }
    };
    uuid_ticket& get_uuid(unsigned long uuid) noexcept {
        return slots.at(uuid & INV_FLAGS);
    };
};

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
    static R call(const any* begin) {
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
    static R call(const any* begin) {
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
    static returnType& call(const any* begin) {
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
    static R call(const any* begin) {
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
    static returnType call(const any* begin, LambdaFunction const& m_attr) {
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
    // implimentation-specific function
    virtual void call(const any* const begin, any* const out) const = 0;
    virtual size_t num_arguments() const = 0;
    virtual GL::type const& argument(size_t index) const = 0;
    virtual GL::type const& returns() const = 0;
    std::array<const GL::type*, 16> arguments() const {
        return std::array<const GL::type*, 16>{
            &argument(0), & argument(1), & argument(2), & argument(3),
                & argument(4), & argument(5), & argument(6), & argument(7),
                & argument(8), & argument(9), & argument(10), & argument(11),
                & argument(12), & argument(13), & argument(14), & argument(15)
        };
    };

#if 0
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9, const any& p10, const any& p11,
        const any& p12, const any& p13, const any& p14, const any& p15
        , any* out) const {
        unsigned char buf[sizeof(any) * 16];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        new (&reinterpret_cast<any*>(&buf[0])[10]) any(p10, true);
        new (&reinterpret_cast<any*>(&buf[0])[11]) any(p11, true);
        new (&reinterpret_cast<any*>(&buf[0])[12]) any(p12, true);
        new (&reinterpret_cast<any*>(&buf[0])[13]) any(p13, true);
        new (&reinterpret_cast<any*>(&buf[0])[14]) any(p14, true);
        new (&reinterpret_cast<any*>(&buf[0])[15]) any(p15, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9, const any& p10, const any& p11,
        const any& p12, const any& p13, const any& p14
        , any* out) const {
        unsigned char buf[sizeof(any) * 15];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        new (&reinterpret_cast<any*>(&buf[0])[10]) any(p10, true);
        new (&reinterpret_cast<any*>(&buf[0])[11]) any(p11, true);
        new (&reinterpret_cast<any*>(&buf[0])[12]) any(p12, true);
        new (&reinterpret_cast<any*>(&buf[0])[13]) any(p13, true);
        new (&reinterpret_cast<any*>(&buf[0])[14]) any(p14, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9, const any& p10, const any& p11,
        const any& p12, const any& p13
        , any* out) const {
        unsigned char buf[sizeof(any) * 14];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        new (&reinterpret_cast<any*>(&buf[0])[10]) any(p10, true);
        new (&reinterpret_cast<any*>(&buf[0])[11]) any(p11, true);
        new (&reinterpret_cast<any*>(&buf[0])[12]) any(p12, true);
        new (&reinterpret_cast<any*>(&buf[0])[13]) any(p13, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9, const any& p10, const any& p11,
        const any& p12
        , any* out) const {
        unsigned char buf[sizeof(any) * 13];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        new (&reinterpret_cast<any*>(&buf[0])[10]) any(p10, true);
        new (&reinterpret_cast<any*>(&buf[0])[11]) any(p11, true);
        new (&reinterpret_cast<any*>(&buf[0])[12]) any(p12, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9, const any& p10, const any& p11
        , any* out) const {
        unsigned char buf[sizeof(any) * 12];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        new (&reinterpret_cast<any*>(&buf[0])[10]) any(p10, true);
        new (&reinterpret_cast<any*>(&buf[0])[11]) any(p11, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9, const any& p10
        , any* out) const {
        unsigned char buf[sizeof(any) * 11];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        new (&reinterpret_cast<any*>(&buf[0])[10]) any(p10, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8, const any& p9
        , any* out) const {
        unsigned char buf[sizeof(any) * 10];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        new (&reinterpret_cast<any*>(&buf[0])[9]) any(p9, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7,
        const any& p8
        , any* out) const {
        unsigned char buf[sizeof(any) * 9];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        new (&reinterpret_cast<any*>(&buf[0])[8]) any(p8, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6, const any& p7
        , any* out) const {
        unsigned char buf[sizeof(any) * 8];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        new (&reinterpret_cast<any*>(&buf[0])[7]) any(p7, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5, const any& p6
        , any* out) const {
        unsigned char buf[sizeof(any) * 7];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        new (&reinterpret_cast<any*>(&buf[0])[6]) any(p6, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4, const any& p5
        , any* out) const {
        unsigned char buf[sizeof(any) * 6];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        new (&reinterpret_cast<any*>(&buf[0])[5]) any(p5, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3,
        const any& p4
        , any* out) const {
        unsigned char buf[sizeof(any) * 5];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        new (&reinterpret_cast<any*>(&buf[0])[4]) any(p4, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2, const any& p3
        , any* out) const {
        unsigned char buf[sizeof(any) * 4];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        new (&reinterpret_cast<any*>(&buf[0])[3]) any(p3, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1, const any& p2
        , any* out) const {
        unsigned char buf[sizeof(any) * 3];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        new (&reinterpret_cast<any*>(&buf[0])[2]) any(p2, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. Adds a small overhead due to the need to make an array per-call. 
    void call(
        const any& p0, const any& p1
        , any* out) const {
        unsigned char buf[sizeof(any) * 2];
        new (&reinterpret_cast<any*>(&buf[0])[0]) any(p0, true);
        new (&reinterpret_cast<any*>(&buf[0])[1]) any(p1, true);
        call(reinterpret_cast<any*>(&buf[0]), out);
    };
    /// Convenience function for easier calling syntax. No overhead is added to make this call.
    void call(
        const any& p0
        , any* out) const {
        call(&p0, out);
    };
    /// Convenience function for easier calling syntax. No overhead is added to make this call.
    void call(
        any* out) const {
        call(nullptr, out);
    };

    // convenience function that will allow the user to easily call a proxy function with arguments. Best performance achieved if passing-in boxed any{} objects already. 
    template<typename T = void, typename... Args> decltype(auto) do_call(Args&&... args) const {
        if constexpr (std::is_same_v<void, T>) {
            call(any::instance(std::forward<Args>(args))..., nullptr);
        }
        else {
            any out;
            call(any::instance(std::forward<Args>(args))..., &out);
            if constexpr (std::is_same_v<any, T>) {
                return out;
            }
            else {
                return (T)out.cast<T>();
            }
        }
    };
#endif
};
namespace {
    template <typename Function> class Const_Member_Function_Caller final : public Function_Caller {
    public:
        using traits = decltype(Const_Member_Function_Traits(std::declval< Function>()));
        Const_Member_Function_Caller(Function&& Func) {
            (void)Const_Member_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
        };
        virtual ~Const_Member_Function_Caller() = default;
        static typename traits::returnType call(const any* begin) {
            return traits::call(begin);
        };
        void call(const any* const begin, any* const out) const override {
            if constexpr (std::is_same_v<void, typename traits::returnType>) {
                call(begin);
            }
            else {
                if (out) {
#if 1
                    if ((out->m_uuid & 0x1000'0000) == 0) {
                        if ((out->m_uuid & uuid::INV_FLAGS) > 0) {
                            auto& ref = uuid::get_uuid(out->m_uuid);
                            if (GL::interlocked::decrement(ref.count) == 0) {
                                ref.count = 1;
                                ref.data = GL::type_erasure::wrap(call(begin));
                                out->m_ptr = ref.data->get();
                                out->m_type = GL::type_of<typename traits::returnType>();
                            }
                            else {
                                out->m_uuid = 0;
                                *out = any::instance(call(begin));
                            }
                        }
                        else {
                            *out = any::instance(call(begin));
                        }
                    }
                    else {
                        *out = any::instance(call(begin));
                    }
#else
                    * out = any::instance(call(begin));
#endif
                }
                else {
                    call(begin);
                }
            }
        };
        size_t num_arguments() const override {
            return traits::numArgs + 1;
        };
        GL::type const& argument(size_t index) const override {
            if (index == 0)
                return GL::type_of<typename traits::classType>();
            switch (index - 1) {
            case 0: if constexpr (traits::numArgs > 0) return GL::type_of<std::tuple_element_t<0, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 1: if constexpr (traits::numArgs > 1) return GL::type_of<std::tuple_element_t<1, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 2: if constexpr (traits::numArgs > 2) return GL::type_of<std::tuple_element_t<2, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 3: if constexpr (traits::numArgs > 3) return GL::type_of<std::tuple_element_t<3, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 4: if constexpr (traits::numArgs > 4) return GL::type_of<std::tuple_element_t<4, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 5: if constexpr (traits::numArgs > 5) return GL::type_of<std::tuple_element_t<5, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 6: if constexpr (traits::numArgs > 6) return GL::type_of<std::tuple_element_t<6, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 7: if constexpr (traits::numArgs > 7) return GL::type_of<std::tuple_element_t<7, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 8: if constexpr (traits::numArgs > 8) return GL::type_of<std::tuple_element_t<8, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 9: if constexpr (traits::numArgs > 9) return GL::type_of<std::tuple_element_t<9, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 10: if constexpr (traits::numArgs > 10) return GL::type_of<std::tuple_element_t<10, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 11: if constexpr (traits::numArgs > 11) return GL::type_of<std::tuple_element_t<11, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 12: if constexpr (traits::numArgs > 12) return GL::type_of<std::tuple_element_t<12, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 13: if constexpr (traits::numArgs > 13) return GL::type_of<std::tuple_element_t<13, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 14: if constexpr (traits::numArgs > 14) return GL::type_of<std::tuple_element_t<14, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 15: if constexpr (traits::numArgs > 15) return GL::type_of<std::tuple_element_t<15, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            default:
                return GL::type_of<GL::undefined>();
            }
        };
        GL::type const& returns() const override {
            return GL::type_of<typename traits::returnType>();
        };
    };
    template <typename Function> class Member_Function_Caller final : public Function_Caller {
    public:
        using traits = decltype(Member_Function_Traits(std::declval< Function>()));
        Member_Function_Caller(Function&& Func) {
            (void)Member_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
        };
        virtual ~Member_Function_Caller() = default;
        static typename traits::returnType call(const any* begin) {
            return traits::call(begin);
        };
        void call(const any* const begin, any* const out) const override {
            if constexpr (std::is_same_v<void, typename traits::returnType>) {
                call(begin);
            }
            else {
                if (out) {
#if 1
                    if ((out->m_uuid & 0x1000'0000) == 0) {
                        if ((out->m_uuid & uuid::INV_FLAGS) > 0) {
                            auto& ref = uuid::get_uuid(out->m_uuid);
                            if (GL::interlocked::decrement(ref.count) == 0) {
                                ref.count = 1;
                                ref.data = GL::type_erasure::wrap(call(begin));
                                out->m_ptr = ref.data->get();
                                out->m_type = GL::type_of<typename traits::returnType>();
                            }
                            else {
                                out->m_uuid = 0;
                                *out = any::instance(call(begin));
                            }
                        }
                        else {
                            *out = any::instance(call(begin));
                        }
                    }
                    else {
                        *out = any::instance(call(begin));
                    }
#else
                    * out = any::instance(call(begin));
#endif
                }
                else {
                    call(begin);
                }
            }
        };
        size_t num_arguments() const override {
            return traits::numArgs + 1;
        };
        GL::type const& argument(size_t index) const override {
            if (index == 0)
                return GL::type_of<typename traits::classType>();
            switch (index - 1) {
            case 0: if constexpr (traits::numArgs > 0) return GL::type_of<std::tuple_element_t<0, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 1: if constexpr (traits::numArgs > 1) return GL::type_of<std::tuple_element_t<1, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 2: if constexpr (traits::numArgs > 2) return GL::type_of<std::tuple_element_t<2, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 3: if constexpr (traits::numArgs > 3) return GL::type_of<std::tuple_element_t<3, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 4: if constexpr (traits::numArgs > 4) return GL::type_of<std::tuple_element_t<4, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 5: if constexpr (traits::numArgs > 5) return GL::type_of<std::tuple_element_t<5, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 6: if constexpr (traits::numArgs > 6) return GL::type_of<std::tuple_element_t<6, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 7: if constexpr (traits::numArgs > 7) return GL::type_of<std::tuple_element_t<7, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 8: if constexpr (traits::numArgs > 8) return GL::type_of<std::tuple_element_t<8, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 9: if constexpr (traits::numArgs > 9) return GL::type_of<std::tuple_element_t<9, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 10: if constexpr (traits::numArgs > 10) return GL::type_of<std::tuple_element_t<10, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 11: if constexpr (traits::numArgs > 11) return GL::type_of<std::tuple_element_t<11, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 12: if constexpr (traits::numArgs > 12) return GL::type_of<std::tuple_element_t<12, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 13: if constexpr (traits::numArgs > 13) return GL::type_of<std::tuple_element_t<13, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 14: if constexpr (traits::numArgs > 14) return GL::type_of<std::tuple_element_t<14, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 15: if constexpr (traits::numArgs > 15) return GL::type_of<std::tuple_element_t<15, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            default:
                return GL::type_of<GL::undefined>();
            }
        };
        GL::type const& returns() const override {
            return GL::type_of<typename traits::returnType>();
        };
    };
    template<typename Ret, typename Class, typename... Param> auto Member_Function_Caller_Impl(Ret(Class::* f)(Param...) const) {
        using Type = decltype(Const_Member_Function_Caller(std::move(f)));
        return GL::make_shared<Type>(std::move(f));
    };
    template<typename Ret, typename Class, typename... Param> auto Member_Function_Caller_Impl(Ret(Class::* f)(Param...)) {
        using Type = decltype(Member_Function_Caller(std::move(f)));
        return GL::make_shared<Type>(std::move(f));
    };
    template <typename Function> class Const_Attribute_Function_Caller final : public Function_Caller {
    public:
        using traits = decltype(Attribute_Function_Traits(std::declval< Function>()));
        Const_Attribute_Function_Caller(Function&& Func) {
            (void)Attribute_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
        };
        virtual ~Const_Attribute_Function_Caller() = default;
        static typename traits::returnType& call(const any* begin) {
            return traits::call(begin);
        };
        void call(const any* const begin, any* const out) const override {
            if (out) {
                if constexpr (std::is_same_v<GL::any, std::decay_t<typename traits::returnType>> || std::is_same_v<any, std::decay_t<typename traits::returnType>>) {
                    *out = call(begin);
                    if (begin->m_type.is_const()) *out |= (GL::type::Const | GL::type::Reference);
                    else *out |= GL::type::Reference;
                }
                else if constexpr (std::is_pointer<typename traits::returnType>::value) {
                    *out = any::wrap_member(*begin, *call(begin));
                    if (begin->m_type.is_const()) *out |= GL::type::Const;
                }
                else {
                    *out = any::wrap_member(*begin, call(begin));
                    if (begin->m_type.is_const()) *out |= GL::type::Const;
                }
            }
        };
        size_t num_arguments() const override {
            return 1;
        };
        GL::type const& argument(size_t index) const override {
            if (index == 0) return GL::type_of<typename traits::classType const&>();
            else return GL::type_of<GL::undefined>();
        };
        GL::type const& returns() const override {
            return GL::type_of<typename traits::returnType const&>();
        };
    };
    template <typename Function> class Attribute_Function_Caller final : public Function_Caller {
    public:
        using traits = decltype(Attribute_Function_Traits(std::declval< Function>()));
        Attribute_Function_Caller(Function&& Func) {
            (void)Attribute_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
        };
        virtual ~Attribute_Function_Caller() = default;
        static typename traits::returnType& call(const any* begin) {
            return traits::call(begin);
        };
        void call(const any* const begin, any* const out) const override {
            if (out) {
                if constexpr (std::is_same_v<GL::any, std::decay_t<typename traits::returnType>> || std::is_same_v<any, std::decay_t<typename traits::returnType>>) {
                    *out = call(begin);
                    if (begin->m_type.is_const()) *out |= (GL::type::Const | GL::type::Reference);                    
                    else *out |= GL::type::Reference;                    
                }
                else if constexpr (std::is_pointer<typename traits::returnType>::value) {
                    *out = any::wrap_member(*begin, *call(begin));
                    if (begin->m_type.is_const()) *out |= GL::type::Const;                    
                }
                else {
                    *out = any::wrap_member(*begin, call(begin));
                    if (begin->m_type.is_const()) *out |= GL::type::Const;                    
                }
            }
        };
        size_t num_arguments() const override {
            return 1;
        };
        GL::type const& argument(size_t index) const override {
            if (index == 0) return GL::type_of<typename traits::classType&>();
            else return GL::type_of<GL::undefined>();
        };
        GL::type const& returns() const override {
            return GL::type_of<typename traits::returnType&>();
        };
    };
    template <typename Function> class Static_Function_Caller final : public Function_Caller {
    public:
        using traits = decltype(Static_Function_Traits(std::declval<Function>()));
        Static_Function_Caller(Function&& Func) {
            (void)Static_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
        };
        virtual ~Static_Function_Caller() = default;

        static typename traits::returnType call(const any* begin) {
            return traits::call(begin);
        };
        void call(const any* const begin, any* const out) const override {
            if constexpr (std::is_same_v<void, typename traits::returnType>) {
                call(begin);
            }
            else {
                if (out) {
#if 1
                    if ((out->m_uuid & 0x1000'0000) == 0){
                        if ((out->m_uuid & uuid::INV_FLAGS) > 0) {
                            auto& ref = uuid::get_uuid(out->m_uuid);
                            if (GL::interlocked::decrement(ref.count) == 0) {
                                ref.count = 1;
                                ref.data = GL::type_erasure::wrap(call(begin));
                                out->m_ptr = ref.data->get();
                                out->m_type = GL::type_of<typename traits::returnType>();
                            }
                            else {
                                out->m_uuid = 0;
                                *out = any::instance(call(begin));
                            }
                        }
                        else {
                            *out = any::instance(call(begin));
                        }
                    }
                    else {
                        *out = any::instance(call(begin));
                    }
#else
                    *out = any::instance(call(begin));
#endif

                }
                else {
                    call(begin);
    }
}
        };
        size_t num_arguments() const override {
            return traits::numArgs;
        };
        GL::type const& argument(size_t index) const override {
            switch (index) {
            case 0: if constexpr (traits::numArgs > 0) return GL::type_of<std::tuple_element_t<0, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 1: if constexpr (traits::numArgs > 1) return GL::type_of<std::tuple_element_t<1, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 2: if constexpr (traits::numArgs > 2) return GL::type_of<std::tuple_element_t<2, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 3: if constexpr (traits::numArgs > 3) return GL::type_of<std::tuple_element_t<3, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 4: if constexpr (traits::numArgs > 4) return GL::type_of<std::tuple_element_t<4, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 5: if constexpr (traits::numArgs > 5) return GL::type_of<std::tuple_element_t<5, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 6: if constexpr (traits::numArgs > 6) return GL::type_of<std::tuple_element_t<6, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 7: if constexpr (traits::numArgs > 7) return GL::type_of<std::tuple_element_t<7, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 8: if constexpr (traits::numArgs > 8) return GL::type_of<std::tuple_element_t<8, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 9: if constexpr (traits::numArgs > 9) return GL::type_of<std::tuple_element_t<9, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 10: if constexpr (traits::numArgs > 10) return GL::type_of<std::tuple_element_t<10, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 11: if constexpr (traits::numArgs > 11) return GL::type_of<std::tuple_element_t<11, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 12: if constexpr (traits::numArgs > 12) return GL::type_of<std::tuple_element_t<12, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 13: if constexpr (traits::numArgs > 13) return GL::type_of<std::tuple_element_t<13, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 14: if constexpr (traits::numArgs > 14) return GL::type_of<std::tuple_element_t<14, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 15: if constexpr (traits::numArgs > 15) return GL::type_of<std::tuple_element_t<15, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            default:
                return GL::type_of<GL::undefined>();
            }
        };
        GL::type const& returns() const override {
            return GL::type_of<typename traits::returnType>();
        };
    };
    /* Converts to a static function caller if the lambda does not capture. Otherwise, uses the local function copy. */
    template <typename Function> class Lambda_Function_Caller final : public Function_Caller {
    private:
        Function func;
    public:
        using traits = typename Lambda_Function_Traits< Function >;
        Lambda_Function_Caller(Function&& Func) : func(std::forward<Function>(Func)) {
            if constexpr (std::is_empty_v< Function >) {
                (void)Static_Function_Traits(+Func); // instantiate the static function pointer
            }
        };
        virtual ~Lambda_Function_Caller() = default;
        typename traits::returnType call(const any* begin) const {
            if constexpr (std::is_empty_v< Function >) {
                using static_traits = decltype(Static_Function_Traits(+std::declval<Function>()));
                return static_traits::call(begin);
            }
            else {
                return Lambda_Function_Traits< Function >::call(begin, func);
            }
        };
        void call(const any* const begin, any* const out) const override {
            if constexpr (std::is_same_v<void, typename traits::returnType>) {
                call(begin);
            }
            else {
                if (out) {
#if 1
                    if ((out->m_uuid & 0x1000'0000) == 0) {
                        if ((out->m_uuid & uuid::INV_FLAGS) > 0) {
                            auto& ref = uuid::get_uuid(out->m_uuid);
                            if (GL::interlocked::decrement(ref.count) == 0) {
                                ref.count = 1;
                                ref.data = GL::type_erasure::wrap(call(begin));
                                out->m_ptr = ref.data->get();
                                out->m_type = GL::type_of<typename traits::returnType>();
                            }
                            else {
                                out->m_uuid = 0;
                                *out = any::instance(call(begin));
                            }
                        }
                        else {
                            *out = any::instance(call(begin));
                        }
                    }
                    else {
                        *out = any::instance(call(begin));
                    }
#else
                    * out = any::instance(call(begin));
#endif
                }
                else {
                    call(begin);
                }
            }
        };
        size_t num_arguments() const override {
            return traits::numArgs;
        };
        GL::type const& argument(size_t index) const override {
            switch (index) {
            case 0: if constexpr (traits::numArgs > 0) return GL::type_of<std::tuple_element_t<0, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 1: if constexpr (traits::numArgs > 1) return GL::type_of<std::tuple_element_t<1, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 2: if constexpr (traits::numArgs > 2) return GL::type_of<std::tuple_element_t<2, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 3: if constexpr (traits::numArgs > 3) return GL::type_of<std::tuple_element_t<3, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 4: if constexpr (traits::numArgs > 4) return GL::type_of<std::tuple_element_t<4, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 5: if constexpr (traits::numArgs > 5) return GL::type_of<std::tuple_element_t<5, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 6: if constexpr (traits::numArgs > 6) return GL::type_of<std::tuple_element_t<6, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 7: if constexpr (traits::numArgs > 7) return GL::type_of<std::tuple_element_t<7, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 8: if constexpr (traits::numArgs > 8) return GL::type_of<std::tuple_element_t<8, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 9: if constexpr (traits::numArgs > 9) return GL::type_of<std::tuple_element_t<9, typename traits::argType>>();
                  else return GL::type_of<GL::undefined>();
            case 10: if constexpr (traits::numArgs > 10) return GL::type_of<std::tuple_element_t<10, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 11: if constexpr (traits::numArgs > 11) return GL::type_of<std::tuple_element_t<11, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 12: if constexpr (traits::numArgs > 12) return GL::type_of<std::tuple_element_t<12, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 13: if constexpr (traits::numArgs > 13) return GL::type_of<std::tuple_element_t<13, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 14: if constexpr (traits::numArgs > 14) return GL::type_of<std::tuple_element_t<14, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            case 15: if constexpr (traits::numArgs > 15) return GL::type_of<std::tuple_element_t<15, typename traits::argType>>();
                   else return GL::type_of<GL::undefined>();
            default:
                return GL::type_of<GL::undefined>();
            }
        };
        GL::type const& returns() const override {
            return GL::type_of<typename traits::returnType>();
        };
    };
};
template <typename Function> GL::shared_ptr<Function_Caller> make_callable(Function&& func) {
    typedef decltype(GL::details::detail::function_signature(func)) function_header;
    if constexpr (function_header::is_object) { // function objects, e.g. auto x = [](){};
        if constexpr (std::is_empty_v< Function >) {
            using Type = decltype(Static_Function_Caller(+std::move(func)));
            return GL::make_shared<Type>(+std::move(func));
        }
        else {
            using Type = decltype(Lambda_Function_Caller(std::move(func)));
            return GL::make_shared<Type>(std::move(func));
        }
    }
    else if constexpr (function_header::is_member_object) { // member objects, e.g. return object.member;    
        using Type = decltype(Attribute_Function_Caller(std::move(func)));
        return GL::make_shared<Type>(std::move(func));
    }
    else if constexpr (function_header::is_member && !function_header::is_member_object) { // member functions, e.g. return object.member();
        return Member_Function_Caller_Impl(std::move(func));
    }
    else if constexpr (function_header::is_static_member_function) { // static function pointers, e.g. static foo(){};   
        using Type = decltype(Static_Function_Caller(std::move(func)));
        return GL::make_shared<Type>(std::move(func));
    }
    else {
        throw std::runtime_error("Did not handle conversion of provided function.");
    }
};

#if 0
// Thread-safe access to a cache of data. Multiple caches are provided that are all, simultanously 
template <typename T> class CategoricalCache {
private: // CacheVersion -> CacheCategory -> Inputs -> Result
    std::atomic<size_t> 
        cache_position{ 0 };
    GL::atomic_shared_ptr<T> 
        current_cache{ nullptr };

public:
    CategoricalCache() = default;
    CategoricalCache(CategoricalCache const&) = delete;
    CategoricalCache(CategoricalCache&&) = delete;
    CategoricalCache& operator=(CategoricalCache const&) = delete;
    CategoricalCache& operator=(CategoricalCache&&) = delete;
    ~CategoricalCache() = default;

    void clear() { };

    // Insert an item into the cache, only if it does not yet exist. If the insert succeeds, it will also remove the older caches. 
    __declspec(noinline) void insert(size_t cache_version, GL::shared_ptr<T> result) {
        while (true) {
            auto old_pos = cache_position.load();
            if (old_pos <= result) return;

            if (cache_position.compare_exchange_strong(old_pos, cache_version)) {

            }


        }
    };



    //// Try to find an item from the cache.
    //GL::shared_ptr<T> try_at(size_t cache_version) {
    //    GL::shared_ptr<T> out;        
    //    auto locked = _current_cache.lock_shared();
    //    _current_cache.do_at_end([&cache_version, &out](size_t const& Key, GL::shared_ptr<T> const& Value) {
    //        if (Key >= cache_version) {
    //            out = Value;
    //        }
    //    });
    //    return out;
    //};

    //// get or make the new ptr
    //GL::shared_ptr<T> at(size_t cache_version) {
    //    while (true) {
    //        if (GL::shared_ptr<T> out = try_at(cache_version); out) 
    //            return out;            
    //        else 
    //            insert(cache_version, GL::make_shared<T>());            
    //    }
    //};

    //// get or make the new ptr
    //template<typename F, typename... Args> GL::shared_ptr<T> at(size_t cache_version, F&& constructor, Args&&... arguments) {
    //    while (true) {
    //        if (GL::shared_ptr<T> out = try_at(cache_version); out) 
    //            return out;            
    //        else 
    //            insert(cache_version, GL::make_shared<T>(constructor(std::forward<Args>(arguments)...)));            
    //    }
    //};

    //// get the most current value
    //GL::shared_ptr<T> current() {
    //    GL::shared_ptr<T> out;
    //    auto locked = _current_cache.lock_shared();
    //    _current_cache.do_at_end([&out](size_t const& Key, GL::shared_ptr<T> const& Value) {
    //        out = Value;
    //    });
    //    return out;
    //};
};
#endif

class Function {
public:
    const GL::shared_ptr<Function_Caller> ptr;
    GL::string name;
    const size_t num_arguments;
    std::array<GL::type, 16> arguments;
    const GL::type* returns;
    std::array<any, 16> defaults;

    Function(GL::shared_ptr<Function_Caller>&& rhs)
        : ptr{ std::forward<GL::shared_ptr<Function_Caller>>(rhs) }
        , name{ "undeclared" }
        , num_arguments{ ptr->num_arguments() }
        , arguments{}
        , returns{ &ptr->returns() }
        , defaults{}
    {
        auto actual_args = ptr->arguments();
        for (int i = 0; i < 16; ++i)
            if (actual_args[i]) arguments[i] = *actual_args[i];
            else arguments[i] = GL::type_of<GL::undefined>();

    };

private:
    GL::string returns_string() const {
        if (returns) return returns->name();
        return "";
    };
    GL::string argument_string(size_t index) const {
        if (arguments[index] == GL::type_of<GL::undefined>()) return GL::string::empty_string();
        else if (defaults[index]) {
            return arguments[index].name() + " = {}"; /// \todo
        }
        else {
            return arguments[index].name();
        }
    };
    GL::string arguments_string() const {
        GL::string out;
        for (size_t i = 0; i < num_arguments; ++i)
            out = out.add_to_delim(argument_string(i), ", ");
        return out;
    };

public:
    GL::string to_string() const {
        return (returns_string() + " " + name + "(" + arguments_string() + ")").remove_leading_and_trailing(' ');
    };

private:
public:
    // Convenience function that will allow the user to easily call a proxy function with arguments.
    // Will automatically add default parameters if the number of arguments is less than required. 
    // Does NOT handle type-conversions and assumes perfect type matches, INCLUDING WITH THE DEFAULT VALUES.
    template<typename T = void, typename... Args> decltype(auto) do_call(Args&&... args) const {
        unsigned char buf[sizeof(any) * 16];
        int position = 0;
        ([&] { // unwrap the parameter pack and copy the relevant data
            if constexpr (std::is_same_v<decltype(args), any const&>) 
                std::memcpy((&reinterpret_cast<any*>(&buf[0])[position++]), &args, sizeof(any));            
            else if constexpr (std::is_same_v<decltype(args), any&>) 
                std::memcpy((&reinterpret_cast<any*>(&buf[0])[position++]), &args, sizeof(any));            
            else if constexpr (std::is_same_v<decltype(args), any&&>) 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(std::forward<any>(args));            
            else if constexpr (std::is_same_v<std::decay_t<decltype(args)>, any>) 
                std::memcpy((&reinterpret_cast<any*>(&buf[0])[position++]), &args, sizeof(any));            
            else 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(any::instance(std::forward<decltype(args)>(args)));            
        }(), ...);

        for (; position < num_arguments; ++position) 
            std::memcpy((&reinterpret_cast<any*>(&buf[0])[position]), &defaults[position], sizeof(any));
        
        defer(position = 0; ([&] { // unwrap the parameter pack and unload the non-reference data
            if constexpr (std::is_same_v<decltype(args), any const&>) 
                position++;            
            else if constexpr (std::is_same_v<decltype(args), any&>) 
                position++;            
            else if constexpr (std::is_same_v<decltype(args), any&&>) 
                reinterpret_cast<any*>(&buf[0])[position++].~any();            
            else if constexpr (std::is_same_v<std::decay_t<decltype(args)>, any>) 
                position++;            
            else 
                reinterpret_cast<any*>(&buf[0])[position++].~any();            
        }(), ...));
               
        if constexpr (std::is_same_v<void, T>) 
            ptr->call(reinterpret_cast<any*>(&buf[0]), nullptr);        
        else {
            thread_local any out;
            ptr->call(reinterpret_cast<any*>(&buf[0]), &out);
            if constexpr (std::is_same_v<any, T>) return std::move(out);            
            else {
                T to_return = out.cast<T>();
                if ((out.m_uuid & 0x1000'0000) == 0) // if not temporary, then potentially call the destructor                    
                    if ((out.m_uuid & uuid::INV_FLAGS) > 0) {
                        auto& ref = uuid::get_uuid(out.m_uuid);
                        // if (!ref.data->is_pod()) 
                            out = nullptr;                        
                    }                
                return to_return;
            }
        }
    };
    // Convenience function that will allow the user to easily call a proxy function with arguments asynchronously.
    // Will automatically add default parameters if the number of arguments is less than required. 
    // Does NOT handle type-conversions and assumes perfect type matches, INCLUDING WITH THE DEFAULT VALUES.
    template<typename T = void, typename... Args> decltype(auto) async_call(Args&&... args) const {
        unsigned char buf[sizeof(any) * 16];
        int position = 0;
        ([&] { // unwrap the parameter pack and copy the relevant data
            if constexpr (std::is_same_v<decltype(args), any const&>) 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(args);            
            else if constexpr (std::is_same_v<decltype(args), any&>) 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(args);            
            else if constexpr (std::is_same_v<decltype(args), any&&>) 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(std::forward<any>(args));            
            else if constexpr (std::is_same_v<std::decay_t<decltype(args)>, any>) 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(args);            
            else 
                new (&reinterpret_cast<any*>(&buf[0])[position++]) any(any::instance(std::forward<decltype(args)>(args)));            
        }(), ...);
        for (; position < num_arguments; ++position) 
            new (&reinterpret_cast<any*>(&buf[0])[position]) any(defaults[position]);
            
        // copies the buffer directly, and copies the shared_ptr with its normal protections for the underlying function
        return GL::parallel::async([buf, Ptr = (GL::shared_ptr<Function_Caller>)(this->ptr)]() {
            defer(int position = 0; ([&] { // unwrap the parameter pack and unload the non-reference data
                if constexpr (std::is_same_v<decltype(args), any const&>)  // this constexpr if statement forces the unroll to happen without impacting performance 
                    const_cast<any&>(reinterpret_cast<const any*>(&buf[0])[position++]).~any();                
                else 
                    const_cast<any&>(reinterpret_cast<const any*>(&buf[0])[position++]).~any();                
            }(), ...));
            if (!Ptr) throw "Function went out-of-scope";

            if constexpr (std::is_same_v<void, T>) 
                Ptr->call(const_cast<any*>(reinterpret_cast<const any*>(&buf[0])), nullptr);            
            else {
                thread_local any out;
                Ptr->call(const_cast<any*>(reinterpret_cast<const any*>(&buf[0])), &out);
                if constexpr (std::is_same_v<any, T>) 
                    return std::move(out);                
                else {
                    T to_return = out.cast<T>();
                    if ((out.m_uuid & 0x1000'0000) == 0) 
                        // if not temporary, then potentially call the destructor
                        if ((out.m_uuid & uuid::INV_FLAGS) > 0) {
                            auto& ref = uuid::get_uuid(out.m_uuid);
                            // if (!ref.data->is_pod()) 
                                out = nullptr;                            
                        }                    
                    return to_return;
                }
            }            
        });
    };

};

// epoch-based reclamation
namespace ebr {
    template <size_t SZ, size_t BlockSize> struct block;
    // actual element data is located at the start. The inner, hidden data is located at the footer. 
    template <size_t SZ, size_t BlockSize> struct element {
        unsigned char
            data[(((SZ + sizeof(element*) + sizeof(block<SZ, BlockSize>*) + sizeof(long long)) + 15) & ~15) - sizeof(element*) - sizeof(block<SZ, BlockSize>*) - sizeof(long long)]; // wrapped to 16-byte blocks for the entire element_t
        element*
            m_pNext;
        block<SZ, BlockSize>*
            m_block;
        long long
            epoch;
    };
    // collection of elements and footer info.
    template <size_t SZ, size_t BlockSize> struct block {
        element<SZ, BlockSize>
            elements[BlockSize];
        block<SZ, BlockSize>*
            m_pNext;
        unsigned long long
            count_free;
        size_t
            block_position;
        long long
            youngest_epoch;        
        size_t // the thread this block is intended to be assigned to. 
            parent_thread; 
    };

    // Allocator that re-uses entire blocks of memory simultaneously. Each thread uses its own free list.
    // In rare cases, if the entire block is not free'd, the memory cannot be re-used. 
    // Handles the edge-case of the destruction of a thread without releasing it's free list - that list is released to a global list.
    template <typename T, size_t BlockSize = 256> class fast_atomic_allocator {
    private:
        using element_t = element<sizeof(T), BlockSize>;
        using block_t = block<sizeof(T), BlockSize>;

        static block_t* PushBlock() {
            block_t* p = reinterpret_cast<block_t*>(GL::malloc(sizeof(block_t)));
            if (p) std::memset(p, 0, sizeof(block_t));
            return p;
        };
        static void PopBlock(block_t* p) {
            GL::mfree(p);
        };

        // Allocate one new block of contiguous elements. These elements will be unique to this thread and unaccessible elsewhere. 
        void AllocBlock() {
            block_t* new_block_ptr = PushBlock();
            blocks.get_or_make(new_block_ptr->block_position = blocks_tickets.get_ticket()) = new_block_ptr;
            block_t& block = *new_block_ptr;
            new_block_ptr->parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[i].m_block = new_block_ptr;
            }
            block.elements[BlockSize - 1].m_pNext = nullptr;
            block.elements[BlockSize - 1].m_block = new_block_ptr;
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Allocate one old block of contiguous elements back onto the free list
        void ReallocBlock(block_t* existing_block_ptr) {
            block_t& block = *existing_block_ptr;
            block.parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].epoch = 0;
                block.elements[i].m_pNext = &block.elements[i + 1];
                ((T*)&block.elements[i].data[0])->~T();
            }
            block.elements[BlockSize - 1].epoch = 0;
            block.elements[BlockSize - 1].m_pNext = nullptr;
            ((T*)&block.elements[BlockSize - 1].data[0])->~T();
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Release memory held by this block
        void ReleaseBlock(block_t* ptr) noexcept {            
            if constexpr (!std::is_pod_v<T>) {
                if (ptr) {
                    for (int element_i = 0; element_i < BlockSize; ++element_i) {
                        auto& element = ptr->elements[element_i];
                        if (element.epoch > 0) {
                            reinterpret_cast<T*>(&element.data[0])->~T();
                            element.epoch = 0;
                        }
                    }
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
            else {
                if (ptr) {
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
        };

        // Release all memory held by all blocks
        void ReleaseBlocks() noexcept {
            for (block_t*& ptr : blocks) {
                if (ptr) {
                    if constexpr (!std::is_pod_v<T>) {
                        for (int element_i = 0; element_i < BlockSize; ++element_i) {
                            auto& element = ptr->elements[element_i];
                            if (element.epoch > 0) {
                                reinterpret_cast<T*>(&element.data[0])->~T();
                                element.epoch = 0;
                            }
                        }
                    }
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                    ptr = nullptr;
                }
            }
        };

    public:
        fast_atomic_allocator()
            : blocks()
            , blocks_tickets()
            , m_free()
            , count_allocated()
        {
            const_cast<size_t&>(count_allocated._default) = 0;            
            const_cast<element_t*&>(m_free._default) = nullptr;
            m_free._before_destruction = [this](element_t*& old_thread) {
                std::set<block_t*> blockss;
                element_t* element{ nullptr };
                while (old_thread) {
                    element = old_thread;
                    old_thread = element->m_pNext;
                    blockss.insert(element->m_block);
                }
                for (auto& x : blockss)
                    ReleaseBlock(x);
            };
        };
        fast_atomic_allocator(fast_atomic_allocator const&) = delete;
        fast_atomic_allocator(fast_atomic_allocator&&) = delete;
        fast_atomic_allocator& operator=(fast_atomic_allocator const&) = delete;
        fast_atomic_allocator& operator=(fast_atomic_allocator&&) = delete;
        ~fast_atomic_allocator() noexcept {
            m_free._before_destruction = nullptr;
            ReleaseBlocks();
        };

        // Acquire a new element from the Free list and construct it.
        template <typename... TArgs> __declspec(noinline) T* Alloc(TArgs &&... a) {
            element_t* element{ nullptr };
            element_t*& free{ *m_free };
            while (1) {
                if (element = free) {
                    free = element->m_pNext;
                    element->epoch = 1ll; // std::numeric_limits<long long>::max(); // indicates it's been initiated
                    T* data{ (T*)&element->data[0] };
                    if constexpr (std::is_pod<T>::value) {
                        if constexpr (sizeof...(a) > 0) {
                            new (data) T(std::forward<TArgs>(a)...);
                        }
                        else {
                            std::memset(data, 0, sizeof(T));
                        }
                    }
                    else {
                        new (data) T(std::forward<TArgs>(a)...);
                    }
                    return data;
                }                
                else {
                    AllocBlock();
                }
            }
        };

        // Destroys the element and return its memory to the Free list
        void Free(T* element) {
            element_t* t = (element_t*)(element);      
            // GL::interlocked::compare_exchange(t->epoch, 1ll, GL::util::get_current_epoch());
            t->epoch = GL::util::get_current_epoch();    
                        
            if (GL::interlocked::decrement(t->m_block->count_free) == 0) {
                auto* Where = &count_allocated[t->m_block->parent_thread];
                while (Where) {
                    auto old = *Where;
                    if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                        old = *count_allocated;
                        if ((old / BlockSize) > 10) {
                            ReleaseBlock(t->m_block);
                        }
                        else {
                            ReallocBlock(t->m_block);
                        }
                        break;
                    }
                }
            }
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

    private:
        GL::atomic_vector<block_t*>
            blocks; // vector of all blocks. May or may not be nullptr. 
        GL::ticket_dispensor<false>
            blocks_tickets; // ticket dispensor to re-use blocks indexes and minimize the size of blocks
        GL::thread_object<element_t*>
            m_free; // since each thread is guarranteed to access their free-list without conflict, it does not need to be managed by an aba-protector.
        GL::thread_object<size_t>
            count_allocated; // this exists as part of a fix for thread_local allocations being free-d on other threads (e.g. consumer-producer pattern). 
    };

    // Allocator that re-uses entire blocks of memory simultaneously. Each thread uses its own free list.
    // In rare cases, if the entire block is not free'd, the memory cannot be re-used. 
    // Handles the edge-case of the destruction of a thread without releasing it's free list - that list is released to a global list.
    template <typename T, size_t BlockSize = 256> class fast_atomic_epoch_allocator {
    private:
        using element_t = element<sizeof(T), BlockSize>;
        using block_t = block<sizeof(T), BlockSize>;

        static block_t* PushBlock() {
            block_t* p = reinterpret_cast<block_t*>(GL::malloc(sizeof(block_t)));
            if (p) std::memset(p, 0, sizeof(block_t));
            return p;
        };
        static void PopBlock(block_t* p) {
            GL::mfree(p);
        };

        // Allocate one new block of contiguous elements onto the free list
        void AllocBlock() {
            block_t* new_block_ptr = PushBlock();
            blocks.get_or_make(new_block_ptr->block_position = blocks_tickets.get_ticket()) = new_block_ptr;
            block_t& block = *new_block_ptr;
            new_block_ptr->parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[i].m_block = new_block_ptr;
            }
            block.elements[BlockSize - 1].m_pNext = nullptr;
            block.elements[BlockSize - 1].m_block = new_block_ptr;
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Allocate one old block of contiguous elements back onto the free list. This requires that it has been retired and is safe to reclaim. 
        void ReallocBlock(block_t* existing_block_ptr) {
            block_t& block = *existing_block_ptr;
            block.parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].epoch = 0;
                block.elements[i].m_pNext = &block.elements[i + 1];
                ((T*)&block.elements[i].data[0])->~T();
            }
            block.elements[BlockSize - 1].epoch = 0;
            block.elements[BlockSize - 1].m_pNext = nullptr;
            ((T*)&block.elements[BlockSize - 1].data[0])->~T();
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Release memory held by this block
        void ReleaseBlock(block_t* ptr) noexcept {
            if constexpr (!std::is_pod_v<T>) {
                if (ptr) {
                    for (int element_i = 0; element_i < BlockSize; ++element_i) {
                        auto& element = ptr->elements[element_i];
                        if (element.epoch > 0) {
                            reinterpret_cast<T*>(&element.data[0])->~T();
                            element.epoch = 0;
                        }
                    }
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
            else {
                if (ptr) {
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
        };

        enum class ReclamationResult {
            NoRetiredBlocks,
            FailedToReclaimAnyRetiredBlocks,
            ReclaimedRetiredBlocks
        };
        ReclamationResult TryReclaimRetiredBlocks() {
            block_t* block{ nullptr };
            ReclamationResult out = ReclamationResult::NoRetiredBlocks;
            bool failed_local = false;
            bool failed_global = false;
            auto& local_q = *retired_blocks;
            int allowed_repeatitions = 1;
            while (true) {
                if (!failed_local && (local_q.size() > 0)) {
                    block = local_q.top();
                    if ((block->youngest_epoch + 3) < this->current_epoch) {
                        local_q.pop();
                        if (out != ReclamationResult::ReclaimedRetiredBlocks || (--allowed_repeatitions > 0)) {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    if ((old / BlockSize) > 10) {
                                        ReleaseBlock(block);
                                    }
                                    else {
                                        ReallocBlock(block);
                                        out = ReclamationResult::ReclaimedRetiredBlocks;
                                    }
                                    break;
                                }
                            }                            
                        }
                        else {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    ReleaseBlock(block);
                                    break;
                                }
                            }
                        }                        
                    }
                    else {
                        if (out == ReclamationResult::NoRetiredBlocks) out = ReclamationResult::FailedToReclaimAnyRetiredBlocks;
                        failed_local = true;
                    }
                }
                else if (!failed_global && global_retired_blocks.try_pop(block)) {                    
                    if ((block->youngest_epoch + 3) < this->current_epoch) {
                        if (out != ReclamationResult::ReclaimedRetiredBlocks || (--allowed_repeatitions > 0)) {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    if ((old / BlockSize) > 10) {
                                        ReleaseBlock(block);
                                    }
                                    else {
                                        ReallocBlock(block);
                                        out = ReclamationResult::ReclaimedRetiredBlocks;
                                    }
                                    break;
                                }
                            }                                                        
                        }
                        else {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    ReleaseBlock(block);
                                    break;
                                }
                            }
                        }
                    }
                    else {
                        if (out == ReclamationResult::NoRetiredBlocks) out = ReclamationResult::FailedToReclaimAnyRetiredBlocks;
                        local_q.push(block);
                        failed_global = true;
                    }
                }
                else {
                    return out;
                }
            }
        };

        // Release all memory held by all blocks
        void ReleaseBlocks() noexcept {            
            for (block_t*& ptr : blocks) {
                if (ptr) {
                    if constexpr (!std::is_pod_v<T>) {
                        for (int element_i = 0; element_i < BlockSize; ++element_i) {
                            auto& element = ptr->elements[element_i];
                            if (element.epoch > 0) {
                                reinterpret_cast<T*>(&element.data[0])->~T();
                                element.epoch = 0;
                            }
                        }
                    }
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                    ptr = nullptr;
                }
            }              
        };

        class ThreadState {
        public:
            mutable long long
                epoch_2,
                epoch_1,
                epoch;
            mutable size_t
                epoch_depth;
            mutable long
                epoch_protected{ 0 };
            mutable long long
                queued_epoch;
            mutable long long
                delete_if_older_than;
            fast_atomic_epoch_allocator*
                parent;
            mutable int 
                deferrment{ 0 };

            __declspec(noinline) void enter_critical_section() const {
                if (++epoch_depth == 1) {
                    GL::interlocked::exchange(epoch_protected, 1l);
                    queued_epoch = parent->current_epoch + 1;
                    if (deferrment <= 0) deferrment = 1'000;
                }
            };
            __declspec(noinline) void exit_critical_section() const {
                if (--epoch_depth == 0) {
                    GL::interlocked::exchange(epoch_protected, 0l);

                    // from the perspective of this thread, we are now OK to free pointers older than "epoch";
                    GL::interlocked::exchange(delete_if_older_than, epoch);
                    epoch = epoch_1;
                    epoch_1 = epoch_2;
                    epoch_2 = queued_epoch;

                    if (--deferrment == 0) {
                        // review the main thread to update the epoch number
                        long long old_epoch = parent->current_epoch;
                        long long currentEpoch = std::numeric_limits<long long>::max();
                        parent->states.for_each([&currentEpoch](auto& state) {
                            if (state.epoch_protected) {
                                currentEpoch = std::min<long long>(currentEpoch, state.delete_if_older_than);
                            }
                        });
                        if (old_epoch != currentEpoch) {                            
                            if (GL::interlocked::compare_exchange(parent->current_epoch, old_epoch, currentEpoch)) {
                                // std::cout << GL::printf("Updating the Current Epoch to %zu\n", (size_t)currentEpoch);
                            }
                        }
                    }                
                }
            };
            auto guard_critical_section() const {
                class wrap {
                    const ThreadState* p;
                public:
                    wrap(const ThreadState* P) : p{ P } {};
                    wrap(wrap const&) = delete;
                    wrap(wrap&&) = delete;
                    wrap& operator=(wrap const&) = delete;
                    wrap& operator=(wrap&&) = delete;
                    ~wrap() {
                        p->exit_critical_section();
                    };
                };
                enter_critical_section();
                return wrap(this);
            };

            ThreadState() noexcept :
                epoch_2{ 0/*GL::util::get_current_epoch()*/ },
                epoch_1{ 0/*GL::util::get_current_epoch()*/ },
                epoch{ 0/*GL::util::get_current_epoch()*/ },
                delete_if_older_than{ 0/*GL::util::get_current_epoch()*/ },
                epoch_depth{ 0ull },
                queued_epoch{ 0/*GL::util::get_current_epoch()*/ },
                parent{ nullptr }
            {};
            ThreadState(ThreadState const&) = default;
            ThreadState(ThreadState&&) noexcept = default;
            ThreadState& operator=(ThreadState const&) = default;
            ThreadState& operator=(ThreadState&&) noexcept = default;
            ~ThreadState() {};
        };

    public:
        fast_atomic_epoch_allocator()
            : blocks()
            , m_free()
            , global_free{ 0ull }
            , retired_blocks()
            , global_retired_blocks{ 0ull }            
            , current_epoch{ 0 }
            , states()
        {
            m_free._after_construction = [this](element_t*& new_thread) {
                new_thread = nullptr;
            };
            m_free._before_destruction = [this](element_t* old_thread) {
                while (old_thread) {
                    element_t* element{ old_thread };
                    if (element) {
                        old_thread = element->m_pNext;
                        GL::aba_problem::Stack_Push(global_free, element);
                    }
                }
            };
            global_free.m_n64 = 0;
            
            retired_blocks._before_destruction = [this](std::priority_queue<block_t*, std::vector<block_t*>, cmp>& old_thread) {
                while (old_thread.size() > 0) {
                    global_retired_blocks.push(old_thread.top());
                    old_thread.pop();
                }
            };

            states._after_construction = [this](ThreadState& state) {
                state.parent = this;
                state.epoch = state.epoch_1 = state.epoch_2 = state.delete_if_older_than = state.queued_epoch = this->current_epoch;
            };
            states._before_destruction = [this](ThreadState& state) {
                state.epoch_protected = 0;
            };
        };
        fast_atomic_epoch_allocator(fast_atomic_epoch_allocator const&) = delete;
        fast_atomic_epoch_allocator(fast_atomic_epoch_allocator&&) = delete;
        fast_atomic_epoch_allocator& operator=(fast_atomic_epoch_allocator const&) = delete;
        fast_atomic_epoch_allocator& operator=(fast_atomic_epoch_allocator&&) = delete;
        ~fast_atomic_epoch_allocator() noexcept {
            m_free._after_construction = nullptr;
            m_free._before_destruction = nullptr;
            retired_blocks._after_construction = nullptr; 
            retired_blocks._before_destruction = nullptr;
            ReleaseBlocks();
        };

        // Acquire a new element from the Free list and construct it.
        template <typename... TArgs> T* Alloc(TArgs &&... a) {
            element_t* element{ nullptr };
            auto*& freeP = *m_free;
            T* data;
            ReclamationResult result;
            while (1) {                
                if (freeP) {
                    element = freeP;
                    freeP = element->m_pNext;
                }
                else element = GL::aba_problem::Pop(global_free);
                if (element) {
                    element->epoch = 1ll; // std::numeric_limits<long long>::max(); // indicates it's been initiated
                    data = (T*)&element->data[0];
                    if constexpr (std::is_pod<T>::value) {
                        if constexpr (sizeof...(a) > 0) {
                            new (data) T(std::forward<TArgs>(a)...);
                        }
                        else {
                            std::memset(data, 0, sizeof(T));
                        }
                    }
                    else {
                        new (data) T(std::forward<TArgs>(a)...);
                    }
                    return data;
                }
                else {
                    result = TryReclaimRetiredBlocks();
                    if (result != ReclamationResult::ReclaimedRetiredBlocks) {
                        AllocBlock();
                    }                    
                }
            }
        };
        // Destroys the element and return its memory to the Free list
        void Free(T* element) {
            element_t* t = (element_t*)(element);
            t->epoch = current_epoch; // GL::util::get_current_epoch();
            // GL::interlocked::compare_exchange(t->epoch, 1ll, current_epoch);

            if (GL::interlocked::decrement(t->m_block->count_free) == 0) {
                // by definition, the most recent (youngest) epoch will be the one we just did that successfully retired the block...
                t->m_block->youngest_epoch = t->epoch; 

                // queue the retired block
                retired_blocks->push(t->m_block);
            }
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

        typedef typename decltype(GL::details::detail::function_signature(&ThreadState::guard_critical_section))::Return_Type GuardType;
        [[nodiscard]] GuardType guard_critical_section() {
            return states->guard_critical_section();
        };        

    private:
        struct cmp {
            constexpr bool operator()(block_t* const& lhs, block_t* const& rhs) {
                return lhs->youngest_epoch >= rhs->youngest_epoch;
            };
        };
        GL::atomic_vector<block_t*>
            blocks; // vector of all blocks currently allocated and alive
        GL::ticket_dispensor<false>
            blocks_tickets; // ticket dispensor to re-use blocks indexes and minimize the size of blocks
        GL::thread_object_no_default<element_t*>
            m_free; // thread-local free list of elements
        GL::aba_problem::THead<element_t>
            global_free; // shared free list of elements
        GL::thread_object_no_default<std::priority_queue<block_t*, std::vector<block_t*>, cmp>>
            retired_blocks; // retired (but alive) blocks, sorted by their youngest element's epoch. This is the thread-local queue.
        concurrency::concurrent_priority_queue<block_t*, cmp>
            global_retired_blocks; // retired (but alive) blocks, sorted by their youngest element's epoch. This is the shared queue.
        GL::thread_object_no_default< ThreadState >
            states; // thread states. Used to manage the scope guard and lifetime of objects. 
        long long
            current_epoch; // the current epoch that has been reached by the allocator. 
        GL::thread_object<size_t>
            count_allocated; // this exists as part of a fix for thread_local allocations being free-d on other threads (e.g. consumer-producer pattern). 
    };

    // Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
    // Attempts to speed-up searching using a binomial search within BTree nodes. In theory should benefit from larger maxChildrenPerNode values. 
    template< class objType, class keyType, int maxChildrenPerNode = 10>
    class epoch_btree {
    private:
        fast_atomic_allocator< objType >
            objAllocator;

    public:
        using lock_type = GL::fast_shared_mutex; // std::shared_mutex; // fast_shared_mutex; //  
        class epoch_btreeNode {
        public:
            epoch_btreeNode() = default;
            epoch_btreeNode(epoch_btreeNode const&) = delete;
            epoch_btreeNode(epoch_btreeNode&&) = delete;
            epoch_btreeNode& operator=(epoch_btreeNode const&) = delete;
            epoch_btreeNode& operator=(epoch_btreeNode&&) = delete;
            __declspec(noinline) ~epoch_btreeNode() noexcept {
                if (is_leaf()) {
                    father->objAllocator.Free(ptr);
                    ptr = nullptr;
                }
            };

            objType
                * ptr{ nullptr };
            std::array<epoch_btreeNode*, maxChildrenPerNode>
                data;
            epoch_btreeNode // parent node
                * parent{ nullptr };
            epoch_btree
                * father{ nullptr };
            keyType	// key used for sorting						
                key;
            int	// number of children							
                numChildren{ 0 };
            int
                parent_index{ 0 };

            bool
                is_leaf() const {
                return ptr;
            };
            template <typename... Args>
            void instantiate_object(Args&&... args) {
                ptr = father->objAllocator.Alloc(std::move(args)...);
            };
            objType* const&
                object() {
                return ptr;
            };
            epoch_btreeNode**
                children() {
                return &data[0];
            };

            epoch_btreeNode* // next sibling
                next() {
                if (parent && (parent->numChildren > (parent_index + 1))) {
                    return parent->children()[parent_index + 1];
                }
                else {
                    return nullptr;
                }
            };
            epoch_btreeNode* // prev sibling
                prev() {
                if (parent && (parent_index >= 1)) {
                    return parent->children()[parent_index - 1];
                }
                else {
                    return nullptr;
                }
            };
            epoch_btreeNode* // first child
                firstChild() {
                if (numChildren == 0) return nullptr;
                return children()[0];
            };
            epoch_btreeNode* // last child
                lastChild() {
                if (numChildren == 0) return nullptr;
                return children()[numChildren - 1];
            };
            void
                add_child(epoch_btreeNode* p) {
                p->parent = this;
                p->parent_index = numChildren;
                children()[numChildren] = p;
                ++numChildren;
                if (this->key < p->key) this->key = p->key;
            }
            __declspec(noinline) void
                add_child_at(epoch_btreeNode* p, int i) {
                if (i >= numChildren) add_child(p);
                else {
                    epoch_btreeNode**
                        ch = children();
                    int
                        j;

                    if (this->key < p->key) this->key = p->key;
                    p->parent = this;
                    p->parent_index = i;
                    // shift everything forward

    #if 1
                    for (j = numChildren; j > i; --j) {
                        ch[j] = ch[j - 1];
                        ch[j]->parent_index = j;
                    }
                    ch[i] = p;
    #else
                    std::memmove(&ch[i + 1], &ch[i], sizeof(epoch_btreeNode*) * (size_t)(numChildren - i));
                    ch[i] = p;
                    ch = &ch[i];
                    for (j = i + 1; j <= numChildren; ++j) {
                        ++ch;
                        (*ch)->parent_index = j;
                    }
    #endif
                    ++numChildren;
                }
            }
            epoch_btreeNode*
                pop_front_child() {
                if (numChildren <= 0) {
                    return nullptr;
                }
                else {
                    auto* out = children()[0];
                    int i = 0;
                    for (i = 0; i < (numChildren - 1); ++i) {
                        children()[i] = children()[i + 1];
                        children()[i]->parent_index = i;
                    }
                    children()[i] = nullptr;
                    --numChildren;
                    return out;
                }
            }
            void
                pop_front_children(int n) {
                if (numChildren >= n) {
                    int i = 0;
                    for (i = 0; i < (numChildren - n); ++i) {
                        children()[i] = children()[i + n];
                        children()[i]->parent_index = i;
                    }
                    for (; i < maxChildrenPerNode; ++i) {
                        children()[i] = nullptr;
                    }
                    numChildren -= n;
                }
            }
            epoch_btreeNode*
                pop_child(int i) {
                if (numChildren <= 0) {
                    return nullptr;
                }
                else {
                    epoch_btreeNode**
                        ch = children();
                    epoch_btreeNode*
                        out = ch[i];
                    int
                        j = numChildren - 1;

    #if 1
                    for (; i < (numChildren - 1); ++i) {
                        ch[i] = ch[i + 1];
                        ch[i]->parent_index = i;
                    }
                    ch[i] = nullptr;
                    --numChildren;
                    if (numChildren > 0) this->key = ch[numChildren - 1]->key;
    #else
                    std::memmove(&ch[i], &ch[i + 1], sizeof(epoch_btreeNode*) * (size_t)((numChildren - i) - 1));
                    if (numChildren > 1) this->key = ch[numChildren - 2]->key;
                    ch[j] = nullptr;
                    ch = &ch[i];
                    for (; (i < j) && ch; ++i) {
                        (*ch)->parent_index = i;
                        ++ch;
                    }
                    --numChildren;
    #endif
                    return out;
                }
            }
            epoch_btreeNode*
                pop_back_child() {
                if (numChildren <= 0) {
                    return nullptr;
                }
                else {
                    auto* out = children()[numChildren - 1];
                    children()[numChildren - 1] = nullptr;
                    --numChildren;
                    if (numChildren > 0) this->key = children()[numChildren - 1]->key;
                    return out;
                }
            };
            __declspec(noinline) epoch_btreeNode*
                binomial_search_smallest_greater_equal_to(keyType const& K) {
    #if 0
                epoch_btreeNode* child = this->firstChild();
                for (; child->next(); child = child->next()) {
                    if (K <= child->key)
                        break;
                }
                return child;
    #else
                //if (K >= children()[numChildren - 1]->key) {
                //	// it will be one of the final children
                //	for (int i = numChildren - 2; i >= 0; --i) {
                //		if (children()[i]->key < K) return children()[i + 1];
                //	}
                //	// worst-case we searched them all...
                //	return children()[0];
                //}
                //else {
                int
                    len,
                    mid,
                    offset;
                bool
                    res;
                epoch_btreeNode
                    * sample;
                epoch_btreeNode
                    ** childrens;
                if (numChildren == 0)
                    return nullptr;
                if (numChildren == 1)
                    return this->children()[0];

                if (numChildren >= maxChildrenPerNode) {
                    // std::cout << "Something went wrong 1...\n";
                    // throw std::runtime_error(std::to_string(numChildren) + " - Bad index");
                }

                childrens = this->children();
                len = numChildren;
                mid = len;
                offset = 0;
                res = false;

                while (mid > 0) {
                    mid = len >> 1;
                    if (((offset + mid) < 0) || ((offset + mid) >= maxChildrenPerNode)) {
                        // std::cout << "Something went wrong 2...\n";
                        // return binomial_search_smallest_greater_equal_to(K);
                        return nullptr;
                        // throw std::runtime_error(std::to_string(offset + mid) + " - Bad index");
                    }
                    sample = childrens[std::min<int>(numChildren - 1, offset + mid)];
                    if (!sample) {
                        // std::cout << "Something went wrong 3...\n";
                        // return binomial_search_smallest_greater_equal_to(K);
                        return nullptr;
                    }
                    if (K >= sample->key) {
                        offset += mid;
                        len -= mid;
                        res = true;
                        if (K == sample->key) return sample;
                    }
                    else {
                        len -= mid;
                        res = false;
                    }
                }
                mid = offset + (int)res;
                if (mid == numChildren) return childrens[std::min<int>(numChildren - 1, offset)];
                else return childrens[std::min<int>(numChildren - 1, mid)];
                //}
    #endif
            };

        };

    private:
        mutable lock_type
            mut; // global tree lock. Should only be held temporarily if at all possible. 
        epoch_btreeNode*
            root;
        epoch_btreeNode*
            first;
        epoch_btreeNode*
            last;
        fast_atomic_epoch_allocator< epoch_btreeNode >
            nodeAllocator;
        long
            count;
    public:
        class // exclusive lock manager. Since this is a course-grained type, though, it can only ever hold one lock at a time. 
            locker {
        public:
            lock_type*
                locked;
            bool
                hard_locked;

            locker() : locked{ nullptr }, hard_locked{ false } {};
            locker(locker const&) = delete;
            locker(locker&& rhs) noexcept : locked{ rhs.locked }, hard_locked{ rhs.hard_locked } { rhs.locked = nullptr; };
            locker& operator=(locker const&) = delete;
            locker& operator=(locker&& rhs) noexcept {
                clear();
                locked = rhs.locked;
                hard_locked = rhs.hard_locked;
                rhs.locked = nullptr;
                return *this;
            };
            ~locker() {
                clear();
            };
            operator bool() const {
                return locked;
            };

        public:
            __declspec(noinline) bool // store a shared lock
                try_push_back(lock_type& source) {
                clear();
                locked = &source;
                if (locked->try_lock()) {
                    hard_locked = true;
                    return true;
                }
                else {
                    hard_locked = false;
                    locked = nullptr;
                    return false;
                }
            };
            void // store a shared lock
                push_back(lock_type& source) {
                clear();
                locked = &source;
                locked->lock();
                hard_locked = true;
            };
            void // store a shared lock
                push_back_shared(lock_type& source) {
                clear();
                locked = &source;
                locked->lock_shared();
                hard_locked = false;
            };
            __declspec(noinline) bool // store a shared lock
                try_push_back_shared(lock_type& source) {
                clear();
                locked = &source;
                if (locked->try_lock_shared()) {
                    hard_locked = false;
                    return true;
                }
                else {
                    hard_locked = false;
                    locked = nullptr;
                    return false;
                }
            };
            void // store a shared lock
                push_pop(lock_type& source) {
                source.lock();
                clear();
                locked = &source;
                hard_locked = true;
            };
            void // store a shared lock
                push_pop_shared(lock_type& source) {
                source.lock_shared();
                clear();
                locked = &source;
                hard_locked = false;
            };
            void // remove the youngest lock
                pop_back() {
                clear();
            };
            void // remove the oldest lock
                pop_front() {
                clear();
            };
            size_t // count of locks
                size() const {
                return (locked) ? 1 : 0;
            };
            void // clear all locks
                clear() {
                if (locked) {
                    if (hard_locked) locked->unlock();
                    else locked->unlock_shared();
                    locked = nullptr;
                }
            };
        };
    public:
        class EpochGuard {
        private:
            typename typename decltype(nodeAllocator)::GuardType guard_1;

        public:
            EpochGuard(epoch_btree* parent) : guard_1{ parent->nodeAllocator.guard_critical_section() } {};
            EpochGuard(EpochGuard const&) = delete;
            EpochGuard(EpochGuard&& rhs) = delete;
            EpochGuard& operator=(EpochGuard const&) = delete;
            EpochGuard& operator=(EpochGuard&&) = delete;
            ~EpochGuard() = default;
        };

        using GuardType = typename EpochGuard;
        [[nodiscard]] GuardType guard_critical_section() const {
            return EpochGuard(const_cast<epoch_btree*>(this));
        };

        epoch_btree()
            : objAllocator()
            , nodeAllocator()
            , root{ nullptr }
            , first{ nullptr }
            , last{ nullptr }
            , mut()
            , count{ 0 }
        {
            root = AllocNode(false);
        };
        epoch_btree(epoch_btree const&)
            = delete;
        epoch_btree(epoch_btree&&) noexcept
            = delete;
        epoch_btree& operator=(epoch_btree const&)
            = delete;
        epoch_btree& operator=(epoch_btree&&) noexcept
            = delete;
        ~epoch_btree() = default;

        epoch_btreeNode* // add an object to the tree
            GetOrInstance(keyType const& key) {
            if (auto [try_found, locked] = NodeFindSmallestLargerEqual(key, false); try_found) {
                return try_found;
            }
            else {
                epoch_btreeNode
                    * node,
                    * child,
                    * newNode;
                locker
                    locking;
                newNode
                    = AllocNode(true);
                newNode->key
                    = key;
                // newNode->instantiate_object(const_cast<epoch_btree*>(this));

                if (!locking) {
                    locking.push_back(mut); // locked
                }

                if (root == nullptr) root = AllocNode(false); // start fresh
                if (root == nullptr) throw std::runtime_error("Root should not have been nullptr");

                if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
                    node = AllocNode(false);
                    node->key = root->key;
                    node->add_child(root);
                    SplitNode(root);
                    root = node;
                    node = nullptr;
                }

                for (node = root; node->numChildren > 0; node = child) {
                    if (key > node->key) node->key = key; // in prep for the insertion

                    // find the first child with a key larger equal to the key of the new node
                    child = node->binomial_search_smallest_greater_equal_to(key);

                    // we are inside of a branch of leafs -- we will do the insert.
                    if (child->object()) {
                        if (key <= child->key) {
                            if (key == child->key) {
                                // *child->object() = std::move(*newNode->object());
                                FreeNode(newNode);
                                return child;
                            }

                            // insert new node before child
                            newNode->instantiate_object();
                            node->add_child_at(newNode, child->parent_index);
                        }
                        else {
                            // insert new node after child
                            newNode->instantiate_object();
                            node->add_child_at(newNode, child->parent_index + 1);
                        }

                        if (!first || (first->key > newNode->key)) first = newNode;
                        if (!last || (last->key < newNode->key)) last = newNode;

                        ++count;
                        return newNode;
                    }
                    else if (child->numChildren >= maxChildrenPerNode) {
                        SplitNode(child);
                        if (key <= child->prev()->key) child = child->prev();
                    }
                }

                // we only end up here if the root node is empty
                newNode->instantiate_object();
                root->add_child(newNode);

                if (!first || (first->key > newNode->key)) first = newNode;
                if (!last || (last->key < newNode->key)) last = newNode;

                ++count;
                return newNode;
            }
        };
        objType& operator[](keyType const& key) {
            return *GetOrInstance(key)->object();
        };

        __declspec(noinline) epoch_btreeNode* // add an object to the tree
            Add(objType&& object, keyType const& key, locker const& Locking = locker(), bool unique = true) {
            epoch_btreeNode
                * node,
                * child,
                * newNode;
            locker&
                locking = const_cast<locker&>(Locking);
            newNode
                = AllocNode(true);
            newNode->key
                = key;
            newNode->instantiate_object(std::move(object));

            if (!locking) {
                locking.push_back(mut); // locked
            }

            if (root == nullptr) root = AllocNode(false); // start fresh
            if (root == nullptr) throw std::runtime_error("Root should not have been nullptr");

            if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
                node = AllocNode(false);
                node->key = root->key;
                node->add_child(root);
                SplitNode(root);
                root = node;
                node = nullptr;
            }

            for (node = root; node->numChildren > 0; node = child) {
                if (key > node->key) node->key = key; // in prep for the insertion

                // find the first child with a key larger equal to the key of the new node
                child = node->binomial_search_smallest_greater_equal_to(key);

                // we are inside of a branch of leafs -- we will do the insert.
                if (child->object()) {
                    if (key <= child->key) {
                        if (unique && (key == child->key)) {
                            // *child->object() = std::move(*newNode->object());
                            FreeNode(newNode);
                            return child;
                        }

                        // insert new node before child
                        node->add_child_at(newNode, child->parent_index);
                    }
                    else {
                        // insert new node after child
                        node->add_child_at(newNode, child->parent_index + 1);
                    }

                    if (!first || (first->key > newNode->key)) first = newNode;
                    if (!last || (last->key < newNode->key)) last = newNode;

                    ++count;
                    return newNode;
                }
                else if (child->numChildren >= maxChildrenPerNode) {
                    SplitNode(child);
                    if (key <= child->prev()->key) child = child->prev();
                }
            }

            // we only end up here if the root node is empty
            root->add_child(newNode);

            if (!first || (first->key > newNode->key)) first = newNode;
            if (!last || (last->key < newNode->key)) last = newNode;

            ++count;
            return newNode;

        };
        bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
            Remove(epoch_btreeNode* node, locker const& Locking = locker()) {
            epoch_btreeNode
                * Node,
                * parent,
                * oldRoot;
            locker&
                locking = const_cast<locker&>(Locking);

            // acquire all relevant locks before we perform the deletion
            if (locking.size() == 0) locking.push_back(mut); // get the global tree lock		

            if (first == node) first = GetNextLeaf(node, locking);
            if (last == node) last = GetPrevLeaf(node, locking);

            // unlink the node from it's parent
            parent = node->parent;
            parent->pop_child(node->parent_index);

            // make sure there are no parent nodes with a single child
            for (; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
                while (true) {
                    if (Node = parent->next()) {
                        if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
                            SplitNode(Node);
                            continue;
                        }
                        parent = MergeNodes(parent, Node);
                        break;
                    }
                    else if (Node = parent->prev()) {
                        if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
                            SplitNode(Node);
                            continue;
                        }
                        parent = MergeNodes(Node, parent);
                        break;
                    }
                }

                if (parent->numChildren > maxChildrenPerNode) {
                    SplitNode(parent);
                    break;
                }
            }

            // a parent may not use a key higher than the key of it's last child. Work backwards and make sure this is true. 
            for (; parent && (parent->numChildren > 0); parent = parent->parent)
                if (Node = parent->children()[parent->numChildren - 1])
                    if (parent->key > Node->key)
                        parent->key = Node->key;

            // actually free the node
            --count;
            FreeNode(node);

            // remove the root node if it has a single internal node as child		
            if ((root->numChildren == 1) && !root->firstChild()->object()) {
                oldRoot = root;

                root = oldRoot->firstChild();
                root->parent = nullptr;
                root->parent_index = 0;

                FreeNode(oldRoot);
            }

            return true;
        };
        std::pair<epoch_btreeNode*, locker> // find an object using the given key
            NodeFind(keyType const& key, bool for_removal = false) {
            std::pair<epoch_btreeNode*, locker>
                out;
            epoch_btreeNode*&
                node = out.first;
            locker&
                locking = out.second;

            if (!for_removal) locking.push_back_shared(mut);
            else locking.push_back(mut);
            if (!root || (root->numChildren <= 0)) {
                node = nullptr;
                locking.clear();
                return out;
            }

            for (node = root; node; ) {
                node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                if (node && node->object()) {
                    if (node->key == key) return out;
                    else {
                        node = nullptr;
                        locking.clear();
                        return out;
                    }
                }
                if (!node || (node->numChildren <= 0)) {
                    node = nullptr;
                    locking.clear();
                    return out;
                }
            }
            node = nullptr;
            locking.clear();
            return out;
        };
        std::pair<epoch_btreeNode*, locker> // find an object using the given key
            NodeFind_ForRemoval(keyType const& key) {
            return NodeFind(key, true);
        };
        locker
            try_lock() {
            locker out;
            out.try_push_back(mut);
            return out;
        };
        locker
            lock(bool do_hard_lock = true) {
            locker out;
            if (do_hard_lock) out.push_back(mut);
            else out.push_back_shared(mut);
            return out;
        };
        locker
            lock_shared() {
            locker out;
            out.push_back_shared(mut);
            return out;
        };
        locker
            try_lock_shared() {
            locker out;
            out.try_push_back_shared(mut);
            return out;
        };

        std::pair<epoch_btreeNode*, locker> // find an object with the smallest key larger equal the given key
            NodeFindSmallestLargerEqual(keyType const& key, bool for_removal = false) {
            std::pair<epoch_btreeNode*, locker>
                out;
            epoch_btreeNode*&
                node = out.first;
            locker&
                locking = out.second;

            if (!for_removal) locking.push_back_shared(mut);
            else locking.push_back(mut);
            if (!root || (root->numChildren <= 0)) {
                node = nullptr;
                locking.clear();
                return out;
            }
            for (node = root; node; ) {
                node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                if (node && node->object()) {
                    if (node->key >= key) return out;
                    else {
                        node = nullptr;
                        locking.clear();
                        return out;
                    }
                }
                if (!node || (node->numChildren <= 0)) {
                    node = nullptr;
                    locking.clear();
                    return out;
                }
            }
            node = nullptr;
            locking.clear();
            return out;
        };
        epoch_btreeNode* // find an object with the smallest key larger equal the given key
            NodeFindSmallestLargerEqual_Locked(keyType const& key, locker const& locked) {
            epoch_btreeNode*
                node = nullptr;

            if (!root || (root->numChildren <= 0)) {
                node = nullptr;
                return node;
            }
            for (node = root; node; ) {
                node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                if (node && node->object()) {
                    if (node->key >= key) return node;
                    else {
                        node = nullptr;
                        return node;
                    }
                }
                if (!node || (node->numChildren <= 0)) {
                    node = nullptr;
                    return node;
                }
            }
            node = nullptr;
            return node;
        };
        std::pair<epoch_btreeNode*, locker> // find an object with the smallest key larger equal the given key
            NodeFindSmallestLargerEqual_ForRemoval(keyType const& key) {
            return NodeFindSmallestLargerEqual(key, true);
        }

    #if 0
        std::pair<epoch_btreeNode*, locker> // find an object with the largest key smaller equal the given key
            NodeFindLargestSmallerEqual(keyType key) {
            epoch_btreeNode
                * smaller;
            std::pair<epoch_btreeNode*, locker>
                out;
            epoch_btreeNode*&
                node = out.first;
            locker&
                locking = out.second;

            locking.push_back(mut);
            if (!root || !root->firstChild) {
                node = nullptr;
                locking.clear();
                return out;
            }

            for (node = root->firstChild, smaller = nullptr; node; ) {
                while (node->next) {
                    if (node->key >= key) break;
                    smaller = node;
                    node = node->next;
                }
                if (node->object) {
                    if (node->key <= key) return node;
                    else if (smaller == nullptr) {
                        node = nullptr;
                        locking.clear();
                        return out;
                    }
                    else {
                        node = smaller;
                        if (node->object) return out;
                    }
                }

                if (!node || !node->firstChild) {
                    node = nullptr;
                    locking.clear();
                    return out;
                }

                node = node->firstChild;
            }
            node = nullptr;
            locking.clear();
            return out;
        };
    #endif
        std::pair<epoch_btreeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
            GetRoot() {
            std::pair<epoch_btreeNode*, locker> out;
            out.second.push_back(mut);
            out.first = root;
            return out;
        };
        std::pair<epoch_btreeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
            GetRootShared() {
            std::pair<epoch_btreeNode*, locker> out;
            out.second.push_back_shared(mut);
            out.first = root;
            return out;
        };
        epoch_btreeNode* // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
            GetRoot(locker const& locked) {
            return root;
        };

    #if 0
        static epoch_btreeNode* // goes through all nodes of the tree		
            GetNext(epoch_btreeNode* node, locker& locking) {
            if (node->firstChild) {
                return node->firstChild;
            }
            else {
                while (node && (node->next == nullptr)) {
                    node = node->parent;
                }
                return node;
            }

        };
    #endif

        static epoch_btreeNode* // goes through all leaf nodes of the tree		
            GetNextLeaf(epoch_btreeNode* node, locker& locking) {
            if (!node) return nullptr;

            epoch_btreeNode*
                nxt;
            if (nxt = node->firstChild()) {
                while (nxt) {
                    node = nxt;
                    nxt = node->firstChild();
                }
                return node;
            }
            else {
                while (node && (node->next() == nullptr)) {
                    nxt = node->parent;
                    node = nxt;
                }
                if (node) {
                    nxt = node->next();
                    node = nxt;
                    nxt = node->firstChild();
                    while (nxt) {
                        node = nxt;
                        nxt = node->firstChild();
                    }
                    return node;
                }
                else return nullptr;
            }
        };
        static epoch_btreeNode*
            GetPrevLeaf(epoch_btreeNode* node, locker& locking) {
            if (!node) return nullptr;
            epoch_btreeNode*
                prev;
            if (node->lastChild()) {
                while (true) {
                    if (prev = node->lastChild(); prev) {
                        node = prev;
                    }
                    else {
                        break;
                    }
                }
                return node;
            }
            else {
                while (node && (node->prev() == nullptr)) {
                    node = node->parent;
                }
                if (node) {
                    node = node->prev();
                    while (true) {
                        if (prev = node->lastChild(); prev) {
                            node = prev;
                        }
                        else {
                            break;
                        }
                    }
                    return node;
                }
                else {
                    return nullptr;
                }
            }
        };	// goes through all leaf nodes of the tree;

        size_t
            size() const {
            auto locked{ std::shared_lock(mut) };
            return (size_t)count;
        };
        void
            clear() {
            while (true) {
                auto [node, locked] = this->GetRoot();
                if (!node) { break; }
                node = GetNextLeaf(node, locked);
                if (!node) { break; }
                Remove(node, locked);
            }
        };
        template <typename Func> epoch_btreeNode* // same as operator[], except it will call the provided function to initialize the value if no value was found. 
            get_or_make(const keyType& time, Func const& func, bool* ExistedAlready = nullptr) {
            auto g{ guard_critical_section() };
            if (auto [node, locked] = NodeFind(time, false); node) {
                if (ExistedAlready) *ExistedAlready = true;
                return node;
            }
            if (ExistedAlready) *ExistedAlready = false;
            return this->Add(func(), time);
        };
        template <typename Func> __declspec(noinline) bool // removes the first (smallest key) node in the map if func(key, object) returns true
            pop_front_if(Func const& func) {
            auto g{ guard_critical_section() };

            if (1) {
                if (epoch_btreeNode* node = first; node) {
                    if (func(node->key, *node->object())) {
                        // need to do removal
                    }
                    else {
                        return false;
                    }
                }

            }

            if (1) {
                auto locked = lock();
                if (epoch_btreeNode* node = first; node) {
                    if (func(node->key, *node->object())) {
                        Remove(node, locked);
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }

            return false;
        };
        template <typename Func> bool // calls func(key, object) on the last (largest key) node in the map
            do_at_end(Func const& func) {
            auto g{ guard_critical_section() };
            if (auto* L = last; L) {
                func(L->key, *L->object());
                return true;
            }
            else {
                return false;
            }

            //auto locked = lock_shared();
            //if (last) {
            //	func(last->key, *last->object());
            //	return true;
            //}
            //else {
            //	return false;
            //}
        };
    private:
        epoch_btreeNode*
            AllocNode(bool is_leaf) {
            epoch_btreeNode
                * node;

            node = nodeAllocator.Alloc();
            if (is_leaf) {

            }
            else {
                for (int i = 0; i < maxChildrenPerNode; ++i) node->children()[i] = nullptr;
            }

            node->key = {};
            node->parent = nullptr;
            node->father = this;
            node->parent_index = 0;
            node->numChildren = 0;

            return node;
        };
        __declspec(noinline) void
            FreeNode(epoch_btreeNode* node) {
            if (node) {
                nodeAllocator.Free(node);
            }
        };
        void // will split node by creating a neighbor next to it in the parent node and sharing half its children
            SplitNode(epoch_btreeNode* node) {
            int
                i, j;
            epoch_btreeNode
                * child,
                * newNode;

            // allocate a new node
            newNode = AllocNode(false);
            newNode->parent = node->parent;

            // divide the children over the two nodes
            child = node->firstChild();
            newNode->children()[0] = child;
            for (j = 1, i = 3; i < node->numChildren; i += 2, j++) {
                child = child->next();
                newNode->children()[j] = child;
            }
            newNode->key = child->key;
            newNode->numChildren = node->numChildren / 2;
            for (i = 0; i < newNode->numChildren; ++i) {
                newNode->children()[i]->parent = newNode;
                newNode->children()[i]->parent_index = i;
            }

            newNode->parent_index = node->parent_index;
            node->pop_front_children(newNode->numChildren);
            node->parent->add_child_at(newNode, newNode->parent_index);
            node->key = node->children()[node->numChildren - 1]->key;
        };;
        epoch_btreeNode* // node1 will be deleted and its children appended to node2
            MergeNodes(epoch_btreeNode* node1, epoch_btreeNode* node2) {
            for (int i = 0; i < node1->numChildren; ++i)
                node2->add_child_at(node1->children()[i], i);
            (void)node1->parent->pop_child(node1->parent_index);
            FreeNode(node1);
            return node2;
        };

    };

    template< class objType, class keyType>
    class epoch_map {
    private:
        mutable epoch_btree<objType, keyType, 10>
            tree;
        using GuardType = typename decltype(tree)::GuardType;

    public:
        [[nodiscard]] GuardType guard_critical_section() const {
            return tree.guard_critical_section();
        };

        class WrappedReference {
        private:
            GuardType guard;

        public:
            const keyType&
                first;
            objType&
                second;

            WrappedReference(
                const keyType& _first,
                objType& _second,
                const epoch_map* _parent)
                : first{ _first }
                , second{ _second }
                , guard{ _parent->guard_critical_section() }
            {};
            WrappedReference(WrappedReference const&) = delete;
            WrappedReference(WrappedReference&&) = delete;
            WrappedReference& operator=(WrappedReference const&) = delete;
            WrappedReference& operator=(WrappedReference&&) = delete;
            ~WrappedReference() = default;
        };
        class WrappedReferenceFast {
        public:
            const keyType&
                first;
            objType&
                second;

            WrappedReferenceFast(const keyType* _first, objType* _second)
                : first{ *_first }
                , second{ *_second }
            {};
            WrappedReferenceFast(WrappedReferenceFast const&) = delete;
            WrappedReferenceFast(WrappedReferenceFast&&) = delete;
            WrappedReferenceFast& operator=(WrappedReferenceFast const&) = delete;
            WrappedReferenceFast& operator=(WrappedReferenceFast&&) = delete;
            ~WrappedReferenceFast() = default;
        };

        epoch_map() = default;
        epoch_map(epoch_map const& rhs) = delete;
        epoch_map(epoch_map&& rhs) = delete;
        epoch_map& operator=(epoch_map const& rhs) = delete;
        epoch_map& operator=(epoch_map&& rhs) = delete;
        ~epoch_map() = default;

        WrappedReference
            insert(const keyType& time, objType&& value) {
            auto g = guard_critical_section();
            auto* node_ptr = tree.Add(std::move(value), time);
            return WrappedReference(node_ptr->key, *node_ptr->object(), this);
        };
        void
            insert_fast(const keyType& time, objType&& value) {
            (void)tree.Add(std::move(value), time);
        };
        objType& // throws if the key is not found. 
            at(const keyType& time) const {
            auto g{ guard_critical_section() };
            if (auto [node, locker] = tree.NodeFind(time); node) {                
                return *node->object();
            }
            throw std::range_error("Could not find key");
        };
        objType* // returns nullptr if the key is not found. 
            try_at(const keyType& time) const {
            auto g{ guard_critical_section() };
            if (auto [node, locker] = tree.NodeFind(time); node) {               
                return node->ptr; //  object();
            }
            return nullptr;
        };
        objType& // if already exists, returns the value. Otherwise, creates the value (default init) and returns the value. May throw under heavy conflict. 
            operator[](const keyType& time) {
            auto g = guard_critical_section();
            if (auto [node, locker] = tree.NodeFind(time); node) return *node->object();
            if (auto [node, locker] = tree.NodeFind_ForRemoval(time); node) return *node->object();
            else {
                if constexpr (std::is_copy_constructible_v< objType > && std::is_constructible_v< objType >) {
                    if (node = tree.Add({}, time, locker); node)
                        return *node->object();
                }
            }
            throw std::range_error("Could not find key");
        };
        template <typename F>
        objType& // if already exists, returns the value. Otherwise, creates the value (using function) and returns the value. May throw under heavy conflict. 
            get_or_make(const keyType& time, F const& func) {
            auto g = guard_critical_section();
            if (auto [node, locker] = tree.NodeFind(time); node) return *node->object();
            //if (auto [node, locker] = tree.NodeFind_ForRemoval(time); node) return *node->object();
            //else {
            if constexpr (std::is_copy_constructible_v< objType > && std::is_constructible_v< objType >) {
                if (auto node = tree.Add(func(), time); node)
                    return *node->object();
            }
            //}
            throw std::range_error("Could not find key or not construct the new object");
        };
        bool // optionally get a copy of the object being deleted. 
            erase(const keyType& time, objType* out = nullptr) const {
            auto g = guard_critical_section();
            if (auto [node, locker] = tree.NodeFind(time, true); node) {                
                if (out) *out = *node->object();
                return tree.Remove(node, locker);
            }
            return false;
        };
        void
            clear() {
            auto g = guard_critical_section();
            auto locked = tree.lock();
            while (true) {
                if (auto* p = tree.GetRoot(locked)) {
                    if (p = tree.GetNextLeaf(p, locked); p) {
                        tree.Remove(p, locked);
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }

        };
        size_t
            size() const {
            return tree.size();
        };

        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair< const keyType*, objType* >;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;
            using iter_type = typename decltype(tree)::epoch_btreeNode;
            using lock_type = typename decltype(tree)::locker;

            Iterator(epoch_map* parent = nullptr, iter_type* ptr = nullptr) : _parent{ parent }, _ptr(ptr), _data(nullptr, nullptr), _lock((parent&& ptr) ? parent->tree.lock_shared() : lock_type{}) {
                if (_ptr) {
                    _data = { &_ptr->key, _ptr->object() };
                }
            };
            Iterator(const Iterator& rhs) : _parent(rhs._parent), _ptr(rhs._ptr), _data(nullptr, nullptr) {
                if (_ptr) {
                    _data = { &_ptr->key, _ptr->object() };
                }
            };

            inline reference operator*() { return _data; }
            inline pointer operator->() { return &_data; }
            inline const reference operator*() const { return _data; }
            inline const pointer operator->() const { return &_data; }

            inline Iterator& operator++() {
                _ptr = typename decltype(tree)::GetNextLeaf(_ptr, _lock);
                if (_ptr) {
                    _data = { &_ptr->key, _ptr->object() };
                }
                else {
                    _data = { nullptr, nullptr };
                }
                return *this;
            }
            inline Iterator operator++(int) { Iterator tmp(*this); this->operator++(); return tmp; }

            inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            inline bool operator>(const Iterator& rhs) const {
                if (_ptr == rhs._ptr) return false;
                if (!_ptr) return false;
                if (!rhs._ptr) return true;
                return _ptr->key > rhs._ptr->key;
            };
            inline bool operator>=(const Iterator& rhs) const {
                if (_ptr == rhs._ptr) return true;
                if (!_ptr) return false;
                if (!rhs._ptr) return true;
                return _ptr->key >= rhs._ptr->key;
            };
            inline bool operator<(const Iterator& rhs) const { return !operator>=(rhs); };
            inline bool operator<=(const Iterator& rhs) const { return !operator>(rhs); };

        protected:
            std::pair< const keyType*, objType* >
                _data;
            iter_type*
                _ptr;
            epoch_map*
                _parent;
            lock_type
                _lock;
        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() {
            auto locked = this->tree.lock_shared();
            return Iterator(this, this->tree.GetNextLeaf(this->tree.GetRoot(locked), locked));
        };
        auto end() {
            return Iterator(nullptr, nullptr);
        };
        auto cbegin() const { return const_cast<epoch_map*>(this)->begin(); };
        auto cend() const { return const_cast<epoch_map*>(this)->end(); };
        auto begin() const { return const_cast<epoch_map*>(this)->begin(); };
        auto end() const { return const_cast<epoch_map*>(this)->end(); };

    };

    template< class objType, class keyType, int maxChildrenPerNode = 10 >
    class bTree {
    public:
        class bTreeNode {
        public:
            keyType							
                key;			// key used for sorting
            objType* 
                object;			// if != NULL pointer to object stored in leaf node
            bTreeNode* 
                parent;			// parent node
            bTreeNode* 
                next;			// next sibling
            bTreeNode* 
                prev;			// prev sibling
            int								
                numChildren;	// number of children
            bTreeNode* 
                firstChild;		// first child
            bTreeNode* 
                lastChild;		// last child
        };

    public:
        bTree(ebr::fast_atomic_allocator<bTreeNode, 128>& allocator) 
            : root{ nullptr }
            , nodeAllocator{ &allocator }
        {};
        bTree()
            : root{ nullptr }
            , nodeAllocator{ nullptr }
        {};
        ~bTree() {};

        __declspec(noinline) bTreeNode* 
            Add(objType* object, keyType key) {
            bTreeNode* node, * child, * newNode;

            if (root == NULL) {
                root = AllocNode();
            }

            if (root->numChildren >= maxChildrenPerNode) {
                newNode = AllocNode();
                newNode->key = root->key;
                newNode->firstChild = root;
                newNode->lastChild = root;
                newNode->numChildren = 1;
                root->parent = newNode;
                SplitNode(root);
                root = newNode;
            }

            newNode = AllocNode();
            newNode->key = key;
            newNode->object = object;

            for (node = root; node->firstChild != NULL; node = child) {

                if (key > node->key) {
                    node->key = key;
                }

                // find the first child with a key larger equal to the key of the new node
                for (child = node->firstChild; child->next; child = child->next) {
                    if (key <= child->key) {
                        break;
                    }
                }

                if (child->numChildren == 0) {
                    if (key <= child->key) {
                        // insert new node before child
                        if (child->prev) {
                            child->prev->next = newNode;
                        }
                        else {
                            node->firstChild = newNode;
                        }
                        newNode->prev = child->prev;
                        newNode->next = child;
                        child->prev = newNode;
                    }
                    else {
                        // insert new node after child
                        if (child->next) {
                            child->next->prev = newNode;
                        }
                        else {
                            node->lastChild = newNode;
                        }
                        newNode->prev = child;
                        newNode->next = child->next;
                        child->next = newNode;
                    }

                    newNode->parent = node;
                    node->numChildren++;

                    return newNode;
                }

                // make sure the child has room to store another node
                if (child->numChildren >= maxChildrenPerNode) {
                    SplitNode(child);
                    if (key <= child->prev->key) {
                        child = child->prev;
                    }
                }
            }

            // we only end up here if the root node is empty
            newNode->parent = root;
            root->key = key;
            root->firstChild = newNode;
            root->lastChild = newNode;
            root->numChildren++;

            return newNode;
        };						// add an object to the tree
        __declspec(noinline) void
            Remove(bTreeNode* node) {
            bTreeNode* parent;

            // unlink the node from it's parent
            if (node->prev) {
                node->prev->next = node->next;
            }
            else {
                node->parent->firstChild = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
            else {
                node->parent->lastChild = node->prev;
            }
            node->parent->numChildren--;

            // make sure there are no parent nodes with a single child
            for (parent = node->parent; parent != root && parent->numChildren <= 1; parent = parent->parent) {

                if (parent->next) {
                    parent = MergeNodes(parent, parent->next);
                }
                else if (parent->prev) {
                    parent = MergeNodes(parent->prev, parent);
                }

                // a parent may not use a key higher than the key of it's last child
                if (parent->key > parent->lastChild->key) {
                    parent->key = parent->lastChild->key;
                }

                if (parent->numChildren > maxChildrenPerNode) {
                    SplitNode(parent);
                    break;
                }
            }
            for (; parent != NULL && parent->lastChild != NULL; parent = parent->parent) {
                // a parent may not use a key higher than the key of it's last child
                if (parent->key > parent->lastChild->key) {
                    parent->key = parent->lastChild->key;
                }
            }

            // free the node
            FreeNode(node);

            // remove the root node if it has a single internal node as child
            if ((root->numChildren == 1) && (root->firstChild->numChildren > 0)) {
                bTreeNode* oldRoot = root;
                root->firstChild->parent = NULL;
                root = root->firstChild;
                FreeNode(oldRoot);
            }

#ifdef BTREE_CHECK
            CheckTree();
#endif
        };				// remove an object node from the tree

        __declspec(noinline) bTreeNode*
            NodeFind(keyType key) const {
            bTreeNode* node;
            if (root) {
                for (node = root->firstChild; node != NULL; node = node->firstChild) {
                    while (node->next) {
                        if (node->key >= key) {
                            break;
                        }
                        node = node->next;
                    }
                    if (node->numChildren == 0) {
                        if (node->key == key) {
                            return node;
                        }
                        else {
                            return NULL;
                        }
                    }
                }
            }
            return NULL;
        };								// find an object using the given key
        __declspec(noinline) bTreeNode*
            NodeFindSmallestLargerEqual(keyType key) const {
            bTreeNode* node;
            if (root) {
                for (node = root->firstChild; node != NULL; node = node->firstChild) {
                    while (node->next) {
                        if (node->key >= key) {
                            break;
                        }
                        node = node->next;
                    }
                    if (node->numChildren == 0) {
                        if (node->key >= key) {
                            return node;
                        }
                        else {
                            return NULL;
                        }
                    }
                }
            }
            return NULL;
        };			// find an object with the smallest key larger equal the given key
        __declspec(noinline) bTreeNode*
            NodeFindLargestSmallerEqual(keyType key) const {
            bTreeNode* node;
            bTreeNode* smaller = NULL;
            if (root) {
                for (node = root->firstChild; node != NULL; node = node->firstChild) {
                    while (node->next) {
                        if (node->key >= key) {
                            break;
                        }
                        smaller = node;
                        node = node->next;
                    }
                    if (node->numChildren == 0) {
                        if (node->key <= key) {
                            return node;
                        }
                        else if (smaller == NULL) {
                            return NULL;
                        }
                        else {
                            node = smaller;
                            if (node->numChildren == 0) {
                                return node;
                            }
                        }
                    }
                }
            }
            return NULL;
        };			// find an object with the largest key smaller equal the given key

        __declspec(noinline) objType*
            Find(keyType key) const {
            bTreeNode* node = NodeFind(key);
            if (node == NULL) {
                return NULL;
            }
            else {
                return node->object;
            }
        };									// find an object using the given key
        __declspec(noinline) objType*
            FindSmallestLargerEqual(keyType key) const {
            bTreeNode* node = NodeFindSmallestLargerEqual(key);
            if (node == NULL) {
                return NULL;
            }
            else {
                return node->object;
            }
        };				// find an object with the smallest key larger equal the given key
        __declspec(noinline) objType*
            FindLargestSmallerEqual(keyType key) const {
            bTreeNode* node = NodeFindLargestSmallerEqual(key);
            if (node == NULL) {
                return NULL;
            }
            else {
                return node->object;
            }
        };				// find an object with the largest key smaller equal the given key

        __declspec(noinline) bTreeNode*
            GetRoot() const {
            return root;
        };											// returns the root node of the tree
        __declspec(noinline) int
            GetNodeCount() const {
            return nodeAllocator->GetAllocCount();
        };										// returns the total number of nodes in the tree
        __declspec(noinline) bTreeNode*
            GetNext(bTreeNode* node) const {
            if (node->firstChild) {
                return node->firstChild;
            }
            else {
                while (node && node->next == NULL) {
                    node = node->parent;
                }
                return node;
            }
        };		// goes through all nodes of the tree
        __declspec(noinline) bTreeNode*
            GetNextLeaf(bTreeNode* node) const {
            if (node->firstChild) {
                while (node->firstChild) {
                    node = node->firstChild;
                }
                return node;
            }
            else {
                while (node && node->next == NULL) {
                    node = node->parent;
                }
                if (node) {
                    node = node->next;
                    while (node->firstChild) {
                        node = node->firstChild;
                    }
                    return node;
                }
                else {
                    return NULL;
                }
            }
        };	// goes through all leaf nodes of the tree

    private:
        bTreeNode* 
            root;
    public:
        ebr::fast_atomic_allocator<bTreeNode, 128>*
            nodeAllocator;
    private:
        __declspec(noinline) bTreeNode*
            AllocNode() {
            bTreeNode* node = nodeAllocator->Alloc();
            node->key = 0;
            node->parent = NULL;
            node->next = NULL;
            node->prev = NULL;
            node->numChildren = 0;
            node->firstChild = NULL;
            node->lastChild = NULL;
            node->object = NULL;
            return node;
        };
        __declspec(noinline) void
            FreeNode(bTreeNode* node) {
            nodeAllocator->Free(node);
        };
        __declspec(noinline) void
            SplitNode(bTreeNode* node) {
            int i;
            bTreeNode* child, * newNode;

            // allocate a new node
            newNode = AllocNode();
            newNode->parent = node->parent;

            // divide the children over the two nodes
            child = node->firstChild;
            child->parent = newNode;
            for (i = 3; i < node->numChildren; i += 2) {
                child = child->next;
                child->parent = newNode;
            }

            newNode->key = child->key;
            newNode->numChildren = node->numChildren / 2;
            newNode->firstChild = node->firstChild;
            newNode->lastChild = child;

            node->numChildren -= newNode->numChildren;
            node->firstChild = child->next;

            child->next->prev = NULL;
            child->next = NULL;

            // add the new child to the parent before the split node
            if (node->prev) {
                node->prev->next = newNode;
            }
            else {
                node->parent->firstChild = newNode;
            }
            newNode->prev = node->prev;
            newNode->next = node;
            node->prev = newNode;

            node->parent->numChildren++;
        };
        __declspec(noinline) bTreeNode*
            MergeNodes(bTreeNode* node1, bTreeNode* node2) {
            bTreeNode* child;

            for (child = node1->firstChild; child->next; child = child->next) {
                child->parent = node2;
            }
            child->parent = node2;
            child->next = node2->firstChild;
            node2->firstChild->prev = child;
            node2->firstChild = node1->firstChild;
            node2->numChildren += node1->numChildren;

            // unlink the first node from the parent
            if (node1->prev) {
                node1->prev->next = node2;
            }
            else {
                node1->parent->firstChild = node2;
            }
            node2->prev = node1->prev;
            node2->parent->numChildren--;

            FreeNode(node1);

            return node2;
        };

    };

    // An unsorted list of items. Items, when inserted, are given a unique (non-contiguous) index to access them later. Erasing items allows for the re-use of their index in the future.
    // Insertion, access, and erasure are all atomic actions.
    // Memory corruption is not prevented if attempting to access an index after it has been erased. 
    template<class type> class atomic_bag {
    private:
        GL::atomic_vector<std::pair<type, bool>>
            items;
        GL::ticket_dispensor<false>
            tickets;

    public:
        // re-uses the position of previous slots as much as is possible. Returns the index or "ticket" for that item.
        size_t push_back(type&& rhs) {
            auto ticket = tickets.get_ticket();
            items.get_or_make(ticket) = { std::forward<type>(rhs), true };
            return ticket;
        };
        // re-uses the position of previous slots as much as is possible. Returns the index or "ticket" for that item.
        size_t push_back(type const& rhs) {
            auto ticket = tickets.get_ticket();
            items.get_or_make(ticket) = { rhs, true };
            return ticket;
        };
        // accessor using a valid position or ticket value.
        type& operator[](size_t position) {
            return items[position];
        };
        // accessor using a valid position or ticket value.
        type const& operator[](size_t position) const {
            return items[position];
        };
        // returns a position for re-use later. May not destroy the object until the position is re-used or until the list is destroyed. Thread-safe. 
        void erase(size_t position) {
            items.at(position).second = false;
            tickets.return_ticket(position);
        };
        // for-each loop on the current, valid items. Not thread-safe. 
        template <typename F> void unsafe_for_each(F const& func) {
            for (auto& x : items) {
                if (x.second) {
                    func(x.first);
                }
            }
        };
    };

    // non-atomic, non-thread-safe allocator that can allocate arrays of items (e.g. 128 floats, 1024 strings, etc.).
    template<class type, int baseBlockSize = 1024 * sizeof(type)>
    class dynamic_allocator {
    public:
        class dynamic_block {
        public:
            type*
                GetMemory() const { return (type*)(((::byte*)this) + sizeof(dynamic_block)); }
            int
                GetSize() const { return abs(size); }
            void
                SetSize(int s, bool isBaseBlock) { size = isBaseBlock ? -s : s; }
            bool
                IsBaseBlock() const { return (size < 0); }
            
            dynamic_block*
                prev = nullptr; // previous memory block
            dynamic_block*
                next = nullptr; // next memory block
            typename bTree<dynamic_block, int>::bTreeNode*
                node = nullptr; // node in the B-Tree with free blocks
            int
                thread_id = 0;
            int
                allocated_block_index = 0;
            int
                size = 0; // size in bytes of the block
            int
                initialized_block_index = 0;
            int 
                num = 0;
        };

    private:
        bTree<dynamic_block, int>
            freeTree;   // B-Tree with free memory blocks
        atomic_bag< dynamic_block* >
            allocated_blocks;
        atomic_bag< dynamic_block* >
            initialized_blocks;

    public:
        void // required to set the node allocator before first use. 
            SetAllocator(ebr::fast_atomic_allocator<typename bTree<dynamic_block, int>::bTreeNode, 128>& allocator) {
            this->freeTree.nodeAllocator = &allocator;
        };
        static dynamic_block* // get the block for a given allocated pointer. 
            Block(type* ptr) {
            return (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));
        };

        dynamic_allocator() = default;
        dynamic_allocator(dynamic_allocator const&) = delete;
        dynamic_allocator(dynamic_allocator &&) noexcept = delete;
        dynamic_allocator& operator=(dynamic_allocator const&) = delete;
        dynamic_allocator& operator=(dynamic_allocator&&) noexcept = delete;
        ~dynamic_allocator() {
            if (!std::is_pod<type>::value) {                
                initialized_blocks.unsafe_for_each([](dynamic_block* block) {
                    type* ptr{ block->GetMemory() };
                    for (int i = 0; i < block->num; ++i)
                        (ptr + i)->~type();
                });
            }
            allocated_blocks.unsafe_for_each([](dynamic_block* block) {
                GL::mfree(block);
            });
        };

    public:
        template <typename Lock, typename Unlock> type*
            Alloc(const int num, Lock const& lock, Unlock const& unlock) {
            dynamic_block
                *block;
            type
                *ptr;

            if (num <= 0) 
                return nullptr;
            
            lock();

            block = AllocInternal(num);
            if (block == nullptr) 
                return nullptr;
            
            block = ResizeInternal(block, num);
            if (block == nullptr) 
                return nullptr;

            unlock();

            ptr = block->GetMemory();

            if (std::is_pod<type>::value) 
                ::memset((void*)ptr, 0, sizeof(type) * num);
            else {
                block->num = num;
                for (int i = 0; i < num; ++i) new (ptr + i) type();
                block->initialized_block_index = initialized_blocks.push_back(block);
            }

            return ptr;
        };
        template <typename Lock, typename Unlock> void
            Free(type* ptr, Lock const& lock, Unlock const& unlock) {
            if (!ptr) { return; }

            dynamic_block* block = (dynamic_block*) (((::byte*)ptr) - (int)sizeof(dynamic_block));

            if (!std::is_pod<type>::value) {
                for (int i = 0; i < block->num; ++i) (ptr + i)->~type();
                initialized_blocks.erase(block->initialized_block_index);
            }

            lock();

            FreeInternal(block);

            unlock();
        };

    private:
        dynamic_block* // find a free block that is big enough for the request, otherwise manufacture it. 
            AllocInternal(const int num) {
            dynamic_block* block;
            int alignedBytes = (num * sizeof(type) + 15) & ~15; // request is aligned to 16 bytes

            block = freeTree.FindSmallestLargerEqual(alignedBytes);
            if (block) {                
                UnlinkFreeInternal(block);
            }
            else {
                int allocSize = std::max(baseBlockSize, alignedBytes + (int)sizeof(dynamic_block));

                block = (dynamic_block*)GL::malloc((size_t)allocSize);

                block->SetSize(allocSize - (int)sizeof(dynamic_block), true);
                block->allocated_block_index = allocated_blocks.push_back(block);                
                block->next = nullptr;
                block->prev = nullptr;
                block->node = nullptr;
            }
            block->thread_id = GL::util::get_thread_id();
            return block;
        };
        void
            UnlinkFreeInternal(dynamic_block* block) {
            freeTree.Remove(block->node);
            block->node = nullptr;
        };
        dynamic_block*
            ResizeInternal(dynamic_block* block, const int num) {
            dynamic_block* newBlock;
            int alignedBytes = (num * sizeof(type) + 15) & ~15;
            // if the new size is larger
            if (alignedBytes > block->GetSize()) {

                dynamic_block* nextBlock = block->next;

                // try to annexate the next block if it's free
                if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL &&
                    block->GetSize() + (int)sizeof(dynamic_block) + nextBlock->GetSize() >= alignedBytes) {

                    UnlinkFreeInternal(nextBlock);
                    block->SetSize(block->GetSize() + (int)sizeof(dynamic_block) + nextBlock->GetSize(), block->IsBaseBlock());
                    block->next = nextBlock->next;
                    if (nextBlock->next) {
                        nextBlock->next->prev = block;
                    }
                }
                else {
                    // allocate a new block and copy
                    dynamic_block* oldBlock = block;
                    block = AllocInternal(num);
                    if (block == NULL) {
                        return NULL;
                    }
                    ::memcpy(block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize());
                    FreeInternal(oldBlock);
                }
            }

            // if the unused space at the end of this block is large enough to hold a block with at least one element
            if ((block->GetSize() - alignedBytes - (int)sizeof(dynamic_block)) < (int)sizeof(type)) 
                return block;
            
            newBlock = (dynamic_block*) (((::byte*)block) + (int)sizeof(dynamic_block) + alignedBytes);
            newBlock->SetSize(block->GetSize() - alignedBytes - (int)sizeof(dynamic_block), false);
            newBlock->next = block->next;
            newBlock->prev = block;
            if (newBlock->next != NULL) {
                newBlock->next->prev = newBlock;
            }
            newBlock->node = NULL;
            block->next = newBlock;
            block->SetSize(alignedBytes, block->IsBaseBlock());

            FreeInternal(newBlock);

            return block;
        };
        void
            FreeInternal(dynamic_block* block) {
            while (true) {
                // try to merge with a previous free block
                if (dynamic_block* prevBlock = block->prev; prevBlock && !prevBlock->IsBaseBlock() && prevBlock->node != NULL) {
                    UnlinkFreeInternal(prevBlock);
                    prevBlock->SetSize(prevBlock->GetSize() + (int)sizeof(dynamic_block) + block->GetSize(), prevBlock->IsBaseBlock());
                    prevBlock->next = block->next;
                    if (block->next) {
                        block->next->prev = prevBlock;
                    }
                    block = prevBlock;
                }
                // try to merge with a next free block
                else if (dynamic_block* nextBlock = block->next; nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL) {
                    UnlinkFreeInternal(nextBlock);
                    block->SetSize(nextBlock->GetSize() + (int)sizeof(dynamic_block) + block->GetSize(), block->IsBaseBlock());
                    block->next = nextBlock->next;
                    if (nextBlock->next) {
                        nextBlock->next->prev = block;
                    }
                }
                else if (dynamic_block *nextBlock = block->next, *prevBlock = block->prev; !nextBlock && !prevBlock) {
                    if (freeTree.FindSmallestLargerEqual(block->GetSize())) {
                        allocated_blocks.erase(block->allocated_block_index);
                        GL::mfree(block);
                    }
                    else {
                        LinkFreeInternal(block);
                    }
                    return;
                }
                else {
                    LinkFreeInternal(block);
                    return;
                }
            }            
        };
        void
            LinkFreeInternal(dynamic_block* block) {
            block->node = freeTree.Add(block, block->GetSize());
        };

    };

    // thread-safe allocator that can allocate arrays of items (e.g. 128 floats, 1024 strings, etc.).
    template<class type, int baseBlockSize = 1024 * sizeof(type)>
    class parallel_dynamic_allocator {
    protected:
        ebr::fast_atomic_allocator<typename ebr::bTree<typename ebr::dynamic_allocator<type, baseBlockSize>::dynamic_block, int>::bTreeNode, 128>
            allocator;
        GL::thread_object_no_default < std::pair<ebr::dynamic_allocator<type, baseBlockSize>, GL::fast_exclusive_mutex> >
            alloc;

    public:
        parallel_dynamic_allocator() 
            : allocator()
            , alloc()
        {
            alloc._after_construction = [this](auto& tree) {
                tree.first.SetAllocator(this->allocator);
            };
        };
        parallel_dynamic_allocator(parallel_dynamic_allocator const&) = delete;
        parallel_dynamic_allocator(parallel_dynamic_allocator &&) noexcept = delete;
        parallel_dynamic_allocator& operator=(parallel_dynamic_allocator const&) = delete;
        parallel_dynamic_allocator& operator=(parallel_dynamic_allocator&&) noexcept = delete;
        ~parallel_dynamic_allocator() = default;

    public:
        type*
            Alloc(const int num) {
            auto& Alloc = *alloc;
            return Alloc.first.Alloc(num, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            });
        };
        void
            Free(type* ptr) {
            auto& Alloc = alloc[ebr::dynamic_allocator<type, baseBlockSize>::Block(ptr)->thread_id];
            Alloc.first.Free(ptr, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            });
        };

    };

};

namespace GL {
    class stopwatch_group {
    protected:
        GL::thread_object_no_default<GL::stopwatch> 
            stopwatches;
        GL::thread_object_no_default<std::vector<GL::nanosecond>>
            time_results;

    public:
        ~stopwatch_group() {   
            std::vector<GL::nanosecond> quantile;
            time_results.for_each([&quantile](std::vector<GL::nanosecond> const& times) {
                quantile.insert(quantile.end(), times.begin(), times.end());
            });
            std::sort(quantile.begin(), quantile.end());            
            if (quantile.size() > 3) {
                int size = quantile.size();
                int mid = size / 2;
                GL::nanosecond median = (size % 2 == 0) ? ((quantile[mid] + quantile[mid - 1]) / 2) : quantile[mid];

                std::vector<GL::nanosecond> first;
                std::vector<GL::nanosecond> third;
                first.resize(mid + 1);
                third.resize(mid + 1);

                for (int i = 0; i != mid; ++i)                
                    first[i] = quantile[i];
                
                for (int i = mid; i != size; ++i)                
                    third[i-mid] = quantile[i];
                
                GL::nanosecond fst;
                GL::nanosecond trd;

                int side_length = 0;
                if (size % 2 == 0) side_length = size / 2;                
                else side_length = (size - 1) / 2;

                fst = (size / 2) % 2 == 0 ? (first[side_length / 2] / 2 + first[(side_length - 1) / 2]) / 2 : first[side_length / 2];
                trd = (size / 2) % 2 == 0 ? (third[side_length / 2] / 2 + third[(side_length - 1) / 2]) / 2 : third[side_length / 2];
                
                auto as_reasonable_time = [](GL::nanosecond const& rhs) -> GL::value {
                    if (GL::second(rhs) > 0.5f) return GL::second(rhs);
                    if (GL::decisecond(rhs) > 0.5f) return GL::millisecond(rhs);
                    if (GL::centisecond(rhs) > 0.5f) return GL::millisecond(rhs);
                    if (GL::millisecond(rhs) > 0.5f) return GL::millisecond(rhs);                    
                    return GL::millisecond(rhs);
                };

                auto out = std::vector{ 
                    as_reasonable_time(quantile[0]).to_string(), 
                    as_reasonable_time(fst).to_string(), 
                    as_reasonable_time(median).to_string(), 
                    as_reasonable_time(trd).to_string(), 
                    as_reasonable_time(quantile[quantile.size() - 1]).to_string() 
                };
                std::cout << GL::printf("[ Min: %s, 25%: %s, 50%: %s, 75%: %s, Max: %s ]\n", 
                    out[0].c_str().data(), 
                    out[1].c_str().data(),
                    out[2].c_str().data(),
                    out[3].c_str().data(),
                    out[4].c_str().data()
                );

                // std::cout << "Min/25%/50%/75%/Max: [ " + quantile[0].to_string().add_to_delim(fst.to_string(), " / ").add_to_delim(median.to_string(), " / ").add_to_delim(trd.to_string(), " / ").add_to_delim(quantile[quantile.size()-1].to_string(), " / ") + " ]\n";
            }
        };

    public:
        std::shared_ptr<void> debug_timer() {
            return std::static_pointer_cast<void>(std::shared_ptr<int>(reinterpret_cast<int*>(1ull << 63ull), [startTime = clock::ns(), this](int*) -> void {
                auto stopTime_s = GL::nanosecond(static_cast<long double>(clock::ns() - startTime));
                this->time_results->push_back(stopTime_s);
            }));
        };

    };

};




int main() {
#if 0
    if (0) {
        GL::atomic_shared_ptr<GL::any> ptr;
        // CategoricalCache<GL::atomic_shared_ptr<any>, 4> cache;
        GL::parallel::For(0, 1'000'000, [&](size_t i) {
            switch (i % 4) {
            case 0:
                ptr /**cache.at<0>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            case 1:
                ptr /**cache.at<1>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            case 2:
                ptr /**cache.at<2>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            case 3:
                ptr /**cache.at<3>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            }
            });
        //std::cout << cache.current<0>()->load_fast()->cast<GL::string&>() << std::endl;
        //std::cout << cache.current<1>()->load_fast()->cast<GL::string&>() << std::endl;
        //std::cout << cache.current<2>()->load_fast()->cast<GL::string&>() << std::endl;
        //std::cout << cache.current<3>()->load_fast()->cast<GL::string&>() << std::endl;

    }

    while(1)
        if (auto timer = GL::stopwatch().debug_timer("1B calls: \t")) {
        CategoricalCache<any> cache;
        
        //for (size_t i = 0; i < 1'000'000; ++i){
            // auto g = cache.guard_critical_section();
        GL::parallel::For(0, 1'000'000/*'000*/, [&](size_t i) {
            cache.insert(i % 10000, GL::make_shared<any>(any::instance(GL::printf("%zu - 0", i))));
        });




        //// GL::atomic_shared_ptr<any> ptr;
        //CategoricalCache<any> cache;
        //GL::parallel::For(0, 1'000'000/*'000*/, [&](size_t i) {
        //    switch (i % 4) {
        //    case 0:
        //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 0", i)); }, i);
        //        break;
        //    case 1:
        //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 1", i)); }, i);
        //        break;
        //    case 2:
        //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 2", i)); }, i);
        //        break;
        //    case 3:
        //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 3", i)); }, i);
        //        break;
        //    default:
        //        break;
        //    }
        //});
        //std::cout << cache.current()->cast<GL::string&>() << std::endl;
        ////std::cout << cache.current<1>()->cast<GL::string&>() << std::endl;
        ////std::cout << cache.current<2>()->cast<GL::string&>() << std::endl;
        ////std::cout << cache.current<3>()->cast<GL::string&>() << std::endl;
    };
#endif

    while (true) {
        while (1) {
            //ebr::fast_atomic_general_allocator alloc;            
            //if (auto timer = GL::stopwatch::debug_timer("fast_atomic_general_allocator"); true) {
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        alloc.Free(alloc.Alloc(GL::util::rand_fast(32,256)));
            //    }
            //    std::vector<void*> ps(1'000'000, nullptr);
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        ps[i] = alloc.Alloc(GL::util::rand_fast(32, 256));
            //    }
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        alloc.Free(ps[i]);
            //    }
            //}

            if (auto timer = GL::stopwatch::debug_timer("parallel_dynamic_allocator float"); true) {
                ebr::parallel_dynamic_allocator<float> allocator;

                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    try {
                        GL::parallel::For(0, 1'000'000, [&](int i) {
                            auto* ptr = allocator.Alloc((int)GL::util::rand_fast(1 << 4, 1 << 16));
                            allocator.Free(ptr);
                            });
                        GL::parallel::For_Each(ptrs, [&](float*& p) {
                            p = allocator.Alloc((int)GL::util::rand_fast(1 << 4, 1 << 16));
                            });
                        //GL::parallel::For_Each(ptrs, [&](float*& p) {
                        //    allocator.Free(p);
                        //    });
                    }
                    catch (std::runtime_error const& rte) {
                        std::cout << rte.what() << std::endl;
                        std::rethrow_exception(std::current_exception());
                    }
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete float"); false) {
                ebr::fast_atomic_allocator<typename ebr::bTree<ebr::dynamic_allocator<float>::dynamic_block, int>::bTreeNode, 128> allocator;
                GL::thread_object_no_default < GL::shared_lockable<ebr::dynamic_allocator<float>> > alloc;
                alloc._after_construction = [&allocator](GL::shared_lockable<ebr::dynamic_allocator<float>>& tree) { tree.lock()->SetAllocator(allocator); };

                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = reinterpret_cast<float*>(GL::malloc(GL::util::rand_fast(1 << 4, 1 << 16) * sizeof(float)));
                        GL::mfree(ptr);
                    });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        p = reinterpret_cast<float*>(GL::malloc(GL::util::rand_fast(1 << 4, 1 << 16) * sizeof(float)));
                    });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        GL::mfree(p);
                    });
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_dynamic_allocator std::string"); true) {
                ebr::parallel_dynamic_allocator<std::string> allocator;

                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.Alloc((int)GL::util::rand_fast(1 << 4, 1 << 10));
                        allocator.Free(ptr);
                    });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        p = allocator.Alloc((int)GL::util::rand_fast(1 << 4, 1 << 10));
                        *p = "TESTING";
                    });
                    //GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                    //    allocator.Free(p);
                    //});
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete std::string"); false) {
                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        int n = GL::util::rand_fast(1 << 4, 1 << 10);
                        auto* ptr = new std::string[n];
                        delete[] ptr;
                    });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        int n = GL::util::rand_fast(1 << 4, 1 << 10);
                        p = new std::string[n];
                        *p = "TESTING";
                    });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        delete[] p;
                    });
                }
            }





            //if (auto timer = GL::stopwatch::debug_timer("bTree"); true) {
            //    ebr::fast_atomic_allocator<ebr::bTree<float, int, 10>::bTreeNode, 128> allocator;
            //    GL::thread_object_no_default< ebr::bTree<float, int, 10>> trees;
            //    trees._after_construction = [&allocator](ebr::bTree<float, int, 10>& tree) { tree.nodeAllocator = &allocator; };

            //    GL::parallel::For(0, 1'000'000, [&](int i) {
            //        //for (int i = 0; i < 1'000'000; ++i){
            //        trees->Add(nullptr, i);
            //        if (auto* node = trees->NodeFind(i)) {
            //            if (i % 10 == 0) {
            //                trees->Remove(node);
            //            }
            //        }
            //        else {
            //            throw "ERROR";
            //        }
            //        });
            //}
        }



        if (1) {
            ebr::epoch_map<int, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_map 1"); true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i] = i;
                });
            }
        }
        if (1) {
            ebr::epoch_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_map 2"); true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i % 10'000] = i;
                    tree.erase(i % 10'000);
                });
            }
        }
        if (1) {
            ebr::epoch_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_map 3"); true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    tree.erase(i % 100);
                    tree[i % 100] = i; // protected (temporarily!) by the epoch-based protections.                     
                });
            }
        }

        if (1) {
            ebr::epoch_map<int, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_map 1 linear"); true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i] = i;
                };
            }
        }
        if (1) {
            ebr::epoch_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_map 2 linear"); true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i % 10'000] = i;
                    tree.erase(i % 10'000);
                };
            }
        }
        if (1) {
            ebr::epoch_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_map 3 linear"); true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    tree.erase(i % 100);
                    tree[i % 100] = i; // protected (temporarily!) by the epoch-based protections.                     
                };
            }
        }
    }


    while (true) {
        ebr::epoch_btree<int, int>
            tree;
        //std::vector<typename decltype(tree)::epoch_btreeNode*> arr;
        //arr.resize(1'000'000);

        //for (int i = 0; i < 100; ++i) {
        //    tree.Add((int)i, i);
        //}
        if (auto timer = GL::stopwatch::debug_timer("epoch_btree"); true) {
            GL::parallel::For(0, 10'000'000, [&](int i) {
                auto g{ tree.guard_critical_section() };
                auto node = tree.Add((int)i, i);
                //arr[i] = node;

                if (*node->object() != i) {
                    std::cout << GL::printf("Intended 1: %i, Found 1: %i\n", i, *node->object());
                }                

                //node->unlock();
                //if (!tree.root->validate_node_structure(node)) {                                        
                    //node->validate_node_structure(node);
                    //std::cout << GL::printf("Not validated! %i @ %i\n", i, node->key);
                //}

                //auto node = tree.GetNextLeaf(tree.root, false, true);
                //while (node) {
                //    node = tree.GetNextLeaf(node, true, true);
                //}

                ////if ((i % 10'000) == 0) {
                //    auto g = tree.guard_critical_section();
                //    if (auto node2 = tree.NodeFindSmallestLargerEqual(i, tree.root, false, true)) {
                //        if (*node2->object() != i) {
                //            if (node != node2) {
                //                std::cout << GL::printf("Intended 2: %i, Found 2: %i\n", i, *node2->object());
                //                node2 = tree.NodeFindSmallestLargerEqual(i, tree.root, true, true);
                //                if (*node2->object() != i) {
                //                    std::cout << GL::printf("Intended 3: %i, Found 3: %i\n", i, *node2->object());
                //                }
                //            }
                //        }
                //        node2->unlock_shared();
                //        
                //        
                //        //while (node) {
                //        //    std::cout << *node->object() << std::endl;
                //        //    node = tree.GetNextLeaf(node, true, true);
                //        //}
                //    }
                ////}

            });

            GL::parallel::For(0, 10'000'000, [&](int i) {
                auto g{ tree.guard_critical_section() };
                if (auto [node2, locker] = tree.NodeFindSmallestLargerEqual(i, false); node2) {
                    if (*node2->object() != i) {
                        std::cout << GL::printf("Intended 4: %i, Found 4: %i\n", i, *node2->object());
                        //if (node2 = arr[i]) {
                        //    if (*node2->object() != i) {
                        //        std::cout << GL::printf("Intended 4.1: %i, Found 4.1: %i\n", i, *node2->object());
                        //    }
                        //}

                    }
                }
            });
        }

        

        auto g = tree.guard_critical_section();
        auto [node, locker] = tree.GetRootShared();
        node = tree.GetNextLeaf(node, locker);
        while (node) {
            std::cout << *node->object() << std::endl;            
            node = tree.GetNextLeaf(node, locker);
        }
    }




    // object pooling test. 3-4 times faster than normal allocation / deallocation. 
    while (true) {        
        if (auto timer = GL::stopwatch().debug_timer("Normal Allocation / Deletion (int)"); false) {
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = new int();
                delete p;
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = new int();
                }
                for (int j = 0; j < 36; ++j) {
                    delete arrs[j];
                }
            });
        }
        if (auto timer = GL::stopwatch().debug_timer("Normal Allocation / Deletion (GL::string)"); false) {
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = new GL::string();
                delete p;
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = new GL::string();
                }
                for (int j = 0; j < 36; ++j) {
                    delete arrs[j];
                }
            });
        }

        if (auto timer = GL::stopwatch().debug_timer("atomic_parallel_allocator<int>"); true) {
            GL::atomic_parallel_allocator<int> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            });
        }
        if (auto timer = GL::stopwatch().debug_timer("atomic_parallel_allocator<GL::string>"); true) {
            GL::atomic_parallel_allocator<GL::string> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            });
        }

        //if (auto timer = GL::stopwatch().debug_timer("GL::epoch_search_tree<GL::string, long long, 10>"); false) {
        //    GL::epoch_search_tree<GL::value, size_t, 10> tree;
        //    GL::parallel::For(0, 1'000'000, [&](size_t i) {
        //        auto g = tree.ProtectCurrentEpoch();
        //        if (i % 100 == 0) {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);
        //            tree.Remove(ptr);
        //            ptr->object()->operator=(i);
        //        }
        //        else {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);
        //            ptr->object()->operator=(i);
        //        }           
        //    });
        //}
        //if (auto timer = GL::stopwatch().debug_timer("ebr::epoch_search_tree<GL::string, long long, 10>"); true) {
        //    ebr::epoch_search_tree<GL::value, size_t, 10> tree;
        //    GL::parallel::For(0, 1'000'000, [&](size_t i) {
        //        auto g = tree.guard_critical_section();
        //        if (i % 100 == 0) {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);
        //            tree.Remove(ptr);
        //            ptr->object()->operator=(i);
        //        }
        //        else {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);                   
        //            ptr->object()->operator=(i);
        //        }
        //    });
        //}



        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<int> (linear only)"); true) {
            ebr::fast_atomic_allocator<int> pool;
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                auto* p = pool.Alloc();
                pool.Free(p);
            };
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            };
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<int>"); true) {
            ebr::fast_atomic_allocator<int> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            });
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<GL::string>"); true) {
            ebr::fast_atomic_allocator<GL::string> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            });
        }
        if (false) {
            ebr::fast_atomic_allocator<GL::string> pool;
            while (true) {
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<GL::string>"); true) {
                    //bool quit = false;
                    //std::thread todo([&]() {
                    //    while (!quit) {
                    //        auto* p = pool.Alloc();
                    //        pool.Free(p);
                    //    }
                    //});
                    GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                        auto* p = pool.Alloc();
                        pool.Free(p);
                        });
                    GL::parallel::For(0, 1'000'000, [&](size_t i) {
                        std::array<GL::string*, 36> arrs;
                        for (int j = 0; j < 36; ++j) {
                            arrs[j] = pool.Alloc();
                        }
                        for (int j = 0; j < 36; ++j) {
                            pool.Free(arrs[j]);
                        }
                        });
                    //quit = true;
                    //todo.join();
                }
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<GL::string> consumer-producer system"); true) {
                    concurrency::concurrent_queue< GL::string* > ptrs;
                    GL::parallel::For(0, 10'000'000, [&](size_t i) {
                        if (i % 2 == 0)
                            ptrs.push(pool.Alloc());
                        else {
                            GL::string* p;
                            if (ptrs.try_pop(p)) {
                                pool.Free(p);
                            }
                        }
                    });
                    GL::string* p;
                    while (ptrs.try_pop(p)) {
                        pool.Free(p);
                    }
                }
            }
        }

        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<int> (linear only)"); true) {
            ebr::fast_atomic_epoch_allocator<int> pool;
            
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                auto* p = pool.Alloc();
                auto g = pool.guard_critical_section();
                pool.Free(p);
            };
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                std::array<int*, 36> arrs;
                auto g = pool.guard_critical_section();
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            };
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<int>"); true) {
            ebr::fast_atomic_epoch_allocator<int> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                auto g = pool.guard_critical_section();
                pool.Free(p);
            });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                auto g = pool.guard_critical_section(); 
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }                
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            });
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<GL::string>"); true) {
            ebr::fast_atomic_epoch_allocator<GL::string> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                auto g = pool.guard_critical_section();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                auto g = pool.guard_critical_section();
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            });
        }
        if (true) {
            ebr::fast_atomic_epoch_allocator<GL::string> pool;
            while (true) {
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<GL::string>"); true) {
                    GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                        auto* p = pool.Alloc();
                        auto g = pool.guard_critical_section();
                        pool.Free(p);
                    });
                    GL::parallel::For(0, 1'000'000, [&](size_t i) {
                        std::array<GL::string*, 36> arrs;
                        for (int j = 0; j < 36; ++j) {
                            arrs[j] = pool.Alloc();
                        }
                        auto g = pool.guard_critical_section();
                        for (int j = 0; j < 36; ++j) {
                            pool.Free(arrs[j]);
                        }
                    });
                }
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<GL::string> consumer-producer system"); true) {
                    concurrency::concurrent_queue< GL::string* > ptrs;
                    GL::parallel::For(0, 10'000'000, [&](size_t i) {
                        if (i % 2 == 0) {
                            auto g = pool.guard_critical_section();
                            ptrs.push(pool.Alloc());
                        }
                        else {
                            auto g = pool.guard_critical_section();
                            GL::string* p;
                            if (ptrs.try_pop(p)) {
                                pool.Free(p);
                            }
                        }
                    });
                    GL::string* p;
                    auto g = pool.guard_critical_section();
                    while (ptrs.try_pop(p)) {
                        pool.Free(p);
                    }
                }
            }
        }
    }

#if 1
    using namespace GL::literals;
    if (1) {
        Function unix_s = make_callable([](GL::second x, GL::minute y) -> GL::hour { return x + y; });
        std::cout << unix_s.do_call<GL::hour>(1_s, 1_min) << std::endl;
    }
    if (1) {
        Function unix_s = make_callable([](GL::second x, GL::datetime y) -> GL::hour { return x + (GL::minute)y; });
        auto N = GL::datetime::Now();
        std::cout << unix_s.do_call<GL::hour>(0_s, N) << std::endl;
        std::cout << unix_s.do_call<GL::hour>(120_s, N) << std::endl;
        std::cout << unix_s.do_call<GL::hour>(12000_s, N) << std::endl;
        std::cout << unix_s.do_call<GL::hour>(120000000000_s, N) << std::endl;
    }
    if (1) {
        Function unix_s = make_callable([](GL::datetime y, GL::second x) -> GL::datetime { return y + x; });
        auto N = GL::datetime::Now();
        std::cout << unix_s.do_call<GL::datetime>(N, 0_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 120_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 1_d) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 12000_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 1200000_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 120000000_s) << std::endl;        
    }

    while (1) {
        switch ((int)std::floor(GL::util::rand(0, 15.999))) {
        case 0: {
            GL::string ref = "this";
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (unboxed value)\t")) {
                // for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)ref.begins_with("this");
                });
            }
        } break;
        case 1: {
            GL::string ref = "this";
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (unboxed ref)\t")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    GL::string& Ref = ref;
                    (void)Ref.begins_with("this");
                });
            }
        } break;
        case 2: {
            auto boxed = any::instance(GL::make_shared<GL::string>("this"));
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed GL::shared_ptr)\t")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)boxed.cast<GL::string&>().begins_with("this");
                });
            }
        } break;
        case 3: {
            auto boxed = any::instance(GL::string("this"));
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed value)\t")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)boxed.cast<GL::string&>().begins_with("this");
                });
            }
        } break;
        case 4: if (0) {
            //auto boxed = any::instance(GL::string("this"));
            //if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed cast)\t")) {
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        GL::string& Ref = boxed.cast();
            //        (void)Ref.begins_with("this");
            //    }
            //}
        } break;
        case 5: {
            auto callable = GL::make_callable("begins_with", &GL::string::begins_with);
            std::array<GL::any::fast_any, 2> example{
                GL::any::fast_any::instance(GL::string("this")),
                GL::any::fast_any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with callable and w/o converters, no conversion needed, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable->operator()(&example[0], &example[0] + example.size(), false);
                });
            }
        } break;
        case 6: {
            auto callable{ Const_Member_Function_Caller(&GL::string::begins_with) };

            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with templatized callable and w/o converters, no conversion needed, w/ return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.call(&example[0]);
                });
            }
        } break;
        case 7: {
            Function callable{ make_callable(&GL::string::begins_with) };
            //std::cout << callable.to_string() << std::endl;
            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with generalized callable and w/o converters, no conversion needed, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.do_call(example[0], example[1]);
                });
            }
        } break;
        case 9: {
            Function callable{ make_callable([](GL::string const& LHS, GL::string const& RHS) -> auto { return LHS.begins_with(RHS); }) };
            //std::cout << callable.to_string() << std::endl;
            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with non-capturing lambda function and w/o converters, no conversion needed, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.do_call(example[0], example[1]);
                });
            }
        } break;
        case 10: {
            Function callable{ make_callable([](GL::string const& LHS, GL::string const& RHS) -> bool { return LHS.begins_with(RHS); }) };
            //std::cout << callable.to_string() << std::endl;
            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with do-call w/ return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    // callable.do_call<bool>(any(example[0], true), any(example[1], true));
                    callable.do_call<bool>(example[0], example[1]);
                });

            }
        } break;
        case 11: {
            struct TEST {
                GL::string obj1;
                GL::string obj2;
            };
            std::array<any, 1> example{
                any::instance(TEST{ GL::string("this"), GL::string("that") })
            };
            Function callable{ make_callable(&TEST::obj2) };
            //std::cout << callable.to_string() << std::endl;
            if (auto timer = GL::stopwatch().debug_timer("TEST::obj, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.do_call(any(example[0], true));
                });
            }
        } break;
        case 12: {
            struct TEST {
                GL::string obj1;
                GL::string obj2;
            };
            std::array<any, 1> example{
                any::instance(TEST{ GL::string("this"), GL::string("that") })
            };
            Function callable{ make_callable(&TEST::obj2) };
            //std::cout << callable.to_string() << std::endl;
            if (auto timer = GL::stopwatch().debug_timer("TEST::obj, w/ return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() { any out;
                    (void)callable.do_call<GL::string&>(any(example[0], true));
                    // if (callable.do_call<GL::string&>(any(example[0], true)) != GL::string("that")) throw "ERROR";
                });                
            }
        } break;
        case 13: {
            Function callable{ make_callable([](GL::string const& LHS, GL::string const& RHS) -> bool { return LHS.begins_with(RHS); }) };
            callable.name = "begins_with";
            callable.defaults[1] = any::instance(GL::string("this"));
            // std::cout << callable.to_string() << std::endl;
            std::array<any, 1> example{
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with do-call w/o return w/ default value")) {
                // for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    callable.do_call(any(example[0], true));
                });
            }
        } break;
        case 14: {
            GL::script_type Custom_Type("Custom");
            Function callable{ make_callable([](any const& LHS, any const& RHS) -> bool {
                return LHS.cast<GL::string const&>().begins_with(RHS.cast<GL::string const&>());
            }) };
            callable.name = "begins_with";
            callable.arguments[0] = Custom_Type.load();
            callable.arguments[1] = Custom_Type.load();
            callable.defaults[1] = any::instance(GL::string("this"));

            // std::cout << callable.to_string() << std::endl;
            std::array<any, 1> example{
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("Custom::begins_with(LHS, RHS) with do-call w/o return w/ default value")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    callable.do_call(any(example[0], true));
                });
            }
        } break;
        case 15: {
            Function callable{ []() { 
                Function out { make_callable([](GL::string const& LHS, GL::string const& RHS) -> bool { return LHS.begins_with(RHS); }) }; 
                out.name = "begins_with";
                out.defaults[1] = any::instance(GL::string("this"));
                return out; 
            }() };            
            std::array<any, 1> example{
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with async_call w/o return w/ default value")) {
                std::vector<GL::parallel::promise> jobs(1'000'000ull, GL::parallel::promise{});
                GL::parallel::For(0, 1'000'000, [&](size_t i) {
                    jobs[i] = callable.async_call<void>(example[0]).as_promise();
                });
            }
        } break;
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

