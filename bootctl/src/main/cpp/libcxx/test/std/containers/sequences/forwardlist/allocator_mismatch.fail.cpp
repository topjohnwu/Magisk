//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>
//   The container's value type must be the same as the allocator's value type

#include <forward_list>

int main()
{
    std::forward_list<int, std::allocator<long> > fl;
}
