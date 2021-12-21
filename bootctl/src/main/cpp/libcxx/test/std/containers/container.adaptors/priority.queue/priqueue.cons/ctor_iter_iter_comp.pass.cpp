//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// template <class InputIterator>
//   priority_queue(InputIterator first, InputIterator last, const Compare& comp);

#include <queue>
#include <cassert>
#include <functional>
#include <cstddef>

int main()
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    int* an = a + sizeof(a)/sizeof(a[0]);
    std::priority_queue<int, std::vector<int>, std::greater<int> >
        q(a, an, std::greater<int>());
    assert(q.size() == static_cast<std::size_t>(an - a));
    assert(q.top() == 0);
}
