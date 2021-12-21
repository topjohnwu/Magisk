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

// charT widen(char c) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<wchar_t> F;
        const F& f = std::use_facet<F>(l);

        assert(f.widen(' ') == L' ');
        assert(f.widen('A') == L'A');
        assert(f.widen('\x07') == L'\x07');
        assert(f.widen('.') == L'.');
        assert(f.widen('a') == L'a');
        assert(f.widen('1') == L'1');
    }
}
