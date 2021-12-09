/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *            http://www.apache.org/licenses/LICENSE-2.0
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

#include <libavb/libavb.h>
#include <openssl/bn.h>

// https://android.googlesource.com/platform/system/core/+/refs/tags/android-12.0.0_r12/libcrypto_utils/android_pubkey.cpp

bool android_pubkey_decode(const uint8_t* data, size_t length, RSA** key) {
    AvbRSAPublicKeyHeader h;
    size_t expected_length;
    bool ret = false;
    RSA* new_key = RSA_new();
    BIGNUM* n = NULL;
    BIGNUM* e = NULL;
    if (!new_key) {
        goto cleanup;
    }

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_rsa.c#55
    if (!avb_rsa_public_key_header_validate_and_byteswap((const AvbRSAPublicKeyHeader*)data, &h)) {
        // Invalid key.
        goto cleanup;
    }

    if (!(h.key_num_bits == 2048 || h.key_num_bits == 4096 || h.key_num_bits == 8192)) {
        // Unexpected key length.
        goto cleanup;
    }
    
    expected_length = sizeof(AvbRSAPublicKeyHeader) + 2 * h.key_num_bits / 8;
    if (length != expected_length) {
        // Key does not match expected length.
        goto cleanup;
    }

    n = BN_bin2bn(data + sizeof(AvbRSAPublicKeyHeader), h.key_num_bits / 8, NULL);
    if (!n) {
        goto cleanup;
    }

    // Set the exponent.
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

bool android_pubkey_encode(const RSA* key, uint8_t* data, size_t length) {
    AvbRSAPublicKeyHeader h;
    size_t expected_length;
    bool ret = false;
    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* r32 = BN_new();
    BIGNUM* n0inv = BN_new();
    BIGNUM* rr = BN_new();

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/avbtool.py#398
    if (BN_get_word(RSA_get0_e(key)) != 65537) {
        // Only RSA keys with exponent 65537 are supported.
        goto cleanup;
    }

    if (!(RSA_size(key) == 2048 / 8 || RSA_size(key) == 4096 / 8 || RSA_size(key) == 8192 / 8)) {
        // Unexpected key length.
        goto cleanup;
    }

    expected_length = sizeof(AvbRSAPublicKeyHeader) + 2 * RSA_size(key);
    if (length != expected_length) {
        // Key does not match expected length.
        goto cleanup;
    }

    // Store the modulus size.
    h.key_num_bits = RSA_size(key) * 8;

    // Compute and store n0inv = -1 / N[0] mod 2^32.
    if (!ctx || !r32 || !n0inv || !BN_set_bit(r32, 32) || !BN_mod(n0inv, RSA_get0_n(key), r32, ctx) ||
            !BN_mod_inverse(n0inv, n0inv, r32, ctx) || !BN_sub(n0inv, r32, n0inv)) {
        goto cleanup;
    }
    h.n0inv = (uint32_t)BN_get_word(n0inv);

    if (!avb_rsa_public_key_header_validate_and_byteswap(&h, (AvbRSAPublicKeyHeader*)data)) {
        goto cleanup;
    }

    // Store the modulus.
    if (!BN_bn2bin_padded(data + sizeof(AvbRSAPublicKeyHeader), RSA_size(key), RSA_get0_n(key))) {
        goto cleanup;
    }

    // Compute and store rr = (2^(rsa_size)) ^ 2 mod N.
    if (!ctx || !rr || !BN_set_bit(rr, RSA_size(key) * 8) ||
            !BN_mod_sqr(rr, rr, RSA_get0_n(key), ctx) ||
            !BN_bn2bin_padded(data + sizeof(AvbRSAPublicKeyHeader) + h.key_num_bits / 8, RSA_size(key), rr)) {
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
