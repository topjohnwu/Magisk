//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// Check that std::multiset and its iterators can be instantiated with an incomplete
// type.

#include <set>

struct A {
    typedef std::multiset<A> Set;
    int data;
    Set m;
    Set::iterator it;
    Set::const_iterator cit;
};

inline bool operator==(A const& L, A const& R) { return &L == &R; }
inline bool operator<(A const& L, A const& R)  { return L.data < R.data; }
int main() {
    A a;
}
