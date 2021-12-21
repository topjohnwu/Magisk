//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: diagnose-if-support
// UNSUPPORTED: c++98, c++03

// Libc++ only provides a defined primary template for std::hash in C++14 and
// newer.
// UNSUPPORTED: c++11

// <unordered_set>

// Test that we generate a reasonable diagnostic when the specified hash is
// not enabled.

#include <unordered_set>
#include <utility>

using VT = std::pair<int, int>;

struct BadHashNoCopy {
  BadHashNoCopy() = default;
  BadHashNoCopy(BadHashNoCopy const&) = delete;

  template <class T>
  size_t operator()(T const&) const { return 0; }
};

struct BadHashNoCall {

};


struct GoodHashNoDefault {
  explicit GoodHashNoDefault(void*) {}
  template <class T>
  size_t operator()(T const&) const { return 0; }
};

int main() {

  {
    using Set = std::unordered_set<VT>;
    Set s; // expected-error@__hash_table:* {{the specified hash does not meet the Hash requirements}}


  // FIXME: It would be great to suppress the below diagnostic all together.
  //        but for now it's sufficient that it appears last. However there is
  //        currently no way to test the order diagnostics are issued.
  // expected-error@memory:* {{call to implicitly-deleted default constructor of '__compressed_pair_elem}}
  }
  {
    using Set = std::unordered_set<int, BadHashNoCopy>;
    Set s; // expected-error@__hash_table:* {{the specified hash does not meet the Hash requirements}}
  }
  {
    using Set = std::unordered_set<int, BadHashNoCall>;
    Set s; // expected-error@__hash_table:* {{the specified hash does not meet the Hash requirements}}
  }
  {
    using Set = std::unordered_set<int, GoodHashNoDefault>;
    Set s(/*bucketcount*/42, GoodHashNoDefault(nullptr));
  }
}
