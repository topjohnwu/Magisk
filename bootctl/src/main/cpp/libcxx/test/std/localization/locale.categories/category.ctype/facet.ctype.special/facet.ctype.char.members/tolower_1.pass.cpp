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

// char tolower(char) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<char> F;
        const F& f = std::use_facet<F>(l);

        assert(f.tolower(' ') == ' ');
        assert(f.tolower('A') == 'a');
        assert(f.tolower('\x07') == '\x07');
        assert(f.tolower('.') == '.');
        assert(f.tolower('a') == 'a');
        assert(f.tolower('1') == '1');
    }
}
