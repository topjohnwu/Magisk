//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class ctype<char>

// const mask* table() const throw();

#include <locale>
#include <cassert>

int main()
{
    typedef std::ctype<char> F;
    {
        std::locale l(std::locale::classic(), new std::ctype<char>);
        const F& f = std::use_facet<F>(l);
        assert(f.table() == f.classic_table());
    }
    {
        std::ctype<char>::mask table[256];
        std::locale l(std::locale::classic(), new std::ctype<char>(table));
        const F& f = std::use_facet<F>(l);
        assert(f.table() == table);
    }
}
