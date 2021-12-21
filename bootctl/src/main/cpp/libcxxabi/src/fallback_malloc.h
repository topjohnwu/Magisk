//===------------------------- fallback_malloc.h --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _FALLBACK_MALLOC_H
#define _FALLBACK_MALLOC_H

#include "__cxxabi_config.h"
#include <cstddef> // for size_t

namespace __cxxabiv1 {

// Allocate some memory from _somewhere_
_LIBCXXABI_HIDDEN void * __aligned_malloc_with_fallback(size_t size);

// Allocate and zero-initialize memory from _somewhere_
_LIBCXXABI_HIDDEN void * __calloc_with_fallback(size_t count, size_t size);

_LIBCXXABI_HIDDEN void __aligned_free_with_fallback(void *ptr);
_LIBCXXABI_HIDDEN void __free_with_fallback(void *ptr);

} // namespace __cxxabiv1

#endif
