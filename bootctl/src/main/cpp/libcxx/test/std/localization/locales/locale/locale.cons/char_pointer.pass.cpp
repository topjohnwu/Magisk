//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support most of LC_* at the moment
// XFAIL: netbsd

// REQUIRES: locale.ru_RU.UTF-8
// REQUIRES: locale.zh_CN.UTF-8

// <locale>

// explicit locale(const char* std_name);

#include <locale>
#include <new>
#include <cassert>

#include "count_new.hpp"
#include "platform_support.h" // locale name macros

#include "test_macros.h"


void check(const std::locale& loc)
{
    assert(std::has_facet<std::collate<char> >(loc));
    assert(std::has_facet<std::collate<wchar_t> >(loc));

    assert(std::has_facet<std::ctype<char> >(loc));
    assert(std::has_facet<std::ctype<wchar_t> >(loc));
    assert((std::has_facet<std::codecvt<char, char, std::mbstate_t> >(loc)));
    assert((std::has_facet<std::codecvt<char16_t, char, std::mbstate_t> >(loc)));
    assert((std::has_facet<std::codecvt<char32_t, char, std::mbstate_t> >(loc)));
    assert((std::has_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(loc)));

    assert((std::has_facet<std::moneypunct<char> >(loc)));
    assert((std::has_facet<std::moneypunct<wchar_t> >(loc)));
    assert((std::has_facet<std::money_get<char> >(loc)));
    assert((std::has_facet<std::money_get<wchar_t> >(loc)));
    assert((std::has_facet<std::money_put<char> >(loc)));
    assert((std::has_facet<std::money_put<wchar_t> >(loc)));

    assert((std::has_facet<std::numpunct<char> >(loc)));
    assert((std::has_facet<std::numpunct<wchar_t> >(loc)));
    assert((std::has_facet<std::num_get<char> >(loc)));
    assert((std::has_facet<std::num_get<wchar_t> >(loc)));
    assert((std::has_facet<std::num_put<char> >(loc)));
    assert((std::has_facet<std::num_put<wchar_t> >(loc)));

    assert((std::has_facet<std::time_get<char> >(loc)));
    assert((std::has_facet<std::time_get<wchar_t> >(loc)));
    assert((std::has_facet<std::time_put<char> >(loc)));
    assert((std::has_facet<std::time_put<wchar_t> >(loc)));

    assert((std::has_facet<std::messages<char> >(loc)));
    assert((std::has_facet<std::messages<wchar_t> >(loc)));
}

int main()
{
    {
        std::locale loc(LOCALE_ru_RU_UTF_8);
        check(loc);
        std::locale loc2(LOCALE_ru_RU_UTF_8);
        check(loc2);
        assert(loc == loc2);
        std::locale loc3(LOCALE_zh_CN_UTF_8);
        check(loc3);
        assert(!(loc == loc3));
        assert(loc != loc3);
#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            std::locale((const char*)0);
            assert(false);
        }
        catch (std::runtime_error&)
        {
        }
        try
        {
            std::locale("spazbot");
            assert(false);
        }
        catch (std::runtime_error&)
        {
        }
#endif
        std::locale ok("");
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}
