//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, class T>
//   requires OutputIterator<Iter, RvalueOf<Iter::reference>::type>
//         && HasEqualTo<Iter::value_type, T>
//   constexpr Iter         // constexpr after C++17
//   remove(Iter first, Iter last, const T& value);

#include <algorithm>
#include <cassert>
#include <memory>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 5, 6};

    auto it = std::remove(std::begin(ia), std::end(ia), 5);

    return (std::begin(ia) + std::size(ia) - 2) == it  // we removed two elements
        && std::none_of(std::begin(ia), it, [](int a) {return a == 5; })
           ;
    }
#endif

template <class Iter>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 2, 3, 4, 2};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    Iter r = std::remove(Iter(ia), Iter(ia+sa), 2);
    assert(base(r) == ia + sa-3);
    assert(ia[0] == 0);
    assert(ia[1] == 1);
    assert(ia[2] == 3);
    assert(ia[3] == 4);
    assert(ia[4] == 3);
    assert(ia[5] == 4);
}

#if TEST_STD_VER >= 11
template <class Iter>
void
test1()
{
    const unsigned sa = 9;
    std::unique_ptr<int> ia[sa];
    ia[0].reset(new int(0));
    ia[1].reset(new int(1));
    ia[3].reset(new int(3));
    ia[4].reset(new int(4));
    ia[6].reset(new int(3));
    ia[7].reset(new int(4));
    Iter r = std::remove(Iter(ia), Iter(ia+sa), std::unique_ptr<int>());
    assert(base(r) == ia + sa-3);
    assert(*ia[0] == 0);
    assert(*ia[1] == 1);
    assert(*ia[2] == 3);
    assert(*ia[3] == 4);
    assert(*ia[4] == 3);
    assert(*ia[5] == 4);
}
#endif // TEST_STD_VER >= 11

int main()
{
    test<forward_iterator<int*> >();
    test<bidirectional_iterator<int*> >();
    test<random_access_iterator<int*> >();
    test<int*>();

#if TEST_STD_VER >= 11
    test1<forward_iterator<std::unique_ptr<int>*> >();
    test1<bidirectional_iterator<std::unique_ptr<int>*> >();
    test1<random_access_iterator<std::unique_ptr<int>*> >();
    test1<std::unique_ptr<int>*>();
#endif // TEST_STD_VER >= 11

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
