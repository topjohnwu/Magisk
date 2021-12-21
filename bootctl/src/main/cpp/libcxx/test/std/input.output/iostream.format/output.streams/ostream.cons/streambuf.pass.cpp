//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ostream>

// template <class charT, class traits = char_traits<charT> >
// class basic_ostream;

// explicit basic_ostream(basic_streambuf<charT,traits>* sb);

#include <ostream>
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
        std::basic_ostream<char> os(&sb);
        assert(os.rdbuf() == &sb);
        assert(os.tie() == 0);
        assert(os.fill() == ' ');
        assert(os.rdstate() == os.goodbit);
        assert(os.exceptions() == os.goodbit);
        assert(os.flags() == (os.skipws | os.dec));
        assert(os.precision() == 6);
        assert(os.getloc().name() == "C");
    }
    {
        testbuf<wchar_t> sb;
        std::basic_ostream<wchar_t> os(&sb);
        assert(os.rdbuf() == &sb);
        assert(os.tie() == 0);
        assert(os.fill() == L' ');
        assert(os.rdstate() == os.goodbit);
        assert(os.exceptions() == os.goodbit);
        assert(os.flags() == (os.skipws | os.dec));
        assert(os.precision() == 6);
        assert(os.getloc().name() == "C");
    }
}
