//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find_first_not_of(basic_string_view sv, size_type pos = 0) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S, class SV>
void
test(const S& s, SV sv, typename S::size_type pos, typename S::size_type x)
{
    assert(s.find_first_not_of(sv, pos) == x);
    if (x != S::npos)
        assert(pos <= x && x < s.size());
}

template <class S, class SV>
void
test(const S& s, SV sv, typename S::size_type x)
{
    assert(s.find_first_not_of(sv) == x);
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
    test(S("cdaih"), SV("dmajblfhsg"), 1, 3);
    test(S("qohtk"), SV("oqftjhdmkgsblacenirp"), 1, S::npos);
    test(S("cshmd"), SV(""), 2, 2);
    test(S("lhcdo"), SV("oebqi"), 2, 2);
    test(S("qnsoh"), SV("kojhpmbsfe"), 2, S::npos);
    test(S("pkrof"), SV("acbsjqogpltdkhinfrem"), 2, S::npos);
    test(S("fmtsp"), SV(""), 4, 4);
    test(S("khbpm"), SV("aobjd"), 4, 4);
    test(S("pbsji"), SV("pcbahntsje"), 4, 4);
    test(S("mprdj"), SV("fhepcrntkoagbmldqijs"), 4, S::npos);
    test(S("eqmpa"), SV(""), 5, S::npos);
    test(S("omigs"), SV("kocgb"), 5, S::npos);
    test(S("onmje"), SV("fbslrjiqkm"), 5, S::npos);
    test(S("oqmrj"), SV("jeidpcmalhfnqbgtrsko"), 5, S::npos);
    test(S("schfa"), SV(""), 6, S::npos);
    test(S("igdsc"), SV("qngpd"), 6, S::npos);
    test(S("brqgo"), SV("rodhqklgmb"), 6, S::npos);
    test(S("tnrph"), SV("thdjgafrlbkoiqcspmne"), 6, S::npos);
    test(S("hcjitbfapl"), SV(""), 0, 0);
    test(S("daiprenocl"), SV("ashjd"), 0, 2);
    test(S("litpcfdghe"), SV("mgojkldsqh"), 0, 1);
    test(S("aidjksrolc"), SV("imqnaghkfrdtlopbjesc"), 0, S::npos);
    test(S("qpghtfbaji"), SV(""), 1, 1);
    test(S("gfshlcmdjr"), SV("nadkh"), 1, 1);
    test(S("nkodajteqp"), SV("ofdrqmkebl"), 1, 4);
    test(S("gbmetiprqd"), SV("bdfjqgatlksriohemnpc"), 1, S::npos);
    test(S("crnklpmegd"), SV(""), 5, 5);
    test(S("jsbtafedoc"), SV("prqgn"), 5, 5);
    test(S("qnmodrtkeb"), SV("pejafmnokr"), 5, 6);
    test(S("cpebqsfmnj"), SV("odnqkgijrhabfmcestlp"), 5, S::npos);
    test(S("lmofqdhpki"), SV(""), 9, 9);
    test(S("hnefkqimca"), SV("rtjpa"), 9, S::npos);
    test(S("drtasbgmfp"), SV("ktsrmnqagd"), 9, 9);
    test(S("lsaijeqhtr"), SV("rtdhgcisbnmoaqkfpjle"), 9, S::npos);
    test(S("elgofjmbrq"), SV(""), 10, S::npos);
    test(S("mjqdgalkpc"), SV("dplqa"), 10, S::npos);
    test(S("kthqnfcerm"), SV("dkacjoptns"), 10, S::npos);
    test(S("dfsjhanorc"), SV("hqfimtrgnbekpdcsjalo"), 10, S::npos);
    test(S("eqsgalomhb"), SV(""), 11, S::npos);
    test(S("akiteljmoh"), SV("lofbc"), 11, S::npos);
    test(S("hlbdfreqjo"), SV("astoegbfpn"), 11, S::npos);
    test(S("taqobhlerg"), SV("pdgreqomsncafklhtibj"), 11, S::npos);
    test(S("snafbdlghrjkpqtoceim"), SV(""), 0, 0);
    test(S("aemtbrgcklhndjisfpoq"), SV("lbtqd"), 0, 0);
    test(S("pnracgfkjdiholtbqsem"), SV("tboimldpjh"), 0, 1);
    test(S("dicfltehbsgrmojnpkaq"), SV("slcerthdaiqjfnobgkpm"), 0, S::npos);
    test(S("jlnkraeodhcspfgbqitm"), SV(""), 1, 1);
    test(S("lhosrngtmfjikbqpcade"), SV("aqibs"), 1, 1);
    test(S("rbtaqjhgkneisldpmfoc"), SV("gtfblmqinc"), 1, 3);
    test(S("gpifsqlrdkbonjtmheca"), SV("mkqpbtdalgniorhfescj"), 1, S::npos);
    test(S("hdpkobnsalmcfijregtq"), SV(""), 10, 10);
    test(S("jtlshdgqaiprkbcoenfm"), SV("pblas"), 10, 11);
    test(S("fkdrbqltsgmcoiphneaj"), SV("arosdhcfme"), 10, 13);
    test(S("crsplifgtqedjohnabmk"), SV("blkhjeogicatqfnpdmsr"), 10, S::npos);
    test(S("niptglfbosehkamrdqcj"), SV(""), 19, 19);
    test(S("copqdhstbingamjfkler"), SV("djkqc"), 19, 19);
    test(S("mrtaefilpdsgocnhqbjk"), SV("lgokshjtpb"), 19, S::npos);
    test(S("kojatdhlcmigpbfrqnes"), SV("bqjhtkfepimcnsgrlado"), 19, S::npos);
    test(S("eaintpchlqsbdgrkjofm"), SV(""), 20, S::npos);
    test(S("gjnhidfsepkrtaqbmclo"), SV("nocfa"), 20, S::npos);
    test(S("spocfaktqdbiejlhngmr"), SV("bgtajmiedc"), 20, S::npos);
    test(S("rphmlekgfscndtaobiqj"), SV("lsckfnqgdahejiopbtmr"), 20, S::npos);
    test(S("liatsqdoegkmfcnbhrpj"), SV(""), 21, S::npos);
    test(S("binjagtfldkrspcomqeh"), SV("gfsrt"), 21, S::npos);
    test(S("latkmisecnorjbfhqpdg"), SV("pfsocbhjtm"), 21, S::npos);
    test(S("lecfratdjkhnsmqpoigb"), SV("tpflmdnoicjgkberhqsa"), 21, S::npos);
}

template <class S, class SV>
void test1()
{
    test(S(""), SV(""), S::npos);
    test(S(""), SV("laenf"), S::npos);
    test(S(""), SV("pqlnkmbdjo"), S::npos);
    test(S(""), SV("qkamfogpnljdcshbreti"), S::npos);
    test(S("nhmko"), SV(""), 0);
    test(S("lahfb"), SV("irkhs"), 0);
    test(S("gmfhd"), SV("kantesmpgj"), 2);
    test(S("odaft"), SV("oknlrstdpiqmjbaghcfe"), S::npos);
    test(S("eolhfgpjqk"), SV(""), 0);
    test(S("nbatdlmekr"), SV("bnrpe"), 2);
    test(S("jdmciepkaq"), SV("jtdaefblso"), 2);
    test(S("hkbgspoflt"), SV("oselktgbcapndfjihrmq"), S::npos);
    test(S("gprdcokbnjhlsfmtieqa"), SV(""), 0);
    test(S("qjghlnftcaismkropdeb"), SV("bjaht"), 0);
    test(S("pnalfrdtkqcmojiesbhg"), SV("hjlcmgpket"), 1);
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
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::string_view SV;
    test0<S, SV>();
    test1<S, SV>();
    }
#endif
}
