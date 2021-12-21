//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, Predicate<auto, Iter::value_type> Pred, class T>
//   requires OutputIterator<Iter, Iter::reference>
//         && OutputIterator<Iter, const T&>
//         && CopyConstructible<Pred>
//   constexpr void      // constexpr after C++17
//   replace_if(Iter first, Iter last, Pred pred, const T& new_value);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

TEST_CONSTEXPR bool equalToTwo(int v) { return v == 2; }

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
          int ia[]       = {0, 1, 2, 3, 4};
    const int expected[] = {0, 1, 5, 3, 4};

    std::replace_if(std::begin(ia), std::end(ia), equalToTwo, 5);
    return std::equal(std::begin(ia), std::end(ia), std::begin(expected), std::end(expected))
        ;
    }
#endif


template <class Iter>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    std::replace_if(Iter(ia), Iter(ia+sa), equalToTwo, 5);
    assert(ia[0] == 0);
    assert(ia[1] == 1);
    assert(ia[2] == 5);
    assert(ia[3] == 3);
    assert(ia[4] == 4);
}

int main()
{
    test<forward_iterator<int*> >();
    test<bidirectional_iterator<int*> >();
    test<random_access_iterator<int*> >();
    test<int*>();

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
