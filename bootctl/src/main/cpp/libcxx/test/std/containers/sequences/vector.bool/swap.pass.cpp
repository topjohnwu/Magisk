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

// void swap(vector& x);

#include <vector>
#include <cassert>
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        std::vector<bool> v1(100);
        std::vector<bool> v2(200);
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() >= 200);
        assert(v2.size() == 100);
        assert(v2.capacity() >= 100);
    }
    {
        typedef test_allocator<bool> A;
        std::vector<bool, A> v1(100, true, A(1, 1));
        std::vector<bool, A> v2(200, false, A(1, 2));
        swap(v1, v2);
        assert(v1.size() == 200);
        assert(v1.capacity() >= 200);
        assert(v2.size() == 100);
        assert(v2.capacity() >= 100);
        assert(v1.get_allocator().get_id() == 1);
        assert(v2.get_allocator().get_id() == 2);
    }
    {
        typedef other_allocator<bool> A;
        std::vector<bool, A> v1(100, true, A(1));
        std::vector<bool, A> v2(200, false, A(2));
        swap(v1, v2);
        assert(v1.size() == 200);
        assert(v1.capacity() >= 200);
        assert(v2.size() == 100);
        assert(v2.capacity() >= 100);
        assert(v1.get_allocator() == A(2));
        assert(v2.get_allocator() == A(1));
    }
    {
        std::vector<bool> v(2);
        std::vector<bool>::reference r1 = v[0];
        std::vector<bool>::reference r2 = v[1];
        r1 = true;
        using std::swap;
        swap(r1, r2);
        assert(v[0] == false);
        assert(v[1] == true);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool>> v1(100);
        std::vector<bool, min_allocator<bool>> v2(200);
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() >= 200);
        assert(v2.size() == 100);
        assert(v2.capacity() >= 100);
    }
    {
        typedef min_allocator<bool> A;
        std::vector<bool, A> v1(100, true, A());
        std::vector<bool, A> v2(200, false, A());
        swap(v1, v2);
        assert(v1.size() == 200);
        assert(v1.capacity() >= 200);
        assert(v2.size() == 100);
        assert(v2.capacity() >= 100);
        assert(v1.get_allocator() == A());
        assert(v2.get_allocator() == A());
    }
    {
        std::vector<bool, min_allocator<bool>> v(2);
        std::vector<bool, min_allocator<bool>>::reference r1 = v[0];
        std::vector<bool, min_allocator<bool>>::reference r2 = v[1];
        r1 = true;
        using std::swap;
        swap(r1, r2);
        assert(v[0] == false);
        assert(v[1] == true);
    }
#endif
}
