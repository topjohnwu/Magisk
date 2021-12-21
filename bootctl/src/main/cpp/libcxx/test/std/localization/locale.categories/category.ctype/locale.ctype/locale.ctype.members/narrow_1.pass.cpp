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

// char narrow(charT c, char dfault) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<wchar_t> F;
        const F& f = std::use_facet<F>(l);

        assert(f.narrow(L' ', '*') == ' ');
        assert(f.narrow(L'A', '*') == 'A');
        assert(f.narrow(L'\x07', '*') == '\x07');
        assert(f.narrow(L'.', '*') == '.');
        assert(f.narrow(L'a', '*') == 'a');
        assert(f.narrow(L'1', '*') == '1');
    }
}
