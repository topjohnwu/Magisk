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

#include <string.h>

#include "mincrypt/p256_ecdsa.h"
#include "mincrypt/p256.h"

int p256_ecdsa_verify(const p256_int* key_x, const p256_int* key_y,
                      const p256_int* message,
                      const p256_int* r, const p256_int* s) {
  p256_int u, v;

  // Check public key.
  if (!p256_is_valid_point(key_x, key_y)) return 0;

  // Check r and s are != 0 % n.
  p256_mod(&SECP256r1_n, r, &u);
  p256_mod(&SECP256r1_n, s, &v);
  if (p256_is_zero(&u) || p256_is_zero(&v)) return 0;

  p256_modinv_vartime(&SECP256r1_n, s, &v);
  p256_modmul(&SECP256r1_n, message, 0, &v, &u);  // message / s % n
  p256_modmul(&SECP256r1_n, r, 0, &v, &v);  // r / s % n

  p256_points_mul_vartime(&u, &v,
                          key_x, key_y,
                          &u, &v);

  p256_mod(&SECP256r1_n, &u, &u);  // (x coord % p) % n
  return p256_cmp(r, &u) == 0;
}

