//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef A_H
#define A_H

#include <cassert>

class A
{
    int id_;
public:
    explicit A(int id) : id_(id) {++count;}
    A(const A& a) : id_(a.id_) {++count;}
    ~A() {assert(id_ >= 0); id_ = -1; --count;}

    int id() const {return id_;}

    static int count;
};

int A::count = 0;

#endif  // A_H
