//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support LC_MONETARY at the moment
// XFAIL: netbsd

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8
// REQUIRES: locale.ru_RU.UTF-8
// REQUIRES: locale.zh_CN.UTF-8

// <locale>

// class moneypunct_byname<charT, International>

// charT thousands_sep() const;

#include <locale>
#include <limits>
#include <cassert>

#include "test_macros.h"
#include "platform_support.h" // locale name macros

class Fnf
    : public std::moneypunct_byname<char, false>
{
public:
    explicit Fnf(const std::string& nm, std::size_t refs = 0)
        : std::moneypunct_byname<char, false>(nm, refs) {}
};

class Fnt
    : public std::moneypunct_byname<char, true>
{
public:
    explicit Fnt(const std::string& nm, std::size_t refs = 0)
        : std::moneypunct_byname<char, true>(nm, refs) {}
};

class Fwf
    : public std::moneypunct_byname<wchar_t, false>
{
public:
    explicit Fwf(const std::string& nm, std::size_t refs = 0)
        : std::moneypunct_byname<wchar_t, false>(nm, refs) {}
};

class Fwt
    : public std::moneypunct_byname<wchar_t, true>
{
public:
    explicit Fwt(const std::string& nm, std::size_t refs = 0)
        : std::moneypunct_byname<wchar_t, true>(nm, refs) {}
};

int main()
{
    {
        Fnf f("C", 1);
        assert(f.thousands_sep() == std::numeric_limits<char>::max());
    }
    {
        Fnt f("C", 1);
        assert(f.thousands_sep() == std::numeric_limits<char>::max());
    }
    {
        Fwf f("C", 1);
        assert(f.thousands_sep() == std::numeric_limits<wchar_t>::max());
    }
    {
        Fwt f("C", 1);
        assert(f.thousands_sep() == std::numeric_limits<wchar_t>::max());
    }

    {
        Fnf f(LOCALE_en_US_UTF_8, 1);
        assert(f.thousands_sep() == ',');
    }
    {
        Fnt f(LOCALE_en_US_UTF_8, 1);
        assert(f.thousands_sep() == ',');
    }
    {
        Fwf f(LOCALE_en_US_UTF_8, 1);
        assert(f.thousands_sep() == L',');
    }
    {
        Fwt f(LOCALE_en_US_UTF_8, 1);
        assert(f.thousands_sep() == L',');
    }
    {
        Fnf f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.thousands_sep() == ' ');
    }
    {
        Fnt f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.thousands_sep() == ' ');
    }
// The below tests work around GLIBC's use of U202F as mon_thousands_sep.
#ifndef TEST_GLIBC_PREREQ
#define TEST_GLIBC_PREREQ(x, y) 0
#endif
#if defined(TEST_HAS_GLIBC) && TEST_GLIBC_PREREQ(2, 27)
    const wchar_t fr_sep = L'\u202F';
#else
    const wchar_t fr_sep = L' ';
#endif
    {
        Fwf f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.thousands_sep() == fr_sep);
    }
    {
        Fwt f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.thousands_sep() == fr_sep);
    }
// The below tests work around GLIBC's use of U00A0 as mon_thousands_sep
// and U002E as mon_decimal_point.
// TODO: Fix thousands_sep for 'char'.
// related to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=16006
#ifndef TEST_HAS_GLIBC
    const char sep = ' ';
    const wchar_t wsep = L' ';
#elif TEST_GLIBC_PREREQ(2, 27)
    // FIXME libc++ specifically works around \u00A0 by translating it into
    // a regular space.
    const char sep = ' ';
    const wchar_t wsep = L'\u202F';
#else
    // FIXME libc++ specifically works around \u00A0 by translating it into
    // a regular space.
    const char sep = ' ';
    const wchar_t wsep = L'\u00A0';
#endif
    {
        Fnf f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.thousands_sep() == sep);
    }
    {
        Fnt f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.thousands_sep() == sep);
    }
    {
        Fwf f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.thousands_sep() == wsep);
    }
    {
        Fwt f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.thousands_sep() == wsep);
    }

    {
        Fnf f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.thousands_sep() == ',');
    }
    {
        Fnt f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.thousands_sep() == ',');
    }
    {
        Fwf f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.thousands_sep() == L',');
    }
    {
        Fwt f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.thousands_sep() == L',');
    }
}
