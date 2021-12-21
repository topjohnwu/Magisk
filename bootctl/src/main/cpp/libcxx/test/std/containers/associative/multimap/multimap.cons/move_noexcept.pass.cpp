//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// multimap(multimap&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value &&
//                 is_nothrow_move_constructible<key_compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

#include <map>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp(const some_comp&);
    bool operator()(const T&, const T&) const { return false; }
};

int main()
{
#if defined(_LIBCPP_VERSION)
    typedef std::pair<const MoveOnly, MoveOnly> V;
    {
        typedef std::multimap<MoveOnly, MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::multimap<MoveOnly, MoveOnly, std::less<MoveOnly>, test_allocator<V>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::multimap<MoveOnly, MoveOnly, std::less<MoveOnly>, other_allocator<V>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
#endif // _LIBCPP_VERSION
    {
        typedef std::multimap<MoveOnly, MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
}
