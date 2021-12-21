//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter1, ForwardIterator Iter2>
//   requires HasEqualTo<Iter1::value_type, Iter2::value_type>
//   constexpr Iter1  // constexpr after C++17
//   find_end(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {0, 1, 2};
    int ib[] = {4, 5, 6};
    int ic[] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 0, 1, 2, 3, 0, 1, 2, 0, 1, 0};
    typedef forward_iterator<int*>       FI;
    typedef bidirectional_iterator<int*> BI;
    typedef random_access_iterator<int*> RI;

    return    (std::find_end(FI(std::begin(ic)), FI(std::end(ic)), FI(std::begin(ia)), FI(std::end(ia))) == FI(ic+15))
           && (std::find_end(FI(std::begin(ic)), FI(std::end(ic)), FI(std::begin(ib)), FI(std::end(ib))) == FI(std::end(ic)))
           && (std::find_end(BI(std::begin(ic)), BI(std::end(ic)), BI(std::begin(ia)), BI(std::end(ia))) == BI(ic+15))
           && (std::find_end(BI(std::begin(ic)), BI(std::end(ic)), BI(std::begin(ib)), BI(std::end(ib))) == BI(std::end(ic)))
           && (std::find_end(RI(std::begin(ic)), RI(std::end(ic)), RI(std::begin(ia)), RI(std::end(ia))) == RI(ic+15))
           && (std::find_end(RI(std::begin(ic)), RI(std::end(ic)), RI(std::begin(ib)), RI(std::end(ib))) == RI(std::end(ic)))
           ;
    }
#endif

template <class Iter1, class Iter2>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 0, 1, 2, 3, 0, 1, 2, 0, 1, 0};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int b[] = {0};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(b), Iter2(b+1)) == Iter1(ia+sa-1));
    int c[] = {0, 1};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(c), Iter2(c+2)) == Iter1(ia+18));
    int d[] = {0, 1, 2};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(d), Iter2(d+3)) == Iter1(ia+15));
    int e[] = {0, 1, 2, 3};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(e), Iter2(e+4)) == Iter1(ia+11));
    int f[] = {0, 1, 2, 3, 4};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(f), Iter2(f+5)) == Iter1(ia+6));
    int g[] = {0, 1, 2, 3, 4, 5};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(g), Iter2(g+6)) == Iter1(ia));
    int h[] = {0, 1, 2, 3, 4, 5, 6};
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(h), Iter2(h+7)) == Iter1(ia+sa));
    assert(std::find_end(Iter1(ia), Iter1(ia+sa), Iter2(b), Iter2(b)) == Iter1(ia+sa));
    assert(std::find_end(Iter1(ia), Iter1(ia), Iter2(b), Iter2(b+1)) == Iter1(ia));
}

int main()
{
    test<forward_iterator<const int*>, forward_iterator<const int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<forward_iterator<const int*>, random_access_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<const int*> >();
    test<random_access_iterator<const int*>, forward_iterator<const int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<const int*> >();

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
