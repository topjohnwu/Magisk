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
//     static size_type max_size(const allocator_type& a) noexcept;
//     ...
// };

#include <limits>
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

template <class T>
struct B
{
    typedef T value_type;

    size_t max_size() const
    {
        return 100;
    }
};

int main()
{
    {
        B<int> b;
        assert(std::allocator_traits<B<int> >::max_size(b) == 100);
    }
    {
        const B<int> b = {};
        assert(std::allocator_traits<B<int> >::max_size(b) == 100);
    }
    {
        typedef IncompleteHolder* VT;
        typedef B<VT> Alloc;
        Alloc a;
        assert(std::allocator_traits<Alloc >::max_size(a) == 100);
    }
#if TEST_STD_VER >= 11
    {
        A<int> a;
        assert(std::allocator_traits<A<int> >::max_size(a) ==
               std::numeric_limits<std::size_t>::max() / sizeof(int));
    }
    {
        const A<int> a = {};
        assert(std::allocator_traits<A<int> >::max_size(a) ==
               std::numeric_limits<std::size_t>::max() / sizeof(int));
    }
    {
        std::allocator<int> a;
        static_assert(noexcept(std::allocator_traits<std::allocator<int>>::max_size(a)) == true, "");
    }
#endif
}
