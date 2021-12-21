//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <ostream>

// template <class charT, class traits = char_traits<charT> >
//   class basic_ostream;

// template <class charT, class traits, class T>
//   basic_ostream<charT, traits>&
//   operator<<(basic_ostream<charT, traits>&& os, const T& x);

#include <ostream>
#include <cassert>


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
        std::ostream(&sb) << "testing...";
        assert(sb.str() == "testing...");
    }
    {
        testbuf<wchar_t> sb;
        std::wostream(&sb) << L"123";
        assert(sb.str() == L"123");
    }
}
