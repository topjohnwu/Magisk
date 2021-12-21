//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class Facet> locale combine(const locale& other) const;

#include <locale>
#include <stdexcept>
#include <cassert>

#include "count_new.hpp"

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

struct my_facet
    : public std::locale::facet
{
    int test() const {return 5;}

    static std::locale::id id;
};

std::locale::id my_facet::id;

int main()
{
{
    {
        std::locale loc;
        std::locale loc2(loc, new my_facet);
        std::locale loc3 = loc.combine<my_facet>(loc2);
        check(loc3);
        assert(loc3.name() == "*");
        assert((std::has_facet<my_facet>(loc3)));
        const my_facet& f = std::use_facet<my_facet>(loc3);
        assert(f.test() == 5);
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}
#ifndef TEST_HAS_NO_EXCEPTIONS
{
    {
        std::locale loc;
        std::locale loc2;
        try
        {
            std::locale loc3 = loc.combine<my_facet>(loc2);
            assert(false);
        }
        catch (std::runtime_error&)
        {
        }
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}
#endif
}
