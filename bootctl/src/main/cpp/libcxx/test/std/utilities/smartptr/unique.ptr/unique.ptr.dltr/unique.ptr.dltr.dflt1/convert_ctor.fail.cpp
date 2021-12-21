//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// default_delete

// Test that default_delete<T[]> does not have a working converting constructor

#include <memory>
#include <cassert>

struct A
{
};

struct B
    : public A
{
};

int main()
{
    std::default_delete<B[]> d2;
    std::default_delete<A[]> d1 = d2;
}
