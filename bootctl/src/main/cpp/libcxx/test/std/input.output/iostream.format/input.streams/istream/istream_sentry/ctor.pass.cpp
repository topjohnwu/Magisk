//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// template <class charT, class traits = char_traits<charT> >
// class basic_istream::sentry;

// explicit sentry(basic_istream<charT,traits>& is, bool noskipws = false);

#include <istream>
#include <cassert>

int sync_called = 0;

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_string<CharT> string_type;
    typedef std::basic_streambuf<CharT> base;
private:
    string_type str_;
public:

    testbuf() {}
    testbuf(const string_type& str)
        : str_(str)
    {
        base::setg(const_cast<CharT*>(str_.data()),
                   const_cast<CharT*>(str_.data()),
                   const_cast<CharT*>(str_.data()) + str_.size());
    }

    CharT* eback() const {return base::eback();}
    CharT* gptr() const {return base::gptr();}
    CharT* egptr() const {return base::egptr();}
protected:

    int virtual sync()
    {
        ++sync_called;
        return 1;
    }
};

int main()
{
    {
        std::istream is((testbuf<char>*)0);
        std::istream::sentry sen(is, true);
        assert(!(bool)sen);
        assert(!is.good());
        assert(is.gcount() == 0);
        assert(sync_called == 0);
    }
    {
        std::wistream is((testbuf<wchar_t>*)0);
        std::wistream::sentry sen(is, true);
        assert(!(bool)sen);
        assert(!is.good());
        assert(is.gcount() == 0);
        assert(sync_called == 0);
    }
    {
        testbuf<char> sb("   123");
        std::istream is(&sb);
        std::istream::sentry sen(is, true);
        assert((bool)sen);
        assert(is.good());
        assert(is.gcount() == 0);
        assert(sync_called == 0);
        assert(sb.gptr() == sb.eback());
    }
    {
        testbuf<wchar_t> sb(L"   123");
        std::wistream is(&sb);
        std::wistream::sentry sen(is, true);
        assert((bool)sen);
        assert(is.good());
        assert(is.gcount() == 0);
        assert(sync_called == 0);
        assert(sb.gptr() == sb.eback());
    }
    {
        testbuf<char> sb("   123");
        std::istream is(&sb);
        std::istream::sentry sen(is);
        assert((bool)sen);
        assert(is.good());
        assert(sync_called == 0);
        assert(sb.gptr() == sb.eback() + 3);
    }
    {
        testbuf<wchar_t> sb(L"   123");
        std::wistream is(&sb);
        std::wistream::sentry sen(is);
        assert((bool)sen);
        assert(is.good());
        assert(sync_called == 0);
        assert(sb.gptr() == sb.eback() + 3);
    }
    {
        testbuf<char> sb("      ");
        std::istream is(&sb);
        std::istream::sentry sen(is);
        assert(!(bool)sen);
        assert(is.fail());
        assert(is.eof());
        assert(sync_called == 0);
        assert(sb.gptr() == sb.eback() + 6);
    }
    {
        testbuf<char> sb("      ");
        std::istream is(&sb);
        std::istream::sentry sen(is, true);
        assert((bool)sen);
        assert(is.good());
        assert(sync_called == 0);
        assert(sb.gptr() == sb.eback());
    }
}
