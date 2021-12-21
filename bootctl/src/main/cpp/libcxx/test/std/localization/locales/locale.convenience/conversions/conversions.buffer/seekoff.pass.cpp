//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// wbuffer_convert<Codecvt, Elem, Tr>

// pos_type seekoff(off_type off, ios_base::seekdir way,
//                  ios_base::openmode which = ios_base::in | ios_base::out);
// pos_type seekpos(pos_type sp,
//                  ios_base::openmode which = ios_base::in | ios_base::out);

// This test is not entirely portable

#include <locale>
#include <codecvt>
#include <fstream>
#include <cassert>

class test_codecvt
    : public std::codecvt<wchar_t, char, std::mbstate_t>
{
    typedef std::codecvt<wchar_t, char, std::mbstate_t> base;
public:
    explicit test_codecvt(std::size_t refs = 0) : base(refs) {}
    ~test_codecvt() {}
};

int main()
{
    {
        wchar_t buf[10];
        typedef std::wbuffer_convert<test_codecvt> test_buf;
        typedef test_buf::pos_type pos_type;
        std::fstream bs("seekoff.dat", std::ios::trunc | std::ios::in
                                                       | std::ios::out);
        test_buf f(bs.rdbuf());
        f.pubsetbuf(buf, sizeof(buf)/sizeof(buf[0]));
        f.sputn(L"abcdefghijklmnopqrstuvwxyz", 26);
        assert(buf[0] == L'v');
        pos_type p = f.pubseekoff(-15, std::ios_base::cur);
        assert(p == 11);
        assert(f.sgetc() == L'l');
        f.pubseekoff(0, std::ios_base::beg);
        assert(f.sgetc() == L'a');
        f.pubseekoff(-1, std::ios_base::end);
        assert(f.sgetc() == L'z');
        assert(f.pubseekpos(p) == p);
        assert(f.sgetc() == L'l');
    }
    std::remove("seekoff.dat");
}
