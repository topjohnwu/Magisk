//===------------------------- cxa_exception.cpp --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//  
//  This file implements the "Exception Handling APIs"
//  http://mentorembedded.github.io/cxx-abi/abi-eh.html
//  
//===----------------------------------------------------------------------===//

// Support functions for the no-exceptions libc++ library

#include "cxxabi.h"

#include <exception>        // for std::terminate
#include "cxa_exception.hpp"
#include "cxa_handlers.hpp"

namespace __cxxabiv1 {

extern "C" {

void
__cxa_increment_exception_refcount(void *thrown_object) throw() {
    if (thrown_object != nullptr)
        std::terminate();
}

void
__cxa_decrement_exception_refcount(void *thrown_object) throw() {
    if (thrown_object != nullptr)
      std::terminate();
}


void *__cxa_current_primary_exception() throw() { return nullptr; }

void
__cxa_rethrow_primary_exception(void* thrown_object) {
    if (thrown_object != nullptr)
      std::terminate();
}

bool
__cxa_uncaught_exception() throw() { return false; }

unsigned int
__cxa_uncaught_exceptions() throw() { return 0; }

}  // extern "C"

// provide dummy implementations for the 'no exceptions' case.
uint64_t __getExceptionClass  (const _Unwind_Exception*)           { return 0; }
void     __setExceptionClass  (      _Unwind_Exception*, uint64_t) {}
bool     __isOurExceptionClass(const _Unwind_Exception*)           { return false; }

}  // abi
