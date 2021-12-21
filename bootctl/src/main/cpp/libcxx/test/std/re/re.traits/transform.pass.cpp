// -*- C++ -*-
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

// REQUIRES: locale.cs_CZ.ISO8859-2

// <regex>

// template <class charT> struct regex_traits;

// template <class ForwardIterator>
//   string_type transform(ForwardIterator first, ForwardIterator last) const;

#include <regex>
#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"
#include "platform_support.h" // locale name macros

int main()
{
    {
        std::regex_traits<char> t;
        const char a[] = "a";
        const char B[] = "B";
        typedef forward_iterator<const char*> F;
        assert(t.transform(F(a), F(a+1)) > t.transform(F(B), F(B+1)));
        t.imbue(std::locale(LOCALE_cs_CZ_ISO8859_2));
        assert(t.transform(F(a), F(a+1)) < t.transform(F(B), F(B+1)));
    }
    {
        std::regex_traits<wchar_t> t;
        const wchar_t a[] = L"a";
        const wchar_t B[] = L"B";
        typedef forward_iterator<const wchar_t*> F;
        assert(t.transform(F(a), F(a+1)) > t.transform(F(B), F(B+1)));
        t.imbue(std::locale(LOCALE_cs_CZ_ISO8859_2));
        assert(t.transform(F(a), F(a+1)) < t.transform(F(B), F(B+1)));
    }
}
