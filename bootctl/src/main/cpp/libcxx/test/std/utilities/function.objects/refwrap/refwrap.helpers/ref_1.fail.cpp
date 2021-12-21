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

// template <ObjectType T> reference_wrapper<T> ref(T& t);

// Don't allow binding to a temp

// XFAIL: c++98, c++03

#include <functional>

struct A {};

const A source() {return A();}

int main()
{
    std::reference_wrapper<const A> r = std::ref(source());
}
