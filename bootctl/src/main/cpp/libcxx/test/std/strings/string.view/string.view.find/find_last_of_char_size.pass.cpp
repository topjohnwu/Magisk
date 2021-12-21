//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// constexpr size_type find_last_of(charT c, size_type pos = npos) const;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type pos,
     typename S::size_type x)
{
    assert(s.find_last_of(c, pos) == x);
    if (x != S::npos)
        assert(x <= pos && x < s.size());
}

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type x)
{
    assert(s.find_last_of(c) == x);
    if (x != S::npos)
        assert(x < s.size());
}

int main()
{
    {
    typedef std::string_view S;
    test(S(""), 'm', 0, S::npos);
    test(S(""), 'm', 1, S::npos);
    test(S("kitcj"), 'm', 0, S::npos);
    test(S("qkamf"), 'm', 1, S::npos);
    test(S("nhmko"), 'm', 2, 2);
    test(S("tpsaf"), 'm', 4, S::npos);
    test(S("lahfb"), 'm', 5, S::npos);
    test(S("irkhs"), 'm', 6, S::npos);
    test(S("gmfhdaipsr"), 'm', 0, S::npos);
    test(S("kantesmpgj"), 'm', 1, S::npos);
    test(S("odaftiegpm"), 'm', 5, S::npos);
    test(S("oknlrstdpi"), 'm', 9, S::npos);
    test(S("eolhfgpjqk"), 'm', 10, S::npos);
    test(S("pcdrofikas"), 'm', 11, S::npos);
    test(S("nbatdlmekrgcfqsophij"), 'm', 0, S::npos);
    test(S("bnrpehidofmqtcksjgla"), 'm', 1, S::npos);
    test(S("jdmciepkaqgotsrfnhlb"), 'm', 10, 2);
    test(S("jtdaefblsokrmhpgcnqi"), 'm', 19, 12);
    test(S("hkbgspofltajcnedqmri"), 'm', 20, 17);
    test(S("oselktgbcapndfjihrmq"), 'm', 21, 18);

    test(S(""), 'm', S::npos);
    test(S("csope"), 'm', S::npos);
    test(S("gfsmthlkon"), 'm', 3);
    test(S("laenfsbridchgotmkqpj"), 'm', 15);
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1;
    constexpr SV  sv2 { "abcde", 5 };

    static_assert (sv1.find_last_of( 'i', 0 ) == SV::npos, "" );
    static_assert (sv1.find_last_of( 'i', 1 ) == SV::npos, "" );
    static_assert (sv2.find_last_of( 'a', 0 ) == 0, "" );
    static_assert (sv2.find_last_of( 'a', 1 ) == 0, "" );
    static_assert (sv2.find_last_of( 'e', 5 ) == 4, "" );
    }
#endif
}
