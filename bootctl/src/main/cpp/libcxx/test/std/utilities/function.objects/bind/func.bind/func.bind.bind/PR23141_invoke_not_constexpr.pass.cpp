//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <functional>

// template<CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);
// template<Returnable R, CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);

// https://bugs.llvm.org/show_bug.cgi?id=23141
#include <functional>
#include <type_traits>

struct Fun
{
  template<typename T, typename U>
  void operator()(T &&, U &&) const
  {
    static_assert(std::is_same<U, int &>::value, "");
  }
};

int main()
{
    std::bind(Fun{}, std::placeholders::_1, 42)("hello");
}
