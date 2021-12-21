//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03, c++11, c++14

// <mutex>

// template <class ...Mutex> class scoped_lock;

// explicit scoped_lock(Mutex&...);

#include <mutex>
#include "test_macros.h"

template <class LG>
void test_conversion(LG) {}

int main()
{
    using M = std::mutex;
    M m0, m1, m2;
    M n0, n1, n2;
    {
        using LG = std::scoped_lock<>;
        LG lg = {}; // expected-error{{chosen constructor is explicit in copy-initialization}}
        test_conversion<LG>({}); // expected-error{{no matching function for call}}
        ((void)lg);
    }
    {
        using LG = std::scoped_lock<M>;
        LG lg = {m0}; // expected-error{{chosen constructor is explicit in copy-initialization}}
        test_conversion<LG>({n0}); // expected-error{{no matching function for call}}
        ((void)lg);
    }
    {
        using LG = std::scoped_lock<M, M>;
        LG lg = {m0, m1}; // expected-error{{chosen constructor is explicit in copy-initialization}}
        test_conversion<LG>({n0, n1}); // expected-error{{no matching function for call}}
        ((void)lg);
    }
    {
        using LG = std::scoped_lock<M, M, M>;
        LG lg = {m0, m1, m2}; // expected-error{{chosen constructor is explicit in copy-initialization}}
        test_conversion<LG>({n0, n1, n2}); // expected-error{{no matching function for call}}
    }
}
