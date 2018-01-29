/*
 * Copyright 2013 The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Google Inc. nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Google Inc. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL Google Inc. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SYSTEM_CORE_INCLUDE_MINCRYPT_P256_ECDSA_H_
#define SYSTEM_CORE_INCLUDE_MINCRYPT_P256_ECDSA_H_

// Using current directory as relative include path here since
// this code typically gets lifted into a variety of build systems
// and directory structures.
#include "p256.h"

#ifdef __cplusplus
extern "C" {
#endif

// Returns 0 if {r,s} is not a signature on message for
// public key {key_x,key_y}.
//
// Note: message is a p256_int.
// Convert from a binary string using p256_from_bin().
int p256_ecdsa_verify(const p256_int* key_x,
                      const p256_int* key_y,
                      const p256_int* message,
                      const p256_int* r, const p256_int* s);

#ifdef __cplusplus
}
#endif

#endif  // SYSTEM_CORE_INCLUDE_MINCRYPT_P256_ECDSA_H_
