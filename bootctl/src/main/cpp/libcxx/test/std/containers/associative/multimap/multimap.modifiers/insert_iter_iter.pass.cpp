//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class multimap

// template <class InputIterator>
//   void insert(InputIterator first, InputIterator last);

#include <map>
#include <cassert>

#include "test_iterators.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::multimap<int, double> M;
        typedef std::pair<int, double> P;
        P ar[] =
        {
            P(1, 1),
            P(1, 1.5),
            P(1, 2),
            P(2, 1),
            P(2, 1.5),
            P(2, 2),
            P(3, 1),
            P(3, 1.5),
            P(3, 2),
        };
        M m;
        m.insert(input_iterator<P*>(ar), input_iterator<P*>(ar + sizeof(ar)/sizeof(ar[0])));
        assert(m.size() == 9);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 1);
        assert(next(m.begin())->first == 1);
        assert(next(m.begin())->second == 1.5);
        assert(next(m.begin(), 2)->first == 1);
        assert(next(m.begin(), 2)->second == 2);
        assert(next(m.begin(), 3)->first == 2);
        assert(next(m.begin(), 3)->second == 1);
        assert(next(m.begin(), 4)->first == 2);
        assert(next(m.begin(), 4)->second == 1.5);
        assert(next(m.begin(), 5)->first == 2);
        assert(next(m.begin(), 5)->second == 2);
        assert(next(m.begin(), 6)->first == 3);
        assert(next(m.begin(), 6)->second == 1);
        assert(next(m.begin(), 7)->first == 3);
        assert(next(m.begin(), 7)->second == 1.5);
        assert(next(m.begin(), 8)->first == 3);
        assert(next(m.begin(), 8)->second == 2);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef std::pair<int, double> P;
        P ar[] =
        {
            P(1, 1),
            P(1, 1.5),
            P(1, 2),
            P(2, 1),
            P(2, 1.5),
            P(2, 2),
            P(3, 1),
            P(3, 1.5),
            P(3, 2),
        };
        M m;
        m.insert(input_iterator<P*>(ar), input_iterator<P*>(ar + sizeof(ar)/sizeof(ar[0])));
        assert(m.size() == 9);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 1);
        assert(next(m.begin())->first == 1);
        assert(next(m.begin())->second == 1.5);
        assert(next(m.begin(), 2)->first == 1);
        assert(next(m.begin(), 2)->second == 2);
        assert(next(m.begin(), 3)->first == 2);
        assert(next(m.begin(), 3)->second == 1);
        assert(next(m.begin(), 4)->first == 2);
        assert(next(m.begin(), 4)->second == 1.5);
        assert(next(m.begin(), 5)->first == 2);
        assert(next(m.begin(), 5)->second == 2);
        assert(next(m.begin(), 6)->first == 3);
        assert(next(m.begin(), 6)->second == 1);
        assert(next(m.begin(), 7)->first == 3);
        assert(next(m.begin(), 7)->second == 1.5);
        assert(next(m.begin(), 8)->first == 3);
        assert(next(m.begin(), 8)->second == 2);
    }
#endif
}
