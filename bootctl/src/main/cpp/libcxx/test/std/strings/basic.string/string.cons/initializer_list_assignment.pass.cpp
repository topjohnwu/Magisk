//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <string>

// basic_string& operator=(initializer_list<charT> il);

#include <string>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::string s;
        s = {'a', 'b', 'c'};
        assert(s == "abc");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        S s;
        s = {'a', 'b', 'c'};
        assert(s == "abc");
    }
}
