//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// reference back();

#include <queue>
#include <cassert>

int main()
{
    std::queue<int> q;
    assert(q.size() == 0);
    q.push(1);
    q.push(2);
    q.push(3);
    int& ir = q.back();
    assert(ir == 3);
}
