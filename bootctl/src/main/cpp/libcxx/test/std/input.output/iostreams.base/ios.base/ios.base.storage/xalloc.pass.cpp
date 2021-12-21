//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// class ios_base

// static int xalloc();

#include <ios>
#include <cassert>

int main()
{
    assert(std::ios_base::xalloc() == 0);
    assert(std::ios_base::xalloc() == 1);
    assert(std::ios_base::xalloc() == 2);
    assert(std::ios_base::xalloc() == 3);
    assert(std::ios_base::xalloc() == 4);
}
