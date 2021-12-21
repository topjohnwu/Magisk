//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <span>

// constexpr       iterator  begin() const noexcept;
// constexpr const_iterator cbegin() const noexcept;

#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

template <class Span>
constexpr bool testConstexprSpan(Span s)
{
    bool ret = true;
    typename Span::iterator b        = s. begin();
    typename Span::const_iterator cb = s.cbegin();

    if (s.empty())
    {
        ret = ret &&  ( b ==  s.end());
        ret = ret &&  (cb == s.cend());
    }
    else
    {
        ret = ret &&  (  *b ==  s[0]);
        ret = ret &&  ( &*b == &s[0]);
        ret = ret &&  ( *cb ==  s[0]);
        ret = ret &&  (&*cb == &s[0]);
    }
    ret = ret && (b == cb);
    return ret;
}


template <class Span>
void testRuntimeSpan(Span s)
{
    typename Span::iterator b        = s. begin();
    typename Span::const_iterator cb = s.cbegin();

    if (s.empty())
    {
        assert( b ==  s.end());
        assert(cb == s.cend());
    }
    else
    {
        assert(  *b ==  s[0]);
        assert( &*b == &s[0]);
        assert( *cb ==  s[0]);
        assert(&*cb == &s[0]);
    }
    assert(b == cb);
}

struct A{};
bool operator==(A, A) {return true;}

constexpr int iArr1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9};
          int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};


int main()
{
    static_assert(testConstexprSpan(std::span<int>()),            "");
    static_assert(testConstexprSpan(std::span<long>()),           "");
    static_assert(testConstexprSpan(std::span<double>()),         "");
    static_assert(testConstexprSpan(std::span<A>()),              "");
    static_assert(testConstexprSpan(std::span<std::string>()),    "");

    static_assert(testConstexprSpan(std::span<int, 0>()),         "");
    static_assert(testConstexprSpan(std::span<long, 0>()),        "");
    static_assert(testConstexprSpan(std::span<double, 0>()),      "");
    static_assert(testConstexprSpan(std::span<A, 0>()),           "");
    static_assert(testConstexprSpan(std::span<std::string, 0>()), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)),    "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5)),    "");


    testRuntimeSpan(std::span<int>        ());
    testRuntimeSpan(std::span<long>       ());
    testRuntimeSpan(std::span<double>     ());
    testRuntimeSpan(std::span<A>          ());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>        ());
    testRuntimeSpan(std::span<long, 0>       ());
    testRuntimeSpan(std::span<double, 0>     ());
    testRuntimeSpan(std::span<A, 0>          ());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::ptrdiff_t) 0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}
