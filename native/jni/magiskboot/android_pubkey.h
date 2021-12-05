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

// Size of an RSA modulus such as an encrypted block or a signature.
#define ANDROID_PUBKEY_MODULUS_SIZE (4096 / 8)

// Size of an encoded RSA key.
#define ANDROID_PUBKEY_ENCODED_SIZE (2 * sizeof(uint32_t) + 2 * ANDROID_PUBKEY_MODULUS_SIZE)

/* Allocates a new RSA |key| object, decodes a public RSA key stored in
 * Android's custom binary format from |key_buffer| and sets the key parameters
 * in |key|. |size| specifies the size of the key buffer and must be at least
 * |ANDROID_PUBKEY_ENCODED_SIZE|. The resulting |*key| can be used with the
 * standard BoringSSL API to perform public operations.
 *
 * Returns true if successful, in which case the caller receives ownership of
 * the |*key| object, i.e. needs to call RSA_free() when done with it. If there
 * is an error, |key| is left untouched and the return value will be false.
 */
bool android_pubkey_decode(const uint8_t* key_buffer, size_t size, RSA** key);

/* Encodes |key| in the Android RSA public key binary format and stores the
 * bytes in |key_buffer|. |key_buffer| should be of size at least
 * |ANDROID_PUBKEY_ENCODED_SIZE|.
 *
 * Returns true if successful, false on error.
 */
bool android_pubkey_encode(const RSA* key, uint8_t* key_buffer, size_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // CRYPTO_UTILS_ANDROID_PUBKEY_H
