//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>
// REQUIRES: c++98 || c++03 || c++11 || c++14

// template<RandomAccessIterator Iter>
//   requires ShuffleIterator<Iter>
//   void
//   random_shuffle(Iter first, Iter last);

#include <algorithm>
#include <cassert>

#include "test_macros.h"

int main()
{
    int ia[] = {1, 2, 3, 4};
    int ia1[] = {1, 4, 3, 2};
    int ia2[] = {4, 1, 2, 3};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    std::random_shuffle(ia, ia+sa);
    LIBCPP_ASSERT(std::equal(ia, ia+sa, ia1));
    assert(std::is_permutation(ia, ia+sa, ia1));
    std::random_shuffle(ia, ia+sa);
    LIBCPP_ASSERT(std::equal(ia, ia+sa, ia2));
    assert(std::is_permutation(ia, ia+sa, ia2));
}
