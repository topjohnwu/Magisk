//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SUPPORT_TEST_CONVERTIBLE_HPP
#define SUPPORT_TEST_CONVERTIBLE_HPP

// "test_convertible<Tp, Args...>()" is a metafunction used to check if 'Tp'
// is implicitly convertible from 'Args...' for any number of arguments,
// Unlike 'std::is_convertible' which only allows checking for single argument
// conversions.

#include <type_traits>

#include "test_macros.h"

#if TEST_STD_VER < 11
#error test_convertible.hpp requires C++11 or newer
#endif

namespace detail {
    template <class Tp> void eat_type(Tp);

    template <class Tp, class ...Args>
    constexpr auto test_convertible_imp(int)
        -> decltype(eat_type<Tp>({std::declval<Args>()...}), true)
    { return true; }

    template <class Tp, class ...Args>
    constexpr auto test_convertible_imp(long) -> bool { return false; }
}

template <class Tp, class ...Args>
constexpr bool test_convertible()
{ return detail::test_convertible_imp<Tp, Args...>(0); }

#endif // SUPPORT_TEST_CONVERTIBLE_HPP
