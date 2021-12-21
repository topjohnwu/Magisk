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

// int_type overflow(int_type c = traits::eof());

#include <sstream>
#include <cassert>

int overflow_called = 0;

template <class CharT>
struct testbuf
    : public std::basic_stringbuf<CharT>
{
    typedef std::basic_stringbuf<CharT> base;
    explicit testbuf(const std::basic_string<CharT>& str,
                     std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
        : base(str, which) {}

    typename base::int_type
        overflow(typename base::int_type c = base::traits_type::eof())
        {++overflow_called; return base::overflow(c);}

    void pbump(int n) {base::pbump(n);}
};

int main()
{
    {  // sanity check
    testbuf<char> tb("");
    tb.overflow();
    }
    {
        testbuf<char> sb("abc");
        assert(sb.sputc('1') == '1');
        assert(sb.str() == "1bc");
        assert(sb.sputc('2') == '2');
        assert(sb.str() == "12c");
        assert(sb.sputc('3') == '3');
        assert(sb.str() == "123");
        assert(sb.sputc('4') == '4');
        assert(sb.str() == "1234");
        assert(sb.sputc('5') == '5');
        assert(sb.str() == "12345");
        assert(sb.sputc('6') == '6');
        assert(sb.str() == "123456");
        assert(sb.sputc('7') == '7');
        assert(sb.str() == "1234567");
        assert(sb.sputc('8') == '8');
        assert(sb.str() == "12345678");
        assert(sb.sputc('9') == '9');
        assert(sb.str() == "123456789");
        assert(sb.sputc('0') == '0');
        assert(sb.str() == "1234567890");
        assert(sb.sputc('1') == '1');
        assert(sb.str() == "12345678901");
    }
    {
        testbuf<wchar_t> sb(L"abc");
        assert(sb.sputc(L'1') == L'1');
        assert(sb.str() == L"1bc");
        assert(sb.sputc(L'2') == L'2');
        assert(sb.str() == L"12c");
        assert(sb.sputc(L'3') == L'3');
        assert(sb.str() == L"123");
        assert(sb.sputc(L'4') == L'4');
        assert(sb.str() == L"1234");
        assert(sb.sputc(L'5') == L'5');
        assert(sb.str() == L"12345");
        assert(sb.sputc(L'6') == L'6');
        assert(sb.str() == L"123456");
        assert(sb.sputc(L'7') == L'7');
        assert(sb.str() == L"1234567");
        assert(sb.sputc(L'8') == L'8');
        assert(sb.str() == L"12345678");
        assert(sb.sputc(L'9') == L'9');
        assert(sb.str() == L"123456789");
        assert(sb.sputc(L'0') == L'0');
        assert(sb.str() == L"1234567890");
        assert(sb.sputc(L'1') == L'1');
        assert(sb.str() == L"12345678901");
    }
    {
        testbuf<char> sb("abc", std::ios_base::app | std::ios_base::out);
        assert(sb.sputc('1') == '1');
        assert(sb.str() == "abc1");
        assert(sb.sputc('2') == '2');
        assert(sb.str() == "abc12");
    }
}
