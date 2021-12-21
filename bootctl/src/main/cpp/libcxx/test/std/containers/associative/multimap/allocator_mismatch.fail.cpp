//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>
//   The container's value type must be the same as the allocator's value type

#include <map>

int main()
{
    std::multimap<int, int, std::less<int>, std::allocator<long> > m;
}
