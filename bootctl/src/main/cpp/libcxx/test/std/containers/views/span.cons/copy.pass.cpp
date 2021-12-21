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

//  constexpr span(const span& other) noexcept = default;

#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

template <typename T>
constexpr bool doCopy(const T &rhs)
{
    ASSERT_NOEXCEPT(T{rhs});
    T lhs{rhs};
    return lhs.data() == rhs.data()
     &&    lhs.size() == rhs.size();
}

struct A{};

template <typename T>
void testCV ()
{
    int  arr[] = {1,2,3};
    assert((doCopy(std::span<T>  ()          )));
    assert((doCopy(std::span<T,0>()          )));
    assert((doCopy(std::span<T>  (&arr[0], 1))));
    assert((doCopy(std::span<T,1>(&arr[0], 1))));
    assert((doCopy(std::span<T>  (&arr[0], 2))));
    assert((doCopy(std::span<T,2>(&arr[0], 2))));
}


int main ()
{
    constexpr int carr[] = {1,2,3};

    static_assert(doCopy(std::span<      int>  ()),            "");
    static_assert(doCopy(std::span<      int,0>()),            "");
    static_assert(doCopy(std::span<const int>  (&carr[0], 1)), "");
    static_assert(doCopy(std::span<const int,1>(&carr[0], 1)), "");
    static_assert(doCopy(std::span<const int>  (&carr[0], 2)), "");
    static_assert(doCopy(std::span<const int,2>(&carr[0], 2)), "");

    static_assert(doCopy(std::span<long>()),   "");
    static_assert(doCopy(std::span<double>()), "");
    static_assert(doCopy(std::span<A>()),      "");

    std::string s;
    assert(doCopy(std::span<std::string>   ()     ));
    assert(doCopy(std::span<std::string, 0>()     ));
    assert(doCopy(std::span<std::string>   (&s, 1)));
    assert(doCopy(std::span<std::string, 1>(&s, 1)));

    testCV<               int>();
    testCV<const          int>();
    testCV<      volatile int>();
    testCV<const volatile int>();
}
