//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_multiset

// void swap(unordered_multiset& x, unordered_multiset& y);

#if _LIBCPP_DEBUG >= 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <unordered_set>
#include <cassert>

int main()
{
#if _LIBCPP_DEBUG >= 1
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::unordered_multiset<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        std::unordered_multiset<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        std::unordered_multiset<int>::iterator i1 = c1.begin();
        std::unordered_multiset<int>::iterator i2 = c2.begin();
        swap(c1, c2);
        c1.erase(i2);
        c2.erase(i1);
        std::unordered_multiset<int>::iterator j = i1;
        c1.erase(i1);
        assert(false);
    }
#endif
}
