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

// <condition_variable>

// enum class cv_status { no_timeout, timeout };

#include <condition_variable>
#include <cassert>

int main()
{
    assert(static_cast<int>(std::cv_status::no_timeout) == 0);
    assert(static_cast<int>(std::cv_status::timeout)    == 1);
}
