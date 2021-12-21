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
//  ... fails assertion line 31

// <atomic>

// template <class T>
//     T
//     atomic_exchange(volatile atomic<T>* obj, T desr);
//
// template <class T>
//     T
//     atomic_exchange(atomic<T>* obj, T desr);

#include <atomic>
#include <type_traits>
#include <cassert>

#include "atomic_helpers.h"

template <class T>
struct TestFn {
  void operator()() const {
    typedef std::atomic<T> A;
    A t;
    std::atomic_init(&t, T(1));
    assert(std::atomic_exchange(&t, T(2)) == T(1));
    assert(t == T(2));
    volatile A vt;
    std::atomic_init(&vt, T(3));
    assert(std::atomic_exchange(&vt, T(4)) == T(3));
    assert(vt == T(4));
  }
};


int main()
{
    TestEachAtomicType<TestFn>()();
}
