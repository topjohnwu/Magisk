//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// tuple_element<I, array<T, N> >::type

#include <array>
#include <type_traits>

template <class T>
void test()
{
    {
    typedef T Exp;
    typedef std::array<T, 3> C;
    static_assert((std::is_same<typename std::tuple_element<0, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<1, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<2, C>::type, Exp>::value), "");
    }
    {
    typedef T const Exp;
    typedef std::array<T, 3> const C;
    static_assert((std::is_same<typename std::tuple_element<0, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<1, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<2, C>::type, Exp>::value), "");
    }
    {
    typedef T volatile Exp;
    typedef std::array<T, 3> volatile C;
    static_assert((std::is_same<typename std::tuple_element<0, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<1, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<2, C>::type, Exp>::value), "");
    }
    {
    typedef T const volatile Exp;
    typedef std::array<T, 3> const volatile C;
    static_assert((std::is_same<typename std::tuple_element<0, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<1, C>::type, Exp>::value), "");
    static_assert((std::is_same<typename std::tuple_element<2, C>::type, Exp>::value), "");
    }
}

int main()
{
    test<double>();
    test<int>();
}
