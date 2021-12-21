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

// priority_queue& operator=(priority_queue&& q);

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
    std::priority_queue<MoveOnly> qo(std::less<MoveOnly>(), make<std::vector<MoveOnly> >(5));
    std::priority_queue<MoveOnly> q;
    q = std::move(qo);
    assert(q.size() == 5);
    assert(q.top() == MoveOnly(4));
}
