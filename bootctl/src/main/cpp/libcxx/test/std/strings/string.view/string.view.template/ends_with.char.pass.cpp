//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <string_view>

//   constexpr bool ends_with(charT x) const noexcept;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

int main()
{
    {
    typedef std::string_view SV;
    SV  sv1 {};
    SV  sv2 { "abcde", 5 };

    ASSERT_NOEXCEPT(sv1.ends_with('e'));

    assert (!sv1.ends_with('e'));
    assert (!sv1.ends_with('x'));
    assert ( sv2.ends_with('e'));
    assert (!sv2.ends_with('x'));
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1 {};
    constexpr SV  sv2 { "abcde", 5 };
    static_assert (!sv1.ends_with('e'), "" );
    static_assert (!sv1.ends_with('x'), "" );
    static_assert ( sv2.ends_with('e'), "" );
    static_assert (!sv2.ends_with('x'), "" );
    }
#endif
}
