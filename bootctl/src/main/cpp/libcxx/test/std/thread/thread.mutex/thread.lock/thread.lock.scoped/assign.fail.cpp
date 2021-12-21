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

// scoped_lock& operator=(scoped_lock const&) = delete;

#include <mutex>
#include "test_macros.h"

int main()
{
    using M = std::mutex;
    M m0, m1, m2;
    M om0, om1, om2;
    {
        using LG = std::scoped_lock<>;
        LG lg1, lg2;
        lg1 = lg2; // expected-error{{overload resolution selected deleted operator '='}}
    }
    {
        using LG = std::scoped_lock<M>;
        LG lg1(m0);
        LG lg2(om0);
        lg1 = lg2; // expected-error{{overload resolution selected deleted operator '='}}
    }
    {
        using LG = std::scoped_lock<M, M>;
        LG lg1(m0, m1);
        LG lg2(om0, om1);
        lg1 = lg2; // expected-error{{overload resolution selected deleted operator '='}}
    }
    {
        using LG = std::scoped_lock<M, M, M>;
        LG lg1(m0, m1, m2);
        LG lg2(om0, om1, om2);
        lg1 = lg2; // expected-error{{overload resolution selected deleted operator '='}}
    }
}
