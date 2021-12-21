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

// int_type underflow();

// This test is not entirely portable

#include <locale>
#include <codecvt>
#include <fstream>
#include <cassert>

struct test_buf
    : public std::wbuffer_convert<std::codecvt_utf8<wchar_t> >
{
    typedef std::wbuffer_convert<std::codecvt_utf8<wchar_t> > base;
    typedef base::char_type   char_type;
    typedef base::int_type    int_type;
    typedef base::traits_type traits_type;

    explicit test_buf(std::streambuf* sb) : base(sb) {}

    char_type* eback() const {return base::eback();}
    char_type* gptr()  const {return base::gptr();}
    char_type* egptr() const {return base::egptr();}
    void gbump(int n) {base::gbump(n);}

    virtual int_type underflow() {return base::underflow();}
};

int main()
{
    {
        std::ifstream bs("underflow.dat");
        test_buf f(bs.rdbuf());
        assert(f.eback() == 0);
        assert(f.gptr() == 0);
        assert(f.egptr() == 0);
        assert(f.underflow() == L'1');
        assert(f.eback() != 0);
        assert(f.eback() == f.gptr());
        assert(*f.gptr() == L'1');
        assert(f.egptr() - f.eback() == 9);
    }
    {
        std::ifstream bs("underflow.dat");
        test_buf f(bs.rdbuf());
        assert(f.eback() == 0);
        assert(f.gptr() == 0);
        assert(f.egptr() == 0);
        assert(f.underflow() == L'1');
        assert(f.eback() != 0);
        assert(f.eback() == f.gptr());
        assert(*f.gptr() == L'1');
        assert(f.egptr() - f.eback() == 9);
        f.gbump(8);
        assert(f.sgetc() == L'9');
        assert(f.eback()[0] == L'1');
        assert(f.eback()[1] == L'2');
        assert(f.eback()[2] == L'3');
        assert(f.eback()[3] == L'4');
        assert(f.gptr() - f.eback() == 8);
        assert(*f.gptr() == L'9');
        assert(f.egptr() - f.gptr() == 1);
    }
    {
        std::ifstream bs("underflow_utf8.dat");
        test_buf f(bs.rdbuf());
        assert(f.sbumpc() == 0x4E51);
        assert(f.sbumpc() == 0x4E52);
        assert(f.sbumpc() == 0x4E53);
        assert(f.sbumpc() == test_buf::traits_type::eof());
    }
}
