//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class ctype_byname;

// bool is(mask m, charT c) const;

// REQUIRES: locale.en_US.UTF-8

#include <locale>
#include <type_traits>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);

            assert(f.is(F::space, L' '));
            assert(!f.is(F::space, L'A'));

            assert(f.is(F::print, L' '));
            assert(!f.is(F::print, L'\x07'));

            assert(f.is(F::cntrl, L'\x07'));
            assert(!f.is(F::cntrl, L' '));

            assert(f.is(F::upper, L'A'));
            assert(!f.is(F::upper, L'a'));

            assert(f.is(F::lower, L'a'));
            assert(!f.is(F::lower, L'A'));

            assert(f.is(F::alpha, L'a'));
            assert(!f.is(F::alpha, L'1'));

            assert(f.is(F::digit, L'1'));
            assert(!f.is(F::digit, L'a'));

            assert(f.is(F::punct, L'.'));
            assert(!f.is(F::punct, L'a'));

            assert(f.is(F::xdigit, L'a'));
            assert(!f.is(F::xdigit, L'g'));

            assert(f.is(F::alnum, L'a'));
            assert(!f.is(F::alnum, L'.'));

            assert(f.is(F::graph, L'.'));
            assert(!f.is(F::graph,  L'\x07'));

            assert(f.is(F::alpha, L'\x00DA'));
            assert(f.is(F::upper, L'\x00DA'));
        }
    }
    {
        std::locale l("C");
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);

            assert(f.is(F::space, L' '));
            assert(!f.is(F::space, L'A'));

            assert(f.is(F::print, L' '));
            assert(!f.is(F::print, L'\x07'));

            assert(f.is(F::cntrl, L'\x07'));
            assert(!f.is(F::cntrl, L' '));

            assert(f.is(F::upper, L'A'));
            assert(!f.is(F::upper, L'a'));

            assert(f.is(F::lower, L'a'));
            assert(!f.is(F::lower, L'A'));

            assert(f.is(F::alpha, L'a'));
            assert(!f.is(F::alpha, L'1'));

            assert(f.is(F::digit, L'1'));
            assert(!f.is(F::digit, L'a'));

            assert(f.is(F::punct, L'.'));
            assert(!f.is(F::punct, L'a'));

            assert(f.is(F::xdigit, L'a'));
            assert(!f.is(F::xdigit, L'g'));

            assert(f.is(F::alnum, L'a'));
            assert(!f.is(F::alnum, L'.'));

            assert(f.is(F::graph, L'.'));
            assert(!f.is(F::graph,  L'\x07'));

            assert(!f.is(F::alpha, L'\x00DA'));
            assert(!f.is(F::upper, L'\x00DA'));
        }
    }
}
