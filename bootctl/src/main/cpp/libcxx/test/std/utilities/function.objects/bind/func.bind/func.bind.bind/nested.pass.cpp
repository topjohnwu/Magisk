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

// https://bugs.llvm.org/show_bug.cgi?id=16343

#include <cmath>
#include <functional>
#include <cassert>

struct power
{
  template <typename T>
  T
  operator()(T a, T b)
  {
    return static_cast<T>(std::pow(a, b));
  }
};

struct plus_one
{
  template <typename T>
  T
  operator()(T a)
  {
    return a + 1;
  }
};

int main()
{
    using std::placeholders::_1;

    auto g = std::bind(power(), 2, _1);
    assert(g(5) == 32);
    assert(std::bind(plus_one(), g)(5) == 33);
}
