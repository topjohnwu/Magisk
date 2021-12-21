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

// void str(const basic_string<charT,traits,Allocator>& s);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream ss(" 123 456");
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss.str(" 789");
        ss.clear();
        assert(ss.good());
        assert(ss.str() == " 789");
        ss >> i;
        assert(i == 789);
    }
    {
        std::wistringstream ss(L" 123 456");
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
        ss.str(L" 789");
        ss.clear();
        assert(ss.good());
        assert(ss.str() == L" 789");
        ss >> i;
        assert(i == 789);
    }
}
