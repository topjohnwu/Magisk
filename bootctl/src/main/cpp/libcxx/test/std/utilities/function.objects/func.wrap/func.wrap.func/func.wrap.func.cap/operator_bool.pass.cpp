//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// explicit operator bool() const

#include <functional>
#include <cassert>

int g(int) {return 0;}

int main()
{
    {
    std::function<int(int)> f;
    assert(!f);
    f = g;
    assert(f);
    }
}
