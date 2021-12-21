//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class money_put<charT, OutputIterator>

// iter_type put(iter_type s, bool intl, ios_base& f, char_type fill,
//               long double units) const;

// REQUIRES: locale.en_US.UTF-8

#include <locale>
#include <ios>
#include <streambuf>
#include <cassert>
#include "test_iterators.h"

#include "platform_support.h" // locale name macros

typedef std::money_put<char, output_iterator<char*> > Fn;

class my_facet
    : public Fn
{
public:
    explicit my_facet(std::size_t refs = 0)
        : Fn(refs) {}
};

typedef std::money_put<wchar_t, output_iterator<wchar_t*> > Fw;

class my_facetw
    : public Fw
{
public:
    explicit my_facetw(std::size_t refs = 0)
        : Fw(refs) {}
};

int main()
{
    std::ios ios(0);
    std::string loc_name(LOCALE_en_US_UTF_8);
    ios.imbue(std::locale(ios.getloc(),
                          new std::moneypunct_byname<char, false>(loc_name)));
    ios.imbue(std::locale(ios.getloc(),
                          new std::moneypunct_byname<char, true>(loc_name)));
    ios.imbue(std::locale(ios.getloc(),
                          new std::moneypunct_byname<wchar_t, false>(loc_name)));
    ios.imbue(std::locale(ios.getloc(),
                          new std::moneypunct_byname<wchar_t, true>(loc_name)));
{
    const my_facet f(1);
    // char, national
    {   // zero
        long double v = 0;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "0.00");
    }
    {   // negative one
        long double v = -1;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-0.01");
    }
    {   // positive
        long double v = 123456789;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "1,234,567.89");
    }
    {   // negative
        long double v = -123456789;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-1,234,567.89");
    }
    {   // zero, showbase
        long double v = 0;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "$0.00");
    }
    {   // negative one, showbase
        long double v = -1;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-$0.01");
    }
    {   // positive, showbase
        long double v = 123456789;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "$1,234,567.89");
    }
    {   // negative, showbase
        long double v = -123456789;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-$1,234,567.89");
    }
    {   // negative, showbase, left
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        left(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, ' ', v);
        std::string ex(str, iter.base());
        assert(ex == "-$1,234,567.89      ");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, internal
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        internal(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, ' ', v);
        std::string ex(str, iter.base());
        assert(ex == "-$      1,234,567.89");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, right
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        right(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            false, ios, ' ', v);
        std::string ex(str, iter.base());
        assert(ex == "      -$1,234,567.89");
        assert(ios.width() == 0);
    }

    // char, international
    noshowbase(ios);
    ios.unsetf(std::ios_base::adjustfield);
    {   // zero
        long double v = 0;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "0.00");
    }
    {   // negative one
        long double v = -1;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-0.01");
    }
    {   // positive
        long double v = 123456789;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "1,234,567.89");
    }
    {   // negative
        long double v = -123456789;
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-1,234,567.89");
    }
    {   // zero, showbase
        long double v = 0;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "USD 0.00");
    }
    {   // negative one, showbase
        long double v = -1;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-USD 0.01");
    }
    {   // positive, showbase
        long double v = 123456789;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "USD 1,234,567.89");
    }
    {   // negative, showbase
        long double v = -123456789;
        showbase(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, '*', v);
        std::string ex(str, iter.base());
        assert(ex == "-USD 1,234,567.89");
    }
    {   // negative, showbase, left
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        left(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, ' ', v);
        std::string ex(str, iter.base());
        assert(ex == "-USD 1,234,567.89   ");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, internal
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        internal(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, ' ', v);
        std::string ex(str, iter.base());
        assert(ex == "-USD    1,234,567.89");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, right
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        right(ios);
        char str[100];
        output_iterator<char*> iter = f.put(output_iterator<char*>(str),
                                            true, ios, ' ', v);
        std::string ex(str, iter.base());
        assert(ex == "   -USD 1,234,567.89");
        assert(ios.width() == 0);
    }
}
{

    const my_facetw f(1);
    // wchar_t, national
    noshowbase(ios);
    ios.unsetf(std::ios_base::adjustfield);
    {   // zero
        long double v = 0;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"0.00");
    }
    {   // negative one
        long double v = -1;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-0.01");
    }
    {   // positive
        long double v = 123456789;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"1,234,567.89");
    }
    {   // negative
        long double v = -123456789;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-1,234,567.89");
    }
    {   // zero, showbase
        long double v = 0;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"$0.00");
    }
    {   // negative one, showbase
        long double v = -1;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-$0.01");
    }
    {   // positive, showbase
        long double v = 123456789;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"$1,234,567.89");
    }
    {   // negative, showbase
        long double v = -123456789;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-$1,234,567.89");
    }
    {   // negative, showbase, left
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        left(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, ' ', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-$1,234,567.89      ");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, internal
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        internal(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, ' ', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-$      1,234,567.89");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, right
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        right(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            false, ios, ' ', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"      -$1,234,567.89");
        assert(ios.width() == 0);
    }

    // wchar_t, international
    noshowbase(ios);
    ios.unsetf(std::ios_base::adjustfield);
    {   // zero
        long double v = 0;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"0.00");
    }
    {   // negative one
        long double v = -1;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-0.01");
    }
    {   // positive
        long double v = 123456789;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"1,234,567.89");
    }
    {   // negative
        long double v = -123456789;
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-1,234,567.89");
    }
    {   // zero, showbase
        long double v = 0;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"USD 0.00");
    }
    {   // negative one, showbase
        long double v = -1;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-USD 0.01");
    }
    {   // positive, showbase
        long double v = 123456789;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"USD 1,234,567.89");
    }
    {   // negative, showbase
        long double v = -123456789;
        showbase(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, '*', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-USD 1,234,567.89");
    }
    {   // negative, showbase, left
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        left(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, ' ', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-USD 1,234,567.89   ");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, internal
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        internal(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, ' ', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"-USD    1,234,567.89");
        assert(ios.width() == 0);
    }
    {   // negative, showbase, right
        long double v = -123456789;
        showbase(ios);
        ios.width(20);
        right(ios);
        wchar_t str[100];
        output_iterator<wchar_t*> iter = f.put(output_iterator<wchar_t*>(str),
                                            true, ios, ' ', v);
        std::wstring ex(str, iter.base());
        assert(ex == L"   -USD 1,234,567.89");
        assert(ios.width() == 0);
    }
}
}
