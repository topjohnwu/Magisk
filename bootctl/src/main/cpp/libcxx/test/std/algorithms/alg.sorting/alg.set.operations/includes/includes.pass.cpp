//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator Iter1, InputIterator Iter2>
//   requires HasLess<Iter1::value_type, Iter2::value_type>
//         && HasLess<Iter2::value_type, Iter1::value_type>
//   constexpr bool             // constexpr after C++17
//   includes(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
    int ib[] = {2, 4};
    int ic[] = {3, 3, 3, 3};

    return  std::includes(std::begin(ia), std::end(ia), std::begin(ib), std::end(ib))
        && !std::includes(std::begin(ia), std::end(ia), std::begin(ic), std::end(ic))
           ;
    }
#endif

template <class Iter1, class Iter2>
void
test()
{
    int ia[] = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ib[] = {2, 4};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int ic[] = {1, 2};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]); ((void)sc);
    int id[] = {3, 3, 3, 3};
    const unsigned sd = sizeof(id)/sizeof(id[0]); ((void)sd);

    assert(std::includes(Iter1(ia), Iter1(ia), Iter2(ib), Iter2(ib)));
    assert(!std::includes(Iter1(ia), Iter1(ia), Iter2(ib), Iter2(ib+1)));
    assert(std::includes(Iter1(ia), Iter1(ia+1), Iter2(ib), Iter2(ib)));
    assert(std::includes(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia+sa)));

    assert(std::includes(Iter1(ia), Iter1(ia+sa), Iter2(ib), Iter2(ib+sb)));
    assert(!std::includes(Iter1(ib), Iter1(ib+sb), Iter2(ia), Iter2(ia+sa)));

    assert(std::includes(Iter1(ia), Iter1(ia+2), Iter2(ic), Iter2(ic+2)));
    assert(!std::includes(Iter1(ia), Iter1(ia+2), Iter2(ib), Iter2(ib+2)));

    assert(std::includes(Iter1(ia), Iter1(ia+sa), Iter2(id), Iter2(id+1)));
    assert(std::includes(Iter1(ia), Iter1(ia+sa), Iter2(id), Iter2(id+2)));
    assert(std::includes(Iter1(ia), Iter1(ia+sa), Iter2(id), Iter2(id+3)));
    assert(!std::includes(Iter1(ia), Iter1(ia+sa), Iter2(id), Iter2(id+4)));
}

int main()
{
    test<input_iterator<const int*>, input_iterator<const int*> >();
    test<input_iterator<const int*>, forward_iterator<const int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<input_iterator<const int*>, random_access_iterator<const int*> >();
    test<input_iterator<const int*>, const int*>();

    test<forward_iterator<const int*>, input_iterator<const int*> >();
    test<forward_iterator<const int*>, forward_iterator<const int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<forward_iterator<const int*>, random_access_iterator<const int*> >();
    test<forward_iterator<const int*>, const int*>();

    test<bidirectional_iterator<const int*>, input_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, const int*>();

    test<random_access_iterator<const int*>, input_iterator<const int*> >();
    test<random_access_iterator<const int*>, forward_iterator<const int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<const int*> >();
    test<random_access_iterator<const int*>, const int*>();

    test<const int*, input_iterator<const int*> >();
    test<const int*, forward_iterator<const int*> >();
    test<const int*, bidirectional_iterator<const int*> >();
    test<const int*, random_access_iterator<const int*> >();
    test<const int*, const int*>();

#if TEST_STD_VER > 17
   static_assert(test_constexpr());
#endif
}
