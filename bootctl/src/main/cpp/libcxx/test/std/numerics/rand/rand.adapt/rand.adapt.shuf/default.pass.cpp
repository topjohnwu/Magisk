//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t k>
// class shuffle_order_engine

// explicit shuffle_order_engine();

#include <random>
#include <cassert>

void
test1()
{
    std::knuth_b e1;
    std::knuth_b e2(std::minstd_rand0::default_seed);
    assert(e1 == e2);
    assert(e1() == 152607844u);
}

int main()
{
    test1();
}
