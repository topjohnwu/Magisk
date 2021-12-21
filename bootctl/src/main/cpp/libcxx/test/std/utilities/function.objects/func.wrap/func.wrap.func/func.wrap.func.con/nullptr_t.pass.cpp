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

// function(nullptr_t);

#include <functional>
#include <cassert>

int main()
{
    std::function<int(int)> f(nullptr);
    assert(!f);
}
