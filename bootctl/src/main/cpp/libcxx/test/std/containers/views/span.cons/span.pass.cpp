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

// template<class OtherElementType, ptrdiff_t OtherExtent>
//    constexpr span(const span<OtherElementType, OtherExtent>& s) noexcept;
//
//  Remarks: This constructor shall not participate in overload resolution unless:
//      Extent == dynamic_extent || Extent == OtherExtent is true, and
//      OtherElementType(*)[] is convertible to ElementType(*)[].


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

void checkCV()
{
    std::span<               int>   sp;
//  std::span<const          int>  csp;
    std::span<      volatile int>  vsp;
//  std::span<const volatile int> cvsp;

    std::span<               int, 0>   sp0;
//  std::span<const          int, 0>  csp0;
    std::span<      volatile int, 0>  vsp0;
//  std::span<const volatile int, 0> cvsp0;

//  dynamic -> dynamic
    {
        std::span<const          int> s1{  sp}; // a span<const          int> pointing at int.
        std::span<      volatile int> s2{  sp}; // a span<      volatile int> pointing at int.
        std::span<const volatile int> s3{  sp}; // a span<const volatile int> pointing at int.
        std::span<const volatile int> s4{ vsp}; // a span<const volatile int> pointing at volatile int.
        assert(s1.size() + s2.size() + s3.size() + s4.size() == 0);
    }

//  static -> static
    {
        std::span<const          int, 0> s1{  sp0}; // a span<const          int> pointing at int.
        std::span<      volatile int, 0> s2{  sp0}; // a span<      volatile int> pointing at int.
        std::span<const volatile int, 0> s3{  sp0}; // a span<const volatile int> pointing at int.
        std::span<const volatile int, 0> s4{ vsp0}; // a span<const volatile int> pointing at volatile int.
        assert(s1.size() + s2.size() + s3.size() + s4.size() == 0);
    }

//  static -> dynamic
    {
        std::span<const          int> s1{  sp0};    // a span<const          int> pointing at int.
        std::span<      volatile int> s2{  sp0};    // a span<      volatile int> pointing at int.
        std::span<const volatile int> s3{  sp0};    // a span<const volatile int> pointing at int.
        std::span<const volatile int> s4{ vsp0};    // a span<const volatile int> pointing at volatile int.
        assert(s1.size() + s2.size() + s3.size() + s4.size() == 0);
    }

//  dynamic -> static
    {
        std::span<const          int, 0> s1{  sp};  // a span<const          int> pointing at int.
        std::span<      volatile int, 0> s2{  sp};  // a span<      volatile int> pointing at int.
        std::span<const volatile int, 0> s3{  sp};  // a span<const volatile int> pointing at int.
        std::span<const volatile int, 0> s4{ vsp};  // a span<const volatile int> pointing at volatile int.
        assert(s1.size() + s2.size() + s3.size() + s4.size() == 0);
    }
}


template <typename T>
constexpr bool testConstexprSpan()
{
    std::span<T>    s0{};
    std::span<T, 0> s1(s0); // dynamic -> static
    std::span<T>    s2(s1); // static -> dynamic
    ASSERT_NOEXCEPT(std::span<T>   {s0});
    ASSERT_NOEXCEPT(std::span<T, 0>{s1});
    ASSERT_NOEXCEPT(std::span<T>   {s1});
    ASSERT_NOEXCEPT(std::span<T, 0>{s0});

    return
        s1.data() == nullptr && s1.size() == 0
    &&  s2.data() == nullptr && s2.size() == 0;
}


template <typename T>
void testRuntimeSpan()
{
    std::span<T>    s0{};
    std::span<T, 0> s1(s0); // dynamic -> static
    std::span<T>    s2(s1); // static -> dynamic
    ASSERT_NOEXCEPT(std::span<T>   {s0});
    ASSERT_NOEXCEPT(std::span<T, 0>{s1});
    ASSERT_NOEXCEPT(std::span<T>   {s1});
    ASSERT_NOEXCEPT(std::span<T, 0>{s0});

    assert(s1.data() == nullptr && s1.size() == 0);
    assert(s2.data() == nullptr && s2.size() == 0);
}


template <typename Dest, typename Src>
bool testConversionSpan()
{
    static_assert(std::is_convertible_v<Src(*)[], Dest(*)[]>, "Bad input types to 'testConversionSpan");
    std::span<Src>    s0d{};
    std::span<Src>    s0s{};
    std::span<Dest, 0> s1(s0d); // dynamic -> static
    std::span<Dest>    s2(s0s); // static -> dynamic
        s1.data() == nullptr && s1.size() == 0
    &&  s2.data() == nullptr && s2.size() == 0;
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

//  TODO: Add some conversion tests here that aren't "X --> const X"
//  assert((testConversionSpan<unsigned char, char>()));

    checkCV();
}
