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

// error_code make_error_code(errc e);

#include <system_error>
#include <cassert>

int main()
{
    {
        std::error_code ec = make_error_code(std::errc::operation_canceled);
        assert(ec.value() == static_cast<int>(std::errc::operation_canceled));
        assert(ec.category() == std::generic_category());
    }
}
