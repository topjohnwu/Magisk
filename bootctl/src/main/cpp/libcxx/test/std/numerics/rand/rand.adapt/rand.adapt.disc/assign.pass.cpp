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

// discard_block_engine& operator=(const discard_block_engine&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::ranlux24 E;
    E e1(2);
    (void)e1();
    E e2(5);
    e2 = e1;
    assert(e1 == e2);
    assert(e1() == e2());
    E::result_type k = e1();
    assert(e1 != e2);
    assert(e2() == k);
    assert(e1 == e2);
}

void
test2()
{
    typedef std::ranlux48 E;
    E e1(3);
    (void)e1();
    E e2(5);
    e2 = e1;
    assert(e1 == e2);
    assert(e1() == e2());
    E::result_type k = e1();
    assert(e1 != e2);
    assert(e2() == k);
    assert(e1 == e2);
}

int main()
{
    test1();
    test2();
}
