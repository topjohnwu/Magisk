//===------------------------- dynamic_cast.pass.cpp ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cassert>

// This test explicitly tests dynamic cast with types that have inaccessible
// bases.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Winaccessible-base"
#endif

typedef char Pad1[43981];
typedef char Pad2[34981];
typedef char Pad3[93481];
typedef char Pad4[13489];
typedef char Pad5[81349];
typedef char Pad6[34819];
typedef char Pad7[3489];

namespace t1
{

// PR33425
struct C3 { virtual ~C3() {} Pad1 _; };
struct C5 : protected virtual C3 { Pad2 _; };
struct C6 : virtual C5 { Pad3 _; };
struct C7 : virtual C3 { Pad4 _; };
struct C9 : C6, C7 { Pad5 _; };

C9 c9;
C3 *c3 = &c9;

void test()
{
    assert(dynamic_cast<C3*>(c3) == static_cast<C3*>(&c9));
    assert(dynamic_cast<C5*>(c3) == static_cast<C5*>(&c9));
    assert(dynamic_cast<C6*>(c3) == static_cast<C6*>(&c9));
    assert(dynamic_cast<C7*>(c3) == static_cast<C7*>(&c9));
    assert(dynamic_cast<C9*>(c3) == static_cast<C9*>(&c9));
}

}  // t1

namespace t2
{

// PR33425
struct Src { virtual ~Src() {} Pad1 _; };
struct Mask : protected virtual Src { Pad2 _; };
struct Dest : Mask { Pad3 _; };
struct Root : Dest, virtual Src { Pad4 _; };

Root root;
Src *src = &root;

void test()
{
    assert(dynamic_cast<Src*>(src) == static_cast<Src*>(&root));
    assert(dynamic_cast<Mask*>(src) == static_cast<Mask*>(&root));
    assert(dynamic_cast<Dest*>(src) == static_cast<Dest*>(&root));
    assert(dynamic_cast<Root*>(src) == static_cast<Root*>(&root));
}

}  // t2

namespace t3
{

// PR33487
struct Class1 { virtual ~Class1() {} Pad1 _; };
struct Shared : virtual Class1 { Pad2 _; };
struct Class6 : virtual Shared { Pad3 _; };
struct Left : Class6 { Pad4 _; };
struct Right : Class6 { Pad5 _; };
struct Main : Left, Right { Pad6 _; };

Main m;
Class1 *c1 = &m;

void test()
{
    assert(dynamic_cast<Class1*>(c1) == static_cast<Class1*>(&m));
    assert(dynamic_cast<Shared*>(c1) == static_cast<Shared*>(&m));
    assert(dynamic_cast<Class6*>(c1) == 0);
    assert(dynamic_cast<Left*>(c1) == static_cast<Left*>(&m));
    assert(dynamic_cast<Right*>(c1) == static_cast<Right*>(&m));
    assert(dynamic_cast<Main*>(c1) == static_cast<Main*>(&m));
}

}  // t3

namespace t4
{

// PR33439
struct C2 { virtual ~C2() {} Pad1 _; };
struct C3 { virtual ~C3() {} Pad2 _; };
struct C4 : C3 { Pad3 _; };
struct C8 : C2, virtual C4 { Pad4 _; };
struct C9 : C4, C8 { Pad5 _; };

C9 c9;
C2 *c2 = &c9;

void test()
{
    assert(dynamic_cast<C2*>(c2) == static_cast<C2*>(&c9));
    assert(dynamic_cast<C3*>(c2) == 0);
    assert(dynamic_cast<C4*>(c2) == 0);
    assert(dynamic_cast<C8*>(c2) == static_cast<C8*>(&c9));
    assert(dynamic_cast<C9*>(c2) == static_cast<C9*>(&c9));
}

}  // t4

namespace t5
{

// PR33439
struct Dummy { virtual ~Dummy() {} Pad1 _; };
struct Src { virtual ~Src() {} Pad2 _; };
struct Dest : Dummy { Pad3 _; };
struct A1 : Dest { Pad4 _; };
struct A2 : Dest { Pad5 _; };
struct Root : Src, A1, A2 { Pad6 _; };

Root root;
Src *src = &root;

void test()
{
    assert(dynamic_cast<Dummy*>(src) == 0);
    assert(dynamic_cast<Src*>(src) == static_cast<Src*>(&root));
    assert(dynamic_cast<Dest*>(src) == 0);
    assert(dynamic_cast<A1*>(src) == static_cast<A1*>(&root));
    assert(dynamic_cast<A2*>(src) == static_cast<A2*>(&root));
}

}  // t5

int main()
{
    t1::test();
    t2::test();
    t3::test();
    t4::test();
    t5::test();
}
