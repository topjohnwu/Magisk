//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>
// vector<bool>

// std::find with vector<bool>::iterator

// https://bugs.llvm.org/show_bug.cgi?id=16816

#include <vector>
#include <algorithm>
#include <cassert>
#include <cstddef>

int main()
{
    {
        for (unsigned i = 1; i < 256; ++i)
        {
            std::vector<bool> b(i,true);
            std::vector<bool>::iterator j = std::find(b.begin()+1, b.end(), false);
            assert(static_cast<std::size_t>(j-b.begin()) == i);
            assert(b.end() == j);
        }
    }
    {
        for (unsigned i = 1; i < 256; ++i)
        {
            std::vector<bool> b(i,false);
            std::vector<bool>::iterator j = std::find(b.begin()+1, b.end(), true);
            assert(static_cast<std::size_t>(j-b.begin()) == i);
            assert(b.end() == j);
        }
    }
}
