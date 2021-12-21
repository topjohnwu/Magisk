//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
// The test requires access control SFINAE.

// <unordered_map>

// Check that std::unordered_multimap fails to instantiate if the comparison predicate is
// not copy-constructible. This is LWG issue 2436

#include <unordered_map>

template <class T>
struct Comp {
    bool operator () (const T& lhs, const T& rhs) const { return lhs == rhs; }

    Comp () {}
private:
    Comp (const Comp &); // declared but not defined
    };


int main() {
    std::unordered_multimap<int, int, std::hash<int>, Comp<int> > m;
}
