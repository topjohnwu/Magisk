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

// scoped_lock(scoped_lock const&) = delete;

#include <mutex>
#include "test_macros.h"

int main()
{
    using M = std::mutex;
    M m0, m1, m2;
    {
        using LG = std::scoped_lock<>;
        const LG Orig;
        LG Copy(Orig); // expected-error{{call to deleted constructor of 'LG'}}
    }
    {
        using LG = std::scoped_lock<M>;
        const LG Orig(m0);
        LG Copy(Orig); // expected-error{{call to deleted constructor of 'LG'}}
    }
    {
        using LG = std::scoped_lock<M, M>;
        const LG Orig(m0, m1);
        LG Copy(Orig); // expected-error{{call to deleted constructor of 'LG'}}
    }
    {
        using LG = std::scoped_lock<M, M, M>;
        const LG Orig(m0, m1, m2);
        LG Copy(Orig); // expected-error{{call to deleted constructor of 'LG'}}
    }
}
