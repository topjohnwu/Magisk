/*
 * Copyright (C) 2017 The Android Open Source Project
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

#pragma once

/* A cross-platform equivalent of bionic's <sys/endian.h>. */

/* For __BIONIC__ and __GLIBC__ */
#include <sys/cdefs.h>

#if defined(__BIONIC__)

#include <sys/endian.h>

#elif defined(__GLIBC__)

/* glibc's <endian.h> is like bionic's <sys/endian.h>. */
#include <endian.h>

/* glibc keeps htons and htonl in <netinet/in.h>. */
#include <netinet/in.h>

/* glibc doesn't have the 64-bit variants. */
#define htonq(x) htobe64(x)
#define ntohq(x) be64toh(x)

/* glibc has different names to BSD for these. */
#define betoh16(x) be16toh(x)
#define betoh32(x) be32toh(x)
#define betoh64(x) be64toh(x)
#define letoh16(x) le16toh(x)
#define letoh32(x) le32toh(x)
#define letoh64(x) le64toh(x)

#else

#if defined(__APPLE__)
/* macOS has some of the basics. */
#include <sys/_endian.h>
#else
/* Windows has some of the basics as well. */
#include <sys/param.h>
#include <winsock2.h>
/* winsock2.h *must* be included before the following four macros. */
#define htons(x) __builtin_bswap16(x)
#define htonl(x) __builtin_bswap32(x)
#define ntohs(x) __builtin_bswap16(x)
#define ntohl(x) __builtin_bswap32(x)
#endif

/* Neither macOS nor Windows have the rest. */

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __BYTE_ORDER __LITTLE_ENDIAN

#define htonq(x) __builtin_bswap64(x)

#define ntohq(x) __builtin_bswap64(x)

#define htobe16(x) __builtin_bswap16(x)
#define htobe32(x) __builtin_bswap32(x)
#define htobe64(x) __builtin_bswap64(x)

#define betoh16(x) __builtin_bswap16(x)
#define betoh32(x) __builtin_bswap32(x)
#define betoh64(x) __builtin_bswap64(x)

#define htole16(x) (x)
#define htole32(x) (x)
#define htole64(x) (x)

#define letoh16(x) (x)
#define letoh32(x) (x)
#define letoh64(x) (x)

#define be16toh(x) __builtin_bswap16(x)
#define be32toh(x) __builtin_bswap32(x)
#define be64toh(x) __builtin_bswap64(x)

#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)

#endif
