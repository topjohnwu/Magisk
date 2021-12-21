//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// size_type erase(const key_type& k);

#include <map>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::map<int, double> M;
        typedef std::pair<int, double> P;
        typedef M::size_type R;
        P ar[] =
        {
            P(1, 1.5),
            P(2, 2.5),
            P(3, 3.5),
            P(4, 4.5),
            P(5, 5.5),
            P(6, 6.5),
            P(7, 7.5),
            P(8, 8.5),
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        R s = m.erase(9);
        assert(s == 0);
        assert(m.size() == 8);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 1.5);
        assert(next(m.begin())->first == 2);
        assert(next(m.begin())->second == 2.5);
        assert(next(m.begin(), 2)->first == 3);
        assert(next(m.begin(), 2)->second == 3.5);
        assert(next(m.begin(), 3)->first == 4);
        assert(next(m.begin(), 3)->second == 4.5);
        assert(next(m.begin(), 4)->first == 5);
        assert(next(m.begin(), 4)->second == 5.5);
        assert(next(m.begin(), 5)->first == 6);
        assert(next(m.begin(), 5)->second == 6.5);
        assert(next(m.begin(), 6)->first == 7);
        assert(next(m.begin(), 6)->second == 7.5);
        assert(next(m.begin(), 7)->first == 8);
        assert(next(m.begin(), 7)->second == 8.5);

        s = m.erase(4);
        assert(m.size() == 7);
        assert(s == 1);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 1.5);
        assert(next(m.begin())->first == 2);
        assert(next(m.begin())->second == 2.5);
        assert(next(m.begin(), 2)->first == 3);
        assert(next(m.begin(), 2)->second == 3.5);
        assert(next(m.begin(), 3)->first == 5);
        assert(next(m.begin(), 3)->second == 5.5);
        assert(next(m.begin(), 4)->first == 6);
        assert(next(m.begin(), 4)->second == 6.5);
        assert(next(m.begin(), 5)->first == 7);
        assert(next(m.begin(), 5)->second == 7.5);
        assert(next(m.begin(), 6)->first == 8);
        assert(next(m.begin(), 6)->second == 8.5);

        s = m.erase(1);
        assert(m.size() == 6);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 3);
        assert(next(m.begin())->second == 3.5);
        assert(next(m.begin(), 2)->first == 5);
        assert(next(m.begin(), 2)->second == 5.5);
        assert(next(m.begin(), 3)->first == 6);
        assert(next(m.begin(), 3)->second == 6.5);
        assert(next(m.begin(), 4)->first == 7);
        assert(next(m.begin(), 4)->second == 7.5);
        assert(next(m.begin(), 5)->first == 8);
        assert(next(m.begin(), 5)->second == 8.5);

        s = m.erase(8);
        assert(m.size() == 5);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 3);
        assert(next(m.begin())->second == 3.5);
        assert(next(m.begin(), 2)->first == 5);
        assert(next(m.begin(), 2)->second == 5.5);
        assert(next(m.begin(), 3)->first == 6);
        assert(next(m.begin(), 3)->second == 6.5);
        assert(next(m.begin(), 4)->first == 7);
        assert(next(m.begin(), 4)->second == 7.5);

        s = m.erase(3);
        assert(m.size() == 4);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 5);
        assert(next(m.begin())->second == 5.5);
        assert(next(m.begin(), 2)->first == 6);
        assert(next(m.begin(), 2)->second == 6.5);
        assert(next(m.begin(), 3)->first == 7);
        assert(next(m.begin(), 3)->second == 7.5);

        s = m.erase(6);
        assert(m.size() == 3);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 5);
        assert(next(m.begin())->second == 5.5);
        assert(next(m.begin(), 2)->first == 7);
        assert(next(m.begin(), 2)->second == 7.5);

        s = m.erase(7);
        assert(m.size() == 2);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 5);
        assert(next(m.begin())->second == 5.5);

        s = m.erase(2);
        assert(m.size() == 1);
        assert(s == 1);
        assert(m.begin()->first == 5);
        assert(m.begin()->second == 5.5);

        s = m.erase(5);
        assert(m.size() == 0);
        assert(s == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef std::pair<int, double> P;
        typedef M::size_type R;
        P ar[] =
        {
            P(1, 1.5),
            P(2, 2.5),
            P(3, 3.5),
            P(4, 4.5),
            P(5, 5.5),
            P(6, 6.5),
            P(7, 7.5),
            P(8, 8.5),
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        R s = m.erase(9);
        assert(s == 0);
        assert(m.size() == 8);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 1.5);
        assert(next(m.begin())->first == 2);
        assert(next(m.begin())->second == 2.5);
        assert(next(m.begin(), 2)->first == 3);
        assert(next(m.begin(), 2)->second == 3.5);
        assert(next(m.begin(), 3)->first == 4);
        assert(next(m.begin(), 3)->second == 4.5);
        assert(next(m.begin(), 4)->first == 5);
        assert(next(m.begin(), 4)->second == 5.5);
        assert(next(m.begin(), 5)->first == 6);
        assert(next(m.begin(), 5)->second == 6.5);
        assert(next(m.begin(), 6)->first == 7);
        assert(next(m.begin(), 6)->second == 7.5);
        assert(next(m.begin(), 7)->first == 8);
        assert(next(m.begin(), 7)->second == 8.5);

        s = m.erase(4);
        assert(m.size() == 7);
        assert(s == 1);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 1.5);
        assert(next(m.begin())->first == 2);
        assert(next(m.begin())->second == 2.5);
        assert(next(m.begin(), 2)->first == 3);
        assert(next(m.begin(), 2)->second == 3.5);
        assert(next(m.begin(), 3)->first == 5);
        assert(next(m.begin(), 3)->second == 5.5);
        assert(next(m.begin(), 4)->first == 6);
        assert(next(m.begin(), 4)->second == 6.5);
        assert(next(m.begin(), 5)->first == 7);
        assert(next(m.begin(), 5)->second == 7.5);
        assert(next(m.begin(), 6)->first == 8);
        assert(next(m.begin(), 6)->second == 8.5);

        s = m.erase(1);
        assert(m.size() == 6);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 3);
        assert(next(m.begin())->second == 3.5);
        assert(next(m.begin(), 2)->first == 5);
        assert(next(m.begin(), 2)->second == 5.5);
        assert(next(m.begin(), 3)->first == 6);
        assert(next(m.begin(), 3)->second == 6.5);
        assert(next(m.begin(), 4)->first == 7);
        assert(next(m.begin(), 4)->second == 7.5);
        assert(next(m.begin(), 5)->first == 8);
        assert(next(m.begin(), 5)->second == 8.5);

        s = m.erase(8);
        assert(m.size() == 5);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 3);
        assert(next(m.begin())->second == 3.5);
        assert(next(m.begin(), 2)->first == 5);
        assert(next(m.begin(), 2)->second == 5.5);
        assert(next(m.begin(), 3)->first == 6);
        assert(next(m.begin(), 3)->second == 6.5);
        assert(next(m.begin(), 4)->first == 7);
        assert(next(m.begin(), 4)->second == 7.5);

        s = m.erase(3);
        assert(m.size() == 4);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 5);
        assert(next(m.begin())->second == 5.5);
        assert(next(m.begin(), 2)->first == 6);
        assert(next(m.begin(), 2)->second == 6.5);
        assert(next(m.begin(), 3)->first == 7);
        assert(next(m.begin(), 3)->second == 7.5);

        s = m.erase(6);
        assert(m.size() == 3);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 5);
        assert(next(m.begin())->second == 5.5);
        assert(next(m.begin(), 2)->first == 7);
        assert(next(m.begin(), 2)->second == 7.5);

        s = m.erase(7);
        assert(m.size() == 2);
        assert(s == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 2.5);
        assert(next(m.begin())->first == 5);
        assert(next(m.begin())->second == 5.5);

        s = m.erase(2);
        assert(m.size() == 1);
        assert(s == 1);
        assert(m.begin()->first == 5);
        assert(m.begin()->second == 5.5);

        s = m.erase(5);
        assert(m.size() == 0);
        assert(s == 1);
    }
#endif
}
