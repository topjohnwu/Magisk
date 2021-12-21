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

// error_code();

#include <system_error>
#include <cassert>

int main()
{
    std::error_code ec;
    assert(ec.value() == 0);
    assert(ec.category() == std::system_category());
}
