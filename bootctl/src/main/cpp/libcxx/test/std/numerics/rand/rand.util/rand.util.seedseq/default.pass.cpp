//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// class seed_seq;

// seed_seq();

#include <random>
#include <cassert>

int main()
{
    std::seed_seq s;
    assert(s.size() == 0);
}
