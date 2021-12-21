//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_CA.ISO8859-1

// <locale>

// template <class charT> class ctype_byname;

// char narrow(charT c, char dfault) const;

#include <locale>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(std::string(LOCALE_fr_CA_ISO8859_1));
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);

            assert(f.narrow(L' ', '*') == ' ');
            assert(f.narrow(L'A', '*') == 'A');
            assert(f.narrow(L'\x07', '*') == '\x07');
            assert(f.narrow(L'.', '*') == '.');
            assert(f.narrow(L'a', '*') == 'a');
            assert(f.narrow(L'1', '*') == '1');
            assert(f.narrow(L'\xDA', '*') == '\xDA');
        }
    }
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);

            assert(f.narrow(L' ', '*') == ' ');
            assert(f.narrow(L'A', '*') == 'A');
            assert(f.narrow(L'\x07', '*') == '\x07');
            assert(f.narrow(L'.', '*') == '.');
            assert(f.narrow(L'a', '*') == 'a');
            assert(f.narrow(L'1', '*') == '1');
            assert(f.narrow(L'\xDA', '*') == '*');
        }
    }
}
