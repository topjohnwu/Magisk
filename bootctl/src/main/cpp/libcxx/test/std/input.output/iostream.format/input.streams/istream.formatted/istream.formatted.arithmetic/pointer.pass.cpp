//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9

// <istream>

// template <class charT, class traits = char_traits<charT> >
//   class basic_istream;

// operator>>(void*& val);

#include <istream>
#include <cassert>

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
};

int main()
{
    {
        std::istream is((std::streambuf*)0);
        void* n = 0;
        is >> n;
        assert(is.fail());
    }
    {
        testbuf<char> sb("0");
        std::istream is(&sb);
        void* n = (void*)1;
        is >> n;
        assert(n == 0);
        assert( is.eof());
        assert(!is.fail());
    }
    {
        testbuf<char> sb(" 1 ");
        std::istream is(&sb);
        void* n = 0;
        is >> n;
        assert(n == (void*)1);
        assert(!is.eof());
        assert(!is.fail());
    }
    {
        testbuf<wchar_t> sb(L" 1 ");
        std::wistream is(&sb);
        void* n = 0;
        is >> n;
        assert(n == (void*)1);
        assert(!is.eof());
        assert(!is.fail());
    }
    {
        testbuf<char> sb("12345678");
        std::istream is(&sb);
        void* n = 0;
        is >> n;
        assert(n == (void*)0x12345678);
        assert( is.eof());
        assert(!is.fail());
    }
    {
        testbuf<wchar_t> sb(L"12345678");
        std::wistream is(&sb);
        void* n = 0;
        is >> n;
        assert(n == (void*)0x12345678);
        assert( is.eof());
        assert(!is.fail());
    }
}
