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

// void swap(basic_ostream& rhs);

#include <ostream>
#include <cassert>

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    testbuf() {}
};

template <class CharT>
struct test_ostream
    : public std::basic_ostream<CharT>
{
    typedef std::basic_ostream<CharT> base;
    test_ostream(testbuf<CharT>* sb) : base(sb) {}

    void swap(test_ostream& s) {base::swap(s);}
};

int main()
{
    {
        testbuf<char> sb1;
        testbuf<char> sb2;
        test_ostream<char> os1(&sb1);
        test_ostream<char> os2(&sb2);
        os1.swap(os2);
        assert(os1.rdbuf() == &sb1);
        assert(os1.tie() == 0);
        assert(os1.fill() == ' ');
        assert(os1.rdstate() == os1.goodbit);
        assert(os1.exceptions() == os1.goodbit);
        assert(os1.flags() == (os1.skipws | os1.dec));
        assert(os1.precision() == 6);
        assert(os1.getloc().name() == "C");
        assert(os2.rdbuf() == &sb2);
        assert(os2.tie() == 0);
        assert(os2.fill() == ' ');
        assert(os2.rdstate() == os2.goodbit);
        assert(os2.exceptions() == os2.goodbit);
        assert(os2.flags() == (os2.skipws | os2.dec));
        assert(os2.precision() == 6);
        assert(os2.getloc().name() == "C");
    }
    {
        testbuf<wchar_t> sb1;
        testbuf<wchar_t> sb2;
        test_ostream<wchar_t> os1(&sb1);
        test_ostream<wchar_t> os2(&sb2);
        os1.swap(os2);
        assert(os1.rdbuf() == &sb1);
        assert(os1.tie() == 0);
        assert(os1.fill() == ' ');
        assert(os1.rdstate() == os1.goodbit);
        assert(os1.exceptions() == os1.goodbit);
        assert(os1.flags() == (os1.skipws | os1.dec));
        assert(os1.precision() == 6);
        assert(os1.getloc().name() == "C");
        assert(os2.rdbuf() == &sb2);
        assert(os2.tie() == 0);
        assert(os2.fill() == ' ');
        assert(os2.rdstate() == os2.goodbit);
        assert(os2.exceptions() == os2.goodbit);
        assert(os2.flags() == (os2.skipws | os2.dec));
        assert(os2.precision() == 6);
        assert(os2.getloc().name() == "C");
    }
}
