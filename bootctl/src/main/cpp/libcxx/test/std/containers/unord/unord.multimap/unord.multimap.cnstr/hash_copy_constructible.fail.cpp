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

// Check that std::unordered_multimap fails to instantiate if the hash function is
// not copy-constructible. This is mentioned in LWG issue 2436

#include <unordered_map>

template <class T>
struct Hash {
    std::size_t operator () (const T& lhs) const { return 0; }

    Hash () {}
private:
    Hash (const Hash &); // declared but not defined
    };


int main() {
    std::unordered_multimap<int, int, Hash<int> > m;
}
