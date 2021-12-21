//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>
// UNSUPPORTED: c++98, c++03, c++11, c++14

// template<class InputIterator, class Size, class Function>
//    constexpr InputIterator      // constexpr after C++17
//    for_each_n(InputIterator first, Size n, Function f);


#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 6, 7};
    int expected[] = {3, 5, 8, 9};
    const size_t N = 4;

    auto it = std::for_each_n(std::begin(ia), N, [](int &a) { a += 2; });
    return it == (std::begin(ia) + N)
        && std::equal(std::begin(ia), std::end(ia), std::begin(expected))
        ;
    }
#endif

struct for_each_test
{
    for_each_test(int c) : count(c) {}
    int count;
    void operator()(int& i) {++i; ++count;}
};

int main()
{
    typedef input_iterator<int*> Iter;
    int ia[] = {0, 1, 2, 3, 4, 5};
    const unsigned s = sizeof(ia)/sizeof(ia[0]);

    {
    auto f = for_each_test(0);
    Iter it = std::for_each_n(Iter(ia), 0, std::ref(f));
    assert(it == Iter(ia));
    assert(f.count == 0);
    }

    {
    auto f = for_each_test(0);
    Iter it = std::for_each_n(Iter(ia), s, std::ref(f));

    assert(it == Iter(ia+s));
    assert(f.count == s);
    for (unsigned i = 0; i < s; ++i)
        assert(ia[i] == static_cast<int>(i+1));
    }

    {
    auto f = for_each_test(0);
    Iter it = std::for_each_n(Iter(ia), 1, std::ref(f));

    assert(it == Iter(ia+1));
    assert(f.count == 1);
    for (unsigned i = 0; i < 1; ++i)
        assert(ia[i] == static_cast<int>(i+2));
    }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
