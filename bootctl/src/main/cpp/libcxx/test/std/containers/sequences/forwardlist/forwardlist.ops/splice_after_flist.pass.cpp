//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// void splice_after(const_iterator p, forward_list&& x);

#include <forward_list>
#include <cassert>
#include <iterator>
#include <cstddef>

#include "min_allocator.h"

typedef int T;
const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7};
const T t2[] = {10, 11, 12, 13, 14, 15};
const std::ptrdiff_t size_t1 = std::end(t1) - std::begin(t1);
const std::ptrdiff_t size_t2 = std::end(t2) - std::begin(t2);

template <class C>
void
testd(const C& c, int p, int l)
{
    typename C::const_iterator i = c.begin();
    int n1 = 0;
    for (; n1 < p; ++n1, ++i)
        assert(*i == t1[n1]);
    for (int n2 = 0; n2 < l; ++n2, ++i)
        assert(*i == t2[n2]);
    for (; n1 < size_t1; ++n1, ++i)
        assert(*i == t1[n1]);
    assert(distance(c.begin(), c.end()) == size_t1 + l);
}

int main()
{
    {
    // splicing different containers
    typedef std::forward_list<T> C;
    for (int l = 0; l <= size_t2; ++l)
    {
        for (int p = 0; p <= size_t1; ++p)
        {
            C c1(std::begin(t1), std::end(t1));
            C c2(t2, t2+l);

            c1.splice_after(next(c1.cbefore_begin(), p), std::move(c2));
            testd(c1, p, l);
        }
    }
    }
#if TEST_STD_VER >= 11
    {
    // splicing different containers
    typedef std::forward_list<T, min_allocator<T>> C;
    for (int l = 0; l <= size_t2; ++l)
    {
        for (int p = 0; p <= size_t1; ++p)
        {
            C c1(std::begin(t1), std::end(t1));
            C c2(t2, t2+l);

            c1.splice_after(next(c1.cbefore_begin(), p), std::move(c2));
            testd(c1, p, l);
        }
    }
    }
#endif
}
