//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// pair(pair const&) = default;
// pair(pair&&) = default;

// Doesn't pass due to use of is_trivially_* trait.
// XFAIL: gcc-4.9

#include <utility>
#include <cassert>

#include "test_macros.h"

struct Dummy {
  Dummy(Dummy const&) = delete;
  Dummy(Dummy &&) = default;
};

int main()
{
    typedef std::pair<int, short> P;
    {
        static_assert(std::is_copy_constructible<P>::value, "");
#if !defined(_LIBCPP_DEPRECATED_ABI_DISABLE_PAIR_TRIVIAL_COPY_CTOR)
        static_assert(std::is_trivially_copy_constructible<P>::value, "");
#endif
    }
#if TEST_STD_VER >= 11
    {
        static_assert(std::is_move_constructible<P>::value, "");
#if !defined(_LIBCPP_DEPRECATED_ABI_DISABLE_PAIR_TRIVIAL_COPY_CTOR)
        static_assert(std::is_trivially_move_constructible<P>::value, "");
#endif
    }
    {
        using P1 = std::pair<Dummy, int>;
        static_assert(!std::is_copy_constructible<P1>::value, "");
        static_assert(!std::is_trivially_copy_constructible<P1>::value, "");
        static_assert(std::is_move_constructible<P1>::value, "");
#if !defined(_LIBCPP_DEPRECATED_ABI_DISABLE_PAIR_TRIVIAL_COPY_CTOR)
        static_assert(std::is_trivially_move_constructible<P1>::value, "");
#endif
    }
#endif
}
