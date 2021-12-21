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

// priority_queue();

// template <class... Args> void emplace(Args&&... args);

#include <queue>
#include <cassert>

#include "../../../Emplaceable.h"

int main()
{
    std::priority_queue<Emplaceable> q;
    q.emplace(1, 2.5);
    assert(q.top() == Emplaceable(1, 2.5));
    q.emplace(3, 4.5);
    assert(q.top() == Emplaceable(3, 4.5));
    q.emplace(2, 3.5);
    assert(q.top() == Emplaceable(3, 4.5));
}
