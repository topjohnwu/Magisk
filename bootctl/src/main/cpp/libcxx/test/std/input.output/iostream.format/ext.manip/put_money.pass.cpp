//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iomanip>

// template <class charT, class moneyT> T8 put_money(const moneyT& mon, bool intl = false);

// REQUIRES: locale.en_US.UTF-8

#include <iomanip>
#include <ostream>
#include <cassert>

#include "platform_support.h" // locale name macros

template <class CharT>
class testbuf
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;
    std::basic_string<CharT> str_;
public:
    testbuf()
    {
    }

    std::basic_string<CharT> str() const
        {return std::basic_string<CharT>(base::pbase(), base::pptr());}

protected:

    virtual typename base::int_type
        overflow(typename base::int_type ch = base::traits_type::eof())
        {
            if (ch != base::traits_type::eof())
            {
                int n = str_.size();
                str_.push_back(static_cast<CharT>(ch));
                str_.resize(str_.capacity());
                base::setp(const_cast<CharT*>(str_.data()),
                           const_cast<CharT*>(str_.data() + str_.size()));
                base::pbump(n+1);
            }
            return ch;
        }
};

int main()
{
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        os.imbue(std::locale(LOCALE_en_US_UTF_8));
        showbase(os);
        long double x = -123456789;
        os << std::put_money(x, false);
        assert(sb.str() == "-$1,234,567.89");
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        os.imbue(std::locale(LOCALE_en_US_UTF_8));
        showbase(os);
        long double x = -123456789;
        os << std::put_money(x, true);
        assert(sb.str() == "-USD 1,234,567.89");
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        os.imbue(std::locale(LOCALE_en_US_UTF_8));
        showbase(os);
        long double x = -123456789;
        os << std::put_money(x, false);
        assert(sb.str() == L"-$1,234,567.89");
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        os.imbue(std::locale(LOCALE_en_US_UTF_8));
        showbase(os);
        long double x = -123456789;
        os << std::put_money(x, true);
        assert(sb.str() == L"-USD 1,234,567.89");
    }
}
