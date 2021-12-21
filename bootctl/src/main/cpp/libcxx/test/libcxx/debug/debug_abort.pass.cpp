// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// MODULES_DEFINES: _LIBCPP_DEBUG=0

// Can't test the system lib because this test enables debug mode
// UNSUPPORTED: with_system_cxx_lib

// Test that the default debug handler aborts the program.

#define _LIBCPP_DEBUG 0

#include <csignal>
#include <cstdlib>
#include <__debug>

void signal_handler(int signal)
{
    if (signal == SIGABRT)
      std::_Exit(EXIT_SUCCESS);
    std::_Exit(EXIT_FAILURE);
}

int main()
{
  if (std::signal(SIGABRT, signal_handler) != SIG_ERR)
    _LIBCPP_ASSERT(false, "foo");
  return EXIT_FAILURE;
}
