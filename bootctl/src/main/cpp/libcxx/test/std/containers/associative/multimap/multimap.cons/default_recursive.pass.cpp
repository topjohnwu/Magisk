//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class multimap

// multimap();

#include <map>

struct X
{
    std::multimap<int, X> m;
    std::multimap<int, X>::iterator i;
    std::multimap<int, X>::const_iterator ci;
    std::multimap<int, X>::reverse_iterator ri;
    std::multimap<int, X>::const_reverse_iterator cri;
};

int main()
{
}
