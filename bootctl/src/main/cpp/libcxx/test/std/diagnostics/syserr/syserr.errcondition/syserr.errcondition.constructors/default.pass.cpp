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

// error_condition();

#include <system_error>
#include <cassert>

int main()
{
    std::error_condition ec;
    assert(ec.value() == 0);
    assert(ec.category() == std::generic_category());
}
