//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// priority_queue& operator=(const priority_queue&) = default;

#include <queue>
#include <cassert>
#include <functional>

template <class C>
C
make(int n)
{
    C c;
    for (int i = 0; i < n; ++i)
        c.push_back(i);
    return c;
}

int main()
{
    std::vector<int> v = make<std::vector<int> >(5);
    std::priority_queue<int, std::vector<int>, std::greater<int> > qo(std::greater<int>(), v);
    std::priority_queue<int, std::vector<int>, std::greater<int> > q;
    q = qo;
    assert(q.size() == 5);
    assert(q.top() == 0);
}
