//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// not2

#include <functional>
#include <cassert>

int main()
{
    typedef std::logical_and<int> F;
    assert(!std::not2(F())(36, 36));
    assert( std::not2(F())(36, 0));
    assert( std::not2(F())(0, 36));
    assert( std::not2(F())(0, 0));
}
