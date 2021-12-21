//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// int_type peek();

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
        testbuf<char> sb(" 1\n2345\n6");
        std::istream is(&sb);
        assert(is.peek() == ' ');
        assert(!is.eof());
        assert(!is.fail());
        assert(is.gcount() == 0);
        is.get();
        assert(is.peek() == '1');
        assert(!is.eof());
        assert(!is.fail());
        assert(is.gcount() == 0);
    }
    {
        testbuf<wchar_t> sb(L" 1\n2345\n6");
        std::wistream is(&sb);
        assert(is.peek() == L' ');
        assert(!is.eof());
        assert(!is.fail());
        assert(is.gcount() == 0);
        is.get();
        assert(is.peek() == L'1');
        assert(!is.eof());
        assert(!is.fail());
        assert(is.gcount() == 0);
    }
}
