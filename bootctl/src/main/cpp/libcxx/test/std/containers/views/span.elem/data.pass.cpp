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

// constexpr pointer data() const noexcept;
//


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"


template <typename Span>
constexpr bool testConstexprSpan(Span sp, typename Span::pointer ptr)
{
    ASSERT_NOEXCEPT(sp.data());
    return sp.data() == ptr;
}


template <typename Span>
void testRuntimeSpan(Span sp, typename Span::pointer ptr)
{
    ASSERT_NOEXCEPT(sp.data());
    assert(sp.data() == ptr);
}

struct A{};
constexpr int iArr1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9};
          int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

int main ()
{

//  dynamic size
    static_assert(testConstexprSpan(std::span<int>(), nullptr),         "");
    static_assert(testConstexprSpan(std::span<long>(), nullptr),        "");
    static_assert(testConstexprSpan(std::span<double>(), nullptr),      "");
    static_assert(testConstexprSpan(std::span<A>(), nullptr),           "");
    static_assert(testConstexprSpan(std::span<std::string>(), nullptr), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), iArr1), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 1, 1), iArr1 + 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 2, 2), iArr1 + 2), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 3, 3), iArr1 + 3), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 4, 4), iArr1 + 4), "");

//  static size
    static_assert(testConstexprSpan(std::span<int, 0>(), nullptr),         "");
    static_assert(testConstexprSpan(std::span<long, 0>(), nullptr),        "");
    static_assert(testConstexprSpan(std::span<double, 0>(), nullptr),      "");
    static_assert(testConstexprSpan(std::span<A, 0>(), nullptr),           "");
    static_assert(testConstexprSpan(std::span<std::string, 0>(), nullptr), "");

    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1, 1), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), iArr1), "");

    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1 + 1, 1), iArr1 + 1), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1 + 2, 2), iArr1 + 2), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1 + 3, 3), iArr1 + 3), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1 + 4, 4), iArr1 + 4), "");


//  dynamic size
    testRuntimeSpan(std::span<int>(), nullptr);
    testRuntimeSpan(std::span<long>(), nullptr);
    testRuntimeSpan(std::span<double>(), nullptr);
    testRuntimeSpan(std::span<A>(), nullptr);
    testRuntimeSpan(std::span<std::string>(), nullptr);

    testRuntimeSpan(std::span<int>(iArr2, 1), iArr2);
    testRuntimeSpan(std::span<int>(iArr2, 2), iArr2);
    testRuntimeSpan(std::span<int>(iArr2, 3), iArr2);
    testRuntimeSpan(std::span<int>(iArr2, 4), iArr2);

    testRuntimeSpan(std::span<int>(iArr2 + 1, 1), iArr2 + 1);
    testRuntimeSpan(std::span<int>(iArr2 + 2, 2), iArr2 + 2);
    testRuntimeSpan(std::span<int>(iArr2 + 3, 3), iArr2 + 3);
    testRuntimeSpan(std::span<int>(iArr2 + 4, 4), iArr2 + 4);

//  static size
    testRuntimeSpan(std::span<int, 0>(), nullptr);
    testRuntimeSpan(std::span<long, 0>(), nullptr);
    testRuntimeSpan(std::span<double, 0>(), nullptr);
    testRuntimeSpan(std::span<A, 0>(), nullptr);
    testRuntimeSpan(std::span<std::string, 0>(), nullptr);

    testRuntimeSpan(std::span<int, 1>(iArr2, 1), iArr2);
    testRuntimeSpan(std::span<int, 2>(iArr2, 2), iArr2);
    testRuntimeSpan(std::span<int, 3>(iArr2, 3), iArr2);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), iArr2);

    testRuntimeSpan(std::span<int, 1>(iArr2 + 1, 1), iArr2 + 1);
    testRuntimeSpan(std::span<int, 2>(iArr2 + 2, 2), iArr2 + 2);
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3), iArr2 + 3);
    testRuntimeSpan(std::span<int, 4>(iArr2 + 4, 4), iArr2 + 4);


    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, 1), &s);
    testRuntimeSpan(std::span<std::string, 1>(&s, 1), &s);

}
