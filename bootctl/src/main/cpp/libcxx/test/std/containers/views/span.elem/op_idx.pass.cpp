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

// constexpr reference operator[](index_type idx) const;
// constexpr reference operator()(index_type idx) const;
//


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"


template <typename Span>
constexpr bool testConstexprSpan(Span sp, ptrdiff_t idx)
{
    _LIBCPP_ASSERT(noexcept(sp[idx]), "");
    _LIBCPP_ASSERT(noexcept(sp(idx)), "");

    typename Span::reference r1 = sp[idx];
    typename Span::reference r2 = sp(idx);
    typename Span::reference r3 = *(sp.data() + idx);
    return r1 == r2 && r2 == r3;
}


template <typename Span>
void testRuntimeSpan(Span sp, ptrdiff_t idx)
{
    _LIBCPP_ASSERT(noexcept(sp[idx]), "");
    _LIBCPP_ASSERT(noexcept(sp(idx)), "");

    typename Span::reference r1 = sp[idx];
    typename Span::reference r2 = sp(idx);
    typename Span::reference r3 = *(sp.data() + idx);
    assert(r1 == r2 && r2 == r3);
}

struct A{};
constexpr int iArr1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9};
          int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

int main ()
{
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), 0), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 0), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 1), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 0), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 2), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 0), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 2), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 3), "");


    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1, 1), 0), "");

    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2), 0), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2), 1), "");

    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), 0), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), 1), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), 2), "");

    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 0), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 1), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 2), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 3), "");


    testRuntimeSpan(std::span<int>(iArr2, 1), 0);

    testRuntimeSpan(std::span<int>(iArr2, 2), 0);
    testRuntimeSpan(std::span<int>(iArr2, 2), 1);

    testRuntimeSpan(std::span<int>(iArr2, 3), 0);
    testRuntimeSpan(std::span<int>(iArr2, 3), 1);
    testRuntimeSpan(std::span<int>(iArr2, 3), 2);

    testRuntimeSpan(std::span<int>(iArr2, 4), 0);
    testRuntimeSpan(std::span<int>(iArr2, 4), 1);
    testRuntimeSpan(std::span<int>(iArr2, 4), 2);
    testRuntimeSpan(std::span<int>(iArr2, 4), 3);


    testRuntimeSpan(std::span<int, 1>(iArr2, 1), 0);

    testRuntimeSpan(std::span<int, 2>(iArr2, 2), 0);
    testRuntimeSpan(std::span<int, 2>(iArr2, 2), 1);

    testRuntimeSpan(std::span<int, 3>(iArr2, 3), 0);
    testRuntimeSpan(std::span<int, 3>(iArr2, 3), 1);
    testRuntimeSpan(std::span<int, 3>(iArr2, 3), 2);

    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 0);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 1);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 2);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 3);

    std::string s;
    testRuntimeSpan(std::span<std::string>   (&s, 1), 0);
    testRuntimeSpan(std::span<std::string, 1>(&s, 1), 0);
}
