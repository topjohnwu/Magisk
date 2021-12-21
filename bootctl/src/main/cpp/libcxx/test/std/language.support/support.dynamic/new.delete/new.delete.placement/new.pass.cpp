//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test placement new

#include <new>
#include <cassert>

int A_constructed = 0;

struct A
{
    A() {++A_constructed;}
    ~A() {--A_constructed;}
};

int main()
{
    char buf[sizeof(A)];

    A* ap = new(buf) A;
    assert((char*)ap == buf);
    assert(A_constructed == 1);
}
