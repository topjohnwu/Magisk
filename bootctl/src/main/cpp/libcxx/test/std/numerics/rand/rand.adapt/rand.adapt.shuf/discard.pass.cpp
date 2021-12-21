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

// void discard(unsigned long long z);

#include <random>
#include <cassert>

void
test1()
{
    std::knuth_b e1;
    std::knuth_b e2 = e1;
    assert(e1 == e2);
    e1.discard(3);
    assert(e1 != e2);
    (void)e2();
    (void)e2();
    (void)e2();
    assert(e1 == e2);
}

int main()
{
    test1();
}
