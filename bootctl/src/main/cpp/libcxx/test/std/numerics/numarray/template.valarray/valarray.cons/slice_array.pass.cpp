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

// valarray(const slice_array<value_type>& sa);

#include <valarray>
#include <cassert>

int main()
{
    int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    std::valarray<int> v1(a, sizeof(a)/sizeof(a[0]));
    std::valarray<int> v(v1[std::slice(1, 5, 3)]);
    assert(v.size() == 5);
    assert(v[0] == 1);
    assert(v[1] == 4);
    assert(v[2] == 7);
    assert(v[3] == 10);
    assert(v[4] == 13);
}
