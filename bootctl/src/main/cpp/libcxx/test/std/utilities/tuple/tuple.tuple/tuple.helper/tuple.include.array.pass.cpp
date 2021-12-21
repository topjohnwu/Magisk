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
//
//  LWG #2212 says that tuple_size and tuple_element must be
//     available after including <utility>

#include <array>
#include <type_traits>

template <class T, std::size_t N, class U, size_t idx>
void test()
{
    static_assert((std::is_base_of<std::integral_constant<std::size_t, N>,
                                   std::tuple_size<T> >::value), "");
    static_assert((std::is_base_of<std::integral_constant<std::size_t, N>,
                                   std::tuple_size<const T> >::value), "");
    static_assert((std::is_base_of<std::integral_constant<std::size_t, N>,
                                   std::tuple_size<volatile T> >::value), "");
    static_assert((std::is_base_of<std::integral_constant<std::size_t, N>,
                                   std::tuple_size<const volatile T> >::value), "");
    static_assert((std::is_same<typename std::tuple_element<idx, T>::type, U>::value), "");
    static_assert((std::is_same<typename std::tuple_element<idx, const T>::type, const U>::value), "");
    static_assert((std::is_same<typename std::tuple_element<idx, volatile T>::type, volatile U>::value), "");
    static_assert((std::is_same<typename std::tuple_element<idx, const volatile T>::type, const volatile U>::value), "");
}

int main()
{
    test<std::array<int, 5>, 5, int, 0>();
    test<std::array<int, 5>, 5, int, 1>();
    test<std::array<const char *, 4>, 4, const char *, 3>();
    test<std::array<volatile int, 4>, 4, volatile int, 3>();
    test<std::array<char *, 3>, 3, char *, 1>();
    test<std::array<char *, 3>, 3, char *, 2>();
}
