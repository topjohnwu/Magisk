//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iostream>

// istream wclog;

#include <iostream>

int main()
{
#if 0
    std::wclog << L"Hello World!\n";
#else
    (void)std::wclog;
#endif
}
