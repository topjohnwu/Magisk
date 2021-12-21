//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<class ForwardIterator, class Size, class T, class BinaryPredicate>
//   constexpr ForwardIterator     // constexpr after C++17
//   search_n(ForwardIterator first, ForwardIterator last, Size count,
//            const T& value, BinaryPredicate pred);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "user_defined_integral.hpp"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool eq(int a, int b) { return a == b; }

TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {0, 0, 1, 1, 2, 2};
    return    (std::search_n(std::begin(ia), std::end(ia), 1, 0, eq) == ia)
           && (std::search_n(std::begin(ia), std::end(ia), 2, 1, eq) == ia+2)
           && (std::search_n(std::begin(ia), std::end(ia), 1, 3, eq) == std::end(ia))
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


template <class Iter>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 0, 0, count_equal()) == Iter(ia));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 1, 0, count_equal()) == Iter(ia+0));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 2, 0, count_equal()) == Iter(ia+sa));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), sa, 0, count_equal()) == Iter(ia+sa));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 0, 3, count_equal()) == Iter(ia));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 1, 3, count_equal()) == Iter(ia+3));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 2, 3, count_equal()) == Iter(ia+sa));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), sa, 3, count_equal()) == Iter(ia+sa));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 0, 5, count_equal()) == Iter(ia));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 1, 5, count_equal()) == Iter(ia+5));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), 2, 5, count_equal()) == Iter(ia+sa));
    assert(count_equal::count <= sa);
    count_equal::count = 0;
    assert(std::search_n(Iter(ia), Iter(ia+sa), sa, 5, count_equal()) == Iter(ia+sa));
    assert(count_equal::count <= sa);
    count_equal::count = 0;

    int ib[] = {0, 0, 1, 1, 2, 2};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    assert(std::search_n(Iter(ib), Iter(ib+sb), 0, 0, count_equal()) == Iter(ib));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 1, 0, count_equal()) == Iter(ib+0));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 2, 0, count_equal()) == Iter(ib+0));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 3, 0, count_equal()) == Iter(ib+sb));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), sb, 0, count_equal()) == Iter(ib+sb));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 0, 1, count_equal()) == Iter(ib));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 1, 1, count_equal()) == Iter(ib+2));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 2, 1, count_equal()) == Iter(ib+2));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 3, 1, count_equal()) == Iter(ib+sb));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), sb, 1, count_equal()) == Iter(ib+sb));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 0, 2, count_equal()) == Iter(ib));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 1, 2, count_equal()) == Iter(ib+4));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 2, 2, count_equal()) == Iter(ib+4));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), 3, 2, count_equal()) == Iter(ib+sb));
    assert(count_equal::count <= sb);
    count_equal::count = 0;
    assert(std::search_n(Iter(ib), Iter(ib+sb), sb, 2, count_equal()) == Iter(ib+sb));
    assert(count_equal::count <= sb);
    count_equal::count = 0;

    int ic[] = {0, 0, 0};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    assert(std::search_n(Iter(ic), Iter(ic+sc), 0, 0, count_equal()) == Iter(ic));
    assert(count_equal::count <= sc);
    count_equal::count = 0;
    assert(std::search_n(Iter(ic), Iter(ic+sc), 1, 0, count_equal()) == Iter(ic));
    assert(count_equal::count <= sc);
    count_equal::count = 0;
    assert(std::search_n(Iter(ic), Iter(ic+sc), 2, 0, count_equal()) == Iter(ic));
    assert(count_equal::count <= sc);
    count_equal::count = 0;
    assert(std::search_n(Iter(ic), Iter(ic+sc), 3, 0, count_equal()) == Iter(ic));
    assert(count_equal::count <= sc);
    count_equal::count = 0;
    assert(std::search_n(Iter(ic), Iter(ic+sc), 4, 0, count_equal()) == Iter(ic+sc));
    assert(count_equal::count <= sc);
    count_equal::count = 0;

    // Check that we properly convert the size argument to an integral.
    TEST_IGNORE_NODISCARD std::search_n(Iter(ic), Iter(ic+sc), UserDefinedIntegral<unsigned>(4), 0, count_equal());
    count_equal::count = 0;
}

int main()
{
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
