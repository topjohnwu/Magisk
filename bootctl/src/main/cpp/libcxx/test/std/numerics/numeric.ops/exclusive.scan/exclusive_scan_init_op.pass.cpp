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
//     exclusive_scan(InputIterator first, InputIterator last,
//                    OutputIterator result,
//                    T init, BinaryOperation binary_op); // C++17

#include <numeric>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <vector>

#include "test_iterators.h"

template <class Iter1, class T, class Op, class Iter2>
void
test(Iter1 first, Iter1 last, T init, Op op, Iter2 rFirst, Iter2 rLast)
{
    std::vector<typename std::iterator_traits<Iter1>::value_type> v;

//  Not in place
    std::exclusive_scan(first, last, std::back_inserter(v), init, op);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));

//  In place
    v.clear();
    v.assign(first, last);
    std::exclusive_scan(v.begin(), v.end(), v.begin(), init, op);
    assert(std::equal(v.begin(), v.end(), rFirst, rLast));
}


template <class Iter>
void
test()
{
          int ia[]   = {1, 3, 5,  7,   9};
    const int pRes[] = {0, 1, 4,  9,  16};
    const int mRes[] = {1, 1, 3, 15, 105};
    const unsigned sa = sizeof(ia) / sizeof(ia[0]);
    static_assert(sa == sizeof(pRes) / sizeof(pRes[0]));       // just to be sure
    static_assert(sa == sizeof(mRes) / sizeof(mRes[0]));       // just to be sure

    for (unsigned int i = 0; i < sa; ++i ) {
        test(Iter(ia), Iter(ia + i), 0, std::plus<>(),       pRes, pRes + i);
        test(Iter(ia), Iter(ia + i), 1, std::multiplies<>(), mRes, mRes + i);
        }
}

int main()
{
//  All the iterator categories
    test<input_iterator        <const int*> >();
    test<forward_iterator      <const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
    test<      int*>();

//  Make sure that the calculations are done using the init typedef
    {
    std::vector<unsigned char> v(10);
    std::iota(v.begin(), v.end(), static_cast<unsigned char>(1));
    std::vector<size_t> res;
    std::exclusive_scan(v.begin(), v.end(), std::back_inserter(res), 1, std::multiplies<>());

    assert(res.size() == 10);
    size_t j = 1;
    assert(res[0] == 1);
    for (size_t i = 1; i < v.size(); ++i)
    {
        j *= i;
        assert(res[i] == j);
    }
    }
}
