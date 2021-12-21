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
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8

// <future>

// template <class F, class... Args>
//     future<typename result_of<F(Args...)>::type>
//     async(F&& f, Args&&... args);

// template <class F, class... Args>
//     future<typename result_of<F(Args...)>::type>
//     async(launch policy, F&& f, Args&&... args);


#include <future>
#include <atomic>
#include <memory>
#include <cassert>

#include "test_macros.h"

int foo (int x) { return x; }

int main ()
{
    std::async(                    foo, 3); // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}
    std::async(std::launch::async, foo, 3); // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}
}
