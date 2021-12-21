//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// size_type find_last_not_of(const basic_string& str, size_type pos = npos) const;

#include <string_view>
#include <cassert>

template <class S>
void
test(const S& s, const S& str, typename S::size_type pos, typename S::size_type x)
{
    assert(s.find_last_not_of(str, pos) == x);
    if (x != S::npos)
        assert(x <= pos && x < s.size());
}

template <class S>
void
test(const S& s, const S& str, typename S::size_type x)
{
    assert(s.find_last_not_of(str) == x);
    if (x != S::npos)
        assert(x < s.size());
}

template <class S>
void test0()
{
    test(S(""), S(""), 0, S::npos);
    test(S(""), S("laenf"), 0, S::npos);
    test(S(""), S("pqlnkmbdjo"), 0, S::npos);
    test(S(""), S("qkamfogpnljdcshbreti"), 0, S::npos);
    test(S(""), S(""), 1, S::npos);
    test(S(""), S("bjaht"), 1, S::npos);
    test(S(""), S("hjlcmgpket"), 1, S::npos);
    test(S(""), S("htaobedqikfplcgjsmrn"), 1, S::npos);
    test(S("fodgq"), S(""), 0, 0);
    test(S("qanej"), S("dfkap"), 0, 0);
    test(S("clbao"), S("ihqrfebgad"), 0, 0);
    test(S("mekdn"), S("ngtjfcalbseiqrphmkdo"), 0, S::npos);
    test(S("srdfq"), S(""), 1, 1);
    test(S("oemth"), S("ikcrq"), 1, 1);
    test(S("cdaih"), S("dmajblfhsg"), 1, 0);
    test(S("qohtk"), S("oqftjhdmkgsblacenirp"), 1, S::npos);
    test(S("cshmd"), S(""), 2, 2);
    test(S("lhcdo"), S("oebqi"), 2, 2);
    test(S("qnsoh"), S("kojhpmbsfe"), 2, 1);
    test(S("pkrof"), S("acbsjqogpltdkhinfrem"), 2, S::npos);
    test(S("fmtsp"), S(""), 4, 4);
    test(S("khbpm"), S("aobjd"), 4, 4);
    test(S("pbsji"), S("pcbahntsje"), 4, 4);
    test(S("mprdj"), S("fhepcrntkoagbmldqijs"), 4, S::npos);
    test(S("eqmpa"), S(""), 5, 4);
    test(S("omigs"), S("kocgb"), 5, 4);
    test(S("onmje"), S("fbslrjiqkm"), 5, 4);
    test(S("oqmrj"), S("jeidpcmalhfnqbgtrsko"), 5, S::npos);
    test(S("schfa"), S(""), 6, 4);
    test(S("igdsc"), S("qngpd"), 6, 4);
    test(S("brqgo"), S("rodhqklgmb"), 6, S::npos);
    test(S("tnrph"), S("thdjgafrlbkoiqcspmne"), 6, S::npos);
    test(S("hcjitbfapl"), S(""), 0, 0);
    test(S("daiprenocl"), S("ashjd"), 0, S::npos);
    test(S("litpcfdghe"), S("mgojkldsqh"), 0, S::npos);
    test(S("aidjksrolc"), S("imqnaghkfrdtlopbjesc"), 0, S::npos);
    test(S("qpghtfbaji"), S(""), 1, 1);
    test(S("gfshlcmdjr"), S("nadkh"), 1, 1);
    test(S("nkodajteqp"), S("ofdrqmkebl"), 1, 0);
    test(S("gbmetiprqd"), S("bdfjqgatlksriohemnpc"), 1, S::npos);
    test(S("crnklpmegd"), S(""), 5, 5);
    test(S("jsbtafedoc"), S("prqgn"), 5, 5);
    test(S("qnmodrtkeb"), S("pejafmnokr"), 5, 4);
    test(S("cpebqsfmnj"), S("odnqkgijrhabfmcestlp"), 5, S::npos);
    test(S("lmofqdhpki"), S(""), 9, 9);
    test(S("hnefkqimca"), S("rtjpa"), 9, 8);
    test(S("drtasbgmfp"), S("ktsrmnqagd"), 9, 9);
    test(S("lsaijeqhtr"), S("rtdhgcisbnmoaqkfpjle"), 9, S::npos);
    test(S("elgofjmbrq"), S(""), 10, 9);
    test(S("mjqdgalkpc"), S("dplqa"), 10, 9);
    test(S("kthqnfcerm"), S("dkacjoptns"), 10, 9);
    test(S("dfsjhanorc"), S("hqfimtrgnbekpdcsjalo"), 10, S::npos);
    test(S("eqsgalomhb"), S(""), 11, 9);
    test(S("akiteljmoh"), S("lofbc"), 11, 9);
    test(S("hlbdfreqjo"), S("astoegbfpn"), 11, 8);
    test(S("taqobhlerg"), S("pdgreqomsncafklhtibj"), 11, S::npos);
    test(S("snafbdlghrjkpqtoceim"), S(""), 0, 0);
    test(S("aemtbrgcklhndjisfpoq"), S("lbtqd"), 0, 0);
    test(S("pnracgfkjdiholtbqsem"), S("tboimldpjh"), 0, S::npos);
    test(S("dicfltehbsgrmojnpkaq"), S("slcerthdaiqjfnobgkpm"), 0, S::npos);
    test(S("jlnkraeodhcspfgbqitm"), S(""), 1, 1);
    test(S("lhosrngtmfjikbqpcade"), S("aqibs"), 1, 1);
    test(S("rbtaqjhgkneisldpmfoc"), S("gtfblmqinc"), 1, 0);
    test(S("gpifsqlrdkbonjtmheca"), S("mkqpbtdalgniorhfescj"), 1, S::npos);
    test(S("hdpkobnsalmcfijregtq"), S(""), 10, 10);
    test(S("jtlshdgqaiprkbcoenfm"), S("pblas"), 10, 9);
    test(S("fkdrbqltsgmcoiphneaj"), S("arosdhcfme"), 10, 9);
    test(S("crsplifgtqedjohnabmk"), S("blkhjeogicatqfnpdmsr"), 10, S::npos);
    test(S("niptglfbosehkamrdqcj"), S(""), 19, 19);
    test(S("copqdhstbingamjfkler"), S("djkqc"), 19, 19);
    test(S("mrtaefilpdsgocnhqbjk"), S("lgokshjtpb"), 19, 16);
    test(S("kojatdhlcmigpbfrqnes"), S("bqjhtkfepimcnsgrlado"), 19, S::npos);
    test(S("eaintpchlqsbdgrkjofm"), S(""), 20, 19);
    test(S("gjnhidfsepkrtaqbmclo"), S("nocfa"), 20, 18);
    test(S("spocfaktqdbiejlhngmr"), S("bgtajmiedc"), 20, 19);
    test(S("rphmlekgfscndtaobiqj"), S("lsckfnqgdahejiopbtmr"), 20, S::npos);
    test(S("liatsqdoegkmfcnbhrpj"), S(""), 21, 19);
    test(S("binjagtfldkrspcomqeh"), S("gfsrt"), 21, 19);
    test(S("latkmisecnorjbfhqpdg"), S("pfsocbhjtm"), 21, 19);
    test(S("lecfratdjkhnsmqpoigb"), S("tpflmdnoicjgkberhqsa"), 21, S::npos);
}

template <class S>
void test1()
{
    test(S(""), S(""), S::npos);
    test(S(""), S("laenf"), S::npos);
    test(S(""), S("pqlnkmbdjo"), S::npos);
    test(S(""), S("qkamfogpnljdcshbreti"), S::npos);
    test(S("nhmko"), S(""), 4);
    test(S("lahfb"), S("irkhs"), 4);
    test(S("gmfhd"), S("kantesmpgj"), 4);
    test(S("odaft"), S("oknlrstdpiqmjbaghcfe"), S::npos);
    test(S("eolhfgpjqk"), S(""), 9);
    test(S("nbatdlmekr"), S("bnrpe"), 8);
    test(S("jdmciepkaq"), S("jtdaefblso"), 9);
    test(S("hkbgspoflt"), S("oselktgbcapndfjihrmq"), S::npos);
    test(S("gprdcokbnjhlsfmtieqa"), S(""), 19);
    test(S("qjghlnftcaismkropdeb"), S("bjaht"), 18);
    test(S("pnalfrdtkqcmojiesbhg"), S("hjlcmgpket"), 17);
    test(S("pniotcfrhqsmgdkjbael"), S("htaobedqikfplcgjsmrn"), S::npos);
}

int main()
{
    {
    typedef std::string_view S;
    test0<S>();
    test1<S>();
    }
}
