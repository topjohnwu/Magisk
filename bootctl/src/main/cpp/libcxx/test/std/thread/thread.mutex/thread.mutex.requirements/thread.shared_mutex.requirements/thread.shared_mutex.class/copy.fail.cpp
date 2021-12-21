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
// UNSUPPORTED: c++98, c++03, c++11, c++14

// <shared_mutex>

// class shared_mutex;

// shared_mutex(const shared_mutex&) = delete;

#include <shared_mutex>

int main()
{
    std::shared_mutex m0;
    std::shared_mutex m1(m0); // expected-error {{call to deleted constructor of 'std::shared_mutex'}}
}
