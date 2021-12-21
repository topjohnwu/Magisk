//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// Check that std::multimap fails to instantiate if the comparison predicate is
// not copy-constructible. This is LWG issue 2436

#include <map>

template <class T>
struct Comp {
    bool operator () (const T& lhs, const T& rhs) const { return lhs < rhs; }

    Comp () {}
private:
    Comp (const Comp &); // declared but not defined
    };


int main() {
    std::multimap<int, int, Comp<int> > m;
}
