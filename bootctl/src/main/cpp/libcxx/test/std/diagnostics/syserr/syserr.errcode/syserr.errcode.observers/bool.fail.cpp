//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: c++98, c++03

// <system_error>

// class error_code

// explicit operator bool() const;

#include <system_error>

bool test_func(void)
{
    const std::error_code ec(0, std::generic_category());
    return ec;   // conversion to bool is explicit; should fail.
}

int main()
{
    return 0;
}

