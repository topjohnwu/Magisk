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
//   string_type
//   transform_primary(ForwardIterator first, ForwardIterator last) const;

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "platform_support.h" // locale name macros

int main()
{
    {
        std::regex_traits<char> t;
        const char A[] = "A";
        const char Aacute[] = "\xC1";
        typedef forward_iterator<const char*> F;
        assert(t.transform_primary(F(A), F(A+1)) !=
               t.transform_primary(F(Aacute), F(Aacute+1)));
        t.imbue(std::locale(LOCALE_cs_CZ_ISO8859_2));
        assert(t.transform_primary(F(A), F(A+1)) ==
               t.transform_primary(F(Aacute), F(Aacute+1)));
    }
    {
        std::regex_traits<wchar_t> t;
        const wchar_t A[] = L"A";
        const wchar_t Aacute[] = L"\xC1";
        typedef forward_iterator<const wchar_t*> F;
        assert(t.transform_primary(F(A), F(A+1)) !=
               t.transform_primary(F(Aacute), F(Aacute+1)));
        t.imbue(std::locale(LOCALE_cs_CZ_ISO8859_2));
        assert(t.transform_primary(F(A), F(A+1)) ==
               t.transform_primary(F(Aacute), F(Aacute+1)));
    }
}
