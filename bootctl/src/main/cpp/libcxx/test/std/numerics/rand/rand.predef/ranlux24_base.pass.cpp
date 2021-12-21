//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// typedef subtract_with_carry_engine<uint_fast32_t, 24, 10, 24>  ranlux24_base;

#include <random>
#include <cassert>

int main()
{
    std::ranlux24_base e;
    e.discard(9999);
    assert(e() == 7937952u);
}
