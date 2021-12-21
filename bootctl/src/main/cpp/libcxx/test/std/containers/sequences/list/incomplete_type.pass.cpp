//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// Check that std::list and its iterators can be instantiated with an incomplete
// type.

#include <list>

struct A {
    std::list<A> l;
    std::list<A>::iterator it;
    std::list<A>::const_iterator cit;
    std::list<A>::reverse_iterator rit;
    std::list<A>::const_reverse_iterator crit;
};

int main() {
    A a;
}
