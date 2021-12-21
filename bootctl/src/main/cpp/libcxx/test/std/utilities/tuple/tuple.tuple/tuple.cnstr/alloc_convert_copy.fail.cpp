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

// template <class Alloc, class ...UTypes>
//   tuple(allocator_arg_t, const Alloc& a, tuple<UTypes...> const&);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <memory>

struct ExplicitCopy {
  explicit ExplicitCopy(int) {}
  explicit ExplicitCopy(ExplicitCopy const&) {}

};

std::tuple<ExplicitCopy> const_explicit_copy_test() {
    const std::tuple<int> t1(42);
    return {std::allocator_arg, std::allocator<void>{}, t1};
    // expected-error@-1 {{chosen constructor is explicit in copy-initialization}}
}

std::tuple<ExplicitCopy> non_const_explicit_copy_test() {
    std::tuple<int> t1(42);
    return {std::allocator_arg, std::allocator<void>{}, t1};
    // expected-error@-1 {{chosen constructor is explicit in copy-initialization}}
}

int main()
{

}
