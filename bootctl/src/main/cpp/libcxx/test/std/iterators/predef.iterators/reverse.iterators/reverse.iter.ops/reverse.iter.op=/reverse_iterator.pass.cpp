//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// reverse_iterator

// template <class U>
//   requires HasAssign<Iter, const U&>
//   constexpr reverse_iterator&
//   operator=(const reverse_iterator<U>& u);
//
//   constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It, class U>
void
test(U u)
{
    const std::reverse_iterator<U> r2(u);
    std::reverse_iterator<It> r1;
    std::reverse_iterator<It>& rr = r1 = r2;
    assert(r1.base() == u);
    assert(&rr == &r1);
}

struct Base {};
struct Derived : Base {};

int main()
{
    Derived d;

    test<bidirectional_iterator<Base*> >(bidirectional_iterator<Derived*>(&d));
    test<random_access_iterator<const Base*> >(random_access_iterator<Derived*>(&d));
    test<Base*>(&d);

#if TEST_STD_VER > 14
    {
        using BaseIter    = std::reverse_iterator<const Base *>;
        using DerivedIter = std::reverse_iterator<const Derived *>;
        constexpr const Derived *p = nullptr;
        constexpr DerivedIter     it1 = std::make_reverse_iterator(p);
        constexpr BaseIter        it2 = (BaseIter{nullptr} = it1);
        static_assert(it2.base() == p, "");
    }
#endif
}
