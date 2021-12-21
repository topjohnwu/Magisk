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

// void swap(priority_queue& q);

#include <queue>
#include <cassert>

int main()
{
    std::priority_queue<int> q1;
    std::priority_queue<int> q2;
    q1.push(1);
    q1.push(3);
    q1.push(2);
    q1.swap(q2);
    assert(q1.empty());
    assert(q2.size() == 3);
    assert(q2.top() == 3);
}
