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
//   priority_queue(InputIterator first, InputIterator last);

#include <queue>
#include <cassert>
#include <cstddef>

int main()
{
    int a[] = {3, 5, 2, 0, 6, 8, 1};
    int* an = a + sizeof(a)/sizeof(a[0]);
    std::priority_queue<int> q(a, an);
    assert(q.size() == static_cast<std::size_t>(an - a));
    assert(q.top() == 8);
}
