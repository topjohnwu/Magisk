//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// See bugs.llvm.org/PR20183
//
// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9
// XFAIL: with_system_cxx_lib=macosx10.8
// XFAIL: with_system_cxx_lib=macosx10.7

// <random>

// class random_device;

// explicit random_device(const string& token = implementation-defined);

// For the following ctors, the standard states: "The semantics and default
// value of the token parameter are implementation-defined". Implementations
// therefore aren't required to accept any string, but the default shouldn't
// throw.

#include <random>
#include <system_error>
#include <cassert>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "test_macros.h"


bool is_valid_random_device(const std::string &token) {
#if defined(_LIBCPP_USING_DEV_RANDOM)
  // Not an exhaustive list: they're the only tokens that are tested below.
  return token == "/dev/urandom" || token == "/dev/random";
#else
  return token == "/dev/urandom";
#endif
}

void check_random_device_valid(const std::string &token) {
  std::random_device r(token);
}

void check_random_device_invalid(const std::string &token) {
#ifndef TEST_HAS_NO_EXCEPTIONS
  try {
    std::random_device r(token);
    LIBCPP_ASSERT(false);
  } catch (const std::system_error&) {
  }
#else
  ((void)token);
#endif
}


int main() {
  {
    std::random_device r;
  }
  {
    std::string token = "wrong file";
    check_random_device_invalid(token);
  }
  {
    std::string token = "/dev/urandom";
    if (is_valid_random_device(token))
      check_random_device_valid(token);
    else
      check_random_device_invalid(token);
  }
  {
    std::string token = "/dev/random";
    if (is_valid_random_device(token))
      check_random_device_valid(token);
    else
      check_random_device_invalid(token);
  }
#if !defined(_WIN32)
// Test that random_device(const string&) properly handles getting
// a file descriptor with the value '0'. Do this by closing the standard
// streams so that the descriptor '0' is available.
  {
    int ec;
    ec = close(STDIN_FILENO);
    assert(!ec);
    ec = close(STDOUT_FILENO);
    assert(!ec);
    ec = close(STDERR_FILENO);
    assert(!ec);
    std::random_device r;
  }
#endif // !defined(_WIN32)
}
