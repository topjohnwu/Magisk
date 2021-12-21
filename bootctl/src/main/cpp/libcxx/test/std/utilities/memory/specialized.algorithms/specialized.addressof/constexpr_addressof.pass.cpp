//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// XFAIL: gcc-4, gcc-5, gcc-6

// <memory>

// template <ObjectType T> constexpr T* addressof(T& r);

#include <memory>
#include <cassert>

struct Pointer {
  constexpr Pointer(void* v) : value(v) {}
  void* value;
};

struct A
{
    constexpr A() : n(42) {}
    void operator&() const { }
    int n;
};

constexpr int i = 0;
constexpr double d = 0.0;
constexpr A a{};

int main()
{
    static_assert(std::addressof(i) == &i, "");
    static_assert(std::addressof(d) == &d, "");
    constexpr const A* ap = std::addressof(a);
    static_assert(&ap->n == &a.n, "");
}
