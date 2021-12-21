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

// subtract_with_carry_engine& operator=(const subtract_with_carry_engine&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::ranlux24_base E;
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
    typedef std::ranlux48_base E;
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
