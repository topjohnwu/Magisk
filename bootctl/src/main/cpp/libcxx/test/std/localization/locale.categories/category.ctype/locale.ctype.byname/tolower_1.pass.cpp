//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <locale>

// template <class charT> class ctype_byname;

// charT tolower(charT) const;

#include <locale>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l;
        {
            typedef std::ctype_byname<char> F;
            std::locale ll(l, new F(LOCALE_en_US_UTF_8));
            const F& f = std::use_facet<F>(ll);

            assert(f.tolower(' ') == ' ');
            assert(f.tolower('A') == 'a');
            assert(f.tolower('\x07') == '\x07');
            assert(f.tolower('.') == '.');
            assert(f.tolower('a') == 'a');
            assert(f.tolower('1') == '1');
            assert(f.tolower('\xFA') == '\xFA');
        }
    }
    {
        std::locale l;
        {
            typedef std::ctype_byname<char> F;
            std::locale ll(l, new F("C"));
            const F& f = std::use_facet<F>(ll);

            assert(f.tolower(' ') == ' ');
            assert(f.tolower('A') == 'a');
            assert(f.tolower('\x07') == '\x07');
            assert(f.tolower('.') == '.');
            assert(f.tolower('a') == 'a');
            assert(f.tolower('1') == '1');
            assert(f.tolower('\xDA') == '\xDA');
            assert(f.tolower('\xFA') == '\xFA');
        }
    }
    {
        std::locale l;
        {
            typedef std::ctype_byname<wchar_t> F;
            std::locale ll(l, new F(LOCALE_en_US_UTF_8));
            const F& f = std::use_facet<F>(ll);

            assert(f.tolower(L' ') == L' ');
            assert(f.tolower(L'A') == L'a');
            assert(f.tolower(L'\x07') == L'\x07');
            assert(f.tolower(L'.') == L'.');
            assert(f.tolower(L'a') == L'a');
            assert(f.tolower(L'1') == L'1');
            assert(f.tolower(L'\xDA') == L'\xFA');
            assert(f.tolower(L'\xFA') == L'\xFA');
        }
    }
    {
        std::locale l;
        {
            typedef std::ctype_byname<wchar_t> F;
            std::locale ll(l, new F("C"));
            const F& f = std::use_facet<F>(ll);

            assert(f.tolower(L' ') == L' ');
            assert(f.tolower(L'A') == L'a');
            assert(f.tolower(L'\x07') == L'\x07');
            assert(f.tolower(L'.') == L'.');
            assert(f.tolower(L'a') == L'a');
            assert(f.tolower(L'1') == L'1');
            assert(f.tolower(L'\xDA') == L'\xDA');
            assert(f.tolower(L'\xFA') == L'\xFA');
        }
    }
}
