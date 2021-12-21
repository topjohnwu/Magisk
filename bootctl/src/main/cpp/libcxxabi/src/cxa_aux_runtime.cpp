//===------------------------ cxa_aux_runtime.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//
// This file implements the "Auxiliary Runtime APIs"
// http://mentorembedded.github.io/cxx-abi/abi-eh.html#cxx-aux
//===----------------------------------------------------------------------===//

#include "cxxabi.h"
#include <new>
#include <typeinfo>

namespace __cxxabiv1 {
extern "C" {
_LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_bad_cast(void) {
#ifndef _LIBCXXABI_NO_EXCEPTIONS
  throw std::bad_cast();
#else
  std::terminate();
#endif
}

_LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_bad_typeid(void) {
#ifndef _LIBCXXABI_NO_EXCEPTIONS
  throw std::bad_typeid();
#else
  std::terminate();
#endif
}

_LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void
__cxa_throw_bad_array_new_length(void) {
#ifndef _LIBCXXABI_NO_EXCEPTIONS
  throw std::bad_array_new_length();
#else
  std::terminate();
#endif
}
} // extern "C"
} // abi
