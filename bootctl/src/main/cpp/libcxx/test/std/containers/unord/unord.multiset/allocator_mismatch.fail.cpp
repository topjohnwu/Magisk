//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>
//   The container's value type must be the same as the allocator's value type

#include <unordered_set>

int main()
{
    std::unordered_multiset<int, std::hash<int>, std::less<int>, std::allocator<long> > v;
}
