//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iostream>

// istream wcerr;

#include <iostream>
#include <cassert>

int main()
{
#if 0
    std::wcerr << L"Hello World!\n";
#else
#ifdef _LIBCPP_HAS_NO_STDOUT
    assert(std::wcerr.tie() == NULL);
#else
    assert(std::wcerr.tie() == &std::wcout);
#endif
    assert(std::wcerr.flags() & std::ios_base::unitbuf);
#endif  // 0
}
