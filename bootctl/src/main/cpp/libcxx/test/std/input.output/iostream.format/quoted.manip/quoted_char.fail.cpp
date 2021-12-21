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

//  Test that mismatches between strings and wide streams are diagnosed

#if TEST_STD_VER > 11

void round_trip ( const char *p ) {
    std::wstringstream ss;
    ss << std::quoted(p);
    std::string s;
    ss >> std::quoted(s);
    }



int main()
{
    round_trip ( "Hi Mom" );
}
#else
#error
#endif
