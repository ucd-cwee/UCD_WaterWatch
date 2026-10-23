#pragma once
#include "types.h"
#include "Parallel.h"
// #include "units.h"
// #include "../GpuProgramming/matrix.h"
// #include "constexpr_math.h"

// GL::Proxy_Functions, which wrap other functions into a shareable interface
namespace GL {
    class Function;
    class Function_Caller;

    template <typename R, typename Class, typename... T> class Const_Member_Function_Traits {
    public:
        using argType = std::tuple<T...>;
        using classType = Class const&;
        using returnType = R;
        static constexpr auto numArgs{ std::tuple_size_v< argType > };
        Const_Member_Function_Traits(R(Class::* f)(T...) const) {};
        ~Const_Member_Function_Traits() = default;

    private:
        inline static R(Class::* m_attr)(T...) const = nullptr;

    public:
        static R call(const any* begin, decltype(m_attr) const& p_attr) {
            if constexpr (numArgs >= 15) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                    begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>(), begin[15].cast<std::tuple_element_t<14, argType>>()
                    );
            }
            else if constexpr (numArgs == 14) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                    begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>()
                    );
            }
            else if constexpr (numArgs == 13) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                    begin[13].cast<std::tuple_element_t<12, argType>>()
                    );
            }
            else if constexpr (numArgs == 12) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>()
                    );
            }
            else if constexpr (numArgs == 11) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>()
                    );
            }
            else if constexpr (numArgs == 10) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>()
                    );
            }
            else if constexpr (numArgs == 9) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>()
                    );
            }
            else if constexpr (numArgs == 8) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>()
                    );
            }
            else if constexpr (numArgs == 7) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>()
                    );
            }
            else if constexpr (numArgs == 6) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>()
                    );
            }
            else if constexpr (numArgs == 5) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>()
                    );
            }
            else if constexpr (numArgs == 4) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>()
                    );
            }
            else if constexpr (numArgs == 3) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>()
                    );
            }
            else if constexpr (numArgs == 2) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>()
                    );
            }
            else if constexpr (numArgs == 1) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>()
                    );
            }
            else if constexpr (numArgs == 0) {
                return (begin[0].cast<classType>().*p_attr)(
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
        Member_Function_Traits(R(Class::* f)(T...)) {};
        ~Member_Function_Traits() = default;

    private:
        inline static R(Class::* m_attr)(T...) = nullptr;

    public:
        static R call(const any* begin, decltype(m_attr) const& p_attr) {
            if constexpr (numArgs >= 15) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                    begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>(), begin[15].cast<std::tuple_element_t<14, argType>>()
                    );
            }
            else if constexpr (numArgs == 14) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                    begin[13].cast<std::tuple_element_t<12, argType>>(), begin[14].cast<std::tuple_element_t<13, argType>>()
                    );
            }
            else if constexpr (numArgs == 13) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>(),
                    begin[13].cast<std::tuple_element_t<12, argType>>()
                    );
            }
            else if constexpr (numArgs == 12) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>(), begin[12].cast<std::tuple_element_t<11, argType>>()
                    );
            }

            else if constexpr (numArgs == 11) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>(), begin[11].cast<std::tuple_element_t<10, argType>>()
                    );
            }
            else if constexpr (numArgs == 10) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>(), begin[10].cast<std::tuple_element_t<9, argType>>()
                    );
            }
            else if constexpr (numArgs == 9) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>(),
                    begin[9].cast<std::tuple_element_t<8, argType>>()
                    );
            }
            else if constexpr (numArgs == 8) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>(), begin[8].cast<std::tuple_element_t<7, argType>>()
                    );
            }

            else if constexpr (numArgs == 7) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>(), begin[7].cast<std::tuple_element_t<6, argType>>()
                    );
            }
            else if constexpr (numArgs == 6) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>(), begin[6].cast<std::tuple_element_t<5, argType>>()
                    );
            }
            else if constexpr (numArgs == 5) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>(),
                    begin[5].cast<std::tuple_element_t<4, argType>>()
                    );
            }
            else if constexpr (numArgs == 4) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>(), begin[4].cast<std::tuple_element_t<3, argType>>()
                    );
            }

            else if constexpr (numArgs == 3) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>(), begin[3].cast<std::tuple_element_t<2, argType>>()
                    );
            }
            else if constexpr (numArgs == 2) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>(), begin[2].cast<std::tuple_element_t<1, argType>>()
                    );
            }
            else if constexpr (numArgs == 1) {
                return (begin[0].cast<classType>().*p_attr)(
                    begin[1].cast<std::tuple_element_t<0, argType>>()
                    );
            }
            else if constexpr (numArgs == 0) {
                return (begin[0].cast<classType>().*p_attr)(
                    );
            }
        };
    };
    template <typename R, typename Class> class Attribute_Function_Traits {
    public:
        using classType = Class;
        using returnType = R;
        static_assert(!std::is_same_v<void, returnType>);

        Attribute_Function_Traits(R Class::* t_attr) {};
        ~Attribute_Function_Traits() = default;

    private:
        inline static R Class::* m_attr = nullptr;

    public:
        static returnType& call(const any* begin, decltype(m_attr) const& p_attr) {
            return begin->cast<Class>().*p_attr;
        };
    };
    template <typename R, typename... T> class Static_Function_Traits {
    public:
        using argType = std::tuple<T...>;
        using returnType = R;
        static constexpr auto numArgs{ std::tuple_size_v< argType > };
        Static_Function_Traits(R(*f)(T...)) {};
        ~Static_Function_Traits() = default;

    public:
        inline static R(*m_attr)(T...) = nullptr;

    public:
        static R call(const any* begin, decltype(m_attr) const& p_attr) {
            if constexpr (numArgs >= 16) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>(), begin[16 - 1].cast<std::tuple_element_t<15, argType>>()
                    );
            }
            else if constexpr (numArgs == 15) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>()
                    );
            }
            else if constexpr (numArgs == 14) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>()
                    );
            }
            else if constexpr (numArgs == 13) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>()
                    );
            }
            else if constexpr (numArgs == 12) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>()
                    );
            }

            else if constexpr (numArgs == 11) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>()
                    );
            }
            else if constexpr (numArgs == 10) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>()
                    );
            }
            else if constexpr (numArgs == 9) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>()
                    );
            }
            else if constexpr (numArgs == 8) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>()
                    );
            }

            else if constexpr (numArgs == 7) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>()
                    );
            }
            else if constexpr (numArgs == 6) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>()
                    );
            }
            else if constexpr (numArgs == 5) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>()
                    );
            }
            else if constexpr (numArgs == 4) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>()
                    );
            }

            else if constexpr (numArgs == 3) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>()
                    );
            }
            else if constexpr (numArgs == 2) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>()
                    );
            }
            else if constexpr (numArgs == 1) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>()
                    );
            }
            else if constexpr (numArgs == 0) {
                return (*p_attr)(
                    );
            }
        };
    };
    template <typename R, typename... T> class Caching_Static_Function_Traits {
    public:
        using argType = std::tuple<T...>;
        using returnType = R;
        static constexpr auto numArgs{ std::tuple_size_v< argType > };
        Caching_Static_Function_Traits(R(*f)(T...)) {};
        ~Caching_Static_Function_Traits() = default;

    public:
        inline static R(*m_attr)(T...) = nullptr;
        
        template <size_t I>
        static decltype(auto) CacheType() {
            if constexpr (I >= numArgs) return;
            else {
                //using type = std::tuple_element_t<I, argType>;
                if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<I, argType>>, GL::any>) return GL::any();
                else if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<I, argType>>, GL::atomic_any>) return GL::atomic_any();
                else if constexpr (GL::any::DataCaster::is_SharedPtr_class<std::tuple_element_t<I, argType>>::value || GL::any::DataCaster::is_stdSharedPtr_class<std::tuple_element_t<I, argType>>::value) return std::tuple_element_t<I, argType>();
                else if constexpr (std::is_pointer_v<std::tuple_element_t<I, argType>>) return static_cast<std::decay_t<std::tuple_element_t<I, argType>>>(0); 
                else if constexpr (std::is_reference_v<std::tuple_element_t<I, argType>>) return static_cast<std::decay_t<std::tuple_element_t<I, argType>>*>(0);                
                else return static_cast<std::decay_t<std::tuple_element_t<I, argType>>*>(0);
            }
        };

        template <size_t I> 
        using cache_type = typename GL::parallel::impl::function_traits<decltype(std::function(&CacheType<I>))>::result_type;

        template <std::size_t... Is>
        static constexpr std::size_t sum_indices_impl(std::index_sequence<Is...>) {
            return (0 + ... + sizeof(typename GL::parallel::impl::function_traits<decltype(std::function(&CacheType<Is>))>::result_type)); // C++17 fold expression
        };
        template <std::size_t Is = numArgs>
        static constexpr size_t CacheSize() {
            return sum_indices_impl(std::make_index_sequence<Is>{});
        }
        static constexpr bool requires_undo_caching_impl() {
            if constexpr (numArgs > 0) {
                if constexpr ((numArgs >= 16) && !std::is_pod_v<cache_type<15>>) return true;
                else if constexpr ((numArgs >= 15) && !std::is_pod_v<cache_type<14>>) return true;
                else if constexpr ((numArgs >= 14) && !std::is_pod_v<cache_type<13>>) return true;
                else if constexpr ((numArgs >= 13) && !std::is_pod_v<cache_type<12>>) return true;
                else if constexpr ((numArgs >= 12) && !std::is_pod_v<cache_type<11>>) return true;
                else if constexpr ((numArgs >= 11) && !std::is_pod_v<cache_type<10>>) return true;
                else if constexpr ((numArgs >= 10) && !std::is_pod_v<cache_type<9>>) return true;
                else if constexpr ((numArgs >= 9) && !std::is_pod_v<cache_type<8>>) return true;
                else if constexpr ((numArgs >= 8) && !std::is_pod_v<cache_type<7>>) return true;
                else if constexpr ((numArgs >= 7) && !std::is_pod_v<cache_type<6>>) return true;
                else if constexpr ((numArgs >= 6) && !std::is_pod_v<cache_type<5>>) return true;
                else if constexpr ((numArgs >= 5) && !std::is_pod_v<cache_type<4>>) return true;
                else if constexpr ((numArgs >= 4) && !std::is_pod_v<cache_type<3>>) return true;
                else if constexpr ((numArgs >= 3) && !std::is_pod_v<cache_type<2>>) return true;
                else if constexpr ((numArgs >= 2) && !std::is_pod_v<cache_type<1>>) return true;
                else if constexpr ((numArgs >= 1) && !std::is_pod_v<cache_type<0>>) return true;
                else return false;
            }   else return false;
        };

        static constexpr bool requires_undo_caching = requires_undo_caching_impl();
        static constexpr auto cache_size_total = CacheSize();
        template<size_t skipFirstN = 0> static void perform_caching(const any* begin, char* ptr) {
            if constexpr (numArgs > 0) {
                char(&buffer)[cache_size_total] = *reinterpret_cast<char(*)[cache_size_total]>(ptr);
                if constexpr (numArgs >= 16 && skipFirstN < 16) new (static_cast<void*>(&buffer[CacheSize<15>()])) cache_type<15>(begin[15].cast<cache_type<15>>());
                if constexpr (numArgs >= 15 && skipFirstN < 15) new (static_cast<void*>(&buffer[CacheSize<14>()])) cache_type<14>(begin[14].cast<cache_type<14>>());
                if constexpr (numArgs >= 14 && skipFirstN < 14) new (static_cast<void*>(&buffer[CacheSize<13>()])) cache_type<13>(begin[13].cast<cache_type<13>>());
                if constexpr (numArgs >= 13 && skipFirstN < 13) new (static_cast<void*>(&buffer[CacheSize<12>()])) cache_type<12>(begin[12].cast<cache_type<12>>());
                if constexpr (numArgs >= 12 && skipFirstN < 12) new (static_cast<void*>(&buffer[CacheSize<11>()])) cache_type<11>(begin[11].cast<cache_type<11>>());
                if constexpr (numArgs >= 11 && skipFirstN < 11) new (static_cast<void*>(&buffer[CacheSize<10>()])) cache_type<10>(begin[10].cast<cache_type<10>>());
                if constexpr (numArgs >= 10 && skipFirstN < 10) new (static_cast<void*>(&buffer[CacheSize<9>()])) cache_type<9>(begin[9].cast<cache_type<9>>());
                if constexpr (numArgs >= 9 && skipFirstN < 9) new (static_cast<void*>(&buffer[CacheSize<8>()])) cache_type<8>(begin[8].cast<cache_type<8>>());
                if constexpr (numArgs >= 8 && skipFirstN < 8) new (static_cast<void*>(&buffer[CacheSize<7>()])) cache_type<7>(begin[7].cast<cache_type<7>>());
                if constexpr (numArgs >= 7 && skipFirstN < 7) new (static_cast<void*>(&buffer[CacheSize<6>()])) cache_type<6>(begin[6].cast<cache_type<6>>());
                if constexpr (numArgs >= 6 && skipFirstN < 6) new (static_cast<void*>(&buffer[CacheSize<5>()])) cache_type<5>(begin[5].cast<cache_type<5>>());
                if constexpr (numArgs >= 5 && skipFirstN < 5) new (static_cast<void*>(&buffer[CacheSize<4>()])) cache_type<4>(begin[4].cast<cache_type<4>>());
                if constexpr (numArgs >= 4 && skipFirstN < 4) new (static_cast<void*>(&buffer[CacheSize<3>()])) cache_type<3>(begin[3].cast<cache_type<3>>());
                if constexpr (numArgs >= 3 && skipFirstN < 3) new (static_cast<void*>(&buffer[CacheSize<2>()])) cache_type<2>(begin[2].cast<cache_type<2>>());
                if constexpr (numArgs >= 2 && skipFirstN < 2) new (static_cast<void*>(&buffer[CacheSize<1>()])) cache_type<1>(begin[1].cast<cache_type<1>>());
                if constexpr (numArgs >= 1 && skipFirstN < 1) new (static_cast<void*>(&buffer[CacheSize<0>()])) cache_type<0>(begin[0].cast<cache_type<0>>());
            }
        };
        template<size_t which = 0> static void supplement_caching(GL::any const& item, char* ptr) {
            if constexpr (numArgs > 0) {
                char(&buffer)[cache_size_total] = *reinterpret_cast<char(*)[cache_size_total]>(ptr);
                if constexpr (std::is_same_v<cache_type<which>, GL::any>)                  
                    new (static_cast<void*>(&buffer[CacheSize<which>()])) cache_type<which>(item.cast<cache_type<which>>(), true);                
                else if constexpr (std::is_same_v<cache_type<which>, GL::atomic_any>)
                    new (static_cast<void*>(&buffer[CacheSize<which>()])) cache_type<which>(item.cast<cache_type<which>>(), true);
                else 
                    new (static_cast<void*>(&buffer[CacheSize<which>()])) cache_type<which>(item.cast<cache_type<which>>());                
            }
        };
        static void undo_caching(char* ptr) {
            if constexpr (numArgs > 0) {
                char(&buffer)[cache_size_total] = *reinterpret_cast<char(*)[cache_size_total]>(ptr);
                if constexpr (numArgs >= 16) if constexpr (!std::is_pod_v<cache_type<15>>) static_cast<cache_type<15>*>(static_cast<void*>(&buffer[CacheSize<15>()]))->~cache_type<15>();
                if constexpr (numArgs >= 15) if constexpr (!std::is_pod_v<cache_type<14>>) static_cast<cache_type<14>*>(static_cast<void*>(&buffer[CacheSize<14>()]))->~cache_type<14>();
                if constexpr (numArgs >= 14) if constexpr (!std::is_pod_v<cache_type<13>>) static_cast<cache_type<13>*>(static_cast<void*>(&buffer[CacheSize<13>()]))->~cache_type<13>();
                if constexpr (numArgs >= 13) if constexpr (!std::is_pod_v<cache_type<12>>) static_cast<cache_type<12>*>(static_cast<void*>(&buffer[CacheSize<12>()]))->~cache_type<12>();
                if constexpr (numArgs >= 12) if constexpr (!std::is_pod_v<cache_type<11>>) static_cast<cache_type<11>*>(static_cast<void*>(&buffer[CacheSize<11>()]))->~cache_type<11>();
                if constexpr (numArgs >= 11) if constexpr (!std::is_pod_v<cache_type<10>>) static_cast<cache_type<10>*>(static_cast<void*>(&buffer[CacheSize<10>()]))->~cache_type<10>();
                if constexpr (numArgs >= 10) if constexpr (!std::is_pod_v<cache_type<9>>) static_cast<cache_type<9>*>(static_cast<void*>(&buffer[CacheSize<9>()]))->~cache_type<9>();
                if constexpr (numArgs >= 9) if constexpr (!std::is_pod_v<cache_type<8>>) static_cast<cache_type<8>*>(static_cast<void*>(&buffer[CacheSize<8>()]))->~cache_type<8>();
                if constexpr (numArgs >= 8) if constexpr (!std::is_pod_v<cache_type<7>>) static_cast<cache_type<7>*>(static_cast<void*>(&buffer[CacheSize<7>()]))->~cache_type<7>();
                if constexpr (numArgs >= 7) if constexpr (!std::is_pod_v<cache_type<6>>) static_cast<cache_type<6>*>(static_cast<void*>(&buffer[CacheSize<6>()]))->~cache_type<6>();
                if constexpr (numArgs >= 6) if constexpr (!std::is_pod_v<cache_type<5>>) static_cast<cache_type<5>*>(static_cast<void*>(&buffer[CacheSize<5>()]))->~cache_type<5>();
                if constexpr (numArgs >= 5) if constexpr (!std::is_pod_v<cache_type<4>>) static_cast<cache_type<4>*>(static_cast<void*>(&buffer[CacheSize<4>()]))->~cache_type<4>();
                if constexpr (numArgs >= 4) if constexpr (!std::is_pod_v<cache_type<3>>) static_cast<cache_type<3>*>(static_cast<void*>(&buffer[CacheSize<3>()]))->~cache_type<3>();
                if constexpr (numArgs >= 3) if constexpr (!std::is_pod_v<cache_type<2>>) static_cast<cache_type<2>*>(static_cast<void*>(&buffer[CacheSize<2>()]))->~cache_type<2>();
                if constexpr (numArgs >= 2) if constexpr (!std::is_pod_v<cache_type<1>>) static_cast<cache_type<1>*>(static_cast<void*>(&buffer[CacheSize<1>()]))->~cache_type<1>();
                if constexpr (numArgs >= 1) if constexpr (!std::is_pod_v<cache_type<0>>) static_cast<cache_type<0>*>(static_cast<void*>(&buffer[CacheSize<0>()]))->~cache_type<0>();
            }
        };
        template <size_t I> static decltype(auto) get_cache(char* ptr) {            
            if constexpr (I >= numArgs) return;
            else {
                char(&buffer)[cache_size_total] = *reinterpret_cast<char(*)[cache_size_total]>(ptr);
                cache_type<I>& val = *static_cast<cache_type<I>*>(static_cast<void*>(&buffer[CacheSize<I>()]));
                if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<I, argType>>, GL::any>) return val;
                else if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<I, argType>>, GL::atomic_any>) return val;
                else if constexpr (GL::any::DataCaster::is_SharedPtr_class<std::tuple_element_t<I, argType>>::value || GL::any::DataCaster::is_stdSharedPtr_class<std::tuple_element_t<I, argType>>::value) return val;
                else if constexpr (std::is_pointer_v<std::tuple_element_t<I, argType>>) return val;
                else if constexpr (std::is_reference_v<std::tuple_element_t<I, argType>>) return *val;
                else return *val;
            }
        };
    private:
        template <std::size_t... Is>
        static R cached_call_impl(char* ptr, std::index_sequence<Is...>, decltype(m_attr) const& p_attr) {
            return (*p_attr)(get_cache<Is>(ptr)...);
        }
    public:
        static R call(const any* begin, decltype(m_attr) const& p_attr) {
            if constexpr (numArgs >= 16) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>(), begin[16 - 1].cast<std::tuple_element_t<15, argType>>()
                    );
            }
            else if constexpr (numArgs == 15) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>(), begin[15 - 1].cast<std::tuple_element_t<14, argType>>()
                    );
            }
            else if constexpr (numArgs == 14) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>(), begin[14 - 1].cast<std::tuple_element_t<13, argType>>()
                    );
            }
            else if constexpr (numArgs == 13) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>(),
                    begin[13 - 1].cast<std::tuple_element_t<12, argType>>()
                    );
            }
            else if constexpr (numArgs == 12) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>(), begin[12 - 1].cast<std::tuple_element_t<11, argType>>()
                    );
            }

            else if constexpr (numArgs == 11) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>(), begin[11 - 1].cast<std::tuple_element_t<10, argType>>()
                    );
            }
            else if constexpr (numArgs == 10) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>(), begin[10 - 1].cast<std::tuple_element_t<9, argType>>()
                    );
            }
            else if constexpr (numArgs == 9) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>(),
                    begin[9 - 1].cast<std::tuple_element_t<8, argType>>()
                    );
            }
            else if constexpr (numArgs == 8) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>(), begin[8 - 1].cast<std::tuple_element_t<7, argType>>()
                    );
            }

            else if constexpr (numArgs == 7) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>(), begin[7 - 1].cast<std::tuple_element_t<6, argType>>()
                    );
            }
            else if constexpr (numArgs == 6) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>(), begin[6 - 1].cast<std::tuple_element_t<5, argType>>()
                    );
            }
            else if constexpr (numArgs == 5) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>(),
                    begin[5 - 1].cast<std::tuple_element_t<4, argType>>()
                    );
            }
            else if constexpr (numArgs == 4) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>(), begin[4 - 1].cast<std::tuple_element_t<3, argType>>()
                    );
            }

            else if constexpr (numArgs == 3) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>(), begin[3 - 1].cast<std::tuple_element_t<2, argType>>()
                    );
            }
            else if constexpr (numArgs == 2) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>(), begin[2 - 1].cast<std::tuple_element_t<1, argType>>()
                    );
            }
            else if constexpr (numArgs == 1) {
                return (*p_attr)(
                    begin[1 - 1].cast<std::tuple_element_t<0, argType>>()
                    );
            }
            else if constexpr (numArgs == 0) {
                return (*p_attr)(
                    );
            }
        };        
        static R cached_call(char* ptr, decltype(m_attr) const& p_attr) {
            return cached_call_impl(ptr, std::make_index_sequence<numArgs>{}, p_attr);
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
        bool UseSimplifiedArguments = false;

        virtual ~Function_Caller() = default;
        // implimentation-specific function that may include a reference, a shared pointer, a value-type, etc. 
        virtual void call(const any* const begin, any* const out) const = 0;
        // implimentation-specific function specifically intended copy-out the return value without wrapping it as an 'any'
        virtual void call(const any* const begin, void* out_destination, size_t out_size) const = 0;
        // returns true if the underlying, real function return of the underlying function exactly matches the provided type.
        virtual bool return_type_explicit_match(std::type_info const& match) const = 0;

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
    };

    namespace {
        template <typename Function> class Const_Member_Function_Caller final : public Function_Caller {
        public:
            using traits = decltype(Const_Member_Function_Traits(std::declval< Function>()));
            decltype(traits::m_attr) m_attr;

            Const_Member_Function_Caller(Function&& Func) : m_attr(std::forward<Function>(Func)) {
                // (void)Const_Member_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
            };
            virtual ~Const_Member_Function_Caller() = default;
            typename traits::returnType call(const any* begin) const {
                return traits::call(begin, m_attr);
            };
            void call(const any* const begin, any* const out) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out) {
#if 1
                        if ((out->uuid() & uuid::TEMPORARY) == 0) {
                            if ((out->uuid() & uuid::INV_FLAGS) > 0) {
                                auto& ref = uuid::get_uuid(out->uuid());
                                if (GL::interlocked::decrement(ref.count) == 0) {
                                    ref.count = 1;
                                    ref.data = GL::type_erasure::wrap(call(begin));
                                    const_cast<void*&>(out->ptr()) = ref.data->get();
                                    const_cast<GL::type&>(out->type()) = GL::type_of<typename traits::returnType>();
                                }
                                else {
                                    const_cast<unsigned long&>(out->uuid()) = 0;
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
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>)))
                        if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                            decltype(auto) recieved = call(begin);
                            std::memcpy(out_destination, &recieved, sizeof(std::decay_t<typename traits::returnType>));
                        }
                        else {
                            new (out_destination) std::decay_t<typename traits::returnType>(call(begin));
                        }
                    else
                        if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                            typeid(Function).name(),
                            out_size,
                            sizeof(std::decay_t<typename traits::returnType>),
                            typeid(std::decay_t<typename traits::returnType>).name()
                        ); err_str.length() > 0)
                            throw std::runtime_error(err_str.to_string());
                }
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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
            decltype(traits::m_attr) m_attr;

            Member_Function_Caller(Function&& Func) : m_attr(std::forward<Function>(Func)) {
                // (void)Member_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
            };
            virtual ~Member_Function_Caller() = default;
            typename traits::returnType call(const any* begin) const {
                return traits::call(begin, m_attr);
            };
            void call(const any* const begin, any* const out) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out) {
#if 1
                        if ((out->uuid() & uuid::TEMPORARY) == 0) {
                            if ((out->uuid() & uuid::INV_FLAGS) > 0) {
                                auto& ref = uuid::get_uuid(out->uuid());
                                if (GL::interlocked::decrement(ref.count) == 0) {
                                    ref.count = 1;
                                    ref.data = GL::type_erasure::wrap(call(begin));
                                    const_cast<void*&>(out->ptr()) = ref.data->get();
                                    const_cast<GL::type&>(out->type()) = GL::type_of<typename traits::returnType>();
                                }
                                else {
                                    const_cast<unsigned long&>(out->uuid()) = 0;
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
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>)))
                        if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                            decltype(auto) recieved = call(begin);
                            std::memcpy(out_destination, &recieved, sizeof(std::decay_t<typename traits::returnType>));
                        }
                        else {
                            new (out_destination) std::decay_t<typename traits::returnType>(call(begin));
                        }
                    else
                        if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                            typeid(Function).name(),
                            out_size,
                            sizeof(std::decay_t<typename traits::returnType>),
                            typeid(std::decay_t<typename traits::returnType>).name()
                        ); err_str.length() > 0)
                            throw std::runtime_error(err_str.to_string());
                }
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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
            decltype(traits::m_attr) m_attr;

            Const_Attribute_Function_Caller(Function&& Func) : m_attr(std::forward<Function>(Func)) {
                // (void)Attribute_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
            };
            virtual ~Const_Attribute_Function_Caller() = default;
            typename traits::returnType& call(const any* begin) const {
                return traits::call(begin, m_attr);
            };
            void call(const any* const begin, any* const out) const override {
                if (out) {
                    if constexpr (std::is_same_v<GL::any, std::decay_t<typename traits::returnType>> || std::is_same_v<any, std::decay_t<typename traits::returnType>>) {
                        *out = call(begin);
                        if (begin->type().is_const()) *out |= (GL::type::Const | GL::type::Reference);
                        else *out |= GL::type::Reference;
                    }
                    else if constexpr (std::is_pointer<typename traits::returnType>::value) {
                        *out = any::wrap_member(*begin, *call(begin));
                        if (begin->type().is_const()) *out |= GL::type::Const;
                    }
                    else {
                        *out = any::wrap_member(*begin, call(begin));
                        if (begin->type().is_const()) *out |= GL::type::Const;
                    }
                }
            };
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>))) {
                    typename traits::returnType& ref = call(begin);
                    if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                        std::memcpy(out_destination, &ref, sizeof(std::decay_t<typename traits::returnType>));
                    }
                    else {
                        new (out_destination) std::decay_t<typename traits::returnType>(ref);
                    }
                }
                else
                    if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                        typeid(Function).name(),
                        out_size,
                        sizeof(std::decay_t<typename traits::returnType>),
                        typeid(std::decay_t<typename traits::returnType>).name()
                    ); err_str.length() > 0)
                        throw std::runtime_error(err_str.to_string());
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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
            decltype(traits::m_attr) m_attr;

            Attribute_Function_Caller(Function&& Func) : m_attr(std::forward<Function>(Func)) {
                // (void)Attribute_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
            };
            virtual ~Attribute_Function_Caller() = default;
            typename traits::returnType& call(const any* begin) const {
                return traits::call(begin, m_attr);
            };
            void call(const any* const begin, any* const out) const override {
                if (out) {
                    if constexpr (std::is_same_v<GL::any, std::decay_t<typename traits::returnType>> || std::is_same_v<any, std::decay_t<typename traits::returnType>>) {
                        *out = call(begin);
                        if (begin->type().is_const()) *out |= (GL::type::Const | GL::type::Reference);
                        else *out |= GL::type::Reference;
                    }
                    else if constexpr (std::is_pointer<typename traits::returnType>::value) {
                        *out = any::wrap_member(*begin, *call(begin));
                        if (begin->type().is_const()) *out |= GL::type::Const;
                    }
                    else {
                        *out = any::wrap_member(*begin, call(begin));
                        if (begin->type().is_const()) *out |= GL::type::Const;
                    }
                }
            };
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>))) {
                    typename traits::returnType& ref = call(begin);
                    if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                        std::memcpy(out_destination, &ref, sizeof(std::decay_t<typename traits::returnType>));
                    }
                    else {
                        new (out_destination) std::decay_t<typename traits::returnType>(ref);
                    }
                }
                else
                    if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                        typeid(Function).name(),
                        out_size,
                        sizeof(std::decay_t<typename traits::returnType>),
                        typeid(std::decay_t<typename traits::returnType>).name()
                    ); err_str.length() > 0)
                        throw std::runtime_error(err_str.to_string());
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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
            decltype(traits::m_attr) m_attr;

            Static_Function_Caller(Function&& Func) : m_attr(std::forward<Function>(Func)) {
                // (void)Static_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
            };
            virtual ~Static_Function_Caller() = default;

            typename traits::returnType call(const any* begin) const {
                return traits::call(begin, m_attr);
            };
            void call(const any* const begin, any* const out) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out) {
#if 1
                        if ((out->uuid() & uuid::TEMPORARY) == 0) {
                            if ((out->uuid() & uuid::INV_FLAGS) > 0) {
                                auto& ref = uuid::get_uuid(out->uuid());
                                if (GL::interlocked::decrement(ref.count) == 0) {
                                    ref.count = 1;
                                    ref.data = GL::type_erasure::wrap(call(begin));
                                    const_cast<void*&>(out->ptr()) = ref.data->get();
                                    const_cast<GL::type&>(out->type()) = GL::type_of<typename traits::returnType>();
                                }
                                else {
                                    const_cast<unsigned long&>(out->uuid()) = 0;
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
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>)))
                        if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                            decltype(auto) recieved = call(begin);
                            std::memcpy(out_destination, &recieved, sizeof(std::decay_t<typename traits::returnType>));
                        }
                        else {
                            new (out_destination) std::decay_t<typename traits::returnType>(call(begin));
                        }
                    else
                        if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                            typeid(Function).name(),
                            out_size,
                            sizeof(std::decay_t<typename traits::returnType>),
                            typeid(std::decay_t<typename traits::returnType>).name()
                        ); err_str.length() > 0)
                            throw std::runtime_error(err_str.to_string());
                }
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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
        template <typename Function> class Caching_Static_Function_Caller final : public Function_Caller {
        public:
            using traits = decltype(Caching_Static_Function_Traits(std::declval<Function>()));
            decltype(traits::m_attr) m_attr;

            Caching_Static_Function_Caller(Function&& Func) : m_attr(std::forward<Function>(Func)) {
                // (void)Caching_Static_Function_Traits(std::forward<Function>(Func)); // instantiate the static function pointer
            };
            virtual ~Caching_Static_Function_Caller() = default;

            typename traits::returnType call(const any* begin) const {
                return traits::call(begin, m_attr);
            };
            void call(const any* const begin, any* const out) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out) {
#if 1
                        if ((out->uuid() & uuid::TEMPORARY) == 0) {
                            if ((out->uuid() & uuid::INV_FLAGS) > 0) {
                                auto& ref = uuid::get_uuid(out->uuid());
                                if (GL::interlocked::decrement(ref.count) == 0) {
                                    ref.count = 1;
                                    ref.data = GL::type_erasure::wrap(call(begin));
                                    const_cast<void*&>(out->ptr()) = ref.data->get();
                                    const_cast<GL::type&>(out->type()) = GL::type_of<typename traits::returnType>();
                                }
                                else {
                                    const_cast<unsigned long&>(out->uuid()) = 0;
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
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>)))
                        if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                            decltype(auto) recieved = call(begin);
                            std::memcpy(out_destination, &recieved, sizeof(std::decay_t<typename traits::returnType>));
                        }
                        else {
                            new (out_destination) std::decay_t<typename traits::returnType>(call(begin));
                        }
                    else
                        if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                            typeid(Function).name(),
                            out_size,
                            sizeof(std::decay_t<typename traits::returnType>),
                            typeid(std::decay_t<typename traits::returnType>).name()
                        ); err_str.length() > 0)
                            throw std::runtime_error(err_str.to_string());
                }
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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
                //if constexpr (std::is_empty_v< Function >) {
                //    (void)Static_Function_Traits(+Func); // instantiate the static function pointer
                //}
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
                        if ((out->uuid() & uuid::TEMPORARY) == 0) {
                            if ((out->uuid() & uuid::INV_FLAGS) > 0) {
                                auto& ref = uuid::get_uuid(out->uuid());
                                if (GL::interlocked::decrement(ref.count) == 0) {
                                    ref.count = 1;
                                    ref.data = GL::type_erasure::wrap(call(begin));
                                    const_cast<void*&>(out->ptr()) = ref.data->get();
                                    const_cast<GL::type&>(out->type()) = GL::type_of<typename traits::returnType>();
                                }
                                else {
                                    const_cast<unsigned long&>(out->uuid()) = 0;
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
            void call(const any* const begin, void* out_destination, size_t out_size) const override {
                if constexpr (std::is_same_v<void, typename traits::returnType>) {
                    call(begin);
                }
                else {
                    if (out_destination && (out_size >= sizeof(std::decay_t<typename traits::returnType>)))
                        if constexpr (std::is_pod_v<std::decay_t<typename traits::returnType>>) {
                            decltype(auto) recieved = call(begin);
                            std::memcpy(out_destination, &recieved, sizeof(std::decay_t<typename traits::returnType>));
                        }
                        else {
                            new (out_destination) std::decay_t<typename traits::returnType>(call(begin));
                        }
                    else
                        if (GL::string err_str = GL::prints(GL::string(R"(
Call to function "%s" with explicit value return failed due to incorrect parameter sizing (%zu, expected %zu for return type of "%s").
                )").remove_leading_and_trailing_whitespace(),
                            typeid(Function).name(),
                            out_size,
                            sizeof(std::decay_t<typename traits::returnType>),
                            typeid(std::decay_t<typename traits::returnType>).name()
                        ); err_str.length() > 0)
                            throw std::runtime_error(err_str.to_string());
                }
            };
            bool return_type_explicit_match(std::type_info const& match) const override {
                return typeid(typename traits::returnType) == match;
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

#if 0
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
        Function_Signature(Ret(Class::* f)(Param...) & noexcept)
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
        Function_Signature(Ret(Class::* f)(Param...) && noexcept)
            ->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true, false, false>; // member function

        template<typename Ret, typename Class, typename... Param>
        Function_Signature(Ret(Class::* f)(Param...) const&& noexcept)
            ->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true, false, false>; // member function

        template<typename Ret, typename Class>
        Function_Signature(Ret Class::* f)
            ->Function_Signature<Ret, Class, Function_Params<Class&>, true, true, false>; // member object

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
#endif
};

