//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// explicit queue(const container_type& c);

#include <queue>
#include <cassert>
#include <cstddef>

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
    std::deque<int> d = make<std::deque<int> >(5);
    std::queue<int> q(d);
    assert(q.size() == 5);
    for (std::size_t i = 0; i < d.size(); ++i)
    {
        assert(q.front() == d[i]);
        q.pop();
    }
}
