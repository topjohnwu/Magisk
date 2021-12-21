//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// pop_back() more than the number of elements in a deque

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <cstdlib>
#include <deque>


int main() {
    std::deque<int> q;
    q.push_back(0);
    q.pop_back();
    q.pop_back();
    std::exit(1);
}
