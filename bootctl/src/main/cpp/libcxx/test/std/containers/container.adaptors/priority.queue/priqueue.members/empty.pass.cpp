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

// bool empty() const;

#include <queue>
#include <cassert>

int main()
{
    std::priority_queue<int> q;
    assert(q.empty());
    q.push(1);
    assert(!q.empty());
    q.pop();
    assert(q.empty());
}
