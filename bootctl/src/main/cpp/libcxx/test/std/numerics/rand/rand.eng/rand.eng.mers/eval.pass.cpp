//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t, UIntType c, size_t l, UIntType f>
// class mersenne_twister_engine;

// result_type operator()();

#include <random>
#include <sstream>
#include <cassert>

void
test1()
{
    std::mt19937 e;
    assert(e() == 3499211612u);
    assert(e() == 581869302u);
    assert(e() == 3890346734u);
}

void
test2()
{
    std::mt19937_64 e;
    assert(e() == 14514284786278117030ull);
    assert(e() == 4620546740167642908ull);
    assert(e() == 13109570281517897720ull);
}

int main()
{
    test1();
    test2();
}
