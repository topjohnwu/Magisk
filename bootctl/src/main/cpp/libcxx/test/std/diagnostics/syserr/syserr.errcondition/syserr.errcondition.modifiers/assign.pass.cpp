//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// class error_condition

// void assign(int val, const error_category& cat);

#include <system_error>
#include <cassert>

int main()
{
    {
        std::error_condition ec;
        ec.assign(6, std::system_category());
        assert(ec.value() == 6);
        assert(ec.category() == std::system_category());
    }
    {
        std::error_condition ec;
        ec.assign(8, std::generic_category());
        assert(ec.value() == 8);
        assert(ec.category() == std::generic_category());
    }
}
