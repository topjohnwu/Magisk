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

// template <class U, class V> pair(const pair<U, V>& p);

#include <utility>
#include <cassert>

int main()
{
    {
        typedef std::pair<int, short> P1;
        typedef std::pair<double, long> P2;
        const P1 p1(3, static_cast<short>(4));
        const P2 p2 = p1;
        assert(p2.first == 3);
        assert(p2.second == 4);
    }
}
