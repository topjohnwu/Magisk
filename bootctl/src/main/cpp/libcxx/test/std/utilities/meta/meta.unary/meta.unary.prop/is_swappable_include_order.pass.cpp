//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_swappable

// IMPORTANT: The include order is part of the test. We want to pick up
// the following definitions in this order:
//   1) is_swappable, is_nothrow_swappable
//   2) iter_swap, swap_ranges
//   3) swap(T (&)[N], T (&)[N])
// This test checks that (1) and (2) see forward declarations
// for (3).
#include <type_traits>
#include <algorithm>
#include <utility>

#include "test_macros.h"

int main()
{
    // Use a builtin type so we don't get ADL lookup.
    typedef double T[17][29];
    {
        LIBCPP_STATIC_ASSERT(std::__is_swappable<T>::value, "");
#if TEST_STD_VER > 14
        static_assert(std::is_swappable_v<T>, "");
#endif
    }
    {
        T t1 = {};
        T t2 = {};
       std::iter_swap(t1, t2);
       std::swap_ranges(t1, t1 + 17, t2);
    }
}
