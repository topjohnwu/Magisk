//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class codecvt<char16_t, char, mbstate_t>

// int length(stateT& state, const externT* from, const externT* from_end, size_t max) const;

#include <locale>
#include <cassert>

typedef std::codecvt<char16_t, char, std::mbstate_t> F;

int main()
{
    std::locale l = std::locale::classic();
    const F& f = std::use_facet<F>(l);
    std::mbstate_t mbs = {};
    const char from[] = "some text";
    assert(f.length(mbs, from, from+10, 0) == 0);
    assert(f.length(mbs, from, from+10, 8) == 8);
    assert(f.length(mbs, from, from+10, 9) == 9);
    assert(f.length(mbs, from, from+10, 10) == 10);
    assert(f.length(mbs, from, from+10, 100) == 10);
}
