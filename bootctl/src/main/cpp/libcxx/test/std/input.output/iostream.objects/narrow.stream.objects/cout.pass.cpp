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

// istream cout;

#include <iostream>

int main()
{
#if 0
    std::cout << "Hello World!\n";
    int i;
    std::cout << "Enter a number: ";
    std::cin >> i;
    std::cout << "The number is : " << i << '\n';
#else  // 0
    (void)std::cout;
#endif
}
