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
#include "array_allocator.h"

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
            //GL::fast_atomic_general_allocator alloc;            
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

            if (auto timer = GL::stopwatch::debug_timer("parallel_array_allocator float"); true) {
                GL::parallel_array_allocator<float> allocator;

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
                GL::fast_atomic_allocator<typename GL::b_tree<GL::array_allocator<float>::dynamic_block, int>::b_treeNode, 128> allocator;
                GL::thread_object_no_default < GL::shared_lockable<GL::array_allocator<float>> > alloc;
                alloc._after_construction = [&allocator](GL::shared_lockable<GL::array_allocator<float>>& tree) { tree.lock()->SetAllocator(allocator); };

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

            if (auto timer = GL::stopwatch::debug_timer("parallel_array_allocator std::string"); true) {
                GL::parallel_array_allocator<std::string> allocator;

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





            //if (auto timer = GL::stopwatch::debug_timer("b_tree"); true) {
            //    GL::fast_atomic_allocator<GL::b_tree<float, int, 10>::b_treeNode, 128> allocator;
            //    GL::thread_object_no_default< GL::b_tree<float, int, 10>> trees;
            //    trees._after_construction = [&allocator](GL::b_tree<float, int, 10>& tree) { tree.nodeAllocator = &allocator; };

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
            GL::epoch_btree_map<int, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_btree_map 1"); true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i] = i;
                });
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_btree_map 2"); true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i % 10'000] = i;
                    tree.erase(i % 10'000);
                });
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_btree_map 3"); true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    tree.erase(i % 100);
                    tree[i % 100] = i; // protected (temporarily!) by the epoch-based protections.                     
                });
            }
        }

        if (1) {
            GL::epoch_btree_map<int, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_btree_map 1 linear"); true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i] = i;
                };
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_btree_map 2 linear"); true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i % 10'000] = i;
                    tree.erase(i % 10'000);
                };
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp;
            if (auto timer = GL::stopwatch::debug_timer("epoch_btree_map 3 linear"); true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    tree.erase(i % 100);
                    tree[i % 100] = i; // protected (temporarily!) by the epoch-based protections.                     
                };
            }
        }
    }


    while (true) {
        GL::epoch_btree<int, int>
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
        //if (auto timer = GL::stopwatch().debug_timer("GL::epoch_search_tree<GL::string, long long, 10>"); true) {
        //    GL::epoch_search_tree<GL::value, size_t, 10> tree;
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
            GL::fast_atomic_allocator<int> pool;
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
            GL::fast_atomic_allocator<int> pool;
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
            GL::fast_atomic_allocator<GL::string> pool;
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
            GL::fast_atomic_allocator<GL::string> pool;
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
            GL::fast_atomic_epoch_allocator<int> pool;
            
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
            GL::fast_atomic_epoch_allocator<int> pool;
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
            GL::fast_atomic_epoch_allocator<GL::string> pool;
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
            GL::fast_atomic_epoch_allocator<GL::string> pool;
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

