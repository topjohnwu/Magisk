//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test ratio:  the absolute values of the template arguments N and D
//               shall be representable by type intmax_t.

#include <ratio>
#include <cstdint>

int main()
{
    const std::intmax_t t1 = std::ratio<1, 0x8000000000000000ULL>::num;
}
