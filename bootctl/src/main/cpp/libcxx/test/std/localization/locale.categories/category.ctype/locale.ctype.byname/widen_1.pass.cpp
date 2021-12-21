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

// charT widen(char c) const;

// I doubt this test is portable


#include <locale>
#include <cassert>
#include <limits.h>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l;
        {
            typedef std::ctype_byname<wchar_t> F;
            std::locale ll(l, new F(LOCALE_en_US_UTF_8));
            const F& f = std::use_facet<F>(ll);

            assert(f.widen(' ') == L' ');
            assert(f.widen('A') == L'A');
            assert(f.widen('\x07') == L'\x07');
            assert(f.widen('.') == L'.');
            assert(f.widen('a') == L'a');
            assert(f.widen('1') == L'1');
            assert(f.widen(char(-5)) == wchar_t(-1));
        }
    }
    {
        std::locale l;
        {
            typedef std::ctype_byname<wchar_t> F;
            std::locale ll(l, new F("C"));
            const F& f = std::use_facet<F>(ll);

            assert(f.widen(' ') == L' ');
            assert(f.widen('A') == L'A');
            assert(f.widen('\x07') == L'\x07');
            assert(f.widen('.') == L'.');
            assert(f.widen('a') == L'a');
            assert(f.widen('1') == L'1');
#if defined(__APPLE__) || defined(__FreeBSD__)
            assert(f.widen(char(-5)) == L'\u00fb');
#else
            assert(f.widen(char(-5)) == wchar_t(-1));
#endif
        }
    }
}
