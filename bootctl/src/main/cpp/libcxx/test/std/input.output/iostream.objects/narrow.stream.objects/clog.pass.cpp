//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iostream>

// istream clog;

#include <iostream>

int main()
{
#if 0
    std::clog << "Hello World!\n";
#else
    (void)std::clog;
#endif
}
