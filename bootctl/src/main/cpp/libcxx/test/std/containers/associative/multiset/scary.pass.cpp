//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class set class multiset

// Extension:  SCARY/N2913 iterator compatibility between set and multiset

#include <set>

int main()
{
    typedef std::set<int> M1;
    typedef std::multiset<int> M2;
    M2::iterator i;
    M1::iterator j = i;
    ((void)j);
}
