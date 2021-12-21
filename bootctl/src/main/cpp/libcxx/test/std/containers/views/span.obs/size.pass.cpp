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

// constexpr index_type size() const noexcept;
//


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"


template <typename Span>
constexpr bool testConstexprSpan(Span sp, ptrdiff_t sz)
{
    ASSERT_NOEXCEPT(sp.size());
    return sp.size() == sz;
}


template <typename Span>
void testRuntimeSpan(Span sp, ptrdiff_t sz)
{
    ASSERT_NOEXCEPT(sp.size());
    assert(sp.size() == sz);
}

struct A{};
constexpr int iArr1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9};
          int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

int main ()
{
    static_assert(testConstexprSpan(std::span<int>(), 0),            "");
    static_assert(testConstexprSpan(std::span<long>(), 0),           "");
    static_assert(testConstexprSpan(std::span<double>(), 0),         "");
    static_assert(testConstexprSpan(std::span<A>(), 0),              "");
    static_assert(testConstexprSpan(std::span<std::string>(), 0),    "");

    static_assert(testConstexprSpan(std::span<int, 0>(), 0),         "");
    static_assert(testConstexprSpan(std::span<long, 0>(), 0),        "");
    static_assert(testConstexprSpan(std::span<double, 0>(), 0),      "");
    static_assert(testConstexprSpan(std::span<A, 0>(), 0),           "");
    static_assert(testConstexprSpan(std::span<std::string, 0>(), 0), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), 1),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 2),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 3),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 4),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5), 5),    "");

    testRuntimeSpan(std::span<int>        (), 0);
    testRuntimeSpan(std::span<long>       (), 0);
    testRuntimeSpan(std::span<double>     (), 0);
    testRuntimeSpan(std::span<A>          (), 0);
    testRuntimeSpan(std::span<std::string>(), 0);

    testRuntimeSpan(std::span<int, 0>        (), 0);
    testRuntimeSpan(std::span<long, 0>       (), 0);
    testRuntimeSpan(std::span<double, 0>     (), 0);
    testRuntimeSpan(std::span<A, 0>          (), 0);
    testRuntimeSpan(std::span<std::string, 0>(), 0);

    testRuntimeSpan(std::span<int>(iArr2, 1), 1);
    testRuntimeSpan(std::span<int>(iArr2, 2), 2);
    testRuntimeSpan(std::span<int>(iArr2, 3), 3);
    testRuntimeSpan(std::span<int>(iArr2, 4), 4);
    testRuntimeSpan(std::span<int>(iArr2, 5), 5);

    testRuntimeSpan(std::span<int, 1>(iArr2 + 5, 1), 1);
    testRuntimeSpan(std::span<int, 2>(iArr2 + 4, 2), 2);
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3), 3);
    testRuntimeSpan(std::span<int, 4>(iArr2 + 2, 4), 4);
    testRuntimeSpan(std::span<int, 5>(iArr2 + 1, 5), 5);

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::ptrdiff_t) 0), 0);
    testRuntimeSpan(std::span<std::string>(&s, 1), 1);
}
