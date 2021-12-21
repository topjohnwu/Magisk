//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// constexpr size_type find_first_not_of(charT c, size_type pos = 0) const;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type pos,
     typename S::size_type x)
{
    assert(s.find_first_not_of(c, pos) == x);
    if (x != S::npos)
        assert(pos <= x && x < s.size());
}

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type x)
{
    assert(s.find_first_not_of(c) == x);
    if (x != S::npos)
        assert(x < s.size());
}

int main()
{
    {
    typedef std::string_view S;
    test(S(""), 'q', 0, S::npos);
    test(S(""), 'q', 1, S::npos);
    test(S("kitcj"), 'q', 0, 0);
    test(S("qkamf"), 'q', 1, 1);
    test(S("nhmko"), 'q', 2, 2);
    test(S("tpsaf"), 'q', 4, 4);
    test(S("lahfb"), 'q', 5, S::npos);
    test(S("irkhs"), 'q', 6, S::npos);
    test(S("gmfhdaipsr"), 'q', 0, 0);
    test(S("kantesmpgj"), 'q', 1, 1);
    test(S("odaftiegpm"), 'q', 5, 5);
    test(S("oknlrstdpi"), 'q', 9, 9);
    test(S("eolhfgpjqk"), 'q', 10, S::npos);
    test(S("pcdrofikas"), 'q', 11, S::npos);
    test(S("nbatdlmekrgcfqsophij"), 'q', 0, 0);
    test(S("bnrpehidofmqtcksjgla"), 'q', 1, 1);
    test(S("jdmciepkaqgotsrfnhlb"), 'q', 10, 10);
    test(S("jtdaefblsokrmhpgcnqi"), 'q', 19, 19);
    test(S("hkbgspofltajcnedqmri"), 'q', 20, S::npos);
    test(S("oselktgbcapndfjihrmq"), 'q', 21, S::npos);

    test(S(""), 'q', S::npos);
    test(S("q"), 'q', S::npos);
    test(S("qqq"), 'q', S::npos);
    test(S("csope"), 'q', 0);
    test(S("gfsmthlkon"), 'q', 0);
    test(S("laenfsbridchgotmkqpj"), 'q', 0);
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1;
    constexpr SV  sv2 { "abcde", 5 };

    static_assert (sv1.find_first_not_of( 'q', 0 ) == SV::npos, "" );
    static_assert (sv1.find_first_not_of( 'q', 1 ) == SV::npos, "" );
    static_assert (sv2.find_first_not_of( 'q', 0 ) == 0, "" );
    static_assert (sv2.find_first_not_of( 'q', 1 ) == 1, "" );
    static_assert (sv2.find_first_not_of( 'q', 5 ) == SV::npos, "" );
    }
#endif
}
