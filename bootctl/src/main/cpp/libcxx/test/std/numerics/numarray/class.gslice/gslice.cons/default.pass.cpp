//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// class glice;

// gslice();

#include <valarray>
#include <cassert>

int main()
{
    std::gslice gs;
    assert(gs.start() == 0);
    assert(gs.size().size() == 0);
    assert(gs.stride().size() == 0);
}
