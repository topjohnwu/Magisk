// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that the __libcpp_version file matches the value of _LIBCPP_VERSION

#include <__config>

#ifndef _LIBCPP_VERSION
#error _LIBCPP_VERSION must be defined
#endif

static const int libcpp_version =
#include <__libcpp_version>
;

static_assert(_LIBCPP_VERSION == libcpp_version,
              "_LIBCPP_VERSION doesn't match __libcpp_version");

int main() {

}
