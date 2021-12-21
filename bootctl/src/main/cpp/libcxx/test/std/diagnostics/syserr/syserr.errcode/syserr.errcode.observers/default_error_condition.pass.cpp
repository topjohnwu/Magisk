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

// error_condition default_error_condition() const;

#include <system_error>
#include <cassert>

int main()
{
    {
        const std::error_code ec(6, std::generic_category());
        std::error_condition e_cond = ec.default_error_condition();
        assert(e_cond == ec);
    }
    {
        const std::error_code ec(6, std::system_category());
        std::error_condition e_cond = ec.default_error_condition();
        assert(e_cond == ec);
    }
}
