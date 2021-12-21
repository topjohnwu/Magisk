//===------------------------- dynamic_cast14.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include "support/timer.hpp"

namespace t1
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
{
    char _[34981];
    virtual ~A2() {}

    A2* getA2() {return this;}
};

struct A3
    : public A1,
      public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1_3() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public virtual A2,
      public A1
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1_4() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[13489];
    virtual ~A5() {}

    A1* getA1_3() {return A3::getA1();}
    A1* getA1_4() {return A4::getA1();}
    A2* getA2() {return A3::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

struct A6
    : public A1
{
    char _[81349];
    virtual ~A6() {}

    A1* getA1_6() {return A1::getA1();}
    A6* getA6() {return this;}
};

struct A7
    : public A5,
      public A6
{
    char _[34819];
    virtual ~A7() {}

    A1* getA1_3() {return A5::getA1_3();}
    A1* getA1_4() {return A5::getA1_4();}
    A1* getA1_6() {return A6::getA1_6();}
    A2* getA2() {return A5::getA2();}
    A3* getA3() {return A5::getA3();}
    A4* getA4() {return A5::getA4();}
    A5* getA5() {return A5::getA5();}
    A6* getA6() {return A6::getA6();}
    A7* getA7() {return this;}
};

struct A8
    : public A7
{
    char _[3489];
    virtual ~A8() {}

    A1* getA1_3() {return A7::getA1_3();}
    A1* getA1_4() {return A7::getA1_4();}
    A1* getA1_6() {return A7::getA1_6();}
    A2* getA2() {return A7::getA2();}
    A3* getA3() {return A7::getA3();}
    A4* getA4() {return A7::getA4();}
    A5* getA5() {return A7::getA5();}
    A6* getA6() {return A7::getA6();}
    A7* getA7() {return A7::getA7();}
    A8* getA8() {return this;}
};

struct A9
    : public A1
{
    char _[3481];
    virtual ~A9() {}

    A1* getA1_9() {return A1::getA1();}
    A9* getA9() {return this;}
};

struct A10
    : public virtual A8
{
    char _[4831];
    virtual ~A10() {}

    A1* getA1_3() {return A8::getA1_3();}
    A1* getA1_4() {return A8::getA1_4();}
    A1* getA1_6() {return A8::getA1_6();}
    A2* getA2() {return A8::getA2();}
    A3* getA3() {return A8::getA3();}
    A4* getA4() {return A8::getA4();}
    A5* getA5() {return A8::getA5();}
    A6* getA6() {return A8::getA6();}
    A7* getA7() {return A8::getA7();}
    A8* getA8() {return A8::getA8();}
    A10* getA10() {return this;}
};

struct A11
    : public virtual A8,
      public A9
{
    char _[6483];
    virtual ~A11() {}

    A1* getA1_3() {return A8::getA1_3();}
    A1* getA1_4() {return A8::getA1_4();}
    A1* getA1_6() {return A8::getA1_6();}
    A1* getA1_9() {return A9::getA1_9();}
    A2* getA2() {return A8::getA2();}
    A3* getA3() {return A8::getA3();}
    A4* getA4() {return A8::getA4();}
    A5* getA5() {return A8::getA5();}
    A6* getA6() {return A8::getA6();}
    A7* getA7() {return A8::getA7();}
    A8* getA8() {return A8::getA8();}
    A9* getA9() {return A9::getA9();}
    A11* getA11() {return this;}
};

struct A12
    : public A10,
      public A11
{
    char _[2283];
    virtual ~A12() {}

    A1* getA1_3() {return A10::getA1_3();}
    A1* getA1_4() {return A10::getA1_4();}
    A1* getA1_6() {return A10::getA1_6();}
    A1* getA1_9() {return A11::getA1_9();}
    A2* getA2() {return A10::getA2();}
    A3* getA3() {return A10::getA3();}
    A4* getA4() {return A10::getA4();}
    A5* getA5() {return A10::getA5();}
    A6* getA6() {return A10::getA6();}
    A7* getA7() {return A10::getA7();}
    A8* getA8() {return A10::getA8();}
    A9* getA9() {return A11::getA9();}
    A10* getA10() {return A10::getA10();}
    A11* getA11() {return A11::getA11();}
    A12* getA12() {return this;}
};

struct A13
    : public A12
{
    char _[1283];
    virtual ~A13() {}

    A1* getA1_3() {return A12::getA1_3();}
    A1* getA1_4() {return A12::getA1_4();}
    A1* getA1_6() {return A12::getA1_6();}
    A1* getA1_9() {return A12::getA1_9();}
    A2* getA2() {return A12::getA2();}
    A3* getA3() {return A12::getA3();}
    A4* getA4() {return A12::getA4();}
    A5* getA5() {return A12::getA5();}
    A6* getA6() {return A12::getA6();}
    A7* getA7() {return A12::getA7();}
    A8* getA8() {return A12::getA8();}
    A9* getA9() {return A12::getA9();}
    A10* getA10() {return A12::getA10();}
    A11* getA11() {return A12::getA11();}
    A12* getA12() {return A12::getA12();}
    A13* getA13() {return this;}
};

A3 a3;
A4 a4;
A5 a5;
A6 a6;
A7 a7;
A8 a8;
A9 a9;
A10 a10;
A11 a11;
A12 a12;
A13 a13;

void test()
{
    assert(dynamic_cast<A3*>(a3.getA1_3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());

    assert(dynamic_cast<A3*>(a4.getA1_4()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1_4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());

    assert(dynamic_cast<A3*>(a5.getA1_3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());

    assert(dynamic_cast<A4*>(a5.getA1_3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA1_4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());

    assert(dynamic_cast<A5*>(a5.getA1_3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA1_4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());

    assert(dynamic_cast<A3*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A4*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A5*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A6*>(a6.getA1_6()) == a6.getA6());

    assert(dynamic_cast<A3*>(a7.getA1_3()) == a7.getA3());
    assert(dynamic_cast<A3*>(a7.getA1_4()) == a7.getA3());
    assert(dynamic_cast<A3*>(a7.getA2()) == a7.getA3());

    assert(dynamic_cast<A4*>(a7.getA1_3()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA1_4()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA2()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA3()) == a7.getA4());

    assert(dynamic_cast<A5*>(a7.getA1_3()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA1_4()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA2()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA3()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA4()) == a7.getA5());

    assert(dynamic_cast<A6*>(a7.getA1_3()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA1_4()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA1_6()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA2()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA3()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA4()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA5()) == a7.getA6());

    assert(dynamic_cast<A7*>(a7.getA1_3()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA1_4()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA1_6()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA2()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA3()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA4()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA5()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA6()) == a7.getA7());

    assert(dynamic_cast<A3*>(a8.getA1_3()) == a8.getA3());
    assert(dynamic_cast<A3*>(a8.getA1_4()) == a8.getA3());
    assert(dynamic_cast<A3*>(a8.getA2()) == a8.getA3());

    assert(dynamic_cast<A4*>(a8.getA1_3()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA1_4()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA2()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA3()) == a8.getA4());

    assert(dynamic_cast<A5*>(a8.getA1_3()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA1_4()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA2()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA3()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA4()) == a8.getA5());

    assert(dynamic_cast<A6*>(a8.getA1_3()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA1_4()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA1_6()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA2()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA3()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA4()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA5()) == a8.getA6());

    assert(dynamic_cast<A7*>(a8.getA1_3()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA1_4()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA1_6()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA2()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA3()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA4()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA5()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA6()) == a8.getA7());

    assert(dynamic_cast<A8*>(a8.getA1_3()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA1_4()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA1_6()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA2()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA3()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA4()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA5()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA6()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA7()) == a8.getA8());

    assert(dynamic_cast<A3*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A4*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A5*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A6*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A7*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A8*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A9*>(a9.getA1_9()) == a9.getA9());

    assert(dynamic_cast<A3*>(a10.getA1_3()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA1_4()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA1_6()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA2()) == a10.getA3());

    assert(dynamic_cast<A4*>(a10.getA1_3()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA1_4()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA1_6()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA2()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA3()) == a10.getA4());

    assert(dynamic_cast<A5*>(a10.getA1_3()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA1_4()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA1_6()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA2()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA3()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA4()) == a10.getA5());

    assert(dynamic_cast<A6*>(a10.getA1_3()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA1_4()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA1_6()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA2()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA3()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA4()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA5()) == a10.getA6());

    assert(dynamic_cast<A7*>(a10.getA1_3()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA1_4()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA1_6()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA2()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA3()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA4()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA5()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA6()) == a10.getA7());

    assert(dynamic_cast<A8*>(a10.getA1_3()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA1_4()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA1_6()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA2()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA3()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA4()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA5()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA6()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA7()) == a10.getA8());

    assert(dynamic_cast<A9*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A9*>(a10.getA1_4()) == 0);
    assert(dynamic_cast<A9*>(a10.getA1_6()) == 0);
    assert(dynamic_cast<A9*>(a10.getA2()) == 0);
    assert(dynamic_cast<A9*>(a10.getA3()) == 0);
    assert(dynamic_cast<A9*>(a10.getA4()) == 0);
    assert(dynamic_cast<A9*>(a10.getA5()) == 0);
    assert(dynamic_cast<A9*>(a10.getA6()) == 0);
    assert(dynamic_cast<A9*>(a10.getA7()) == 0);
    assert(dynamic_cast<A9*>(a10.getA8()) == 0);

    assert(dynamic_cast<A10*>(a10.getA1_3()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA1_4()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA1_6()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA2()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA3()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA4()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA5()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA6()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA7()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA8()) == a10.getA10());

    assert(dynamic_cast<A3*>(a11.getA1_3()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_4()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_6()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_9()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA2()) == a11.getA3());

    assert(dynamic_cast<A4*>(a11.getA1_3()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_4()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_6()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_9()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA2()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA3()) == a11.getA4());

    assert(dynamic_cast<A5*>(a11.getA1_3()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_4()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_6()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_9()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA2()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA3()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA4()) == a11.getA5());

    assert(dynamic_cast<A6*>(a11.getA1_3()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_4()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_6()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_9()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA2()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA3()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA4()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA5()) == a11.getA6());

    assert(dynamic_cast<A7*>(a11.getA1_3()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_4()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_6()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_9()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA2()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA3()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA4()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA5()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA6()) == a11.getA7());

    assert(dynamic_cast<A8*>(a11.getA1_3()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_4()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_6()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_9()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA2()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA3()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA4()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA5()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA6()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA7()) == a11.getA8());

    assert(dynamic_cast<A9*>(a11.getA1_3()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_4()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_6()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_9()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA2()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA3()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA4()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA5()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA6()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA7()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA8()) == a11.getA9());

    assert(dynamic_cast<A10*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_4()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_6()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_9()) == 0);
    assert(dynamic_cast<A10*>(a11.getA2()) == 0);
    assert(dynamic_cast<A10*>(a11.getA3()) == 0);
    assert(dynamic_cast<A10*>(a11.getA4()) == 0);
    assert(dynamic_cast<A10*>(a11.getA5()) == 0);
    assert(dynamic_cast<A10*>(a11.getA6()) == 0);
    assert(dynamic_cast<A10*>(a11.getA7()) == 0);
    assert(dynamic_cast<A10*>(a11.getA8()) == 0);
    assert(dynamic_cast<A10*>(a11.getA9()) == 0);

    assert(dynamic_cast<A11*>(a11.getA1_3()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_4()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_6()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_9()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA2()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA3()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA4()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA5()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA6()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA7()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA8()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA9()) == a11.getA11());

    assert(dynamic_cast<A3*>(a12.getA1_3()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_4()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_6()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_9()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA2()) == a12.getA3());

    assert(dynamic_cast<A4*>(a12.getA1_3()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_4()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_6()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_9()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA2()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA3()) == a12.getA4());

    assert(dynamic_cast<A5*>(a12.getA1_3()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_4()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_6()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_9()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA2()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA3()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA4()) == a12.getA5());

    assert(dynamic_cast<A6*>(a12.getA1_3()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_4()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_6()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_9()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA2()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA3()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA4()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA5()) == a12.getA6());

    assert(dynamic_cast<A7*>(a12.getA1_3()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_4()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_6()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_9()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA2()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA3()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA4()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA5()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA6()) == a12.getA7());

    assert(dynamic_cast<A8*>(a12.getA1_3()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_4()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_6()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_9()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA2()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA3()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA4()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA5()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA6()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA7()) == a12.getA8());

    assert(dynamic_cast<A9*>(a12.getA1_3()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_4()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_6()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_9()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA2()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA3()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA4()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA5()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA6()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA7()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA8()) == a12.getA9());

    assert(dynamic_cast<A10*>(a12.getA1_3()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_4()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_6()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_9()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA2()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA3()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA4()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA5()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA6()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA7()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA8()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA9()) == a12.getA10());

    assert(dynamic_cast<A11*>(a12.getA1_3()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_4()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_6()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_9()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA2()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA3()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA4()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA5()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA6()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA7()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA8()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA9()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA10()) == a12.getA11());

    assert(dynamic_cast<A12*>(a12.getA1_3()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_4()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_6()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_9()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA2()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA3()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA4()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA5()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA6()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA7()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA8()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA9()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA10()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA11()) == a12.getA12());

    assert(dynamic_cast<A3*>(a13.getA1_3()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_4()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_6()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_9()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA2()) == a13.getA3());

    assert(dynamic_cast<A4*>(a13.getA1_3()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_4()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_6()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_9()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA2()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA3()) == a13.getA4());

    assert(dynamic_cast<A5*>(a13.getA1_3()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_4()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_6()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_9()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA2()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA3()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA4()) == a13.getA5());

    assert(dynamic_cast<A6*>(a13.getA1_3()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_4()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_6()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_9()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA2()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA3()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA4()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA5()) == a13.getA6());

    assert(dynamic_cast<A7*>(a13.getA1_3()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_4()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_6()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_9()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA2()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA3()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA4()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA5()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA6()) == a13.getA7());

    assert(dynamic_cast<A8*>(a13.getA1_3()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_4()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_6()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_9()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA2()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA3()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA4()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA5()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA6()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA7()) == a13.getA8());

    assert(dynamic_cast<A9*>(a13.getA1_3()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_4()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_6()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_9()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA2()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA3()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA4()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA5()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA6()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA7()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA8()) == a13.getA9());

    assert(dynamic_cast<A10*>(a13.getA1_3()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_4()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_6()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_9()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA2()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA3()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA4()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA5()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA6()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA7()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA8()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA9()) == a13.getA10());

    assert(dynamic_cast<A11*>(a13.getA1_3()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_4()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_6()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_9()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA2()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA3()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA4()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA5()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA6()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA7()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA8()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA9()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA10()) == a13.getA11());

    assert(dynamic_cast<A12*>(a13.getA1_3()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_4()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_6()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_9()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA2()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA3()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA4()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA5()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA6()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA7()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA8()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA9()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA10()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA11()) == a13.getA12());

    assert(dynamic_cast<A13*>(a13.getA1_3()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_4()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_6()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_9()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA2()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA3()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA4()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA5()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA6()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA7()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA8()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA9()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA10()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA11()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA12()) == a13.getA13());
}

}  // t1

namespace t2
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
{
    char _[34981];
    virtual ~A2() {}

    A2* getA2() {return this;}
};

struct A3
    : protected A1,
      public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1_3() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public virtual A2,
      public A1
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1_4() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[13489];
    virtual ~A5() {}

    A1* getA1_3() {return A3::getA1();}
    A1* getA1_4() {return A4::getA1();}
    A2* getA2() {return A3::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

struct A6
    : public A1
{
    char _[81349];
    virtual ~A6() {}

    A1* getA1_6() {return A1::getA1();}
    A6* getA6() {return this;}
};

struct A7
    : public A5,
      public A6
{
    char _[34819];
    virtual ~A7() {}

    A1* getA1_3() {return A5::getA1_3();}
    A1* getA1_4() {return A5::getA1_4();}
    A1* getA1_6() {return A6::getA1_6();}
    A2* getA2() {return A5::getA2();}
    A3* getA3() {return A5::getA3();}
    A4* getA4() {return A5::getA4();}
    A5* getA5() {return A5::getA5();}
    A6* getA6() {return A6::getA6();}
    A7* getA7() {return this;}
};

struct A8
    : public A7
{
    char _[3489];
    virtual ~A8() {}

    A1* getA1_3() {return A7::getA1_3();}
    A1* getA1_4() {return A7::getA1_4();}
    A1* getA1_6() {return A7::getA1_6();}
    A2* getA2() {return A7::getA2();}
    A3* getA3() {return A7::getA3();}
    A4* getA4() {return A7::getA4();}
    A5* getA5() {return A7::getA5();}
    A6* getA6() {return A7::getA6();}
    A7* getA7() {return A7::getA7();}
    A8* getA8() {return this;}
};

struct A9
    : public A1
{
    char _[3481];
    virtual ~A9() {}

    A1* getA1_9() {return A1::getA1();}
    A9* getA9() {return this;}
};

struct A10
    : public virtual A8
{
    char _[4831];
    virtual ~A10() {}

    A1* getA1_3() {return A8::getA1_3();}
    A1* getA1_4() {return A8::getA1_4();}
    A1* getA1_6() {return A8::getA1_6();}
    A2* getA2() {return A8::getA2();}
    A3* getA3() {return A8::getA3();}
    A4* getA4() {return A8::getA4();}
    A5* getA5() {return A8::getA5();}
    A6* getA6() {return A8::getA6();}
    A7* getA7() {return A8::getA7();}
    A8* getA8() {return A8::getA8();}
    A10* getA10() {return this;}
};

struct A11
    : public virtual A8,
      public A9
{
    char _[6483];
    virtual ~A11() {}

    A1* getA1_3() {return A8::getA1_3();}
    A1* getA1_4() {return A8::getA1_4();}
    A1* getA1_6() {return A8::getA1_6();}
    A1* getA1_9() {return A9::getA1_9();}
    A2* getA2() {return A8::getA2();}
    A3* getA3() {return A8::getA3();}
    A4* getA4() {return A8::getA4();}
    A5* getA5() {return A8::getA5();}
    A6* getA6() {return A8::getA6();}
    A7* getA7() {return A8::getA7();}
    A8* getA8() {return A8::getA8();}
    A9* getA9() {return A9::getA9();}
    A11* getA11() {return this;}
};

struct A12
    : public A10,
      public A11
{
    char _[2283];
    virtual ~A12() {}

    A1* getA1_3() {return A10::getA1_3();}
    A1* getA1_4() {return A10::getA1_4();}
    A1* getA1_6() {return A10::getA1_6();}
    A1* getA1_9() {return A11::getA1_9();}
    A2* getA2() {return A10::getA2();}
    A3* getA3() {return A10::getA3();}
    A4* getA4() {return A10::getA4();}
    A5* getA5() {return A10::getA5();}
    A6* getA6() {return A10::getA6();}
    A7* getA7() {return A10::getA7();}
    A8* getA8() {return A10::getA8();}
    A9* getA9() {return A11::getA9();}
    A10* getA10() {return A10::getA10();}
    A11* getA11() {return A11::getA11();}
    A12* getA12() {return this;}
};

struct A13
    : public A12
{
    char _[1283];
    virtual ~A13() {}

    A1* getA1_3() {return A12::getA1_3();}
    A1* getA1_4() {return A12::getA1_4();}
    A1* getA1_6() {return A12::getA1_6();}
    A1* getA1_9() {return A12::getA1_9();}
    A2* getA2() {return A12::getA2();}
    A3* getA3() {return A12::getA3();}
    A4* getA4() {return A12::getA4();}
    A5* getA5() {return A12::getA5();}
    A6* getA6() {return A12::getA6();}
    A7* getA7() {return A12::getA7();}
    A8* getA8() {return A12::getA8();}
    A9* getA9() {return A12::getA9();}
    A10* getA10() {return A12::getA10();}
    A11* getA11() {return A12::getA11();}
    A12* getA12() {return A12::getA12();}
    A13* getA13() {return this;}
};

A3 a3;
A4 a4;
A5 a5;
A6 a6;
A7 a7;
A8 a8;
A9 a9;
A10 a10;
A11 a11;
A12 a12;
A13 a13;

void test()
{
    assert(dynamic_cast<A3*>(a3.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());

    assert(dynamic_cast<A3*>(a4.getA1_4()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1_4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());

    assert(dynamic_cast<A3*>(a5.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());

    assert(dynamic_cast<A4*>(a5.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a5.getA1_4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());

    assert(dynamic_cast<A5*>(a5.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a5.getA1_4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());

    assert(dynamic_cast<A3*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A4*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A5*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A6*>(a6.getA1_6()) == a6.getA6());

    assert(dynamic_cast<A3*>(a7.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a7.getA1_4()) == a7.getA3());
    assert(dynamic_cast<A3*>(a7.getA2()) == a7.getA3());

    assert(dynamic_cast<A4*>(a7.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a7.getA1_4()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA2()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA3()) == a7.getA4());

    assert(dynamic_cast<A5*>(a7.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a7.getA1_4()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA2()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA3()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA4()) == a7.getA5());

    assert(dynamic_cast<A6*>(a7.getA1_3()) == 0);
    assert(dynamic_cast<A6*>(a7.getA1_4()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA1_6()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA2()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA3()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA4()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA5()) == a7.getA6());

    assert(dynamic_cast<A7*>(a7.getA1_3()) == 0);
    assert(dynamic_cast<A7*>(a7.getA1_4()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA1_6()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA2()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA3()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA4()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA5()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA6()) == a7.getA7());

    assert(dynamic_cast<A3*>(a8.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a8.getA1_4()) == a8.getA3());
    assert(dynamic_cast<A3*>(a8.getA2()) == a8.getA3());

    assert(dynamic_cast<A4*>(a8.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a8.getA1_4()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA2()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA3()) == a8.getA4());

    assert(dynamic_cast<A5*>(a8.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a8.getA1_4()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA2()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA3()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA4()) == a8.getA5());

    assert(dynamic_cast<A6*>(a8.getA1_3()) == 0);
    assert(dynamic_cast<A6*>(a8.getA1_4()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA1_6()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA2()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA3()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA4()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA5()) == a8.getA6());

    assert(dynamic_cast<A7*>(a8.getA1_3()) == 0);
    assert(dynamic_cast<A7*>(a8.getA1_4()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA1_6()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA2()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA3()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA4()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA5()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA6()) == a8.getA7());

    assert(dynamic_cast<A8*>(a8.getA1_3()) == 0);
    assert(dynamic_cast<A8*>(a8.getA1_4()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA1_6()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA2()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA3()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA4()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA5()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA6()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA7()) == a8.getA8());

    assert(dynamic_cast<A3*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A4*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A5*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A6*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A7*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A8*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A9*>(a9.getA1_9()) == a9.getA9());

    assert(dynamic_cast<A3*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a10.getA1_4()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA1_6()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA2()) == a10.getA3());

    assert(dynamic_cast<A4*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a10.getA1_4()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA1_6()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA2()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA3()) == a10.getA4());

    assert(dynamic_cast<A5*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a10.getA1_4()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA1_6()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA2()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA3()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA4()) == a10.getA5());

    assert(dynamic_cast<A6*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A6*>(a10.getA1_4()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA1_6()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA2()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA3()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA4()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA5()) == a10.getA6());

    assert(dynamic_cast<A7*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A7*>(a10.getA1_4()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA1_6()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA2()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA3()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA4()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA5()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA6()) == a10.getA7());

    assert(dynamic_cast<A8*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A8*>(a10.getA1_4()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA1_6()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA2()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA3()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA4()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA5()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA6()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA7()) == a10.getA8());

    assert(dynamic_cast<A9*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A9*>(a10.getA1_4()) == 0);
    assert(dynamic_cast<A9*>(a10.getA1_6()) == 0);
    assert(dynamic_cast<A9*>(a10.getA2()) == 0);
    assert(dynamic_cast<A9*>(a10.getA3()) == 0);
    assert(dynamic_cast<A9*>(a10.getA4()) == 0);
    assert(dynamic_cast<A9*>(a10.getA5()) == 0);
    assert(dynamic_cast<A9*>(a10.getA6()) == 0);
    assert(dynamic_cast<A9*>(a10.getA7()) == 0);
    assert(dynamic_cast<A9*>(a10.getA8()) == 0);

    assert(dynamic_cast<A10*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A10*>(a10.getA1_4()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA1_6()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA2()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA3()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA4()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA5()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA6()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA7()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA8()) == a10.getA10());

    assert(dynamic_cast<A3*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a11.getA1_4()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_6()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_9()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA2()) == a11.getA3());

    assert(dynamic_cast<A4*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a11.getA1_4()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_6()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_9()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA2()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA3()) == a11.getA4());

    assert(dynamic_cast<A5*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a11.getA1_4()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_6()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_9()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA2()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA3()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA4()) == a11.getA5());

    assert(dynamic_cast<A6*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A6*>(a11.getA1_4()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_6()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_9()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA2()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA3()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA4()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA5()) == a11.getA6());

    assert(dynamic_cast<A7*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A7*>(a11.getA1_4()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_6()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_9()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA2()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA3()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA4()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA5()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA6()) == a11.getA7());

    assert(dynamic_cast<A8*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A8*>(a11.getA1_4()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_6()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_9()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA2()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA3()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA4()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA5()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA6()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA7()) == a11.getA8());

    assert(dynamic_cast<A9*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A9*>(a11.getA1_4()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_6()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_9()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA2()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA3()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA4()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA5()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA6()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA7()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA8()) == a11.getA9());

    assert(dynamic_cast<A10*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_4()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_6()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_9()) == 0);
    assert(dynamic_cast<A10*>(a11.getA2()) == 0);
    assert(dynamic_cast<A10*>(a11.getA3()) == 0);
    assert(dynamic_cast<A10*>(a11.getA4()) == 0);
    assert(dynamic_cast<A10*>(a11.getA5()) == 0);
    assert(dynamic_cast<A10*>(a11.getA6()) == 0);
    assert(dynamic_cast<A10*>(a11.getA7()) == 0);
    assert(dynamic_cast<A10*>(a11.getA8()) == 0);
    assert(dynamic_cast<A10*>(a11.getA9()) == 0);

    assert(dynamic_cast<A11*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A11*>(a11.getA1_4()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_6()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_9()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA2()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA3()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA4()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA5()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA6()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA7()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA8()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA9()) == a11.getA11());

    assert(dynamic_cast<A3*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a12.getA1_4()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_6()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_9()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA2()) == a12.getA3());

    assert(dynamic_cast<A4*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a12.getA1_4()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_6()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_9()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA2()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA3()) == a12.getA4());

    assert(dynamic_cast<A5*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a12.getA1_4()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_6()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_9()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA2()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA3()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA4()) == a12.getA5());

    assert(dynamic_cast<A6*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A6*>(a12.getA1_4()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_6()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_9()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA2()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA3()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA4()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA5()) == a12.getA6());

    assert(dynamic_cast<A7*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A7*>(a12.getA1_4()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_6()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_9()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA2()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA3()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA4()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA5()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA6()) == a12.getA7());

    assert(dynamic_cast<A8*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A8*>(a12.getA1_4()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_6()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_9()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA2()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA3()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA4()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA5()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA6()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA7()) == a12.getA8());

    assert(dynamic_cast<A9*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A9*>(a12.getA1_4()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_6()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_9()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA2()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA3()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA4()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA5()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA6()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA7()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA8()) == a12.getA9());

    assert(dynamic_cast<A10*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A10*>(a12.getA1_4()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_6()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_9()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA2()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA3()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA4()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA5()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA6()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA7()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA8()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA9()) == a12.getA10());

    assert(dynamic_cast<A11*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A11*>(a12.getA1_4()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_6()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_9()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA2()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA3()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA4()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA5()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA6()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA7()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA8()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA9()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA10()) == a12.getA11());

    assert(dynamic_cast<A12*>(a12.getA1_3()) == 0);
    assert(dynamic_cast<A12*>(a12.getA1_4()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_6()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_9()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA2()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA3()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA4()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA5()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA6()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA7()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA8()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA9()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA10()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA11()) == a12.getA12());

    assert(dynamic_cast<A3*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A3*>(a13.getA1_4()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_6()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_9()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA2()) == a13.getA3());

    assert(dynamic_cast<A4*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A4*>(a13.getA1_4()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_6()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_9()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA2()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA3()) == a13.getA4());

    assert(dynamic_cast<A5*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A5*>(a13.getA1_4()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_6()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_9()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA2()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA3()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA4()) == a13.getA5());

    assert(dynamic_cast<A6*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A6*>(a13.getA1_4()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_6()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_9()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA2()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA3()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA4()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA5()) == a13.getA6());

    assert(dynamic_cast<A7*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A7*>(a13.getA1_4()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_6()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_9()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA2()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA3()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA4()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA5()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA6()) == a13.getA7());

    assert(dynamic_cast<A8*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A8*>(a13.getA1_4()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_6()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_9()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA2()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA3()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA4()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA5()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA6()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA7()) == a13.getA8());

    assert(dynamic_cast<A9*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A9*>(a13.getA1_4()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_6()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_9()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA2()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA3()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA4()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA5()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA6()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA7()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA8()) == a13.getA9());

    assert(dynamic_cast<A10*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A10*>(a13.getA1_4()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_6()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_9()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA2()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA3()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA4()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA5()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA6()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA7()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA8()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA9()) == a13.getA10());

    assert(dynamic_cast<A11*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A11*>(a13.getA1_4()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_6()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_9()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA2()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA3()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA4()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA5()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA6()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA7()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA8()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA9()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA10()) == a13.getA11());

    assert(dynamic_cast<A12*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A12*>(a13.getA1_4()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_6()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_9()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA2()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA3()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA4()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA5()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA6()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA7()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA8()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA9()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA10()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA11()) == a13.getA12());

    assert(dynamic_cast<A13*>(a13.getA1_3()) == 0);
    assert(dynamic_cast<A13*>(a13.getA1_4()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_6()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_9()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA2()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA3()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA4()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA5()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA6()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA7()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA8()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA9()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA10()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA11()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA12()) == a13.getA13());
}

}  // t2

namespace t3
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
{
    char _[34981];
    virtual ~A2() {}

    A2* getA2() {return this;}
};

struct A3
    : public A1,
      protected virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1_3() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public virtual A2,
      public A1
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1_4() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[13489];
    virtual ~A5() {}

    A1* getA1_3() {return A3::getA1();}
    A1* getA1_4() {return A4::getA1();}
    A2* getA2() {return A3::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

struct A6
    : public A1
{
    char _[81349];
    virtual ~A6() {}

    A1* getA1_6() {return A1::getA1();}
    A6* getA6() {return this;}
};

struct A7
    : public A5,
      public A6
{
    char _[34819];
    virtual ~A7() {}

    A1* getA1_3() {return A5::getA1_3();}
    A1* getA1_4() {return A5::getA1_4();}
    A1* getA1_6() {return A6::getA1_6();}
    A2* getA2() {return A5::getA2();}
    A3* getA3() {return A5::getA3();}
    A4* getA4() {return A5::getA4();}
    A5* getA5() {return A5::getA5();}
    A6* getA6() {return A6::getA6();}
    A7* getA7() {return this;}
};

struct A8
    : public A7
{
    char _[3489];
    virtual ~A8() {}

    A1* getA1_3() {return A7::getA1_3();}
    A1* getA1_4() {return A7::getA1_4();}
    A1* getA1_6() {return A7::getA1_6();}
    A2* getA2() {return A7::getA2();}
    A3* getA3() {return A7::getA3();}
    A4* getA4() {return A7::getA4();}
    A5* getA5() {return A7::getA5();}
    A6* getA6() {return A7::getA6();}
    A7* getA7() {return A7::getA7();}
    A8* getA8() {return this;}
};

struct A9
    : public A1
{
    char _[3481];
    virtual ~A9() {}

    A1* getA1_9() {return A1::getA1();}
    A9* getA9() {return this;}
};

struct A10
    : public virtual A8
{
    char _[4831];
    virtual ~A10() {}

    A1* getA1_3() {return A8::getA1_3();}
    A1* getA1_4() {return A8::getA1_4();}
    A1* getA1_6() {return A8::getA1_6();}
    A2* getA2() {return A8::getA2();}
    A3* getA3() {return A8::getA3();}
    A4* getA4() {return A8::getA4();}
    A5* getA5() {return A8::getA5();}
    A6* getA6() {return A8::getA6();}
    A7* getA7() {return A8::getA7();}
    A8* getA8() {return A8::getA8();}
    A10* getA10() {return this;}
};

struct A11
    : public virtual A8,
      public A9
{
    char _[6483];
    virtual ~A11() {}

    A1* getA1_3() {return A8::getA1_3();}
    A1* getA1_4() {return A8::getA1_4();}
    A1* getA1_6() {return A8::getA1_6();}
    A1* getA1_9() {return A9::getA1_9();}
    A2* getA2() {return A8::getA2();}
    A3* getA3() {return A8::getA3();}
    A4* getA4() {return A8::getA4();}
    A5* getA5() {return A8::getA5();}
    A6* getA6() {return A8::getA6();}
    A7* getA7() {return A8::getA7();}
    A8* getA8() {return A8::getA8();}
    A9* getA9() {return A9::getA9();}
    A11* getA11() {return this;}
};

struct A12
    : public A10,
      public A11
{
    char _[2283];
    virtual ~A12() {}

    A1* getA1_3() {return A10::getA1_3();}
    A1* getA1_4() {return A10::getA1_4();}
    A1* getA1_6() {return A10::getA1_6();}
    A1* getA1_9() {return A11::getA1_9();}
    A2* getA2() {return A10::getA2();}
    A3* getA3() {return A10::getA3();}
    A4* getA4() {return A10::getA4();}
    A5* getA5() {return A10::getA5();}
    A6* getA6() {return A10::getA6();}
    A7* getA7() {return A10::getA7();}
    A8* getA8() {return A10::getA8();}
    A9* getA9() {return A11::getA9();}
    A10* getA10() {return A10::getA10();}
    A11* getA11() {return A11::getA11();}
    A12* getA12() {return this;}
};

struct A13
    : public A12
{
    char _[1283];
    virtual ~A13() {}

    A1* getA1_3() {return A12::getA1_3();}
    A1* getA1_4() {return A12::getA1_4();}
    A1* getA1_6() {return A12::getA1_6();}
    A1* getA1_9() {return A12::getA1_9();}
    A2* getA2() {return A12::getA2();}
    A3* getA3() {return A12::getA3();}
    A4* getA4() {return A12::getA4();}
    A5* getA5() {return A12::getA5();}
    A6* getA6() {return A12::getA6();}
    A7* getA7() {return A12::getA7();}
    A8* getA8() {return A12::getA8();}
    A9* getA9() {return A12::getA9();}
    A10* getA10() {return A12::getA10();}
    A11* getA11() {return A12::getA11();}
    A12* getA12() {return A12::getA12();}
    A13* getA13() {return this;}
};

A3 a3;
A4 a4;
A5 a5;
A6 a6;
A7 a7;
A8 a8;
A9 a9;
A10 a10;
A11 a11;
A12 a12;
A13 a13;

void test()
{
    assert(dynamic_cast<A3*>(a3.getA1_3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);

    assert(dynamic_cast<A3*>(a4.getA1_4()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1_4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());

    assert(dynamic_cast<A3*>(a5.getA1_3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());

    assert(dynamic_cast<A4*>(a5.getA1_3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA1_4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());

    assert(dynamic_cast<A5*>(a5.getA1_3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA1_4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());

    assert(dynamic_cast<A3*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A4*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A5*>(a6.getA1_6()) == 0);

    assert(dynamic_cast<A6*>(a6.getA1_6()) == a6.getA6());

    assert(dynamic_cast<A3*>(a7.getA1_3()) == a7.getA3());
    assert(dynamic_cast<A3*>(a7.getA1_4()) == a7.getA3());
    assert(dynamic_cast<A3*>(a7.getA2()) == a7.getA3());

    assert(dynamic_cast<A4*>(a7.getA1_3()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA1_4()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA2()) == a7.getA4());
    assert(dynamic_cast<A4*>(a7.getA3()) == a7.getA4());

    assert(dynamic_cast<A5*>(a7.getA1_3()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA1_4()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA2()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA3()) == a7.getA5());
    assert(dynamic_cast<A5*>(a7.getA4()) == a7.getA5());

    assert(dynamic_cast<A6*>(a7.getA1_3()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA1_4()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA1_6()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA2()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA3()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA4()) == a7.getA6());
    assert(dynamic_cast<A6*>(a7.getA5()) == a7.getA6());

    assert(dynamic_cast<A7*>(a7.getA1_3()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA1_4()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA1_6()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA2()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA3()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA4()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA5()) == a7.getA7());
    assert(dynamic_cast<A7*>(a7.getA6()) == a7.getA7());

    assert(dynamic_cast<A3*>(a8.getA1_3()) == a8.getA3());
    assert(dynamic_cast<A3*>(a8.getA1_4()) == a8.getA3());
    assert(dynamic_cast<A3*>(a8.getA2()) == a8.getA3());

    assert(dynamic_cast<A4*>(a8.getA1_3()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA1_4()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA2()) == a8.getA4());
    assert(dynamic_cast<A4*>(a8.getA3()) == a8.getA4());

    assert(dynamic_cast<A5*>(a8.getA1_3()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA1_4()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA2()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA3()) == a8.getA5());
    assert(dynamic_cast<A5*>(a8.getA4()) == a8.getA5());

    assert(dynamic_cast<A6*>(a8.getA1_3()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA1_4()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA1_6()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA2()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA3()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA4()) == a8.getA6());
    assert(dynamic_cast<A6*>(a8.getA5()) == a8.getA6());

    assert(dynamic_cast<A7*>(a8.getA1_3()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA1_4()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA1_6()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA2()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA3()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA4()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA5()) == a8.getA7());
    assert(dynamic_cast<A7*>(a8.getA6()) == a8.getA7());

    assert(dynamic_cast<A8*>(a8.getA1_3()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA1_4()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA1_6()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA2()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA3()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA4()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA5()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA6()) == a8.getA8());
    assert(dynamic_cast<A8*>(a8.getA7()) == a8.getA8());

    assert(dynamic_cast<A3*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A4*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A5*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A6*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A7*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A8*>(a9.getA1_9()) == 0);

    assert(dynamic_cast<A9*>(a9.getA1_9()) == a9.getA9());

    assert(dynamic_cast<A3*>(a10.getA1_3()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA1_4()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA1_6()) == a10.getA3());
    assert(dynamic_cast<A3*>(a10.getA2()) == a10.getA3());

    assert(dynamic_cast<A4*>(a10.getA1_3()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA1_4()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA1_6()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA2()) == a10.getA4());
    assert(dynamic_cast<A4*>(a10.getA3()) == a10.getA4());

    assert(dynamic_cast<A5*>(a10.getA1_3()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA1_4()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA1_6()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA2()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA3()) == a10.getA5());
    assert(dynamic_cast<A5*>(a10.getA4()) == a10.getA5());

    assert(dynamic_cast<A6*>(a10.getA1_3()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA1_4()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA1_6()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA2()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA3()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA4()) == a10.getA6());
    assert(dynamic_cast<A6*>(a10.getA5()) == a10.getA6());

    assert(dynamic_cast<A7*>(a10.getA1_3()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA1_4()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA1_6()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA2()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA3()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA4()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA5()) == a10.getA7());
    assert(dynamic_cast<A7*>(a10.getA6()) == a10.getA7());

    assert(dynamic_cast<A8*>(a10.getA1_3()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA1_4()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA1_6()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA2()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA3()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA4()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA5()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA6()) == a10.getA8());
    assert(dynamic_cast<A8*>(a10.getA7()) == a10.getA8());

    assert(dynamic_cast<A9*>(a10.getA1_3()) == 0);
    assert(dynamic_cast<A9*>(a10.getA1_4()) == 0);
    assert(dynamic_cast<A9*>(a10.getA1_6()) == 0);
    assert(dynamic_cast<A9*>(a10.getA2()) == 0);
    assert(dynamic_cast<A9*>(a10.getA3()) == 0);
    assert(dynamic_cast<A9*>(a10.getA4()) == 0);
    assert(dynamic_cast<A9*>(a10.getA5()) == 0);
    assert(dynamic_cast<A9*>(a10.getA6()) == 0);
    assert(dynamic_cast<A9*>(a10.getA7()) == 0);
    assert(dynamic_cast<A9*>(a10.getA8()) == 0);

    assert(dynamic_cast<A10*>(a10.getA1_3()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA1_4()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA1_6()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA2()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA3()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA4()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA5()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA6()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA7()) == a10.getA10());
    assert(dynamic_cast<A10*>(a10.getA8()) == a10.getA10());

    assert(dynamic_cast<A3*>(a11.getA1_3()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_4()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_6()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA1_9()) == a11.getA3());
    assert(dynamic_cast<A3*>(a11.getA2()) == a11.getA3());

    assert(dynamic_cast<A4*>(a11.getA1_3()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_4()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_6()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA1_9()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA2()) == a11.getA4());
    assert(dynamic_cast<A4*>(a11.getA3()) == a11.getA4());

    assert(dynamic_cast<A5*>(a11.getA1_3()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_4()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_6()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA1_9()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA2()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA3()) == a11.getA5());
    assert(dynamic_cast<A5*>(a11.getA4()) == a11.getA5());

    assert(dynamic_cast<A6*>(a11.getA1_3()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_4()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_6()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA1_9()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA2()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA3()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA4()) == a11.getA6());
    assert(dynamic_cast<A6*>(a11.getA5()) == a11.getA6());

    assert(dynamic_cast<A7*>(a11.getA1_3()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_4()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_6()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA1_9()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA2()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA3()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA4()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA5()) == a11.getA7());
    assert(dynamic_cast<A7*>(a11.getA6()) == a11.getA7());

    assert(dynamic_cast<A8*>(a11.getA1_3()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_4()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_6()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA1_9()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA2()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA3()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA4()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA5()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA6()) == a11.getA8());
    assert(dynamic_cast<A8*>(a11.getA7()) == a11.getA8());

    assert(dynamic_cast<A9*>(a11.getA1_3()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_4()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_6()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA1_9()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA2()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA3()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA4()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA5()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA6()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA7()) == a11.getA9());
    assert(dynamic_cast<A9*>(a11.getA8()) == a11.getA9());

    assert(dynamic_cast<A10*>(a11.getA1_3()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_4()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_6()) == 0);
    assert(dynamic_cast<A10*>(a11.getA1_9()) == 0);
    assert(dynamic_cast<A10*>(a11.getA2()) == 0);
    assert(dynamic_cast<A10*>(a11.getA3()) == 0);
    assert(dynamic_cast<A10*>(a11.getA4()) == 0);
    assert(dynamic_cast<A10*>(a11.getA5()) == 0);
    assert(dynamic_cast<A10*>(a11.getA6()) == 0);
    assert(dynamic_cast<A10*>(a11.getA7()) == 0);
    assert(dynamic_cast<A10*>(a11.getA8()) == 0);
    assert(dynamic_cast<A10*>(a11.getA9()) == 0);

    assert(dynamic_cast<A11*>(a11.getA1_3()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_4()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_6()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA1_9()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA2()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA3()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA4()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA5()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA6()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA7()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA8()) == a11.getA11());
    assert(dynamic_cast<A11*>(a11.getA9()) == a11.getA11());

    assert(dynamic_cast<A3*>(a12.getA1_3()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_4()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_6()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA1_9()) == a12.getA3());
    assert(dynamic_cast<A3*>(a12.getA2()) == a12.getA3());

    assert(dynamic_cast<A4*>(a12.getA1_3()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_4()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_6()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA1_9()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA2()) == a12.getA4());
    assert(dynamic_cast<A4*>(a12.getA3()) == a12.getA4());

    assert(dynamic_cast<A5*>(a12.getA1_3()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_4()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_6()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA1_9()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA2()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA3()) == a12.getA5());
    assert(dynamic_cast<A5*>(a12.getA4()) == a12.getA5());

    assert(dynamic_cast<A6*>(a12.getA1_3()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_4()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_6()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA1_9()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA2()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA3()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA4()) == a12.getA6());
    assert(dynamic_cast<A6*>(a12.getA5()) == a12.getA6());

    assert(dynamic_cast<A7*>(a12.getA1_3()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_4()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_6()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA1_9()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA2()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA3()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA4()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA5()) == a12.getA7());
    assert(dynamic_cast<A7*>(a12.getA6()) == a12.getA7());

    assert(dynamic_cast<A8*>(a12.getA1_3()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_4()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_6()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA1_9()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA2()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA3()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA4()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA5()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA6()) == a12.getA8());
    assert(dynamic_cast<A8*>(a12.getA7()) == a12.getA8());

    assert(dynamic_cast<A9*>(a12.getA1_3()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_4()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_6()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA1_9()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA2()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA3()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA4()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA5()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA6()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA7()) == a12.getA9());
    assert(dynamic_cast<A9*>(a12.getA8()) == a12.getA9());

    assert(dynamic_cast<A10*>(a12.getA1_3()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_4()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_6()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA1_9()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA2()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA3()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA4()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA5()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA6()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA7()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA8()) == a12.getA10());
    assert(dynamic_cast<A10*>(a12.getA9()) == a12.getA10());

    assert(dynamic_cast<A11*>(a12.getA1_3()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_4()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_6()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA1_9()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA2()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA3()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA4()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA5()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA6()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA7()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA8()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA9()) == a12.getA11());
    assert(dynamic_cast<A11*>(a12.getA10()) == a12.getA11());

    assert(dynamic_cast<A12*>(a12.getA1_3()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_4()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_6()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA1_9()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA2()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA3()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA4()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA5()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA6()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA7()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA8()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA9()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA10()) == a12.getA12());
    assert(dynamic_cast<A12*>(a12.getA11()) == a12.getA12());

    assert(dynamic_cast<A3*>(a13.getA1_3()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_4()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_6()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA1_9()) == a13.getA3());
    assert(dynamic_cast<A3*>(a13.getA2()) == a13.getA3());

    assert(dynamic_cast<A4*>(a13.getA1_3()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_4()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_6()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA1_9()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA2()) == a13.getA4());
    assert(dynamic_cast<A4*>(a13.getA3()) == a13.getA4());

    assert(dynamic_cast<A5*>(a13.getA1_3()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_4()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_6()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA1_9()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA2()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA3()) == a13.getA5());
    assert(dynamic_cast<A5*>(a13.getA4()) == a13.getA5());

    assert(dynamic_cast<A6*>(a13.getA1_3()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_4()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_6()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA1_9()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA2()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA3()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA4()) == a13.getA6());
    assert(dynamic_cast<A6*>(a13.getA5()) == a13.getA6());

    assert(dynamic_cast<A7*>(a13.getA1_3()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_4()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_6()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA1_9()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA2()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA3()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA4()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA5()) == a13.getA7());
    assert(dynamic_cast<A7*>(a13.getA6()) == a13.getA7());

    assert(dynamic_cast<A8*>(a13.getA1_3()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_4()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_6()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA1_9()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA2()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA3()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA4()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA5()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA6()) == a13.getA8());
    assert(dynamic_cast<A8*>(a13.getA7()) == a13.getA8());

    assert(dynamic_cast<A9*>(a13.getA1_3()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_4()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_6()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA1_9()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA2()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA3()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA4()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA5()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA6()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA7()) == a13.getA9());
    assert(dynamic_cast<A9*>(a13.getA8()) == a13.getA9());

    assert(dynamic_cast<A10*>(a13.getA1_3()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_4()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_6()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA1_9()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA2()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA3()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA4()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA5()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA6()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA7()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA8()) == a13.getA10());
    assert(dynamic_cast<A10*>(a13.getA9()) == a13.getA10());

    assert(dynamic_cast<A11*>(a13.getA1_3()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_4()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_6()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA1_9()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA2()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA3()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA4()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA5()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA6()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA7()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA8()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA9()) == a13.getA11());
    assert(dynamic_cast<A11*>(a13.getA10()) == a13.getA11());

    assert(dynamic_cast<A12*>(a13.getA1_3()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_4()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_6()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA1_9()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA2()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA3()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA4()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA5()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA6()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA7()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA8()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA9()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA10()) == a13.getA12());
    assert(dynamic_cast<A12*>(a13.getA11()) == a13.getA12());

    assert(dynamic_cast<A13*>(a13.getA1_3()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_4()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_6()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA1_9()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA2()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA3()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA4()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA5()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA6()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA7()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA8()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA9()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA10()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA11()) == a13.getA13());
    assert(dynamic_cast<A13*>(a13.getA12()) == a13.getA13());
}

}  // t3

int main()
{
    timer t;
    t1::test();
    t2::test();
    t3::test();
}
