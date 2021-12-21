//===------------------------- dynamic_cast3.cpp --------------------------===//
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

/*

A1   A2   A3

*/

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
{
    char _[93481];
    virtual ~A3() {}

    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA3()) == 0);
    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == 0);
    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t1

/*

A1   A2
|
A3

*/

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
    : public A1
{
    char _[93481];
    virtual ~A3() {}

    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA3()) == 0);

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
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
    : public virtual A1
{
    char _[93481];
    virtual ~A3() {}

    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA3()) == 0);

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
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
    : private A1
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA3()) == 0);

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
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
    : private virtual A1
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA3()) == 0);

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t5

/*

A1   A2
 \  /
  A3

*/

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
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
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
    : public virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
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
    : private A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
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
    : private virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t9

namespace t10
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
      public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t10

namespace t11
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
    : private A1,
      public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t11

namespace t12
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
    : private virtual A1,
      public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t12

namespace t13
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
    : private A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t13

namespace t14
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
    : private virtual A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t14

namespace t15
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
    : private virtual A1,
      private virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == 0);
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == 0);

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t15

/*

A1
|
A2
|
A3

*/

namespace t16
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t16

namespace t17
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t17

namespace t18
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A2
{
    char _[93481];
    virtual ~A3() {}

    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t18

namespace t19
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : protected virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A2
{
    char _[93481];
    virtual ~A3() {}

    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t19

namespace t20
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t20

namespace t21
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t21

namespace t22
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : protected virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A2
{
    char _[93481];
    virtual ~A3() {}

    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t22

namespace t23
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private A2
{
    char _[93481];
    virtual ~A3() {}

    t23::A1* getA1() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t23

namespace t24
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : protected virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private A2
{
    char _[93481];
    virtual ~A3() {}

    t24::A1* getA1() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t24

namespace t25
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : protected virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private virtual A2
{
    char _[93481];
    virtual ~A3() {}

    t25::A1* getA1() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t25

/*

A1 A1
|  |
A2 |
 \ |  
  A3

*/

namespace t26
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA12()) == a3.getA12());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t26

namespace t27
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA12()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t27

namespace t28
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t28

namespace t29
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t29

namespace t30
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA12()) == a3.getA12());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t30

namespace t31
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA12()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t31

namespace t32
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA12()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t32

namespace t33
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA12() {return A2::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA12()) == a3.getA12());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA12()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA12()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t33

/*

A1
| \
A2 \
 \ |  
  A3

*/

namespace t34
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t34

namespace t35
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t35

namespace t36
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t36

namespace t37
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t37

namespace t38
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : public virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private virtual A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a2.getA2()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA2()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == a2.getA2());
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t38

namespace t39
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : public virtual A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());
    assert(dynamic_cast<A1*>(a3.getA3()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t39

namespace t40
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private virtual A1,
      public A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());
    assert(dynamic_cast<A2*>(a3.getA3()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == a3.getA3());
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t40

namespace t41
{

struct A1
{
    char _[43981];
    virtual ~A1() {}

    A1* getA1() {return this;}
};

struct A2
    : private virtual A1
{
    char _[34981];
    virtual ~A2() {}

    A1* getA1() {return this;}
    A2* getA2() {return this;}
};

struct A3
    : private virtual A1,
      private A2
{
    char _[93481];
    virtual ~A3() {}

    A1* getA1() {return A1::getA1();}
    A2* getA2() {return this;}
    A3* getA3() {return this;}
};

void test()
{
    A1 a1;
    A2 a2;
    A3 a3;
    assert(dynamic_cast<A1*>(a1.getA1()) == a1.getA1());
    assert(dynamic_cast<A1*>(a2.getA1()) == a2.getA1());
    assert(dynamic_cast<A1*>(a3.getA1()) == a3.getA1());

    assert(dynamic_cast<A2*>(a1.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA1()) == 0);
    assert(dynamic_cast<A2*>(a2.getA2()) == a2.getA2());
    assert(dynamic_cast<A2*>(a3.getA1()) == 0);
    assert(dynamic_cast<A2*>(a3.getA2()) == a3.getA2());

    assert(dynamic_cast<A3*>(a1.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA1()) == 0);
    assert(dynamic_cast<A3*>(a2.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA1()) == 0);
    assert(dynamic_cast<A3*>(a3.getA2()) == 0);
    assert(dynamic_cast<A3*>(a3.getA3()) == a3.getA3());
}

}  // t41

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
    t10::test();
    t11::test();
    t12::test();
    t13::test();
    t14::test();
    t15::test();
    t16::test();
    t17::test();
    t18::test();
    t19::test();
    t20::test();
    t21::test();
    t22::test();
    t23::test();
    t24::test();
    t25::test();
    t26::test();
    t27::test();
    t28::test();
    t29::test();
    t30::test();
    t31::test();
    t32::test();
    t33::test();
    t34::test();
    t35::test();
    t36::test();
    t37::test();
    t38::test();
    t39::test();
    t40::test();
    t41::test();
}
