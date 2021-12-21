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

// result_type operator()();

#include <random>
#include <cassert>

template <class T>
void
randu()
{
    typedef std::linear_congruential_engine<T, 65539, 0, 2147483648u> E;
    E e(1);
    assert(e() == 65539);
    assert(e() == 393225);
    assert(e() == 1769499);
    assert(e() == 7077969);
    assert(e() == 26542323);
    assert(e() == 95552217);
    assert(e() == 334432395);
    assert(e() == 1146624417);
    assert(e() == 1722371299);
    assert(e() == 14608041);
    assert(e() == 1766175739);
    assert(e() == 1875647473);
}

template <class T>
void
minstd()
{
    typedef std::linear_congruential_engine<T, 16807, 0, 2147483647> E;
    E e(1);
    assert(e() == 16807);
    assert(e() == 282475249);
    assert(e() == 1622650073);
    assert(e() == 984943658);
    assert(e() == 1144108930);
    assert(e() == 470211272);
    assert(e() == 101027544);
    assert(e() == 1457850878);
    assert(e() == 1458777923);
    assert(e() == 2007237709);
    assert(e() == 823564440);
    assert(e() == 1115438165);
}

template <class T>
void
Haldir()
{
    typedef std::linear_congruential_engine<T, 16807, 78125, 2147483647> E;
    E e(207560540);
    assert(e() == 956631177);
    assert(e() == 2037688522);
    assert(e() == 1509348670);
    assert(e() == 1546336451);
    assert(e() == 429714088);
    assert(e() == 217250280);
}

int main()
{
    randu<unsigned int>();
    randu<unsigned long>();
    randu<unsigned long long>();

    minstd<unsigned int>();
    minstd<unsigned long>();
    minstd<unsigned long long>();

    Haldir<unsigned int>();
    Haldir<unsigned long>();
    Haldir<unsigned long long>();
}
