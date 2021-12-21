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

// basic_string& assign(initializer_list<charT> il);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
        std::string s("123");
        s.assign({'a', 'b', 'c'});
        assert(s == "abc");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        S s("123");
        s.assign({'a', 'b', 'c'});
        assert(s == "abc");
    }
}
