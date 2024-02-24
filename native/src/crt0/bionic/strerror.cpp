/*
 * Copyright (C) 2012 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// -std=gnu++XX automatically defines _GNU_SOURCE, which then means that <string.h>
// gives us the GNU variant, which is not what we're defining here.
#undef _GNU_SOURCE

#include <string.h>

#include <errno.h>
#include <limits.h>

#include "private/ErrnoRestorer.h"

#include <string.h>
#include <stdio.h>

// The arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that arraysize() doesn't accept any array of an
// anonymous type or a type defined inside a function.
//
// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];  // NOLINT(readability/casting)

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

static const char* __sys_error_descriptions[] = {
#define __BIONIC_ERRDEF(error_number, error_description) [error_number] = error_description,
#include "private/bionic_errdefs.h"
};

static inline const char* __strerror_lookup(int error_number) {
  if (error_number < 0 || error_number >= static_cast<int>(arraysize(__sys_error_descriptions))) {
    return nullptr;
  }
  return __sys_error_descriptions[error_number];
}

int strerror_r(int error_number, char* buf, size_t buf_len) {
  ErrnoRestorer errno_restorer;
  size_t length;

  const char* error_name = __strerror_lookup(error_number);
  if (error_name != nullptr) {
    length = strlcpy(buf, error_name, buf_len);
  } else {
    length = snprintf(buf, buf_len, "Unknown error %d", error_number);
  }
  if (length >= buf_len) {
    return ERANGE;
  }

  return 0;
}

char* strerror(int error_number) {
  static char strerror_buf[NL_TEXTMAX];

  // Just return the original constant in the easy cases.
  char* result = const_cast<char*>(__strerror_lookup(error_number));
  if (result != nullptr) {
    return result;
  }

  result = strerror_buf;
  strerror_r(error_number, result, sizeof(strerror_buf));
  return result;
}
