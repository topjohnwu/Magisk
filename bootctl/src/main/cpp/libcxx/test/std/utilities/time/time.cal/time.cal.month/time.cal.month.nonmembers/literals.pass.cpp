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

// inline constexpr month January{1};
// inline constexpr month February{2};
// inline constexpr month March{3};
// inline constexpr month April{4};
// inline constexpr month May{5};
// inline constexpr month June{6};
// inline constexpr month July{7};
// inline constexpr month August{8};
// inline constexpr month September{9};
// inline constexpr month October{10};
// inline constexpr month November{11};
// inline constexpr month December{12};


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{

    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::January));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::February));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::March));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::April));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::May));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::June));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::July));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::August));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::September));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::October));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::November));
    ASSERT_SAME_TYPE(const std::chrono::month, decltype(std::chrono::December));

    static_assert( std::chrono::January   == std::chrono::month(1),  "");
    static_assert( std::chrono::February  == std::chrono::month(2),  "");
    static_assert( std::chrono::March     == std::chrono::month(3),  "");
    static_assert( std::chrono::April     == std::chrono::month(4),  "");
    static_assert( std::chrono::May       == std::chrono::month(5),  "");
    static_assert( std::chrono::June      == std::chrono::month(6),  "");
    static_assert( std::chrono::July      == std::chrono::month(7),  "");
    static_assert( std::chrono::August    == std::chrono::month(8),  "");
    static_assert( std::chrono::September == std::chrono::month(9),  "");
    static_assert( std::chrono::October   == std::chrono::month(10), "");
    static_assert( std::chrono::November  == std::chrono::month(11), "");
    static_assert( std::chrono::December  == std::chrono::month(12), "");

    assert(std::chrono::January   == std::chrono::month(1));
    assert(std::chrono::February  == std::chrono::month(2));
    assert(std::chrono::March     == std::chrono::month(3));
    assert(std::chrono::April     == std::chrono::month(4));
    assert(std::chrono::May       == std::chrono::month(5));
    assert(std::chrono::June      == std::chrono::month(6));
    assert(std::chrono::July      == std::chrono::month(7));
    assert(std::chrono::August    == std::chrono::month(8));
    assert(std::chrono::September == std::chrono::month(9));
    assert(std::chrono::October   == std::chrono::month(10));
    assert(std::chrono::November  == std::chrono::month(11));
    assert(std::chrono::December  == std::chrono::month(12));

    assert(static_cast<unsigned>(std::chrono::January)   ==  1);
    assert(static_cast<unsigned>(std::chrono::February)  ==  2);
    assert(static_cast<unsigned>(std::chrono::March)     ==  3);
    assert(static_cast<unsigned>(std::chrono::April)     ==  4);
    assert(static_cast<unsigned>(std::chrono::May)       ==  5);
    assert(static_cast<unsigned>(std::chrono::June)      ==  6);
    assert(static_cast<unsigned>(std::chrono::July)      ==  7);
    assert(static_cast<unsigned>(std::chrono::August)    ==  8);
    assert(static_cast<unsigned>(std::chrono::September) ==  9);
    assert(static_cast<unsigned>(std::chrono::October)   == 10);
    assert(static_cast<unsigned>(std::chrono::November)  == 11);
    assert(static_cast<unsigned>(std::chrono::December)  == 12);
}
