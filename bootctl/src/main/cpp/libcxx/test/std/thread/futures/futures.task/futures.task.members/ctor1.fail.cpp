//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03

// <future>

// class packaged_task<R(ArgTypes...)>
// template <class F>
//   packaged_task(F&& f);
// These constructors shall not participate in overload resolution if
//    decay<F>::type is the same type as std::packaged_task<R(ArgTypes...)>.

#include <future>
#include <cassert>

struct A {};
typedef std::packaged_task<A(int, char)> PT;
typedef volatile std::packaged_task<A(int, char)> VPT;


int main()
{
    VPT init{};
    auto const& c_init = init;
    PT p1{init}; // expected-error {{no matching constructor}}
    PT p2{c_init}; // expected-error {{no matching constructor}}
    PT p3{std::move(init)}; // expected-error {{no matching constructor for initialization of 'PT' (aka 'packaged_task<A (int, char)>')}}
}
