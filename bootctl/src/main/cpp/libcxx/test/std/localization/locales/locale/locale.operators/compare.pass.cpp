//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT, class Traits, class Allocator>
//   bool operator()(const basic_string<charT,Traits,Allocator>& s1,
//                   const basic_string<charT,Traits,Allocator>& s2) const;

#include <locale>
#include <cassert>

int main()
{
    {
        std::locale l;
        {
            std::string s2("aaaaaaA");
            std::string s3("BaaaaaA");
            assert(l(s3, s2));
        }
        {
            std::wstring s2(L"aaaaaaA");
            std::wstring s3(L"BaaaaaA");
            assert(l(s3, s2));
        }
    }
}
