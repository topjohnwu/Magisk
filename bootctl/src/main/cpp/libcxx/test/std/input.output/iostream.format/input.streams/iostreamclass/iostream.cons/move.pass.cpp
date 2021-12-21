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
// class basic_iostream;

// basic_iostream(basic_iostream&& rhs);

#include <istream>
#include <cassert>


template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    testbuf() {}
};

template <class CharT>
struct test_iostream
    : public std::basic_iostream<CharT>
{
    typedef std::basic_iostream<CharT> base;
    test_iostream(testbuf<CharT>* sb) : base(sb) {}

    test_iostream(test_iostream&& s)
        : base(std::move(s)) {}
};


int main()
{
    {
        testbuf<char> sb;
        test_iostream<char> is1(&sb);
        test_iostream<char> is(std::move(is1));
        assert(is1.rdbuf() == &sb);
        assert(is1.gcount() == 0);
        assert(is.gcount() == 0);
        assert(is.rdbuf() == 0);
        assert(is.tie() == 0);
        assert(is.fill() == ' ');
        assert(is.rdstate() == is.goodbit);
        assert(is.exceptions() == is.goodbit);
        assert(is.flags() == (is.skipws | is.dec));
        assert(is.precision() == 6);
        assert(is.getloc().name() == "C");
    }
    {
        testbuf<wchar_t> sb;
        test_iostream<wchar_t> is1(&sb);
        test_iostream<wchar_t> is(std::move(is1));
        assert(is1.gcount() == 0);
        assert(is.gcount() == 0);
        assert(is1.rdbuf() == &sb);
        assert(is.rdbuf() == 0);
        assert(is.tie() == 0);
        assert(is.fill() == L' ');
        assert(is.rdstate() == is.goodbit);
        assert(is.exceptions() == is.goodbit);
        assert(is.flags() == (is.skipws | is.dec));
        assert(is.precision() == 6);
        assert(is.getloc().name() == "C");
    }
}
