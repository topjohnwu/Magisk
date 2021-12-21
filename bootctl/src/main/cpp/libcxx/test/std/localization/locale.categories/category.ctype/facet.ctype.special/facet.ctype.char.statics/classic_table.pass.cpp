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

// static const mask* classic_table() throw();

#include <locale>
#include <cassert>

int main()
{
    typedef std::ctype<char> F;
    assert(F::classic_table() != 0);
    assert(F::table_size >= 256);

    typedef F::mask mask;
    const mask *p = F::classic_table();
    const mask defined = F::space | F::print | F::cntrl | F::upper | F::lower
                    | F::alpha | F::digit | F::punct | F::xdigit | F::blank;

    for ( size_t i = 0; i < 128; ++i ) // values above 128 are not consistent
    {
        mask set = 0;

        if ( i  < 32  || i  > 126 ) set |= F::cntrl;
        if ( i >= 32  && i <= 126 ) set |= F::print;

        if (( i >= 9 && i <= 13) || i == 32 ) set |= F::space;
        if ( i == 9 || i == 32 ) set |= F::blank;

        if ( i >= 'A' && i <= 'Z' ) set |= F::alpha;
        if ( i >= 'a' && i <= 'z' ) set |= F::alpha;
        if ( i >= 'A' && i <= 'Z' ) set |= F::upper;
        if ( i >= 'a' && i <= 'z' ) set |= F::lower;

        if ( i >= '0' && i <= '9' ) set |= F::digit;
        if ( i >= '0' && i <= '9' ) set |= F::xdigit;
        if ( i >= 'A' && i <= 'F' ) set |= F::xdigit;
        if ( i >= 'a' && i <= 'f' ) set |= F::xdigit;

        if ( i >=  33 && i <=  47 ) set |= F::punct;    // ' ' .. '/'
        if ( i >=  58 && i <=  64 ) set |= F::punct;    // ':' .. '@'
        if ( i >=  91 && i <=  96 ) set |= F::punct;    // '[' .. '`'
        if ( i >= 123 && i <= 126 ) set |= F::punct;    // '{' .. '~'    }

        assert(( p[i] &  set) == set);            // all the right bits set
        assert(((p[i] & ~set) & defined) == 0);   // no extra ones
    }

}
