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
//   pair<Iter, Iter>
//   minmax_element(Iter first, Iter last, Compare comp);

#include <algorithm>
#include <functional>
#include <random>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

std::mt19937 randomness;

template <class Iter>
void
test(Iter first, Iter last)
{
    typedef std::greater<int> Compare;
    Compare comp;
    std::pair<Iter, Iter> p = std::minmax_element(first, last, comp);
    if (first != last)
    {
        for (Iter j = first; j != last; ++j)
        {
            assert(!comp(*j, *p.first));
            assert(!comp(*p.second, *j));
        }
    }
    else
    {
        assert(p.first == last);
        assert(p.second == last);
    }
}

template <class Iter>
void
test(int N)
{
    int* a = new int[N];
    for (int i = 0; i < N; ++i)
        a[i] = i;
    std::shuffle(a, a+N, randomness);
    test(Iter(a), Iter(a+N));
    delete [] a;
}

template <class Iter>
void
test()
{
    test<Iter>(0);
    test<Iter>(1);
    test<Iter>(2);
    test<Iter>(3);
    test<Iter>(10);
    test<Iter>(1000);
    {
    const int N = 100;
    int* a = new int[N];
    for (int i = 0; i < N; ++i)
        a[i] = 5;
    std::shuffle(a, a+N, randomness);
    typedef std::greater<int> Compare;
    Compare comp;
    std::pair<Iter, Iter> p = std::minmax_element(Iter(a), Iter(a+N), comp);
    assert(base(p.first) == a);
    assert(base(p.second) == a+N-1);
    delete [] a;
    }
}

#if TEST_STD_VER >= 14
constexpr int il[] = { 2, 4, 6, 8, 7, 5, 3, 1 };
struct less { constexpr bool operator ()( const int &x, const int &y) const { return x < y; }};
#endif

void constexpr_test()
{
#if TEST_STD_VER >= 14
    constexpr auto p = std::minmax_element(il, il+8, less());
    static_assert ( *(p.first)  == 1, "" );
    static_assert ( *(p.second) == 8, "" );
#endif
}

int main()
{
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();

    constexpr_test();
}
