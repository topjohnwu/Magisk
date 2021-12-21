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
// class basic_istringstream

// void swap(basic_istringstream& rhs);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream ss0(" 123 456");
        std::istringstream ss(" 789 321");
        ss.swap(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss0 >> i;
        assert(i == 789);
        ss0 >> i;
        assert(i == 321);
    }
    {
        std::wistringstream ss0(L" 123 456");
        std::wistringstream ss(L" 789 321");
        ss.swap(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss0 >> i;
        assert(i == 789);
        ss0 >> i;
        assert(i == 321);
    }
}
