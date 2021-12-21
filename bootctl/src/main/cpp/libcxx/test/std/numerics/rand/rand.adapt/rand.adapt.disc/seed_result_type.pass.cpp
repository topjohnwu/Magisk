//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t p, size_t r>
// class discard_block_engine

// void seed(result_type s = default_seed);

#include <random>
#include <cassert>

void
test1()
{
    for (int s = 0; s < 20; ++s)
    {
        typedef std::ranlux24 E;
        E e1(s);
        E e2;
        e2.seed(s);
        assert(e1 == e2);
    }
}

void
test2()
{
    for (int s = 0; s < 20; ++s)
    {
        typedef std::ranlux48 E;
        E e1(s);
        E e2;
        e2.seed(s);
        assert(e1 == e2);
    }
}

int main()
{
    test1();
    test2();
}
