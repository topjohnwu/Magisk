//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template <class InputIterator, class Predicate>
//     constpexr bool       // constexpr after C++17
//   any_of(InputIterator first, InputIterator last, Predicate pred);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct test1
{
    TEST_CONSTEXPR bool operator()(const int& i) const
    {
        return i % 2 == 0;
    }
};

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {2, 4, 6, 8};
    int ib[] = {1, 3, 5, 7};
    return  std::any_of(std::begin(ia), std::end(ia), test1())
        && !std::any_of(std::begin(ib), std::end(ib), test1())
        ;
    }
#endif

int main()
{
    {
        int ia[] = {2, 4, 6, 8};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::any_of(input_iterator<const int*>(ia),
                           input_iterator<const int*>(ia + sa), test1()) == true);
        assert(std::any_of(input_iterator<const int*>(ia),
                           input_iterator<const int*>(ia), test1()) == false);
    }
    {
        const int ia[] = {2, 4, 5, 8};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::any_of(input_iterator<const int*>(ia),
                           input_iterator<const int*>(ia + sa), test1()) == true);
        assert(std::any_of(input_iterator<const int*>(ia),
                           input_iterator<const int*>(ia), test1()) == false);
    }
    {
        const int ia[] = {1, 3, 5, 7};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::any_of(input_iterator<const int*>(ia),
                           input_iterator<const int*>(ia + sa), test1()) == false);
        assert(std::any_of(input_iterator<const int*>(ia),
                           input_iterator<const int*>(ia), test1()) == false);
    }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
