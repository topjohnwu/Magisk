/*
 * An implementation of C11 stdatomic.h directly borrowed from FreeBSD
 * (original copyright follows), with minor modifications for
 * portability to other systems. Works for recent Clang (that
 * implement the feature c_atomic) and GCC 4.7+; includes
 * compatibility for GCC below 4.7 but I wouldn't recommend it.
 *
 * Caveats and limitations:
 * - Only the ``_Atomic parentheses'' notation is implemented, while
 *   the ``_Atomic space'' one is not.
 * - _Atomic types must be typedef'ed, or programs using them will
 *   not type check correctly (incompatible anonymous structure
 *   types).
 * - Non-scalar _Atomic types would require runtime support for
 *   runtime locking, which, as far as I know, is not currently
 *   available on any system.
 */

/*-
 * Copyright (c) 2011 Ed Schouten <ed@FreeBSD.org>
 *                    David Chisnall <theraven@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/include/stdatomic.h,v 1.10.2.2 2012/05/30 19:21:54 theraven Exp $
 */

#ifndef _STDATOMIC_H_
#define	_STDATOMIC_H_

#include <stddef.h>
#include <stdint.h>
#define _Bool bool

#if !defined(__has_feature)
#define __has_feature(x) 0
#endif
#if !defined(__has_builtin)
#define __has_builtin(x) 0
#endif
#if !defined(__GNUC_PREREQ__)
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ__(maj, min)					\
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ__(maj, min) 0
#endif
#endif

#if !defined(__CLANG_ATOMICS) && !defined(__GNUC_ATOMICS)
#if __has_feature(c_atomic)
#define	__CLANG_ATOMICS
#elif __GNUC_PREREQ__(4, 7)
#define	__GNUC_ATOMICS
#elif !defined(__GNUC__)
#error "stdatomic.h does not support your compiler"
#endif
#endif

#if !defined(__CLANG_ATOMICS)
#define	_Atomic(T)			struct { volatile __typeof__(T) __val; }
#endif

/*
 * 7.17.2 Initialization.
 */

#if defined(__CLANG_ATOMICS)
#define	ATOMIC_VAR_INIT(value)		(value)
#define	atomic_init(obj, value)		__c11_atomic_init(obj, value)
#else
#define	ATOMIC_VAR_INIT(value)		{ .__val = (value) }
#define	atomic_init(obj, value) do {					\
	(obj)->__val = (value);						\
} while (0)
#endif

/*
 * Clang and recent GCC both provide predefined macros for the memory
 * orderings.  If we are using a compiler that doesn't define them, use the
 * clang values - these will be ignored in the fallback path.
 */

#ifndef __ATOMIC_RELAXED
#define __ATOMIC_RELAXED		0
#endif
#ifndef __ATOMIC_CONSUME
#define __ATOMIC_CONSUME		1
#endif
#ifndef __ATOMIC_ACQUIRE
#define __ATOMIC_ACQUIRE		2
#endif
#ifndef __ATOMIC_RELEASE
#define __ATOMIC_RELEASE		3
#endif
#ifndef __ATOMIC_ACQ_REL
#define __ATOMIC_ACQ_REL		4
#endif
#ifndef __ATOMIC_SEQ_CST
#define __ATOMIC_SEQ_CST		5
#endif

/*
 * 7.17.3 Order and consistency.
 *
 * The memory_order_* constants that denote the barrier behaviour of the
 * atomic operations.
 */

enum memory_order {
	memory_order_relaxed = __ATOMIC_RELAXED,
	memory_order_consume = __ATOMIC_CONSUME,
	memory_order_acquire = __ATOMIC_ACQUIRE,
	memory_order_release = __ATOMIC_RELEASE,
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	memory_order_seq_cst = __ATOMIC_SEQ_CST
};

typedef enum memory_order memory_order;

/*
 * 7.17.4 Fences.
 */

#ifdef __CLANG_ATOMICS
#define	atomic_thread_fence(order)	__c11_atomic_thread_fence(order)
#define	atomic_signal_fence(order)	__c11_atomic_signal_fence(order)
#elif defined(__GNUC_ATOMICS)
#define	atomic_thread_fence(order)	__atomic_thread_fence(order)
#define	atomic_signal_fence(order)	__atomic_signal_fence(order)
#else
#define	atomic_thread_fence(order)	__sync_synchronize()
#define	atomic_signal_fence(order)	__asm volatile ("" : : : "memory")
#endif

/*
 * 7.17.5 Lock-free property.
 */

#if defined(__CLANG_ATOMICS)
#define	atomic_is_lock_free(obj) \
	__c11_atomic_is_lock_free(sizeof(obj))
#elif defined(__GNUC_ATOMICS)
#define	atomic_is_lock_free(obj) \
	__atomic_is_lock_free(sizeof((obj)->__val))
#else
#define	atomic_is_lock_free(obj) \
	(sizeof((obj)->__val) <= sizeof(void *))
#endif

/*
 * 7.17.6 Atomic integer types.
 */

typedef _Atomic(_Bool)			atomic_bool;
typedef _Atomic(char)			atomic_char;
typedef _Atomic(signed char)		atomic_schar;
typedef _Atomic(unsigned char)		atomic_uchar;
typedef _Atomic(short)			atomic_short;
typedef _Atomic(unsigned short)		atomic_ushort;
typedef _Atomic(int)			atomic_int;
typedef _Atomic(unsigned int)		atomic_uint;
typedef _Atomic(long)			atomic_long;
typedef _Atomic(unsigned long)		atomic_ulong;
typedef _Atomic(long long)		atomic_llong;
typedef _Atomic(unsigned long long)	atomic_ullong;
#if 0
typedef _Atomic(char16_t)		atomic_char16_t;
typedef _Atomic(char32_t)		atomic_char32_t;
#endif
typedef _Atomic(wchar_t)		atomic_wchar_t;
typedef _Atomic(int_least8_t)		atomic_int_least8_t;
typedef _Atomic(uint_least8_t)		atomic_uint_least8_t;
typedef _Atomic(int_least16_t)		atomic_int_least16_t;
typedef _Atomic(uint_least16_t)		atomic_uint_least16_t;
typedef _Atomic(int_least32_t)		atomic_int_least32_t;
typedef _Atomic(uint_least32_t)		atomic_uint_least32_t;
typedef _Atomic(int_least64_t)		atomic_int_least64_t;
typedef _Atomic(uint_least64_t)		atomic_uint_least64_t;
typedef _Atomic(int_fast8_t)		atomic_int_fast8_t;
typedef _Atomic(uint_fast8_t)		atomic_uint_fast8_t;
typedef _Atomic(int_fast16_t)		atomic_int_fast16_t;
typedef _Atomic(uint_fast16_t)		atomic_uint_fast16_t;
typedef _Atomic(int_fast32_t)		atomic_int_fast32_t;
typedef _Atomic(uint_fast32_t)		atomic_uint_fast32_t;
typedef _Atomic(int_fast64_t)		atomic_int_fast64_t;
typedef _Atomic(uint_fast64_t)		atomic_uint_fast64_t;
typedef _Atomic(intptr_t)		atomic_intptr_t;
typedef _Atomic(uintptr_t)		atomic_uintptr_t;
typedef _Atomic(size_t)			atomic_size_t;
typedef _Atomic(ptrdiff_t)		atomic_ptrdiff_t;
typedef _Atomic(intmax_t)		atomic_intmax_t;
typedef _Atomic(uintmax_t)		atomic_uintmax_t;

/*
 * 7.17.7 Operations on atomic types.
 */

/*
 * Compiler-specific operations.
 */

#if defined(__CLANG_ATOMICS)
#define	atomic_compare_exchange_strong_explicit(object, expected,	\
    desired, success, failure)						\
	__c11_atomic_compare_exchange_strong(object, expected, desired,	\
	    success, failure)
#define	atomic_compare_exchange_weak_explicit(object, expected,		\
    desired, success, failure)						\
	__c11_atomic_compare_exchange_weak(object, expected, desired,	\
	    success, failure)
#define	atomic_exchange_explicit(object, desired, order)		\
	__c11_atomic_exchange(object, desired, order)
#define	atomic_fetch_add_explicit(object, operand, order)		\
	__c11_atomic_fetch_add(object, operand, order)
#define	atomic_fetch_and_explicit(object, operand, order)		\
	__c11_atomic_fetch_and(object, operand, order)
#define	atomic_fetch_or_explicit(object, operand, order)		\
	__c11_atomic_fetch_or(object, operand, order)
#define	atomic_fetch_sub_explicit(object, operand, order)		\
	__c11_atomic_fetch_sub(object, operand, order)
#define	atomic_fetch_xor_explicit(object, operand, order)		\
	__c11_atomic_fetch_xor(object, operand, order)
#define	atomic_load_explicit(object, order)				\
	__c11_atomic_load(object, order)
#define	atomic_store_explicit(object, desired, order)			\
	__c11_atomic_store(object, desired, order)
#elif defined(__GNUC_ATOMICS)
#define	atomic_compare_exchange_strong_explicit(object, expected,	\
    desired, success, failure)						\
	__atomic_compare_exchange_n(&(object)->__val, expected,		\
	    desired, 0, success, failure)
#define	atomic_compare_exchange_weak_explicit(object, expected,		\
    desired, success, failure)						\
	__atomic_compare_exchange_n(&(object)->__val, expected,		\
	    desired, 1, success, failure)
#define	atomic_exchange_explicit(object, desired, order)		\
	__atomic_exchange_n(&(object)->__val, desired, order)
#define	atomic_fetch_add_explicit(object, operand, order)		\
	__atomic_fetch_add(&(object)->__val, operand, order)
#define	atomic_fetch_and_explicit(object, operand, order)		\
	__atomic_fetch_and(&(object)->__val, operand, order)
#define	atomic_fetch_or_explicit(object, operand, order)		\
	__atomic_fetch_or(&(object)->__val, operand, order)
#define	atomic_fetch_sub_explicit(object, operand, order)		\
	__atomic_fetch_sub(&(object)->__val, operand, order)
#define	atomic_fetch_xor_explicit(object, operand, order)		\
	__atomic_fetch_xor(&(object)->__val, operand, order)
#define	atomic_load_explicit(object, order)				\
	__atomic_load_n(&(object)->__val, order)
#define	atomic_store_explicit(object, desired, order)			\
	__atomic_store_n(&(object)->__val, desired, order)
#else
#define	atomic_compare_exchange_strong_explicit(object, expected,	\
    desired, success, failure) ({					\
	__typeof__((object)->__val) __v;				\
	_Bool __r;							\
	__v = __sync_val_compare_and_swap(&(object)->__val,		\
	    *(expected), desired);					\
	__r = *(expected) == __v;					\
	*(expected) = __v;						\
	__r;								\
})

#define	atomic_compare_exchange_weak_explicit(object, expected,		\
    desired, success, failure)						\
	atomic_compare_exchange_strong_explicit(object, expected,	\
		desired, success, failure)
#if __has_builtin(__sync_swap)
/* Clang provides a full-barrier atomic exchange - use it if available. */
#define atomic_exchange_explicit(object, desired, order)		\
	__sync_swap(&(object)->__val, desired)
#else
/*
 * __sync_lock_test_and_set() is only an acquire barrier in theory (although in
 * practice it is usually a full barrier) so we need an explicit barrier after
 * it.
 */
#define	atomic_exchange_explicit(object, desired, order) ({		\
	__typeof__((object)->__val) __v;				\
	__v = __sync_lock_test_and_set(&(object)->__val, desired);	\
	__sync_synchronize();						\
	__v;								\
})
#endif
#define	atomic_fetch_add_explicit(object, operand, order)		\
	__sync_fetch_and_add(&(object)->__val, operand)
#define	atomic_fetch_and_explicit(object, operand, order)		\
	__sync_fetch_and_and(&(object)->__val, operand)
#define	atomic_fetch_or_explicit(object, operand, order)		\
	__sync_fetch_and_or(&(object)->__val, operand)
#define	atomic_fetch_sub_explicit(object, operand, order)		\
	__sync_fetch_and_sub(&(object)->__val, operand)
#define	atomic_fetch_xor_explicit(object, operand, order)		\
	__sync_fetch_and_xor(&(object)->__val, operand)
#define	atomic_load_explicit(object, order)				\
	__sync_fetch_and_add(&(object)->__val, 0)
#define	atomic_store_explicit(object, desired, order) do {		\
	__sync_synchronize();						\
	(object)->__val = (desired);					\
	__sync_synchronize();						\
} while (0)
#endif

/*
 * Convenience functions.
 */

#define	atomic_compare_exchange_strong(object, expected, desired)	\
	atomic_compare_exchange_strong_explicit(object, expected,	\
	    desired, memory_order_seq_cst, memory_order_seq_cst)
#define	atomic_compare_exchange_weak(object, expected, desired)		\
	atomic_compare_exchange_weak_explicit(object, expected,		\
	    desired, memory_order_seq_cst, memory_order_seq_cst)
#define	atomic_exchange(object, desired)				\
	atomic_exchange_explicit(object, desired, memory_order_seq_cst)
#define	atomic_fetch_add(object, operand)				\
	atomic_fetch_add_explicit(object, operand, memory_order_seq_cst)
#define	atomic_fetch_and(object, operand)				\
	atomic_fetch_and_explicit(object, operand, memory_order_seq_cst)
#define	atomic_fetch_or(object, operand)				\
	atomic_fetch_or_explicit(object, operand, memory_order_seq_cst)
#define	atomic_fetch_sub(object, operand)				\
	atomic_fetch_sub_explicit(object, operand, memory_order_seq_cst)
#define	atomic_fetch_xor(object, operand)				\
	atomic_fetch_xor_explicit(object, operand, memory_order_seq_cst)
#define	atomic_load(object)						\
	atomic_load_explicit(object, memory_order_seq_cst)
#define	atomic_store(object, desired)					\
	atomic_store_explicit(object, desired, memory_order_seq_cst)

/*
 * 7.17.8 Atomic flag type and operations.
 */

typedef atomic_bool			atomic_flag;

#define	ATOMIC_FLAG_INIT		ATOMIC_VAR_INIT(0)

#define	atomic_flag_clear_explicit(object, order)			\
	atomic_store_explicit(object, 0, order)
#define	atomic_flag_test_and_set_explicit(object, order)		\
	atomic_compare_exchange_strong_explicit(object, 0, 1, order, order)

#define	atomic_flag_clear(object)					\
	atomic_flag_clear_explicit(object, memory_order_seq_cst)
#define	atomic_flag_test_and_set(object)				\
	atomic_flag_test_and_set_explicit(object, memory_order_seq_cst)

#endif /* !_STDATOMIC_H_ */