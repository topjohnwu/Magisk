//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find_last_not_of(charT c, size_type pos = npos) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

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
    typedef std::string S;
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
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
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
#endif
}
