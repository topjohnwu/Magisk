//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class UIntType, size_t w, size_t s, size_t r>
// class subtract_with_carry_engine;

// template<class Sseq> void seed(Sseq& q);

#include <random>
#include <cassert>

void
test1()
{
    unsigned a[] = {3, 5, 7};
    std::seed_seq sseq(a, a+3);
    std::ranlux24_base e1;
    std::ranlux24_base e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

void
test2()
{
    unsigned a[] = {3, 5, 7};
    std::seed_seq sseq(a, a+3);
    std::ranlux48_base e1;
    std::ranlux48_base e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

int main()
{
    test1();
    test2();
}
