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

// explicit subtract_with_carry_engine();

#include <random>
#include <cassert>

void
test1()
{
    std::ranlux24_base e1;
    std::ranlux24_base e2(std::ranlux24_base::default_seed);
    assert(e1 == e2);
    assert(e1() == 15039276);
}

void
test2()
{
    std::ranlux48_base e1;
    std::ranlux48_base e2(std::ranlux48_base::default_seed);
    assert(e1 == e2);
    assert(e1() == 23459059301164ull);
}

int main()
{
    test1();
    test2();
}
