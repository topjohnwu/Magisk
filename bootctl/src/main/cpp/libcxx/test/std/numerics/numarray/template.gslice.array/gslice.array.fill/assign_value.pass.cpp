//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template <class T> class gslice_array

// void operator=(const value_type& x) const;

#include <valarray>
#include <cassert>

int main()
{
    int a1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
                12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                36, 37, 38, 39, 40};
    std::valarray<int> v1(a1, sizeof(a1)/sizeof(a1[0]));
    std::size_t sz[] = {2, 4, 3};
    std::size_t st[] = {19, 4, 1};
    typedef std::valarray<std::size_t> sizes;
    typedef std::valarray<std::size_t> strides;
    v1[std::gslice(3, sizes(sz, sizeof(sz)/sizeof(sz[0])),
                      strides(st, sizeof(st)/sizeof(st[0])))] = 51;
    assert(v1.size() == 41);
    assert(v1[ 0] ==  0);
    assert(v1[ 1] ==  1);
    assert(v1[ 2] ==  2);
    assert(v1[ 3] == 51);
    assert(v1[ 4] == 51);
    assert(v1[ 5] == 51);
    assert(v1[ 6] ==  6);
    assert(v1[ 7] == 51);
    assert(v1[ 8] == 51);
    assert(v1[ 9] == 51);
    assert(v1[10] == 10);
    assert(v1[11] == 51);
    assert(v1[12] == 51);
    assert(v1[13] == 51);
    assert(v1[14] == 14);
    assert(v1[15] == 51);
    assert(v1[16] == 51);
    assert(v1[17] == 51);
    assert(v1[18] == 18);
    assert(v1[19] == 19);
    assert(v1[20] == 20);
    assert(v1[21] == 21);
    assert(v1[22] == 51);
    assert(v1[23] == 51);
    assert(v1[24] == 51);
    assert(v1[25] == 25);
    assert(v1[26] == 51);
    assert(v1[27] == 51);
    assert(v1[28] == 51);
    assert(v1[29] == 29);
    assert(v1[30] == 51);
    assert(v1[31] == 51);
    assert(v1[32] == 51);
    assert(v1[33] == 33);
    assert(v1[34] == 51);
    assert(v1[35] == 51);
    assert(v1[36] == 51);
    assert(v1[37] == 37);
    assert(v1[38] == 38);
    assert(v1[39] == 39);
    assert(v1[40] == 40);
}
