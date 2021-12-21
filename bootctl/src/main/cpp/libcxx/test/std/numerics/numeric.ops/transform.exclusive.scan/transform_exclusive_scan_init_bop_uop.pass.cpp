//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>
// UNSUPPORTED: c++98, c++03, c++11, c++14

// template<class InputIterator, class OutputIterator, class T,
//          class BinaryOperation, class UnaryOperation>
//   OutputIterator transform_exclusive_scan(InputIterator first, InputIterator last,
//                                           OutputIterator result, T init,
//                                           BinaryOperation binary_op,
//                                           UnaryOperation unary_op);


#include <numeric>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <vector>

#include "test_iterators.h"

struct add_one {
    template <typename T>
    constexpr auto operator()(T x) const noexcept {
        return static_cast<T>(x + 1);
    }
};

template <class Iter1, class BOp, class UOp, class T, class Iter2>
void
test(Iter1 first, Iter1 last, BOp bop, UOp uop, T init, Iter2 rFirst, Iter2 rLast)
{
    std::vector<typename std::iterator_traits<Iter1>::value_type> v;
//  Test not in-place
    std::transform_exclusive_scan(first, last, std::back_inserter(v), init, bop, uop);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));

//  Test in-place
    v.clear();
    v.assign(first, last);
    std::transform_exclusive_scan(v.begin(), v.end(), v.begin(), init, bop, uop);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));
}


template <class Iter>
void
test()
{
          int ia[]     = { 1,  3,  5,    7,   9 };
    const int pResI0[] = { 0,  2,  6,   12,  20 };        // with add_one
    const int mResI0[] = { 0,  0,  0,    0,   0 };
    const int pResN0[] = { 0, -1, -4,   -9, -16 };        // with negate
    const int mResN0[] = { 0,  0,  0,    0,   0 };
    const int pResI2[] = { 2,  4,  8,   14,  22 };        // with add_one
    const int mResI2[] = { 2,  4, 16,   96, 768 };
    const int pResN2[] = { 2,  1,  -2,  -7, -14 };        // with negate
    const int mResN2[] = { 2, -2,   6, -30, 210 };
    const unsigned sa = sizeof(ia) / sizeof(ia[0]);
    static_assert(sa == sizeof(pResI0) / sizeof(pResI0[0]));       // just to be sure
    static_assert(sa == sizeof(mResI0) / sizeof(mResI0[0]));       // just to be sure
    static_assert(sa == sizeof(pResN0) / sizeof(pResN0[0]));       // just to be sure
    static_assert(sa == sizeof(mResN0) / sizeof(mResN0[0]));       // just to be sure
    static_assert(sa == sizeof(pResI2) / sizeof(pResI2[0]));       // just to be sure
    static_assert(sa == sizeof(mResI2) / sizeof(mResI2[0]));       // just to be sure
    static_assert(sa == sizeof(pResN2) / sizeof(pResN2[0]));       // just to be sure
    static_assert(sa == sizeof(mResN2) / sizeof(mResN2[0]));       // just to be sure

    for (unsigned int i = 0; i < sa; ++i ) {
        test(Iter(ia), Iter(ia + i), std::plus<>(),       add_one{},       0, pResI0, pResI0 + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), add_one{},       0, mResI0, mResI0 + i);
        test(Iter(ia), Iter(ia + i), std::plus<>(),       std::negate<>(), 0, pResN0, pResN0 + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), std::negate<>(), 0, mResN0, mResN0 + i);
        test(Iter(ia), Iter(ia + i), std::plus<>(),       add_one{},       2, pResI2, pResI2 + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), add_one{},       2, mResI2, mResI2 + i);
        test(Iter(ia), Iter(ia + i), std::plus<>(),       std::negate<>(), 2, pResN2, pResN2 + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), std::negate<>(), 2, mResN2, mResN2 + i);
        }
}

size_t triangle(size_t n) { return n*(n+1)/2; }

//  Basic sanity
void basic_tests()
{
    {
    std::vector<size_t> v(10);
    std::fill(v.begin(), v.end(), 3);
    std::transform_exclusive_scan(v.begin(), v.end(), v.begin(), size_t{50}, std::plus<>(), add_one{});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == 50 + i * 4);
    }

    {
    std::vector<size_t> v(10);
    std::iota(v.begin(), v.end(), 0);
    std::transform_exclusive_scan(v.begin(), v.end(), v.begin(), size_t{30}, std::plus<>(), add_one{});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == 30 + triangle(i - 1) + i);
    }

    {
    std::vector<size_t> v(10);
    std::iota(v.begin(), v.end(), 1);
    std::transform_exclusive_scan(v.begin(), v.end(), v.begin(), size_t{40}, std::plus<>(), add_one{});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == 40 + triangle(i) + i);
    }

    {
    std::vector<size_t> v, res;
    std::transform_exclusive_scan(v.begin(), v.end(), std::back_inserter(res), size_t{40}, std::plus<>(), add_one{});
    assert(res.empty());
    }

//  Make sure that the calculations are done using the init typedef
    {
    std::vector<unsigned char> v(10);
    std::iota(v.begin(), v.end(), static_cast<unsigned char>(1));
    std::vector<size_t> res;
    std::transform_exclusive_scan(v.begin(), v.end(), std::back_inserter(res), size_t{1}, std::multiplies<>(), add_one{});

    assert(res.size() == 10);
    size_t j = 1;
    assert(res[0] == 1);
    for (size_t i = 1; i < res.size(); ++i)
    {
        j *= i + 1;
        assert(res[i] == j);
    }
    }
}

int main()
{
    basic_tests();

//  All the iterator categories
    test<input_iterator        <const int*> >();
    test<forward_iterator      <const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
    test<      int*>();
}
