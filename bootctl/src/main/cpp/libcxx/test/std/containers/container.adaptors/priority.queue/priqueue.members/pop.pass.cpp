//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// priority_queue();

// void pop();

#include <queue>
#include <cassert>

int main()
{
    std::priority_queue<int> q;
    q.push(1);
    assert(q.top() == 1);
    q.push(3);
    assert(q.top() == 3);
    q.push(2);
    assert(q.top() == 3);
    q.pop();
    assert(q.top() == 2);
    q.pop();
    assert(q.top() == 1);
    q.pop();
    assert(q.empty());
}
