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

//   constexpr bool starts_with(string_view x) const noexcept;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

int main()
{
    {
    typedef std::string_view SV;
    const char *s = "abcde";
    SV  sv0 {};
    SV  sv1 { s, 1 };
    SV  sv2 { s, 2 };
//     SV  sv3 { s, 3 };
//     SV  sv4 { s, 4 };
//     SV  sv5 { s, 5 };
    SV  svNot {"def", 3 };

    LIBCPP_ASSERT_NOEXCEPT(sv0.starts_with(""));

    assert ( sv0.starts_with(""));
    assert (!sv0.starts_with("a"));

    assert ( sv1.starts_with(""));
    assert ( sv1.starts_with("a"));
    assert (!sv1.starts_with("ab"));
    assert (!sv1.starts_with("abc"));
    assert (!sv1.starts_with("abcd"));
    assert (!sv1.starts_with("abcde"));
    assert (!sv1.starts_with("def"));

    assert ( sv2.starts_with(s + 5));
    assert ( sv2.starts_with("a"));
    assert ( sv2.starts_with("ab"));
    assert (!sv2.starts_with("abc"));
    assert (!sv2.starts_with("abcd"));
    assert (!sv2.starts_with("abcde"));
    assert (!sv2.starts_with("def"));

    assert ( svNot.starts_with(""));
    assert (!svNot.starts_with("a"));
    assert (!svNot.starts_with("ab"));
    assert (!svNot.starts_with("abc"));
    assert (!svNot.starts_with("abcd"));
    assert (!svNot.starts_with("abcde"));
    assert ( svNot.starts_with("def"));
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr const char *s = "abcde";
    constexpr SV  sv0 {};
    constexpr SV  sv1 { s, 1 };
    constexpr SV  sv2 { s, 2 };
//     constexpr SV  sv3 { s, 3 };
//     constexpr SV  sv4 { s, 4 };
//     constexpr SV  sv5 { s, 5 };
    constexpr SV  svNot {"def", 3 };

    static_assert ( sv0.starts_with(""), "" );
    static_assert (!sv0.starts_with("a"), "" );

    static_assert ( sv1.starts_with(""), "" );
    static_assert ( sv1.starts_with("a"), "" );
    static_assert (!sv1.starts_with("ab"), "" );
    static_assert (!sv1.starts_with("abc"), "" );
    static_assert (!sv1.starts_with("abcd"), "" );
    static_assert (!sv1.starts_with("abcde"), "" );
    static_assert (!sv1.starts_with("def"), "" );

    static_assert ( sv2.starts_with(s + 5), "" );
    static_assert ( sv2.starts_with("a"), "" );
    static_assert ( sv2.starts_with("ab"), "" );
    static_assert (!sv2.starts_with("abc"), "" );
    static_assert (!sv2.starts_with("abcd"), "" );
    static_assert (!sv2.starts_with("abcde"), "" );
    static_assert (!sv2.starts_with("def"), "" );

    static_assert ( svNot.starts_with(""), "" );
    static_assert (!svNot.starts_with("a"), "" );
    static_assert (!svNot.starts_with("ab"), "" );
    static_assert (!svNot.starts_with("abc"), "" );
    static_assert (!svNot.starts_with("abcd"), "" );
    static_assert (!svNot.starts_with("abcde"), "" );
    static_assert ( svNot.starts_with("def"), "" );
    }
#endif
}
