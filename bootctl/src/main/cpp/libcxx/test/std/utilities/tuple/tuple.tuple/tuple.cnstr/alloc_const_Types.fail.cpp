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

// template <class Alloc>
//   EXPLICIT tuple(allocator_arg_t, const Alloc& a, const Types&...);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <memory>
#include <cassert>

struct ExplicitCopy {
  explicit ExplicitCopy(ExplicitCopy const&) {}
  explicit ExplicitCopy(int) {}
};

std::tuple<ExplicitCopy> const_explicit_copy_test() {
    const ExplicitCopy e(42);
    return {std::allocator_arg, std::allocator<void>{}, e};
    // expected-error@-1 {{chosen constructor is explicit in copy-initialization}}
}

std::tuple<ExplicitCopy> non_const_explicity_copy_test() {
    ExplicitCopy e(42);
    return {std::allocator_arg, std::allocator<void>{}, e};
    // expected-error@-1 {{chosen constructor is explicit in copy-initialization}}
}
int main()
{
    const_explicit_copy_test();
    non_const_explicity_copy_test();
}
