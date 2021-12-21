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

// const charT* scan_not(mask m, const charT* low, const charT* high) const;

// REQUIRES: locale.en_US.UTF-8

#include <locale>
#include <string>
#include <vector>
#include <cassert>

#include <stdio.h>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
            const std::wstring in(L"\x00DA A\x07.a1");
            std::vector<F::mask> m(in.size());
            assert(f.scan_not(F::space, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::print, in.data(), in.data() + in.size()) - in.data() == 3);
            assert(f.scan_not(F::cntrl, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::upper, in.data(), in.data() + in.size()) - in.data() == 1);
            assert(f.scan_not(F::lower, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::alpha, in.data(), in.data() + in.size()) - in.data() == 1);
            assert(f.scan_not(F::digit, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::punct, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::xdigit, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::blank, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::alnum, in.data(), in.data() + in.size()) - in.data() == 1);
            assert(f.scan_not(F::graph, in.data(), in.data() + in.size()) - in.data() == 1);
        }
    }
    {
        std::locale l("C");
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
            const std::wstring in(L"\x00DA A\x07.a1");
            std::vector<F::mask> m(in.size());
            assert(f.scan_not(F::space, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::print, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::cntrl, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::upper, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::lower, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::alpha, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::digit, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::punct, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::xdigit, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::blank, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::alnum, in.data(), in.data() + in.size()) - in.data() == 0);
            assert(f.scan_not(F::graph, in.data(), in.data() + in.size()) - in.data() == 0);
        }
    }
}
