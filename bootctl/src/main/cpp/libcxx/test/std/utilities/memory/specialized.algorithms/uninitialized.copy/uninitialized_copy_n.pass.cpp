//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class InputIterator, class Size, class ForwardIterator>
//   ForwardIterator
//   uninitialized_copy_n(InputIterator first, Size n,
//                        ForwardIterator result);

#include <memory>
#include <cassert>

#include "test_macros.h"

struct B
{
    static int count_;
    static int population_;
    int data_;
    explicit B() : data_(1) { ++population_; }
    B(const B &b) {
      ++count_;
      if (count_ == 3)
        TEST_THROW(1);
      data_ = b.data_;
      ++population_;
    }
    ~B() {data_ = 0; --population_; }
};

int B::population_ = 0;
int B::count_ = 0;

struct Nasty
{
    Nasty() : i_ ( counter_++ ) {}
    Nasty * operator &() const { return NULL; }
    int i_;
    static int counter_;
};

int Nasty::counter_ = 0;

int main()
{
    {
    const int N = 5;
    char pool[sizeof(B)*N] = {0};
    B* bp = (B*)pool;
    B b[N];
    assert(B::population_ == N);
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        std::uninitialized_copy_n(b, 5, bp);
        assert(false);
    }
    catch (...)
    {
        assert(B::population_ == N);
    }
#endif
    B::count_ = 0;
    std::uninitialized_copy_n(b, 2, bp);
    for (int i = 0; i < 2; ++i)
        assert(bp[i].data_ == 1);
    assert(B::population_ == N + 2);
    }

    {
    const int N = 5;
    char pool[sizeof(Nasty)*N] = {0};
    Nasty * p = (Nasty *) pool;
    Nasty arr[N];
    std::uninitialized_copy_n(arr, N, p);
    for (int i = 0; i < N; ++i) {
        assert(arr[i].i_ == i);
        assert(  p[i].i_ == i);
    }
    }
}
