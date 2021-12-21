//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class multiset

// template <class InputIterator>
//   void insert(InputIterator first, InputIterator last);

#include <set>
#include <cassert>

#include "test_iterators.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::multiset<int> M;
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        M m;
        m.insert(input_iterator<const V*>(ar),
                 input_iterator<const V*>(ar + sizeof(ar)/sizeof(ar[0])));
        assert(m.size() == 9);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 1);
        assert(*next(m.begin(), 2) == 1);
        assert(*next(m.begin(), 3) == 2);
        assert(*next(m.begin(), 4) == 2);
        assert(*next(m.begin(), 5) == 2);
        assert(*next(m.begin(), 6) == 3);
        assert(*next(m.begin(), 7) == 3);
        assert(*next(m.begin(), 8) == 3);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        M m;
        m.insert(input_iterator<const V*>(ar),
                 input_iterator<const V*>(ar + sizeof(ar)/sizeof(ar[0])));
        assert(m.size() == 9);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 1);
        assert(*next(m.begin(), 2) == 1);
        assert(*next(m.begin(), 3) == 2);
        assert(*next(m.begin(), 4) == 2);
        assert(*next(m.begin(), 5) == 2);
        assert(*next(m.begin(), 6) == 3);
        assert(*next(m.begin(), 7) == 3);
        assert(*next(m.begin(), 8) == 3);
    }
#endif
}
