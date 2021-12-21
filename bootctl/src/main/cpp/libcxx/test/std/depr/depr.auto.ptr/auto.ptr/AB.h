//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef AB_H
#define AB_H

#include <cassert>

class A
{
    int id_;
public:
    explicit A(int id) : id_(id) {++count;}
    A(const A& a) : id_(a.id_) {++count;}
    virtual ~A() {assert(id_ >= 0); id_ = -1; --count;}

    static int count;
};

int A::count = 0;

class B
    : public A
{
public:
    explicit B(int id) : A(id) {++count;}
    B(const B& a) : A(a) {++count;}
    virtual ~B() {--count;}

    static int count;
};

int B::count = 0;

#endif  // AB_H
