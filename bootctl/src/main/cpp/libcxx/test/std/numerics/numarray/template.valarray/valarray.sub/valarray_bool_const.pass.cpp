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

// valarray operator[](const valarray<bool>& vb) const;

#include <valarray>
#include <cassert>

int main()
{
    int a1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    const std::size_t N1 = sizeof(a1)/sizeof(a1[0]);
    bool b[N1] = {true,  false, false, true,  true,  false,
                  false, true,  false, false, false, true};
    std::valarray<int> v1(a1, N1);
    std::valarray<bool> vb(b, N1);
    std::valarray<int> v2(v1[vb]);
    assert(v2.size() == 5);
    assert(v2[ 0] ==  0);
    assert(v2[ 1] ==  3);
    assert(v2[ 2] ==  4);
    assert(v2[ 3] ==  7);
    assert(v2[ 4] == 11);
}
