//===------------------------ stdexcept.cpp -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "include/refstring.h"
#include "stdexcept"
#include "new"
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstddef>

static_assert(sizeof(std::__libcpp_refstring) == sizeof(const char *), "");

namespace std  // purposefully not using versioning namespace
{

logic_error::~logic_error() _NOEXCEPT {}

const char*
logic_error::what() const _NOEXCEPT
{
    return __imp_.c_str();
}

runtime_error::~runtime_error() _NOEXCEPT {}

const char*
runtime_error::what() const _NOEXCEPT
{
    return __imp_.c_str();
}

domain_error::~domain_error() _NOEXCEPT {}
invalid_argument::~invalid_argument() _NOEXCEPT {}
length_error::~length_error() _NOEXCEPT {}
out_of_range::~out_of_range() _NOEXCEPT {}

range_error::~range_error() _NOEXCEPT {}
overflow_error::~overflow_error() _NOEXCEPT {}
underflow_error::~underflow_error() _NOEXCEPT {}

}  // std
