//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class set

// template <class InputIterator>
//   void insert(InputIterator first, InputIterator last);

#include <set>
#include <cassert>

#include "test_iterators.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::set<int> M;
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
        assert(m.size() == 3);
        assert(*m.begin() == 1);
        assert(*next(m.begin()) == 2);
        assert(*next(m.begin(), 2) == 3);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
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
        assert(m.size() == 3);
        assert(*m.begin() == 1);
        assert(*next(m.begin()) == 2);
        assert(*next(m.begin(), 2) == 3);
    }
#endif
}
