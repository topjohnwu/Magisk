//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// typedef mersenne_twister_engine<uint_fast32_t, 32, 624, 397, 31,
//                                 0x9908b0df,
//                                 11, 0xffffffff,
//                                 7,  0x9d2c5680,
//                                 15, 0xefc60000,
//                                 18, 1812433253>                      mt19937;

#include <random>
#include <cassert>

int main()
{
    std::mt19937 e;
    e.discard(9999);
    assert(e() == 4123659995u);
}
