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
// provided a non-const callable comparator.

#include <set>
#include <map>

struct BadCompare {
  template <class T, class U>
  bool operator()(T const& t, U const& u) {
    return t < u;
  }
};

int main() {
  static_assert(!std::__invokable<BadCompare const&, int const&, int const&>::value, "");
  static_assert(std::__invokable<BadCompare&, int const&, int const&>::value, "");

  // expected-warning@set:* 2 {{the specified comparator type does not provide a const call operator}}
  // expected-warning@map:* 2 {{the specified comparator type does not provide a const call operator}}
  {
    using C = std::set<int, BadCompare>;
    C s;
  }
  {
    using C = std::multiset<long, BadCompare>;
    C s;
  }
  {
    using C = std::map<int, int, BadCompare>;
    C s;
  }
  {
    using C = std::multimap<long, int, BadCompare>;
    C s;
  }
}
