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

// char narrow(char c, char dfault) const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<char> F;
        const F& f = std::use_facet<F>(l);

        assert(f.narrow(' ', '*') == ' ');
        assert(f.narrow('A', '*') == 'A');
        assert(f.narrow('\x07', '*') == '\x07');
        assert(f.narrow('.', '*') == '.');
        assert(f.narrow('a', '*') == 'a');
        assert(f.narrow('1', '*') == '1');
    }
}
