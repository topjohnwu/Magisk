//===-------------------------- cxa_virtual.cpp ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "cxxabi.h"
#include "abort_message.h"

namespace __cxxabiv1 {
extern "C" {
_LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN
void __cxa_pure_virtual(void) {
  abort_message("Pure virtual function called!");
}

_LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN
void __cxa_deleted_virtual(void) {
  abort_message("Deleted virtual function called!");
}
} // extern "C"
} // abi
