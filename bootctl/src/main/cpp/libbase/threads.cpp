/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <android-base/threads.h>

#include <stdint.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <pthread.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <syscall.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace android {
namespace base {

uint64_t GetThreadId() {
#if defined(__BIONIC__)
  return gettid();
#elif defined(__APPLE__)
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  return tid;
#elif defined(__linux__)
  return syscall(__NR_gettid);
#elif defined(_WIN32)
  return GetCurrentThreadId();
#endif
}

}  // namespace base
}  // namespace android

#if defined(__GLIBC__)
int tgkill(int tgid, int tid, int sig) {
  return syscall(__NR_tgkill, tgid, tid, sig);
}
#endif
