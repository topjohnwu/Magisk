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

// pos_type seekoff(off_type off, ios_base::seekdir way,
//                  ios_base::openmode which = ios_base::in | ios_base::out);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringbuf sb(std::ios_base::in);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(0, std::ios_base::beg, std::ios_base::in) == 0);
        assert(sb.pubseekoff(0, std::ios_base::cur, std::ios_base::in) == 0);
        assert(sb.pubseekoff(0, std::ios_base::end, std::ios_base::in) == 0);
    }
    {
        std::stringbuf sb(std::ios_base::out);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(0, std::ios_base::beg, std::ios_base::out) == 0);
        assert(sb.pubseekoff(0, std::ios_base::cur, std::ios_base::out) == 0);
        assert(sb.pubseekoff(0, std::ios_base::end, std::ios_base::out) == 0);
    }
    {
        std::stringbuf sb("0123456789", std::ios_base::in);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == 6);
        assert(sb.sgetc() == '6');
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == 7);
        assert(sb.sgetc() == '7');
    }
    {
        std::stringbuf sb("0123456789", std::ios_base::out);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == 3);
        assert(sb.sputc('a') == 'a');
        assert(sb.str() == "012a456789");
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == 7);
        assert(sb.sputc('b') == 'b');
        assert(sb.str() == "012a456b89");
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == 7);
        assert(sb.sputc('c') == 'c');
        assert(sb.str() == "012a456c89");
    }
    {
        std::stringbuf sb("0123456789");
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == 6);
        assert(sb.sgetc() == '6');
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == 7);
        assert(sb.sgetc() == '7');
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out | std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
        assert(sb.sputc('a') == 'a');
        assert(sb.str() == "012a456789");
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out | std::ios_base::in) == 7);
        assert(sb.sgetc() == '7');
        assert(sb.sputc('c') == 'c');
        assert(sb.str() == "012a456c89");
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == 3);
        assert(sb.sputc('3') == '3');
        assert(sb.str() == "0123456c89");
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == 7);
        assert(sb.sputc('7') == '7');
        assert(sb.str() == "0123456789");
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == 7);
        assert(sb.sputc('c') == 'c');
        assert(sb.str() == "0123456c89");
    }
    {
        std::wstringbuf sb(L"0123456789", std::ios_base::in);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == 3);
        assert(sb.sgetc() == L'3');
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == 6);
        assert(sb.sgetc() == L'6');
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == 7);
        assert(sb.sgetc() == L'7');
    }
    {
        std::wstringbuf sb(L"0123456789", std::ios_base::out);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == 3);
        assert(sb.sputc(L'a') == L'a');
        assert(sb.str() == L"012a456789");
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == 7);
        assert(sb.sputc(L'b') == L'b');
        assert(sb.str() == L"012a456b89");
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == 7);
        assert(sb.sputc(L'c') == L'c');
        assert(sb.str() == L"012a456c89");
    }
    {
        std::wstringbuf sb(L"0123456789");
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == 3);
        assert(sb.sgetc() == L'3');
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == 6);
        assert(sb.sgetc() == L'6');
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == 7);
        assert(sb.sgetc() == L'7');
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out | std::ios_base::in) == 3);
        assert(sb.sgetc() == L'3');
        assert(sb.sputc(L'a') == L'a');
        assert(sb.str() == L"012a456789");
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out | std::ios_base::in) == 7);
        assert(sb.sgetc() == L'7');
        assert(sb.sputc(L'c') == L'c');
        assert(sb.str() == L"012a456c89");
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == 3);
        assert(sb.sputc(L'3') == L'3');
        assert(sb.str() == L"0123456c89");
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == 7);
        assert(sb.sputc(L'7') == L'7');
        assert(sb.str() == L"0123456789");
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == 7);
        assert(sb.sputc(L'c') == L'c');
        assert(sb.str() == L"0123456c89");
    }
}
