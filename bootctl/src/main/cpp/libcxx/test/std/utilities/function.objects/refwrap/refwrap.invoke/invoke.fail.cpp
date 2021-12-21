//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// template <class... ArgTypes>
//   requires Callable<T, ArgTypes&&...>
//   Callable<T, ArgTypes&&...>::result_type
//   operator()(ArgTypes&&... args) const;

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
    std::reference_wrapper<int A_int_1::*> r1(fp);
    A_int_1 a;
    assert(r1(a) == 5);
    r1(a) = 6;
    assert(r1(a) == 6);
    const A_int_1* ap = &a;
    assert(r1(ap) == 6);
    r1(ap) = 7;
    assert(r1(ap) == 7);
    }
}

int main()
{
    test_int_1();
}
