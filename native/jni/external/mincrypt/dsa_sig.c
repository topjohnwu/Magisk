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

#include "mincrypt/dsa_sig.h"
#include "mincrypt/p256.h"

/**
 * Trims off the leading zero bytes and copy it to a buffer aligning it to the end.
 */
static inline int trim_to_p256_bytes(unsigned char dst[P256_NBYTES], unsigned char *src,
        int src_len) {
    int dst_offset;
    while (*src == '\0' && src_len > 0) {
        src++;
        src_len--;
    }
    if (src_len > P256_NBYTES || src_len < 1) {
        return 0;
    }
    dst_offset = P256_NBYTES - src_len;
    memset(dst, 0, dst_offset);
    memcpy(dst + dst_offset, src, src_len);
    return 1;
}

/**
 * Unpacks the ASN.1 DSA signature sequence.
 */
int dsa_sig_unpack(unsigned char* sig, int sig_len, p256_int* r_int, p256_int* s_int) {
    /*
     * Structure is:
     *   0x30 0xNN  SEQUENCE + s_length
     *     0x02 0xNN  INTEGER + r_length
     *       0xAA 0xBB ..   r_length bytes of "r" (offset 4)
     *     0x02 0xNN  INTEGER + s_length
     *       0xMM 0xNN ..   s_length bytes of "s" (offset 6 + r_len)
     */
    int seq_len;
    unsigned char r_bytes[P256_NBYTES];
    unsigned char s_bytes[P256_NBYTES];
    int r_len;
    int s_len;

    memset(r_bytes, 0, sizeof(r_bytes));
    memset(s_bytes, 0, sizeof(s_bytes));

    /*
     * Must have at least:
     * 2 bytes sequence header and length
     * 2 bytes R integer header and length
     * 1 byte of R
     * 2 bytes S integer header and length
     * 1 byte of S
     *
     * 8 bytes total
     */
    if (sig_len < 8 || sig[0] != 0x30 || sig[2] != 0x02) {
        return 0;
    }

    seq_len = sig[1];
    if ((seq_len <= 0) || (seq_len + 2 != sig_len)) {
        return 0;
    }

    r_len = sig[3];
    /*
     * Must have at least:
     * 2 bytes for R header and length
     * 2 bytes S integer header and length
     * 1 byte of S
     */
    if ((r_len < 1) || (r_len > seq_len - 5) || (sig[4 + r_len] != 0x02)) {
        return 0;
    }
    s_len = sig[5 + r_len];

    /**
     * Must have:
     * 2 bytes for R header and length
     * r_len bytes for R
     * 2 bytes S integer header and length
     */
    if ((s_len < 1) || (s_len != seq_len - 4 - r_len)) {
        return 0;
    }

    /*
     * ASN.1 encoded integers are zero-padded for positive integers. Make sure we have
     * a correctly-sized buffer and that the resulting integer isn't too large.
     */
    if (!trim_to_p256_bytes(r_bytes, &sig[4], r_len)
            || !trim_to_p256_bytes(s_bytes, &sig[6 + r_len], s_len)) {
        return 0;
    }

    p256_from_bin(r_bytes, r_int);
    p256_from_bin(s_bytes, s_int);

    return 1;
}
