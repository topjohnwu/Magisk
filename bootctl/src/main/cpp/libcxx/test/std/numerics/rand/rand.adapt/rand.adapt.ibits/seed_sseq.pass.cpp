//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t w, class UIntType>
// class independent_bits_engine

// template<class Sseq> void seed(Sseq& q);

#include <random>
#include <cassert>

void
test1()
{
    unsigned a[] = {3, 5, 7};
    std::seed_seq sseq(a, a+3);
    std::independent_bits_engine<std::ranlux24, 32, unsigned> e1;
    std::independent_bits_engine<std::ranlux24, 32, unsigned> e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

void
test2()
{
    unsigned a[] = {3, 5, 7};
    std::seed_seq sseq(a, a+3);
    std::independent_bits_engine<std::ranlux48, 64, unsigned long long> e1;
    std::independent_bits_engine<std::ranlux48, 64, unsigned long long> e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

int main()
{
    test1();
    test2();
}
