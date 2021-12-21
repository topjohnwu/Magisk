// -*- C++ -*-
//===------------------------------ span ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <span>

// constexpr span() noexcept;

#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

void checkCV()
{
//  Types the same (dynamic sized)
    {
    std::span<               int> s1;
    std::span<const          int> s2;
    std::span<      volatile int> s3;
    std::span<const volatile int> s4;
    assert(s1.size() + s2.size() + s3.size() + s4.size() == 0);
    }

//  Types the same (static sized)
    {
    std::span<               int,0> s1;
    std::span<const          int,0> s2;
    std::span<      volatile int,0> s3;
    std::span<const volatile int,0> s4;
    assert(s1.size() + s2.size() + s3.size() + s4.size() == 0);
    }
}


template <typename T>
constexpr bool testConstexprSpan()
{
    std::span<const T>    s1;
    std::span<const T, 0> s2;
    return
        s1.data() == nullptr && s1.size() == 0
    &&  s2.data() == nullptr && s2.size() == 0;
}


template <typename T>
void testRuntimeSpan()
{
    ASSERT_NOEXCEPT(T{});
    std::span<const T>    s1;
    std::span<const T, 0> s2;
    assert(s1.data() == nullptr && s1.size() == 0);
    assert(s2.data() == nullptr && s2.size() == 0);
}


struct A{};

int main ()
{
    static_assert(testConstexprSpan<int>(),    "");
    static_assert(testConstexprSpan<long>(),   "");
    static_assert(testConstexprSpan<double>(), "");
    static_assert(testConstexprSpan<A>(),      "");

    testRuntimeSpan<int>();
    testRuntimeSpan<long>();
    testRuntimeSpan<double>();
    testRuntimeSpan<std::string>();
    testRuntimeSpan<A>();

    checkCV();
}
