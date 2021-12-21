//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// Check that std::multiset fails to instantiate if the comparison predicate is
// not copy-constructible. This is LWG issue 2436

#include <set>

template <class T>
struct Comp {
    bool operator () (const T& lhs, const T& rhs) const { return lhs < rhs; }

    Comp () {}
private:
    Comp (const Comp &); // declared but not defined
    };


int main() {
    std::multiset<int, Comp<int> > m;
}
