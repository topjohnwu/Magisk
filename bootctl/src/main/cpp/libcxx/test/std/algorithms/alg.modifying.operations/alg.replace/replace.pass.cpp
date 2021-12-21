//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, class T>
//   requires OutputIterator<Iter, Iter::reference>
//         && OutputIterator<Iter, const T&>
//         && HasEqualTo<Iter::value_type, T>
//   constexpr void      // constexpr after C++17
//   replace(Iter first, Iter last, const T& old_value, const T& new_value);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"


#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
          int ia[]       = {0, 1, 2, 3, 4};
    const int expected[] = {0, 1, 5, 3, 4};

    std::replace(std::begin(ia), std::end(ia), 2, 5);
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
    std::replace(Iter(ia), Iter(ia+sa), 2, 5);
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
