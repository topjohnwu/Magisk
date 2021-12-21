
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
//   OutputIterator transform_inclusive_scan(InputIterator first, InputIterator last,
//                                           OutputIterator result,
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

template <class Iter1, class BOp, class UOp, class Iter2>
void
test(Iter1 first, Iter1 last, BOp bop, UOp uop, Iter2 rFirst, Iter2 rLast)
{
    std::vector<typename std::iterator_traits<Iter1>::value_type> v;
//  Test not in-place
    std::transform_inclusive_scan(first, last, std::back_inserter(v), bop, uop);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));

//  Test in-place
    v.clear();
    v.assign(first, last);
    std::transform_inclusive_scan(v.begin(), v.end(), v.begin(), bop, uop);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));
}


template <class Iter>
void
test()
{
          int ia[]     = {  1,  3,   5,   7,    9 };
    const int pResI0[] = {  2,  6,  12,  20,   30 };        // with add_one
    const int mResI0[] = {  2,  8, 48,  384, 3840 };
    const int pResN0[] = { -1, -4,  -9, -16,  -25 };        // with negate
    const int mResN0[] = { -1,  3, -15, 105, -945 };
    const unsigned sa = sizeof(ia) / sizeof(ia[0] );
    static_assert(sa == sizeof(pResI0) / sizeof(pResI0[0]));       // just to be sure
    static_assert(sa == sizeof(mResI0) / sizeof(mResI0[0]));       // just to be sure
    static_assert(sa == sizeof(pResN0) / sizeof(pResN0[0]));       // just to be sure
    static_assert(sa == sizeof(mResN0) / sizeof(mResN0[0]));       // just to be sure

    for (unsigned int i = 0; i < sa; ++i ) {
        test(Iter(ia), Iter(ia + i), std::plus<>(),       add_one{},       pResI0, pResI0 + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), add_one{},       mResI0, mResI0 + i);
        test(Iter(ia), Iter(ia + i), std::plus<>(),       std::negate<>(), pResN0, pResN0 + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), std::negate<>(), mResN0, mResN0 + i);
        }
}

size_t triangle(size_t n) { return n*(n+1)/2; }

//  Basic sanity
void basic_tests()
{
    {
    std::vector<size_t> v(10);
    std::fill(v.begin(), v.end(), 3);
    std::transform_inclusive_scan(v.begin(), v.end(), v.begin(), std::plus<>(), add_one{});
    std::copy(v.begin(), v.end(), std::ostream_iterator<size_t>(std::cout, " "));
    std::cout << std::endl;
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == (i+1) * 4);
    }

    {
    std::vector<size_t> v(10);
    std::iota(v.begin(), v.end(), 0);
    std::transform_inclusive_scan(v.begin(), v.end(), v.begin(), std::plus<>(), add_one{});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == triangle(i) + i + 1);
    }

    {
    std::vector<size_t> v(10);
    std::iota(v.begin(), v.end(), 1);
    std::transform_inclusive_scan(v.begin(), v.end(), v.begin(), std::plus<>(), add_one{});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == triangle(i + 1) + i + 1);
    }

    {
    std::vector<size_t> v, res;
    std::transform_inclusive_scan(v.begin(), v.end(), std::back_inserter(res), std::plus<>(), add_one{});
    assert(res.empty());
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
