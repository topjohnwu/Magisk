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

// promise(const promise&) = delete;

#include <future>

#include "test_macros.h"

int main()
{
#if TEST_STD_VER >= 11
    {
        std::promise<int> p0;
        std::promise<int> p(p0); // expected-error {{call to deleted constructor of 'std::promise<int>'}}
    }
    {
        std::promise<int &> p0;
        std::promise<int &> p(p0); // expected-error {{call to deleted constructor of 'std::promise<int &>'}}
    }
    {
        std::promise<void> p0;
        std::promise<void> p(p0); // expected-error {{call to deleted constructor of 'std::promise<void>'}}
    }
#else
    {
        std::promise<int> p0;
        std::promise<int> p(p0); // expected-error {{calling a private constructor of class 'std::__1::promise<int>'}}
    }
    {
        std::promise<int &> p0;
        std::promise<int &> p(p0); // expected-error {{calling a private constructor of class 'std::__1::promise<int &>'}}
    }
    {
        std::promise<void> p0;
        std::promise<void> p(p0); // expected-error {{calling a private constructor of class 'std::__1::promise<void>'}}
    }
#endif
}
