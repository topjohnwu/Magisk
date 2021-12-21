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

// const error_category& category() const;

#include <system_error>
#include <cassert>

int main()
{
    const std::error_condition ec(6, std::generic_category());
    assert(ec.category() == std::generic_category());
}
