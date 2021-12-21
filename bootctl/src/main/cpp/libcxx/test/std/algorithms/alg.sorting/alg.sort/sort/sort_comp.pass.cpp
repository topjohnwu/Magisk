//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<RandomAccessIterator Iter, StrictWeakOrder<auto, Iter::value_type> Compare>
//   requires ShuffleIterator<Iter>
//         && CopyConstructible<Compare>
//   void
//   sort(Iter first, Iter last, Compare comp);

#include <algorithm>
#include <functional>
#include <vector>
#include <cassert>
#include <cstddef>
#include <memory>

#include "test_macros.h"

struct indirect_less
{
    template <class P>
    bool operator()(const P& x, const P& y)
        {return *x < *y;}
};

int main()
{
    {
    std::vector<int> v(1000);
    for (int i = 0; static_cast<std::size_t>(i) < v.size(); ++i)
        v[i] = i;
    std::sort(v.begin(), v.end(), std::greater<int>());
    std::reverse(v.begin(), v.end());
    assert(std::is_sorted(v.begin(), v.end()));
    }

#if TEST_STD_VER >= 11
    {
    std::vector<std::unique_ptr<int> > v(1000);
    for (int i = 0; static_cast<std::size_t>(i) < v.size(); ++i)
        v[i].reset(new int(i));
    std::sort(v.begin(), v.end(), indirect_less());
    assert(std::is_sorted(v.begin(), v.end(), indirect_less()));
    assert(*v[0] == 0);
    assert(*v[1] == 1);
    assert(*v[2] == 2);
    }
#endif
}
