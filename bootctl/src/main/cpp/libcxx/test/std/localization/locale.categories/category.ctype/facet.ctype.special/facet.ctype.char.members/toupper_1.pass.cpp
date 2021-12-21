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

// char toupper(char) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<char> F;
        const F& f = std::use_facet<F>(l);

        assert(f.toupper(' ') == ' ');
        assert(f.toupper('A') == 'A');
        assert(f.toupper('\x07') == '\x07');
        assert(f.toupper('.') == '.');
        assert(f.toupper('a') == 'A');
        assert(f.toupper('1') == '1');
    }
}
