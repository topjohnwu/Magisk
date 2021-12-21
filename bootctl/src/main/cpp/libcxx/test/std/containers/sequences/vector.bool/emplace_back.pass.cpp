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
//  vector.bool

// template <class... Args> reference emplace_back(Args&&... args);
// return type is 'reference' in C++17; 'void' before

#include <vector>
#include <cassert>
#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::vector<bool> C;
        C c;
#if TEST_STD_VER > 14
        typedef C::reference Ref;
        Ref r1 = c.emplace_back();
        assert(c.size() == 1);
        assert(c.front() == false);
        r1 = true;
        assert(c.front() == true);
        r1 = false;
        Ref r2 = c.emplace_back(true);
        assert(c.size() == 2);
        assert(c.front() == false);
        assert(c.back() == true);
        r2 = false;
        assert(c.back() == false);
        r2 = true;
#else
        c.emplace_back();
        assert(c.size() == 1);
        assert(c.front() == false);
        c.emplace_back(true);
        assert(c.size() == 2);
        assert(c.front() == false);
        assert(c.back() == true);
#endif
        c.emplace_back(true);
        assert(c.size() == 3);
        assert(c.front() == false);
        assert(c[1] == true);
        assert(c.back() == true);
    }
    {
        typedef std::vector<bool, min_allocator<bool>> C;
        C c;

#if TEST_STD_VER > 14
        typedef C::reference Ref;
        Ref r1 = c.emplace_back();
        assert(c.size() == 1);
        assert(c.front() == false);
        r1 = true;
        assert(c.front() == true);
        r1 = false;
        Ref r2 = c.emplace_back(true);
        assert(c.size() == 2);
        assert(c.front() == false);
        assert(c.back() == true);
        r2 = false;
        assert(c.back() == false);
        r2 = true;
#else
        c.emplace_back();
        assert(c.size() == 1);
        assert(c.front() == false);
        c.emplace_back(true);
        assert(c.size() == 2);
        assert(c.front() == false);
        assert(c.back() == true);
#endif
        c.emplace_back(true);
        assert(c.size() == 3);
        assert(c.front() == false);
        assert(c[1] == true);
        assert(c.back() == true);
    }
}
