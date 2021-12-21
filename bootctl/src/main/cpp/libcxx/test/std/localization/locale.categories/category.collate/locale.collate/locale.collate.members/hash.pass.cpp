//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class collate;

// long hash(const charT* low, const charT* high) const;

//   This test is not portable

#include <locale>
#include <string>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
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
