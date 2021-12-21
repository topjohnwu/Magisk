//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <istream>

// template <class charT, class traits = char_traits<charT> >
// class basic_istream;

// basic_istream& operator=(basic_istream&& rhs);

#include <istream>
#include <cassert>


template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    testbuf() {}
};

template <class CharT>
struct test_istream
    : public std::basic_istream<CharT>
{
    typedef std::basic_istream<CharT> base;
    test_istream(testbuf<CharT>* sb) : base(sb) {}

    test_istream& operator=(test_istream&& s)
        {base::operator=(std::move(s)); return *this;}
};


int main()
{
    {
        testbuf<char> sb1;
        testbuf<char> sb2;
        test_istream<char> is1(&sb1);
        test_istream<char> is2(&sb2);
        is2 = (std::move(is1));
        assert(is1.rdbuf() == &sb1);
        assert(is1.tie() == 0);
        assert(is1.fill() == ' ');
        assert(is1.rdstate() == is1.goodbit);
        assert(is1.exceptions() == is1.goodbit);
        assert(is1.flags() == (is1.skipws | is1.dec));
        assert(is1.precision() == 6);
        assert(is1.getloc().name() == "C");
        assert(is2.rdbuf() == &sb2);
        assert(is2.tie() == 0);
        assert(is2.fill() == ' ');
        assert(is2.rdstate() == is2.goodbit);
        assert(is2.exceptions() == is2.goodbit);
        assert(is2.flags() == (is2.skipws | is2.dec));
        assert(is2.precision() == 6);
        assert(is2.getloc().name() == "C");
    }
    {
        testbuf<wchar_t> sb1;
        testbuf<wchar_t> sb2;
        test_istream<wchar_t> is1(&sb1);
        test_istream<wchar_t> is2(&sb2);
        is2 = (std::move(is1));
        assert(is1.rdbuf() == &sb1);
        assert(is1.tie() == 0);
        assert(is1.fill() == ' ');
        assert(is1.rdstate() == is1.goodbit);
        assert(is1.exceptions() == is1.goodbit);
        assert(is1.flags() == (is1.skipws | is1.dec));
        assert(is1.precision() == 6);
        assert(is1.getloc().name() == "C");
        assert(is2.rdbuf() == &sb2);
        assert(is2.tie() == 0);
        assert(is2.fill() == ' ');
        assert(is2.rdstate() == is2.goodbit);
        assert(is2.exceptions() == is2.goodbit);
        assert(is2.flags() == (is2.skipws | is2.dec));
        assert(is2.precision() == 6);
        assert(is2.getloc().name() == "C");
    }
}
