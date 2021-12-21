//===------------------------- dynamic_cast5.cpp --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include "support/timer.hpp"

// This test explicitly tests dynamic cast with types that have inaccessible
// bases.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Winaccessible-base"
#endif

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
    : public virtual A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A3,
      public A2
{
    char _[13489];
    virtual ~A4() {}

    t1::A1* getA1() {return A3::getA1();}
    A2* getA2() {return A3::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return this;}
};

struct A5
    : public A4,
      public A3
{
    char _[13489];
    virtual ~A5() {}

    t1::A1* getA1() {return A4::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A4::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == 0);
    assert(dynamic_cast<A1*>(a4.getA3()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA1()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA5()) == a5.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
//    assert(dynamic_cast<A2*>(a3.getA3()) == 0);  // cast to private base
    assert(dynamic_cast<A2*>(a4.getA1()) == 0);
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
//    assert(dynamic_cast<A2*>(a4.getA3()) == 0);  // cast to private base
//    assert(dynamic_cast<A2*>(a4.getA4()) == 0);  // cast to ambiguous base
    assert(dynamic_cast<A2*>(a5.getA1()) == 0);
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
//    assert(dynamic_cast<A2*>(a5.getA3()) == 0);  // cast to private base
//    assert(dynamic_cast<A2*>(a5.getA4()) == 0);  // cast to ambiguous base
//    assert(dynamic_cast<A2*>(a5.getA5()) == 0);  // cast to ambiguous base

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == a4.getA3());
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA3()) == a4.getA3());
    assert(dynamic_cast<A3*>(a4.getA4()) == a4.getA3());
    assert(dynamic_cast<A3*>(a5.getA1()) == 0);
    assert(dynamic_cast<A3*>(a5.getA2()) == 0);
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
//    assert(dynamic_cast<A3*>(a5.getA5()) == 0);  // cast to ambiguous base

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == 0);
    assert(dynamic_cast<A4*>(a4.getA3()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA1()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA2()) == 0);
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA1()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == 0);
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
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
    : public virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A3,
      public A2
{
    char _[13489];
    virtual ~A4() {}

    t2::A1* getA1() {return A3::getA1();}
    A2* getA2() {return A3::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return this;}
};

struct A5
    : public A4,
      public A3
{
    char _[13489];
    virtual ~A5() {}

    t2::A1* getA1() {return A4::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A4::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA3()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA1()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA2()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA1());
    assert(dynamic_cast<A1*>(a5.getA5()) == a5.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == 0);
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA3()) == a4.getA2());
//    assert(dynamic_cast<A2*>(a4.getA4()) == 0);  // cast to ambiguous base
    assert(dynamic_cast<A2*>(a5.getA1()) == 0);
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
//    assert(dynamic_cast<A2*>(a5.getA4()) == 0);  // cast to ambiguous base
//    assert(dynamic_cast<A2*>(a5.getA5()) == 0);  // cast to ambiguous base

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == a4.getA3());
    assert(dynamic_cast<A3*>(a4.getA2()) == a4.getA3());
    assert(dynamic_cast<A3*>(a4.getA3()) == a4.getA3());
    assert(dynamic_cast<A3*>(a4.getA4()) == a4.getA3());
    assert(dynamic_cast<A3*>(a5.getA1()) == 0);
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
//    assert(dynamic_cast<A3*>(a5.getA5()) == 0);  // cast to ambiguous base

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA3()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA1()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA1()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
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
      public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A1,
      public virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());
    assert(dynamic_cast<A2*>(a5.getA14()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA13()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA13()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA13()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA13()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t3

namespace t4
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

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A1,
      public virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);
//    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());  // cast to protected base
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
//    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());  // cast to protected base
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());
    assert(dynamic_cast<A2*>(a5.getA14()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA13()) == 0);
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA13()) == 0);
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA13()) == 0);
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA13()) == 0);
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t4

namespace t5
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

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A1,
      public virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
//    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());  // cast to protected base
    assert(dynamic_cast<A2*>(a4.getA1()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());
    assert(dynamic_cast<A2*>(a5.getA14()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA13()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
//    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());  // cast to protected base
    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA13()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA13()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA13()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t5

namespace t6
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

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : protected A1,
      public virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == 0);
//    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());  // cast to protected base
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());
//    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());  // cast to protected base

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == 0);
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());
    assert(dynamic_cast<A2*>(a5.getA14()) == 0);
    assert(dynamic_cast<A2*>(a5.getA13()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == 0);
    assert(dynamic_cast<A3*>(a5.getA13()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == 0);
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == 0);
    assert(dynamic_cast<A4*>(a5.getA13()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == 0);
    assert(dynamic_cast<A5*>(a5.getA13()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t6

namespace t7
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

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A1,
      protected virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      public A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == 0);
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == 0);
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
//    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());  // cast to protected base
    assert(dynamic_cast<A2*>(a5.getA14()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA13()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
//    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());  // cast to protected base
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA13()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == 0);
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA13()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA13()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t7

namespace t8
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

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A1,
      public virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : protected A3,
      public A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());
    assert(dynamic_cast<A2*>(a5.getA14()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA13()) == 0);
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == 0);
    assert(dynamic_cast<A3*>(a5.getA13()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == 0);
//    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());  // cast to protected base

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA13()) == 0);
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == 0);
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA13()) == 0);
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == 0);
    assert(dynamic_cast<A5*>(a5.getA4()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t8

namespace t9
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

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A3* getA3() {return this;}
};

struct A4
    : public A1,
      public virtual A2
{
    char _[13489];
    virtual ~A4() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return A2::getA2();}
    A4* getA4() {return this;}
};

struct A5
    : public A3,
      protected A4
{
    char _[41389];
    virtual ~A5() {}

    A1* getA14() {return A4::getA1();}
    A1* getA13() {return A3::getA1();}
    A2* getA2() {return A4::getA2();}
    A3* getA3() {return A3::getA3();}
    A4* getA4() {return A4::getA4();}
    A5* getA5() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;

    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());
    assert(dynamic_cast<A1*>(a4.getA1()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA2()) == a4.getA1());
    assert(dynamic_cast<A1*>(a4.getA4()) == a4.getA1());
    assert(dynamic_cast<A1*>(a5.getA14()) == a5.getA14());
    assert(dynamic_cast<A1*>(a5.getA13()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA2()) == 0);
    assert(dynamic_cast<A1*>(a5.getA3()) == a5.getA13());
    assert(dynamic_cast<A1*>(a5.getA4()) == a5.getA14());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());
    assert(dynamic_cast<A2*>(a4.getA1()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA2()) == a4.getA2());
    assert(dynamic_cast<A2*>(a4.getA4()) == a4.getA2());
    assert(dynamic_cast<A2*>(a5.getA14()) == 0);
    assert(dynamic_cast<A2*>(a5.getA13()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA2()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA3()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA4()) == a5.getA2());
    assert(dynamic_cast<A2*>(a5.getA5()) == a5.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
    assert(dynamic_cast<A3*>(a4.getA1()) == 0);
    assert(dynamic_cast<A3*>(a4.getA2()) == 0);
    assert(dynamic_cast<A3*>(a4.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA14()) == 0);
    assert(dynamic_cast<A3*>(a5.getA13()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA2()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA3()) == a5.getA3());
    assert(dynamic_cast<A3*>(a5.getA4()) == 0);
    assert(dynamic_cast<A3*>(a5.getA5()) == a5.getA3());

    assert(dynamic_cast<A4*>(a1.getA1()) == 0);
    assert(dynamic_cast<A4*>(a2.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA1()) == 0);
    assert(dynamic_cast<A4*>(a3.getA2()) == 0);
    assert(dynamic_cast<A4*>(a3.getA3()) == 0);
    assert(dynamic_cast<A4*>(a4.getA1()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA2()) == a4.getA4());
    assert(dynamic_cast<A4*>(a4.getA4()) == a4.getA4());
    assert(dynamic_cast<A4*>(a5.getA14()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA13()) == 0);
    assert(dynamic_cast<A4*>(a5.getA2()) == a5.getA4());
    assert(dynamic_cast<A4*>(a5.getA3()) == 0);
    assert(dynamic_cast<A4*>(a5.getA4()) == a5.getA4());
//    assert(dynamic_cast<A4*>(a5.getA5()) == a5.getA4());  // cast to protected base

    assert(dynamic_cast<A5*>(a1.getA1()) == 0);
    assert(dynamic_cast<A5*>(a2.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA1()) == 0);
    assert(dynamic_cast<A5*>(a3.getA2()) == 0);
    assert(dynamic_cast<A5*>(a3.getA3()) == 0);
    assert(dynamic_cast<A5*>(a4.getA1()) == 0);
    assert(dynamic_cast<A5*>(a4.getA2()) == 0);
    assert(dynamic_cast<A5*>(a4.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA14()) == 0);
    assert(dynamic_cast<A5*>(a5.getA13()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA2()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA3()) == a5.getA5());
    assert(dynamic_cast<A5*>(a5.getA4()) == 0);
    assert(dynamic_cast<A5*>(a5.getA5()) == a5.getA5());
}

}  // t9


int main()
{
    timer t;
    t1::test();
    t2::test();
    t3::test();
    t4::test();
    t5::test();
    t6::test();
    t7::test();
    t8::test();
    t9::test();
}
