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
//  ... test crashes clang

// <atomic>

// template <class Integral>
//     Integral
//     atomic_fetch_add(volatile atomic<Integral>* obj, Integral op);
//
// template <class Integral>
//     Integral
//     atomic_fetch_add(atomic<Integral>* obj, Integral op);
//
// template <class T>
//     T*
//     atomic_fetch_add(volatile atomic<T*>* obj, ptrdiff_t op);
//
// template <class T>
//     T*
//     atomic_fetch_add(atomic<T*>* obj, ptrdiff_t op);

#include <atomic>
#include <type_traits>
#include <cassert>

#include "atomic_helpers.h"

template <class T>
struct TestFn {
  void operator()() const {
    {
        typedef std::atomic<T> A;
        A t;
        std::atomic_init(&t, T(1));
        assert(std::atomic_fetch_add(&t, T(2)) == T(1));
        assert(t == T(3));
    }
    {
        typedef std::atomic<T> A;
        volatile A t;
        std::atomic_init(&t, T(1));
        assert(std::atomic_fetch_add(&t, T(2)) == T(1));
        assert(t == T(3));
    }
  }
};

template <class T>
void testp()
{
    {
        typedef std::atomic<T> A;
        typedef typename std::remove_pointer<T>::type X;
        A t;
        std::atomic_init(&t, T(1*sizeof(X)));
        assert(std::atomic_fetch_add(&t, 2) == T(1*sizeof(X)));
        assert(t == T(3*sizeof(X)));
    }
    {
        typedef std::atomic<T> A;
        typedef typename std::remove_pointer<T>::type X;
        volatile A t;
        std::atomic_init(&t, T(1*sizeof(X)));
        assert(std::atomic_fetch_add(&t, 2) == T(1*sizeof(X)));
        assert(t == T(3*sizeof(X)));
    }
}

int main()
{
    TestEachIntegralType<TestFn>()();
    testp<int*>();
    testp<const int*>();
}
