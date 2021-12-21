//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find_last_not_of(basic_string_view sv, size_type pos = npos) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S, class SV>
void
test(const S& s, SV sv, typename S::size_type pos, typename S::size_type x)
{
    assert(s.find_last_not_of(sv, pos) == x);
    if (x != S::npos)
        assert(x <= pos && x < s.size());
}

template <class S, class SV>
void
test(const S& s, SV sv, typename S::size_type x)
{
    assert(s.find_last_not_of(sv) == x);
    if (x != S::npos)
        assert(x < s.size());
}

template <class S, class SV>
void test0()
{
    test(S(""), SV(""), 0, S::npos);
    test(S(""), SV("laenf"), 0, S::npos);
    test(S(""), SV("pqlnkmbdjo"), 0, S::npos);
    test(S(""), SV("qkamfogpnljdcshbreti"), 0, S::npos);
    test(S(""), SV(""), 1, S::npos);
    test(S(""), SV("bjaht"), 1, S::npos);
    test(S(""), SV("hjlcmgpket"), 1, S::npos);
    test(S(""), SV("htaobedqikfplcgjsmrn"), 1, S::npos);
    test(S("fodgq"), SV(""), 0, 0);
    test(S("qanej"), SV("dfkap"), 0, 0);
    test(S("clbao"), SV("ihqrfebgad"), 0, 0);
    test(S("mekdn"), SV("ngtjfcalbseiqrphmkdo"), 0, S::npos);
    test(S("srdfq"), SV(""), 1, 1);
    test(S("oemth"), SV("ikcrq"), 1, 1);
    test(S("cdaih"), SV("dmajblfhsg"), 1, 0);
    test(S("qohtk"), SV("oqftjhdmkgsblacenirp"), 1, S::npos);
    test(S("cshmd"), SV(""), 2, 2);
    test(S("lhcdo"), SV("oebqi"), 2, 2);
    test(S("qnsoh"), SV("kojhpmbsfe"), 2, 1);
    test(S("pkrof"), SV("acbsjqogpltdkhinfrem"), 2, S::npos);
    test(S("fmtsp"), SV(""), 4, 4);
    test(S("khbpm"), SV("aobjd"), 4, 4);
    test(S("pbsji"), SV("pcbahntsje"), 4, 4);
    test(S("mprdj"), SV("fhepcrntkoagbmldqijs"), 4, S::npos);
    test(S("eqmpa"), SV(""), 5, 4);
    test(S("omigs"), SV("kocgb"), 5, 4);
    test(S("onmje"), SV("fbslrjiqkm"), 5, 4);
    test(S("oqmrj"), SV("jeidpcmalhfnqbgtrsko"), 5, S::npos);
    test(S("schfa"), SV(""), 6, 4);
    test(S("igdsc"), SV("qngpd"), 6, 4);
    test(S("brqgo"), SV("rodhqklgmb"), 6, S::npos);
    test(S("tnrph"), SV("thdjgafrlbkoiqcspmne"), 6, S::npos);
    test(S("hcjitbfapl"), SV(""), 0, 0);
    test(S("daiprenocl"), SV("ashjd"), 0, S::npos);
    test(S("litpcfdghe"), SV("mgojkldsqh"), 0, S::npos);
    test(S("aidjksrolc"), SV("imqnaghkfrdtlopbjesc"), 0, S::npos);
    test(S("qpghtfbaji"), SV(""), 1, 1);
    test(S("gfshlcmdjr"), SV("nadkh"), 1, 1);
    test(S("nkodajteqp"), SV("ofdrqmkebl"), 1, 0);
    test(S("gbmetiprqd"), SV("bdfjqgatlksriohemnpc"), 1, S::npos);
    test(S("crnklpmegd"), SV(""), 5, 5);
    test(S("jsbtafedoc"), SV("prqgn"), 5, 5);
    test(S("qnmodrtkeb"), SV("pejafmnokr"), 5, 4);
    test(S("cpebqsfmnj"), SV("odnqkgijrhabfmcestlp"), 5, S::npos);
    test(S("lmofqdhpki"), SV(""), 9, 9);
    test(S("hnefkqimca"), SV("rtjpa"), 9, 8);
    test(S("drtasbgmfp"), SV("ktsrmnqagd"), 9, 9);
    test(S("lsaijeqhtr"), SV("rtdhgcisbnmoaqkfpjle"), 9, S::npos);
    test(S("elgofjmbrq"), SV(""), 10, 9);
    test(S("mjqdgalkpc"), SV("dplqa"), 10, 9);
    test(S("kthqnfcerm"), SV("dkacjoptns"), 10, 9);
    test(S("dfsjhanorc"), SV("hqfimtrgnbekpdcsjalo"), 10, S::npos);
    test(S("eqsgalomhb"), SV(""), 11, 9);
    test(S("akiteljmoh"), SV("lofbc"), 11, 9);
    test(S("hlbdfreqjo"), SV("astoegbfpn"), 11, 8);
    test(S("taqobhlerg"), SV("pdgreqomsncafklhtibj"), 11, S::npos);
    test(S("snafbdlghrjkpqtoceim"), SV(""), 0, 0);
    test(S("aemtbrgcklhndjisfpoq"), SV("lbtqd"), 0, 0);
    test(S("pnracgfkjdiholtbqsem"), SV("tboimldpjh"), 0, S::npos);
    test(S("dicfltehbsgrmojnpkaq"), SV("slcerthdaiqjfnobgkpm"), 0, S::npos);
    test(S("jlnkraeodhcspfgbqitm"), SV(""), 1, 1);
    test(S("lhosrngtmfjikbqpcade"), SV("aqibs"), 1, 1);
    test(S("rbtaqjhgkneisldpmfoc"), SV("gtfblmqinc"), 1, 0);
    test(S("gpifsqlrdkbonjtmheca"), SV("mkqpbtdalgniorhfescj"), 1, S::npos);
    test(S("hdpkobnsalmcfijregtq"), SV(""), 10, 10);
    test(S("jtlshdgqaiprkbcoenfm"), SV("pblas"), 10, 9);
    test(S("fkdrbqltsgmcoiphneaj"), SV("arosdhcfme"), 10, 9);
    test(S("crsplifgtqedjohnabmk"), SV("blkhjeogicatqfnpdmsr"), 10, S::npos);
    test(S("niptglfbosehkamrdqcj"), SV(""), 19, 19);
    test(S("copqdhstbingamjfkler"), SV("djkqc"), 19, 19);
    test(S("mrtaefilpdsgocnhqbjk"), SV("lgokshjtpb"), 19, 16);
    test(S("kojatdhlcmigpbfrqnes"), SV("bqjhtkfepimcnsgrlado"), 19, S::npos);
    test(S("eaintpchlqsbdgrkjofm"), SV(""), 20, 19);
    test(S("gjnhidfsepkrtaqbmclo"), SV("nocfa"), 20, 18);
    test(S("spocfaktqdbiejlhngmr"), SV("bgtajmiedc"), 20, 19);
    test(S("rphmlekgfscndtaobiqj"), SV("lsckfnqgdahejiopbtmr"), 20, S::npos);
    test(S("liatsqdoegkmfcnbhrpj"), SV(""), 21, 19);
    test(S("binjagtfldkrspcomqeh"), SV("gfsrt"), 21, 19);
    test(S("latkmisecnorjbfhqpdg"), SV("pfsocbhjtm"), 21, 19);
    test(S("lecfratdjkhnsmqpoigb"), SV("tpflmdnoicjgkberhqsa"), 21, S::npos);
}

template <class S, class SV>
void test1()
{
    test(S(""), SV(""), S::npos);
    test(S(""), SV("laenf"), S::npos);
    test(S(""), SV("pqlnkmbdjo"), S::npos);
    test(S(""), SV("qkamfogpnljdcshbreti"), S::npos);
    test(S("nhmko"), SV(""), 4);
    test(S("lahfb"), SV("irkhs"), 4);
    test(S("gmfhd"), SV("kantesmpgj"), 4);
    test(S("odaft"), SV("oknlrstdpiqmjbaghcfe"), S::npos);
    test(S("eolhfgpjqk"), SV(""), 9);
    test(S("nbatdlmekr"), SV("bnrpe"), 8);
    test(S("jdmciepkaq"), SV("jtdaefblso"), 9);
    test(S("hkbgspoflt"), SV("oselktgbcapndfjihrmq"), S::npos);
    test(S("gprdcokbnjhlsfmtieqa"), SV(""), 19);
    test(S("qjghlnftcaismkropdeb"), SV("bjaht"), 18);
    test(S("pnalfrdtkqcmojiesbhg"), SV("hjlcmgpket"), 17);
    test(S("pniotcfrhqsmgdkjbael"), SV("htaobedqikfplcgjsmrn"), S::npos);
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test0<S, SV>();
    test1<S, SV>();
    }
#if TEST_STD_VER >= 11
    {
//     typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
//     typedef std::string_view SV;
//     test0<S, SV>();
//     test1<S, SV>();
    }
#endif
}
