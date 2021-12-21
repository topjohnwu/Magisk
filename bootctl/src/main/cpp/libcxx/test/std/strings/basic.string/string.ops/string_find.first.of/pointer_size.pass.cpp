//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find_first_of(const charT* s, size_type pos = 0) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s, const typename S::value_type* str, typename S::size_type pos,
     typename S::size_type x)
{
    assert(s.find_first_of(str, pos) == x);
    if (x != S::npos)
        assert(pos <= x && x < s.size());
}

template <class S>
void
test(const S& s, const typename S::value_type* str, typename S::size_type x)
{
    assert(s.find_first_of(str) == x);
    if (x != S::npos)
        assert(x < s.size());
}

template <class S>
void test0()
{
    test(S(""), "", 0, S::npos);
    test(S(""), "laenf", 0, S::npos);
    test(S(""), "pqlnkmbdjo", 0, S::npos);
    test(S(""), "qkamfogpnljdcshbreti", 0, S::npos);
    test(S(""), "", 1, S::npos);
    test(S(""), "bjaht", 1, S::npos);
    test(S(""), "hjlcmgpket", 1, S::npos);
    test(S(""), "htaobedqikfplcgjsmrn", 1, S::npos);
    test(S("fodgq"), "", 0, S::npos);
    test(S("qanej"), "dfkap", 0, 1);
    test(S("clbao"), "ihqrfebgad", 0, 2);
    test(S("mekdn"), "ngtjfcalbseiqrphmkdo", 0, 0);
    test(S("srdfq"), "", 1, S::npos);
    test(S("oemth"), "ikcrq", 1, S::npos);
    test(S("cdaih"), "dmajblfhsg", 1, 1);
    test(S("qohtk"), "oqftjhdmkgsblacenirp", 1, 1);
    test(S("cshmd"), "", 2, S::npos);
    test(S("lhcdo"), "oebqi", 2, 4);
    test(S("qnsoh"), "kojhpmbsfe", 2, 2);
    test(S("pkrof"), "acbsjqogpltdkhinfrem", 2, 2);
    test(S("fmtsp"), "", 4, S::npos);
    test(S("khbpm"), "aobjd", 4, S::npos);
    test(S("pbsji"), "pcbahntsje", 4, S::npos);
    test(S("mprdj"), "fhepcrntkoagbmldqijs", 4, 4);
    test(S("eqmpa"), "", 5, S::npos);
    test(S("omigs"), "kocgb", 5, S::npos);
    test(S("onmje"), "fbslrjiqkm", 5, S::npos);
    test(S("oqmrj"), "jeidpcmalhfnqbgtrsko", 5, S::npos);
    test(S("schfa"), "", 6, S::npos);
    test(S("igdsc"), "qngpd", 6, S::npos);
    test(S("brqgo"), "rodhqklgmb", 6, S::npos);
    test(S("tnrph"), "thdjgafrlbkoiqcspmne", 6, S::npos);
    test(S("hcjitbfapl"), "", 0, S::npos);
    test(S("daiprenocl"), "ashjd", 0, 0);
    test(S("litpcfdghe"), "mgojkldsqh", 0, 0);
    test(S("aidjksrolc"), "imqnaghkfrdtlopbjesc", 0, 0);
    test(S("qpghtfbaji"), "", 1, S::npos);
    test(S("gfshlcmdjr"), "nadkh", 1, 3);
    test(S("nkodajteqp"), "ofdrqmkebl", 1, 1);
    test(S("gbmetiprqd"), "bdfjqgatlksriohemnpc", 1, 1);
    test(S("crnklpmegd"), "", 5, S::npos);
    test(S("jsbtafedoc"), "prqgn", 5, S::npos);
    test(S("qnmodrtkeb"), "pejafmnokr", 5, 5);
    test(S("cpebqsfmnj"), "odnqkgijrhabfmcestlp", 5, 5);
    test(S("lmofqdhpki"), "", 9, S::npos);
    test(S("hnefkqimca"), "rtjpa", 9, 9);
    test(S("drtasbgmfp"), "ktsrmnqagd", 9, S::npos);
    test(S("lsaijeqhtr"), "rtdhgcisbnmoaqkfpjle", 9, 9);
    test(S("elgofjmbrq"), "", 10, S::npos);
    test(S("mjqdgalkpc"), "dplqa", 10, S::npos);
    test(S("kthqnfcerm"), "dkacjoptns", 10, S::npos);
    test(S("dfsjhanorc"), "hqfimtrgnbekpdcsjalo", 10, S::npos);
    test(S("eqsgalomhb"), "", 11, S::npos);
    test(S("akiteljmoh"), "lofbc", 11, S::npos);
    test(S("hlbdfreqjo"), "astoegbfpn", 11, S::npos);
    test(S("taqobhlerg"), "pdgreqomsncafklhtibj", 11, S::npos);
    test(S("snafbdlghrjkpqtoceim"), "", 0, S::npos);
    test(S("aemtbrgcklhndjisfpoq"), "lbtqd", 0, 3);
    test(S("pnracgfkjdiholtbqsem"), "tboimldpjh", 0, 0);
    test(S("dicfltehbsgrmojnpkaq"), "slcerthdaiqjfnobgkpm", 0, 0);
    test(S("jlnkraeodhcspfgbqitm"), "", 1, S::npos);
    test(S("lhosrngtmfjikbqpcade"), "aqibs", 1, 3);
    test(S("rbtaqjhgkneisldpmfoc"), "gtfblmqinc", 1, 1);
    test(S("gpifsqlrdkbonjtmheca"), "mkqpbtdalgniorhfescj", 1, 1);
    test(S("hdpkobnsalmcfijregtq"), "", 10, S::npos);
    test(S("jtlshdgqaiprkbcoenfm"), "pblas", 10, 10);
    test(S("fkdrbqltsgmcoiphneaj"), "arosdhcfme", 10, 10);
    test(S("crsplifgtqedjohnabmk"), "blkhjeogicatqfnpdmsr", 10, 10);
    test(S("niptglfbosehkamrdqcj"), "", 19, S::npos);
    test(S("copqdhstbingamjfkler"), "djkqc", 19, S::npos);
    test(S("mrtaefilpdsgocnhqbjk"), "lgokshjtpb", 19, 19);
    test(S("kojatdhlcmigpbfrqnes"), "bqjhtkfepimcnsgrlado", 19, 19);
    test(S("eaintpchlqsbdgrkjofm"), "", 20, S::npos);
    test(S("gjnhidfsepkrtaqbmclo"), "nocfa", 20, S::npos);
    test(S("spocfaktqdbiejlhngmr"), "bgtajmiedc", 20, S::npos);
    test(S("rphmlekgfscndtaobiqj"), "lsckfnqgdahejiopbtmr", 20, S::npos);
    test(S("liatsqdoegkmfcnbhrpj"), "", 21, S::npos);
    test(S("binjagtfldkrspcomqeh"), "gfsrt", 21, S::npos);
    test(S("latkmisecnorjbfhqpdg"), "pfsocbhjtm", 21, S::npos);
    test(S("lecfratdjkhnsmqpoigb"), "tpflmdnoicjgkberhqsa", 21, S::npos);
}

template <class S>
void test1()
{
    test(S(""), "", S::npos);
    test(S(""), "laenf", S::npos);
    test(S(""), "pqlnkmbdjo", S::npos);
    test(S(""), "qkamfogpnljdcshbreti", S::npos);
    test(S("nhmko"), "", S::npos);
    test(S("lahfb"), "irkhs", 2);
    test(S("gmfhd"), "kantesmpgj", 0);
    test(S("odaft"), "oknlrstdpiqmjbaghcfe", 0);
    test(S("eolhfgpjqk"), "", S::npos);
    test(S("nbatdlmekr"), "bnrpe", 0);
    test(S("jdmciepkaq"), "jtdaefblso", 0);
    test(S("hkbgspoflt"), "oselktgbcapndfjihrmq", 0);
    test(S("gprdcokbnjhlsfmtieqa"), "", S::npos);
    test(S("qjghlnftcaismkropdeb"), "bjaht", 1);
    test(S("pnalfrdtkqcmojiesbhg"), "hjlcmgpket", 0);
    test(S("pniotcfrhqsmgdkjbael"), "htaobedqikfplcgjsmrn", 0);
}

int main()
{
    {
    typedef std::string S;
    test0<S>();
    test1<S>();
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test0<S>();
    test1<S>();
    }
#endif
}
