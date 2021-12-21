//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support LC_TIME at the moment
// XFAIL: netbsd

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8
// REQUIRES: locale.ru_RU.UTF-8
// REQUIRES: locale.zh_CN.UTF-8

// <locale>

// class time_get_byname<charT, InputIterator>

// iter_type get(iter_type s, iter_type end, ios_base& f,
//               ios_base::iostate& err, tm *t, char format, char modifier = 0) const;

// TODO: investigation needed
// XFAIL: linux-gnu

#include <locale>
#include <cassert>
#include "test_iterators.h"

#include "platform_support.h" // locale name macros

typedef input_iterator<const char*> I;

typedef std::time_get_byname<char, I> F;

class my_facet
    : public F
{
public:
    explicit my_facet(const std::string& nm, std::size_t refs = 0)
        : F(nm, refs) {}
};

int main()
{
    std::ios ios(0);
    std::ios_base::iostate err;
    std::tm t;
    {
        const my_facet f(LOCALE_en_US_UTF_8, 1);
        const char in[] = "Sat Dec 31 23:55:59 2061";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'c');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(t.tm_mday == 31);
        assert(t.tm_mon == 11);
        assert(t.tm_year == 161);
        assert(t.tm_wday == 6);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_en_US_UTF_8, 1);
        const char in[] = "23:55:59";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'X');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_fr_FR_UTF_8, 1);
        const char in[] = "Sam 31 d""\xC3\xA9""c 23:55:59 2061";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'c');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(t.tm_mday == 31);
        assert(t.tm_mon == 11);
        assert(t.tm_year == 161);
        assert(t.tm_wday == 6);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_fr_FR_UTF_8, 1);
        const char in[] = "23:55:59";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'X');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_ru_RU_UTF_8, 1);
        const char in[] = "\xD1\x81\xD1\x83\xD0\xB1\xD0\xB1"
                          "\xD0\xBE\xD1\x82\xD0\xB0"
                          ", 31 "
                          "\xD0\xB4\xD0\xB5\xD0\xBA\xD0\xB0"
                          "\xD0\xB1\xD1\x80\xD1\x8F"
                          " 2061 "
                          "\xD0\xB3"
                          ". 23:55:59";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'c');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(t.tm_mday == 31);
        assert(t.tm_mon == 11);
        assert(t.tm_year == 161);
        assert(t.tm_wday == 6);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_ru_RU_UTF_8, 1);
        const char in[] = "23:55:59";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'X');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_zh_CN_UTF_8, 1);
        const char in[] = "\xE5\x85\xAD"
                          " 12/31 23:55:59 2061";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'c');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(t.tm_mday == 31);
        assert(t.tm_mon == 11);
        assert(t.tm_year == 161);
        assert(t.tm_wday == 6);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_zh_CN_UTF_8, 1);
        const char in[] = "23""\xE6\x97\xB6""55""\xE5\x88\x86""59""\xE7\xA7\x92";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t, 'X');
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_sec == 59);
        assert(t.tm_min == 55);
        assert(t.tm_hour == 23);
        assert(err == std::ios_base::eofbit);
    }
}
