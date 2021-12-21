//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <fstream>

// int_type overflow(int_type c = traits::eof());

// This test is not entirely portable

#include <fstream>
#include <cassert>

#include "platform_support.h" // locale name macros

template <class CharT>
struct test_buf
    : public std::basic_filebuf<CharT>
{
    typedef std::basic_filebuf<CharT>  base;
    typedef typename base::char_type   char_type;
    typedef typename base::int_type    int_type;
    typedef typename base::traits_type traits_type;

    char_type* pbase() const {return base::pbase();}
    char_type* pptr()  const {return base::pptr();}
    char_type* epptr() const {return base::epptr();}
    void gbump(int n) {base::gbump(n);}

    virtual int_type overflow(int_type c = traits_type::eof()) {return base::overflow(c);}
};

int main()
{
    {
        test_buf<char> f;
        assert(f.open("overflow.dat", std::ios_base::out) != 0);
        assert(f.is_open());
        assert(f.pbase() == 0);
        assert(f.pptr() == 0);
        assert(f.epptr() == 0);
        assert(f.overflow('a') == 'a');
        assert(f.pbase() != 0);
        assert(f.pptr() == f.pbase());
        assert(f.epptr() - f.pbase() == 4095);
    }
    {
        test_buf<char> f;
        assert(f.open("overflow.dat", std::ios_base::in) != 0);
        assert(f.is_open());
        assert(f.sgetc() == 'a');
    }
    std::remove("overflow.dat");
    {
        test_buf<char> f;
        f.pubsetbuf(0, 0);
        assert(f.open("overflow.dat", std::ios_base::out) != 0);
        assert(f.is_open());
        assert(f.pbase() == 0);
        assert(f.pptr() == 0);
        assert(f.epptr() == 0);
        assert(f.overflow('a') == 'a');
        assert(f.pbase() == 0);
        assert(f.pptr() == 0);
        assert(f.epptr() == 0);
    }
    {
        test_buf<char> f;
        assert(f.open("overflow.dat", std::ios_base::in) != 0);
        assert(f.is_open());
        assert(f.sgetc() == 'a');
    }
    std::remove("overflow.dat");
    {
        test_buf<wchar_t> f;
        assert(f.open("overflow.dat", std::ios_base::out) != 0);
        assert(f.is_open());
        assert(f.pbase() == 0);
        assert(f.pptr() == 0);
        assert(f.epptr() == 0);
        assert(f.overflow(L'a') == L'a');
        assert(f.pbase() != 0);
        assert(f.pptr() == f.pbase());
        assert(f.epptr() - f.pbase() == 4095);
    }
    {
        test_buf<wchar_t> f;
        assert(f.open("overflow.dat", std::ios_base::in) != 0);
        assert(f.is_open());
        assert(f.sgetc() == L'a');
    }
    std::remove("overflow.dat");
    {
        test_buf<wchar_t> f;
        f.pubsetbuf(0, 0);
        assert(f.open("overflow.dat", std::ios_base::out) != 0);
        assert(f.is_open());
        assert(f.pbase() == 0);
        assert(f.pptr() == 0);
        assert(f.epptr() == 0);
        assert(f.overflow(L'a') == L'a');
        assert(f.pbase() == 0);
        assert(f.pptr() == 0);
        assert(f.epptr() == 0);
    }
    {
        test_buf<wchar_t> f;
        assert(f.open("overflow.dat", std::ios_base::in) != 0);
        assert(f.is_open());
        assert(f.sgetc() == L'a');
    }
    std::remove("overflow.dat");
    {
        test_buf<wchar_t> f;
        f.pubimbue(std::locale(LOCALE_en_US_UTF_8));
        assert(f.open("overflow.dat", std::ios_base::out) != 0);
        assert(f.sputc(0x4E51) == 0x4E51);
        assert(f.sputc(0x4E52) == 0x4E52);
        assert(f.sputc(0x4E53) == 0x4E53);
    }
    {
        test_buf<char> f;
        assert(f.open("overflow.dat", std::ios_base::in) != 0);
        assert(f.is_open());
        assert(f.sbumpc() == 0xE4);
        assert(f.sbumpc() == 0xB9);
        assert(f.sbumpc() == 0x91);
        assert(f.sbumpc() == 0xE4);
        assert(f.sbumpc() == 0xB9);
        assert(f.sbumpc() == 0x92);
        assert(f.sbumpc() == 0xE4);
        assert(f.sbumpc() == 0xB9);
        assert(f.sbumpc() == 0x93);
        assert(f.sbumpc() == -1);
    }
    std::remove("overflow.dat");
}
