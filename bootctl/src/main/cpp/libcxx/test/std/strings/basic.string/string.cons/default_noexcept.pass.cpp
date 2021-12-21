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

// basic_string()
//        noexcept(is_nothrow_default_constructible<allocator_type>::value);

// This tests a conforming extension

#include <string>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"

int main()
{
    {
        typedef std::string C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, test_allocator<char>> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, limited_allocator<char, 10>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
}
