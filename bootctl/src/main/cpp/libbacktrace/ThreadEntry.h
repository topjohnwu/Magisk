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

#ifndef _LIBBACKTRACE_THREAD_ENTRY_H
#define _LIBBACKTRACE_THREAD_ENTRY_H

#include <pthread.h>
#include <sys/types.h>
#include <ucontext.h>

class ThreadEntry {
 public:
  static ThreadEntry* Get(pid_t pid, pid_t tid, bool create = true);

  static void Remove(ThreadEntry* entry);

  void Wake();

  bool Wait(int);

  void CopyUcontextFromSigcontext(void*);

  inline void Lock() {
    pthread_mutex_lock(&mutex_);

    // Always reset the wait value since this could be the first or nth
    // time this entry is locked.
    wait_value_ = 0;
  }

  inline void Unlock() {
    pthread_mutex_unlock(&mutex_);
  }

  inline ucontext_t* GetUcontext() { return &ucontext_; }

 private:
  ThreadEntry(pid_t pid, pid_t tid);
  ~ThreadEntry();

  bool Match(pid_t chk_pid, pid_t chk_tid) { return (chk_pid == pid_ && chk_tid == tid_); }

  pid_t pid_;
  pid_t tid_;
  int ref_count_;
  pthread_mutex_t mutex_;
  pthread_mutex_t wait_mutex_;
  pthread_cond_t wait_cond_;
  int wait_value_;
  ThreadEntry* next_;
  ThreadEntry* prev_;
  ucontext_t ucontext_;

  static ThreadEntry* list_;
  static pthread_mutex_t list_mutex_;
};

#endif // _LIBBACKTRACE_THREAD_ENTRY_H
