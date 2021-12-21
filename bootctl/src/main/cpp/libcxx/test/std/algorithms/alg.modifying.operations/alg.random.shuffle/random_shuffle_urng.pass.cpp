//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<class RandomAccessIterator, class UniformRandomNumberGenerator>
//     void shuffle(RandomAccessIterator first, RandomAccessIterator last,
//                  UniformRandomNumberGenerator& g);

#include <algorithm>
#include <random>
#include <cassert>

#include "test_macros.h"

int main()
{
    int ia[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int ia1[] = {2, 7, 1, 4, 3, 6, 5, 10, 9, 8};
    int ia2[] = {1, 8, 3, 4, 6, 9, 5, 7, 2, 10};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    std::minstd_rand g;
    std::shuffle(ia, ia+sa, g);
    LIBCPP_ASSERT(std::equal(ia, ia+sa, ia1));
    assert(std::is_permutation(ia, ia+sa, ia1));
    std::shuffle(ia, ia+sa, std::move(g));
    LIBCPP_ASSERT(std::equal(ia, ia+sa, ia2));
    assert(std::is_permutation(ia, ia+sa, ia2));
}
