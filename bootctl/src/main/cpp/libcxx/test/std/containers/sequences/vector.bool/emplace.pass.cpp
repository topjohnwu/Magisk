//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <vector>
//  vector<bool>

// template <class... Args> iterator emplace(const_iterator pos, Args&&... args);

#include <vector>
#include <cassert>
#include "min_allocator.h"

int main()
{
    {
        typedef std::vector<bool> C;
        C c;

        C::iterator i = c.emplace(c.cbegin());
        assert(i == c.begin());
        assert(c.size() == 1);
        assert(c.front() == false);

        i = c.emplace(c.cend(), true);
        assert(i == c.end()-1);
        assert(c.size() == 2);
        assert(c.front() == false);
        assert(c.back() == true);

        i = c.emplace(c.cbegin()+1, true);
        assert(i == c.begin()+1);
        assert(c.size() == 3);
        assert(c.front() == false);
        assert(c[1] == true);
        assert(c.back() == true);
    }
    {
        typedef std::vector<bool, min_allocator<bool>> C;
        C c;

        C::iterator i = c.emplace(c.cbegin());
        assert(i == c.begin());
        assert(c.size() == 1);
        assert(c.front() == false);

        i = c.emplace(c.cend(), true);
        assert(i == c.end()-1);
        assert(c.size() == 2);
        assert(c.front() == false);
        assert(c.back() == true);

        i = c.emplace(c.cbegin()+1, true);
        assert(i == c.begin()+1);
        assert(c.size() == 3);
        assert(c.size() == 3);
        assert(c.front() == false);
        assert(c[1] == true);
        assert(c.back() == true);
    }
}
