//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>
//   The container's value type must be the same as the allocator's value type

#include <set>

int main()
{
    std::set<int, std::less<int>, std::allocator<long> > s;
}
