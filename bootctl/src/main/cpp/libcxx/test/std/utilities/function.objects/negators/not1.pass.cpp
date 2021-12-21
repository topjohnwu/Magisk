//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// not1

#include <functional>
#include <cassert>

int main()
{
    typedef std::logical_not<int> F;
    assert(std::not1(F())(36));
    assert(!std::not1(F())(0));
}
