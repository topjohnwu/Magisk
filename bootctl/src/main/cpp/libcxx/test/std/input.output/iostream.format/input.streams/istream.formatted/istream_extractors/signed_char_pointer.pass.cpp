//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// template<class traits>
//   basic_istream<char,traits>& operator>>(basic_istream<char,traits>&& in, signed char* s);

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
        testbuf<char> sb("   abcdefghijk    ");
        std::istream is(&sb);
        signed char s[20];
        is >> s;
        assert(!is.eof());
        assert(!is.fail());
        assert(std::string((char*)s) == "abcdefghijk");
    }
    {
        testbuf<char> sb("   abcdefghijk    ");
        std::istream is(&sb);
        is.width(4);
        signed char s[20];
        is >> s;
        assert(!is.eof());
        assert(!is.fail());
        assert(std::string((char*)s) == "abc");
        assert(is.width() == 0);
    }
#if TEST_STD_VER > 17
    {
        testbuf<char> sb("   abcdefghijk    ");
        std::istream is(&sb);
        signed char s[4];
        is >> s;
        assert(!is.eof());
        assert(!is.fail());
        assert(std::string((char*)s) == "abc");
    }
#endif
    {
        testbuf<char> sb("   abcdefghijk");
        std::istream is(&sb);
        signed char s[20];
        is >> s;
        assert( is.eof());
        assert(!is.fail());
        assert(std::string((char*)s) == "abcdefghijk");
        assert(is.width() == 0);
    }
    {
        testbuf<char> sb("   abcdefghijk");
        std::istream is(&sb);
        signed char s[20];
        is.width(1);
        is >> s;
        assert(!is.eof());
        assert( is.fail());
        assert(std::string((char*)s) == "");
        assert(is.width() == 0);
    }
#if TEST_STD_VER > 17
    {
        testbuf<char> sb("   abcdefghijk");
        std::istream is(&sb);
        signed char s[1];
        is >> s;
        assert(!is.eof());
        assert( is.fail());
        assert(std::string((char*)s) == "");
    }
#endif
}
