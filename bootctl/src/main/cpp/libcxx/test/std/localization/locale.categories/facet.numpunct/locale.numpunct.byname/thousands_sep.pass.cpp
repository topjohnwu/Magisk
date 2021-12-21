//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support LC_NUMERIC at the moment
// XFAIL: netbsd

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8

// <locale>

// template <class charT> class numpunct_byname;

// char_type thousands_sep() const;


#include <locale>
#include <cassert>
#include <iostream> // FIXME: for debugging purposes only

#include "test_macros.h"
#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l("C");
        {
            typedef char C;
            const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
            assert(np.thousands_sep() == ',');
        }
        {
            typedef wchar_t C;
            const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
            assert(np.thousands_sep() == L',');
        }
    }
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            typedef char C;
            const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
            assert(np.thousands_sep() == ',');
        }
        {
            typedef wchar_t C;
            const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
            assert(np.thousands_sep() == L',');
        }
    }
    {
        std::locale l(LOCALE_fr_FR_UTF_8);
#if defined(TEST_HAS_GLIBC)
        const char sep = ' ';
// The below tests work around GLIBC's use of U202F as LC_NUMERIC thousands_sep.
# if TEST_GLIBC_PREREQ(2, 27)
        const wchar_t wsep = L'\u202f';
# else
        const wchar_t wsep = L' ';
# endif
#else
        const char sep = ',';
        const wchar_t wsep = L',';
#endif
        {
            typedef char C;
            const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
            if (np.thousands_sep() != sep) {
              std::cout << "np.thousands_sep() = '" << np.thousands_sep() << "'\n";
              std::cout << "sep = '" << sep << "'\n";
              std::cout << std::endl;
            }
            assert(np.thousands_sep() == sep);
        }
        {
            typedef wchar_t C;
            const std::numpunct<C>& np = std::use_facet<std::numpunct<C> >(l);
            assert(np.thousands_sep() == wsep);
        }
    }
}
