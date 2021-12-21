//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <sstream>

// template <class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
// class basic_stringbuf

// int_type underflow();

#include <sstream>
#include <cassert>

template <class CharT>
struct testbuf
    : public std::basic_stringbuf<CharT>
{
    typedef std::basic_stringbuf<CharT> base;
    explicit testbuf(const std::basic_string<CharT>& str)
        : base(str) {}

    typename base::int_type underflow() {return base::underflow();}
    void pbump(int n) {base::pbump(n);}
};

int main()
{
    {
        testbuf<char> sb("123");
        sb.pbump(3);
        assert(sb.underflow() == '1');
        assert(sb.underflow() == '1');
        assert(sb.snextc() == '2');
        assert(sb.underflow() == '2');
        assert(sb.underflow() == '2');
        assert(sb.snextc() == '3');
        assert(sb.underflow() == '3');
        assert(sb.underflow() == '3');
        assert(sb.snextc() == std::char_traits<char>::eof());
        assert(sb.underflow() == std::char_traits<char>::eof());
        assert(sb.underflow() == std::char_traits<char>::eof());
        sb.sputc('4');
        assert(sb.underflow() == '4');
        assert(sb.underflow() == '4');
    }
    {
        testbuf<wchar_t> sb(L"123");
        sb.pbump(3);
        assert(sb.underflow() == L'1');
        assert(sb.underflow() == L'1');
        assert(sb.snextc() == L'2');
        assert(sb.underflow() == L'2');
        assert(sb.underflow() == L'2');
        assert(sb.snextc() == L'3');
        assert(sb.underflow() == L'3');
        assert(sb.underflow() == L'3');
        assert(sb.snextc() == std::char_traits<wchar_t>::eof());
        assert(sb.underflow() == std::char_traits<wchar_t>::eof());
        assert(sb.underflow() == std::char_traits<wchar_t>::eof());
        sb.sputc(L'4');
        assert(sb.underflow() == L'4');
        assert(sb.underflow() == L'4');
    }
}
