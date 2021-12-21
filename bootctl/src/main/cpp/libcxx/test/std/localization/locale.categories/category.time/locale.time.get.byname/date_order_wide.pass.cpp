//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8
// REQUIRES: locale.ru_RU.UTF-8
// REQUIRES: locale.zh_CN.UTF-8

// <locale>

// class time_get_byname<charT, InputIterator>

// dateorder date_order() const;

#include <locale>
#include <cassert>
#include "test_iterators.h"

#include "platform_support.h" // locale name macros

typedef std::time_get_byname<wchar_t, input_iterator<const wchar_t*> > F;

class my_facet
    : public F
{
public:
    explicit my_facet(const std::string& nm, std::size_t refs = 0)
        : F(nm, refs) {}
};

int main()
{
    {
        const my_facet f(LOCALE_en_US_UTF_8, 1);
        assert(f.date_order() == std::time_base::mdy);
    }
    {
        const my_facet f(LOCALE_fr_FR_UTF_8, 1);
        assert(f.date_order() == std::time_base::dmy);
    }
    {
        const my_facet f(LOCALE_ru_RU_UTF_8, 1);
        assert(f.date_order() == std::time_base::dmy);
    }
    {
        const my_facet f(LOCALE_zh_CN_UTF_8, 1);
        assert(f.date_order() == std::time_base::ymd);
    }
}
