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

// void push(value_type&& v);

#include <queue>
#include <cassert>

#include "MoveOnly.h"

int main()
{
    std::queue<MoveOnly> q;
    q.push(MoveOnly(1));
    assert(q.size() == 1);
    assert(q.front() == MoveOnly(1));
    assert(q.back() == MoveOnly(1));
    q.push(MoveOnly(2));
    assert(q.size() == 2);
    assert(q.front() == MoveOnly(1));
    assert(q.back() == MoveOnly(2));
    q.push(MoveOnly(3));
    assert(q.size() == 3);
    assert(q.front() == MoveOnly(1));
    assert(q.back() == MoveOnly(3));
}
