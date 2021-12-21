//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// int_type get();

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
        testbuf<char> sb("          ");
        std::istream is(&sb);
        char c = static_cast<char>(is.get());
        assert(!is.eof());
        assert(!is.fail());
        assert(c == ' ');
        assert(is.gcount() == 1);
    }
    {
        testbuf<char> sb(" abc");
        std::istream is(&sb);
        char c = static_cast<char>(is.get());
        assert(!is.eof());
        assert(!is.fail());
        assert(c == ' ');
        assert(is.gcount() == 1);
        c = static_cast<char>(is.get());
        assert(!is.eof());
        assert(!is.fail());
        assert(c == 'a');
        assert(is.gcount() == 1);
        c = static_cast<char>(is.get());
        assert(!is.eof());
        assert(!is.fail());
        assert(c == 'b');
        assert(is.gcount() == 1);
        c = static_cast<char>(is.get());
        assert(!is.eof());
        assert(!is.fail());
        assert(c == 'c');
        assert(is.gcount() == 1);
    }
    {
        testbuf<wchar_t> sb(L" abc");
        std::wistream is(&sb);
        wchar_t c = is.get();
        assert(!is.eof());
        assert(!is.fail());
        assert(c == L' ');
        assert(is.gcount() == 1);
        c = is.get();
        assert(!is.eof());
        assert(!is.fail());
        assert(c == L'a');
        assert(is.gcount() == 1);
        c = is.get();
        assert(!is.eof());
        assert(!is.fail());
        assert(c == L'b');
        assert(is.gcount() == 1);
        c = is.get();
        assert(!is.eof());
        assert(!is.fail());
        assert(c == L'c');
        assert(is.gcount() == 1);
    }
}
