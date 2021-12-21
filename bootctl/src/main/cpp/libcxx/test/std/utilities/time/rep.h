//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef REP_H
#define REP_H

#include "test_macros.h"

class Rep
{
    int data_;
public:
    TEST_CONSTEXPR Rep() : data_(-1) {}
    explicit TEST_CONSTEXPR Rep(int i) : data_(i) {}

    bool TEST_CONSTEXPR operator==(int i) const {return data_ == i;}
    bool TEST_CONSTEXPR operator==(const Rep& r) const {return data_ == r.data_;}

    Rep& operator*=(Rep x) {data_ *= x.data_; return *this;}
    Rep& operator/=(Rep x) {data_ /= x.data_; return *this;}
};

#endif  // REP_H
