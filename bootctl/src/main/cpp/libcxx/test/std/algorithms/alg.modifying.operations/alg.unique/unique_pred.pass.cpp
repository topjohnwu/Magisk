//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, EquivalenceRelation<auto, Iter::value_type> Pred>
//   requires OutputIterator<Iter, RvalueOf<Iter::reference>::type>
//         && CopyConstructible<Pred>
//   constexpr Iter        // constexpr after C++17
//   unique(Iter first, Iter last, Pred pred);

#include <algorithm>
#include <cassert>
#include <memory>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
          int ia[]       = {0, 1, 1, 3, 4};
    const int expected[] = {0, 1, 3, 4};
    const size_t N = 4;

    auto it = std::unique(std::begin(ia), std::end(ia), [](int a, int b) {return a == b; });
    return it == (std::begin(ia) + N)
        && std::equal(std::begin(ia), it, std::begin(expected), std::end(expected))
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
    int ia[] = {0};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    count_equal::count = 0;
    Iter r = std::unique(Iter(ia), Iter(ia+sa), count_equal());
    assert(base(r) == ia + sa);
    assert(ia[0] == 0);
    assert(count_equal::count == sa-1);

    int ib[] = {0, 1};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ib), Iter(ib+sb), count_equal());
    assert(base(r) == ib + sb);
    assert(ib[0] == 0);
    assert(ib[1] == 1);
    assert(count_equal::count == sb-1);

    int ic[] = {0, 0};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ic), Iter(ic+sc), count_equal());
    assert(base(r) == ic + 1);
    assert(ic[0] == 0);
    assert(count_equal::count == sc-1);

    int id[] = {0, 0, 1};
    const unsigned sd = sizeof(id)/sizeof(id[0]);
    count_equal::count = 0;
    r = std::unique(Iter(id), Iter(id+sd), count_equal());
    assert(base(r) == id + 2);
    assert(id[0] == 0);
    assert(id[1] == 1);
    assert(count_equal::count == sd-1);

    int ie[] = {0, 0, 1, 0};
    const unsigned se = sizeof(ie)/sizeof(ie[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ie), Iter(ie+se), count_equal());
    assert(base(r) == ie + 3);
    assert(ie[0] == 0);
    assert(ie[1] == 1);
    assert(ie[2] == 0);
    assert(count_equal::count == se-1);

    int ig[] = {0, 0, 1, 1};
    const unsigned sg = sizeof(ig)/sizeof(ig[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ig), Iter(ig+sg), count_equal());
    assert(base(r) == ig + 2);
    assert(ig[0] == 0);
    assert(ig[1] == 1);
    assert(count_equal::count == sg-1);

    int ih[] = {0, 1, 1};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ih), Iter(ih+sh), count_equal());
    assert(base(r) == ih + 2);
    assert(ih[0] == 0);
    assert(ih[1] == 1);
    assert(count_equal::count == sh-1);

    int ii[] = {0, 1, 1, 1, 2, 2, 2};
    const unsigned si = sizeof(ii)/sizeof(ii[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ii), Iter(ii+si), count_equal());
    assert(base(r) == ii + 3);
    assert(ii[0] == 0);
    assert(ii[1] == 1);
    assert(ii[2] == 2);
    assert(count_equal::count == si-1);
}

#if TEST_STD_VER >= 11

struct do_nothing
{
    void operator()(void*) const {}
};

typedef std::unique_ptr<int, do_nothing> Ptr;

template <class Iter>
void
test1()
{
    int one = 1;
    int two = 2;
    Ptr ia[1];
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    count_equal::count = 0;
    Iter r = std::unique(Iter(ia), Iter(ia+sa), count_equal());
    assert(base(r) == ia + sa);
    assert(ia[0] == 0);
    assert(count_equal::count == sa-1);

    Ptr ib[2];
    ib[1].reset(&one);
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ib), Iter(ib+sb), count_equal());
    assert(base(r) == ib + sb);
    assert(ib[0] == 0);
    assert(*ib[1] == 1);
    assert(count_equal::count == sb-1);

    Ptr ic[2];
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ic), Iter(ic+sc), count_equal());
    assert(base(r) == ic + 1);
    assert(ic[0] == 0);
    assert(count_equal::count == sc-1);

    Ptr id[3];
    id[2].reset(&one);
    const unsigned sd = sizeof(id)/sizeof(id[0]);
    count_equal::count = 0;
    r = std::unique(Iter(id), Iter(id+sd), count_equal());
    assert(base(r) == id + 2);
    assert(id[0] == 0);
    assert(*id[1] == 1);
    assert(count_equal::count == sd-1);

    Ptr ie[4];
    ie[2].reset(&one);
    const unsigned se = sizeof(ie)/sizeof(ie[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ie), Iter(ie+se), count_equal());
    assert(base(r) == ie + 3);
    assert(ie[0] == 0);
    assert(*ie[1] == 1);
    assert(ie[2] == 0);
    assert(count_equal::count == se-1);

    Ptr ig[4];
    ig[2].reset(&one);
    ig[3].reset(&one);
    const unsigned sg = sizeof(ig)/sizeof(ig[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ig), Iter(ig+sg), count_equal());
    assert(base(r) == ig + 2);
    assert(ig[0] == 0);
    assert(*ig[1] == 1);
    assert(count_equal::count == sg-1);

    Ptr ih[3];
    ih[1].reset(&one);
    ih[2].reset(&one);
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ih), Iter(ih+sh), count_equal());
    assert(base(r) == ih + 2);
    assert(ih[0] == 0);
    assert(*ih[1] == 1);
    assert(count_equal::count == sh-1);

    Ptr ii[7];
    ii[1].reset(&one);
    ii[2].reset(&one);
    ii[3].reset(&one);
    ii[4].reset(&two);
    ii[5].reset(&two);
    ii[6].reset(&two);
    const unsigned si = sizeof(ii)/sizeof(ii[0]);
    count_equal::count = 0;
    r = std::unique(Iter(ii), Iter(ii+si), count_equal());
    assert(base(r) == ii + 3);
    assert(ii[0] == 0);
    assert(*ii[1] == 1);
    assert(*ii[2] == 2);
    assert(count_equal::count == si-1);
}
#endif // TEST_STD_VER >= 11

int main()
{
    test<forward_iterator<int*> >();
    test<bidirectional_iterator<int*> >();
    test<random_access_iterator<int*> >();
    test<int*>();

#if TEST_STD_VER >= 11
    test1<forward_iterator<Ptr*> >();
    test1<bidirectional_iterator<Ptr*> >();
    test1<random_access_iterator<Ptr*> >();
    test1<Ptr*>();
#endif

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
