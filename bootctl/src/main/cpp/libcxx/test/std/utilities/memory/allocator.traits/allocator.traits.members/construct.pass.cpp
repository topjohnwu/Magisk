//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class Alloc>
// struct allocator_traits
// {
//     template <class Ptr, class... Args>
//         static void construct(allocator_type& a, Ptr p, Args&&... args);
//     ...
// };

#include <memory>
#include <new>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "incomplete_type_helper.h"

template <class T>
struct A
{
    typedef T value_type;

};

int b_construct = 0;

template <class T>
struct B
{
    typedef T value_type;

#if TEST_STD_VER >= 11
    template <class U, class ...Args>
    void construct(U* p, Args&& ...args)
    {
        ++b_construct;
        ::new ((void*)p) U(std::forward<Args>(args)...);
    }
#endif
};

struct A0
{
    static int count;
    A0() {++count;}
};

int A0::count = 0;

struct A1
{
    static int count;
    A1(char c)
    {
        assert(c == 'c');
        ++count;
    }
};

int A1::count = 0;

struct A2
{
    static int count;
    A2(char c, int i)
    {
        assert(c == 'd');
        assert(i == 5);
        ++count;
    }
};

int A2::count = 0;

int main()
{
    {
        A0::count = 0;
        A<int> a;
        std::aligned_storage<sizeof(A0)>::type a0;
        assert(A0::count == 0);
        std::allocator_traits<A<int> >::construct(a, (A0*)&a0);
        assert(A0::count == 1);
    }
    {
        A1::count = 0;
        A<int> a;
        std::aligned_storage<sizeof(A1)>::type a1;
        assert(A1::count == 0);
        std::allocator_traits<A<int> >::construct(a, (A1*)&a1, 'c');
        assert(A1::count == 1);
    }
    {
        A2::count = 0;
        A<int> a;
        std::aligned_storage<sizeof(A2)>::type a2;
        assert(A2::count == 0);
        std::allocator_traits<A<int> >::construct(a, (A2*)&a2, 'd', 5);
        assert(A2::count == 1);
    }
    {
      typedef IncompleteHolder* VT;
      typedef A<VT> Alloc;
      Alloc a;
      std::aligned_storage<sizeof(VT)>::type store;
      std::allocator_traits<Alloc>::construct(a, (VT*)&store, nullptr);
    }
#if TEST_STD_VER >= 11
    {
        A0::count = 0;
        b_construct = 0;
        B<int> b;
        std::aligned_storage<sizeof(A0)>::type a0;
        assert(A0::count == 0);
        assert(b_construct == 0);
        std::allocator_traits<B<int> >::construct(b, (A0*)&a0);
        assert(A0::count == 1);
        assert(b_construct == 1);
    }
    {
        A1::count = 0;
        b_construct = 0;
        B<int> b;
        std::aligned_storage<sizeof(A1)>::type a1;
        assert(A1::count == 0);
        assert(b_construct == 0);
        std::allocator_traits<B<int> >::construct(b, (A1*)&a1, 'c');
        assert(A1::count == 1);
        assert(b_construct == 1);
    }
    {
        A2::count = 0;
        b_construct = 0;
        B<int> b;
        std::aligned_storage<sizeof(A2)>::type a2;
        assert(A2::count == 0);
        assert(b_construct == 0);
        std::allocator_traits<B<int> >::construct(b, (A2*)&a2, 'd', 5);
        assert(A2::count == 1);
        assert(b_construct == 1);
    }
#endif
}
