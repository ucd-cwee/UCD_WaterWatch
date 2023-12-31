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

namespace chaiscript {
    namespace exception {
        /// \brief Error thrown when there's a problem with type conversion
        class conversion_error : public bad_boxed_cast {
        public:
            conversion_error(const Type_Info t_to, const Type_Info t_from, const utility::Static_String what) noexcept
                : bad_boxed_cast(t_from, (*t_to.bare_type_info()), what)
                , type_to(t_to) {
            }

            Type_Info type_to;
        };

        class bad_boxed_dynamic_cast : public bad_boxed_cast {
        public:
            bad_boxed_dynamic_cast(const Type_Info& t_from, const std::type_info& t_to, const utility::Static_String& t_what) noexcept
                : bad_boxed_cast(t_from, t_to, t_what) {
            }

            bad_boxed_dynamic_cast(const Type_Info& t_from, const std::type_info& t_to) noexcept
                : bad_boxed_cast(t_from, t_to) {
            }

            explicit bad_boxed_dynamic_cast(const utility::Static_String& w) noexcept
                : bad_boxed_cast(w) {
            }

            bad_boxed_dynamic_cast(const bad_boxed_dynamic_cast&) = default;

            ~bad_boxed_dynamic_cast() noexcept override = default;
        };

        class bad_boxed_type_cast : public bad_boxed_cast {
        public:
            bad_boxed_type_cast(const Type_Info& t_from, const std::type_info& t_to, const utility::Static_String& t_what) noexcept
                : bad_boxed_cast(t_from, t_to, t_what) {
            }

            bad_boxed_type_cast(const Type_Info& t_from, const std::type_info& t_to) noexcept
                : bad_boxed_cast(t_from, t_to) {
            }

            explicit bad_boxed_type_cast(const utility::Static_String& w) noexcept
                : bad_boxed_cast(w) {
            }

            bad_boxed_type_cast(const bad_boxed_type_cast&) = default;

            ~bad_boxed_type_cast() noexcept override = default;
        };
    } // namespace exception

    namespace detail {
        class Type_Conversion_Base {
        public:
            virtual Boxed_Value convert(const Boxed_Value& from) const = 0;
            virtual Boxed_Value convert_down(const Boxed_Value& to) const = 0;

            const Type_Info& to() const noexcept { return m_to; }
            const Type_Info& from() const noexcept { return m_from; }

            virtual bool bidir() const noexcept { return true; }
            virtual bool polymorphic() const noexcept { return false; }

            virtual ~Type_Conversion_Base() = default;

        protected:
            Type_Conversion_Base(Type_Info t_to, Type_Info t_from)
                : m_to(std::move(t_to))
                , m_from(std::move(t_from)) {
            }

        private:
            const Type_Info m_to;
            const Type_Info m_from;
        };

        template<typename From, typename To>
        class Static_Caster {
        public:
            static Boxed_Value cast(const Boxed_Value& t_from) {
                if (t_from.get_type_info().bare_equal(chaiscript::user_type<From>())) {
                    if (t_from.is_pointer()) {
                        // Dynamic cast out the contained boxed value, which we know is the type we want
                        if (t_from.is_const()) {
                            return Boxed_Value([&]() {
                                if (auto data
                                    = chaiscript::static_pointer_cast<const To>(detail::Cast_Helper<chaiscript::shared_ptr<const From>>::cast(t_from, nullptr))) {
                                    return data;
                                }
                                else {
                                    throw std::bad_cast();
                                }
                                }());
                        }
                        else {
                            return Boxed_Value([&]() {
                                if (auto data = chaiscript::static_pointer_cast<To>(detail::Cast_Helper<chaiscript::shared_ptr<From>>::cast(t_from, nullptr))) {
                                    return data;
                                }
                                else {
                                    throw std::bad_cast();
                                }
                                }());
                        }
                    }
                    else {
                        // Pull the reference out of the contained boxed value, which we know is the type we want
                        if (t_from.is_const()) {
                            const From& d = detail::Cast_Helper<const From&>::cast(t_from, nullptr);
                            const To& data = static_cast<const To&>(d);
                            return Boxed_Value(std::cref(data));
                        }
                        else {
                            From& d = detail::Cast_Helper<From&>::cast(t_from, nullptr);
                            To& data = static_cast<To&>(d);
                            return Boxed_Value(std::ref(data));
                        }
                    }
                }
                else {
                    throw chaiscript::exception::bad_boxed_dynamic_cast(t_from.get_type_info(), typeid(To), "Unknown dynamic_cast_conversion");
                }
            }
        };

        template<typename From, typename To>
        class Dynamic_Caster {
        public:
            static Boxed_Value cast(const Boxed_Value& t_from) {
                if (t_from.get_type_info().bare_equal(chaiscript::user_type<From>())) {
                    if (t_from.is_pointer()) {
                        // Dynamic cast out the contained boxed value, which we know is the type we want
                        if (t_from.is_const()) {
                            return Boxed_Value([&]() {
                                if (auto data
                                    = chaiscript::dynamic_shared_ptr_cast<const To>(detail::Cast_Helper<chaiscript::shared_ptr<const From>>::cast(t_from, nullptr))) {
                                    return data;
                                }
                                else {
                                    throw std::bad_cast();
                                }
                                }());
                        }
                        else {
                            return Boxed_Value([&]() {
                                if (auto data = chaiscript::dynamic_shared_ptr_cast<To>(detail::Cast_Helper<chaiscript::shared_ptr<From>>::cast(t_from, nullptr))) {
                                    return data;
                                }
                                else {
#ifdef CHAISCRIPT_LIBCPP
                                    /// \todo fix this someday after libc++ is fixed.
                                    if (std::string(typeid(To).name()).find("Assignable_Proxy_Function") != std::string::npos) {
                                        auto from = detail::Cast_Helper<chaiscript::shared_ptr<From>>::cast(t_from, nullptr);
                                        if (std::string(typeid(*from).name()).find("Assignable_Proxy_Function_Impl") != std::string::npos) {
                                            return chaiscript::static_pointer_cast<To>(from);
                                        }
                                    }
#endif
                                    throw std::bad_cast();
                                }
                                }());
                        }
                    }
                    else {
                        // Pull the reference out of the contained boxed value, which we know is the type we want
                        if (t_from.is_const()) {
                            const From& d = detail::Cast_Helper<const From&>::cast(t_from, nullptr);
                            const To& data = dynamic_cast<const To&>(d);
                            return Boxed_Value(std::cref(data));
                        }
                        else {
                            From& d = detail::Cast_Helper<From&>::cast(t_from, nullptr);
                            To& data = dynamic_cast<To&>(d);
                            return Boxed_Value(std::ref(data));
                        }
                    }
                }
                else {
                    throw chaiscript::exception::bad_boxed_dynamic_cast(t_from.get_type_info(), typeid(To), "Unknown dynamic_cast_conversion");
                }
            }
        };

        template<typename From, typename To>
        class Value_Caster {
        public:
            static Boxed_Value cast(const Boxed_Value& t_from) {
                if (t_from.get_type_info().bare_equal(chaiscript::user_type<From>())) {
                    if (t_from.is_pointer()) {
                        // Dynamic cast out the contained boxed value, which we know is the type we want
                        if (t_from.is_const()) {
                            AUTO dPtr = detail::Cast_Helper<chaiscript::shared_ptr<const From>>::cast(t_from, nullptr);
                            const From& d = *dPtr;
                            return Boxed_Value((To)(d));
                        }
                        else {
                            AUTO dPtr = detail::Cast_Helper<chaiscript::shared_ptr<From>>::cast(t_from, nullptr);
                            From& d = *dPtr;
                            return Boxed_Value((To)(d));
                        }
                    }
                    else {
                        // Pull the reference out of the contained boxed value, which we know is the type we want
                        if (t_from.is_const()) {
                            const From& d = detail::Cast_Helper<const From&>::cast(t_from, nullptr);
                            return Boxed_Value((To)(d));
                        }
                        else {
                            From& d = detail::Cast_Helper<From&>::cast(t_from, nullptr);
                            return Boxed_Value((To)(d));
                        }
                    }
                }
                else {
                    throw chaiscript::exception::bad_boxed_dynamic_cast(t_from.get_type_info(), typeid(To), "Unknown dynamic_cast_conversion");
                }
            }
        };


        template<typename Base, typename Derived>
        class Dynamic_Conversion_Impl : public Type_Conversion_Base {
        public:
            Dynamic_Conversion_Impl()
                : Type_Conversion_Base(chaiscript::user_type<Base>(), chaiscript::user_type<Derived>()) {
            }

            bool polymorphic() const noexcept override { return true; }

            Boxed_Value convert_down(const Boxed_Value& t_base) const override { return Dynamic_Caster<Base, Derived>::cast(t_base); }

            Boxed_Value convert(const Boxed_Value& t_derived) const override { return Static_Caster<Derived, Base>::cast(t_derived); }
        };

        template<typename Base, typename Derived>
        class Static_Conversion_Impl : public Type_Conversion_Base {
        public:
            Static_Conversion_Impl()
                : Type_Conversion_Base(chaiscript::user_type<Base>(), chaiscript::user_type<Derived>()) {
            }

            Boxed_Value convert_down(const Boxed_Value& t_base) const override {
                throw chaiscript::exception::bad_boxed_dynamic_cast(t_base.get_type_info(),
                    typeid(Derived),
                    "Unable to cast down inheritance hierarchy with non-polymorphic types");
            }

            bool bidir() const noexcept override { return false; }

            Boxed_Value convert(const Boxed_Value& t_derived) const override { return Static_Caster<Derived, Base>::cast(t_derived); }
        };

        template<typename Base, typename Derived>
        class Value_Conversion_Impl : public Type_Conversion_Base {
        public:
            Value_Conversion_Impl()
                : Type_Conversion_Base(chaiscript::user_type<Base>(), chaiscript::user_type<Derived>()) {
            }

            Boxed_Value convert_down(const Boxed_Value& t_base) const override { return Value_Caster<Base, Derived>::cast(t_base); }

            Boxed_Value convert(const Boxed_Value& t_derived) const override { return Value_Caster<Derived, Base>::cast(t_derived); }
        };


        template<typename Callable>
        class Type_Conversion_Impl : public Type_Conversion_Base {
        public:
            Type_Conversion_Impl(Type_Info t_from, Type_Info t_to, Callable t_func)
                : Type_Conversion_Base(t_to, t_from)
                , m_func(std::move(t_func)) {
            }

            Boxed_Value convert_down(const Boxed_Value&) const override {
                throw chaiscript::exception::bad_boxed_type_cast("No conversion exists");
            }

            Boxed_Value convert(const Boxed_Value& t_from) const override {
                /// \todo better handling of errors from the conversion function
                return m_func(t_from);
            }

            bool bidir() const noexcept override { return false; }

        private:
            Callable m_func;
        };
    } // namespace detail

    class Type_Conversions {
    public:
        class Conversion_Saves_Impl {
        public:
            bool enabled = false;
            chaiscript::small_vector<Boxed_Value> saves;
        };
        typedef cweeUnpooledInterlocked< Conversion_Saves_Impl > Conversion_Saves;

        chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> GUARD() const {
            return chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex>(m_mutex);
        };
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> SHARED_GUARD() const {
            return chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex>(m_mutex);
        };
        class Less_Than {
        public:
            bool operator()(const std::type_info* t_lhs, const std::type_info* t_rhs) const noexcept {
                if (t_lhs && t_rhs) {
                    return *t_lhs != *t_rhs && t_lhs->before(*t_rhs);
                }
                else {
                    return false;
                }
            }
        };

        Type_Conversions()
            : m_mutex()
            , m_conversions()
            , m_conversions_cache()
            , m_convertableTypes()
            , m_num_types(0) {
        }

        Type_Conversions(const Type_Conversions& t_other) = delete;
        Type_Conversions(Type_Conversions&&) = delete;

        Type_Conversions& operator=(const Type_Conversions&) = delete;
        Type_Conversions& operator=(Type_Conversions&&) = delete;

        const cweeUnpooledInterlocked<std::set<const std::type_info*, Less_Than>>& thread_cache() const {
            AUTO cache = m_thread_cache->GetExclusive();
            if (cache->size() != m_num_types) {
                AUTO shared_guard = SHARED_GUARD();
                cache = m_convertableTypes;
            }
            return *m_thread_cache;
        }

        void add_conversion(const chaiscript::shared_ptr<detail::Type_Conversion_Base>& conversion) {
            AUTO shared_guard = GUARD();
            if (find_bidir(conversion->to(), conversion->from()) != m_conversions.end()) {
                return; 
                // throw exception::conversion_error(conversion->to(), conversion->from(), "Trying to re-insert an existing conversion!");
            }
            m_conversions_cache.Clear();
            m_conversions.insert(conversion);
            m_convertableTypes.insert({ conversion->to().bare_type_info(), conversion->from().bare_type_info() });
            m_num_types = m_convertableTypes.size();
        }

        template<typename T>
        bool convertable_type() const noexcept {
            const auto type = user_type<T>().bare_type_info();
            return thread_cache().GetExclusive()->count(type) != 0;
        }

        template<typename To, typename From>
        bool converts() const noexcept {
            return converts(user_type<To>(), user_type<From>());
        }

        bool converts(const Type_Info& to, const Type_Info& from) const noexcept {
            AUTO types = thread_cache().GetExclusive();
            if (types->count(to.bare_type_info()) != 0 && types->count(from.bare_type_info()) != 0) {
                return has_conversion(to, from);
            }
            else {
                return false;
            }
        }

        bool converts_polymorphic(const Type_Info& to, const Type_Info& from) const noexcept {
            AUTO types = thread_cache().GetExclusive();
            if (types->count(to.bare_type_info()) != 0 && types->count(from.bare_type_info()) != 0) {
                return can_polymorphic_cast(to, from);
            }
            else {
                return false;
            }
        };

        template<typename To>
        Boxed_Value boxed_type_conversion(Conversion_Saves& t_saves, const Boxed_Value& from) const {
            return boxed_type_conversion(user_type<To>(), t_saves, from);
        }

        template<typename From>
        Boxed_Value boxed_type_down_conversion(Conversion_Saves& t_saves, const Boxed_Value& to) const {
            return boxed_type_down_conversion(user_type<From>(), t_saves, to);
        }

        Boxed_Value boxed_type_conversion_polymorphic(const Type_Info& to, Conversion_Saves& t_saves, const Boxed_Value& from) const {
            try {
                Boxed_Value ret = get_conversion_polymorphic(to, from.get_type_info())->convert(from);
                decltype(auto) t_saves_locked = t_saves.GetExclusive();
                if (t_saves_locked->enabled) {
                    t_saves_locked->saves.push_back(ret);
                }
                return ret;
            }
            catch (const std::out_of_range&) {
                throw exception::bad_boxed_dynamic_cast(from.get_type_info(), *to.bare_type_info(), "No known polymorphic conversion");
            }
            catch (const std::bad_cast&) {
                throw exception::bad_boxed_dynamic_cast(from.get_type_info(), *to.bare_type_info(), "Unable to perform dynamic_cast operation");
            }
        }

        Boxed_Value boxed_type_conversion(const Type_Info& to, Conversion_Saves& t_saves, const Boxed_Value& from) const {
            try {
                Boxed_Value ret = get_conversion(to, from.get_type_info())->convert(from);
                decltype(auto) t_saves_locked = t_saves.GetExclusive();
                if (t_saves_locked->enabled) {
                    t_saves_locked->saves.push_back(ret);
                }
                return ret;
            }
            catch (const std::out_of_range&) {
                throw exception::bad_boxed_dynamic_cast(from.get_type_info(), *to.bare_type_info(), "No known conversion");
            }
            catch (const std::bad_cast&) {
                throw exception::bad_boxed_dynamic_cast(from.get_type_info(), *to.bare_type_info(), "Unable to perform dynamic_cast operation");
            }
        }

        Boxed_Value boxed_type_down_conversion(const Type_Info& from, Conversion_Saves& t_saves, const Boxed_Value& to) const {
            try {
                Boxed_Value ret = get_conversion(to.get_type_info(), from)->convert_down(to);
                decltype(auto) t_saves_locked = t_saves.GetExclusive();
                if (t_saves_locked->enabled) {
                    t_saves_locked->saves.push_back(ret);
                }
                return ret;
            }
            catch (const std::out_of_range&) {
                throw exception::bad_boxed_dynamic_cast(to.get_type_info(), *from.bare_type_info(), "No known conversion");
            }
            catch (const std::bad_cast&) {
                throw exception::bad_boxed_dynamic_cast(to.get_type_info(), *from.bare_type_info(), "Unable to perform dynamic_cast operation");
            }
        }

        static void enable_conversion_saves(Conversion_Saves& t_saves, bool t_val) { t_saves.GetExclusive()->enabled = t_val; }

        chaiscript::small_vector<Boxed_Value> take_saves(Conversion_Saves& t_saves) {
            decltype(auto) locked = t_saves.GetExclusive();
            chaiscript::small_vector<Boxed_Value> ret = locked->saves;
            locked->saves = chaiscript::small_vector<Boxed_Value>();
            return ret;
        }

        bool has_conversion(const Type_Info& to, const Type_Info& from) const {
            AUTO shared_guard = SHARED_GUARD();
            return find_bidir(to, from) != m_conversions.end();
        }
        bool can_polymorphic_cast(const Type_Info& to, const Type_Info& from) const {
            AUTO shared_guard = SHARED_GUARD();
            return find_polymorphic_bidir(to, from) != m_conversions.end();
        }

        chaiscript::shared_ptr<detail::Type_Conversion_Base> get_conversion(const Type_Info& to, const Type_Info& from) const {
            AUTO shared_guard = SHARED_GUARD();

            const auto itr = find(to, from);

            if (itr != m_conversions.end()) {
                return *itr;
            }
            else {
                throw std::out_of_range(std::string("No such conversion exists from ") + from.bare_name() + " to " + to.bare_name());
            }
        }

        chaiscript::shared_ptr<detail::Type_Conversion_Base> get_conversion_polymorphic(const Type_Info& to, const Type_Info& from) const {
            AUTO shared_guard = SHARED_GUARD();

            const auto itr = find_polymorphic_bidir(to, from);

            if (itr != m_conversions.end()) {
                return *itr;
            }
            else {
                throw std::out_of_range(std::string("No such polymorphic conversion exists from ") + from.bare_name() + " to " + to.bare_name());
            }
        }

        Conversion_Saves& conversion_saves() const noexcept { return *m_conversion_saves; }
        //cweeUnpooledInterlocked<std::set<const std::type_info*, Less_Than>> &thread_cache() const noexcept { return *m_thread_cache; }

        std::set<chaiscript::shared_ptr<detail::Type_Conversion_Base>>::const_iterator find_bidir(const Type_Info& to, const Type_Info& from) const {
            AUTO map_of_tos = m_conversions_cache.GetPtr(from.bare_type_info());
            if (map_of_tos) {
                if (map_of_tos->Check(to.bare_type_info())) {
                    return *map_of_tos->GetPtr(to.bare_type_info());
                } else {
                    AUTO iterator = std::find_if(m_conversions.begin(),
                        m_conversions.end(),
                        [&to, &from](const chaiscript::shared_ptr<detail::Type_Conversion_Base>& conversion) -> bool {
                            return (conversion->to().bare_equal(to) && conversion->from().bare_equal(from))
                                || (conversion->bidir() && conversion->from().bare_equal(to) && conversion->to().bare_equal(from));
                        });

                    map_of_tos->Emplace(to.bare_type_info(), iterator);
                    return iterator;
                }
            }
            else {
                throw std::out_of_range(std::string("No m_conversions_cache could be created for the conversion from ") + from.bare_name() + " to " + to.bare_name());
            }
        }

        std::set<chaiscript::shared_ptr<detail::Type_Conversion_Base>>::const_iterator find_polymorphic_bidir(const Type_Info& to, const Type_Info& from) const {
            AUTO map_of_tos = m_conversions_cache.GetPtr(from.bare_type_info());
            if (map_of_tos) {
                if (map_of_tos->Check(to.bare_type_info())) {
                    return *map_of_tos->GetPtr(to.bare_type_info());
                }
                else {
                    AUTO iterator = std::find_if(m_conversions.begin(),
                        m_conversions.end(),
                        [&to, &from](const chaiscript::shared_ptr<detail::Type_Conversion_Base>& conversion) -> bool {
                            return (conversion->polymorphic() && conversion->to().bare_equal(to) && conversion->from().bare_equal(from))
                                || (conversion->polymorphic() && conversion->bidir() && conversion->from().bare_equal(to) && conversion->to().bare_equal(from));
                        });

                    map_of_tos->Emplace(to.bare_type_info(), iterator);
                    return iterator;
                }
            }
            else {
                throw std::out_of_range(std::string("No m_conversions_cache could be created for the conversion from ") + from.bare_name() + " to " + to.bare_name());
            }
        }

        std::set<chaiscript::shared_ptr<detail::Type_Conversion_Base>>::const_iterator find(const Type_Info& to, const Type_Info& from) const {
            AUTO map_of_tos = m_conversions_cache.GetPtr(from.bare_type_info());
            if (map_of_tos) {
                if (map_of_tos->Check(to.bare_type_info())) {
                    return *map_of_tos->GetPtr(to.bare_type_info());
                }
                else {
                    AUTO iterator = std::find_if(m_conversions.begin(),
                        m_conversions.end(),
                        [&to, &from](const chaiscript::shared_ptr<detail::Type_Conversion_Base>& conversion) {
                            return conversion->to().bare_equal(to) && conversion->from().bare_equal(from);
                        });

                    map_of_tos->Emplace(to.bare_type_info(), iterator);
                    return iterator;
                }
            }
            else {
                throw std::out_of_range(std::string("No m_conversions_cache could be created for the conversion from ") + from.bare_name() + " to " + to.bare_name());
            }
        }

        const std::set<chaiscript::shared_ptr<detail::Type_Conversion_Base>>& get_conversions() const {
            AUTO shared_guard = SHARED_GUARD();
            return m_conversions;
        }
    private:
        mutable chaiscript::detail::threading::shared_mutex m_mutex;
        std::set<chaiscript::shared_ptr<detail::Type_Conversion_Base>> m_conversions;
        cweeThreadedMap<const std::type_info*, cweeThreadedMap<const std::type_info*, decltype(Type_Conversions::m_conversions)::const_iterator>> m_conversions_cache;

        std::set<const std::type_info*, Less_Than> m_convertableTypes;
        std::atomic_size_t m_num_types;
        mutable chaiscript::detail::threading::Thread_Storage< cweeUnpooledInterlocked<std::set<const std::type_info*, Less_Than>> > m_thread_cache; // thread-safe
        mutable chaiscript::detail::threading::Thread_Storage< Conversion_Saves > m_conversion_saves; // thread-safe
    };

    class Type_Conversions_State {
    public:
        Type_Conversions_State(const Type_Conversions& t_conversions, Type_Conversions::Conversion_Saves& t_saves)
            : m_conversions(t_conversions)
            , m_saves(t_saves) {
        }

        const Type_Conversions* operator->() const noexcept { return &m_conversions.get(); }

        const Type_Conversions* get() const noexcept { return &m_conversions.get(); }

        Type_Conversions::Conversion_Saves& saves() const noexcept { return m_saves; }

    private:
        std::reference_wrapper<const Type_Conversions> m_conversions;
        std::reference_wrapper<Type_Conversions::Conversion_Saves> m_saves;
    };

    using Type_Conversion = chaiscript::shared_ptr<chaiscript::detail::Type_Conversion_Base>;

    /// \brief Used to register a to / parent class relationship with ChaiScript. Necessary if you
    ///        want automatic conversions up your inheritance hierarchy.
    ///
    /// Create a new to class registration for applying to a module or to the ChaiScript engine
    /// Currently, due to limitations in module loading on Windows, and for the sake of portability,
    /// if you have a type that is introduced in a loadable module and is used by multiple modules
    /// (through a tertiary dll that is shared between the modules, static linking the new type
    /// into both loadable modules would not be portable), you need to register the type
    /// relationship in all modules that use the newly added type in a polymorphic way.
    ///
    /// Example:
    /// \code
    /// class Base
    /// {};
    /// class Derived : public Base
    /// {};
    ///
    /// chaiscript::ChaiScript chai;
    /// chai.add(chaiscript::to_class<Base, Derived>());
    /// \endcode
    ///
    template<typename Base, typename Derived>
    Type_Conversion base_class() {
        // Can only be used with related polymorphic types
        // static_assert(std::is_base_of<Base, Derived>::value, "Classes are not related by inheritance");

        if constexpr (std::is_polymorphic<Base>::value && std::is_polymorphic<Derived>::value && std::is_base_of<Base, Derived>::value) {
            return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Dynamic_Conversion_Impl<Base, Derived>>();
        }
        else {
            return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Static_Conversion_Impl<Base, Derived>>();
        }
    }

    template<typename Base, typename Derived>
    Type_Conversion castable_class() {
        // Can only be used with related polymorphic types
        // static_assert(std::is_base_of<Base, Derived>::value, "Classes are not related by inheritance");

        if constexpr (std::is_polymorphic<Base>::value && std::is_polymorphic<Derived>::value && std::is_base_of<Base, Derived>::value) {
            return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Dynamic_Conversion_Impl<Base, Derived>>();
        }
        else {
            return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Value_Conversion_Impl<Base, Derived>>();
        }
    }

    template<typename Callable>
    Type_Conversion type_conversion(const Type_Info& t_from, const Type_Info& t_to, const Callable& t_func) {
        return chaiscript::dynamic_shared_ptr_cast<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<Callable>>(chaiscript::make_shared<detail::Type_Conversion_Impl<Callable>>(t_from, t_to, t_func));
        // return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<Callable>>(t_from, t_to, t_func);
    };

    template<typename From, typename To, typename Callable>
    Type_Conversion type_conversion(const Callable& t_function, const Type_Conversions_State* state) {
        auto func = [t_function, state](const Boxed_Value& t_bv) -> Boxed_Value {
            // not even attempting to call boxed_cast so that we don't get caught in some call recursion
            return chaiscript::Boxed_Value(t_function(detail::Cast_Helper<const From&>::cast(t_bv, state))); // nullptr
        };

        return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<From>(), user_type<To>(), func);
    };

    template<typename From, typename To>
    Type_Conversion type_conversion(const Type_Conversions_State* state) {
        static_assert(std::is_convertible<From, To>::value, "Types are not automatically convertible");
        auto func = [state](const Boxed_Value& t_bv) -> Boxed_Value {

            // not even attempting to call boxed_cast so that we don't get caught in some call recursion
            return chaiscript::Boxed_Value(To(detail::Cast_Helper<From>::cast(t_bv, &state))); 
        };
        return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<From>(), user_type<To>(), func);
    };

    template<typename To>
    Type_Conversion vector_conversion(const Type_Conversions_State* state) {
        auto func = [](const Boxed_Value& t_bv, Type_Conversions_State const& state2) -> Boxed_Value {            
            const chaiscript::small_vector<Boxed_Value>& from_vec = detail::Cast_Helper<const chaiscript::small_vector<Boxed_Value>&>::cast(t_bv, &state2); 

            To vec;
            vec.reserve(from_vec.size());
            for (const Boxed_Value& bv : from_vec) {
                vec.push_back(detail::Cast_Helper<typename To::value_type>::cast(bv, &state2));
            }

            return Boxed_Value(std::move(vec));
        };
        return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<chaiscript::small_vector<Boxed_Value>>(), user_type<To>(), func);
    };

    template<typename From, typename To>
    Type_Conversion vector_conversion(const Type_Conversions_State* state) {
        auto func = [state](const Boxed_Value& t_bv) -> Boxed_Value {

            const From& from_vec = detail::Cast_Helper<const From&>::cast(t_bv, state);

            To vec;
            vec.reserve(from_vec.size());
            for (const Boxed_Value& bv : from_vec) {
                vec.push_back(detail::Cast_Helper<typename To::value_type>::cast(bv, state)); 
                // vec.push_back((typename To::value_type)bv);
            }

            return Boxed_Value(std::move(vec));
        };
        return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<From>(), user_type<To>(), func);
    };

    template<typename To>
    Type_Conversion map_conversion(const Type_Conversions_State* state) {
        auto func = [state](const Boxed_Value& t_bv) -> Boxed_Value {

            const std::map<std::string, Boxed_Value>& from_map = detail::Cast_Helper<const std::map<std::string, Boxed_Value>&>::cast(t_bv, state);

            To map;
            for (const std::pair<const std::string, Boxed_Value>& p : from_map) {
                map.insert(std::make_pair(p.first, detail::Cast_Helper<typename To::mapped_type>::cast(p.second, state))); 
            }
            return Boxed_Value(std::move(map));
        };
        return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<std::map<std::string, Boxed_Value>>(), user_type<To>(), func);
    };
} // namespace chaiscript
