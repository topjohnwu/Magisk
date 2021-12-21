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

#ifndef _LIBBACKTRACE_BACKTRACE_TESTLIB_H
#define _LIBBACKTRACE_BACKTRACE_TESTLIB_H

#include <sys/cdefs.h>

__BEGIN_DECLS

void test_loop_forever();
void test_signal_handler(int);
void test_signal_action(int, siginfo_t*, void*);
int test_level_four(int, int, int, int, void (*)(void*), void*);
int test_level_three(int, int, int, int, void (*)(void*), void*);
int test_level_two(int, int, int, int, void (*)(void*), void*);
int test_level_one(int, int, int, int, void (*)(void*), void*);
int test_recursive_call(int, void (*)(void*), void*);
void test_get_context_and_wait(void*, volatile int*);

__END_DECLS

#endif  // _LIBBACKTRACE_BACKTRACE_TESTLIB_H
