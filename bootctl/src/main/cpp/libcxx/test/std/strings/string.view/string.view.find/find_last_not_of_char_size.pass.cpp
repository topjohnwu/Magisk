//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// const size_type find_last_not_of(charT c, size_type pos = npos) const;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type pos,
     typename S::size_type x)
{
    assert(s.find_last_not_of(c, pos) == x);
    if (x != S::npos)
        assert(x <= pos && x < s.size());
}

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type x)
{
    assert(s.find_last_not_of(c) == x);
    if (x != S::npos)
        assert(x < s.size());
}

int main()
{
    {
    typedef std::string_view S;
    test(S(""), 'i', 0, S::npos);
    test(S(""), 'i', 1, S::npos);
    test(S("kitcj"), 'i', 0, 0);
    test(S("qkamf"), 'i', 1, 1);
    test(S("nhmko"), 'i', 2, 2);
    test(S("tpsaf"), 'i', 4, 4);
    test(S("lahfb"), 'i', 5, 4);
    test(S("irkhs"), 'i', 6, 4);
    test(S("gmfhdaipsr"), 'i', 0, 0);
    test(S("kantesmpgj"), 'i', 1, 1);
    test(S("odaftiegpm"), 'i', 5, 4);
    test(S("oknlrstdpi"), 'i', 9, 8);
    test(S("eolhfgpjqk"), 'i', 10, 9);
    test(S("pcdrofikas"), 'i', 11, 9);
    test(S("nbatdlmekrgcfqsophij"), 'i', 0, 0);
    test(S("bnrpehidofmqtcksjgla"), 'i', 1, 1);
    test(S("jdmciepkaqgotsrfnhlb"), 'i', 10, 10);
    test(S("jtdaefblsokrmhpgcnqi"), 'i', 19, 18);
    test(S("hkbgspofltajcnedqmri"), 'i', 20, 18);
    test(S("oselktgbcapndfjihrmq"), 'i', 21, 19);

    test(S(""), 'i', S::npos);
    test(S("csope"), 'i', 4);
    test(S("gfsmthlkon"), 'i', 9);
    test(S("laenfsbridchgotmkqpj"), 'i', 19);
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1;
    constexpr SV  sv2 { "abcde", 5 };

    static_assert (sv1.find_last_not_of( 'i', 0 ) == SV::npos, "" );
    static_assert (sv1.find_last_not_of( 'i', 1 ) == SV::npos, "" );
    static_assert (sv2.find_last_not_of( 'a', 0 ) == SV::npos, "" );
    static_assert (sv2.find_last_not_of( 'a', 1 ) == 1, "" );
    static_assert (sv2.find_last_not_of( 'e', 5 ) == 3, "" );
    }
#endif
}
