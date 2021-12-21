//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class money_get<charT, InputIterator>

// iter_type get(iter_type b, iter_type e, bool intl, ios_base& iob,
//               ios_base::iostate& err, string_type& v) const;

// REQUIRES: locale.en_US.UTF-8

#include <locale>
#include <ios>
#include <streambuf>
#include <cassert>
#include "test_iterators.h"

#include "platform_support.h" // locale name macros

typedef std::money_get<char, input_iterator<const char*> > Fn;

class my_facet
    : public Fn
{
public:
    explicit my_facet(std::size_t refs = 0)
        : Fn(refs) {}
};

typedef std::money_get<wchar_t, input_iterator<const wchar_t*> > Fw;

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
            std::string v = "0.00";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "0");
        }
        {   // negative one
            std::string v = "-0.01";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-1");
        }
        {   // positive
            std::string v = "1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "123456789");
        }
        {   // negative
            std::string v = "-1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-123456789");
        }
        {   // negative
            std::string v = "-1234567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-123456789");
        }
        {   // zero, showbase
            std::string v = "$0.00";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "0");
        }
        {   // zero, showbase
            std::string v = "$0.00";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "0");
            noshowbase(ios);
        }
        {   // negative one, showbase
            std::string v = "-$0.01";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-1");
        }
        {   // negative one, showbase
            std::string v = "-$0.01";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-1");
            noshowbase(ios);
        }
        {   // positive, showbase
            std::string v = "$1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "123456789");
        }
        {   // positive, showbase
            std::string v = "$1,234,567.89";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::string v = "-$1,234,567.89";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::string v = "-USD 1,234,567.89";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == "");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::string v = "-USD 1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == "");
        }
    }
    {
        const my_facet f(1);
        // char, international
        {   // zero
            std::string v = "0.00";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "0");
        }
        {   // negative one
            std::string v = "-0.01";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-1");
        }
        {   // positive
            std::string v = "1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "123456789");
        }
        {   // negative
            std::string v = "-1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-123456789");
        }
        {   // negative
            std::string v = "-1234567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-123456789");
        }
        {   // zero, showbase
            std::string v = "USD 0.00";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "0");
        }
        {   // zero, showbase
            std::string v = "USD 0.00";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "0");
            noshowbase(ios);
        }
        {   // negative one, showbase
            std::string v = "-USD 0.01";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-1");
        }
        {   // negative one, showbase
            std::string v = "-USD 0.01";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-1");
            noshowbase(ios);
        }
        {   // positive, showbase
            std::string v = "USD 1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "123456789");
        }
        {   // positive, showbase
            std::string v = "USD 1,234,567.89";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::string v = "-USD 1,234,567.89";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == "-123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::string v = "-$1,234,567.89";
            showbase(ios);
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == "");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::string v = "-$1,234,567.89";
            typedef input_iterator<const char*> I;
            std::string ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == "");
        }
    }
    {
        const my_facetw f(1);
        // wchar_t, national
        {   // zero
            std::wstring v = L"0.00";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"0");
        }
        {   // negative one
            std::wstring v = L"-0.01";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-1");
        }
        {   // positive
            std::wstring v = L"1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"123456789");
        }
        {   // negative
            std::wstring v = L"-1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-123456789");
        }
        {   // negative
            std::wstring v = L"-1234567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-123456789");
        }
        {   // zero, showbase
            std::wstring v = L"$0.00";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"0");
        }
        {   // zero, showbase
            std::wstring v = L"$0.00";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"0");
            noshowbase(ios);
        }
        {   // negative one, showbase
            std::wstring v = L"-$0.01";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-1");
        }
        {   // negative one, showbase
            std::wstring v = L"-$0.01";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-1");
            noshowbase(ios);
        }
        {   // positive, showbase
            std::wstring v = L"$1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"123456789");
        }
        {   // positive, showbase
            std::wstring v = L"$1,234,567.89";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::wstring v = L"-$1,234,567.89";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::wstring v = L"-USD 1,234,567.89";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == L"");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::wstring v = L"-USD 1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                false, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == L"");
        }
    }
    {
        const my_facetw f(1);
        // wchar_t, international
        {   // zero
            std::wstring v = L"0.00";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"0");
        }
        {   // negative one
            std::wstring v = L"-0.01";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-1");
        }
        {   // positive
            std::wstring v = L"1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"123456789");
        }
        {   // negative
            std::wstring v = L"-1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-123456789");
        }
        {   // negative
            std::wstring v = L"-1234567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-123456789");
        }
        {   // zero, showbase
            std::wstring v = L"USD 0.00";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"0");
        }
        {   // zero, showbase
            std::wstring v = L"USD 0.00";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"0");
            noshowbase(ios);
        }
        {   // negative one, showbase
            std::wstring v = L"-USD 0.01";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-1");
        }
        {   // negative one, showbase
            std::wstring v = L"-USD 0.01";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-1");
            noshowbase(ios);
        }
        {   // positive, showbase
            std::wstring v = L"USD 1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"123456789");
        }
        {   // positive, showbase
            std::wstring v = L"USD 1,234,567.89";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::wstring v = L"-USD 1,234,567.89";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + v.size());
            assert(err == std::ios_base::eofbit);
            assert(ex == L"-123456789");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::wstring v = L"-$1,234,567.89";
            showbase(ios);
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == L"");
            noshowbase(ios);
        }
        {   // negative, showbase
            std::wstring v = L"-$1,234,567.89";
            typedef input_iterator<const wchar_t*> I;
            std::wstring ex;
            std::ios_base::iostate err = std::ios_base::goodbit;
            I iter = f.get(I(v.data()), I(v.data() + v.size()),
                                                true, ios, err, ex);
            assert(iter.base() == v.data() + 1);
            assert(err == std::ios_base::failbit);
            assert(ex == L"");
        }
    }
}
