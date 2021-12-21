//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// template <class T, size_t N> void swap(array<T,N>& x, array<T,N>& y);

#include <array>
#include <cassert>

#include "test_macros.h"
// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

struct NonSwappable {
  NonSwappable() {}
private:
  NonSwappable(NonSwappable const&);
  NonSwappable& operator=(NonSwappable const&);
};

template <class Tp>
decltype(swap(std::declval<Tp>(), std::declval<Tp>()))
can_swap_imp(int);

template <class Tp>
std::false_type can_swap_imp(...);

template <class Tp>
struct can_swap : std::is_same<decltype(can_swap_imp<Tp>(0)), void> {};

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        C c1 = {1, 2, 3.5};
        C c2 = {4, 5, 6.5};
        swap(c1, c2);
        assert(c1.size() == 3);
        assert(c1[0] == 4);
        assert(c1[1] == 5);
        assert(c1[2] == 6.5);
        assert(c2.size() == 3);
        assert(c2[0] == 1);
        assert(c2[1] == 2);
        assert(c2[2] == 3.5);
    }
    {
        typedef double T;
        typedef std::array<T, 0> C;
        C c1 = {};
        C c2 = {};
        swap(c1, c2);
        assert(c1.size() == 0);
        assert(c2.size() == 0);
    }
    {
        typedef NonSwappable T;
        typedef std::array<T, 0> C0;
        static_assert(can_swap<C0&>::value, "");
        C0 l = {};
        C0 r = {};
        swap(l, r);
#if TEST_STD_VER >= 11
        static_assert(noexcept(swap(l, r)), "");
#endif
    }
#if TEST_STD_VER >= 11
    {
        // NonSwappable is still considered swappable in C++03 because there
        // is no access control SFINAE.
        typedef NonSwappable T;
        typedef std::array<T, 42> C1;
        static_assert(!can_swap<C1&>::value, "");
    }
#endif
}
