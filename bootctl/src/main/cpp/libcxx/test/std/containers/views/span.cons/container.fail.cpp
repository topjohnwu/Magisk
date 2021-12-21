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
#include <list>
#include <forward_list>
#include <deque>

#include "test_macros.h"

//  Look ma - I'm a container!
template <typename T>
struct IsAContainer {
    constexpr IsAContainer() : v_{} {}
    constexpr size_t size() const {return 1;}
    constexpr       T *data() {return &v_;}
    constexpr const T *data() const {return &v_;}

    constexpr const T *getV() const {return &v_;} // for checking
    T v_;
};

template <typename T>
struct NotAContainerNoData {
    size_t size() const {return 0;}
};

template <typename T>
struct NotAContainerNoSize {
    const T *data() const {return nullptr;}
};

template <typename T>
struct NotAContainerPrivate {
private:
    size_t size() const {return 0;}
    const T *data() const {return nullptr;}
};


int main ()
{

//  Missing size and/or data
    {
    std::span<int>    s1{IsAContainer<int>()};          // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s2{IsAContainer<int>()};          // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}
    std::span<int>    s3{NotAContainerNoData<int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s4{NotAContainerNoData<int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}
    std::span<int>    s5{NotAContainerNoSize<int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s6{NotAContainerNoSize<int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}
    std::span<int>    s7{NotAContainerPrivate<int>()};  // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s8{NotAContainerPrivate<int>()};  // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}

//  Again with the standard containers
    std::span<int>    s11{std::deque<int>()};           // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s12{std::deque<int>()};           // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}
    std::span<int>    s13{std::list<int>()};            // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s14{std::list<int>()};            // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}
    std::span<int>    s15{std::forward_list<int>()};    // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<int, 0> s16{std::forward_list<int>()};    // expected-error {{no matching constructor for initialization of 'std::span<int, 0>'}}
    }

//  Not the same type
    {
    std::span<float>    s1{IsAContainer<int>()};   // expected-error {{no matching constructor for initialization of 'std::span<float>'}}
    std::span<float, 0> s2{IsAContainer<int>()};   // expected-error {{no matching constructor for initialization of 'std::span<float, 0>'}}
    }

//  CV wrong (dynamically sized)
    {
    std::span<               int> s1{IsAContainer<const          int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<               int> s2{IsAContainer<      volatile int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<               int> s3{IsAContainer<const volatile int>()};   // expected-error {{no matching constructor for initialization of 'std::span<int>'}}
    std::span<const          int> s4{IsAContainer<      volatile int>()};   // expected-error {{no matching constructor for initialization of 'std::span<const int>'}}
    std::span<const          int> s5{IsAContainer<const volatile int>()};   // expected-error {{no matching constructor for initialization of 'std::span<const int>'}}
    std::span<      volatile int> s6{IsAContainer<const          int>()};   // expected-error {{no matching constructor for initialization of 'std::span<volatile int>'}}
    std::span<      volatile int> s7{IsAContainer<const volatile int>()};   // expected-error {{no matching constructor for initialization of 'std::span<volatile int>'}}
    }

//  CV wrong (statically sized)
    {
    std::span<               int,1> s1{IsAContainer<const          int>()}; // expected-error {{no matching constructor for initialization of 'std::span<int, 1>'}}
    std::span<               int,1> s2{IsAContainer<      volatile int>()}; // expected-error {{no matching constructor for initialization of 'std::span<int, 1>'}}
    std::span<               int,1> s3{IsAContainer<const volatile int>()}; // expected-error {{no matching constructor for initialization of 'std::span<int, 1>'}}
    std::span<const          int,1> s4{IsAContainer<      volatile int>()}; // expected-error {{no matching constructor for initialization of 'std::span<const int, 1>'}}
    std::span<const          int,1> s5{IsAContainer<const volatile int>()}; // expected-error {{no matching constructor for initialization of 'std::span<const int, 1>'}}
    std::span<      volatile int,1> s6{IsAContainer<const          int>()}; // expected-error {{no matching constructor for initialization of 'std::span<volatile int, 1>'}}
    std::span<      volatile int,1> s7{IsAContainer<const volatile int>()}; // expected-error {{no matching constructor for initialization of 'std::span<volatile int, 1>'}}
    }

}
