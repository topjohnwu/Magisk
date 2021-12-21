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

// template <class charT> class collate_byname

// long hash(const charT* low, const charT* high) const;

//   This test is not portable

#include <locale>
#include <string>
#include <cassert>

#include "platform_support.h" // locale name macros

int main()
{
    std::locale l(LOCALE_en_US_UTF_8);
    {
        std::string x1("1234");
        std::string x2("12345");
        const std::collate<char>& f = std::use_facet<std::collate<char> >(l);
        assert(f.hash(x1.data(), x1.data() + x1.size())
            != f.hash(x2.data(), x2.data() + x2.size()));
    }
    {
        std::wstring x1(L"1234");
        std::wstring x2(L"12345");
        const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t> >(l);
        assert(f.hash(x1.data(), x1.data() + x1.size())
            != f.hash(x2.data(), x2.data() + x2.size()));
    }
}
