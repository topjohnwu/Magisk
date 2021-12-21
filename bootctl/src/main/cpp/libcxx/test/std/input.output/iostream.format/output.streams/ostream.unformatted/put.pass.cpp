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

// basic_ostream<charT,traits>& put(char_type c);

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
        wchar_t c = L'a';
        os.put(c);
        assert(os.bad());
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        wchar_t c = L'a';
        os.put(c);
        assert(sb.str() == L"a");
        assert(os.good());
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        char c = 'a';
        os.put(c);
        assert(sb.str() == "a");
        assert(os.good());
    }
}
