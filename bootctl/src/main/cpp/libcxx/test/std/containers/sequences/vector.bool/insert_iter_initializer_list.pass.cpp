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

// iterator insert(const_iterator p, initializer_list<value_type> il);

#include <vector>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    std::vector<bool> d(10, true);
    std::vector<bool>::iterator i = d.insert(d.cbegin() + 2, {false, true, true, false});
    assert(d.size() == 14);
    assert(i == d.begin() + 2);
    assert(d[0] == true);
    assert(d[1] == true);
    assert(d[2] == false);
    assert(d[3] == true);
    assert(d[4] == true);
    assert(d[5] == false);
    assert(d[6] == true);
    assert(d[7] == true);
    assert(d[8] == true);
    assert(d[9] == true);
    assert(d[10] == true);
    assert(d[11] == true);
    assert(d[12] == true);
    assert(d[13] == true);
    }
    {
    std::vector<bool, min_allocator<bool>> d(10, true);
    std::vector<bool, min_allocator<bool>>::iterator i = d.insert(d.cbegin() + 2, {false, true, true, false});
    assert(d.size() == 14);
    assert(i == d.begin() + 2);
    assert(d[0] == true);
    assert(d[1] == true);
    assert(d[2] == false);
    assert(d[3] == true);
    assert(d[4] == true);
    assert(d[5] == false);
    assert(d[6] == true);
    assert(d[7] == true);
    assert(d[8] == true);
    assert(d[9] == true);
    assert(d[10] == true);
    assert(d[11] == true);
    assert(d[12] == true);
    assert(d[13] == true);
    }
}
