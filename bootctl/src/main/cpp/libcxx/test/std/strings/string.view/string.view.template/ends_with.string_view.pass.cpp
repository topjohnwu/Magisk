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

//   constexpr bool ends_with(string_view x) const noexcept;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

int main()
{
    {
    typedef std::string_view SV;
    const char *s = "abcde";
    SV  sv0;
    SV  sv1 { s + 4, 1 };
    SV  sv2 { s + 3, 2 };
    SV  sv3 { s + 2, 3 };
    SV  sv4 { s + 1, 4 };
    SV  sv5 { s    , 5 };
    SV  svNot {"def", 3 };

    ASSERT_NOEXCEPT(sv0.ends_with(sv0));

    assert ( sv0.ends_with(sv0));
    assert (!sv0.ends_with(sv1));

    assert ( sv1.ends_with(sv0));
    assert ( sv1.ends_with(sv1));
    assert (!sv1.ends_with(sv2));
    assert (!sv1.ends_with(sv3));
    assert (!sv1.ends_with(sv4));
    assert (!sv1.ends_with(sv5));
    assert (!sv1.ends_with(svNot));

    assert ( sv2.ends_with(sv0));
    assert ( sv2.ends_with(sv1));
    assert ( sv2.ends_with(sv2));
    assert (!sv2.ends_with(sv3));
    assert (!sv2.ends_with(sv4));
    assert (!sv2.ends_with(sv5));
    assert (!sv2.ends_with(svNot));

    assert ( svNot.ends_with(sv0));
    assert (!svNot.ends_with(sv1));
    assert (!svNot.ends_with(sv2));
    assert (!svNot.ends_with(sv3));
    assert (!svNot.ends_with(sv4));
    assert (!svNot.ends_with(sv5));
    assert ( svNot.ends_with(svNot));
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr const char *s = "abcde";
    constexpr SV  sv0 {};
    constexpr SV  sv1 { s + 4, 1 };
    constexpr SV  sv2 { s + 3, 2 };
    constexpr SV  sv3 { s + 2, 3 };
    constexpr SV  sv4 { s + 1, 4 };
    constexpr SV  sv5 { s,     5 };
    constexpr SV  svNot {"def", 3 };

    static_assert ( sv0.ends_with(sv0), "" );
    static_assert (!sv0.ends_with(sv1), "" );

    static_assert ( sv1.ends_with(sv0), "" );
    static_assert ( sv1.ends_with(sv1), "" );
    static_assert (!sv1.ends_with(sv2), "" );
    static_assert (!sv1.ends_with(sv3), "" );
    static_assert (!sv1.ends_with(sv4), "" );
    static_assert (!sv1.ends_with(sv5), "" );
    static_assert (!sv1.ends_with(svNot), "" );

    static_assert ( sv2.ends_with(sv0), "" );
    static_assert ( sv2.ends_with(sv1), "" );
    static_assert ( sv2.ends_with(sv2), "" );
    static_assert (!sv2.ends_with(sv3), "" );
    static_assert (!sv2.ends_with(sv4), "" );
    static_assert (!sv2.ends_with(sv5), "" );
    static_assert (!sv2.ends_with(svNot), "" );

    static_assert ( svNot.ends_with(sv0), "" );
    static_assert (!svNot.ends_with(sv1), "" );
    static_assert (!svNot.ends_with(sv2), "" );
    static_assert (!svNot.ends_with(sv3), "" );
    static_assert (!svNot.ends_with(sv4), "" );
    static_assert (!svNot.ends_with(sv5), "" );
    static_assert ( svNot.ends_with(svNot), "" );
    }
#endif
}
