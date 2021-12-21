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
// class year;

// constexpr bool is_is_leap() const noexcept;
//  y_ % 4 == 0 && (y_ % 100 != 0 || y_ % 400 == 0)
//

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year = std::chrono::year;

    ASSERT_NOEXCEPT(                year(1).is_leap());
    ASSERT_SAME_TYPE(bool, decltype(year(1).is_leap()));

    static_assert(!year{1}.is_leap(), "");
    static_assert(!year{2}.is_leap(), "");
    static_assert(!year{3}.is_leap(), "");
    static_assert( year{4}.is_leap(), "");

    assert( year{-2000}.is_leap());
    assert( year{ -400}.is_leap());
    assert(!year{ -300}.is_leap());
    assert(!year{ -200}.is_leap());

    assert(!year{  200}.is_leap());
    assert(!year{  300}.is_leap());
    assert( year{  400}.is_leap());
    assert(!year{ 1997}.is_leap());
    assert(!year{ 1998}.is_leap());
    assert(!year{ 1999}.is_leap());
    assert( year{ 2000}.is_leap());
    assert(!year{ 2001}.is_leap());
    assert(!year{ 2002}.is_leap());
    assert(!year{ 2003}.is_leap());
    assert( year{ 2004}.is_leap());
    assert(!year{ 2100}.is_leap());
}
