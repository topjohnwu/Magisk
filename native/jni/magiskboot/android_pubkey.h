/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CRYPTO_UTILS_ANDROID_PUBKEY_H
#define CRYPTO_UTILS_ANDROID_PUBKEY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <openssl/rsa.h>

#ifdef __cplusplus
extern "C" {
#endif

// https://android.googlesource.com/platform/system/core/+/refs/tags/android-12.0.0_r12/libcrypto_utils/include/crypto_utils/android_pubkey.h

/* Allocates a new RSA |key| object, decodes a public RSA key stored in
 * |AvbRSAPublicKeyHeader| format from |key_buffer| and sets the key parameters
 * in |key|. |size| specifies the size of the key buffer.
 * The resulting |*key| can be used with the
 * standard BoringSSL API to perform public operations.
 *
 * Returns true if successful, in which case the caller receives ownership of
 * the |*key| object, i.e. needs to call RSA_free() when done with it. If there
 * is an error, |key| is left untouched and the return value will be false.
 */
bool android_pubkey_decode(const uint8_t* data, size_t length, RSA** key);

/* Encodes |key| in the |AvbRSAPublicKeyHeader| format and stores the
 * bytes in |key_buffer|.
 *
 * Returns true if successful, false on error.
 */
bool android_pubkey_encode(const RSA* key, uint8_t* data, size_t length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // CRYPTO_UTILS_ANDROID_PUBKEY_H
