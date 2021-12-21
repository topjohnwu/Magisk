//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// REQUIRES: long_tests

// <deque>

// iterator erase(const_iterator f, const_iterator l)

#include <deque>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"
#include "test_macros.h"

#ifndef TEST_HAS_NO_EXCEPTIONS
struct Throws {
    Throws() : v_(0) {}
    Throws(int v) : v_(v) {}
    Throws(const Throws  &rhs) : v_(rhs.v_) { if (sThrows) throw 1; }
    Throws(      Throws &&rhs) : v_(rhs.v_) { if (sThrows) throw 1; }
    Throws& operator=(const Throws  &rhs) { v_ = rhs.v_; return *this; }
    Throws& operator=(      Throws &&rhs) { v_ = rhs.v_; return *this; }
    int v_;

    static bool sThrows;
    };

bool Throws::sThrows = false;
#endif


template <class C>
C
make(int size, int start = 0 )
{
    const int b = 4096 / sizeof(int);
    int init = 0;
    if (start > 0)
    {
        init = (start+1) / b + ((start+1) % b != 0);
        init *= b;
        --init;
    }
    C c(init, 0);
    for (int i = 0; i < init-start; ++i)
        c.pop_back();
    for (int i = 0; i < size; ++i)
        c.push_back(i);
    for (int i = 0; i < start; ++i)
        c.pop_front();
    return c;
}

template <class C>
void
test(int P, C& c1, int size)
{
    typedef typename C::iterator I;
    assert(static_cast<std::size_t>(P + size) <= c1.size());
    std::size_t c1_osize = c1.size();
    I i = c1.erase(c1.cbegin() + P, c1.cbegin() + (P + size));
    assert(i == c1.begin() + P);
    assert(c1.size() == c1_osize - size);
    assert(static_cast<std::size_t>(distance(c1.begin(), c1.end())) == c1.size());
    i = c1.begin();
    int j = 0;
    for (; j < P; ++j, ++i)
        assert(*i == j);
    for (j += size; static_cast<std::size_t>(j) < c1_osize; ++j, ++i)
        assert(*i == j);
}

template <class C>
void
testN(int start, int N)
{
    int pstep = std::max(N / std::max(std::min(N, 10), 1), 1);
    for (int p = 0; p <= N; p += pstep)
    {
        int sstep = std::max((N - p) / std::max(std::min(N - p, 10), 1), 1);
        for (int s = 0; s <= N - p; s += sstep)
        {
            C c1 = make<C>(N, start);
            test(p, c1, s);
        }
    }
}

int main()
{
    {
    int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
    const int N = sizeof(rng)/sizeof(rng[0]);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            testN<std::deque<int> >(rng[i], rng[j]);
    }
#if TEST_STD_VER >= 11
    {
    int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
    const int N = sizeof(rng)/sizeof(rng[0]);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            testN<std::deque<int, min_allocator<int>> >(rng[i], rng[j]);
    }
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
// Test for LWG2953:
// Throws: Nothing unless an exception is thrown by the assignment operator of T.
// (which includes move assignment)
    {
    Throws arr[] = {1, 2, 3};
    std::deque<Throws> v(arr, arr+3);
    Throws::sThrows = true;
    v.erase(v.begin(), --v.end());
    assert(v.size() == 1);
    v.erase(v.begin(), v.end());
    assert(v.size() == 0);
    }
#endif
}
