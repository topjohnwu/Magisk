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

// This is an implementation of the P256 elliptic curve group. It's written to
// be portable 32-bit, although it's still constant-time.
//
// WARNING: Implementing these functions in a constant-time manner is far from
//          obvious. Be careful when touching this code.
//
// See http://www.imperialviolet.org/2010/12/04/ecc.html ([1]) for background.

#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include "mincrypt/p256.h"

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;

/* Our field elements are represented as nine 32-bit limbs.
 *
 * The value of an felem (field element) is:
 *   x[0] + (x[1] * 2**29) + (x[2] * 2**57) + ... + (x[8] * 2**228)
 *
 * That is, each limb is alternately 29 or 28-bits wide in little-endian
 * order.
 *
 * This means that an felem hits 2**257, rather than 2**256 as we would like. A
 * 28, 29, ... pattern would cause us to hit 2**256, but that causes problems
 * when multiplying as terms end up one bit short of a limb which would require
 * much bit-shifting to correct.
 *
 * Finally, the values stored in an felem are in Montgomery form. So the value
 * |y| is stored as (y*R) mod p, where p is the P-256 prime and R is 2**257.
 */
typedef u32 limb;
#define NLIMBS 9
typedef limb felem[NLIMBS];

static const limb kBottom28Bits = 0xfffffff;
static const limb kBottom29Bits = 0x1fffffff;

/* kOne is the number 1 as an felem. It's 2**257 mod p split up into 29 and
 * 28-bit words. */
static const felem kOne = {
    2, 0, 0, 0xffff800,
    0x1fffffff, 0xfffffff, 0x1fbfffff, 0x1ffffff,
    0
};
static const felem kZero = {0};
static const felem kP = {
    0x1fffffff, 0xfffffff, 0x1fffffff, 0x3ff,
    0, 0, 0x200000, 0xf000000,
    0xfffffff
};
static const felem k2P = {
    0x1ffffffe, 0xfffffff, 0x1fffffff, 0x7ff,
    0, 0, 0x400000, 0xe000000,
    0x1fffffff
};
/* kPrecomputed contains precomputed values to aid the calculation of scalar
 * multiples of the base point, G. It's actually two, equal length, tables
 * concatenated.
 *
 * The first table contains (x,y) felem pairs for 16 multiples of the base
 * point, G.
 *
 *   Index  |  Index (binary) | Value
 *       0  |           0000  | 0G (all zeros, omitted)
 *       1  |           0001  | G
 *       2  |           0010  | 2**64G
 *       3  |           0011  | 2**64G + G
 *       4  |           0100  | 2**128G
 *       5  |           0101  | 2**128G + G
 *       6  |           0110  | 2**128G + 2**64G
 *       7  |           0111  | 2**128G + 2**64G + G
 *       8  |           1000  | 2**192G
 *       9  |           1001  | 2**192G + G
 *      10  |           1010  | 2**192G + 2**64G
 *      11  |           1011  | 2**192G + 2**64G + G
 *      12  |           1100  | 2**192G + 2**128G
 *      13  |           1101  | 2**192G + 2**128G + G
 *      14  |           1110  | 2**192G + 2**128G + 2**64G
 *      15  |           1111  | 2**192G + 2**128G + 2**64G + G
 *
 * The second table follows the same style, but the terms are 2**32G,
 * 2**96G, 2**160G, 2**224G.
 *
 * This is ~2KB of data. */
static const limb kPrecomputed[NLIMBS * 2 * 15 * 2] = {
    0x11522878, 0xe730d41, 0xdb60179, 0x4afe2ff, 0x12883add, 0xcaddd88, 0x119e7edc, 0xd4a6eab, 0x3120bee,
    0x1d2aac15, 0xf25357c, 0x19e45cdd, 0x5c721d0, 0x1992c5a5, 0xa237487, 0x154ba21, 0x14b10bb, 0xae3fe3,
    0xd41a576, 0x922fc51, 0x234994f, 0x60b60d3, 0x164586ae, 0xce95f18, 0x1fe49073, 0x3fa36cc, 0x5ebcd2c,
    0xb402f2f, 0x15c70bf, 0x1561925c, 0x5a26704, 0xda91e90, 0xcdc1c7f, 0x1ea12446, 0xe1ade1e, 0xec91f22,
    0x26f7778, 0x566847e, 0xa0bec9e, 0x234f453, 0x1a31f21a, 0xd85e75c, 0x56c7109, 0xa267a00, 0xb57c050,
    0x98fb57, 0xaa837cc, 0x60c0792, 0xcfa5e19, 0x61bab9e, 0x589e39b, 0xa324c5, 0x7d6dee7, 0x2976e4b,
    0x1fc4124a, 0xa8c244b, 0x1ce86762, 0xcd61c7e, 0x1831c8e0, 0x75774e1, 0x1d96a5a9, 0x843a649, 0xc3ab0fa,
    0x6e2e7d5, 0x7673a2a, 0x178b65e8, 0x4003e9b, 0x1a1f11c2, 0x7816ea, 0xf643e11, 0x58c43df, 0xf423fc2,
    0x19633ffa, 0x891f2b2, 0x123c231c, 0x46add8c, 0x54700dd, 0x59e2b17, 0x172db40f, 0x83e277d, 0xb0dd609,
    0xfd1da12, 0x35c6e52, 0x19ede20c, 0xd19e0c0, 0x97d0f40, 0xb015b19, 0x449e3f5, 0xe10c9e, 0x33ab581,
    0x56a67ab, 0x577734d, 0x1dddc062, 0xc57b10d, 0x149b39d, 0x26a9e7b, 0xc35df9f, 0x48764cd, 0x76dbcca,
    0xca4b366, 0xe9303ab, 0x1a7480e7, 0x57e9e81, 0x1e13eb50, 0xf466cf3, 0x6f16b20, 0x4ba3173, 0xc168c33,
    0x15cb5439, 0x6a38e11, 0x73658bd, 0xb29564f, 0x3f6dc5b, 0x53b97e, 0x1322c4c0, 0x65dd7ff, 0x3a1e4f6,
    0x14e614aa, 0x9246317, 0x1bc83aca, 0xad97eed, 0xd38ce4a, 0xf82b006, 0x341f077, 0xa6add89, 0x4894acd,
    0x9f162d5, 0xf8410ef, 0x1b266a56, 0xd7f223, 0x3e0cb92, 0xe39b672, 0x6a2901a, 0x69a8556, 0x7e7c0,
    0x9b7d8d3, 0x309a80, 0x1ad05f7f, 0xc2fb5dd, 0xcbfd41d, 0x9ceb638, 0x1051825c, 0xda0cf5b, 0x812e881,
    0x6f35669, 0x6a56f2c, 0x1df8d184, 0x345820, 0x1477d477, 0x1645db1, 0xbe80c51, 0xc22be3e, 0xe35e65a,
    0x1aeb7aa0, 0xc375315, 0xf67bc99, 0x7fdd7b9, 0x191fc1be, 0x61235d, 0x2c184e9, 0x1c5a839, 0x47a1e26,
    0xb7cb456, 0x93e225d, 0x14f3c6ed, 0xccc1ac9, 0x17fe37f3, 0x4988989, 0x1a90c502, 0x2f32042, 0xa17769b,
    0xafd8c7c, 0x8191c6e, 0x1dcdb237, 0x16200c0, 0x107b32a1, 0x66c08db, 0x10d06a02, 0x3fc93, 0x5620023,
    0x16722b27, 0x68b5c59, 0x270fcfc, 0xfad0ecc, 0xe5de1c2, 0xeab466b, 0x2fc513c, 0x407f75c, 0xbaab133,
    0x9705fe9, 0xb88b8e7, 0x734c993, 0x1e1ff8f, 0x19156970, 0xabd0f00, 0x10469ea7, 0x3293ac0, 0xcdc98aa,
    0x1d843fd, 0xe14bfe8, 0x15be825f, 0x8b5212, 0xeb3fb67, 0x81cbd29, 0xbc62f16, 0x2b6fcc7, 0xf5a4e29,
    0x13560b66, 0xc0b6ac2, 0x51ae690, 0xd41e271, 0xf3e9bd4, 0x1d70aab, 0x1029f72, 0x73e1c35, 0xee70fbc,
    0xad81baf, 0x9ecc49a, 0x86c741e, 0xfe6be30, 0x176752e7, 0x23d416, 0x1f83de85, 0x27de188, 0x66f70b8,
    0x181cd51f, 0x96b6e4c, 0x188f2335, 0xa5df759, 0x17a77eb6, 0xfeb0e73, 0x154ae914, 0x2f3ec51, 0x3826b59,
    0xb91f17d, 0x1c72949, 0x1362bf0a, 0xe23fddf, 0xa5614b0, 0xf7d8f, 0x79061, 0x823d9d2, 0x8213f39,
    0x1128ae0b, 0xd095d05, 0xb85c0c2, 0x1ecb2ef, 0x24ddc84, 0xe35e901, 0x18411a4a, 0xf5ddc3d, 0x3786689,
    0x52260e8, 0x5ae3564, 0x542b10d, 0x8d93a45, 0x19952aa4, 0x996cc41, 0x1051a729, 0x4be3499, 0x52b23aa,
    0x109f307e, 0x6f5b6bb, 0x1f84e1e7, 0x77a0cfa, 0x10c4df3f, 0x25a02ea, 0xb048035, 0xe31de66, 0xc6ecaa3,
    0x28ea335, 0x2886024, 0x1372f020, 0xf55d35, 0x15e4684c, 0xf2a9e17, 0x1a4a7529, 0xcb7beb1, 0xb2a78a1,
    0x1ab21f1f, 0x6361ccf, 0x6c9179d, 0xb135627, 0x1267b974, 0x4408bad, 0x1cbff658, 0xe3d6511, 0xc7d76f,
    0x1cc7a69, 0xe7ee31b, 0x54fab4f, 0x2b914f, 0x1ad27a30, 0xcd3579e, 0xc50124c, 0x50daa90, 0xb13f72,
    0xb06aa75, 0x70f5cc6, 0x1649e5aa, 0x84a5312, 0x329043c, 0x41c4011, 0x13d32411, 0xb04a838, 0xd760d2d,
    0x1713b532, 0xbaa0c03, 0x84022ab, 0x6bcf5c1, 0x2f45379, 0x18ae070, 0x18c9e11e, 0x20bca9a, 0x66f496b,
    0x3eef294, 0x67500d2, 0xd7f613c, 0x2dbbeb, 0xb741038, 0xe04133f, 0x1582968d, 0xbe985f7, 0x1acbc1a,
    0x1a6a939f, 0x33e50f6, 0xd665ed4, 0xb4b7bd6, 0x1e5a3799, 0x6b33847, 0x17fa56ff, 0x65ef930, 0x21dc4a,
    0x2b37659, 0x450fe17, 0xb357b65, 0xdf5efac, 0x15397bef, 0x9d35a7f, 0x112ac15f, 0x624e62e, 0xa90ae2f,
    0x107eecd2, 0x1f69bbe, 0x77d6bce, 0x5741394, 0x13c684fc, 0x950c910, 0x725522b, 0xdc78583, 0x40eeabb,
    0x1fde328a, 0xbd61d96, 0xd28c387, 0x9e77d89, 0x12550c40, 0x759cb7d, 0x367ef34, 0xae2a960, 0x91b8bdc,
    0x93462a9, 0xf469ef, 0xb2e9aef, 0xd2ca771, 0x54e1f42, 0x7aaa49, 0x6316abb, 0x2413c8e, 0x5425bf9,
    0x1bed3e3a, 0xf272274, 0x1f5e7326, 0x6416517, 0xea27072, 0x9cedea7, 0x6e7633, 0x7c91952, 0xd806dce,
    0x8e2a7e1, 0xe421e1a, 0x418c9e1, 0x1dbc890, 0x1b395c36, 0xa1dc175, 0x1dc4ef73, 0x8956f34, 0xe4b5cf2,
    0x1b0d3a18, 0x3194a36, 0x6c2641f, 0xe44124c, 0xa2f4eaa, 0xa8c25ba, 0xf927ed7, 0x627b614, 0x7371cca,
    0xba16694, 0x417bc03, 0x7c0a7e3, 0x9c35c19, 0x1168a205, 0x8b6b00d, 0x10e3edc9, 0x9c19bf2, 0x5882229,
    0x1b2b4162, 0xa5cef1a, 0x1543622b, 0x9bd433e, 0x364e04d, 0x7480792, 0x5c9b5b3, 0xe85ff25, 0x408ef57,
    0x1814cfa4, 0x121b41b, 0xd248a0f, 0x3b05222, 0x39bb16a, 0xc75966d, 0xa038113, 0xa4a1769, 0x11fbc6c,
    0x917e50e, 0xeec3da8, 0x169d6eac, 0x10c1699, 0xa416153, 0xf724912, 0x15cd60b7, 0x4acbad9, 0x5efc5fa,
    0xf150ed7, 0x122b51, 0x1104b40a, 0xcb7f442, 0xfbb28ff, 0x6ac53ca, 0x196142cc, 0x7bf0fa9, 0x957651,
    0x4e0f215, 0xed439f8, 0x3f46bd5, 0x5ace82f, 0x110916b6, 0x6db078, 0xffd7d57, 0xf2ecaac, 0xca86dec,
    0x15d6b2da, 0x965ecc9, 0x1c92b4c2, 0x1f3811, 0x1cb080f5, 0x2d8b804, 0x19d1c12d, 0xf20bd46, 0x1951fa7,
    0xa3656c3, 0x523a425, 0xfcd0692, 0xd44ddc8, 0x131f0f5b, 0xaf80e4a, 0xcd9fc74, 0x99bb618, 0x2db944c,
    0xa673090, 0x1c210e1, 0x178c8d23, 0x1474383, 0x10b8743d, 0x985a55b, 0x2e74779, 0x576138, 0x9587927,
    0x133130fa, 0xbe05516, 0x9f4d619, 0xbb62570, 0x99ec591, 0xd9468fe, 0x1d07782d, 0xfc72e0b, 0x701b298,
    0x1863863b, 0x85954b8, 0x121a0c36, 0x9e7fedf, 0xf64b429, 0x9b9d71e, 0x14e2f5d8, 0xf858d3a, 0x942eea8,
    0xda5b765, 0x6edafff, 0xa9d18cc, 0xc65e4ba, 0x1c747e86, 0xe4ea915, 0x1981d7a1, 0x8395659, 0x52ed4e2,
    0x87d43b7, 0x37ab11b, 0x19d292ce, 0xf8d4692, 0x18c3053f, 0x8863e13, 0x4c146c0, 0x6bdf55a, 0x4e4457d,
    0x16152289, 0xac78ec2, 0x1a59c5a2, 0x2028b97, 0x71c2d01, 0x295851f, 0x404747b, 0x878558d, 0x7d29aa4,
    0x13d8341f, 0x8daefd7, 0x139c972d, 0x6b7ea75, 0xd4a9dde, 0xff163d8, 0x81d55d7, 0xa5bef68, 0xb7b30d8,
    0xbe73d6f, 0xaa88141, 0xd976c81, 0x7e7a9cc, 0x18beb771, 0xd773cbd, 0x13f51951, 0x9d0c177, 0x1c49a78,
};


/* Field element operations: */

/* NON_ZERO_TO_ALL_ONES returns:
 *   0xffffffff for 0 < x <= 2**31
 *   0 for x == 0 or x > 2**31.
 *
 * x must be a u32 or an equivalent type such as limb. */
#define NON_ZERO_TO_ALL_ONES(x) ((((u32)(x) - 1) >> 31) - 1)

/* felem_reduce_carry adds a multiple of p in order to cancel |carry|,
 * which is a term at 2**257.
 *
 * On entry: carry < 2**3, inout[0,2,...] < 2**29, inout[1,3,...] < 2**28.
 * On exit: inout[0,2,..] < 2**30, inout[1,3,...] < 2**29. */
static void felem_reduce_carry(felem inout, limb carry) {
  const u32 carry_mask = NON_ZERO_TO_ALL_ONES(carry);

  inout[0] += carry << 1;
  inout[3] += 0x10000000 & carry_mask;
  /* carry < 2**3 thus (carry << 11) < 2**14 and we added 2**28 in the
   * previous line therefore this doesn't underflow. */
  inout[3] -= carry << 11;
  inout[4] += (0x20000000 - 1) & carry_mask;
  inout[5] += (0x10000000 - 1) & carry_mask;
  inout[6] += (0x20000000 - 1) & carry_mask;
  inout[6] -= carry << 22;
  /* This may underflow if carry is non-zero but, if so, we'll fix it in the
   * next line. */
  inout[7] -= 1 & carry_mask;
  inout[7] += carry << 25;
}

/* felem_sum sets out = in+in2.
 *
 * On entry, in[i]+in2[i] must not overflow a 32-bit word.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29 */
static void felem_sum(felem out, const felem in, const felem in2) {
  limb carry = 0;
  unsigned i;

  for (i = 0;; i++) {
    out[i] = in[i] + in2[i];
    out[i] += carry;
    carry = out[i] >> 29;
    out[i] &= kBottom29Bits;

    i++;
    if (i == NLIMBS)
      break;

    out[i] = in[i] + in2[i];
    out[i] += carry;
    carry = out[i] >> 28;
    out[i] &= kBottom28Bits;
  }

  felem_reduce_carry(out, carry);
}

#define two31m3 (((limb)1) << 31) - (((limb)1) << 3)
#define two30m2 (((limb)1) << 30) - (((limb)1) << 2)
#define two30p13m2 (((limb)1) << 30) + (((limb)1) << 13) - (((limb)1) << 2)
#define two31m2 (((limb)1) << 31) - (((limb)1) << 2)
#define two31p24m2 (((limb)1) << 31) + (((limb)1) << 24) - (((limb)1) << 2)
#define two30m27m2 (((limb)1) << 30) - (((limb)1) << 27) - (((limb)1) << 2)

/* zero31 is 0 mod p. */
static const felem zero31 = { two31m3, two30m2, two31m2, two30p13m2, two31m2, two30m2, two31p24m2, two30m27m2, two31m2 };

/* felem_diff sets out = in-in2.
 *
 * On entry: in[0,2,...] < 2**30, in[1,3,...] < 2**29 and
 *           in2[0,2,...] < 2**30, in2[1,3,...] < 2**29.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29. */
static void felem_diff(felem out, const felem in, const felem in2) {
  limb carry = 0;
  unsigned i;

   for (i = 0;; i++) {
    out[i] = in[i] - in2[i];
    out[i] += zero31[i];
    out[i] += carry;
    carry = out[i] >> 29;
    out[i] &= kBottom29Bits;

    i++;
    if (i == NLIMBS)
      break;

    out[i] = in[i] - in2[i];
    out[i] += zero31[i];
    out[i] += carry;
    carry = out[i] >> 28;
    out[i] &= kBottom28Bits;
  }

  felem_reduce_carry(out, carry);
}

/* felem_reduce_degree sets out = tmp/R mod p where tmp contains 64-bit words
 * with the same 29,28,... bit positions as an felem.
 *
 * The values in felems are in Montgomery form: x*R mod p where R = 2**257.
 * Since we just multiplied two Montgomery values together, the result is
 * x*y*R*R mod p. We wish to divide by R in order for the result also to be
 * in Montgomery form.
 *
 * On entry: tmp[i] < 2**64
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29 */
static void felem_reduce_degree(felem out, u64 tmp[17]) {
   /* The following table may be helpful when reading this code:
    *
    * Limb number:   0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10...
    * Width (bits):  29| 28| 29| 28| 29| 28| 29| 28| 29| 28| 29
    * Start bit:     0 | 29| 57| 86|114|143|171|200|228|257|285
    *   (odd phase): 0 | 28| 57| 85|114|142|171|199|228|256|285 */
  limb tmp2[18], carry, x, xMask;
  unsigned i;

  /* tmp contains 64-bit words with the same 29,28,29-bit positions as an
   * felem. So the top of an element of tmp might overlap with another
   * element two positions down. The following loop eliminates this
   * overlap. */
  tmp2[0] = (limb)(tmp[0] & kBottom29Bits);

  /* In the following we use "(limb) tmp[x]" and "(limb) (tmp[x]>>32)" to try
   * and hint to the compiler that it can do a single-word shift by selecting
   * the right register rather than doing a double-word shift and truncating
   * afterwards. */
  tmp2[1] = ((limb) tmp[0]) >> 29;
  tmp2[1] |= (((limb)(tmp[0] >> 32)) << 3) & kBottom28Bits;
  tmp2[1] += ((limb) tmp[1]) & kBottom28Bits;
  carry = tmp2[1] >> 28;
  tmp2[1] &= kBottom28Bits;

  for (i = 2; i < 17; i++) {
    tmp2[i] = ((limb)(tmp[i - 2] >> 32)) >> 25;
    tmp2[i] += ((limb)(tmp[i - 1])) >> 28;
    tmp2[i] += (((limb)(tmp[i - 1] >> 32)) << 4) & kBottom29Bits;
    tmp2[i] += ((limb) tmp[i]) & kBottom29Bits;
    tmp2[i] += carry;
    carry = tmp2[i] >> 29;
    tmp2[i] &= kBottom29Bits;

    i++;
    if (i == 17)
      break;
    tmp2[i] = ((limb)(tmp[i - 2] >> 32)) >> 25;
    tmp2[i] += ((limb)(tmp[i - 1])) >> 29;
    tmp2[i] += (((limb)(tmp[i - 1] >> 32)) << 3) & kBottom28Bits;
    tmp2[i] += ((limb) tmp[i]) & kBottom28Bits;
    tmp2[i] += carry;
    carry = tmp2[i] >> 28;
    tmp2[i] &= kBottom28Bits;
  }

  tmp2[17] = ((limb)(tmp[15] >> 32)) >> 25;
  tmp2[17] += ((limb)(tmp[16])) >> 29;
  tmp2[17] += (((limb)(tmp[16] >> 32)) << 3);
  tmp2[17] += carry;

  /* Montgomery elimination of terms.
   *
   * Since R is 2**257, we can divide by R with a bitwise shift if we can
   * ensure that the right-most 257 bits are all zero. We can make that true by
   * adding multiplies of p without affecting the value.
   *
   * So we eliminate limbs from right to left. Since the bottom 29 bits of p
   * are all ones, then by adding tmp2[0]*p to tmp2 we'll make tmp2[0] == 0.
   * We can do that for 8 further limbs and then right shift to eliminate the
   * extra factor of R. */
  for (i = 0;; i += 2) {
    tmp2[i + 1] += tmp2[i] >> 29;
    x = tmp2[i] & kBottom29Bits;
    xMask = NON_ZERO_TO_ALL_ONES(x);
    tmp2[i] = 0;

    /* The bounds calculations for this loop are tricky. Each iteration of
     * the loop eliminates two words by adding values to words to their
     * right.
     *
     * The following table contains the amounts added to each word (as an
     * offset from the value of i at the top of the loop). The amounts are
     * accounted for from the first and second half of the loop separately
     * and are written as, for example, 28 to mean a value <2**28.
     *
     * Word:                   3   4   5   6   7   8   9   10
     * Added in top half:     28  11      29  21  29  28
     *                                        28  29
     *                                            29
     * Added in bottom half:      29  10      28  21  28   28
     *                                            29
     *
     * The value that is currently offset 7 will be offset 5 for the next
     * iteration and then offset 3 for the iteration after that. Therefore
     * the total value added will be the values added at 7, 5 and 3.
     *
     * The following table accumulates these values. The sums at the bottom
     * are written as, for example, 29+28, to mean a value < 2**29+2**28.
     *
     * Word:                   3   4   5   6   7   8   9  10  11  12  13
     *                        28  11  10  29  21  29  28  28  28  28  28
     *                            29  28  11  28  29  28  29  28  29  28
     *                                    29  28  21  21  29  21  29  21
     *                                        10  29  28  21  28  21  28
     *                                        28  29  28  29  28  29  28
     *                                            11  10  29  10  29  10
     *                                            29  28  11  28  11
     *                                                    29      29
     *                        --------------------------------------------
     *                                                30+ 31+ 30+ 31+ 30+
     *                                                28+ 29+ 28+ 29+ 21+
     *                                                21+ 28+ 21+ 28+ 10
     *                                                10  21+ 10  21+
     *                                                    11      11
     *
     * So the greatest amount is added to tmp2[10] and tmp2[12]. If
     * tmp2[10/12] has an initial value of <2**29, then the maximum value
     * will be < 2**31 + 2**30 + 2**28 + 2**21 + 2**11, which is < 2**32,
     * as required. */
    tmp2[i + 3] += (x << 10) & kBottom28Bits;
    tmp2[i + 4] += (x >> 18);

    tmp2[i + 6] += (x << 21) & kBottom29Bits;
    tmp2[i + 7] += x >> 8;

    /* At position 200, which is the starting bit position for word 7, we
     * have a factor of 0xf000000 = 2**28 - 2**24. */
    tmp2[i + 7] += 0x10000000 & xMask;
    /* Word 7 is 28 bits wide, so the 2**28 term exactly hits word 8. */
    tmp2[i + 8] += (x - 1) & xMask;
    tmp2[i + 7] -= (x << 24) & kBottom28Bits;
    tmp2[i + 8] -= x >> 4;

    tmp2[i + 8] += 0x20000000 & xMask;
    tmp2[i + 8] -= x;
    tmp2[i + 8] += (x << 28) & kBottom29Bits;
    tmp2[i + 9] += ((x >> 1) - 1) & xMask;

    if (i+1 == NLIMBS)
      break;
    tmp2[i + 2] += tmp2[i + 1] >> 28;
    x = tmp2[i + 1] & kBottom28Bits;
    xMask = NON_ZERO_TO_ALL_ONES(x);
    tmp2[i + 1] = 0;

    tmp2[i + 4] += (x << 11) & kBottom29Bits;
    tmp2[i + 5] += (x >> 18);

    tmp2[i + 7] += (x << 21) & kBottom28Bits;
    tmp2[i + 8] += x >> 7;

    /* At position 199, which is the starting bit of the 8th word when
     * dealing with a context starting on an odd word, we have a factor of
     * 0x1e000000 = 2**29 - 2**25. Since we have not updated i, the 8th
     * word from i+1 is i+8. */
    tmp2[i + 8] += 0x20000000 & xMask;
    tmp2[i + 9] += (x - 1) & xMask;
    tmp2[i + 8] -= (x << 25) & kBottom29Bits;
    tmp2[i + 9] -= x >> 4;

    tmp2[i + 9] += 0x10000000 & xMask;
    tmp2[i + 9] -= x;
    tmp2[i + 10] += (x - 1) & xMask;
  }

  /* We merge the right shift with a carry chain. The words above 2**257 have
   * widths of 28,29,... which we need to correct when copying them down.  */
  carry = 0;
  for (i = 0; i < 8; i++) {
    /* The maximum value of tmp2[i + 9] occurs on the first iteration and
     * is < 2**30+2**29+2**28. Adding 2**29 (from tmp2[i + 10]) is
     * therefore safe. */
    out[i] = tmp2[i + 9];
    out[i] += carry;
    out[i] += (tmp2[i + 10] << 28) & kBottom29Bits;
    carry = out[i] >> 29;
    out[i] &= kBottom29Bits;

    i++;
    out[i] = tmp2[i + 9] >> 1;
    out[i] += carry;
    carry = out[i] >> 28;
    out[i] &= kBottom28Bits;
  }

  out[8] = tmp2[17];
  out[8] += carry;
  carry = out[8] >> 29;
  out[8] &= kBottom29Bits;

  felem_reduce_carry(out, carry);
}

/* felem_square sets out=in*in.
 *
 * On entry: in[0,2,...] < 2**30, in[1,3,...] < 2**29.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29. */
static void felem_square(felem out, const felem in) {
  u64 tmp[17];

  tmp[0] = ((u64) in[0]) * in[0];
  tmp[1] = ((u64) in[0]) * (in[1] << 1);
  tmp[2] = ((u64) in[0]) * (in[2] << 1) +
           ((u64) in[1]) * (in[1] << 1);
  tmp[3] = ((u64) in[0]) * (in[3] << 1) +
           ((u64) in[1]) * (in[2] << 1);
  tmp[4] = ((u64) in[0]) * (in[4] << 1) +
           ((u64) in[1]) * (in[3] << 2) + ((u64) in[2]) * in[2];
  tmp[5] = ((u64) in[0]) * (in[5] << 1) + ((u64) in[1]) *
           (in[4] << 1) + ((u64) in[2]) * (in[3] << 1);
  tmp[6] = ((u64) in[0]) * (in[6] << 1) + ((u64) in[1]) *
           (in[5] << 2) + ((u64) in[2]) * (in[4] << 1) +
           ((u64) in[3]) * (in[3] << 1);
  tmp[7] = ((u64) in[0]) * (in[7] << 1) + ((u64) in[1]) *
           (in[6] << 1) + ((u64) in[2]) * (in[5] << 1) +
           ((u64) in[3]) * (in[4] << 1);
  /* tmp[8] has the greatest value of 2**61 + 2**60 + 2**61 + 2**60 + 2**60,
   * which is < 2**64 as required. */
  tmp[8] = ((u64) in[0]) * (in[8] << 1) + ((u64) in[1]) *
           (in[7] << 2) + ((u64) in[2]) * (in[6] << 1) +
           ((u64) in[3]) * (in[5] << 2) + ((u64) in[4]) * in[4];
  tmp[9] = ((u64) in[1]) * (in[8] << 1) + ((u64) in[2]) *
           (in[7] << 1) + ((u64) in[3]) * (in[6] << 1) +
           ((u64) in[4]) * (in[5] << 1);
  tmp[10] = ((u64) in[2]) * (in[8] << 1) + ((u64) in[3]) *
            (in[7] << 2) + ((u64) in[4]) * (in[6] << 1) +
            ((u64) in[5]) * (in[5] << 1);
  tmp[11] = ((u64) in[3]) * (in[8] << 1) + ((u64) in[4]) *
            (in[7] << 1) + ((u64) in[5]) * (in[6] << 1);
  tmp[12] = ((u64) in[4]) * (in[8] << 1) +
            ((u64) in[5]) * (in[7] << 2) + ((u64) in[6]) * in[6];
  tmp[13] = ((u64) in[5]) * (in[8] << 1) +
            ((u64) in[6]) * (in[7] << 1);
  tmp[14] = ((u64) in[6]) * (in[8] << 1) +
            ((u64) in[7]) * (in[7] << 1);
  tmp[15] = ((u64) in[7]) * (in[8] << 1);
  tmp[16] = ((u64) in[8]) * in[8];

  felem_reduce_degree(out, tmp);
}

/* felem_mul sets out=in*in2.
 *
 * On entry: in[0,2,...] < 2**30, in[1,3,...] < 2**29 and
 *           in2[0,2,...] < 2**30, in2[1,3,...] < 2**29.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29. */
static void felem_mul(felem out, const felem in, const felem in2) {
  u64 tmp[17];

  tmp[0] = ((u64) in[0]) * in2[0];
  tmp[1] = ((u64) in[0]) * (in2[1] << 0) +
           ((u64) in[1]) * (in2[0] << 0);
  tmp[2] = ((u64) in[0]) * (in2[2] << 0) + ((u64) in[1]) *
           (in2[1] << 1) + ((u64) in[2]) * (in2[0] << 0);
  tmp[3] = ((u64) in[0]) * (in2[3] << 0) + ((u64) in[1]) *
           (in2[2] << 0) + ((u64) in[2]) * (in2[1] << 0) +
           ((u64) in[3]) * (in2[0] << 0);
  tmp[4] = ((u64) in[0]) * (in2[4] << 0) + ((u64) in[1]) *
           (in2[3] << 1) + ((u64) in[2]) * (in2[2] << 0) +
           ((u64) in[3]) * (in2[1] << 1) +
           ((u64) in[4]) * (in2[0] << 0);
  tmp[5] = ((u64) in[0]) * (in2[5] << 0) + ((u64) in[1]) *
           (in2[4] << 0) + ((u64) in[2]) * (in2[3] << 0) +
           ((u64) in[3]) * (in2[2] << 0) + ((u64) in[4]) *
           (in2[1] << 0) + ((u64) in[5]) * (in2[0] << 0);
  tmp[6] = ((u64) in[0]) * (in2[6] << 0) + ((u64) in[1]) *
           (in2[5] << 1) + ((u64) in[2]) * (in2[4] << 0) +
           ((u64) in[3]) * (in2[3] << 1) + ((u64) in[4]) *
           (in2[2] << 0) + ((u64) in[5]) * (in2[1] << 1) +
           ((u64) in[6]) * (in2[0] << 0);
  tmp[7] = ((u64) in[0]) * (in2[7] << 0) + ((u64) in[1]) *
           (in2[6] << 0) + ((u64) in[2]) * (in2[5] << 0) +
           ((u64) in[3]) * (in2[4] << 0) + ((u64) in[4]) *
           (in2[3] << 0) + ((u64) in[5]) * (in2[2] << 0) +
           ((u64) in[6]) * (in2[1] << 0) +
           ((u64) in[7]) * (in2[0] << 0);
  /* tmp[8] has the greatest value but doesn't overflow. See logic in
   * felem_square. */
  tmp[8] = ((u64) in[0]) * (in2[8] << 0) + ((u64) in[1]) *
           (in2[7] << 1) + ((u64) in[2]) * (in2[6] << 0) +
           ((u64) in[3]) * (in2[5] << 1) + ((u64) in[4]) *
           (in2[4] << 0) + ((u64) in[5]) * (in2[3] << 1) +
           ((u64) in[6]) * (in2[2] << 0) + ((u64) in[7]) *
           (in2[1] << 1) + ((u64) in[8]) * (in2[0] << 0);
  tmp[9] = ((u64) in[1]) * (in2[8] << 0) + ((u64) in[2]) *
           (in2[7] << 0) + ((u64) in[3]) * (in2[6] << 0) +
           ((u64) in[4]) * (in2[5] << 0) + ((u64) in[5]) *
           (in2[4] << 0) + ((u64) in[6]) * (in2[3] << 0) +
           ((u64) in[7]) * (in2[2] << 0) +
           ((u64) in[8]) * (in2[1] << 0);
  tmp[10] = ((u64) in[2]) * (in2[8] << 0) + ((u64) in[3]) *
            (in2[7] << 1) + ((u64) in[4]) * (in2[6] << 0) +
            ((u64) in[5]) * (in2[5] << 1) + ((u64) in[6]) *
            (in2[4] << 0) + ((u64) in[7]) * (in2[3] << 1) +
            ((u64) in[8]) * (in2[2] << 0);
  tmp[11] = ((u64) in[3]) * (in2[8] << 0) + ((u64) in[4]) *
            (in2[7] << 0) + ((u64) in[5]) * (in2[6] << 0) +
            ((u64) in[6]) * (in2[5] << 0) + ((u64) in[7]) *
            (in2[4] << 0) + ((u64) in[8]) * (in2[3] << 0);
  tmp[12] = ((u64) in[4]) * (in2[8] << 0) + ((u64) in[5]) *
            (in2[7] << 1) + ((u64) in[6]) * (in2[6] << 0) +
            ((u64) in[7]) * (in2[5] << 1) +
            ((u64) in[8]) * (in2[4] << 0);
  tmp[13] = ((u64) in[5]) * (in2[8] << 0) + ((u64) in[6]) *
            (in2[7] << 0) + ((u64) in[7]) * (in2[6] << 0) +
            ((u64) in[8]) * (in2[5] << 0);
  tmp[14] = ((u64) in[6]) * (in2[8] << 0) + ((u64) in[7]) *
            (in2[7] << 1) + ((u64) in[8]) * (in2[6] << 0);
  tmp[15] = ((u64) in[7]) * (in2[8] << 0) +
            ((u64) in[8]) * (in2[7] << 0);
  tmp[16] = ((u64) in[8]) * (in2[8] << 0);

  felem_reduce_degree(out, tmp);
}

static void felem_assign(felem out, const felem in) {
  memcpy(out, in, sizeof(felem));
}

/* felem_inv calculates |out| = |in|^{-1}
 *
 * Based on Fermat's Little Theorem:
 *   a^p = a (mod p)
 *   a^{p-1} = 1 (mod p)
 *   a^{p-2} = a^{-1} (mod p)
 */
static void felem_inv(felem out, const felem in) {
  felem ftmp, ftmp2;
  /* each e_I will hold |in|^{2^I - 1} */
  felem e2, e4, e8, e16, e32, e64;
  unsigned i;

  felem_square(ftmp, in); /* 2^1 */
  felem_mul(ftmp, in, ftmp); /* 2^2 - 2^0 */
  felem_assign(e2, ftmp);
  felem_square(ftmp, ftmp); /* 2^3 - 2^1 */
  felem_square(ftmp, ftmp); /* 2^4 - 2^2 */
  felem_mul(ftmp, ftmp, e2); /* 2^4 - 2^0 */
  felem_assign(e4, ftmp);
  felem_square(ftmp, ftmp); /* 2^5 - 2^1 */
  felem_square(ftmp, ftmp); /* 2^6 - 2^2 */
  felem_square(ftmp, ftmp); /* 2^7 - 2^3 */
  felem_square(ftmp, ftmp); /* 2^8 - 2^4 */
  felem_mul(ftmp, ftmp, e4); /* 2^8 - 2^0 */
  felem_assign(e8, ftmp);
  for (i = 0; i < 8; i++) {
    felem_square(ftmp, ftmp);
  } /* 2^16 - 2^8 */
  felem_mul(ftmp, ftmp, e8); /* 2^16 - 2^0 */
  felem_assign(e16, ftmp);
  for (i = 0; i < 16; i++) {
    felem_square(ftmp, ftmp);
  } /* 2^32 - 2^16 */
  felem_mul(ftmp, ftmp, e16); /* 2^32 - 2^0 */
  felem_assign(e32, ftmp);
  for (i = 0; i < 32; i++) {
    felem_square(ftmp, ftmp);
  } /* 2^64 - 2^32 */
  felem_assign(e64, ftmp);
  felem_mul(ftmp, ftmp, in); /* 2^64 - 2^32 + 2^0 */
  for (i = 0; i < 192; i++) {
    felem_square(ftmp, ftmp);
  } /* 2^256 - 2^224 + 2^192 */

  felem_mul(ftmp2, e64, e32); /* 2^64 - 2^0 */
  for (i = 0; i < 16; i++) {
    felem_square(ftmp2, ftmp2);
  } /* 2^80 - 2^16 */
  felem_mul(ftmp2, ftmp2, e16); /* 2^80 - 2^0 */
  for (i = 0; i < 8; i++) {
    felem_square(ftmp2, ftmp2);
  } /* 2^88 - 2^8 */
  felem_mul(ftmp2, ftmp2, e8); /* 2^88 - 2^0 */
  for (i = 0; i < 4; i++) {
    felem_square(ftmp2, ftmp2);
  } /* 2^92 - 2^4 */
  felem_mul(ftmp2, ftmp2, e4); /* 2^92 - 2^0 */
  felem_square(ftmp2, ftmp2); /* 2^93 - 2^1 */
  felem_square(ftmp2, ftmp2); /* 2^94 - 2^2 */
  felem_mul(ftmp2, ftmp2, e2); /* 2^94 - 2^0 */
  felem_square(ftmp2, ftmp2); /* 2^95 - 2^1 */
  felem_square(ftmp2, ftmp2); /* 2^96 - 2^2 */
  felem_mul(ftmp2, ftmp2, in); /* 2^96 - 3 */

  felem_mul(out, ftmp2, ftmp); /* 2^256 - 2^224 + 2^192 + 2^96 - 3 */
}

/* felem_scalar_3 sets out=3*out.
 *
 * On entry: out[0,2,...] < 2**30, out[1,3,...] < 2**29.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29. */
static void felem_scalar_3(felem out) {
  limb carry = 0;
  unsigned i;

  for (i = 0;; i++) {
    out[i] *= 3;
    out[i] += carry;
    carry = out[i] >> 29;
    out[i] &= kBottom29Bits;

    i++;
    if (i == NLIMBS)
      break;

    out[i] *= 3;
    out[i] += carry;
    carry = out[i] >> 28;
    out[i] &= kBottom28Bits;
  }

  felem_reduce_carry(out, carry);
}

/* felem_scalar_4 sets out=4*out.
 *
 * On entry: out[0,2,...] < 2**30, out[1,3,...] < 2**29.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29. */
static void felem_scalar_4(felem out) {
  limb carry = 0, next_carry;
  unsigned i;

  for (i = 0;; i++) {
    next_carry = out[i] >> 27;
    out[i] <<= 2;
    out[i] &= kBottom29Bits;
    out[i] += carry;
    carry = next_carry + (out[i] >> 29);
    out[i] &= kBottom29Bits;

    i++;
    if (i == NLIMBS)
      break;

    next_carry = out[i] >> 26;
    out[i] <<= 2;
    out[i] &= kBottom28Bits;
    out[i] += carry;
    carry = next_carry + (out[i] >> 28);
    out[i] &= kBottom28Bits;
  }

  felem_reduce_carry(out, carry);
}

/* felem_scalar_8 sets out=8*out.
 *
 * On entry: out[0,2,...] < 2**30, out[1,3,...] < 2**29.
 * On exit: out[0,2,...] < 2**30, out[1,3,...] < 2**29. */
static void felem_scalar_8(felem out) {
  limb carry = 0, next_carry;
  unsigned i;

  for (i = 0;; i++) {
    next_carry = out[i] >> 26;
    out[i] <<= 3;
    out[i] &= kBottom29Bits;
    out[i] += carry;
    carry = next_carry + (out[i] >> 29);
    out[i] &= kBottom29Bits;

    i++;
    if (i == NLIMBS)
      break;

    next_carry = out[i] >> 25;
    out[i] <<= 3;
    out[i] &= kBottom28Bits;
    out[i] += carry;
    carry = next_carry + (out[i] >> 28);
    out[i] &= kBottom28Bits;
  }

  felem_reduce_carry(out, carry);
}

/* felem_is_zero_vartime returns 1 iff |in| == 0. It takes a variable amount of
 * time depending on the value of |in|. */
static char felem_is_zero_vartime(const felem in) {
  limb carry;
  int i;
  limb tmp[NLIMBS];

  felem_assign(tmp, in);

  /* First, reduce tmp to a minimal form. */
  do {
    carry = 0;
    for (i = 0;; i++) {
      tmp[i] += carry;
      carry = tmp[i] >> 29;
      tmp[i] &= kBottom29Bits;

      i++;
      if (i == NLIMBS)
        break;

      tmp[i] += carry;
      carry = tmp[i] >> 28;
      tmp[i] &= kBottom28Bits;
    }

    felem_reduce_carry(tmp, carry);
  } while (carry);

  /* tmp < 2**257, so the only possible zero values are 0, p and 2p. */
  return memcmp(tmp, kZero, sizeof(tmp)) == 0 ||
         memcmp(tmp, kP, sizeof(tmp)) == 0 ||
         memcmp(tmp, k2P, sizeof(tmp)) == 0;
}


/* Group operations:
 *
 * Elements of the elliptic curve group are represented in Jacobian
 * coordinates: (x, y, z). An affine point (x', y') is x'=x/z**2, y'=y/z**3 in
 * Jacobian form. */

/* point_double sets {x_out,y_out,z_out} = 2*{x,y,z}.
 *
 * See http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#doubling-dbl-2009-l */
static void point_double(felem x_out, felem y_out, felem z_out, const felem x,
                         const felem y, const felem z) {
  felem delta, gamma, alpha, beta, tmp, tmp2;

  felem_square(delta, z);
  felem_square(gamma, y);
  felem_mul(beta, x, gamma);

  felem_sum(tmp, x, delta);
  felem_diff(tmp2, x, delta);
  felem_mul(alpha, tmp, tmp2);
  felem_scalar_3(alpha);

  felem_sum(tmp, y, z);
  felem_square(tmp, tmp);
  felem_diff(tmp, tmp, gamma);
  felem_diff(z_out, tmp, delta);

  felem_scalar_4(beta);
  felem_square(x_out, alpha);
  felem_diff(x_out, x_out, beta);
  felem_diff(x_out, x_out, beta);

  felem_diff(tmp, beta, x_out);
  felem_mul(tmp, alpha, tmp);
  felem_square(tmp2, gamma);
  felem_scalar_8(tmp2);
  felem_diff(y_out, tmp, tmp2);
}

/* point_add_mixed sets {x_out,y_out,z_out} = {x1,y1,z1} + {x2,y2,1}.
 * (i.e. the second point is affine.)
 *
 * See http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#addition-add-2007-bl
 *
 * Note that this function does not handle P+P, infinity+P nor P+infinity
 * correctly. */
static void point_add_mixed(felem x_out, felem y_out, felem z_out,
                            const felem x1, const felem y1, const felem z1,
                            const felem x2, const felem y2) {
  felem z1z1, z1z1z1, s2, u2, h, i, j, r, rr, v, tmp;

  felem_square(z1z1, z1);
  felem_sum(tmp, z1, z1);

  felem_mul(u2, x2, z1z1);
  felem_mul(z1z1z1, z1, z1z1);
  felem_mul(s2, y2, z1z1z1);
  felem_diff(h, u2, x1);
  felem_sum(i, h, h);
  felem_square(i, i);
  felem_mul(j, h, i);
  felem_diff(r, s2, y1);
  felem_sum(r, r, r);
  felem_mul(v, x1, i);

  felem_mul(z_out, tmp, h);
  felem_square(rr, r);
  felem_diff(x_out, rr, j);
  felem_diff(x_out, x_out, v);
  felem_diff(x_out, x_out, v);

  felem_diff(tmp, v, x_out);
  felem_mul(y_out, tmp, r);
  felem_mul(tmp, y1, j);
  felem_diff(y_out, y_out, tmp);
  felem_diff(y_out, y_out, tmp);
}

/* point_add sets {x_out,y_out,z_out} = {x1,y1,z1} + {x2,y2,z2}.
 *
 * See http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#addition-add-2007-bl
 *
 * Note that this function does not handle P+P, infinity+P nor P+infinity
 * correctly. */
static void point_add(felem x_out, felem y_out, felem z_out, const felem x1,
                      const felem y1, const felem z1, const felem x2,
                      const felem y2, const felem z2) {
  felem z1z1, z1z1z1, z2z2, z2z2z2, s1, s2, u1, u2, h, i, j, r, rr, v, tmp;

  felem_square(z1z1, z1);
  felem_square(z2z2, z2);
  felem_mul(u1, x1, z2z2);

  felem_sum(tmp, z1, z2);
  felem_square(tmp, tmp);
  felem_diff(tmp, tmp, z1z1);
  felem_diff(tmp, tmp, z2z2);

  felem_mul(z2z2z2, z2, z2z2);
  felem_mul(s1, y1, z2z2z2);

  felem_mul(u2, x2, z1z1);
  felem_mul(z1z1z1, z1, z1z1);
  felem_mul(s2, y2, z1z1z1);
  felem_diff(h, u2, u1);
  felem_sum(i, h, h);
  felem_square(i, i);
  felem_mul(j, h, i);
  felem_diff(r, s2, s1);
  felem_sum(r, r, r);
  felem_mul(v, u1, i);

  felem_mul(z_out, tmp, h);
  felem_square(rr, r);
  felem_diff(x_out, rr, j);
  felem_diff(x_out, x_out, v);
  felem_diff(x_out, x_out, v);

  felem_diff(tmp, v, x_out);
  felem_mul(y_out, tmp, r);
  felem_mul(tmp, s1, j);
  felem_diff(y_out, y_out, tmp);
  felem_diff(y_out, y_out, tmp);
}

/* point_add_or_double_vartime sets {x_out,y_out,z_out} = {x1,y1,z1} +
 *                                                        {x2,y2,z2}.
 *
 * See http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#addition-add-2007-bl
 *
 * This function handles the case where {x1,y1,z1}={x2,y2,z2}. */
static void point_add_or_double_vartime(
    felem x_out, felem y_out, felem z_out, const felem x1, const felem y1,
    const felem z1, const felem x2, const felem y2, const felem z2) {
  felem z1z1, z1z1z1, z2z2, z2z2z2, s1, s2, u1, u2, h, i, j, r, rr, v, tmp;
  char x_equal, y_equal;

  felem_square(z1z1, z1);
  felem_square(z2z2, z2);
  felem_mul(u1, x1, z2z2);

  felem_sum(tmp, z1, z2);
  felem_square(tmp, tmp);
  felem_diff(tmp, tmp, z1z1);
  felem_diff(tmp, tmp, z2z2);

  felem_mul(z2z2z2, z2, z2z2);
  felem_mul(s1, y1, z2z2z2);

  felem_mul(u2, x2, z1z1);
  felem_mul(z1z1z1, z1, z1z1);
  felem_mul(s2, y2, z1z1z1);
  felem_diff(h, u2, u1);
  x_equal = felem_is_zero_vartime(h);
  felem_sum(i, h, h);
  felem_square(i, i);
  felem_mul(j, h, i);
  felem_diff(r, s2, s1);
  y_equal = felem_is_zero_vartime(r);
  if (x_equal && y_equal) {
    point_double(x_out, y_out, z_out, x1, y1, z1);
    return;
  }
  felem_sum(r, r, r);
  felem_mul(v, u1, i);

  felem_mul(z_out, tmp, h);
  felem_square(rr, r);
  felem_diff(x_out, rr, j);
  felem_diff(x_out, x_out, v);
  felem_diff(x_out, x_out, v);

  felem_diff(tmp, v, x_out);
  felem_mul(y_out, tmp, r);
  felem_mul(tmp, s1, j);
  felem_diff(y_out, y_out, tmp);
  felem_diff(y_out, y_out, tmp);
}

/* copy_conditional sets out=in if mask = 0xffffffff in constant time.
 *
 * On entry: mask is either 0 or 0xffffffff. */
static void copy_conditional(felem out, const felem in, limb mask) {
  int i;

  for (i = 0; i < NLIMBS; i++) {
    const limb tmp = mask & (in[i] ^ out[i]);
    out[i] ^= tmp;
  }
}

/* select_affine_point sets {out_x,out_y} to the index'th entry of table.
 * On entry: index < 16, table[0] must be zero. */
static void select_affine_point(felem out_x, felem out_y, const limb* table,
                                limb index) {
  limb i, j;

  memset(out_x, 0, sizeof(felem));
  memset(out_y, 0, sizeof(felem));

  for (i = 1; i < 16; i++) {
    limb mask = i ^ index;
    mask |= mask >> 2;
    mask |= mask >> 1;
    mask &= 1;
    mask--;
    for (j = 0; j < NLIMBS; j++, table++) {
      out_x[j] |= *table & mask;
    }
    for (j = 0; j < NLIMBS; j++, table++) {
      out_y[j] |= *table & mask;
    }
  }
}

/* select_jacobian_point sets {out_x,out_y,out_z} to the index'th entry of
 * table. On entry: index < 16, table[0] must be zero. */
static void select_jacobian_point(felem out_x, felem out_y, felem out_z,
                                  const limb* table, limb index) {
  limb i, j;

  memset(out_x, 0, sizeof(felem));
  memset(out_y, 0, sizeof(felem));
  memset(out_z, 0, sizeof(felem));

  /* The implicit value at index 0 is all zero. We don't need to perform that
   * iteration of the loop because we already set out_* to zero. */
  table += 3 * NLIMBS;

  // Hit all entries to obscure cache profiling.
  for (i = 1; i < 16; i++) {
    limb mask = i ^ index;
    mask |= mask >> 2;
    mask |= mask >> 1;
    mask &= 1;
    mask--;
    for (j = 0; j < NLIMBS; j++, table++) {
      out_x[j] |= *table & mask;
    }
    for (j = 0; j < NLIMBS; j++, table++) {
      out_y[j] |= *table & mask;
    }
    for (j = 0; j < NLIMBS; j++, table++) {
      out_z[j] |= *table & mask;
    }
  }
}

/* scalar_base_mult sets {nx,ny,nz} = scalar*G where scalar is a little-endian
 * number. Note that the value of scalar must be less than the order of the
 * group. */
static void scalar_base_mult(felem nx, felem ny, felem nz,
                             const p256_int* scalar) {
  int i, j;
  limb n_is_infinity_mask = -1, p_is_noninfinite_mask, mask;
  u32 table_offset;

  felem px, py;
  felem tx, ty, tz;

  memset(nx, 0, sizeof(felem));
  memset(ny, 0, sizeof(felem));
  memset(nz, 0, sizeof(felem));

  /* The loop adds bits at positions 0, 64, 128 and 192, followed by
   * positions 32,96,160 and 224 and does this 32 times. */
  for (i = 0; i < 32; i++) {
    if (i) {
      point_double(nx, ny, nz, nx, ny, nz);
    }
    table_offset = 0;
    for (j = 0; j <= 32; j += 32) {
      char bit0 = p256_get_bit(scalar, 31 - i + j);
      char bit1 = p256_get_bit(scalar, 95 - i + j);
      char bit2 = p256_get_bit(scalar, 159 - i + j);
      char bit3 = p256_get_bit(scalar, 223 - i + j);
      limb index = bit0 | (bit1 << 1) | (bit2 << 2) | (bit3 << 3);

      select_affine_point(px, py, kPrecomputed + table_offset, index);
      table_offset += 30 * NLIMBS;

      /* Since scalar is less than the order of the group, we know that
       * {nx,ny,nz} != {px,py,1}, unless both are zero, which we handle
       * below. */
      point_add_mixed(tx, ty, tz, nx, ny, nz, px, py);
      /* The result of point_add_mixed is incorrect if {nx,ny,nz} is zero
       * (a.k.a.  the point at infinity). We handle that situation by
       * copying the point from the table. */
      copy_conditional(nx, px, n_is_infinity_mask);
      copy_conditional(ny, py, n_is_infinity_mask);
      copy_conditional(nz, kOne, n_is_infinity_mask);

      /* Equally, the result is also wrong if the point from the table is
       * zero, which happens when the index is zero. We handle that by
       * only copying from {tx,ty,tz} to {nx,ny,nz} if index != 0. */
      p_is_noninfinite_mask = NON_ZERO_TO_ALL_ONES(index);
      mask = p_is_noninfinite_mask & ~n_is_infinity_mask;
      copy_conditional(nx, tx, mask);
      copy_conditional(ny, ty, mask);
      copy_conditional(nz, tz, mask);
      /* If p was not zero, then n is now non-zero. */
      n_is_infinity_mask &= ~p_is_noninfinite_mask;
    }
  }
}

/* point_to_affine converts a Jacobian point to an affine point. If the input
 * is the point at infinity then it returns (0, 0) in constant time. */
static void point_to_affine(felem x_out, felem y_out, const felem nx,
                            const felem ny, const felem nz) {
  felem z_inv, z_inv_sq;
  felem_inv(z_inv, nz);
  felem_square(z_inv_sq, z_inv);
  felem_mul(x_out, nx, z_inv_sq);
  felem_mul(z_inv, z_inv, z_inv_sq);
  felem_mul(y_out, ny, z_inv);
}

/* scalar_base_mult sets {nx,ny,nz} = scalar*{x,y}. */
static void scalar_mult(felem nx, felem ny, felem nz, const felem x,
                        const felem y, const p256_int* scalar) {
  int i;
  felem px, py, pz, tx, ty, tz;
  felem precomp[16][3];
  limb n_is_infinity_mask, index, p_is_noninfinite_mask, mask;

  /* We precompute 0,1,2,... times {x,y}. */
  memset(precomp, 0, sizeof(felem) * 3);
  memcpy(&precomp[1][0], x, sizeof(felem));
  memcpy(&precomp[1][1], y, sizeof(felem));
  memcpy(&precomp[1][2], kOne, sizeof(felem));

  for (i = 2; i < 16; i += 2) {
    point_double(precomp[i][0], precomp[i][1], precomp[i][2],
                 precomp[i / 2][0], precomp[i / 2][1], precomp[i / 2][2]);

    point_add_mixed(precomp[i + 1][0], precomp[i + 1][1], precomp[i + 1][2],
                    precomp[i][0], precomp[i][1], precomp[i][2], x, y);
  }

  memset(nx, 0, sizeof(felem));
  memset(ny, 0, sizeof(felem));
  memset(nz, 0, sizeof(felem));
  n_is_infinity_mask = -1;

  /* We add in a window of four bits each iteration and do this 64 times. */
  for (i = 0; i < 256; i += 4) {
    if (i) {
      point_double(nx, ny, nz, nx, ny, nz);
      point_double(nx, ny, nz, nx, ny, nz);
      point_double(nx, ny, nz, nx, ny, nz);
      point_double(nx, ny, nz, nx, ny, nz);
    }

    index = (p256_get_bit(scalar, 255 - i - 0) << 3) |
            (p256_get_bit(scalar, 255 - i - 1) << 2) |
            (p256_get_bit(scalar, 255 - i - 2) << 1) |
            p256_get_bit(scalar, 255 - i - 3);

    /* See the comments in scalar_base_mult about handling infinities. */
    select_jacobian_point(px, py, pz, precomp[0][0], index);
    point_add(tx, ty, tz, nx, ny, nz, px, py, pz);
    copy_conditional(nx, px, n_is_infinity_mask);
    copy_conditional(ny, py, n_is_infinity_mask);
    copy_conditional(nz, pz, n_is_infinity_mask);

    p_is_noninfinite_mask = NON_ZERO_TO_ALL_ONES(index);
    mask = p_is_noninfinite_mask & ~n_is_infinity_mask;

    copy_conditional(nx, tx, mask);
    copy_conditional(ny, ty, mask);
    copy_conditional(nz, tz, mask);
    n_is_infinity_mask &= ~p_is_noninfinite_mask;
  }
}

#define kRDigits {2, 0, 0, 0xfffffffe, 0xffffffff, 0xffffffff, 0xfffffffd, 1} // 2^257 mod p256.p

#define kRInvDigits {0x80000000, 1, 0xffffffff, 0, 0x80000001, 0xfffffffe, 1, 0x7fffffff}  // 1 / 2^257 mod p256.p

static const p256_int kR = { kRDigits };
static const p256_int kRInv = { kRInvDigits };

/* to_montgomery sets out = R*in. */
static void to_montgomery(felem out, const p256_int* in) {
  p256_int in_shifted;
  int i;

  p256_init(&in_shifted);
  p256_modmul(&SECP256r1_p, in, 0, &kR, &in_shifted);

  for (i = 0; i < NLIMBS; i++) {
    if ((i & 1) == 0) {
      out[i] = P256_DIGIT(&in_shifted, 0) & kBottom29Bits;
      p256_shr(&in_shifted, 29, &in_shifted);
    } else {
      out[i] = P256_DIGIT(&in_shifted, 0) & kBottom28Bits;
      p256_shr(&in_shifted, 28, &in_shifted);
    }
  }

  p256_clear(&in_shifted);
}

/* from_montgomery sets out=in/R. */
static void from_montgomery(p256_int* out, const felem in) {
  p256_int result, tmp;
  int i, top;

  p256_init(&result);
  p256_init(&tmp);

  p256_add_d(&tmp, in[NLIMBS - 1], &result);
  for (i = NLIMBS - 2; i >= 0; i--) {
    if ((i & 1) == 0) {
      top = p256_shl(&result, 29, &tmp);
    } else {
      top = p256_shl(&result, 28, &tmp);
    }
    top |= p256_add_d(&tmp, in[i], &result);
  }

  p256_modmul(&SECP256r1_p, &kRInv, top, &result, out);

  p256_clear(&result);
  p256_clear(&tmp);
}

/* p256_base_point_mul sets {out_x,out_y} = nG, where n is < the
 * order of the group. */
void p256_base_point_mul(const p256_int* n, p256_int* out_x, p256_int* out_y) {
  felem x, y, z;

  scalar_base_mult(x, y, z, n);

  {
    felem x_affine, y_affine;

    point_to_affine(x_affine, y_affine, x, y, z);
    from_montgomery(out_x, x_affine);
    from_montgomery(out_y, y_affine);
  }
}

/* p256_points_mul_vartime sets {out_x,out_y} = n1*G + n2*{in_x,in_y}, where
 * n1 and n2 are < the order of the group.
 *
 * As indicated by the name, this function operates in variable time. This
 * is safe because it's used for signature validation which doesn't deal
 * with secrets. */
void p256_points_mul_vartime(
    const p256_int* n1, const p256_int* n2, const p256_int* in_x,
    const p256_int* in_y, p256_int* out_x, p256_int* out_y) {
  felem x1, y1, z1, x2, y2, z2, px, py;

  /* If both scalars are zero, then the result is the point at infinity. */
  if (p256_is_zero(n1) != 0 && p256_is_zero(n2) != 0) {
    p256_clear(out_x);
    p256_clear(out_y);
    return;
  }

  to_montgomery(px, in_x);
  to_montgomery(py, in_y);
  scalar_base_mult(x1, y1, z1, n1);
  scalar_mult(x2, y2, z2, px, py, n2);

  if (p256_is_zero(n2) != 0) {
    /* If n2 == 0, then {x2,y2,z2} is zero and the result is just
         * {x1,y1,z1}. */
  } else if (p256_is_zero(n1) != 0) {
    /* If n1 == 0, then {x1,y1,z1} is zero and the result is just
         * {x2,y2,z2}. */
    memcpy(x1, x2, sizeof(x2));
    memcpy(y1, y2, sizeof(y2));
    memcpy(z1, z2, sizeof(z2));
  } else {
    /* This function handles the case where {x1,y1,z1} == {x2,y2,z2}. */
    point_add_or_double_vartime(x1, y1, z1, x1, y1, z1, x2, y2, z2);
  }

  point_to_affine(px, py, x1, y1, z1);
  from_montgomery(out_x, px);
  from_montgomery(out_y, py);
}
