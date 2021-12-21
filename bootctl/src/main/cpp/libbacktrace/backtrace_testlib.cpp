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

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>

#include "backtrace_testlib.h"

void test_loop_forever() {
  while (1)
    ;
}

void test_signal_handler(int) { test_loop_forever(); }

void test_signal_action(int, siginfo_t*, void*) { test_loop_forever(); }

int test_level_four(int one, int two, int three, int four, void (*callback_func)(void*),
                    void* data) {
  if (callback_func != NULL) {
    callback_func(data);
  } else {
    while (1)
      ;
  }
  return one + two + three + four;
}

int test_level_three(int one, int two, int three, int four, void (*callback_func)(void*),
                     void* data) {
  return test_level_four(one + 3, two + 6, three + 9, four + 12, callback_func, data) + 3;
}

int test_level_two(int one, int two, int three, int four, void (*callback_func)(void*), void* data) {
  return test_level_three(one + 2, two + 4, three + 6, four + 8, callback_func, data) + 2;
}

int test_level_one(int one, int two, int three, int four, void (*callback_func)(void*), void* data) {
  return test_level_two(one + 1, two + 2, three + 3, four + 4, callback_func, data) + 1;
}

int test_recursive_call(int level, void (*callback_func)(void*), void* data) {
  if (level > 0) {
    return test_recursive_call(level - 1, callback_func, data) + level;
  } else if (callback_func != NULL) {
    callback_func(data);
  } else {
    while (1) {
    }
  }
  return 0;
}

typedef struct {
  std::vector<uint8_t>* ucontext;
  volatile int* exit_flag;
} GetContextArg;

static void GetContextAndExit(void* data) {
  GetContextArg* arg = reinterpret_cast<GetContextArg*>(data);

  std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::CreateFromLocal());
  unwindstack::RegsGetLocal(regs.get());

  ucontext_t ucontext;
  memset(&ucontext, 0, sizeof(ucontext));
#if defined(__arm__)
  memcpy(&ucontext.uc_mcontext, regs->RawData(), sizeof(uint32_t) * 16);
#elif defined(__aarch64__)
  memcpy(&ucontext.uc_mcontext, regs->RawData(), sizeof(uint64_t) * 33);
#elif defined(__i386__)
  uint32_t* reg_data = reinterpret_cast<uint32_t*>(regs->RawData());
  ucontext.uc_mcontext.gregs[0] = reg_data[15];
  ucontext.uc_mcontext.gregs[1] = reg_data[14];
  ucontext.uc_mcontext.gregs[2] = reg_data[13];
  ucontext.uc_mcontext.gregs[3] = reg_data[12];
  ucontext.uc_mcontext.gregs[4] = reg_data[7];
  ucontext.uc_mcontext.gregs[5] = reg_data[6];
  ucontext.uc_mcontext.gregs[6] = reg_data[5];
  ucontext.uc_mcontext.gregs[7] = reg_data[4];
  ucontext.uc_mcontext.gregs[8] = reg_data[3];
  ucontext.uc_mcontext.gregs[9] = reg_data[2];
  ucontext.uc_mcontext.gregs[10] = reg_data[1];
  ucontext.uc_mcontext.gregs[11] = reg_data[0];
  ucontext.uc_mcontext.gregs[14] = reg_data[8];
  ucontext.uc_mcontext.gregs[15] = reg_data[10];
#elif defined(__x86_64__)
  uint64_t* reg_data = reinterpret_cast<uint64_t*>(regs->RawData());
  ucontext.uc_mcontext.gregs[0] = reg_data[8];
  ucontext.uc_mcontext.gregs[1] = reg_data[9];
  ucontext.uc_mcontext.gregs[2] = reg_data[10];
  ucontext.uc_mcontext.gregs[3] = reg_data[11];
  ucontext.uc_mcontext.gregs[4] = reg_data[12];
  ucontext.uc_mcontext.gregs[5] = reg_data[13];
  ucontext.uc_mcontext.gregs[6] = reg_data[14];
  ucontext.uc_mcontext.gregs[7] = reg_data[15];
  ucontext.uc_mcontext.gregs[8] = reg_data[5];
  ucontext.uc_mcontext.gregs[9] = reg_data[4];
  ucontext.uc_mcontext.gregs[10] = reg_data[6];
  ucontext.uc_mcontext.gregs[11] = reg_data[3];
  ucontext.uc_mcontext.gregs[12] = reg_data[1];
  ucontext.uc_mcontext.gregs[13] = reg_data[0];
  ucontext.uc_mcontext.gregs[14] = reg_data[2];
  ucontext.uc_mcontext.gregs[15] = reg_data[7];
  ucontext.uc_mcontext.gregs[16] = reg_data[16];
#endif

  arg->ucontext->resize(sizeof(ucontext));
  memcpy(arg->ucontext->data(), &ucontext, sizeof(ucontext));

  // Don't touch the stack anymore.
  while (*arg->exit_flag == 0) {
  }
}

void test_get_context_and_wait(void* ucontext, volatile int* exit_flag) {
  GetContextArg arg;
  arg.ucontext = reinterpret_cast<std::vector<uint8_t>*>(ucontext);
  arg.exit_flag = exit_flag;
  test_level_one(1, 2, 3, 4, GetContextAndExit, &arg);
}
