//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <locale>

// template <class CharT>
// class ctype_byname
//     : public ctype<CharT>
// {
// public:
//     explicit ctype_byname(const char*, size_t = 0);
//     explicit ctype_byname(const string&, size_t = 0);
//
// protected:
//     ~ctype_byname();
// };

#include <locale>
#include <type_traits>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            assert(std::has_facet<std::ctype_byname<char> >(l));
            assert(&std::use_facet<std::ctype<char> >(l)
                == &std::use_facet<std::ctype_byname<char> >(l));
        }
        {
            assert(std::has_facet<std::ctype_byname<wchar_t> >(l));
            assert(&std::use_facet<std::ctype<wchar_t> >(l)
                == &std::use_facet<std::ctype_byname<wchar_t> >(l));
        }
    }
    {
        std::locale l("C");
        {
            assert(std::has_facet<std::ctype_byname<char> >(l));
            assert(&std::use_facet<std::ctype<char> >(l)
                == &std::use_facet<std::ctype_byname<char> >(l));
        }
        {
            assert(std::has_facet<std::ctype_byname<wchar_t> >(l));
            assert(&std::use_facet<std::ctype<wchar_t> >(l)
                == &std::use_facet<std::ctype_byname<wchar_t> >(l));
        }
    }
}
