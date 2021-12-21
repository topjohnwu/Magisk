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
// class year;

// constexpr year operator""y(unsigned long long y) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    using namespace std::chrono;
    ASSERT_NOEXCEPT(4y);

    static_assert( 2017y == year(2017), "");
    year y1 = 2018y;
    assert (y1 == year(2018));
    }

    {
    using namespace std::literals;
    ASSERT_NOEXCEPT(4d);

    static_assert( 2017y == std::chrono::year(2017), "");

    std::chrono::year y1 = 2020y;
    assert (y1 == std::chrono::year(2020));
    }
}
