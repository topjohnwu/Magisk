//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// class unordered_set class unordered_multiset

// Extension:  SCARY/N2913 iterator compatibility between unordered_set and unordered_multiset

#include <unordered_set>

int main()
{
    typedef std::unordered_set<int> M1;
    typedef std::unordered_multiset<int> M2;
    M2::iterator i;
    M1::iterator j = i;
    ((void)j);
}
