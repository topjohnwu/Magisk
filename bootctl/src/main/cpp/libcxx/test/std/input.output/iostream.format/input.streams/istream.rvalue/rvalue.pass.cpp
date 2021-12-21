//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <istream>

// template <class charT, class traits, class T>
//   basic_istream<charT, traits>&
//   operator>>(basic_istream<charT, traits>&& is, T&& x);

#include <istream>
#include <sstream>
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


struct A{};
bool called = false;
void operator>>(std::istream&, A&&){ called = true; }

int main()
{
    {
        testbuf<char> sb("   123");
        int i = 0;
        std::istream(&sb) >> i;
        assert(i == 123);
    }
    {
        testbuf<wchar_t> sb(L"   123");
        int i = 0;
        std::wistream(&sb) >> i;
        assert(i == 123);
    }
    { // test perfect forwarding
        assert(called == false);
        std::istringstream ss;
        auto&& out = (std::move(ss) >> A{});
        assert(&out == &ss);
        assert(called);
    }
}
