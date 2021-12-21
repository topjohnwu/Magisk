//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// typedef shuffle_order_engine<minstd_rand0, 256>                      knuth_b;

#include <random>
#include <cassert>

int main()
{
    std::knuth_b e;
    e.discard(9999);
    assert(e() == 1112339016u);
}
