//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// class slice;

// slice();

#include <valarray>
#include <cassert>

int main()
{
    std::slice s;
    assert(s.start() == 0);
    assert(s.size() == 0);
    assert(s.stride() == 0);
}
