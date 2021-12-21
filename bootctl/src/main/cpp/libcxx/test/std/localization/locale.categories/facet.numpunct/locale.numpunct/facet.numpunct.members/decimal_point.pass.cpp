//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class numpunct;

// char_type decimal_point() const;

#include <locale>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef char C;
        const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
        assert(np.decimal_point() == '.');
    }
    {
        typedef wchar_t C;
        const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
        assert(np.decimal_point() == L'.');
    }
}
