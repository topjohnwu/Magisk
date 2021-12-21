//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template <class T> class slice_array

// const slice_array& operator=(const slice_array& sa) const;

#include <valarray>
#include <cassert>

int main()
{
    {
    int a1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int a2[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12};
    std::valarray<int> v1(a1, sizeof(a1)/sizeof(a1[0]));
    const std::valarray<int> v2(a2, sizeof(a2)/sizeof(a2[0]));
    v1[std::slice(1, 5, 3)] = v2[std::slice(2, 5, 2)];
    assert(v1.size() == 16);
    assert(v1[ 0] ==  0);
    assert(v1[ 1] == -3);
    assert(v1[ 2] ==  2);
    assert(v1[ 3] ==  3);
    assert(v1[ 4] == -5);
    assert(v1[ 5] ==  5);
    assert(v1[ 6] ==  6);
    assert(v1[ 7] == -7);
    assert(v1[ 8] ==  8);
    assert(v1[ 9] ==  9);
    assert(v1[10] == -9);
    assert(v1[11] == 11);
    assert(v1[12] == 12);
    assert(v1[13] == -11);
    assert(v1[14] == 14);
    assert(v1[15] == 15);
    }
    // Test return value of assignment.
    {
    int a1[] = {0, 1, 2};
    std::valarray<int> v1(a1, 3);
    std::slice_array<int> s1 = v1[std::slice(1, 1, 1)];
    std::slice_array<int> s2 = v1[std::slice(0, 1, 1)];
    std::slice_array<int> const & s3 = (s1 = s2);
    assert(&s1 == &s3);
    }
}
