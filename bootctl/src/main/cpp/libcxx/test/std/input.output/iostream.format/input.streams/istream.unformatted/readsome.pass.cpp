//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// streamsize readsome(char_type* s, streamsize n);

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
        testbuf<char> sb(" 1234567890");
        std::istream is(&sb);
        char s[5];
        assert(is.readsome(s, 5) == 5);
        assert(!is.eof());
        assert(!is.fail());
        assert(std::string(s, 5) == " 1234");
        assert(is.gcount() == 5);
        is.readsome(s, 5);
        assert(!is.eof());
        assert(!is.fail());
        assert(std::string(s, 5) == "56789");
        assert(is.gcount() == 5);
        is.readsome(s, 5);
        assert(!is.eof());
        assert(!is.fail());
        assert(is.gcount() == 1);
        assert(std::string(s, 1) == "0");
        assert(is.readsome(s, 5) == 0);
    }
    {
        testbuf<wchar_t> sb(L" 1234567890");
        std::wistream is(&sb);
        wchar_t s[5];
        assert(is.readsome(s, 5) == 5);
        assert(!is.eof());
        assert(!is.fail());
        assert(std::wstring(s, 5) == L" 1234");
        assert(is.gcount() == 5);
        is.readsome(s, 5);
        assert(!is.eof());
        assert(!is.fail());
        assert(std::wstring(s, 5) == L"56789");
        assert(is.gcount() == 5);
        is.readsome(s, 5);
        assert(!is.eof());
        assert(!is.fail());
        assert(is.gcount() == 1);
        assert(std::wstring(s, 1) == L"0");
        assert(is.readsome(s, 5) == 0);
    }
}
