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

// const char* widen(const char* low, const char* high, charT* to) const;

// I doubt this test is portable

#include <locale>
#include <string>
#include <vector>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            typedef std::ctype_byname<wchar_t> F;
            std::locale ll(l, new F(LOCALE_en_US_UTF_8));
            F const& f = std::use_facet<F>(ll);
            std::string in(" A\x07.a1\x85");
            std::vector<wchar_t> v(in.size());

            assert(f.widen(&in[0], in.data() + in.size(), v.data()) == in.data() + in.size());
            assert(v[0] == L' ');
            assert(v[1] == L'A');
            assert(v[2] == L'\x07');
            assert(v[3] == L'.');
            assert(v[4] == L'a');
            assert(v[5] == L'1');
            assert(v[6] == wchar_t(-1));
        }
    }
    {
        std::locale l("C");
        {
            typedef std::ctype_byname<wchar_t> F;
            std::locale ll(l, new F("C"));
            const F& f = std::use_facet<F>(ll);
            std::string in(" A\x07.a1\x85");
            std::vector<wchar_t> v(in.size());

            assert(f.widen(&in[0], in.data() + in.size(), v.data()) == in.data() + in.size());
            assert(v[0] == L' ');
            assert(v[1] == L'A');
            assert(v[2] == L'\x07');
            assert(v[3] == L'.');
            assert(v[4] == L'a');
            assert(v[5] == L'1');
#if defined(__APPLE__) || defined(__FreeBSD__)
            assert(v[6] == L'\x85');
#else
            assert(v[6] == wchar_t(-1));
#endif
        }
    }
}
