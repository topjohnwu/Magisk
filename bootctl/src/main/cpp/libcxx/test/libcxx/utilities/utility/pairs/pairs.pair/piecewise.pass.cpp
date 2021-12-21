//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// template <class... Args1, class... Args2>
//     pair(piecewise_construct_t, tuple<Args1...> first_args,
//                                 tuple<Args2...> second_args);

#include <tuple>
#include <type_traits>
#include <utility>

#include "archetypes.hpp"


int main() {
    using NonThrowingConvert = NonThrowingTypes::ConvertingType;
    using ThrowingConvert = NonTrivialTypes::ConvertingType;
    static_assert(!std::is_nothrow_constructible<std::pair<ThrowingConvert, ThrowingConvert>,
                                                 std::piecewise_construct_t, std::tuple<int, int>, std::tuple<long, long>>::value, "");
    static_assert(!std::is_nothrow_constructible<std::pair<NonThrowingConvert, ThrowingConvert>,
                                                 std::piecewise_construct_t, std::tuple<int, int>, std::tuple<long, long>>::value, "");
    static_assert(!std::is_nothrow_constructible<std::pair<ThrowingConvert, NonThrowingConvert>,
                                                 std::piecewise_construct_t, std::tuple<int, int>, std::tuple<long, long>>::value, "");
    static_assert( std::is_nothrow_constructible<std::pair<NonThrowingConvert, NonThrowingConvert>,
                                                 std::piecewise_construct_t, std::tuple<int, int>, std::tuple<long, long>>::value, "");
}
