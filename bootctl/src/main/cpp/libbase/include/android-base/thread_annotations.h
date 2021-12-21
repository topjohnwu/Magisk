/*
 * Copyright (C) 2016 The Android Open Source Project
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

#pragma once

#include <mutex>

#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))

#define CAPABILITY(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
      THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define SHARED_CAPABILITY(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(shared_capability(__VA_ARGS__))

#define GUARDED_BY(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define EXCLUSIVE_LOCKS_REQUIRED(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(__VA_ARGS__))

#define SHARED_LOCKS_REQUIRED(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(shared_locks_required(__VA_ARGS__))

#define ACQUIRED_BEFORE(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define EXCLUSIVE_LOCK_FUNCTION(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))

#define EXCLUSIVE_TRYLOCK_FUNCTION(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))

#define SHARED_LOCK_FUNCTION(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(shared_lock_function(__VA_ARGS__))

#define SHARED_TRYLOCK_FUNCTION(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(shared_trylock_function(__VA_ARGS__))

#define UNLOCK_FUNCTION(...) \
      THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))

#define SCOPED_LOCKABLE \
      THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define LOCK_RETURNED(x) \
      THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
      THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

namespace android {
namespace base {

// A class to help thread safety analysis deal with std::unique_lock and condition_variable.
//
// Clang's thread safety analysis currently doesn't perform alias analysis, so movable types
// like std::unique_lock can't be marked with thread safety annotations. This helper allows
// for manual assertion of lock state in a scope.
//
// For example:
//
//   std::mutex mutex;
//   std::condition_variable cv;
//   std::vector<int> vec GUARDED_BY(mutex);
//
//   int pop() {
//     std::unique_lock lock(mutex);
//     ScopedLockAssertion lock_assertion(mutex);
//     cv.wait(lock, []() {
//       ScopedLockAssertion lock_assertion(mutex);
//       return !vec.empty();
//     });
//
//     int result = vec.back();
//     vec.pop_back();
//     return result;
//   }
class SCOPED_CAPABILITY ScopedLockAssertion {
 public:
  ScopedLockAssertion(std::mutex& mutex) ACQUIRE(mutex) {}
  ~ScopedLockAssertion() RELEASE() {}
};

}  // namespace base
}  // namespace android
