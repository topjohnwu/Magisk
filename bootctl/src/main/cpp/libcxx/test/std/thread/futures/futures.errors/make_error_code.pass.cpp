//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <future>

// class error_code

// error_code make_error_code(future_errc e);

#include <future>
#include <cassert>

int main()
{
    {
        std::error_code ec = make_error_code(std::future_errc::broken_promise);
        assert(ec.value() == static_cast<int>(std::future_errc::broken_promise));
        assert(ec.category() == std::future_category());
    }
}
