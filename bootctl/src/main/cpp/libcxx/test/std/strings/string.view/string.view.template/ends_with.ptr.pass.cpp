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

//   constexpr bool starts_with(const CharT *x) const;

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
    SV  sv1 { s + 4, 1 };
    SV  sv2 { s + 3, 2 };
//     SV  sv3 { s + 2, 3 };
//     SV  sv4 { s + 1, 4 };
//     SV  sv5 { s    , 5 };
    SV  svNot {"def", 3 };

    LIBCPP_ASSERT_NOEXCEPT(sv0.ends_with(""));

    assert ( sv0.ends_with(""));
    assert (!sv0.ends_with("e"));

    assert ( sv1.ends_with(""));
    assert ( sv1.ends_with("e"));
    assert (!sv1.ends_with("de"));
    assert (!sv1.ends_with("cde"));
    assert (!sv1.ends_with("bcde"));
    assert (!sv1.ends_with("abcde"));
    assert (!sv1.ends_with("def"));

    assert ( sv2.ends_with(""));
    assert ( sv2.ends_with("e"));
    assert ( sv2.ends_with("de"));
    assert (!sv2.ends_with("cde"));
    assert (!sv2.ends_with("bcde"));
    assert (!sv2.ends_with("abcde"));
    assert (!sv2.ends_with("def"));

    assert ( svNot.ends_with(""));
    assert (!svNot.ends_with("e"));
    assert (!svNot.ends_with("de"));
    assert (!svNot.ends_with("cde"));
    assert (!svNot.ends_with("bcde"));
    assert (!svNot.ends_with("abcde"));
    assert ( svNot.ends_with("def"));
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr const char *s = "abcde";
    constexpr SV  sv0 {};
    constexpr SV  sv1 { s + 4, 1 };
    constexpr SV  sv2 { s + 3, 2 };
//     constexpr SV  sv3 { s + 2, 3 };
//     constexpr SV  sv4 { s + 1, 4 };
//     constexpr SV  sv5 { s,     5 };
    constexpr SV  svNot {"def", 3 };

    static_assert ( sv0.ends_with(""), "" );
    static_assert (!sv0.ends_with("e"), "" );

    static_assert ( sv1.ends_with(""), "" );
    static_assert ( sv1.ends_with("e"), "" );
    static_assert (!sv1.ends_with("de"), "" );
    static_assert (!sv1.ends_with("cde"), "" );
    static_assert (!sv1.ends_with("bcde"), "" );
    static_assert (!sv1.ends_with("abcde"), "" );
    static_assert (!sv1.ends_with("def"), "" );

    static_assert ( sv2.ends_with(""), "" );
    static_assert ( sv2.ends_with("e"), "" );
    static_assert ( sv2.ends_with("de"), "" );
    static_assert (!sv2.ends_with("cde"), "" );
    static_assert (!sv2.ends_with("bcde"), "" );
    static_assert (!sv2.ends_with("abcde"), "" );
    static_assert (!sv2.ends_with("def"), "" );

    static_assert ( svNot.ends_with(""), "" );
    static_assert (!svNot.ends_with("e"), "" );
    static_assert (!svNot.ends_with("de"), "" );
    static_assert (!svNot.ends_with("cde"), "" );
    static_assert (!svNot.ends_with("bcde"), "" );
    static_assert (!svNot.ends_with("abcde"), "" );
    static_assert ( svNot.ends_with("def"), "" );
    }
#endif
}
