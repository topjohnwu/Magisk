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

#include <system_error>
#include <cassert>
#include <type_traits>

#include "test_macros.h"

void
test(int i)
{
    typedef std::error_condition T;
    typedef std::hash<T> H;
    static_assert((std::is_same<H::argument_type, T>::value), "" );
    static_assert((std::is_same<H::result_type, std::size_t>::value), "" );
    ASSERT_NOEXCEPT(H()(T()));
    H h;
    T ec(i, std::system_category());
    const std::size_t result = h(ec);
    LIBCPP_ASSERT(result == static_cast<std::size_t>(i));
    ((void)result); // Prevent unused warning
}

int main()
{
    test(0);
    test(2);
    test(10);
}
