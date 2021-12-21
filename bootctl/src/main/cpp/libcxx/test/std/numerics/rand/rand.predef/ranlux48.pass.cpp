//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// typedef discard_block_engine<ranlux48_base, 389, 11>                ranlux48;

#include <random>
#include <cassert>

int main()
{
    std::ranlux48 e;
    e.discard(9999);
    assert(e() == 249142670248501ull);
}
