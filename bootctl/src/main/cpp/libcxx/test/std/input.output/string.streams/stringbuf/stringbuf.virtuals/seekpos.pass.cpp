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

// pos_type seekpos(pos_type sp,
//                  ios_base::openmode which = ios_base::in | ios_base::out);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringbuf sb("0123456789", std::ios_base::in);
        assert(sb.pubseekpos(3, std::ios_base::out) == -1);
        assert(sb.pubseekpos(3, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekpos(3, std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
    }
    {
        std::stringbuf sb("0123456789", std::ios_base::out);
        assert(sb.pubseekpos(3, std::ios_base::in) == -1);
        assert(sb.pubseekpos(3, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekpos(3, std::ios_base::out) == 3);
        assert(sb.sputc('a') == 'a');
        assert(sb.str() == "012a456789");
    }
    {
        std::stringbuf sb("0123456789");
        assert(sb.pubseekpos(3, std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
        assert(sb.pubseekpos(3, std::ios_base::out | std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
        assert(sb.sputc('a') == 'a');
        assert(sb.str() == "012a456789");
        assert(sb.pubseekpos(3, std::ios_base::out) == 3);
        assert(sb.sputc('3') == '3');
        assert(sb.str() == "0123456789");
    }
    {
        std::wstringbuf sb(L"0123456789", std::ios_base::in);
        assert(sb.pubseekpos(3, std::ios_base::out) == -1);
        assert(sb.pubseekpos(3, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekpos(3, std::ios_base::in) == 3);
        assert(sb.sgetc() == L'3');
    }
    {
        std::wstringbuf sb(L"0123456789", std::ios_base::out);
        assert(sb.pubseekpos(3, std::ios_base::in) == -1);
        assert(sb.pubseekpos(3, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekpos(3, std::ios_base::out) == 3);
        assert(sb.sputc(L'a') == L'a');
        assert(sb.str() == L"012a456789");
    }
    {
        std::wstringbuf sb(L"0123456789");
        assert(sb.pubseekpos(3, std::ios_base::in) == 3);
        assert(sb.sgetc() == L'3');
        assert(sb.pubseekpos(3, std::ios_base::out | std::ios_base::in) == 3);
        assert(sb.sgetc() == L'3');
        assert(sb.sputc(L'a') == L'a');
        assert(sb.str() == L"012a456789");
        assert(sb.pubseekpos(3, std::ios_base::out) == 3);
        assert(sb.sputc(L'3') == L'3');
        assert(sb.str() == L"0123456789");
    }
}
