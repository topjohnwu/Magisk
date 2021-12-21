//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, class T, class Compare>
//   constexpr Iter    // constexpr after c++17
//   lower_bound(Iter first, Iter last, const T& value, Compare comp);

#include <algorithm>
#include <functional>
#include <vector>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool lt(int a, int b) { return a < b; }

TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 6, 7};

    return (std::lower_bound(std::begin(ia), std::end(ia), 2, lt) == ia+1)
        && (std::lower_bound(std::begin(ia), std::end(ia), 3, lt) == ia+1)
        && (std::lower_bound(std::begin(ia), std::end(ia), 9, lt) == std::end(ia))
        ;
    }
#endif

template <class Iter, class T>
void
test(Iter first, Iter last, const T& value)
{
    Iter i = std::lower_bound(first, last, value, std::greater<int>());
    for (Iter j = first; j != i; ++j)
        assert(std::greater<int>()(*j, value));
    for (Iter j = i; j != last; ++j)
        assert(!std::greater<int>()(*j, value));
}

template <class Iter>
void
test()
{
    const unsigned N = 1000;
    const int M = 10;
    std::vector<int> v(N);
    int x = 0;
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        v[i] = x;
        if (++x == M)
            x = 0;
    }
    std::sort(v.begin(), v.end(), std::greater<int>());
    for (x = 0; x <= M; ++x)
        test(Iter(v.data()), Iter(v.data()+v.size()), x);
}

int main()
{
    int d[] = {3, 2, 1, 0};
    for (int* e = d; e <= d+4; ++e)
        for (int x = -1; x <= 4; ++x)
            test(d, e, x);

    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
