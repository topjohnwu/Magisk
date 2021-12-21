//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>
//   The container's value type must be the same as the allocator's value type

#include <string>

int main()
{
    std::basic_string<char, std::char_traits<char>, std::allocator<int> > s;
}
