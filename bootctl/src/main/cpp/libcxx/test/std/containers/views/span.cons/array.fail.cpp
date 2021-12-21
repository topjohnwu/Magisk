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
//     constexpr span(element_type (&arr)[N]) noexcept;
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

               int   arr[] = {1,2,3};
const          int  carr[] = {4,5,6};
      volatile int  varr[] = {7,8,9};
const volatile int cvarr[] = {1,3,5};

int main ()
{
//  Size wrong
    {
    std::span<int, 2>   s1(arr); // expected-error {{no matching constructor for initialization of 'std::span<int, 2>'}}
    }

//  Type wrong
    {
    std::span<float>    s1(arr);   // expected-error {{no matching constructor for initialization of 'std::span<float>'}}
    std::span<float, 3> s2(arr);   // expected-error {{no matching constructor for initialization of 'std::span<float, 3>'}}
    }

//  CV wrong (dynamically sized)
    {
    std::span<               int> s1{ carr};    // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<               int> s2{ varr};    // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<               int> s3{cvarr};    // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<const          int> s4{ varr};    // expected-error {{no matching constructor for initialization of 'std::span<const int>'}}
    std::span<const          int> s5{cvarr};    // expected-error {{no matching constructor for initialization of 'std::span<const int>'}}
    std::span<      volatile int> s6{ carr};    // expected-error {{no matching constructor for initialization of 'std::span<volatile int>'}}
    std::span<      volatile int> s7{cvarr};    // expected-error {{no matching constructor for initialization of 'std::span<volatile int>'}}
    }

//  CV wrong (statically sized)
    {
    std::span<               int,3> s1{ carr};  // expected-error {{no matching constructor for initialization of 'std::span<int, 3>'}}
    std::span<               int,3> s2{ varr};  // expected-error {{no matching constructor for initialization of 'std::span<int, 3>'}}
    std::span<               int,3> s3{cvarr};  // expected-error {{no matching constructor for initialization of 'std::span<int, 3>'}}
    std::span<const          int,3> s4{ varr};  // expected-error {{no matching constructor for initialization of 'std::span<const int, 3>'}}
    std::span<const          int,3> s5{cvarr};  // expected-error {{no matching constructor for initialization of 'std::span<const int, 3>'}}
    std::span<      volatile int,3> s6{ carr};  // expected-error {{no matching constructor for initialization of 'std::span<volatile int, 3>'}}
    std::span<      volatile int,3> s7{cvarr};  // expected-error {{no matching constructor for initialization of 'std::span<volatile int, 3>'}}
    }
}
