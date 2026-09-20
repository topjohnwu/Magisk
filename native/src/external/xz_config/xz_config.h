/* SPDX-License-Identifier: 0BSD */

/*
 * Private includes and definitions for userspace use of XZ Embedded
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 */

#ifndef XZ_CONFIG_H
#define XZ_CONFIG_H

/* Uncomment to enable building of xz_dec_catrun(). */
/* #define XZ_DEC_CONCATENATED */

/* Uncomment to enable CRC64 support. */
/* #define XZ_USE_CRC64 */

/* Enable BCJ filter decoders based on the current architecture. */
#if defined(__i386__) || defined(__x86_64__)
#define XZ_DEC_X86
#elif defined(__arm__)
#define XZ_DEC_ARMTHUMB
#elif defined(__aarch64__)
#define XZ_DEC_ARM64
#elif defined(__riscv)
#define XZ_DEC_RISCV
#endif

/* We don't need the ARM BCJ filter. */
/* #define XZ_DEC_ARM */

/*
 * Visual Studio 2013 update 2 supports only __inline, not inline.
 * MSVC v19.0 / VS 2015 and newer support both.
 */
#if defined(_MSC_VER) && _MSC_VER < 1900 && !defined(inline)
#	define inline __inline
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "xz.h"

#define kmalloc(size, flags) malloc(size)
#define kfree(ptr) free(ptr)
#define vmalloc(size) malloc(size)
#define vfree(ptr) free(ptr)

#define memeq(a, b, size) (memcmp(a, b, size) == 0)
#define memzero(buf, size) memset(buf, 0, size)

#ifndef min
#	define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#define min_t(type, x, y) min(x, y)

#ifndef fallthrough
#	if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311
#		define fallthrough [[fallthrough]]
#	elif (defined(__GNUC__) && __GNUC__ >= 7) \
			|| (defined(__clang_major__) && __clang_major__ >= 10)
#		define fallthrough __attribute__((__fallthrough__))
#	else
#		define fallthrough do {} while (0)
#	endif
#endif

/*
 * Some functions have been marked with __always_inline to keep the
 * performance reasonable even when the compiler is optimizing for
 * small code size. You may be able to save a few bytes by #defining
 * __always_inline to plain inline, but don't complain if the code
 * becomes slow.
 *
 * NOTE: System headers on GNU/Linux may #define this macro already,
 * so if you want to change it, you need to #undef it first.
 */
#ifndef __always_inline
#	ifdef __GNUC__
#		define __always_inline \
			inline __attribute__((__always_inline__))
#	else
#		define __always_inline inline
#	endif
#endif

/* Inline functions to access unaligned unsigned 32-bit integers */
#ifndef get_unaligned_le32
static inline uint32_t get_unaligned_le32(const uint8_t *buf)
{
	return (uint32_t)buf[0]
			| ((uint32_t)buf[1] << 8)
			| ((uint32_t)buf[2] << 16)
			| ((uint32_t)buf[3] << 24);
}
#endif

#ifndef get_unaligned_be32
static inline uint32_t get_unaligned_be32(const uint8_t *buf)
{
	return (uint32_t)((uint32_t)buf[0] << 24)
			| ((uint32_t)buf[1] << 16)
			| ((uint32_t)buf[2] << 8)
			| (uint32_t)buf[3];
}
#endif

#ifndef put_unaligned_le32
static inline void put_unaligned_le32(uint32_t val, uint8_t *buf)
{
	buf[0] = (uint8_t)val;
	buf[1] = (uint8_t)(val >> 8);
	buf[2] = (uint8_t)(val >> 16);
	buf[3] = (uint8_t)(val >> 24);
}
#endif

#ifndef put_unaligned_be32
static inline void put_unaligned_be32(uint32_t val, uint8_t *buf)
{
	buf[0] = (uint8_t)(val >> 24);
	buf[1] = (uint8_t)(val >> 16);
	buf[2] = (uint8_t)(val >> 8);
	buf[3] = (uint8_t)val;
}
#endif

/*
 * To keep things simpler, use the generic unaligned methods also for
 * aligned access. The only place where performance could matter is
 * SHA-256 but files using SHA-256 aren't common.
 */
#ifndef get_le32
#	define get_le32 get_unaligned_le32
#endif
#ifndef get_be32
#	define get_be32 get_unaligned_be32
#endif

#endif
