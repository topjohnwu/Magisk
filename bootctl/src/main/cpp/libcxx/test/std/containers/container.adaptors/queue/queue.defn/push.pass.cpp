//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// void push(const value_type& v);

#include <queue>
#include <cassert>

int main()
{
    std::queue<int> q;
    q.push(1);
    assert(q.size() == 1);
    assert(q.front() == 1);
    assert(q.back() == 1);
    q.push(2);
    assert(q.size() == 2);
    assert(q.front() == 1);
    assert(q.back() == 2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.front() == 1);
    assert(q.back() == 3);
}
