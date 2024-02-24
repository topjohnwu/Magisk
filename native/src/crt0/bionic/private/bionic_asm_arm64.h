/*	$OpenBSD: asm.h,v 1.1 2004/02/01 05:09:49 drahn Exp $	*/
/*	$NetBSD: asm.h,v 1.4 2001/07/16 05:43:32 matt Exp $	*/

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)asm.h	5.5 (Berkeley) 5/7/91
 */

#pragma once

#define __bionic_asm_align 16

#undef __bionic_asm_function_type
#define __bionic_asm_function_type %function

#if defined(__ARM_FEATURE_BTI_DEFAULT)
#define __bionic_asm_aarch64_feature_bti    (1 << 0)
#undef __bionic_asm_custom_entry
#define __bionic_asm_custom_entry(f)        bti c
#else
#define __bionic_asm_aarch64_feature_bti    0
#endif

#if defined(__ARM_FEATURE_PAC_DEFAULT)
#define __bionic_asm_aarch64_feature_pac    (1 << 1)
#else
#define __bionic_asm_aarch64_feature_pac    0
#endif

#undef __bionic_asm_custom_note_gnu_section
#define __bionic_asm_custom_note_gnu_section() \
    .pushsection .note.gnu.property, "a"; \
    .balign 8; \
    .long 4; \
    .long 0x10; \
    .long 0x5; /* NT_GNU_PROPERTY_TYPE_0 */ \
    .asciz "GNU"; \
    .long 0xc0000000; /* GNU_PROPERTY_AARCH64_FEATURE_1_AND */ \
    .long 4; \
    .long (__bionic_asm_aarch64_feature_pac | \
           __bionic_asm_aarch64_feature_bti); \
    .long 0; \
    .popsection;

#define NT_MEMTAG_LEVEL_MASK 3
#define NT_MEMTAG_LEVEL_NONE 0
#define NT_MEMTAG_LEVEL_ASYNC 1
#define NT_MEMTAG_LEVEL_SYNC 2
#define NT_MEMTAG_HEAP 4
#define NT_MEMTAG_STACK 8
