//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template<class T, T N>
//   using make_integer_sequence = integer_sequence<T, 0, 1, ..., N-1>;

// UNSUPPORTED: c++98, c++03, c++11

#include <utility>
#include <type_traits>
#include <cassert>

int main()
{
    static_assert(std::is_same<std::make_integer_sequence<int, 0>, std::integer_sequence<int>>::value, "");
    static_assert(std::is_same<std::make_integer_sequence<int, 1>, std::integer_sequence<int, 0>>::value, "");
    static_assert(std::is_same<std::make_integer_sequence<int, 2>, std::integer_sequence<int, 0, 1>>::value, "");
    static_assert(std::is_same<std::make_integer_sequence<int, 3>, std::integer_sequence<int, 0, 1, 2>>::value, "");

    static_assert(std::is_same<std::make_integer_sequence<unsigned long long, 0>, std::integer_sequence<unsigned long long>>::value, "");
    static_assert(std::is_same<std::make_integer_sequence<unsigned long long, 1>, std::integer_sequence<unsigned long long, 0>>::value, "");
    static_assert(std::is_same<std::make_integer_sequence<unsigned long long, 2>, std::integer_sequence<unsigned long long, 0, 1>>::value, "");
    static_assert(std::is_same<std::make_integer_sequence<unsigned long long, 3>, std::integer_sequence<unsigned long long, 0, 1, 2>>::value, "");
}
