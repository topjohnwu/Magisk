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

#include "android_pubkey.h"

#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/bn.h>

// Better safe than sorry.
#if (ANDROID_PUBKEY_MODULUS_SIZE % 4) != 0
#error RSA modulus size must be multiple of the word size!
#endif

// This file implements encoding and decoding logic for Android's custom RSA
// public key binary format. Public keys are stored as a sequence of
// little-endian 32 bit words. Note that Android only supports little-endian
// processors, so we don't do any byte order conversions when parsing the binary
// struct.
struct RSAPublicKey {
  // Modulus size in bits. This must be ANDROID_PUBKEY_MODULUS_SIZE * 8.
  uint32_t modulus_bit_size;

  // Precomputed montgomery parameter: -1 / n[0] mod 2^32
  uint32_t n0inv;

  // RSA modulus as a big-endian array.
  uint8_t modulus[ANDROID_PUBKEY_MODULUS_SIZE];

  // Montgomery parameter R^2 as a big-endian array.
  uint8_t rr[ANDROID_PUBKEY_MODULUS_SIZE];
};

bool android_pubkey_decode(const uint8_t* key_buffer, size_t size, RSA** key) {
  const RSAPublicKey* key_struct = (RSAPublicKey*)key_buffer;
  bool ret = false;
  RSA* new_key = RSA_new();
  BIGNUM* n = NULL;
  BIGNUM* e = NULL;
  if (!new_key) {
    goto cleanup;
  }

  // Check |size| is large enough and the modulus size is correct.
  if (size < sizeof(RSAPublicKey)) {
    goto cleanup;
  }
  if (__builtin_bswap32(key_struct->modulus_bit_size) != ANDROID_PUBKEY_MODULUS_SIZE * 8) {
    goto cleanup;
  }

  n = BN_bin2bn(key_struct->modulus, ANDROID_PUBKEY_MODULUS_SIZE, NULL);
  if (!n) {
    goto cleanup;
  }

  e = BN_new();
  if (!e || !BN_set_word(e, 65537)) {
    goto cleanup;
  }

  if (!RSA_set0_key(new_key, n, e, NULL)) {
    goto cleanup;
  }
  // RSA_set0_key takes ownership of its inputs on success.
  n = NULL;
  e = NULL;

  // Note that we don't extract the montgomery parameters n0inv and rr from
  // the RSAPublicKey structure. They assume a word size of 32 bits, but
  // BoringSSL may use a word size of 64 bits internally, so we're lacking the
  // top 32 bits of n0inv in general. For now, we just ignore the parameters
  // and have BoringSSL recompute them internally. More sophisticated logic can
  // be added here if/when we want the additional speedup from using the
  // pre-computed montgomery parameters.

  *key = new_key;
  new_key = NULL;
  ret = true;

cleanup:
  RSA_free(new_key);
  BN_free(n);
  BN_free(e);
  return ret;
}

bool android_pubkey_encode(const RSA* key, uint8_t* key_buffer, size_t size) {
  RSAPublicKey* key_struct = (RSAPublicKey*)key_buffer;
  bool ret = false;
  BN_CTX* ctx = BN_CTX_new();
  BIGNUM* r32 = BN_new();
  BIGNUM* n0inv = BN_new();
  BIGNUM* rr = BN_new();

  if (sizeof(RSAPublicKey) > size || RSA_size(key) != ANDROID_PUBKEY_MODULUS_SIZE) {
    goto cleanup;
  }

  // Store the modulus size.
  key_struct->modulus_bit_size = __builtin_bswap32(ANDROID_PUBKEY_MODULUS_SIZE * 8);

  // Compute and store n0inv = -1 / N[0] mod 2^32.
  if (!ctx || !r32 || !n0inv || !BN_set_bit(r32, 32) || !BN_mod(n0inv, RSA_get0_n(key), r32, ctx) ||
      !BN_mod_inverse(n0inv, n0inv, r32, ctx) || !BN_sub(n0inv, r32, n0inv)) {
    goto cleanup;
  }
  key_struct->n0inv = __builtin_bswap32((uint32_t)BN_get_word(n0inv));

  // Store the modulus.
  if (!BN_bn2bin_padded(key_struct->modulus, ANDROID_PUBKEY_MODULUS_SIZE, RSA_get0_n(key))) {
    goto cleanup;
  }

  // Compute and store rr = (2^(rsa_size)) ^ 2 mod N.
  if (!ctx || !rr || !BN_set_bit(rr, ANDROID_PUBKEY_MODULUS_SIZE * 8) ||
      !BN_mod_sqr(rr, rr, RSA_get0_n(key), ctx) ||
      !BN_bn2bin_padded(key_struct->rr, ANDROID_PUBKEY_MODULUS_SIZE, rr)) {
    goto cleanup;
  }

  ret = true;

cleanup:
  BN_free(rr);
  BN_free(n0inv);
  BN_free(r32);
  BN_CTX_free(ctx);
  return ret;
}
