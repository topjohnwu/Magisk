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

// template <class... Args> reference emplace_back(Args&&... args);
// return type is 'reference' in C++17; 'void' before

#include <vector>
#include <cassert>
#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "test_allocator.h"
#include "asan_testing.h"

class A
{
    int i_;
    double d_;

    A(const A&);
    A& operator=(const A&);
public:
    A(int i, double d)
        : i_(i), d_(d) {}

    A(A&& a)
        : i_(a.i_),
          d_(a.d_)
    {
        a.i_ = 0;
        a.d_ = 0;
    }

    A& operator=(A&& a)
    {
        i_ = a.i_;
        d_ = a.d_;
        a.i_ = 0;
        a.d_ = 0;
        return *this;
    }

    int geti() const {return i_;}
    double getd() const {return d_;}
};

int main()
{
    {
        std::vector<A> c;
#if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
#else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
#endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, limited_allocator<A, 4> > c;
#if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
#else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
#endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, min_allocator<A>> c;
#if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
#else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
#endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<Tag_X, TaggingAllocator<Tag_X>> c;
        c.emplace_back();
        assert(c.size() == 1);
        c.emplace_back(1, 2, 3);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
    }
}
