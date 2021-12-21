//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: libcpp-has-no-stdin

// <iostream>

// istream cin;

#include <iostream>
#include <cassert>

int main()
{
#if 0
    std::cout << "Hello World!\n";
    int i;
    std::cout << "Enter a number: ";
    std::cin >> i;
    std::cout << "The number is : " << i << '\n';
#else  // 0
#ifdef _LIBCPP_HAS_NO_STDOUT
    assert(std::cin.tie() == NULL);
#else
    assert(std::cin.tie() == &std::cout);
#endif
#endif
}
