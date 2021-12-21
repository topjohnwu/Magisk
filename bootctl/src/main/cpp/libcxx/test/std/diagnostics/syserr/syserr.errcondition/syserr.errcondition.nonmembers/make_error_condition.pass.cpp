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

// error_condition make_error_condition(errc e);

#include <system_error>
#include <cassert>

int main()
{
    {
        const std::error_condition ec1 = std::make_error_condition(std::errc::message_size);
        assert(ec1.value() == static_cast<int>(std::errc::message_size));
        assert(ec1.category() == std::generic_category());
    }
}
