//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iomanip>

// quoted

#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 11

//  Test that mismatches in the traits between the quoted object and the dest string are diagnosed.

template <class charT>
struct test_traits
{
    typedef charT     char_type;
};

void round_trip ( const char *p ) {
    std::stringstream ss;
    ss << std::quoted(p);
    std::basic_string<char, test_traits<char>> s;
    ss >> std::quoted(s);
    }



int main()
{
    round_trip ( "Hi Mom" );
}
#else
#error
#endif
