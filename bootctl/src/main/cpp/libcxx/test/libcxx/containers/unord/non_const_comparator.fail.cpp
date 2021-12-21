//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
// REQUIRES: diagnose-if-support, verify-support

// Test that libc++ generates a warning diagnostic when the container is
// provided a non-const callable comparator or a non-const hasher.

#include <unordered_set>
#include <unordered_map>

struct BadHash {
  template <class T>
  size_t operator()(T const& t) {
    return std::hash<T>{}(t);
  }
};

struct BadEqual {
  template <class T, class U>
  bool operator()(T const& t, U const& u) {
    return t == u;
  }
};

int main() {
  static_assert(!std::__invokable<BadEqual const&, int const&, int const&>::value, "");
  static_assert(std::__invokable<BadEqual&, int const&, int const&>::value, "");

  // expected-warning@unordered_set:* 2 {{the specified comparator type does not provide a const call operator}}
  // expected-warning@unordered_map:* 2 {{the specified comparator type does not provide a const call operator}}
  // expected-warning@unordered_set:* 2 {{the specified hash functor does not provide a const call operator}}
  // expected-warning@unordered_map:* 2 {{the specified hash functor does not provide a const call operator}}

  {
    using C = std::unordered_set<int, BadHash, BadEqual>;
    C s;
  }
  {
    using C = std::unordered_multiset<long, BadHash, BadEqual>;
    C s;
  }
  {
    using C = std::unordered_map<int, int, BadHash, BadEqual>;
    C s;
  }
  {
    using C = std::unordered_multimap<long, int, BadHash, BadEqual>;
    C s;
  }
}
