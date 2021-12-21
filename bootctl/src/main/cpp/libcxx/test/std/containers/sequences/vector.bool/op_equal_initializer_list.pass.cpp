//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <vector>

// vector& operator=(initializer_list<value_type> il);

#include <vector>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    std::vector<bool> d;
    d = {true, false, false, true};
    assert(d.size() == 4);
    assert(d[0] == true);
    assert(d[1] == false);
    assert(d[2] == false);
    assert(d[3] == true);
    }
    {
    std::vector<bool, min_allocator<bool>> d;
    d = {true, false, false, true};
    assert(d.size() == 4);
    assert(d[0] == true);
    assert(d[1] == false);
    assert(d[2] == false);
    assert(d[3] == true);
    }
}
