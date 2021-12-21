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

// template<class InputIterator, class OutputIterator, class T, class BinaryOperation>
//     OutputIterator
//     inclusive_scan(InputIterator first, InputIterator last,
//                    OutputIterator result,
//                    BinaryOperation binary_op, T init); // C++17

#include <numeric>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <vector>

#include "test_iterators.h"

template <class Iter1, class T, class Op, class Iter2>
void
test(Iter1 first, Iter1 last, Op op, T init, Iter2 rFirst, Iter2 rLast)
{
    std::vector<typename std::iterator_traits<Iter1>::value_type> v;

//  Not in place
    std::inclusive_scan(first, last, std::back_inserter(v), op, init);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));

//  In place
    v.clear();
    v.assign(first, last);
    std::inclusive_scan(v.begin(), v.end(), v.begin(), op, init);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));
}


template <class Iter>
void
test()
{
          int ia[]   = {1, 3,  5,   7,   9};
    const int pRes[] = {1, 4,  9,  16,  25};
    const int mRes[] = {1, 3, 15, 105, 945};
    const unsigned sa = sizeof(ia) / sizeof(ia[0]);
    static_assert(sa == sizeof(pRes) / sizeof(pRes[0]));       // just to be sure
    static_assert(sa == sizeof(mRes) / sizeof(mRes[0]));       // just to be sure

    for (unsigned int i = 0; i < sa; ++i ) {
        test(Iter(ia), Iter(ia + i), std::plus<>(),       0, pRes, pRes + i);
        test(Iter(ia), Iter(ia + i), std::multiplies<>(), 1, mRes, mRes + i);
        }
}

size_t triangle(size_t n) { return n*(n+1)/2; }

//  Basic sanity
void basic_tests()
{
    {
    std::vector<size_t> v(10);
    std::fill(v.begin(), v.end(), 3);
    std::inclusive_scan(v.begin(), v.end(), v.begin(), std::plus<>(), size_t{50});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == 50 + (i+1) * 3);
    }

    {
    std::vector<size_t> v(10);
    std::iota(v.begin(), v.end(), 0);
    std::inclusive_scan(v.begin(), v.end(), v.begin(), std::plus<>(), size_t{40});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == 40 + triangle(i));
    }

    {
    std::vector<size_t> v(10);
    std::iota(v.begin(), v.end(), 1);
    std::inclusive_scan(v.begin(), v.end(), v.begin(), std::plus<>(), size_t{30});
    for (size_t i = 0; i < v.size(); ++i)
        assert(v[i] == 30 + triangle(i + 1));
    }

    {
    std::vector<size_t> v, res;
    std::inclusive_scan(v.begin(), v.end(), std::back_inserter(res), std::plus<>(), size_t{40});
    assert(res.empty());
    }

//  Make sure that the calculations are done using the init typedef
    {
    std::vector<unsigned char> v(10);
    std::iota(v.begin(), v.end(), static_cast<unsigned char>(1));
    std::vector<size_t> res;
    std::inclusive_scan(v.begin(), v.end(), std::back_inserter(res), std::multiplies<>(), size_t{1});

    assert(res.size() == 10);
    size_t j = 1;
    assert(res[0] == 1);
    for (size_t i = 1; i < v.size(); ++i)
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
