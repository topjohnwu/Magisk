//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <sstream>

// template <class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
// class basic_ostringstream

// explicit basic_ostringstream(const basic_string<charT,traits,allocator>& str,
//                              ios_base::openmode which = ios_base::in);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::ostringstream ss(" 123 456");
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456");
        int i = 234;
        ss << i << ' ' << 567;
        assert(ss.str() == "234 5676");
    }
    {
        std::ostringstream ss(" 123 456", std::ios_base::in);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456");
        int i = 234;
        ss << i << ' ' << 567;
        assert(ss.str() == "234 5676");
    }
    {
        std::wostringstream ss(L" 123 456");
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456");
        int i = 234;
        ss << i << ' ' << 567;
        assert(ss.str() == L"234 5676");
    }
    {
        std::wostringstream ss(L" 123 456", std::ios_base::in);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456");
        int i = 234;
        ss << i << ' ' << 567;
        assert(ss.str() == L"234 5676");
    }
}
