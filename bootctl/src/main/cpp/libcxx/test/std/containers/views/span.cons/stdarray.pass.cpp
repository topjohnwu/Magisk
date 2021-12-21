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

// template<size_t N>
//     constexpr span(array<value_type, N>& arr) noexcept;
// template<size_t N>
//     constexpr span(const array<value_type, N>& arr) noexcept;
//
// Remarks: These constructors shall not participate in overload resolution unless:
//   — extent == dynamic_extent || N == extent is true, and
//   — remove_pointer_t<decltype(data(arr))>(*)[] is convertible to ElementType(*)[].
//


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"
// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"


void checkCV()
{
    std::array<int, 3> arr  = {1,2,3};
//  STL says these are not cromulent
//  std::array<const int,3> carr = {4,5,6};
//  std::array<volatile int, 3> varr = {7,8,9};
//  std::array<const volatile int, 3> cvarr = {1,3,5};

//  Types the same (dynamic sized)
    {
    std::span<               int> s1{  arr};    // a span<               int> pointing at int.
    }

//  Types the same (static sized)
    {
    std::span<               int,3> s1{  arr};  // a span<               int> pointing at int.
    }


//  types different (dynamic sized)
    {
    std::span<const          int> s1{ arr};     // a span<const          int> pointing at int.
    std::span<      volatile int> s2{ arr};     // a span<      volatile int> pointing at int.
    std::span<      volatile int> s3{ arr};     // a span<      volatile int> pointing at const int.
    std::span<const volatile int> s4{ arr};     // a span<const volatile int> pointing at int.
    }

//  types different (static sized)
    {
    std::span<const          int,3> s1{ arr};   // a span<const          int> pointing at int.
    std::span<      volatile int,3> s2{ arr};   // a span<      volatile int> pointing at int.
    std::span<      volatile int,3> s3{ arr};   // a span<      volatile int> pointing at const int.
    std::span<const volatile int,3> s4{ arr};   // a span<const volatile int> pointing at int.
    }
}


template <typename T>
constexpr bool testConstexprSpan()
{
    constexpr std::array<T,2> val = { T(), T() };
    ASSERT_NOEXCEPT(std::span<const T>   {val});
    ASSERT_NOEXCEPT(std::span<const T, 2>{val});
    std::span<const T>    s1{val};
    std::span<const T, 2> s2{val};
    return
        s1.data() == &val[0] && s1.size() == 2
    &&  s2.data() == &val[0] && s2.size() == 2;
}


template <typename T>
void testRuntimeSpan()
{
    std::array<T,2> val;
    ASSERT_NOEXCEPT(std::span<T>   {val});
    ASSERT_NOEXCEPT(std::span<T, 2>{val});
    std::span<T>    s1{val};
    std::span<T, 2> s2{val};
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
