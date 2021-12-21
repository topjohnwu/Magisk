//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, StrictWeakOrder<auto, Iter::value_type> Compare>
//   requires CopyConstructible<Compare>
//   Iter
//   is_sorted_until(Iter first, Iter last, Compare comp);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 0, 1};
    int ib[] = {1, 1, 0};
    return    (std::is_sorted_until(std::begin(ia), std::end(ia), std::greater<int>()) == ia+2)
           && (std::is_sorted_until(std::begin(ib), std::end(ib), std::greater<int>()) == ib+3);
    }
#endif

template <class Iter>
void
test()
{
    {
    int a[] = {0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a), std::greater<int>()) == Iter(a));
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }

    {
    int a[] = {0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }

    {
    int a[] = {0, 0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {0, 0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+2));
    }
    {
    int a[] = {0, 1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {0, 1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {1, 0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {1, 0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+2));
    }
    {
    int a[] = {1, 1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {1, 1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }

    {
    int a[] = {0, 0, 0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {0, 0, 0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+3));
    }
    {
    int a[] = {0, 0, 1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+2));
    }
    {
    int a[] = {0, 0, 1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+2));
    }
    {
    int a[] = {0, 1, 0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {0, 1, 0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {0, 1, 1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {0, 1, 1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+1));
    }
    {
    int a[] = {1, 0, 0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {1, 0, 0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+3));
    }
    {
    int a[] = {1, 0, 1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+2));
    }
    {
    int a[] = {1, 0, 1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+2));
    }
    {
    int a[] = {1, 1, 0, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {1, 1, 0, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+3));
    }
    {
    int a[] = {1, 1, 1, 0};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
    {
    int a[] = {1, 1, 1, 1};
    unsigned sa = sizeof(a) / sizeof(a[0]);
    assert(std::is_sorted_until(Iter(a), Iter(a+sa), std::greater<int>()) == Iter(a+sa));
    }
}

int main()
{
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
