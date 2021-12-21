//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class ctype<char>;

// const char* scan_not(mask m, const char* low, const char* high) const;

#include <locale>
#include <string>
#include <vector>
#include <cassert>

#include <stdio.h>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<char> F;
        const F& f = std::use_facet<F>(l);
        const std::string in(" A\x07.a1");
        std::vector<F::mask> m(in.size());
        assert(f.scan_not(F::space, in.data(), in.data() + in.size()) - in.data() == 1);
        assert(f.scan_not(F::print, in.data(), in.data() + in.size()) - in.data() == 2);
        assert(f.scan_not(F::cntrl, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::upper, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::lower, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::alpha, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::digit, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::punct, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::xdigit, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::blank, in.data(), in.data() + in.size()) - in.data() == 1);
        assert(f.scan_not(F::alnum, in.data(), in.data() + in.size()) - in.data() == 0);
        assert(f.scan_not(F::graph, in.data(), in.data() + in.size()) - in.data() == 0);
    }
}
