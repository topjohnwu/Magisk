//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-has-no-threads

// <mutex>

// template <class Mutex> class lock_guard;

// explicit lock_guard(mutex_type& m);

#include <mutex>

int main()
{
    std::mutex m;
    std::lock_guard<std::mutex> lg = m; // expected-error{{no viable conversion}}
}
