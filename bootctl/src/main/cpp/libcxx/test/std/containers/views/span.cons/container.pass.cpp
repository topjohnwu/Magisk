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

//  template<class Container>
//     constexpr span(Container& cont);
//   template<class Container>
//     constexpr span(const Container& cont);
//
// Remarks: These constructors shall not participate in overload resolution unless:
//   — Container is not a specialization of span,
//   — Container is not a specialization of array,
//   — is_array_v<Container> is false,
//   — data(cont) and size(cont) are both well-formed, and
//   — remove_pointer_t<decltype(data(cont))>(*)[] is convertible to ElementType(*)[].
//


#include <span>
#include <cassert>
#include <string>
#include <vector>

#include "test_macros.h"

//  Look ma - I'm a container!
template <typename T>
struct IsAContainer {
    constexpr IsAContainer() : v_{} {}
    constexpr size_t size() const {return 1;}
    constexpr       T *data() {return &v_;}
    constexpr const T *data() const {return &v_;}

    constexpr T const *getV() const {return &v_;} // for checking
    T v_;
};


void checkCV()
{
    std::vector<int> v  = {1,2,3};

//  Types the same (dynamic sized)
    {
    std::span<               int> s1{v};    // a span<               int> pointing at int.
    }

//  Types the same (static sized)
    {
    std::span<               int,3> s1{v};  // a span<               int> pointing at int.
    }

//  types different (dynamic sized)
    {
    std::span<const          int> s1{v};    // a span<const          int> pointing at int.
    std::span<      volatile int> s2{v};    // a span<      volatile int> pointing at int.
    std::span<      volatile int> s3{v};    // a span<      volatile int> pointing at const int.
    std::span<const volatile int> s4{v};    // a span<const volatile int> pointing at int.
    }

//  types different (static sized)
    {
    std::span<const          int,3> s1{v};  // a span<const          int> pointing at int.
    std::span<      volatile int,3> s2{v};  // a span<      volatile int> pointing at int.
    std::span<      volatile int,3> s3{v};  // a span<      volatile int> pointing at const int.
    std::span<const volatile int,3> s4{v};  // a span<const volatile int> pointing at int.
    }
}


template <typename T>
constexpr bool testConstexprSpan()
{
    constexpr IsAContainer<const T> val{};
    std::span<const T>    s1{val};
    std::span<const T, 1> s2{val};
    return
        s1.data() == val.getV() && s1.size() == 1
    &&  s2.data() == val.getV() && s2.size() == 1;
}


template <typename T>
void testRuntimeSpan()
{
    IsAContainer<T> val{};
    std::span<const T>    s1{val};
    std::span<const T, 1> s2{val};
    assert(s1.data() == val.getV() && s1.size() == 1);
    assert(s2.data() == val.getV() && s2.size() == 1);
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
