//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// explicit tuple(const T&...);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

struct ExplicitCopy {
  ExplicitCopy(int) {}
  explicit ExplicitCopy(ExplicitCopy const&) {}
};

std::tuple<ExplicitCopy> const_explicit_copy() {
    const ExplicitCopy e(42);
    return {e};
    // expected-error@-1 {{chosen constructor is explicit in copy-initialization}}
}


std::tuple<ExplicitCopy> non_const_explicit_copy() {
    ExplicitCopy e(42);
    return {e};
    // expected-error@-1 {{chosen constructor is explicit in copy-initialization}}
}

std::tuple<ExplicitCopy> const_explicit_copy_no_brace() {
    const ExplicitCopy e(42);
    return e;
    // expected-error@-1 {{no viable conversion}}
}

int main()
{
}
