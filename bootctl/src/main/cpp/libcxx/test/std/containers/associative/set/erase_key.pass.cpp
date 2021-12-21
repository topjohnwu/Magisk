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

// size_type erase(const key_type& k);

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::set<int> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(9);
        assert(m.size() == 8);
        assert(i == 0);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 4);
        assert(*next(m.begin(), 4) == 5);
        assert(*next(m.begin(), 5) == 6);
        assert(*next(m.begin(), 6) == 7);
        assert(*next(m.begin(), 7) == 8);

        i = m.erase(4);
        assert(m.size() == 7);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 5);
        assert(*next(m.begin(), 4) == 6);
        assert(*next(m.begin(), 5) == 7);
        assert(*next(m.begin(), 6) == 8);

        i = m.erase(1);
        assert(m.size() == 6);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 5);
        assert(*next(m.begin(), 3) == 6);
        assert(*next(m.begin(), 4) == 7);
        assert(*next(m.begin(), 5) == 8);

        i = m.erase(8);
        assert(m.size() == 5);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 5);
        assert(*next(m.begin(), 3) == 6);
        assert(*next(m.begin(), 4) == 7);

        i = m.erase(3);
        assert(m.size() == 4);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 5);
        assert(*next(m.begin(), 2) == 6);
        assert(*next(m.begin(), 3) == 7);

        i = m.erase(6);
        assert(m.size() == 3);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 5);
        assert(*next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 2);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 5);

        i = m.erase(2);
        assert(m.size() == 1);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 5);

        i = m.erase(5);
        assert(m.size() == 0);
        assert(i == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(9);
        assert(m.size() == 8);
        assert(i == 0);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 4);
        assert(*next(m.begin(), 4) == 5);
        assert(*next(m.begin(), 5) == 6);
        assert(*next(m.begin(), 6) == 7);
        assert(*next(m.begin(), 7) == 8);

        i = m.erase(4);
        assert(m.size() == 7);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 5);
        assert(*next(m.begin(), 4) == 6);
        assert(*next(m.begin(), 5) == 7);
        assert(*next(m.begin(), 6) == 8);

        i = m.erase(1);
        assert(m.size() == 6);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 5);
        assert(*next(m.begin(), 3) == 6);
        assert(*next(m.begin(), 4) == 7);
        assert(*next(m.begin(), 5) == 8);

        i = m.erase(8);
        assert(m.size() == 5);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 5);
        assert(*next(m.begin(), 3) == 6);
        assert(*next(m.begin(), 4) == 7);

        i = m.erase(3);
        assert(m.size() == 4);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 5);
        assert(*next(m.begin(), 2) == 6);
        assert(*next(m.begin(), 3) == 7);

        i = m.erase(6);
        assert(m.size() == 3);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 5);
        assert(*next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 2);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 2);
        assert(*next(m.begin(), 1) == 5);

        i = m.erase(2);
        assert(m.size() == 1);
        assert(i == 1);
        assert(*next(m.begin(), 0) == 5);

        i = m.erase(5);
        assert(m.size() == 0);
        assert(i == 1);
    }
#endif
}
