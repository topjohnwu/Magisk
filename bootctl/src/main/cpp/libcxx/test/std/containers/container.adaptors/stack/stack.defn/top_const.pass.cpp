//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>

// const_reference top() const;

#include <stack>
#include <cassert>

int main()
{
    std::stack<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    const std::stack<int>& cqr = q;
    const int& cir = cqr.top();
    assert(cir == 3);
}
