//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// template <class T>
// struct hash
//     : public unary_function<T, size_t>
// {
//     size_t operator()(T val) const;
// };

// Not very portable

#include <vector>
#include <cassert>
#include <type_traits>

#include "min_allocator.h"

int main()
{
    {
    typedef std::vector<bool> T;
    typedef std::hash<T> H;
    static_assert((std::is_same<H::argument_type, T>::value), "" );
    static_assert((std::is_same<H::result_type, std::size_t>::value), "" );
    ASSERT_NOEXCEPT(H()(T()));

    bool ba[] = {true, false, true, true, false};
    T vb(std::begin(ba), std::end(ba));
    H h;
    assert(h(vb) != 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::vector<bool, min_allocator<bool>> T;
    typedef std::hash<T> H;
    static_assert((std::is_same<H::argument_type, T>::value), "" );
    static_assert((std::is_same<H::result_type, std::size_t>::value), "" );
    ASSERT_NOEXCEPT(H()(T()));
    bool ba[] = {true, false, true, true, false};
    T vb(std::begin(ba), std::end(ba));
    H h;
    assert(h(vb) != 0);
    }
#endif
}
