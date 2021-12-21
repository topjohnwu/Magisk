//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// Check that std::unordered_multiset fails to instantiate if the hash function is
// not copy-constructible. This is mentioned in LWG issue 2436

#include <unordered_set>

template <class T>
struct Hash {
    std::size_t operator () (const T& lhs) const { return 0; }

    Hash () {}
private:
    Hash (const Hash &); // declared but not defined
    };


int main() {
    std::unordered_multiset<int, Hash<int> > m;
}
