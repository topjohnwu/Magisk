//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test ratio:  The template argument D shall not be zero

#include <ratio>
#include <cstdint>

int main()
{
    const std::intmax_t t1 = std::ratio<1, 0>::num;
}
