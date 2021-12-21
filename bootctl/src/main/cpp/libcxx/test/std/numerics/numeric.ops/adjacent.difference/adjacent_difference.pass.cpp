//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <numeric>

// template <InputIterator InIter,
//           OutputIterator<auto, const InIter::value_type&> OutIter>
//   requires HasMinus<InIter::value_type, InIter::value_type>
//         && Constructible<InIter::value_type, InIter::reference>
//         && OutputIterator<OutIter,
//                           HasMinus<InIter::value_type, InIter::value_type>::result_type>
//         && MoveAssignable<InIter::value_type>
//   OutIter
//   adjacent_difference(InIter first, InIter last, OutIter result);

#include <numeric>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class InIter, class OutIter>
void
test()
{
    int ia[] = {15, 10, 6, 3, 1};
    int ir[] = {15, -5, -4, -3, -2};
    const unsigned s = sizeof(ia) / sizeof(ia[0]);
    int ib[s] = {0};
    OutIter r = std::adjacent_difference(InIter(ia), InIter(ia+s), OutIter(ib));
    assert(base(r) == ib + s);
    for (unsigned i = 0; i < s; ++i)
        assert(ib[i] == ir[i]);
}

#if TEST_STD_VER >= 11

class Y;

class X
{
    int i_;

    X& operator=(const X&);
public:
    explicit X(int i) : i_(i) {}
    X(const X& x) : i_(x.i_) {}
    X& operator=(X&& x)
    {
        i_ = x.i_;
        x.i_ = -1;
        return *this;
    }

    friend X operator-(const X& x, const X& y) {return X(x.i_ - y.i_);}

    friend class Y;
};

class Y
{
    int i_;

    Y& operator=(const Y&);
public:
    explicit Y(int i) : i_(i) {}
    Y(const Y& y) : i_(y.i_) {}
    void operator=(const X& x) {i_ = x.i_;}
};

#endif

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

#if TEST_STD_VER >= 11
    X x[3] = {X(1), X(2), X(3)};
    Y y[3] = {Y(1), Y(2), Y(3)};
    std::adjacent_difference(x, x+3, y);
#endif
}
