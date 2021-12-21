//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <sstream>

// template <class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
// class basic_stringstream

// basic_stringstream& operator=(basic_stringstream&& rhs);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringstream ss0(" 123 456 ");
        std::stringstream ss;
        ss = std::move(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456 ");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss << i << ' ' << 123;
        assert(ss.str() == "456 1236 ");
    }
    {
        std::wstringstream ss0(L" 123 456 ");
        std::wstringstream ss;
        ss = std::move(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456 ");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss << i << ' ' << 123;
        assert(ss.str() == L"456 1236 ");
    }
}
