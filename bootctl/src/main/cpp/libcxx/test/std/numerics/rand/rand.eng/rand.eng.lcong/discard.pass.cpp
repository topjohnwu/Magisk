//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, UIntType a, UIntType c, UIntType m>
//   class linear_congruential_engine;

// void discard(unsigned long long z);

#include <random>
#include <cassert>

template <class T>
void
rand0()
{
    typedef std::linear_congruential_engine<T, 16807, 0, 2147483647> E;
    E e;
    e.discard(9999);
    assert(e() == 1043618065);
}

template <class T>
void
rand()
{
    typedef std::linear_congruential_engine<T, 48271, 0, 2147483647> E;
    E e;
    e.discard(9999);
    assert(e() == 399268537);
}

template <class T>
void
other()
{
    typedef std::linear_congruential_engine<T, 48271, 123465789, 2147483647> E;
    E e1;
    E e2;
    assert(e1 == e2);
    e1.discard(1);
    assert(e1 != e2);
    (void)e2();
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
    rand0<unsigned int>();
    rand0<unsigned long>();
    rand0<unsigned long long>();

    rand<unsigned int>();
    rand<unsigned long>();
    rand<unsigned long long>();

    other<unsigned int>();
    other<unsigned long>();
    other<unsigned long long>();
}
