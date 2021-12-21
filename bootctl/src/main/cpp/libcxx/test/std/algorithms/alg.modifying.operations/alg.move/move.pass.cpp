//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter, typename OutIter>
//   requires OutputIterator<OutIter, RvalueOf<InIter::reference>::type>
//   OutIter
//   move(InIter first, InIter last, OutIter result);

#include <algorithm>
#include <cassert>
#include <memory>

#include "test_macros.h"
#include "test_iterators.h"

template <class InIter, class OutIter>
void
test()
{
    const unsigned N = 1000;
    int ia[N];
    for (unsigned i = 0; i < N; ++i)
        ia[i] = i;
    int ib[N] = {0};

    OutIter r = std::move(InIter(ia), InIter(ia+N), OutIter(ib));
    assert(base(r) == ib+N);
    for (unsigned i = 0; i < N; ++i)
        assert(ia[i] == ib[i]);
}

#if TEST_STD_VER >= 11
template <class InIter, class OutIter>
void
test1()
{
    const unsigned N = 100;
    std::unique_ptr<int> ia[N];
    for (unsigned i = 0; i < N; ++i)
        ia[i].reset(new int(i));
    std::unique_ptr<int> ib[N];

    OutIter r = std::move(InIter(ia), InIter(ia+N), OutIter(ib));
    assert(base(r) == ib+N);
    for (unsigned i = 0; i < N; ++i)
        assert(*ib[i] == static_cast<int>(i));
}
#endif

int main()
{
    test<input_iterator<const int*>, output_iterator<int*> >();
    test<input_iterator<const int*>, input_iterator<int*> >();
    test<input_iterator<const int*>, forward_iterator<int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<int*> >();
    test<input_iterator<const int*>, random_access_iterator<int*> >();
    test<input_iterator<const int*>, int*>();

    test<forward_iterator<const int*>, output_iterator<int*> >();
    test<forward_iterator<const int*>, input_iterator<int*> >();
    test<forward_iterator<const int*>, forward_iterator<int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<int*> >();
    test<forward_iterator<const int*>, random_access_iterator<int*> >();
    test<forward_iterator<const int*>, int*>();

    test<bidirectional_iterator<const int*>, output_iterator<int*> >();
    test<bidirectional_iterator<const int*>, input_iterator<int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<const int*>, int*>();

    test<random_access_iterator<const int*>, output_iterator<int*> >();
    test<random_access_iterator<const int*>, input_iterator<int*> >();
    test<random_access_iterator<const int*>, forward_iterator<int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<int*> >();
    test<random_access_iterator<const int*>, int*>();

    test<const int*, output_iterator<int*> >();
    test<const int*, input_iterator<int*> >();
    test<const int*, forward_iterator<int*> >();
    test<const int*, bidirectional_iterator<int*> >();
    test<const int*, random_access_iterator<int*> >();
    test<const int*, int*>();

#if TEST_STD_VER >= 11
    test1<input_iterator<std::unique_ptr<int>*>, output_iterator<std::unique_ptr<int>*> >();
    test1<input_iterator<std::unique_ptr<int>*>, input_iterator<std::unique_ptr<int>*> >();
    test1<input_iterator<std::unique_ptr<int>*>, forward_iterator<std::unique_ptr<int>*> >();
    test1<input_iterator<std::unique_ptr<int>*>, bidirectional_iterator<std::unique_ptr<int>*> >();
    test1<input_iterator<std::unique_ptr<int>*>, random_access_iterator<std::unique_ptr<int>*> >();
    test1<input_iterator<std::unique_ptr<int>*>, std::unique_ptr<int>*>();

    test1<forward_iterator<std::unique_ptr<int>*>, output_iterator<std::unique_ptr<int>*> >();
    test1<forward_iterator<std::unique_ptr<int>*>, input_iterator<std::unique_ptr<int>*> >();
    test1<forward_iterator<std::unique_ptr<int>*>, forward_iterator<std::unique_ptr<int>*> >();
    test1<forward_iterator<std::unique_ptr<int>*>, bidirectional_iterator<std::unique_ptr<int>*> >();
    test1<forward_iterator<std::unique_ptr<int>*>, random_access_iterator<std::unique_ptr<int>*> >();
    test1<forward_iterator<std::unique_ptr<int>*>, std::unique_ptr<int>*>();

    test1<bidirectional_iterator<std::unique_ptr<int>*>, output_iterator<std::unique_ptr<int>*> >();
    test1<bidirectional_iterator<std::unique_ptr<int>*>, input_iterator<std::unique_ptr<int>*> >();
    test1<bidirectional_iterator<std::unique_ptr<int>*>, forward_iterator<std::unique_ptr<int>*> >();
    test1<bidirectional_iterator<std::unique_ptr<int>*>, bidirectional_iterator<std::unique_ptr<int>*> >();
    test1<bidirectional_iterator<std::unique_ptr<int>*>, random_access_iterator<std::unique_ptr<int>*> >();
    test1<bidirectional_iterator<std::unique_ptr<int>*>, std::unique_ptr<int>*>();

    test1<random_access_iterator<std::unique_ptr<int>*>, output_iterator<std::unique_ptr<int>*> >();
    test1<random_access_iterator<std::unique_ptr<int>*>, input_iterator<std::unique_ptr<int>*> >();
    test1<random_access_iterator<std::unique_ptr<int>*>, forward_iterator<std::unique_ptr<int>*> >();
    test1<random_access_iterator<std::unique_ptr<int>*>, bidirectional_iterator<std::unique_ptr<int>*> >();
    test1<random_access_iterator<std::unique_ptr<int>*>, random_access_iterator<std::unique_ptr<int>*> >();
    test1<random_access_iterator<std::unique_ptr<int>*>, std::unique_ptr<int>*>();

    test1<std::unique_ptr<int>*, output_iterator<std::unique_ptr<int>*> >();
    test1<std::unique_ptr<int>*, input_iterator<std::unique_ptr<int>*> >();
    test1<std::unique_ptr<int>*, forward_iterator<std::unique_ptr<int>*> >();
    test1<std::unique_ptr<int>*, bidirectional_iterator<std::unique_ptr<int>*> >();
    test1<std::unique_ptr<int>*, random_access_iterator<std::unique_ptr<int>*> >();
    test1<std::unique_ptr<int>*, std::unique_ptr<int>*>();
#endif // TEST_STD_VER >= 11
}
