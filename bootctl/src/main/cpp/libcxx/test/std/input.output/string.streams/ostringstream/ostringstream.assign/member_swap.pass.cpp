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

// void swap(basic_ostringstream& rhs);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::ostringstream ss0(" 123 456");
        std::ostringstream ss;
        ss.swap(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456");
        int i = 234;
        ss << i << ' ' << 567;
        assert(ss.str() == "234 5676");
        ss0 << i << ' ' << 567;
        assert(ss0.str() == "234 567");
    }
    {
        std::wostringstream ss0(L" 123 456");
        std::wostringstream ss;
        ss.swap(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456");
        int i = 234;
        ss << i << ' ' << 567;
        assert(ss.str() == L"234 5676");
        ss0 << i << ' ' << 567;
        assert(ss0.str() == L"234 567");
    }
}
