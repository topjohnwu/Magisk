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
// class unordered_set

// iterator erase(const_iterator p)

#include <unordered_set>
#include <cassert>

#include "min_allocator.h"

struct TemplateConstructor
{
    template<typename T>
    TemplateConstructor (const T&) {}
};

bool operator==(const TemplateConstructor&, const TemplateConstructor&) { return false; }
struct Hash { size_t operator() (const TemplateConstructor &) const { return 0; } };

int main()
{
    {
        typedef std::unordered_set<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        C::const_iterator i = c.find(2);
        C::const_iterator i_next = i;
        ++i_next;
        C::iterator j = c.erase(i);
        assert(j == i_next);

        assert(c.size() == 3);
        assert(c.count(1) == 1);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        C::const_iterator i = c.find(2);
        C::const_iterator i_next = i;
        ++i_next;
        C::iterator j = c.erase(i);
        assert(j == i_next);

        assert(c.size() == 3);
        assert(c.count(1) == 1);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
    }
#endif
#if TEST_STD_VER >= 14
    {
    //  This is LWG #2059
        typedef TemplateConstructor T;
        typedef std::unordered_set<T, Hash> C;
        typedef C::iterator I;

        C m;
        T a{0};
        I it = m.find(a);
        if (it != m.end())
            m.erase(it);
    }
#endif
}
