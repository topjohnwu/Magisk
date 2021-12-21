//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find_first_of(charT c, size_type pos = 0) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type pos,
     typename S::size_type x)
{
    assert(s.find_first_of(c, pos) == x);
    if (x != S::npos)
        assert(pos <= x && x < s.size());
}

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type x)
{
    assert(s.find_first_of(c) == x);
    if (x != S::npos)
        assert(x < s.size());
}

int main()
{
    {
    typedef std::string S;
    test(S(""), 'e', 0, S::npos);
    test(S(""), 'e', 1, S::npos);
    test(S("kitcj"), 'e', 0, S::npos);
    test(S("qkamf"), 'e', 1, S::npos);
    test(S("nhmko"), 'e', 2, S::npos);
    test(S("tpsaf"), 'e', 4, S::npos);
    test(S("lahfb"), 'e', 5, S::npos);
    test(S("irkhs"), 'e', 6, S::npos);
    test(S("gmfhdaipsr"), 'e', 0, S::npos);
    test(S("kantesmpgj"), 'e', 1, 4);
    test(S("odaftiegpm"), 'e', 5, 6);
    test(S("oknlrstdpi"), 'e', 9, S::npos);
    test(S("eolhfgpjqk"), 'e', 10, S::npos);
    test(S("pcdrofikas"), 'e', 11, S::npos);
    test(S("nbatdlmekrgcfqsophij"), 'e', 0, 7);
    test(S("bnrpehidofmqtcksjgla"), 'e', 1, 4);
    test(S("jdmciepkaqgotsrfnhlb"), 'e', 10, S::npos);
    test(S("jtdaefblsokrmhpgcnqi"), 'e', 19, S::npos);
    test(S("hkbgspofltajcnedqmri"), 'e', 20, S::npos);
    test(S("oselktgbcapndfjihrmq"), 'e', 21, S::npos);

    test(S(""), 'e', S::npos);
    test(S("csope"), 'e', 4);
    test(S("gfsmthlkon"), 'e', S::npos);
    test(S("laenfsbridchgotmkqpj"), 'e', 2);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), 'e', 0, S::npos);
    test(S(""), 'e', 1, S::npos);
    test(S("kitcj"), 'e', 0, S::npos);
    test(S("qkamf"), 'e', 1, S::npos);
    test(S("nhmko"), 'e', 2, S::npos);
    test(S("tpsaf"), 'e', 4, S::npos);
    test(S("lahfb"), 'e', 5, S::npos);
    test(S("irkhs"), 'e', 6, S::npos);
    test(S("gmfhdaipsr"), 'e', 0, S::npos);
    test(S("kantesmpgj"), 'e', 1, 4);
    test(S("odaftiegpm"), 'e', 5, 6);
    test(S("oknlrstdpi"), 'e', 9, S::npos);
    test(S("eolhfgpjqk"), 'e', 10, S::npos);
    test(S("pcdrofikas"), 'e', 11, S::npos);
    test(S("nbatdlmekrgcfqsophij"), 'e', 0, 7);
    test(S("bnrpehidofmqtcksjgla"), 'e', 1, 4);
    test(S("jdmciepkaqgotsrfnhlb"), 'e', 10, S::npos);
    test(S("jtdaefblsokrmhpgcnqi"), 'e', 19, S::npos);
    test(S("hkbgspofltajcnedqmri"), 'e', 20, S::npos);
    test(S("oselktgbcapndfjihrmq"), 'e', 21, S::npos);

    test(S(""), 'e', S::npos);
    test(S("csope"), 'e', 4);
    test(S("gfsmthlkon"), 'e', S::npos);
    test(S("laenfsbridchgotmkqpj"), 'e', 2);
    }
#endif
}
