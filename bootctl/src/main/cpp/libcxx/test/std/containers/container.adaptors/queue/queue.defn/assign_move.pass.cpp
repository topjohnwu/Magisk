//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <queue>

// queue& operator=(queue&& q);

#include <queue>
#include <cassert>

#include "MoveOnly.h"


template <class C>
C
make(int n)
{
    C c;
    for (int i = 0; i < n; ++i)
        c.push_back(MoveOnly(i));
    return c;
}


int main()
{
    std::queue<MoveOnly> q(make<std::deque<MoveOnly> >(5));
    std::queue<MoveOnly> q2;
    q2 = std::move(q);
    assert(q2.size() == 5);
    assert(q.empty());
}
