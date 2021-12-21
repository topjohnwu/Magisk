//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// void swap(queue& q);

#include <queue>
#include <cassert>

template <class C>
C
make(int n)
{
    C c;
    for (int i = 0; i < n; ++i)
        c.push(i);
    return c;
}

int main()
{
    std::queue<int> q1 = make<std::queue<int> >(5);
    std::queue<int> q2 = make<std::queue<int> >(10);
    std::queue<int> q1_save = q1;
    std::queue<int> q2_save = q2;
    q1.swap(q2);
    assert(q1 == q2_save);
    assert(q2 == q1_save);
}
