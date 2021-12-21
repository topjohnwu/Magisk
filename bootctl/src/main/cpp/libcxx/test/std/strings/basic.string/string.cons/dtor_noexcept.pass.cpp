//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <string>

// ~basic_string() // implied noexcept;

#include <string>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"

template <class T>
struct throwing_alloc
{
    typedef T value_type;
    throwing_alloc(const throwing_alloc&);
    T *allocate(size_t);
    ~throwing_alloc() noexcept(false);
};

// Test that it's possible to take the address of basic_string's destructors
// by creating globals which will register their destructors with cxa_atexit.
std::string s;
std::wstring ws;

int main()
{
    {
        typedef std::string C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, test_allocator<char>> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
#if defined(_LIBCPP_VERSION)
    {
        typedef std::basic_string<char, std::char_traits<char>, throwing_alloc<char>> C;
        static_assert(!std::is_nothrow_destructible<C>::value, "");
    }
#endif // _LIBCPP_VERSION
}
