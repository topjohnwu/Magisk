//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// ~unordered_multimap() // implied noexcept;

// UNSUPPORTED: c++98, c++03

#include <unordered_map>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    ~some_comp() noexcept(false);
    bool operator()(const T&, const T&) const { return false; }
};

template <class T>
struct some_hash
{
    typedef T value_type;
    some_hash();
    some_hash(const some_hash&);
    ~some_hash() noexcept(false);
    std::size_t operator()(T const&) const;
};

int main()
{
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                           std::equal_to<MoveOnly>, test_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                          std::equal_to<MoveOnly>, other_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
#if defined(_LIBCPP_VERSION)
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly, some_hash<MoveOnly>> C;
        static_assert(!std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                                                         some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_destructible<C>::value, "");
    }
#endif // _LIBCPP_VERSION
}
