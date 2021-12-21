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

// iterator insert(const_iterator p, initializer_list<charT> il);


#include <string>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::string s("123456");
        std::string::iterator i = s.insert(s.begin() + 3, {'a', 'b', 'c'});
        assert(i - s.begin() == 3);
        assert(s == "123abc456");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        S s("123456");
        S::iterator i = s.insert(s.begin() + 3, {'a', 'b', 'c'});
        assert(i - s.begin() == 3);
        assert(s == "123abc456");
    }
}
