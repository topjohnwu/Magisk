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

// class error_condition

// error_condition make_error_condition(future_errc e);

#include <future>
#include <cassert>

int main()
{
    {
        const std::error_condition ec1 =
          std::make_error_condition(std::future_errc::future_already_retrieved);
        assert(ec1.value() ==
                  static_cast<int>(std::future_errc::future_already_retrieved));
        assert(ec1.category() == std::future_category());
    }
}
