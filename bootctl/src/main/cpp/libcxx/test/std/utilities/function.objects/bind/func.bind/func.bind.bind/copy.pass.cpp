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

// https://bugs.llvm.org/show_bug.cgi?id=16385

#include <functional>
#include <cmath>
#include <cassert>

float _pow(float a, float b)
{
    return std::pow(a, b);
}

int main()
{
    std::function<float(float, float)> fnc = _pow;
    auto task = std::bind(fnc, 2.f, 4.f);
    auto task2(task);
    assert(task() == 16);
    assert(task2() == 16);
}
