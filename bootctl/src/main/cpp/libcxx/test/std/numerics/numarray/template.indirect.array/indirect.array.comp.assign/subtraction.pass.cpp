//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template <class T> class indirect_array

// void operator-=(const valarray<value_type>& v) const;

#include <valarray>
#include <cassert>

int main()
{
    int a1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
                12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                36, 37, 38, 39, 40};
    const std::size_t N1 = sizeof(a1)/sizeof(a1[0]);
    int a2[] = {  1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11, 12,
                 13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23, 24};
    const std::size_t N2 = sizeof(a2)/sizeof(a2[0]);
    std::size_t s[N2] = { 3,  4,  5,  7,  8,  9, 11, 12, 13, 15, 16, 17,
                         22, 23, 24, 26, 27, 28, 30, 31, 32, 34, 35, 36};
    std::valarray<int> v1(a1, N1);
    std::valarray<int> v2(a2, N2);
    std::valarray<std::size_t> ia(s, N2);
    v1[ia] -= v2;
    assert(v1.size() == 41);
    assert(v1[ 0] ==  0);
    assert(v1[ 1] ==  1);
    assert(v1[ 2] ==  2);
    assert(v1[ 3] ==  2);
    assert(v1[ 4] ==  2);
    assert(v1[ 5] ==  2);
    assert(v1[ 6] ==  6);
    assert(v1[ 7] ==  3);
    assert(v1[ 8] ==  3);
    assert(v1[ 9] ==  3);
    assert(v1[10] == 10);
    assert(v1[11] ==  4);
    assert(v1[12] ==  4);
    assert(v1[13] ==  4);
    assert(v1[14] == 14);
    assert(v1[15] ==  5);
    assert(v1[16] ==  5);
    assert(v1[17] ==  5);
    assert(v1[18] == 18);
    assert(v1[19] == 19);
    assert(v1[20] == 20);
    assert(v1[21] == 21);
    assert(v1[22] ==  9);
    assert(v1[23] ==  9);
    assert(v1[24] ==  9);
    assert(v1[25] == 25);
    assert(v1[26] == 10);
    assert(v1[27] == 10);
    assert(v1[28] == 10);
    assert(v1[29] == 29);
    assert(v1[30] == 11);
    assert(v1[31] == 11);
    assert(v1[32] == 11);
    assert(v1[33] == 33);
    assert(v1[34] == 12);
    assert(v1[35] == 12);
    assert(v1[36] == 12);
    assert(v1[37] == 37);
    assert(v1[38] == 38);
    assert(v1[39] == 39);
    assert(v1[40] == 40);
}
