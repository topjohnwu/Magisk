//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// size_type find_last_of(const basic_string& str, size_type pos = npos) const;

#include <string_view>
#include <cassert>

template <class S>
void
test(const S& s, const S& str, typename S::size_type pos, typename S::size_type x)
{
    assert(s.find_last_of(str, pos) == x);
    if (x != S::npos)
        assert(x <= pos && x < s.size());
}

template <class S>
void
test(const S& s, const S& str, typename S::size_type x)
{
    assert(s.find_last_of(str) == x);
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
    test(S("fodgq"), S(""), 0, S::npos);
    test(S("qanej"), S("dfkap"), 0, S::npos);
    test(S("clbao"), S("ihqrfebgad"), 0, S::npos);
    test(S("mekdn"), S("ngtjfcalbseiqrphmkdo"), 0, 0);
    test(S("srdfq"), S(""), 1, S::npos);
    test(S("oemth"), S("ikcrq"), 1, S::npos);
    test(S("cdaih"), S("dmajblfhsg"), 1, 1);
    test(S("qohtk"), S("oqftjhdmkgsblacenirp"), 1, 1);
    test(S("cshmd"), S(""), 2, S::npos);
    test(S("lhcdo"), S("oebqi"), 2, S::npos);
    test(S("qnsoh"), S("kojhpmbsfe"), 2, 2);
    test(S("pkrof"), S("acbsjqogpltdkhinfrem"), 2, 2);
    test(S("fmtsp"), S(""), 4, S::npos);
    test(S("khbpm"), S("aobjd"), 4, 2);
    test(S("pbsji"), S("pcbahntsje"), 4, 3);
    test(S("mprdj"), S("fhepcrntkoagbmldqijs"), 4, 4);
    test(S("eqmpa"), S(""), 5, S::npos);
    test(S("omigs"), S("kocgb"), 5, 3);
    test(S("onmje"), S("fbslrjiqkm"), 5, 3);
    test(S("oqmrj"), S("jeidpcmalhfnqbgtrsko"), 5, 4);
    test(S("schfa"), S(""), 6, S::npos);
    test(S("igdsc"), S("qngpd"), 6, 2);
    test(S("brqgo"), S("rodhqklgmb"), 6, 4);
    test(S("tnrph"), S("thdjgafrlbkoiqcspmne"), 6, 4);
    test(S("hcjitbfapl"), S(""), 0, S::npos);
    test(S("daiprenocl"), S("ashjd"), 0, 0);
    test(S("litpcfdghe"), S("mgojkldsqh"), 0, 0);
    test(S("aidjksrolc"), S("imqnaghkfrdtlopbjesc"), 0, 0);
    test(S("qpghtfbaji"), S(""), 1, S::npos);
    test(S("gfshlcmdjr"), S("nadkh"), 1, S::npos);
    test(S("nkodajteqp"), S("ofdrqmkebl"), 1, 1);
    test(S("gbmetiprqd"), S("bdfjqgatlksriohemnpc"), 1, 1);
    test(S("crnklpmegd"), S(""), 5, S::npos);
    test(S("jsbtafedoc"), S("prqgn"), 5, S::npos);
    test(S("qnmodrtkeb"), S("pejafmnokr"), 5, 5);
    test(S("cpebqsfmnj"), S("odnqkgijrhabfmcestlp"), 5, 5);
    test(S("lmofqdhpki"), S(""), 9, S::npos);
    test(S("hnefkqimca"), S("rtjpa"), 9, 9);
    test(S("drtasbgmfp"), S("ktsrmnqagd"), 9, 7);
    test(S("lsaijeqhtr"), S("rtdhgcisbnmoaqkfpjle"), 9, 9);
    test(S("elgofjmbrq"), S(""), 10, S::npos);
    test(S("mjqdgalkpc"), S("dplqa"), 10, 8);
    test(S("kthqnfcerm"), S("dkacjoptns"), 10, 6);
    test(S("dfsjhanorc"), S("hqfimtrgnbekpdcsjalo"), 10, 9);
    test(S("eqsgalomhb"), S(""), 11, S::npos);
    test(S("akiteljmoh"), S("lofbc"), 11, 8);
    test(S("hlbdfreqjo"), S("astoegbfpn"), 11, 9);
    test(S("taqobhlerg"), S("pdgreqomsncafklhtibj"), 11, 9);
    test(S("snafbdlghrjkpqtoceim"), S(""), 0, S::npos);
    test(S("aemtbrgcklhndjisfpoq"), S("lbtqd"), 0, S::npos);
    test(S("pnracgfkjdiholtbqsem"), S("tboimldpjh"), 0, 0);
    test(S("dicfltehbsgrmojnpkaq"), S("slcerthdaiqjfnobgkpm"), 0, 0);
    test(S("jlnkraeodhcspfgbqitm"), S(""), 1, S::npos);
    test(S("lhosrngtmfjikbqpcade"), S("aqibs"), 1, S::npos);
    test(S("rbtaqjhgkneisldpmfoc"), S("gtfblmqinc"), 1, 1);
    test(S("gpifsqlrdkbonjtmheca"), S("mkqpbtdalgniorhfescj"), 1, 1);
    test(S("hdpkobnsalmcfijregtq"), S(""), 10, S::npos);
    test(S("jtlshdgqaiprkbcoenfm"), S("pblas"), 10, 10);
    test(S("fkdrbqltsgmcoiphneaj"), S("arosdhcfme"), 10, 10);
    test(S("crsplifgtqedjohnabmk"), S("blkhjeogicatqfnpdmsr"), 10, 10);
    test(S("niptglfbosehkamrdqcj"), S(""), 19, S::npos);
    test(S("copqdhstbingamjfkler"), S("djkqc"), 19, 16);
    test(S("mrtaefilpdsgocnhqbjk"), S("lgokshjtpb"), 19, 19);
    test(S("kojatdhlcmigpbfrqnes"), S("bqjhtkfepimcnsgrlado"), 19, 19);
    test(S("eaintpchlqsbdgrkjofm"), S(""), 20, S::npos);
    test(S("gjnhidfsepkrtaqbmclo"), S("nocfa"), 20, 19);
    test(S("spocfaktqdbiejlhngmr"), S("bgtajmiedc"), 20, 18);
    test(S("rphmlekgfscndtaobiqj"), S("lsckfnqgdahejiopbtmr"), 20, 19);
    test(S("liatsqdoegkmfcnbhrpj"), S(""), 21, S::npos);
    test(S("binjagtfldkrspcomqeh"), S("gfsrt"), 21, 12);
    test(S("latkmisecnorjbfhqpdg"), S("pfsocbhjtm"), 21, 17);
    test(S("lecfratdjkhnsmqpoigb"), S("tpflmdnoicjgkberhqsa"), 21, 19);
}

template <class S>
void test1()
{
    test(S(""), S(""), S::npos);
    test(S(""), S("laenf"), S::npos);
    test(S(""), S("pqlnkmbdjo"), S::npos);
    test(S(""), S("qkamfogpnljdcshbreti"), S::npos);
    test(S("nhmko"), S(""), S::npos);
    test(S("lahfb"), S("irkhs"), 2);
    test(S("gmfhd"), S("kantesmpgj"), 1);
    test(S("odaft"), S("oknlrstdpiqmjbaghcfe"), 4);
    test(S("eolhfgpjqk"), S(""), S::npos);
    test(S("nbatdlmekr"), S("bnrpe"), 9);
    test(S("jdmciepkaq"), S("jtdaefblso"), 8);
    test(S("hkbgspoflt"), S("oselktgbcapndfjihrmq"), 9);
    test(S("gprdcokbnjhlsfmtieqa"), S(""), S::npos);
    test(S("qjghlnftcaismkropdeb"), S("bjaht"), 19);
    test(S("pnalfrdtkqcmojiesbhg"), S("hjlcmgpket"), 19);
    test(S("pniotcfrhqsmgdkjbael"), S("htaobedqikfplcgjsmrn"), 19);
}

int main()
{
    {
    typedef std::string_view S;
    test0<S>();
    test1<S>();
    }
}
