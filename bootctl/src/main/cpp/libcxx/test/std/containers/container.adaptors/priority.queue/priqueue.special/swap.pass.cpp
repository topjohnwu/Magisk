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

// template <class T, class Container, class Compare>
//   void swap(priority_queue<T, Container, Compare>& x,
//             priority_queue<T, Container, Compare>& y);

#include <queue>
#include <cassert>

int main()
{
    std::priority_queue<int> q1;
    std::priority_queue<int> q2;
    q1.push(1);
    q1.push(3);
    q1.push(2);
    swap(q1, q2);
    assert(q1.empty());
    assert(q2.size() == 3);
    assert(q2.top() == 3);
}
