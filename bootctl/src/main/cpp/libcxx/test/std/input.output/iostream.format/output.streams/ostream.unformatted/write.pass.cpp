//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ostream>

// template <class charT, class traits = char_traits<charT> >
//   class basic_ostream;

// basic_ostream& write(const char_type* s, streamsize n);

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
        std::wostream os((std::wstreambuf*)0);
        const wchar_t s[] = L"123456790";
        os.write(s, sizeof(s)/sizeof(s[0])-1);
        assert(os.bad());
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        const wchar_t s[] = L"123456790";
        os.write(s, sizeof(s)/sizeof(s[0])-1);
        assert(os.good());
        assert(sb.str() == s);
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        const char s[] = "123456790";
        os.write(s, sizeof(s)/sizeof(s[0])-1);
        assert(sb.str() == s);
        assert(os.good());
    }
}
