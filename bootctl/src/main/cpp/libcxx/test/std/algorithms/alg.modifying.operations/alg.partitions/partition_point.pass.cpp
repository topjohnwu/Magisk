//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<class ForwardIterator, class Predicate>
//     constpexr ForwardIterator       // constexpr after C++17
//     partition_point(ForwardIterator first, ForwardIterator last, Predicate pred);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct is_odd
{
    TEST_CONSTEXPR bool operator()(const int& i) const {return i & 1;}
};


#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 4, 6};
    int ib[] = {1, 2, 3, 4, 5, 6};
    return    (std::partition_point(std::begin(ia), std::end(ia), is_odd()) == ia+3)
           && (std::partition_point(std::begin(ib), std::end(ib), is_odd()) == ib+1)
           ;
    }
#endif


int main()
{
    {
        const int ia[] = {2, 4, 6, 8, 10};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia));
    }
    {
        const int ia[] = {1, 2, 4, 6, 8};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia + 1));
    }
    {
        const int ia[] = {1, 3, 2, 4, 6};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia + 2));
    }
    {
        const int ia[] = {1, 3, 5, 2, 4, 6};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia + 3));
    }
    {
        const int ia[] = {1, 3, 5, 7, 2, 4};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia + 4));
    }
    {
        const int ia[] = {1, 3, 5, 7, 9, 2};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia + 5));
    }
    {
        const int ia[] = {1, 3, 5, 7, 9, 11};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::end(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia + 6));
    }
    {
        const int ia[] = {1, 3, 5, 2, 4, 6, 7};
        assert(std::partition_point(forward_iterator<const int*>(std::begin(ia)),
                                    forward_iterator<const int*>(std::begin(ia)),
                                    is_odd()) == forward_iterator<const int*>(ia));
    }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
