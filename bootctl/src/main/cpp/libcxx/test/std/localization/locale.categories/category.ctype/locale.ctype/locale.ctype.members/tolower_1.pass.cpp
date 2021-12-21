//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class ctype;

// charT tolower(charT) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<wchar_t> F;
        const F& f = std::use_facet<F>(l);

        assert(f.tolower(L' ') == L' ');
        assert(f.tolower(L'A') == L'a');
        assert(f.tolower(L'\x07') == L'\x07');
        assert(f.tolower(L'.') == L'.');
        assert(f.tolower(L'a') == L'a');
        assert(f.tolower(L'1') == L'1');
    }
}
