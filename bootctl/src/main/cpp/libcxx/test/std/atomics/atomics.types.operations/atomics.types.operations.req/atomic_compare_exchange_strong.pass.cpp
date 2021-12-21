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
//  ... assertion fails line 34

// <atomic>

// template <class T>
//     bool
//     atomic_compare_exchange_strong(volatile atomic<T>* obj, T* expc, T desr);
//
// template <class T>
//     bool
//     atomic_compare_exchange_strong(atomic<T>* obj, T* expc, T desr);

#include <atomic>
#include <type_traits>
#include <cassert>

#include "atomic_helpers.h"

template <class T>
struct TestFn {
  void operator()() const {
    {
        typedef std::atomic<T> A;
        A a;
        T t(T(1));
        std::atomic_init(&a, t);
        assert(std::atomic_compare_exchange_strong(&a, &t, T(2)) == true);
        assert(a == T(2));
        assert(t == T(1));
        assert(std::atomic_compare_exchange_strong(&a, &t, T(3)) == false);
        assert(a == T(2));
        assert(t == T(2));
    }
    {
        typedef std::atomic<T> A;
        volatile A a;
        T t(T(1));
        std::atomic_init(&a, t);
        assert(std::atomic_compare_exchange_strong(&a, &t, T(2)) == true);
        assert(a == T(2));
        assert(t == T(1));
        assert(std::atomic_compare_exchange_strong(&a, &t, T(3)) == false);
        assert(a == T(2));
        assert(t == T(2));
    }
  }
};

int main()
{
    TestEachAtomicType<TestFn>()();
}
