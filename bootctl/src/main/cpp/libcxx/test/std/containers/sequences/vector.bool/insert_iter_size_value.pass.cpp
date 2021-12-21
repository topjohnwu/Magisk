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

// iterator insert(const_iterator position, size_type n, const value_type& x);

#include <vector>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"

int main()
{
    {
        std::vector<bool> v(100);
        std::vector<bool>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        assert(i == v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        std::vector<bool> v(100);
        while(v.size() < v.capacity()) v.push_back(false);
        size_t sz = v.size();
        std::vector<bool>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == sz + 5);
        assert(i == v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        std::vector<bool> v(100);
        while(v.size() < v.capacity()) v.push_back(false);
        v.pop_back(); v.pop_back();
        size_t sz = v.size();
        std::vector<bool>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == sz + 5);
        assert(i == v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool>> v(100);
        std::vector<bool, min_allocator<bool>>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        assert(i == v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
#endif
}
