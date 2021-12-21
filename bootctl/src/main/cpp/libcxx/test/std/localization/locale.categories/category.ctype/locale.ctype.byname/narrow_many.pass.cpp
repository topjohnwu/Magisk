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

// const charT* narrow(const charT* low, const charT*, char dfault, char* to) const;

#include <locale>
#include <string>
#include <vector>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(LOCALE_fr_CA_ISO8859_1);
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
            std::wstring in(L" A\x07.a1\xDA");
            std::vector<char> v(in.size());

            assert(f.narrow(&in[0], in.data() + in.size(), '*', v.data()) == in.data() + in.size());
            assert(v[0] == ' ');
            assert(v[1] == 'A');
            assert(v[2] == '\x07');
            assert(v[3] == '.');
            assert(v[4] == 'a');
            assert(v[5] == '1');
            assert(v[6] == '\xDA');
        }
    }
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
            std::wstring in(L" A\x07.a1\xDA");
            std::vector<char> v(in.size());

            assert(f.narrow(&in[0], in.data() + in.size(), '*', v.data()) == in.data() + in.size());
            assert(v[0] == ' ');
            assert(v[1] == 'A');
            assert(v[2] == '\x07');
            assert(v[3] == '.');
            assert(v[4] == 'a');
            assert(v[5] == '1');
            assert(v[6] == '*');
        }
    }
}
