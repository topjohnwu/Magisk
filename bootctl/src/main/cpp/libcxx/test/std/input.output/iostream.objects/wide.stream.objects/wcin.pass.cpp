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

// istream wcin;

#include <iostream>
#include <cassert>

int main()
{
#if 0
    std::wcout << L"Hello World!\n";
    int i;
    std::wcout << L"Enter a number: ";
    std::wcin >> i;
    std::wcout << L"The number is : " << i << L'\n';
#else  // 0
#ifdef _LIBCPP_HAS_NO_STDOUT
    assert(std::wcin.tie() == NULL);
#else
    assert(std::wcin.tie() == &std::wcout);
#endif
#endif
}
