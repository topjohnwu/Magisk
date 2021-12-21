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

// const indirect_array& operator=(const indirect_array& ia) const;

#include <valarray>
#include <cassert>

int main()
{
    int a1[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
                12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                36, 37, 38, 39, 40};
    const std::size_t N1 = sizeof(a1)/sizeof(a1[0]);
    int a2[] = { -0,  -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11,
                -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23,
                -24, -25, -26, -27, -28, -29, -30, -31, -32, -33, -34, -35,
                -36, -37};
    const std::size_t N2 = sizeof(a2)/sizeof(a2[0]);
    std::size_t s[] = { 3,  4,  5,  7,  8,  9, 11, 12, 13, 15, 16, 17,
                       22, 23, 24, 26, 27, 28, 30, 31, 32, 34, 35, 36};
    const std::size_t S = sizeof(s)/sizeof(s[0]);
    std::valarray<int> v1(a1, N1);
    const std::valarray<int> v2(a2, N2);
    std::valarray<std::size_t> ia(s, S);
    v1[ia] = v2[ia];
    assert(v1.size() == 41);
    assert(v1[ 0] ==  0);
    assert(v1[ 1] ==  1);
    assert(v1[ 2] ==  2);
    assert(v1[ 3] == -3);
    assert(v1[ 4] == -4);
    assert(v1[ 5] == -5);
    assert(v1[ 6] ==  6);
    assert(v1[ 7] == -7);
    assert(v1[ 8] == -8);
    assert(v1[ 9] == -9);
    assert(v1[10] == 10);
    assert(v1[11] == -11);
    assert(v1[12] == -12);
    assert(v1[13] == -13);
    assert(v1[14] == 14);
    assert(v1[15] == -15);
    assert(v1[16] == -16);
    assert(v1[17] == -17);
    assert(v1[18] == 18);
    assert(v1[19] == 19);
    assert(v1[20] == 20);
    assert(v1[21] == 21);
    assert(v1[22] == -22);
    assert(v1[23] == -23);
    assert(v1[24] == -24);
    assert(v1[25] == 25);
    assert(v1[26] == -26);
    assert(v1[27] == -27);
    assert(v1[28] == -28);
    assert(v1[29] == 29);
    assert(v1[30] == -30);
    assert(v1[31] == -31);
    assert(v1[32] == -32);
    assert(v1[33] == 33);
    assert(v1[34] == -34);
    assert(v1[35] == -35);
    assert(v1[36] == -36);
    assert(v1[37] == 37);
    assert(v1[38] == 38);
    assert(v1[39] == 39);
    assert(v1[40] == 40);
}
