//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <fstream>

// int_type pbackfail(int_type c = traits::eof());

#include <fstream>
#include <cassert>

#include "test_macros.h"

template <class CharT>
struct test_buf
    : public std::basic_filebuf<CharT>
{
    typedef std::basic_filebuf<CharT>  base;
    typedef typename base::char_type   char_type;
    typedef typename base::int_type    int_type;
    typedef typename base::traits_type traits_type;

    char_type* eback() const {return base::eback();}
    char_type* gptr()  const {return base::gptr();}
    char_type* egptr() const {return base::egptr();}
    void gbump(int n) {base::gbump(n);}

    virtual int_type pbackfail(int_type c = traits_type::eof()) {return base::pbackfail(c);}
};

int main()
{
    {
        test_buf<char> f;
        assert(f.open("underflow.dat", std::ios_base::in) != 0);
        assert(f.is_open());
        assert(f.sbumpc() == '1');
        assert(f.sgetc() == '2');
        typename test_buf<char>::int_type pbackResult = f.pbackfail('a');
        LIBCPP_ASSERT(pbackResult == -1);
        if (pbackResult != -1) {
            assert(f.sbumpc() == 'a');
            assert(f.sgetc() == '2');
        }
    }
    {
        test_buf<char> f;
        assert(f.open("underflow.dat", std::ios_base::in | std::ios_base::out) != 0);
        assert(f.is_open());
        assert(f.sbumpc() == '1');
        assert(f.sgetc() == '2');
        typename test_buf<char>::int_type pbackResult = f.pbackfail('a');
        LIBCPP_ASSERT(pbackResult == 'a');
        if (pbackResult != -1) {
            assert(f.sbumpc() == 'a');
            assert(f.sgetc() == '2');
        }
    }
}
