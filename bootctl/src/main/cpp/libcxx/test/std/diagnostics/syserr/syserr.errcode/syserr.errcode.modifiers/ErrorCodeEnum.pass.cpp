//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// class error_code

// template <ErrorCodeEnum E> error_code& operator=(E e);

#include <system_error>
#include <cassert>

enum testing
{
    zero, one, two
};

namespace std
{

template <> struct is_error_code_enum<testing> : public std::true_type {};

}

std::error_code
make_error_code(testing x)
{
    return std::error_code(static_cast<int>(x), std::generic_category());
}

int main()
{
    {
        std::error_code ec;
        ec = two;
        assert(ec.value() == 2);
        assert(ec.category() == std::generic_category());
    }
}
