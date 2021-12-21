//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class codecvt<char32_t, char, mbstate_t>

// bool always_noconv() const throw();

#include <locale>
#include <cassert>

typedef std::codecvt<char32_t, char, std::mbstate_t> F;

int main()
{
    std::locale l = std::locale::classic();
    const F& f = std::use_facet<F>(l);
    assert(!f.always_noconv());
}
