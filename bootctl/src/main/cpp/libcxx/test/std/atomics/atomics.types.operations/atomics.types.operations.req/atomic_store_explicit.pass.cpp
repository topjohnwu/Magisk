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

// <atomic>

// template <class T>
//     void
//     atomic_store_explicit(volatile atomic<T>* obj, T desr, memory_order m);
//
// template <class T>
//     void
//     atomic_store_explicit(atomic<T>* obj, T desr, memory_order m);

#include <atomic>
#include <type_traits>
#include <cassert>

#include "atomic_helpers.h"

template <class T>
struct TestFn {
  void operator()() const {
    typedef std::atomic<T> A;
    A t;
    std::atomic_store_explicit(&t, T(1), std::memory_order_seq_cst);
    assert(t == T(1));
    volatile A vt;
    std::atomic_store_explicit(&vt, T(2), std::memory_order_seq_cst);
    assert(vt == T(2));
  }
};


int main()
{
    TestEachAtomicType<TestFn>()();
}
