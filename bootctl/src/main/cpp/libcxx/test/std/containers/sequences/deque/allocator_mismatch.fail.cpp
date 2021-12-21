//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>
//   The container's value type must be the same as the allocator's value type

#include <deque>

int main()
{
    std::deque<int, std::allocator<long> > d;
}
