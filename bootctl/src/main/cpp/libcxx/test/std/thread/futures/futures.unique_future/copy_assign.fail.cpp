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

// <future>

// class future<R>

// future& operator=(const future&) = delete;

#include <future>

#include "test_macros.h"

int main()
{
#if TEST_STD_VER >= 11
    {
        std::future<int> f0, f;
        f = f0; // expected-error {{overload resolution selected deleted operator '='}}
    }
    {
        std::future<int &> f0, f;
        f = f0; // expected-error {{overload resolution selected deleted operator '='}}
    }
    {
        std::future<void> f0, f;
        f = f0; // expected-error {{overload resolution selected deleted operator '='}}
    }
#else
    {
        std::future<int> f0, f;
        f = f0; // expected-error {{'operator=' is a private member of 'std::__1::future<int>'}}
    }
    {
        std::future<int &> f0, f;
        f = f0; // expected-error {{'operator=' is a private member of 'std::__1::future<int &>'}}
    }
    {
        std::future<void> f0, f;
        f = f0; // expected-error {{'operator=' is a private member of 'std::__1::future<void>'}}
    }
#endif
}
