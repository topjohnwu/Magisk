/*
** Copyright (C) 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <cutils/threads.h>

#if defined(__APPLE__)
#include <stdint.h>
#elif defined(__linux__)
#include <syscall.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#if defined(__BIONIC__) || defined(__GLIBC__) && __GLIBC_MINOR__ >= 32
// No definition needed for Android because we'll just pick up bionic's copy.
// No definition needed for Glibc >= 2.32 because it exposes its own copy.
#else
pid_t gettid() {
#if defined(__APPLE__)
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  return tid;
#elif defined(__linux__)
  return syscall(__NR_gettid);
#elif defined(_WIN32)
  return GetCurrentThreadId();
#endif
}
#endif
