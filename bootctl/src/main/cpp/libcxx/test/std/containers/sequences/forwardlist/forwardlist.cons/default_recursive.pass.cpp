//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// class forward_list

// forward_list();

#include <forward_list>

struct X
{
    std::forward_list<X> q;
};

int main()
{
}
