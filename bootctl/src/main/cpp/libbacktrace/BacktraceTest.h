/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef _LIBBACKTRACE_BACKTRACE_TEST_H
#define _LIBBACKTRACE_BACKTRACE_TEST_H

#include <dlfcn.h>

#include <gtest/gtest.h>

class BacktraceTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    dl_handle_ = dlopen("libbacktrace_test.so", RTLD_NOW | RTLD_LOCAL);

    test_level_one_ = reinterpret_cast<int (*)(int, int, int, int, void (*)(void*), void*)>(
        dlsym(dl_handle_, "test_level_one"));

    test_level_two_ = reinterpret_cast<int (*)(int, int, int, int, void (*)(void*), void*)>(
        dlsym(dl_handle_, "test_level_two"));

    test_level_three_ = reinterpret_cast<int (*)(int, int, int, int, void (*)(void*), void*)>(
        dlsym(dl_handle_, "test_level_three"));

    test_level_four_ = reinterpret_cast<int (*)(int, int, int, int, void (*)(void*), void*)>(
        dlsym(dl_handle_, "test_level_four"));

    test_recursive_call_ = reinterpret_cast<int (*)(int, void (*)(void*), void*)>(
        dlsym(dl_handle_, "test_recursive_call"));

    test_get_context_and_wait_ = reinterpret_cast<void (*)(void*, volatile int*)>(
        dlsym(dl_handle_, "test_get_context_and_wait"));

    test_signal_action_ =
        reinterpret_cast<void (*)(int, siginfo_t*, void*)>(dlsym(dl_handle_, "test_signal_action"));

    test_signal_handler_ =
        reinterpret_cast<void (*)(int)>(dlsym(dl_handle_, "test_signal_handler"));
  }

  void SetUp() override {
    ASSERT_TRUE(dl_handle_ != nullptr);
    ASSERT_TRUE(test_level_one_ != nullptr);
    ASSERT_TRUE(test_level_two_ != nullptr);
    ASSERT_TRUE(test_level_three_ != nullptr);
    ASSERT_TRUE(test_level_four_ != nullptr);
    ASSERT_TRUE(test_recursive_call_ != nullptr);
    ASSERT_TRUE(test_get_context_and_wait_ != nullptr);
    ASSERT_TRUE(test_signal_action_ != nullptr);
    ASSERT_TRUE(test_signal_handler_ != nullptr);
  }

 public:
  static void* dl_handle_;
  static int (*test_level_one_)(int, int, int, int, void (*)(void*), void*);
  static int (*test_level_two_)(int, int, int, int, void (*)(void*), void*);
  static int (*test_level_three_)(int, int, int, int, void (*)(void*), void*);
  static int (*test_level_four_)(int, int, int, int, void (*)(void*), void*);
  static int (*test_recursive_call_)(int, void (*)(void*), void*);
  static void (*test_get_context_and_wait_)(void*, volatile int*);
  static void (*test_signal_action_)(int, siginfo_t*, void*);
  static void (*test_signal_handler_)(int);
};

#endif  // _LIBBACKTRACE_BACKTRACE_TEST_H
