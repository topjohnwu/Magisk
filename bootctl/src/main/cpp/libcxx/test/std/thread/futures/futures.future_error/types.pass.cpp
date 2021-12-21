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

// class future_error : public logic_error {...};

#include <future>
#include <type_traits>

int main()
{
    static_assert((std::is_convertible<std::future_error*,
                                       std::logic_error*>::value), "");
}
