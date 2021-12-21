//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// multiset& operator=(multiset&& c)
//     noexcept(
//          allocator_type::propagate_on_container_move_assignment::value &&
//          is_nothrow_move_assignable<allocator_type>::value &&
//          is_nothrow_move_assignable<key_compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

#include <set>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp& operator=(const some_comp&);
    bool operator()(const T&, const T&) const { return false; }
};

int main()
{
    {
        typedef std::multiset<MoveOnly> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef std::multiset<MoveOnly, std::less<MoveOnly>, test_allocator<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#if defined(_LIBCPP_VERSION)
    {
        typedef std::multiset<MoveOnly, std::less<MoveOnly>, other_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
#endif // _LIBCPP_VERSION
    {
        typedef std::multiset<MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
}
