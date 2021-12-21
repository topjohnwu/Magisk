//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// void splice_after(const_iterator p, forward_list&& x,
//                   const_iterator first, const_iterator last);

#include <forward_list>
#include <cassert>
#include <iterator>

#include "min_allocator.h"

typedef int T;
const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7};
const T t2[] = {10, 11, 12, 13, 14, 15};
const int size_t1 = std::end(t1) - std::begin(t1);
const int size_t2 = std::end(t2) - std::begin(t2);

template <class C>
void
testd(const C& c, int p, int f, int l)
{
    typename C::const_iterator i = c.begin();
    int n1 = 0;
    for (; n1 < p; ++n1, ++i)
        assert(*i == t1[n1]);
    for (int n2 = f; n2 < l-1; ++n2, ++i)
        assert(*i == t2[n2]);
    for (; n1 < size_t1; ++n1, ++i)
        assert(*i == t1[n1]);
    assert(distance(c.begin(), c.end()) == size_t1 + (l > f+1 ? l-1-f : 0));
}

template <class C>
void
tests(const C& c, int p, int f, int l)
{
    typename C::const_iterator i = c.begin();
    int n = 0;
    int d = l > f+1 ? l-1-f : 0;
    if (d == 0 || p == f)
    {
        for (n = 0; n < size_t1; ++n, ++i)
            assert(*i == t1[n]);
    }
    else if (p < f)
    {
        for (n = 0; n < p; ++n, ++i)
            assert(*i == t1[n]);
        for (n = f; n < l-1; ++n, ++i)
            assert(*i == t1[n]);
        for (n = p; n < f; ++n, ++i)
            assert(*i == t1[n]);
        for (n = l-1; n < size_t1; ++n, ++i)
            assert(*i == t1[n]);
    }
    else // p > f
    {
        for (n = 0; n < f; ++n, ++i)
            assert(*i == t1[n]);
        for (n = l-1; n < p; ++n, ++i)
            assert(*i == t1[n]);
        for (n = f; n < l-1; ++n, ++i)
            assert(*i == t1[n]);
        for (n = p; n < size_t1; ++n, ++i)
            assert(*i == t1[n]);
    }
    assert(distance(c.begin(), c.end()) == size_t1);
}

int main()
{
    {
    // splicing different containers
    typedef std::forward_list<T> C;
    for (int f = 0; f <= size_t2+1; ++f)
    {
        for (int l = f; l <= size_t2+1; ++l)
        {
            for (int p = 0; p <= size_t1; ++p)
            {
                C c1(std::begin(t1), std::end(t1));
                C c2(std::begin(t2), std::end(t2));

                c1.splice_after(next(c1.cbefore_begin(), p), std::move(c2),
                      next(c2.cbefore_begin(), f), next(c2.cbefore_begin(), l));
                testd(c1, p, f, l);
            }
        }
    }

    // splicing within same container
    for (int f = 0; f <= size_t1+1; ++f)
    {
        for (int l = f; l <= size_t1; ++l)
        {
            for (int p = 0; p <= f; ++p)
            {
                C c1(std::begin(t1), std::end(t1));

                c1.splice_after(next(c1.cbefore_begin(), p), std::move(c1),
                      next(c1.cbefore_begin(), f), next(c1.cbefore_begin(), l));
                tests(c1, p, f, l);
            }
            for (int p = l; p <= size_t1; ++p)
            {
                C c1(std::begin(t1), std::end(t1));

                c1.splice_after(next(c1.cbefore_begin(), p), std::move(c1),
                      next(c1.cbefore_begin(), f), next(c1.cbefore_begin(), l));
                tests(c1, p, f, l);
            }
        }
    }
    }
#if TEST_STD_VER >= 11
    {
    // splicing different containers
    typedef std::forward_list<T, min_allocator<T>> C;
    for (int f = 0; f <= size_t2+1; ++f)
    {
        for (int l = f; l <= size_t2+1; ++l)
        {
            for (int p = 0; p <= size_t1; ++p)
            {
                C c1(std::begin(t1), std::end(t1));
                C c2(std::begin(t2), std::end(t2));

                c1.splice_after(next(c1.cbefore_begin(), p), std::move(c2),
                      next(c2.cbefore_begin(), f), next(c2.cbefore_begin(), l));
                testd(c1, p, f, l);
            }
        }
    }

    // splicing within same container
    for (int f = 0; f <= size_t1+1; ++f)
    {
        for (int l = f; l <= size_t1; ++l)
        {
            for (int p = 0; p <= f; ++p)
            {
                C c1(std::begin(t1), std::end(t1));

                c1.splice_after(next(c1.cbefore_begin(), p), std::move(c1),
                      next(c1.cbefore_begin(), f), next(c1.cbefore_begin(), l));
                tests(c1, p, f, l);
            }
            for (int p = l; p <= size_t1; ++p)
            {
                C c1(std::begin(t1), std::end(t1));

                c1.splice_after(next(c1.cbefore_begin(), p), std::move(c1),
                      next(c1.cbefore_begin(), f), next(c1.cbefore_begin(), l));
                tests(c1, p, f, l);
            }
        }
    }
    }
#endif
}
