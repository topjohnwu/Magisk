//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support LC_COLLATE at the moment
// XFAIL: netbsd

// <locale>

// template <class charT> class collate_byname

// string_type transform(const charT* low, const charT* high) const;

// REQUIRES: locale.en_US.UTF-8

#include <locale>
#include <string>
#include <cassert>

#include <stdio.h>

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::locale l(LOCALE_en_US_UTF_8);
        {
            std::string x("1234");
            const std::collate<char>& f = std::use_facet<std::collate<char> >(l);
            assert(f.transform(x.data(), x.data() + x.size()) != x);
        }
        {
            std::wstring x(L"1234");
            const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t> >(l);
            assert(f.transform(x.data(), x.data() + x.size()) != x);
        }
    }
    {
        std::locale l("C");
        {
            std::string x("1234");
            const std::collate<char>& f = std::use_facet<std::collate<char> >(l);
            assert(f.transform(x.data(), x.data() + x.size()) == x);
        }
        {
            std::wstring x(L"1234");
            const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t> >(l);
            assert(f.transform(x.data(), x.data() + x.size()) == x);
        }
    }
}
