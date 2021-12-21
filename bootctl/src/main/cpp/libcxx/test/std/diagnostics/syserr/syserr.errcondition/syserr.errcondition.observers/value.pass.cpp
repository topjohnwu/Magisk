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

// int value() const;

#include <system_error>
#include <cassert>

int main()
{
    const std::error_condition ec(6, std::system_category());
    assert(ec.value() == 6);
}
