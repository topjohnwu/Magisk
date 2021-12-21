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

// result_type operator()();

#include <random>
#include <cassert>

void
test1()
{
    std::ranlux24_base e;
    assert(e() == 15039276u);
    assert(e() == 16323925u);
    assert(e() == 14283486u);
}

void
test2()
{
    std::ranlux48_base e;
    assert(e() == 23459059301164ull);
    assert(e() == 28639057539807ull);
    assert(e() == 276846226770426ull);
}

int main()
{
    test1();
    test2();
}
