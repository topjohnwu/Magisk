//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <iomanip>

// template <class charT> T10 put_time(const struct tm* tmb, const charT* fmt);

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
                int n = static_cast<int>(str_.size());
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
        std::tm t = {};
        t.tm_sec = 59;
        t.tm_min = 55;
        t.tm_hour = 23;
        t.tm_mday = 31;
        t.tm_mon = 11;
        t.tm_year = 161;
        t.tm_wday = 6;
        t.tm_isdst = 0;
        os << std::put_time(&t, "%a %b %d %H:%M:%S %Y");
        assert(sb.str() == "Sat Dec 31 23:55:59 2061");
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        os.imbue(std::locale(LOCALE_en_US_UTF_8));
        std::tm t = {};
        t.tm_sec = 59;
        t.tm_min = 55;
        t.tm_hour = 23;
        t.tm_mday = 31;
        t.tm_mon = 11;
        t.tm_year = 161;
        t.tm_wday = 6;
        os << std::put_time(&t, L"%a %b %d %H:%M:%S %Y");
        assert(sb.str() == L"Sat Dec 31 23:55:59 2061");
    }
}
