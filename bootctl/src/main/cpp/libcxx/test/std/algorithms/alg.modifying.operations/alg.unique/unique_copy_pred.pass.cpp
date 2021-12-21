//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter, class OutIter,
//          EquivalenceRelation<auto, InIter::value_type> Pred>
//   requires OutputIterator<OutIter, RvalueOf<InIter::value_type>::type>
//         && HasAssign<InIter::value_type, InIter::reference>
//         && Constructible<InIter::value_type, InIter::reference>
//         && CopyConstructible<Pred>
//   constexpr OutIter        // constexpr after C++17
//   unique_copy(InIter first, InIter last, OutIter result, Pred pred);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
          int ia[]       = {0, 1, 2, 2, 4};
          int ib[]       = {0, 0, 0, 0, 0};
    const int expected[] = {0, 1, 2, 4};

    auto it = std::unique_copy(std::begin(ia), std::end(ia), std::begin(ib),
                         [](int a, int b) {return a == b; });
    return it == (std::begin(ib) + std::size(expected))
        && *it == 0 // don't overwrite final value in output
        && std::equal(std::begin(ib), it, std::begin(expected), std::end(expected))
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

template <class InIter, class OutIter>
void
test()
{
    const int ia[] = {0};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    int ja[sa] = {-1};
    count_equal::count = 0;
    OutIter r = std::unique_copy(InIter(ia), InIter(ia+sa), OutIter(ja), count_equal());
    assert(base(r) == ja + sa);
    assert(ja[0] == 0);
    assert(count_equal::count == sa-1);

    const int ib[] = {0, 1};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    int jb[sb] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(ib), InIter(ib+sb), OutIter(jb), count_equal());
    assert(base(r) == jb + sb);
    assert(jb[0] == 0);
    assert(jb[1] == 1);
    assert(count_equal::count == sb-1);

    const int ic[] = {0, 0};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    int jc[sc] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(ic), InIter(ic+sc), OutIter(jc), count_equal());
    assert(base(r) == jc + 1);
    assert(jc[0] == 0);
    assert(count_equal::count == sc-1);

    const int id[] = {0, 0, 1};
    const unsigned sd = sizeof(id)/sizeof(id[0]);
    int jd[sd] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(id), InIter(id+sd), OutIter(jd), count_equal());
    assert(base(r) == jd + 2);
    assert(jd[0] == 0);
    assert(jd[1] == 1);
    assert(count_equal::count == sd-1);

    const int ie[] = {0, 0, 1, 0};
    const unsigned se = sizeof(ie)/sizeof(ie[0]);
    int je[se] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(ie), InIter(ie+se), OutIter(je), count_equal());
    assert(base(r) == je + 3);
    assert(je[0] == 0);
    assert(je[1] == 1);
    assert(je[2] == 0);
    assert(count_equal::count == se-1);

    const int ig[] = {0, 0, 1, 1};
    const unsigned sg = sizeof(ig)/sizeof(ig[0]);
    int jg[sg] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(ig), InIter(ig+sg), OutIter(jg), count_equal());
    assert(base(r) == jg + 2);
    assert(jg[0] == 0);
    assert(jg[1] == 1);
    assert(count_equal::count == sg-1);

    const int ih[] = {0, 1, 1};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    int jh[sh] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(ih), InIter(ih+sh), OutIter(jh), count_equal());
    assert(base(r) == jh + 2);
    assert(jh[0] == 0);
    assert(jh[1] == 1);
    assert(count_equal::count == sh-1);

    const int ii[] = {0, 1, 1, 1, 2, 2, 2};
    const unsigned si = sizeof(ii)/sizeof(ii[0]);
    int ji[si] = {-1};
    count_equal::count = 0;
    r = std::unique_copy(InIter(ii), InIter(ii+si), OutIter(ji), count_equal());
    assert(base(r) == ji + 3);
    assert(ji[0] == 0);
    assert(ji[1] == 1);
    assert(ji[2] == 2);
    assert(count_equal::count == si-1);
}

int main()
{
    test<input_iterator<const int*>, output_iterator<int*> >();
    test<input_iterator<const int*>, forward_iterator<int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<int*> >();
    test<input_iterator<const int*>, random_access_iterator<int*> >();
    test<input_iterator<const int*>, int*>();

    test<forward_iterator<const int*>, output_iterator<int*> >();
    test<forward_iterator<const int*>, forward_iterator<int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<int*> >();
    test<forward_iterator<const int*>, random_access_iterator<int*> >();
    test<forward_iterator<const int*>, int*>();

    test<bidirectional_iterator<const int*>, output_iterator<int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<const int*>, int*>();

    test<random_access_iterator<const int*>, output_iterator<int*> >();
    test<random_access_iterator<const int*>, forward_iterator<int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<int*> >();
    test<random_access_iterator<const int*>, int*>();

    test<const int*, output_iterator<int*> >();
    test<const int*, forward_iterator<int*> >();
    test<const int*, bidirectional_iterator<int*> >();
    test<const int*, random_access_iterator<int*> >();
    test<const int*, int*>();

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
