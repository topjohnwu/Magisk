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

// class promise<R>

// promise& operator=(const promise& rhs) = delete;

#include <future>

#include "test_macros.h"

int main()
{
#if TEST_STD_VER >= 11
    {
        std::promise<int> p0, p;
        p = p0; // expected-error {{overload resolution selected deleted operator '='}}
    }
    {
        std::promise<int&> p0, p;
        p = p0; // expected-error {{overload resolution selected deleted operator '='}}
    }
    {
        std::promise<void> p0, p;
        p = p0; // expected-error {{overload resolution selected deleted operator '='}}
    }
#else
    {
        std::promise<int> p0, p;
        p = p0; // expected-error {{'operator=' is a private member of 'std::__1::promise<int>'}}
    }
    {
        std::promise<int&> p0, p;
        p = p0; // expected-error {{'operator=' is a private member of 'std::__1::promise<int &>'}}
    }
    {
        std::promise<void> p0, p;
        p = p0; // expected-error {{'operator=' is a private member of 'std::__1::promise<void>'}}
    }
#endif
}
