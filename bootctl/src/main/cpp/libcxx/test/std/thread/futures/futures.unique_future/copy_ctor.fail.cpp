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

// future(const future&) = delete;

#include <future>

#include "test_macros.h"

int main()
{
#if TEST_STD_VER >= 11
    {
        std::future<int> f0;
        std::future<int> f = f0; // expected-error {{call to deleted constructor of 'std::future<int>'}}
    }
    {
        std::future<int &> f0;
        std::future<int &> f = f0; // expected-error {{call to deleted constructor of 'std::future<int &>'}}
    }
    {
        std::future<void> f0;
        std::future<void> f = f0; // expected-error {{call to deleted constructor of 'std::future<void>'}}
    }
#else
    {
        std::future<int> f0;
        std::future<int> f = f0; // expected-error {{calling a private constructor of class 'std::__1::future<int>'}}
    }
    {
        std::future<int &> f0;
        std::future<int &> f = f0; // expected-error {{calling a private constructor of class 'std::__1::future<int &>'}}
    }
    {
        std::future<void> f0;
        std::future<void> f = f0; // expected-error {{calling a private constructor of class 'std::__1::future<void>'}}
    }
#endif
}
