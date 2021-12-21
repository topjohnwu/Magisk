//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: libcpp-has-no-stdout

// <iostream>

// istream wcout;

#include <iostream>

int main()
{
#if 0
    std::wcout << L"Hello World!\n";
#else
    (void)std::wcout;
#endif
}
