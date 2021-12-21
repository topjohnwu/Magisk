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

// bool operator<(const error_condition& lhs, const error_condition& rhs);

#include <system_error>
#include <string>
#include <cassert>

int main()
{
    {
        const std::error_condition ec1(6, std::generic_category());
        const std::error_condition ec2(7, std::generic_category());
        assert(ec1 < ec2);
    }
}
