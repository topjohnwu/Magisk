//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <list>

// void push_front(value_type&& x);

#include <list>
#include <cassert>

#include "MoveOnly.h"
#include "min_allocator.h"

int main()
{
    {
    std::list<MoveOnly> l1;
    l1.push_front(MoveOnly(1));
    assert(l1.size() == 1);
    assert(l1.front() == MoveOnly(1));
    l1.push_front(MoveOnly(2));
    assert(l1.size() == 2);
    assert(l1.front() == MoveOnly(2));
    assert(l1.back() == MoveOnly(1));
    }
    {
    std::list<MoveOnly, min_allocator<MoveOnly>> l1;
    l1.push_front(MoveOnly(1));
    assert(l1.size() == 1);
    assert(l1.front() == MoveOnly(1));
    l1.push_front(MoveOnly(2));
    assert(l1.size() == 2);
    assert(l1.front() == MoveOnly(2));
    assert(l1.back() == MoveOnly(1));
    }
}
