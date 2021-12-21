//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// R operator()(ArgTypes... args) const

#include <functional>
#include <cassert>

// member data pointer:  cv qualifiers should transfer from argument to return type

struct A_int_1
{
    A_int_1() : data_(5) {}

    int data_;
};

void
test_int_1()
{
    // member data pointer
    {
        int A_int_1::*fp = &A_int_1::data_;
        A_int_1 a;
        std::function<int& (const A_int_1*)> r2(fp);
        const A_int_1* ap = &a;
        assert(r2(ap) == 6);
        r2(ap) = 7;
        assert(r2(ap) == 7);
    }
}

int main()
{
    test_int_1();
}
