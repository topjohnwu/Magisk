//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>
//   The container's value type must be the same as the allocator's value type

#include <unordered_map>

int main()
{
    std::unordered_multimap<int, int, std::hash<int>, std::less<int>, std::allocator<long> > m;
}
