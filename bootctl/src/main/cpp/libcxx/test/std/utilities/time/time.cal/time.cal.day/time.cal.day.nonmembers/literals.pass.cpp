//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// UNSUPPORTED: clang-5, clang-6, clang-7
// UNSUPPORTED: apple-clang-6, apple-clang-7, apple-clang-8, apple-clang-9, apple-clang-10

// <chrono>
// class day;

// constexpr day operator""d(unsigned long long d) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    using namespace std::chrono;
    ASSERT_NOEXCEPT(               4d);
    ASSERT_SAME_TYPE(day, decltype(4d));

    static_assert( 7d == day(7), "");
    day d1 = 4d;
    assert (d1 == day(4));
}

    {
    using namespace std::literals;
    ASSERT_NOEXCEPT(                            4d);
    ASSERT_SAME_TYPE(std::chrono::day, decltype(4d));

    static_assert( 7d == std::chrono::day(7), "");

    std::chrono::day d1 = 4d;
    assert (d1 == std::chrono::day(4));
    }

}
