//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// Period::num shall be positive, diagnostic required.

#include <chrono>

int main()
{
    typedef std::chrono::duration<int, std::ratio<5, -1> > D;
    D d;
}
