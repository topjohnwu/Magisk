//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// template <class charT, class traits = char_traits<charT> >
// class basic_istream;

// explicit basic_istream(basic_streambuf<charT,traits>* sb);

#include <istream>
#include <cassert>

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    testbuf() {}
};

int main()
{
    {
        testbuf<char> sb;
        std::basic_istream<char> is(&sb);
        assert(is.rdbuf() == &sb);
        assert(is.tie() == 0);
        assert(is.fill() == ' ');
        assert(is.rdstate() == is.goodbit);
        assert(is.exceptions() == is.goodbit);
        assert(is.flags() == (is.skipws | is.dec));
        assert(is.precision() == 6);
        assert(is.getloc().name() == "C");
        assert(is.gcount() == 0);
    }
    {
        testbuf<wchar_t> sb;
        std::basic_istream<wchar_t> is(&sb);
        assert(is.rdbuf() == &sb);
        assert(is.tie() == 0);
        assert(is.fill() == L' ');
        assert(is.rdstate() == is.goodbit);
        assert(is.exceptions() == is.goodbit);
        assert(is.flags() == (is.skipws | is.dec));
        assert(is.precision() == 6);
        assert(is.getloc().name() == "C");
        assert(is.gcount() == 0);
    }
}
