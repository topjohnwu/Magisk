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

// template<class char, class traits>
//   basic_ostream<char,traits>& operator<<(basic_ostream<char,traits>& out, signed char c);

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
        std::ostream os((std::streambuf*)0);
        signed char c = 'a';
        os << c;
        assert(os.bad());
        assert(os.fail());
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        signed char c = 'a';
        os << c;
        assert(sb.str() == "a");
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        os.width(5);
        signed char c = 'a';
        os << c;
        assert(sb.str() == "    a");
        assert(os.width() == 0);
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        os.width(5);
        left(os);
        signed char c = 'a';
        os << c;
        assert(sb.str() == "a    ");
        assert(os.width() == 0);
    }
}
