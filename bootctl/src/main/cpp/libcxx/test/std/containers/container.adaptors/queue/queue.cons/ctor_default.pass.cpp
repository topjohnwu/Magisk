//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// queue();

#include <queue>
#include <cassert>

#include "test_allocator.h"

int main()
{
    std::queue<int, std::vector<int, limited_allocator<int, 10> > > q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    assert(q.size() == 2);
    assert(q.front() == 1);
    assert(q.back() == 2);
}
