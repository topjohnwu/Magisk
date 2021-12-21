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

// GLIBC Expects "10/06/2009" for fr_FR as opposed to "10.06.2009"
// GLIBC also fails on the zh_CN test.
// XFAIL: linux

// <locale>

// class time_get_byname<charT, InputIterator>

// iter_type
// get_date(iter_type s, iter_type end, ios_base& str,
//          ios_base::iostate& err, tm* t) const;

#include <locale>
#include <cassert>
#include "test_iterators.h"

#include "platform_support.h" // locale name macros

typedef input_iterator<const wchar_t*> I;

typedef std::time_get_byname<wchar_t, I> F;

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
        const wchar_t in[] = L"06/10/2009";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_date(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_mon == 5);
        assert(t.tm_mday == 10);
        assert(t.tm_year == 109);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_fr_FR_UTF_8, 1);
        const wchar_t in[] = L"10.06.2009";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_date(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_mon == 5);
        assert(t.tm_mday == 10);
        assert(t.tm_year == 109);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_ru_RU_UTF_8, 1);
        const wchar_t in[] = L"10.06.2009";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_date(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_mon == 5);
        assert(t.tm_mday == 10);
        assert(t.tm_year == 109);
        assert(err == std::ios_base::eofbit);
    }
    {
        const my_facet f(LOCALE_zh_CN_UTF_8, 1);
        const wchar_t in[] = L"2009/06/10";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_date(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_mon == 5);
        assert(t.tm_mday == 10);
        assert(t.tm_year == 109);
        assert(err == std::ios_base::eofbit);
    }
}
