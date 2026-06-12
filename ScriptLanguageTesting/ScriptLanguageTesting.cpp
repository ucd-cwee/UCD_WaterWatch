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
    any(any const& rhs) : m_uuid{ rhs.m_uuid & uuid::INV_FLAGS }, m_type{ rhs.m_type }, m_ptr{ rhs.m_ptr } { if (m_uuid > 0) InterlockedIncrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(m_uuid).count)); };
    // declares that the parent will NOT go out-of-scope before this child does. That guarrantee allows us to skip a increment and decrement call to the counter. 
    any(any const& rhs, bool) : m_uuid{ rhs.m_uuid | 0x1000'0000 }, m_type{ rhs.m_type }, m_ptr{ rhs.m_ptr } {};
    any(any && rhs) noexcept : m_uuid{ std::move(rhs.m_uuid) }, m_type{ std::move(rhs.m_type) }, m_ptr{ rhs.m_ptr } { rhs.m_uuid = 0; };
    any const& operator=(any const& rhs) {
        if ((m_uuid & 0x1000'0000) == 0)
            if ((m_uuid & uuid::INV_FLAGS) > 0)
                if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(m_uuid).count)) == 0)
                    uuid::free_uuid(m_uuid);

        m_uuid = rhs.m_uuid & uuid::INV_FLAGS;
        m_type = rhs.m_type; 
        m_ptr = rhs.m_ptr;
        if (m_uuid > 0) InterlockedIncrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(m_uuid).count));
        return *this;
    };
    any const& operator=(any&& rhs) noexcept {
        if ((m_uuid & 0x1000'0000) == 0)
            if ((m_uuid & uuid::INV_FLAGS) > 0)
                if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(m_uuid).count)) == 0)
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
                if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(m_uuid).count)) == 0)
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
            if (uuid > 0) InterlockedIncrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(uuid).count));
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
            if (uuid > 0) InterlockedIncrementNoFence(reinterpret_cast<volatile size_t*>(&uuid::get_uuid(uuid).count));
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
                            if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&ref.count)) == 0) {
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
                            if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&ref.count)) == 0) {
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
                            if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&ref.count)) == 0) {
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
                            if (InterlockedDecrementNoFence(reinterpret_cast<volatile size_t*>(&ref.count)) == 0) {
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

        // Allocate one new block of contiguous elements onto the free list
        void AllocBlock() {
            block_t* new_block_ptr = PushBlock();
            blocks.get_or_make(new_block_ptr->block_position = blocks_tickets.get_ticket()) = new_block_ptr;
            block_t& block = *new_block_ptr;

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[i].m_block = new_block_ptr;
            }
            block.elements[BlockSize - 1].m_pNext = nullptr;
            block.elements[BlockSize - 1].m_block = new_block_ptr;
            block.count_free = BlockSize;

            // push pNode onto head of list.
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Allocate one old block of contiguous elements back onto the free list
        void ReallocBlock(block_t* existing_block_ptr) {
            block_t& block = *existing_block_ptr;

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
        {
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
        template <typename... TArgs> T* Alloc(TArgs &&... a) {
            element_t* element{ nullptr };
            while (1) {
                element = *m_free;    
                if (element) {
                    *m_free = element->m_pNext;
                    element->epoch = std::numeric_limits<long long>::max(); // indicates it's been initiated
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
            t->epoch = GL::util::get_current_epoch();
            if (--t->m_block->count_free == 0) {
                // this entire block has been queued for return and release. 
                ReallocBlock(t->m_block);
            }
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

    private:
        GL::atomic_vector<block_t*>
            blocks; // vector of all blocks currently allocated and alive
        GL::ticket_dispensor<false>
            blocks_tickets; // ticket dispensor to re-use blocks indexes and minimize the size of blocks
        GL::thread_object<element_t*>
            m_free;
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

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[i].m_block = new_block_ptr;
            }
            block.elements[BlockSize - 1].m_pNext = nullptr;
            block.elements[BlockSize - 1].m_block = new_block_ptr;
            block.count_free = BlockSize;

            // push pNode onto head of list.
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Allocate one old block of contiguous elements back onto the free list. This requires that it has been retired and is safe to reclaim. 
        void ReallocBlock(block_t* existing_block_ptr) {
            block_t& block = *existing_block_ptr;

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
                            ReallocBlock(block);
                            out = ReclamationResult::ReclaimedRetiredBlocks;
                        }
                        else {
                            ReleaseBlock(block);
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
                            ReallocBlock(block);
                            out = ReclamationResult::ReclaimedRetiredBlocks;
                        }
                        else {
                            ReleaseBlock(block);
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
                    InterlockedExchangeNoFence(reinterpret_cast<volatile long*>(&epoch_protected), 1);
                    queued_epoch = parent->current_epoch + 1;
                    if (deferrment <= 0) deferrment = 1'000;
                }
            };
            __declspec(noinline) void exit_critical_section() const {
                if (--epoch_depth == 0) {
                    InterlockedExchangeNoFence(reinterpret_cast<volatile long*>(&epoch_protected), 0);

                    // from the perspective of this thread, we are now OK to free pointers older than "epoch";
                    InterlockedExchangeNoFence64(reinterpret_cast<volatile long long*>(&delete_if_older_than), epoch);
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
                            if (InterlockedCompareExchangeNoFence64(reinterpret_cast<volatile long long*>(&parent->current_epoch), currentEpoch, old_epoch) == old_epoch) {
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
                    element->epoch = std::numeric_limits<long long>::max(); // indicates it's been initiated
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
            
            // size_t free_count = InterlockedDecrementNoFence(reinterpret_cast<volatile unsigned long long*>(&t->m_block->count_free));
            // if this is the first item to be returned...
            //if (free_count == (BlockSize - 1)) {
            //    // by definition, the first (oldest) epoch will be the one we just did that successfully started us on the path towards retirement...
            //    t->m_block->oldest_epoch = t->epoch;
            //}
            //// if this entire block has been queued for return and release...
            //else 
            if (InterlockedDecrementNoFence(reinterpret_cast<volatile unsigned long long*>(&t->m_block->count_free)) == 0) {
                // by definition, the most recent (youngest) epoch will be the one we just did that successfully retired the block...
                t->m_block->youngest_epoch = t->epoch; 

                // queue the retired block
                retired_blocks->push(t->m_block);
            }
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

        [[nodiscard]] auto guard_critical_section() {
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
            states; // thread states. Used to manage the 
        long long
            current_epoch;
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
            // auto g = cache.ProtectCurrentEpoch();
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
        while (true) {
            ebr::fast_atomic_allocator<GL::string> pool;
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

