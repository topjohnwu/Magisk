//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// map();

#include <map>

struct X
{
    std::map<int, X> m;
    std::map<int, X>::iterator i;
    std::map<int, X>::const_iterator ci;
    std::map<int, X>::reverse_iterator ri;
    std::map<int, X>::const_reverse_iterator cri;
};

int main()
{
}
