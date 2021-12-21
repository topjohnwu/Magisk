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

// explicit independent_bits_engine();

#include <random>
#include <cassert>

void
test1()
{
    std::independent_bits_engine<std::ranlux24, 32, unsigned> e1;
    std::independent_bits_engine<std::ranlux24, 32, unsigned> e2(std::ranlux24_base::default_seed);
    assert(e1 == e2);
    assert(e1() == 2066486613);
}

void
test2()
{
    std::independent_bits_engine<std::ranlux48, 64, unsigned long long> e1;
    std::independent_bits_engine<std::ranlux48, 64, unsigned long long> e2(std::ranlux48_base::default_seed);
    assert(e1 == e2);
    assert(e1() == 18223106896348967647ull);
}

int main()
{
    test1();
    test2();
}
