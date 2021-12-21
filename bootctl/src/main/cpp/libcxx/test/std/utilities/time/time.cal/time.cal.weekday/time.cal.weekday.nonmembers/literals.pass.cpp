//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <chrono>

// inline constexpr weekday   Sunday{0};
// inline constexpr weekday   Monday{1};
// inline constexpr weekday   Tuesday{2};
// inline constexpr weekday   Wednesday{3};
// inline constexpr weekday   Thursday{4};
// inline constexpr weekday   Friday{5};
// inline constexpr weekday   Saturday{6};


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{

    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Sunday));
    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Monday));
    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Tuesday));
    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Wednesday));
    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Thursday));
    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Friday));
    ASSERT_SAME_TYPE(const std::chrono::weekday, decltype(std::chrono::Saturday));

    static_assert( std::chrono::Sunday    == std::chrono::weekday(0),  "");
    static_assert( std::chrono::Monday    == std::chrono::weekday(1),  "");
    static_assert( std::chrono::Tuesday   == std::chrono::weekday(2),  "");
    static_assert( std::chrono::Wednesday == std::chrono::weekday(3),  "");
    static_assert( std::chrono::Thursday  == std::chrono::weekday(4),  "");
    static_assert( std::chrono::Friday    == std::chrono::weekday(5),  "");
    static_assert( std::chrono::Saturday  == std::chrono::weekday(6),  "");

    assert(std::chrono::Sunday    == std::chrono::weekday(0));
    assert(std::chrono::Monday    == std::chrono::weekday(1));
    assert(std::chrono::Tuesday   == std::chrono::weekday(2));
    assert(std::chrono::Wednesday == std::chrono::weekday(3));
    assert(std::chrono::Thursday  == std::chrono::weekday(4));
    assert(std::chrono::Friday    == std::chrono::weekday(5));
    assert(std::chrono::Saturday  == std::chrono::weekday(6));

    assert(static_cast<unsigned>(std::chrono::Sunday)    ==  0);
    assert(static_cast<unsigned>(std::chrono::Monday)    ==  1);
    assert(static_cast<unsigned>(std::chrono::Tuesday)   ==  2);
    assert(static_cast<unsigned>(std::chrono::Wednesday) ==  3);
    assert(static_cast<unsigned>(std::chrono::Thursday)  ==  4);
    assert(static_cast<unsigned>(std::chrono::Friday)    ==  5);
    assert(static_cast<unsigned>(std::chrono::Saturday)  ==  6);
}
