/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ucontext.h>

#include "BacktraceAsyncSafeLog.h"
#include "ThreadEntry.h"

// Initialize static member variables.
ThreadEntry* ThreadEntry::list_ = nullptr;
pthread_mutex_t ThreadEntry::list_mutex_ = PTHREAD_MUTEX_INITIALIZER;

// Assumes that ThreadEntry::list_mutex_ has already been locked before
// creating a ThreadEntry object.
ThreadEntry::ThreadEntry(pid_t pid, pid_t tid)
    : pid_(pid), tid_(tid), ref_count_(1), mutex_(PTHREAD_MUTEX_INITIALIZER),
      wait_mutex_(PTHREAD_MUTEX_INITIALIZER), wait_value_(0),
      next_(ThreadEntry::list_), prev_(nullptr) {
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  pthread_cond_init(&wait_cond_, &attr);

  // Add ourselves to the list.
  if (ThreadEntry::list_) {
    ThreadEntry::list_->prev_ = this;
  }
  ThreadEntry::list_ = this;
}

ThreadEntry* ThreadEntry::Get(pid_t pid, pid_t tid, bool create) {
  pthread_mutex_lock(&ThreadEntry::list_mutex_);
  ThreadEntry* entry = list_;
  while (entry != nullptr) {
    if (entry->Match(pid, tid)) {
      break;
    }
    entry = entry->next_;
  }

  if (!entry) {
    if (create) {
      entry = new ThreadEntry(pid, tid);
    }
  } else {
    entry->ref_count_++;
  }
  pthread_mutex_unlock(&ThreadEntry::list_mutex_);

  return entry;
}

void ThreadEntry::Remove(ThreadEntry* entry) {
  entry->Unlock();

  pthread_mutex_lock(&ThreadEntry::list_mutex_);
  if (--entry->ref_count_ == 0) {
    delete entry;
  }
  pthread_mutex_unlock(&ThreadEntry::list_mutex_);
}

// Assumes that ThreadEntry::list_mutex_ has already been locked before
// deleting a ThreadEntry object.
ThreadEntry::~ThreadEntry() {
  if (list_ == this) {
    list_ = next_;
  } else {
    if (next_) {
      next_->prev_ = prev_;
    }
    prev_->next_ = next_;
  }

  next_ = nullptr;
  prev_ = nullptr;

  pthread_cond_destroy(&wait_cond_);
}

bool ThreadEntry::Wait(int value) {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  ts.tv_sec += 5;

  bool wait_completed = true;
  pthread_mutex_lock(&wait_mutex_);
  while (wait_value_ != value) {
    int ret = pthread_cond_timedwait(&wait_cond_, &wait_mutex_, &ts);
    if (ret != 0) {
      BACK_ASYNC_SAFE_LOGW("pthread_cond_timedwait for value %d failed: %s", value, strerror(ret));
      wait_completed = false;
      break;
    }
  }
  pthread_mutex_unlock(&wait_mutex_);

  return wait_completed;
}

void ThreadEntry::Wake() {
  pthread_mutex_lock(&wait_mutex_);
  wait_value_++;
  pthread_mutex_unlock(&wait_mutex_);

  pthread_cond_signal(&wait_cond_);
}

void ThreadEntry::CopyUcontextFromSigcontext(void* sigcontext) {
  ucontext_t* ucontext = reinterpret_cast<ucontext_t*>(sigcontext);
  // The only thing the unwinder cares about is the mcontext data.
  memcpy(&ucontext_.uc_mcontext, &ucontext->uc_mcontext, sizeof(ucontext->uc_mcontext));
}
