//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>

// bool empty() const;

#include <stack>
#include <cassert>

int main()
{
    std::stack<int> q;
    assert(q.empty());
    q.push(1);
    assert(!q.empty());
    q.pop();
    assert(q.empty());
}
