//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// void swap(pair& p);

#include <utility>
#include <cassert>

struct S {
    int i;
    S() : i(0) {}
    S(int j) : i(j) {}
    S * operator& () { assert(false); return this; }
    S const * operator& () const { assert(false); return this; }
    bool operator==(int x) const { return i == x; }
    };

int main()
{
    {
        typedef std::pair<int, short> P1;
        P1 p1(3, static_cast<short>(4));
        P1 p2(5, static_cast<short>(6));
        p1.swap(p2);
        assert(p1.first == 5);
        assert(p1.second == 6);
        assert(p2.first == 3);
        assert(p2.second == 4);
    }
    {
        typedef std::pair<int, S> P1;
        P1 p1(3, S(4));
        P1 p2(5, S(6));
        p1.swap(p2);
        assert(p1.first == 5);
        assert(p1.second == 6);
        assert(p2.first == 3);
        assert(p2.second == 4);
    }
}
