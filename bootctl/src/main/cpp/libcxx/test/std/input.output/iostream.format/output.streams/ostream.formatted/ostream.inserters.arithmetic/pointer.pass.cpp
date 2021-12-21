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

// operator<<(const void* val);

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
        const void* n = 0;
        os << n;
        assert(os.bad());
        assert(os.fail());
    }
    {
        testbuf<char> sb1;
        std::ostream os1(&sb1);
        int n1;
        os1 << &n1;
        assert(os1.good());
        std::string s1(sb1.str());

        testbuf<char> sb2;
        std::ostream os2(&sb2);
        int n2;
        os2 << &n2;
        assert(os2.good());
        std::string s2(sb2.str());

        // %p is implementation defined. Instead of validating the
        // output, at least ensure that it does not generate an empty
        // string. Also make sure that given two distinct addresses, the
        // output of %p is different.
        assert(!s1.empty());
        assert(!s2.empty());
        assert(s1 != s2);
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        const void* n = &sb;
        os << n;
        assert(os.good());
    }
}
