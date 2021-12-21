//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// void pop();

#include <queue>
#include <cassert>

int main()
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.front() == 1);
    assert(q.back() == 3);
    q.pop();
    assert(q.size() == 2);
    assert(q.front() == 2);
    assert(q.back() == 3);
    q.pop();
    assert(q.size() == 1);
    assert(q.front() == 3);
    assert(q.back() == 3);
    q.pop();
    assert(q.size() == 0);
}
