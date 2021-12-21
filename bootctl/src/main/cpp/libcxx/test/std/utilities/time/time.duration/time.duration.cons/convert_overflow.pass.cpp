//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// template <class Rep2, class Period2>
//   duration(const duration<Rep2, Period2>& d);

// overflow should SFINAE instead of error out, LWG 2094

#include <chrono>
#include <cassert>

bool called = false;

void f(std::chrono::milliseconds);
void f(std::chrono::seconds)
{
    called = true;
}

int main()
{
    {
    std::chrono::duration<int, std::exa> r(1);
    f(r);
    assert(called);
    }
}
