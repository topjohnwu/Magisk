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
//   constexpr Iter1     // constexpr after C++17
//   search(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool eq(int a, int b) { return a == b; }

TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {0, 1, 2, 3};
    int ib[] = {0, 1, 5, 3};
    int ic[] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
    return    (std::search(std::begin(ic), std::end(ic), std::begin(ia), std::end(ia), eq) == ic+3)
           && (std::search(std::begin(ic), std::end(ic), std::begin(ib), std::end(ib), eq) == std::end(ic))
           ;
    }
#endif

struct count_equal
{
    static unsigned count;
    template <class T>
    bool operator()(const T& x, const T& y)
        {++count; return x == y;}
};

unsigned count_equal::count = 0;

template <class Iter1, class Iter2>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia), count_equal()) == Iter1(ia));
    assert(count_equal::count <= 0);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia+1), count_equal()) == Iter1(ia));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+1), Iter2(ia+2), count_equal()) == Iter1(ia+1));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+2), Iter2(ia+2), count_equal()) == Iter1(ia));
    assert(count_equal::count <= 0);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+2), Iter2(ia+3), count_equal()) == Iter1(ia+2));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+2), Iter2(ia+3), count_equal()) == Iter1(ia+2));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia), Iter2(ia+2), Iter2(ia+3), count_equal()) == Iter1(ia));
    assert(count_equal::count <= 0);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+sa-1), Iter2(ia+sa), count_equal()) == Iter1(ia+sa-1));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia+sa-3), Iter2(ia+sa), count_equal()) == Iter1(ia+sa-3));
    assert(count_equal::count <= sa*3);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa), Iter2(ia), Iter2(ia+sa), count_equal()) == Iter1(ia));
    assert(count_equal::count <= sa*sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+sa-1), Iter2(ia), Iter2(ia+sa), count_equal()) == Iter1(ia+sa-1));
    assert(count_equal::count <= (sa-1)*sa);
    count_equal::count = 0;
    assert(std::search(Iter1(ia), Iter1(ia+1), Iter2(ia), Iter2(ia+sa), count_equal()) == Iter1(ia+1));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    int ib[] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int ic[] = {1};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(ic), Iter2(ic+1), count_equal()) == Iter1(ib+1));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    int id[] = {1, 2};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(id), Iter2(id+2), count_equal()) == Iter1(ib+1));
    assert(count_equal::count <= sb*2);
    count_equal::count = 0;
    int ie[] = {1, 2, 3};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(ie), Iter2(ie+3), count_equal()) == Iter1(ib+4));
    assert(count_equal::count <= sb*3);
    count_equal::count = 0;
    int ig[] = {1, 2, 3, 4};
    assert(std::search(Iter1(ib), Iter1(ib+sb), Iter2(ig), Iter2(ig+4), count_equal()) == Iter1(ib+8));
    assert(count_equal::count <= sb*4);
    count_equal::count = 0;
    int ih[] = {0, 1, 1, 1, 1, 2, 3, 0, 1, 2, 3, 4};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    int ii[] = {1, 1, 2};
    assert(std::search(Iter1(ih), Iter1(ih+sh), Iter2(ii), Iter2(ii+3), count_equal()) == Iter1(ih+3));
    assert(count_equal::count <= sh*3);
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
