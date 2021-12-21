//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<BidirectionalIterator Iter, Predicate<auto, Iter::value_type> Pred>
//   requires ShuffleIterator<Iter>
//         && CopyConstructible<Pred>
//   Iter
//   partition(Iter first, Iter last, Pred pred);

#include <algorithm>
#include <cassert>


#include "test_iterators.h"

struct is_odd
{
    bool operator()(const int& i) const {return i & 1;}
};

template <class Iter>
void
test()
{
    // check mixed
    int ia[] = {1, 2, 3, 4, 5, 6, 7, 8 ,9};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    Iter r = std::partition(Iter(ia), Iter(ia + sa), is_odd());
    assert(base(r) == ia + 5);
    for (int* i = ia; i < base(r); ++i)
        assert(is_odd()(*i));
    for (int* i = base(r); i < ia+sa; ++i)
        assert(!is_odd()(*i));
    // check empty
    r = std::partition(Iter(ia), Iter(ia), is_odd());
    assert(base(r) == ia);
    // check all false
    for (unsigned i = 0; i < sa; ++i)
        ia[i] = 2*i;
    r = std::partition(Iter(ia), Iter(ia+sa), is_odd());
    assert(base(r) == ia);
    // check all true
    for (unsigned i = 0; i < sa; ++i)
        ia[i] = 2*i+1;
    r = std::partition(Iter(ia), Iter(ia+sa), is_odd());
    assert(base(r) == ia+sa);
    // check all true but last
    for (unsigned i = 0; i < sa; ++i)
        ia[i] = 2*i+1;
    ia[sa-1] = 10;
    r = std::partition(Iter(ia), Iter(ia+sa), is_odd());
    assert(base(r) == ia+sa-1);
    for (int* i = ia; i < base(r); ++i)
        assert(is_odd()(*i));
    for (int* i = base(r); i < ia+sa; ++i)
        assert(!is_odd()(*i));
    // check all true but first
    for (unsigned i = 0; i < sa; ++i)
        ia[i] = 2*i+1;
    ia[0] = 10;
    r = std::partition(Iter(ia), Iter(ia+sa), is_odd());
    assert(base(r) == ia+sa-1);
    for (int* i = ia; i < base(r); ++i)
        assert(is_odd()(*i));
    for (int* i = base(r); i < ia+sa; ++i)
        assert(!is_odd()(*i));
    // check all false but last
    for (unsigned i = 0; i < sa; ++i)
        ia[i] = 2*i;
    ia[sa-1] = 11;
    r = std::partition(Iter(ia), Iter(ia+sa), is_odd());
    assert(base(r) == ia+1);
    for (int* i = ia; i < base(r); ++i)
        assert(is_odd()(*i));
    for (int* i = base(r); i < ia+sa; ++i)
        assert(!is_odd()(*i));
    // check all false but first
    for (unsigned i = 0; i < sa; ++i)
        ia[i] = 2*i;
    ia[0] = 11;
    r = std::partition(Iter(ia), Iter(ia+sa), is_odd());
    assert(base(r) == ia+1);
    for (int* i = ia; i < base(r); ++i)
        assert(is_odd()(*i));
    for (int* i = base(r); i < ia+sa; ++i)
        assert(!is_odd()(*i));
}

int main()
{
    test<bidirectional_iterator<int*> >();
    test<random_access_iterator<int*> >();
    test<int*>();
}
