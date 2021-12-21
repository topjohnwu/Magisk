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
//  ... assertion fails line 31

// <atomic>

// template <class T>
//     T
//     atomic_load_explicit(const volatile atomic<T>* obj, memory_order m);
//
// template <class T>
//     T
//     atomic_load_explicit(const atomic<T>* obj, memory_order m);

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
    assert(std::atomic_load_explicit(&t, std::memory_order_seq_cst) == T(1));
    volatile A vt;
    std::atomic_init(&vt, T(2));
    assert(std::atomic_load_explicit(&vt, std::memory_order_seq_cst) == T(2));
  }
};

int main()
{
    TestEachAtomicType<TestFn>()();
}
