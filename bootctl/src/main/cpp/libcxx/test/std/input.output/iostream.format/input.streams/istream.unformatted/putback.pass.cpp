//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// basic_istream<charT,traits>& putback(char_type c);

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
        testbuf<char> sb(" 123456789");
        std::istream is(&sb);
        is.get();
        is.get();
        is.get();
        is.putback('a');
        assert(is.bad());
        assert(is.gcount() == 0);
        is.clear();
        is.putback('2');
        assert(is.good());
        assert(is.gcount() == 0);
        is.putback('1');
        assert(is.good());
        assert(is.gcount() == 0);
        is.putback(' ');
        assert(is.good());
        assert(is.gcount() == 0);
        is.putback(' ');
        assert(is.bad());
        assert(is.gcount() == 0);
    }
    {
        testbuf<wchar_t> sb(L" 123456789");
        std::wistream is(&sb);
        is.get();
        is.get();
        is.get();
        is.putback(L'a');
        assert(is.bad());
        assert(is.gcount() == 0);
        is.clear();
        is.putback(L'2');
        assert(is.good());
        assert(is.gcount() == 0);
        is.putback(L'1');
        assert(is.good());
        assert(is.gcount() == 0);
        is.putback(L' ');
        assert(is.good());
        assert(is.gcount() == 0);
        is.putback(L' ');
        assert(is.bad());
        assert(is.gcount() == 0);
    }
}
