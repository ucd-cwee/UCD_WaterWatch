/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

This file is distributed under the BSD License.
Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
http://www.chaiscript.com

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and for better real-time support, including object-typing from parsed code, pre-parsing code without running, real multithreaded code analysis, and more.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "chaiscript_wrapper.h"

namespace chaiscript::bootstrap::standard_library {
    /// Bidir_Range, based on the D concept of ranges.
    /// \todo Update the Range code to base its capabilities on the user_typetraits of the iterator passed in
    template<typename Container, typename IterType>
    struct Bidir_Range {
        using container_type = Container;
        // using const_reference = Container::const_reference;
        // typedef Container::const_reference const_reference;

        Bidir_Range(Container& c)
            : m_begin(c.begin())
            , m_end(c.end())
        {
        }

        bool empty() const noexcept { return m_begin == m_end; }

        void pop_front() {
            if (empty()) {
                throw std::range_error("Range empty");
            }
            ++m_begin;
        }

        void pop_back() {
            if (empty()) {
                throw std::range_error("Range empty");
            }
            --m_end;
        }

        typename Container::reference front() const { // decltype(auto)
            if (empty()) {
                throw std::range_error("Range empty");
            }
            return const_cast<typename Container::reference>(m_begin.operator*());
        }

        typename Container::reference back() const { // decltype(auto)
            if (empty()) {
                throw std::range_error("Range empty");
            }
            auto pos = m_end;
            --pos;
            // return (*(pos));
            return const_cast<typename Container::reference>(pos.operator*());
        }

        IterType m_begin;
        IterType m_end;
    };

    namespace detail {
        template<typename T>
        size_t count(const T& t_target, const typename T::key_type& t_key) {
            return t_target.count(t_key);
        }

        template<typename T>
        bool contains(const T& t_target, const typename T::key_type& t_key) {
            return (t_target.find(t_key) != t_target.end());
        }

        template<typename T>
        std::vector<chaiscript::Boxed_Value> keys(const T& t_target) {
            std::vector<chaiscript::Boxed_Value> out;
            for (auto& pair : t_target) {
                out.push_back(chaiscript::Boxed_Value(pair.first));
            }
            return out;
        }

        template<typename T>
        void insert(T& t_target, const T& t_other) {
            t_target.insert(t_other.begin(), t_other.end());
        }

        template<typename T>
        void insert_ref(T& t_target, const typename T::value_type& t_val) {
            t_target.insert(t_val);
        }

        /// Add Bidir_Range support for the given ContainerType
        template<typename Bidir_Type>
        void input_range_type_impl(const std::string& type, Module& m) {
            m.add(user_type<Bidir_Type>(), type + "_Range");

            copy_constructor<Bidir_Type>(type + "_Range", m);

            m.add(constructor<Bidir_Type(typename Bidir_Type::container_type&)>(), "range_internal");

            m.add(fun(&Bidir_Type::empty), "empty");
            m.add(fun(&Bidir_Type::pop_front), "pop_front");
            m.add(fun(&Bidir_Type::front), "front");
            m.add(fun(&Bidir_Type::pop_back), "pop_back");
            m.add(fun(&Bidir_Type::back), "back");
        }

        /// Algorithm for inserting at a specific position into a container
        template<typename Type>
        void insert_at(Type& container, int pos, const typename Type::value_type& v) {
            auto itr = container.begin();
            auto end = container.end();

            if (pos < 0 || container.size() < pos) { // std::distance(itr, end) < pos) {
                throw std::range_error("Cannot insert past end of range");
            }

            for (int i = 0; i < pos; i++)
                itr++;

            container.insert(itr, v);
        }

        /// Algorithm for erasing a specific position from a container
        template<typename Type>
        void erase_at(Type& container, int pos) {
            auto itr = container.begin();
            auto end = container.end();

            if (pos < 0 || container.size() <= pos) { // std::distance(itr, end) <= pos) {
                throw std::range_error("Cannot erase past end of range");
            }

            for (int i = 0; i < pos; i++)
                itr++;

            container.erase(itr);
        }
    } // namespace detail

    template<typename ContainerType>
    void input_range_type(const std::string& type, Module& m) {
        detail::input_range_type_impl<Bidir_Range<ContainerType, typename ContainerType::iterator>>(type, m);
        detail::input_range_type_impl<Bidir_Range<const ContainerType, typename ContainerType::const_iterator>>("Const_" + type, m);
    }

    /// Add random_access_container concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/RandomAccessContainer.html
    template<typename ContainerType>
    void random_access_container_type(const std::string& /*type*/, Module& m) {
        // In the interest of runtime safety for the m, we prefer the at() method for [] access,
        // to throw an exception in an out of bounds condition.
        m.add(fun([](ContainerType& c, int index) -> typename ContainerType::reference {
            /// \todo we are preferring to keep the key as 'int' to avoid runtime conversions
            /// during dispatch. reevaluate
            return c.at(static_cast<typename ContainerType::size_type>(index));
            }),
            "[]");

        m.add(fun([](const ContainerType& c, int index) -> typename ContainerType::const_reference {
            /// \todo we are preferring to keep the key as 'int' to avoid runtime conversions
            /// during dispatch. reevaluate
            return c.at(static_cast<typename ContainerType::size_type>(index));
            }),
            "[]");
    }

    /// Add assignable concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/Assignable.html
    template<typename ContainerType>
    void assignable_type(const std::string& type, Module& m) {
        copy_constructor<ContainerType>(type, m);
        operators::assign<ContainerType>(m);
    }

    /// Add container resize concept to the given ContainerType
    /// http://www.cplusplus.com/reference/stl/
    template<typename ContainerType>
    void resizable_type(const std::string& /*type*/, Module& m) {
        m.add(fun([](ContainerType* a, typename ContainerType::size_type n, const typename ContainerType::value_type& val) {
            return a->resize(n, val);
            }),
            "resize");
        m.add(fun([](ContainerType* a, typename ContainerType::size_type n) { return a->resize(n); }), "resize");
    }

    /// Add container reserve concept to the given ContainerType
    /// http://www.cplusplus.com/reference/stl/
    template<typename ContainerType>
    void reservable_type(const std::string& /*type*/, Module& m) {
        m.add(fun([](ContainerType* a, typename ContainerType::size_type n) { return a->reserve(n); }), "reserve");
        m.add(fun([](const ContainerType* a) { return a->capacity(); }), "capacity");
    }

    /// Add container concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/Container.html
    template<typename ContainerType>
    void container_type(const std::string& /*type*/, Module& m) {
        m.add(fun([](const ContainerType* a) { return a->size(); }), "size");
        m.add(fun([](const ContainerType* a) { return a->empty(); }), "empty");
        m.add(fun([](ContainerType* a) { a->clear(); }), "clear");
    }

    /// Add default constructable concept to the given Type
    /// http://www.sgi.com/tech/stl/DefaultConstructible.html
    template<typename Type>
    void default_constructible_type(const std::string& type, Module& m) {
        m.add(constructor<Type()>(), type);
    }

    /// Add sequence concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/Sequence.html
    template<typename ContainerType>
    void sequence_type(const std::string& /*type*/, Module& m) {
        m.add(fun(&detail::insert_at<ContainerType>), []() -> std::string {
            if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value)) {
                return "insert_ref_at";
            }
            else {
                return "insert_at";
            }
            }());

        m.add(fun(&detail::erase_at<ContainerType>), "erase_at");
    }

    /// Add back insertion sequence concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/BackInsertionSequence.html
    template<typename ContainerType>
    void back_insertion_sequence_type(const std::string& type, Module& m) {
        m.add(fun([](ContainerType& container) -> decltype(auto) {
            if (container.empty()) {
                throw std::range_error("Container empty");
            }
            else {
                return (container.back());
            }
            }),
            "back");
        m.add(fun([](const ContainerType& container) -> decltype(auto) {
            if (container.empty()) {
                throw std::range_error("Container empty");
            }
            else {
                return (container.back());
            }
            }),
            "back");

        using push_back = void (ContainerType::*)(const typename ContainerType::value_type&);
        m.add(fun(static_cast<push_back>(&ContainerType::push_back)), [&]() -> std::string {
            // m.add(chaiscript::fun([](const typename ContainerType& a) { return user_type<typename ContainerType::value_type>(); }), "value_type");
            if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value)) {
#if 0

                m.eval("# Pushes the second value onto the container while making a clone of the value\n"
                    "def push_back("
                    + type
                    + " container, x)\n"
                    "{ \n"
                    "  if (x.is_var_return_value()) {\n"
                    "    x.reset_var_return_value() \n"
                    "    container.push_back_ref(x) \n"
                    "  } else { \n"
                    "    container.push_back_ref(clone(x)); \n"
                    "  }\n"
                    "} \n");

                return "push_back_ref";
#else
                /*
                m.eval("# Pushes the second value onto the container\n"
                    "def push_back_ref("
                    + type
                    + " container, x)\n"
                    "{ \n"
                    "    if (x.is_var_return_value()) { x.reset_var_return_value(); } \n"
                    "    if (x.is_var_const()) { container.__push_back_ref_impl(x.clone()); } \n"
                    "    else { container.__push_back_ref_impl(x); } \n"
                    "} \n");
                m.eval("# Pushes the second value onto the container while making a clone of the value\n"
                    "def push_back("
                    + type
                    + " container, x)\n"
                    "{ \n"
                    "   container.push_back_ref(x.clone()); \n"
                    "} \n");
                return "__push_back_ref_impl";
                */
                m.eval(cweeStr::printf(R"chai(
                    def %s::push_back_ref(x){
                        if (x.is_var_return_value()) { x.reset_var_return_value(); }
                        if (x.is_var_const()) { this.__push_back_ref_impl(x.clone()); }
                        else { this.__push_back_ref_impl(x); }
                    };
                )chai", type.c_str()).c_str());
                m.eval(cweeStr::printf(R"chai(
                    def %s::push_back(x){
                        this.push_back_ref(x);
                    };
                )chai", type.c_str()).c_str());
                
                return "__push_back_ref_impl";
#endif
            }
            else {
#if 0
                return "push_back";
#else
                m.eval(cweeStr::printf(R"chai(
                    def %s::push_back_ref(x){
                        this.__push_back_impl(x);
                    };
                )chai", type.c_str()).c_str());
                m.eval(cweeStr::printf(R"chai(
                    def %s::push_back(x){
                        this.__push_back_impl(x);
                    };
                )chai", type.c_str()).c_str());
                //m.eval("# Pushes the second value onto the container while making a clone of the value\n"
                //    "def push_back_ref("
                //    + type
                //    + " container, x)\n"
                //    "{ \n"
                //    "    container.push_back(x); \n"
                //    "} \n");
                return "__push_back_impl";
#endif
            }
        }());

        m.add(fun(&ContainerType::pop_back), "pop_back");
    }

    /// Front insertion sequence
    /// http://www.sgi.com/tech/stl/FrontInsertionSequence.html
    template<typename ContainerType>
    void front_insertion_sequence_type(const std::string& type, Module& m) {
        using push_ptr = void (ContainerType::*)(typename ContainerType::const_reference);
        using pop_ptr = void (ContainerType::*)();

        m.add(fun([](ContainerType& container) -> decltype(auto) {
            if (container.empty()) {
                throw std::range_error("Container empty");
            }
            else {
                return (container.front());
            }
            }),
            "front");

        m.add(fun([](const ContainerType& container) -> decltype(auto) {
            if (container.empty()) {
                throw std::range_error("Container empty");
            }
            else {
                return (container.front());
            }
            }),
            "front");

        m.add(fun(static_cast<push_ptr>(&ContainerType::push_front)), [&]() -> std::string {
            if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value)) {
                m.eval("# Pushes the second value onto the front of the container while making a clone of the value\n"
                    "def push_front("
                    + type
                    + " container, x)\n"
                    "{ \n"
                    "   container.push_front_ref(x.clone()); \n"
                    "} \n");
                m.eval("# Pushes the second value onto the front of the container\n"
                    "def push_front_ref("
                    + type
                    + " container, x)\n"
                    "{ \n"
                    "    if (x.is_var_return_value()) { x.reset_var_return_value(); } \n"
                    "    if (x.is_var_const()) { container.__push_front_ref_impl(x.clone()); } \n"
                    "    else { container.__push_front_ref_impl(x); } \n"
                    "} \n");
                return "__push_front_ref_impl";
            }
            else {
                m.eval("# Pushes the second value onto the container while making a clone of the value\n"
                    "def push_front_ref("
                    + type
                    + " container, x)\n"
                    "{ \n"
                    "    container.push_front(x); \n"
                    "} \n");
                return "push_front";
            }
        }());

        m.add(fun(static_cast<pop_ptr>(&ContainerType::pop_front)), "pop_front");
    }

    /// bootstrap a given PairType
    /// http://www.sgi.com/tech/stl/pair.html
    template<typename PairType>
    void pair_type(const std::string& type, Module& m) {
        m.add(user_type<PairType>(), type);

        m.add(fun(&PairType::first), "first");
        m.add(fun(&PairType::second), "second");

        basic_constructors<PairType>(type, m);
        m.add(constructor<PairType(const typename PairType::first_type&, const typename PairType::second_type&)>(), type);
    }

    /// Add pair associative container concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/PairAssociativeContainer.html

    template<typename ContainerType>
    void pair_associative_container_type(const std::string& type, Module& m) {
        pair_type<typename ContainerType::value_type>(type + "_Pair", m);
    }

    /// Add unique associative container concept to the given ContainerType
    /// http://www.sgi.com/tech/stl/UniqueAssociativeContainer.html
    template<typename ContainerType>
    void unique_associative_container_type(const std::string& /*type*/, Module& m) {
        m.add(fun(detail::count<ContainerType>), "count");

        m.add(fun(detail::contains<ContainerType>), "contains");

        m.add(fun(detail::keys<ContainerType>), "keys");

        using erase_ptr = size_t(ContainerType::*)(const typename ContainerType::key_type&);

        m.add(fun(static_cast<erase_ptr>(&ContainerType::erase)), "erase");

        m.add(fun(&detail::insert<ContainerType>), "insert");

        m.add(fun(&detail::insert_ref<ContainerType>), []() -> std::string {
            if (typeid(typename ContainerType::mapped_type) == typeid(Boxed_Value)) {
                return "insert_ref";
            }
            else {
                return "insert";
            }
            }());
    }

    /// Add a MapType container
    /// http://www.sgi.com/tech/stl/Map.html
    template<typename MapType>
    void map_type(const std::string& type, Module& m) {
        m.add(user_type<MapType>(), type);

        using elem_access = typename MapType::mapped_type& (MapType::*)(const typename MapType::key_type&);
        using const_elem_access = const typename MapType::mapped_type& (MapType::*)(const typename MapType::key_type&) const;

        m.add(fun(static_cast<elem_access>(&MapType::operator[])), "[]");
        m.add(fun([](const MapType& container, const typename MapType::key_type& key) -> decltype(auto) {
            return container.at(key);
            }), "[]");


        m.add(fun(static_cast<elem_access>(&MapType::at)), "at");
        m.add(fun(static_cast<const_elem_access>(&MapType::at)), "at");

        if (typeid(MapType) == typeid(std::map<std::string, Boxed_Value>)) {
            m.eval(R"(
                    def Map::`==`(Map rhs) {
                       if ( rhs.size() != this.size() ) {
                         return false;
                       } else {
                         auto r1 = range(this);
                         auto r2 = range(rhs);
                         while (!r1.empty())
                         {
                           if (!eq(r1.front().first, r2.front().first) || !eq(r1.front().second, r2.front().second))
                           {
                             return false;
                           }
                           r1.pop_front();
                           r2.pop_front();
                         }
                         true;
                       }
                   } )");
        }

        container_type<MapType>(type, m);
        default_constructible_type<MapType>(type, m);
        assignable_type<MapType>(type, m);
        unique_associative_container_type<MapType>(type, m);
        pair_associative_container_type<MapType>(type, m);
        input_range_type<MapType>(type, m);
    }

    /// http://www.sgi.com/tech/stl/List.html
    template<typename ListType>
    void list_type(const std::string& type, Module& m) {
        m.add(user_type<ListType>(), type);

        front_insertion_sequence_type<ListType>(type, m);
        back_insertion_sequence_type<ListType>(type, m);
        sequence_type<ListType>(type, m);
        resizable_type<ListType>(type, m);
        container_type<ListType>(type, m);
        default_constructible_type<ListType>(type, m);
        assignable_type<ListType>(type, m);
        input_range_type<ListType>(type, m);
    }

    /// Create a vector type with associated concepts
    /// http://www.sgi.com/tech/stl/Vector.html
    template<typename VectorType>
    void vector_type(const std::string& type, Module& m) {
        m.add(user_type<VectorType>(), type);
        m.add(chaiscript::containerValueType(user_type<VectorType>(), user_type<typename VectorType::value_type>()));

        m.add(fun([](VectorType& container) -> decltype(auto) {
            if (container.empty()) {
                throw std::range_error("Container empty");
            }
            else {
                return (container.front());
            }
            }),
            "front");

        m.add(fun([](const VectorType& container) -> decltype(auto) {
            if (container.empty()) {
                throw std::range_error("Container empty");
            }
            else {
                return (container.front());
            }
            }),
            "front");

        back_insertion_sequence_type<VectorType>(type, m);
        sequence_type<VectorType>(type, m);
        random_access_container_type<VectorType>(type, m);
        resizable_type<VectorType>(type, m);
        reservable_type<VectorType>(type, m);
        container_type<VectorType>(type, m);
        default_constructible_type<VectorType>(type, m);
        assignable_type<VectorType>(type, m);
        input_range_type<VectorType>(type, m);

        if (typeid(VectorType) == typeid(std::vector<Boxed_Value>)) {
            m.eval(R"(
                    def Vector::`==`(Vector rhs) {
                       if ( rhs.size() != this.size() ) {
                         return false;
                       } else {
                         auto r1 = range(this);
                         auto r2 = range(rhs);
                         while (!r1.empty())
                         {
                           if (!eq(r1.front(), r2.front()))
                           {
                             return false;
                           }
                           r1.pop_front();
                           r2.pop_front();
                         }
                         true;
                       }
                   } )");
        }
    }

    /// Add a String container
    /// http://www.sgi.com/tech/stl/basic_string.html
    template<typename String>
    void string_type(const std::string& type, Module& m) {
        m.add(user_type<String>(), type);
        operators::addition<String>(m);
        operators::assign_sum<String>(m);
        opers_comparison<String>(m);
        random_access_container_type<String>(type, m);
        sequence_type<String>(type, m);
        default_constructible_type<String>(type, m);
        // container_type<String>(type, m);
        assignable_type<String>(type, m);
        input_range_type<String>(type, m);

        // Special case: add push_back to string (which doesn't support other back_insertion operations
        m.add(fun(&String::push_back), []() -> std::string {
            if (typeid(typename String::value_type) == typeid(Boxed_Value)) {
                return "push_back_ref";
            }
            else {
                return "push_back";
            }
            }());

        m.add(fun([](const String* s, const String& f, size_t pos) { return s->find(f, pos); }), "find");
        m.add(fun([](const String* s, const String& f, size_t pos) { return s->rfind(f, pos); }), "rfind");
        m.add(fun([](const String* s, const String& f, size_t pos) { return s->find_first_of(f, pos); }), "find_first_of");
        m.add(fun([](const String* s, const String& f, size_t pos) { return s->find_last_of(f, pos); }), "find_last_of");
        m.add(fun([](const String* s, const String& f, size_t pos) { return s->find_last_not_of(f, pos); }), "find_last_not_of");
        m.add(fun([](const String* s, const String& f, size_t pos) { return s->find_first_not_of(f, pos); }), "find_first_not_of");

        m.add(fun([](String* s, typename String::value_type c) -> decltype(auto) { return (*s += c); }), "+=");

        m.add(fun([](String* s) { s->clear(); }), "clear");
        m.add(fun([](const String* s) { return s->empty(); }), "empty");
        m.add(fun([](const String* s) { return s->size(); }), "size");

        m.add(fun([](const String* s) { return s->c_str(); }), "c_str");
        m.add(fun([](const String* s) { return s->data(); }), "data");
        m.add(fun([](const String* s, size_t pos, size_t len) { return s->substr(pos, len); }), "substr");
    }

    /// Add a MapType container
    /// http://www.sgi.com/tech/stl/Map.html
    template<typename FutureType>
    void future_type(const std::string& type, Module& m) {
        m.add(user_type<FutureType>(), type);

        m.add(fun([](const FutureType& t) { return t.valid(); }), "valid");
        m.add(fun([](FutureType& t) { return t.get(); }), "get");
        m.add(fun(&FutureType::wait), "wait");
    }
} // namespace chaiscript::bootstrap::standard_library
