//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// class unordered_map class unordered_multimap

// Extension:  SCARY/N2913 iterator compatibility between unordered_map and unordered_multimap

#include <unordered_map>

int main()
{
    typedef std::unordered_map<int, int> M1;
    typedef std::unordered_multimap<int, int> M2;
    M2::iterator i;
    M1::iterator j = i;
    ((void)j);
}
