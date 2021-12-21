//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iomanip>

// template<charT> T4 setfill(charT c);

#include <iomanip>
#include <ostream>
#include <cassert>

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    testbuf() {}
};

int main()
{
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        os << std::setfill('*');
        assert(os.fill() == '*');
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        os << std::setfill(L'*');
        assert(os.fill() == L'*');
    }
}
