//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string substr(size_type pos = 0, size_type n = npos) const;

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(const S& s, typename S::size_type pos, typename S::size_type n)
{
    if (pos <= s.size())
    {
        S str = s.substr(pos, n);
        LIBCPP_ASSERT(str.__invariants());
        assert(pos <= s.size());
        typename S::size_type rlen = std::min(n, s.size() - pos);
        assert(str.size() == rlen);
        assert(S::traits_type::compare(s.data()+pos, str.data(), rlen) == 0);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            S str = s.substr(pos, n);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > s.size());
        }
    }
#endif
}

int main()
{
    {
    typedef std::string S;
    test(S(""), 0, 0);
    test(S(""), 1, 0);
    test(S("pniot"), 0, 0);
    test(S("htaob"), 0, 1);
    test(S("fodgq"), 0, 2);
    test(S("hpqia"), 0, 4);
    test(S("qanej"), 0, 5);
    test(S("dfkap"), 1, 0);
    test(S("clbao"), 1, 1);
    test(S("ihqrf"), 1, 2);
    test(S("mekdn"), 1, 3);
    test(S("ngtjf"), 1, 4);
    test(S("srdfq"), 2, 0);
    test(S("qkdrs"), 2, 1);
    test(S("ikcrq"), 2, 2);
    test(S("cdaih"), 2, 3);
    test(S("dmajb"), 4, 0);
    test(S("karth"), 4, 1);
    test(S("lhcdo"), 5, 0);
    test(S("acbsj"), 6, 0);
    test(S("pbsjikaole"), 0, 0);
    test(S("pcbahntsje"), 0, 1);
    test(S("mprdjbeiak"), 0, 5);
    test(S("fhepcrntko"), 0, 9);
    test(S("eqmpaidtls"), 0, 10);
    test(S("joidhalcmq"), 1, 0);
    test(S("omigsphflj"), 1, 1);
    test(S("kocgbphfji"), 1, 4);
    test(S("onmjekafbi"), 1, 8);
    test(S("fbslrjiqkm"), 1, 9);
    test(S("oqmrjahnkg"), 5, 0);
    test(S("jeidpcmalh"), 5, 1);
    test(S("schfalibje"), 5, 2);
    test(S("crliponbqe"), 5, 4);
    test(S("igdscopqtm"), 5, 5);
    test(S("qngpdkimlc"), 9, 0);
    test(S("thdjgafrlb"), 9, 1);
    test(S("hcjitbfapl"), 10, 0);
    test(S("mgojkldsqh"), 11, 0);
    test(S("gfshlcmdjreqipbontak"), 0, 0);
    test(S("nadkhpfemgclosibtjrq"), 0, 1);
    test(S("nkodajteqplrbifhmcgs"), 0, 10);
    test(S("ofdrqmkeblthacpgijsn"), 0, 19);
    test(S("gbmetiprqdoasckjfhln"), 0, 20);
    test(S("bdfjqgatlksriohemnpc"), 1, 0);
    test(S("crnklpmegdqfiashtojb"), 1, 1);
    test(S("ejqcnahdrkfsmptilgbo"), 1, 9);
    test(S("jsbtafedocnirgpmkhql"), 1, 18);
    test(S("prqgnlbaejsmkhdctoif"), 1, 19);
    test(S("qnmodrtkebhpasifgcjl"), 10, 0);
    test(S("pejafmnokrqhtisbcdgl"), 10, 1);
    test(S("cpebqsfmnjdolhkratgi"), 10, 5);
    test(S("odnqkgijrhabfmcestlp"), 10, 9);
    test(S("lmofqdhpkibagnrcjste"), 10, 10);
    test(S("lgjqketopbfahrmnsicd"), 19, 0);
    test(S("ktsrmnqagdecfhijpobl"), 19, 1);
    test(S("lsaijeqhtrbgcdmpfkno"), 20, 0);
    test(S("dplqartnfgejichmoskb"), 21, 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), 0, 0);
    test(S(""), 1, 0);
    test(S("pniot"), 0, 0);
    test(S("htaob"), 0, 1);
    test(S("fodgq"), 0, 2);
    test(S("hpqia"), 0, 4);
    test(S("qanej"), 0, 5);
    test(S("dfkap"), 1, 0);
    test(S("clbao"), 1, 1);
    test(S("ihqrf"), 1, 2);
    test(S("mekdn"), 1, 3);
    test(S("ngtjf"), 1, 4);
    test(S("srdfq"), 2, 0);
    test(S("qkdrs"), 2, 1);
    test(S("ikcrq"), 2, 2);
    test(S("cdaih"), 2, 3);
    test(S("dmajb"), 4, 0);
    test(S("karth"), 4, 1);
    test(S("lhcdo"), 5, 0);
    test(S("acbsj"), 6, 0);
    test(S("pbsjikaole"), 0, 0);
    test(S("pcbahntsje"), 0, 1);
    test(S("mprdjbeiak"), 0, 5);
    test(S("fhepcrntko"), 0, 9);
    test(S("eqmpaidtls"), 0, 10);
    test(S("joidhalcmq"), 1, 0);
    test(S("omigsphflj"), 1, 1);
    test(S("kocgbphfji"), 1, 4);
    test(S("onmjekafbi"), 1, 8);
    test(S("fbslrjiqkm"), 1, 9);
    test(S("oqmrjahnkg"), 5, 0);
    test(S("jeidpcmalh"), 5, 1);
    test(S("schfalibje"), 5, 2);
    test(S("crliponbqe"), 5, 4);
    test(S("igdscopqtm"), 5, 5);
    test(S("qngpdkimlc"), 9, 0);
    test(S("thdjgafrlb"), 9, 1);
    test(S("hcjitbfapl"), 10, 0);
    test(S("mgojkldsqh"), 11, 0);
    test(S("gfshlcmdjreqipbontak"), 0, 0);
    test(S("nadkhpfemgclosibtjrq"), 0, 1);
    test(S("nkodajteqplrbifhmcgs"), 0, 10);
    test(S("ofdrqmkeblthacpgijsn"), 0, 19);
    test(S("gbmetiprqdoasckjfhln"), 0, 20);
    test(S("bdfjqgatlksriohemnpc"), 1, 0);
    test(S("crnklpmegdqfiashtojb"), 1, 1);
    test(S("ejqcnahdrkfsmptilgbo"), 1, 9);
    test(S("jsbtafedocnirgpmkhql"), 1, 18);
    test(S("prqgnlbaejsmkhdctoif"), 1, 19);
    test(S("qnmodrtkebhpasifgcjl"), 10, 0);
    test(S("pejafmnokrqhtisbcdgl"), 10, 1);
    test(S("cpebqsfmnjdolhkratgi"), 10, 5);
    test(S("odnqkgijrhabfmcestlp"), 10, 9);
    test(S("lmofqdhpkibagnrcjste"), 10, 10);
    test(S("lgjqketopbfahrmnsicd"), 19, 0);
    test(S("ktsrmnqagdecfhijpobl"), 19, 1);
    test(S("lsaijeqhtrbgcdmpfkno"), 20, 0);
    test(S("dplqartnfgejichmoskb"), 21, 0);
    }
#endif
}
