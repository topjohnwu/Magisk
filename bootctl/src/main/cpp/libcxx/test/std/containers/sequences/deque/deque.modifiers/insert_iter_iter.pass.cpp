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

// template <class InputIterator>
//   iterator insert (const_iterator p, InputIterator f, InputIterator l);

#include <deque>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_iterators.h"
#include "MoveOnly.h"
#include "test_allocator.h"
#include "min_allocator.h"

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
test(int P, const C& c0, const C& c2)
{
    {
    typedef typename C::const_iterator CI;
    typedef input_iterator<CI> BCI;
    C c1 = c0;
    std::size_t c1_osize = c1.size();
    CI i = c1.insert(c1.begin() + P, BCI(c2.begin()), BCI(c2.end()));
    assert(i == c1.begin() + P);
    assert(c1.size() == c1_osize + c2.size());
    assert(static_cast<std::size_t>(distance(c1.begin(), c1.end())) == c1.size());
    i = c1.begin();
    for (int j = 0; j < P; ++j, ++i)
        assert(*i == j);
    for (int j = 0; static_cast<std::size_t>(j) < c2.size(); ++j, ++i)
        assert(*i == j);
    for (int j = P; static_cast<std::size_t>(j) < c1_osize; ++j, ++i)
        assert(*i == j);
    }
    {
    typedef typename C::const_iterator CI;
    typedef forward_iterator<CI> BCI;
    C c1 = c0;
    std::size_t c1_osize = c1.size();
    CI i = c1.insert(c1.begin() + P, BCI(c2.begin()), BCI(c2.end()));
    assert(i == c1.begin() + P);
    assert(c1.size() == c1_osize + c2.size());
    assert(static_cast<std::size_t>(distance(c1.begin(), c1.end())) == c1.size());
    i = c1.begin();
    for (int j = 0; j < P; ++j, ++i)
        assert(*i == j);
    for (int j = 0; static_cast<std::size_t>(j) < c2.size(); ++j, ++i)
        assert(*i == j);
    for (int j = P; static_cast<std::size_t>(j) < c1_osize; ++j, ++i)
        assert(*i == j);
    }
    {
    typedef typename C::const_iterator CI;
    typedef bidirectional_iterator<CI> BCI;
    C c1 = c0;
    std::size_t c1_osize = c1.size();
    CI i = c1.insert(c1.begin() + P, BCI(c2.begin()), BCI(c2.end()));
    assert(i == c1.begin() + P);
    assert(c1.size() == c1_osize + c2.size());
    assert(static_cast<std::size_t>(distance(c1.begin(), c1.end())) == c1.size());
    i = c1.begin();
    for (int j = 0; j < P; ++j, ++i)
        assert(*i == j);
    for (int j = 0; static_cast<std::size_t>(j) < c2.size(); ++j, ++i)
        assert(*i == j);
    for (int j = P; static_cast<std::size_t>(j) < c1_osize; ++j, ++i)
        assert(*i == j);
    }
}

template <class C>
void
testN(int start, int N, int M)
{
    for (int i = 0; i <= 3; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            test(i, c1, c2);
        }
    }
    for (int i = M-1; i <= M+1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            test(i, c1, c2);
        }
    }
    for (int i = N/2-1; i <= N/2+1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            test(i, c1, c2);
        }
    }
    for (int i = N - M - 1; i <= N - M + 1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            test(i, c1, c2);
        }
    }
    for (int i = N - M - 1; i <= N - M + 1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            test(i, c1, c2);
        }
    }
    for (int i = N - 3; i <= N; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            test(i, c1, c2);
        }
    }
}

template <class C>
void
testI(int P, C& c1, const C& c2)
{
    typedef typename C::const_iterator CI;
    typedef input_iterator<CI> ICI;
    std::size_t c1_osize = c1.size();
    CI i = c1.insert(c1.begin() + P, ICI(c2.begin()), ICI(c2.end()));
    assert(i == c1.begin() + P);
    assert(c1.size() == c1_osize + c2.size());
    assert(static_cast<std::size_t>(distance(c1.begin(), c1.end())) == c1.size());
    i = c1.begin();
    for (int j = 0; j < P; ++j, ++i)
        assert(*i == j);
    for (int j = 0; static_cast<std::size_t>(j) < c2.size(); ++j, ++i)
        assert(*i == j);
    for (int j = P; static_cast<std::size_t>(j) < c1_osize; ++j, ++i)
        assert(*i == j);
}

template <class C>
void
testNI(int start, int N, int M)
{
    for (int i = 0; i <= 3; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            testI(i, c1, c2);
        }
    }
    for (int i = M-1; i <= M+1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            testI(i, c1, c2);
        }
    }
    for (int i = N/2-1; i <= N/2+1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            testI(i, c1, c2);
        }
    }
    for (int i = N - M - 1; i <= N - M + 1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            testI(i, c1, c2);
        }
    }
    for (int i = N - 3; i <= N; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            C c2 = make<C>(M);
            testI(i, c1, c2);
        }
    }
}

template <class C>
void
test_move()
{
#if TEST_STD_VER >= 11
    C c;
    typedef typename C::const_iterator CI;
    {
        MoveOnly mo(0);
        typedef MoveOnly* I;
        c.insert(c.end(), std::move_iterator<I>(&mo), std::move_iterator<I>(&mo+1));
    }
    int j = 0;
    for (CI i = c.begin(); i != c.end(); ++i, ++j)
        assert(*i == MoveOnly(j));
    {
        MoveOnly mo(1);
        typedef input_iterator<MoveOnly*> I;
        c.insert(c.end(), std::move_iterator<I>(I(&mo)), std::move_iterator<I>(I(&mo+1)));
    }
    j = 0;
    for (CI i = c.begin(); i != c.end(); ++i, ++j)
        assert(*i == MoveOnly(j));
#endif
}

int main()
{
    {
    int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
    const int N = sizeof(rng)/sizeof(rng[0]);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            for (int k = 0; k < N; ++k)
                testN<std::deque<int> >(rng[i], rng[j], rng[k]);
    testNI<std::deque<int> >(1500, 2000, 1000);
#if TEST_STD_VER >= 11
    test_move<std::deque<MoveOnly, limited_allocator<MoveOnly, 2000> > >();
#endif
    }
#if TEST_STD_VER >= 11
    {
    int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
    const int N = sizeof(rng)/sizeof(rng[0]);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            for (int k = 0; k < N; ++k)
                testN<std::deque<int, min_allocator<int>> >(rng[i], rng[j], rng[k]);
    testNI<std::deque<int> >(1500, 2000, 1000);
    test_move<std::deque<MoveOnly, min_allocator<MoveOnly> > >();
    }
#endif
}
