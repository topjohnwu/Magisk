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

// constexpr span(pointer first, pointer last);
// Requires: [first, last) shall be a valid range.
//   If extent is not equal to dynamic_extent, then last - first shall be equal to extent.
//

#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

void checkCV()
{
                   int   arr[] = {1,2,3};
    const          int  carr[] = {4,5,6};
          volatile int  varr[] = {7,8,9};
    const volatile int cvarr[] = {1,3,5};

//  Types the same (dynamic sized)
    {
    std::span<               int> s1{  arr,   arr + 3}; // a span<               int> pointing at int.
    std::span<const          int> s2{ carr,  carr + 3}; // a span<const          int> pointing at const int.
    std::span<      volatile int> s3{ varr,  varr + 3}; // a span<      volatile int> pointing at volatile int.
    std::span<const volatile int> s4{cvarr, cvarr + 3}; // a span<const volatile int> pointing at const volatile int.
    assert(s1.size() + s2.size() + s3.size() + s4.size() == 12);
    }

//  Types the same (static sized)
    {
    std::span<               int,3> s1{  arr,   arr + 3};   // a span<               int> pointing at int.
    std::span<const          int,3> s2{ carr,  carr + 3};   // a span<const          int> pointing at const int.
    std::span<      volatile int,3> s3{ varr,  varr + 3};   // a span<      volatile int> pointing at volatile int.
    std::span<const volatile int,3> s4{cvarr, cvarr + 3};   // a span<const volatile int> pointing at const volatile int.
    assert(s1.size() + s2.size() + s3.size() + s4.size() == 12);
    }


//  types different (dynamic sized)
    {
    std::span<const          int> s1{ arr,  arr + 3};       // a span<const          int> pointing at int.
    std::span<      volatile int> s2{ arr,  arr + 3};       // a span<      volatile int> pointing at int.
    std::span<      volatile int> s3{ arr,  arr + 3};       // a span<      volatile int> pointing at const int.
    std::span<const volatile int> s4{ arr,  arr + 3};       // a span<const volatile int> pointing at int.
    std::span<const volatile int> s5{carr, carr + 3};       // a span<const volatile int> pointing at const int.
    std::span<const volatile int> s6{varr, varr + 3};       // a span<const volatile int> pointing at volatile int.
    assert(s1.size() + s2.size() + s3.size() + s4.size() + s5.size() + s6.size() == 18);
    }

//  types different (static sized)
    {
    std::span<const          int,3> s1{ arr,  arr + 3}; // a span<const          int> pointing at int.
    std::span<      volatile int,3> s2{ arr,  arr + 3}; // a span<      volatile int> pointing at int.
    std::span<      volatile int,3> s3{ arr,  arr + 3}; // a span<      volatile int> pointing at const int.
    std::span<const volatile int,3> s4{ arr,  arr + 3}; // a span<const volatile int> pointing at int.
    std::span<const volatile int,3> s5{carr, carr + 3}; // a span<const volatile int> pointing at const int.
    std::span<const volatile int,3> s6{varr, varr + 3}; // a span<const volatile int> pointing at volatile int.
    assert(s1.size() + s2.size() + s3.size() + s4.size() + s5.size() + s6.size() == 18);
    }
}


template <typename T>
constexpr bool testConstexprSpan()
{
    constexpr T val[2] = {};
    std::span<const T>   s1{val, val+2};
    std::span<const T,2> s2{val, val+2};
    return
        s1.data() == &val[0] && s1.size() == 2
    &&  s2.data() == &val[0] && s2.size() == 2;
}


template <typename T>
void testRuntimeSpan()
{
    T val[2] = {};
    std::span<T>   s1{val, val+2};
    std::span<T,2> s2{val, val+2};
    assert(s1.data() == &val[0] && s1.size() == 2);
    assert(s2.data() == &val[0] && s2.size() == 2);
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
