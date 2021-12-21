//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8
// REQUIRES: locale.ru_RU.UTF-8
// REQUIRES: locale.zh_CN.UTF-8

// class moneypunct_byname<charT, International>

// int frac_digits() const;

#include <locale>
#include <limits>
#include <cassert>

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
        assert(f.frac_digits() == 0);
    }
    {
        Fnt f("C", 1);
        assert(f.frac_digits() == 0);
    }
    {
        Fwf f("C", 1);
        assert(f.frac_digits() == 0);
    }
    {
        Fwt f("C", 1);
        assert(f.frac_digits() == 0);
    }

    {
        Fnf f(LOCALE_en_US_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fnt f(LOCALE_en_US_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwf f(LOCALE_en_US_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwt f(LOCALE_en_US_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }

    {
        Fnf f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fnt f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwf f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwt f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }

    {
        Fnf f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fnt f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwf f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwt f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }

    {
        Fnf f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fnt f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwf f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
    {
        Fwt f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.frac_digits() == 2);
    }
}
