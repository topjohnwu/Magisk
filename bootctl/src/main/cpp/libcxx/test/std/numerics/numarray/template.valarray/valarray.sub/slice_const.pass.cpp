//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template<class T> class valarray;

// valarray operator[](slice s) const;

#include <valarray>
#include <cassert>

int main()
{
    int a1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    std::valarray<int> v1(a1, sizeof(a1)/sizeof(a1[0]));
    std::valarray<int> v2 = v1[std::slice(1, 5, 3)];
    assert(v2.size() == 5);
    assert(v2[0] ==  1);
    assert(v2[1] ==  4);
    assert(v2[2] ==  7);
    assert(v2[3] == 10);
    assert(v2[4] == 13);
}
