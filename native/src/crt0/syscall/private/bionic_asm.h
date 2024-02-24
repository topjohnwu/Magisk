/*
 * Copyright (C) 2013 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#define __ASSEMBLY__

/* https://github.com/android/ndk/issues/1422 */
#include <features.h>

#include <asm/unistd.h> /* For system call numbers. */
#define MAX_ERRNO 4095  /* For recognizing system call error returns. */

#define __bionic_asm_custom_entry(f)
#define __bionic_asm_custom_end(f)
#define __bionic_asm_function_type @function
#define __bionic_asm_custom_note_gnu_section()

#if defined(__aarch64__)
#include "bionic_asm_arm64.h"
#elif defined(__arm__)
#include "bionic_asm_arm.h"
#elif defined(__i386__)
#include "bionic_asm_x86.h"
#elif defined(__riscv)
#include "bionic_asm_riscv64.h"
#elif defined(__x86_64__)
#include "bionic_asm_x86_64.h"
#endif

// Starts a normal assembler routine.
#define ENTRY(__f) __ENTRY_WITH_BINDING(__f, .globl)

// Starts an assembler routine with hidden visibility.
#define ENTRY_PRIVATE(__f)           \
  __ENTRY_WITH_BINDING(__f, .globl); \
  .hidden __f;

// Starts an assembler routine that's weak so native bridges can override it.
#define ENTRY_WEAK_FOR_NATIVE_BRIDGE(__f) __ENTRY_WITH_BINDING(__f, .weak)

// Starts an assembler routine with hidden visibility and no DWARF information.
// Only used for internal functions passed via sa_restorer.
// TODO: can't we just delete all those and let the kernel do its thing?
#define ENTRY_NO_DWARF_PRIVATE(__f) \
  __ENTRY_NO_DWARF(__f, .globl);    \
  .hidden __f;

// (Implementation detail.)
#define __ENTRY_NO_DWARF(__f, __binding) \
  .text;                                 \
  __binding __f;                         \
  .balign __bionic_asm_align;            \
  .type __f, __bionic_asm_function_type; \
  __f:                                   \
  __bionic_asm_custom_entry(__f);

// (Implementation detail.)
#define __ENTRY_WITH_BINDING(__f, __binding) \
  __ENTRY_NO_DWARF(__f, __binding);          \
  .cfi_startproc;

// Ends a normal assembler routine.
#define END(__f) \
  .cfi_endproc;  \
  END_NO_DWARF(__f)

// Ends an assembler routine with no DWARF information.
// Only used for internal functions passed via sa_restorer.
// TODO: can't we just delete all those and let the kernel do its thing?
#define END_NO_DWARF(__f) \
  .size __f, .- __f;      \
  __bionic_asm_custom_end(__f)

// Creates an alias `alias` for the symbol `original`.
#define ALIAS_SYMBOL(alias, original) \
  .globl alias;                       \
  .equ alias, original

// Creates an alias `alias` for the symbol `original` that's weak so it can be
// separately overridden by native bridges.
#define ALIAS_SYMBOL_WEAK_FOR_NATIVE_BRIDGE(alias, original) \
  .weak alias;                                               \
  .equ alias, original

// Adds a GNU property ELF note. Important on arm64 to declare PAC/BTI support.
#define NOTE_GNU_PROPERTY() __bionic_asm_custom_note_gnu_section()

// Gives local labels a more convenient and readable syntax.
#define L(__label) .L##__label
