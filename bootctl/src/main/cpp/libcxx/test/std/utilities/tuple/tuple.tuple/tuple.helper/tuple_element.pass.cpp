//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <size_t I, class... Types>
// class tuple_element<I, tuple<Types...> >
// {
// public:
//     typedef Ti type;
// };

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <type_traits>

#include "test_macros.h"

template <class T, std::size_t N, class U>
void test()
{
    static_assert((std::is_same<typename std::tuple_element<N, T>::type, U>::value), "");
    static_assert((std::is_same<typename std::tuple_element<N, const T>::type, const U>::value), "");
    static_assert((std::is_same<typename std::tuple_element<N, volatile T>::type, volatile U>::value), "");
    static_assert((std::is_same<typename std::tuple_element<N, const volatile T>::type, const volatile U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<typename std::tuple_element_t<N, T>, U>::value), "");
    static_assert((std::is_same<typename std::tuple_element_t<N, const T>, const U>::value), "");
    static_assert((std::is_same<typename std::tuple_element_t<N, volatile T>, volatile U>::value), "");
    static_assert((std::is_same<typename std::tuple_element_t<N, const volatile T>, const volatile U>::value), "");
#endif
}

int main()
{
    test<std::tuple<int>, 0, int>();
    test<std::tuple<char, int>, 0, char>();
    test<std::tuple<char, int>, 1, int>();
    test<std::tuple<int*, char, int>, 0, int*>();
    test<std::tuple<int*, char, int>, 1, char>();
    test<std::tuple<int*, char, int>, 2, int>();
}
