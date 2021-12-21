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

#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#if !defined(_WIN32)
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

// DO NOT INCLUDE OTHER LIBBASE HEADERS!
// This file gets used in libbinder, and libbinder is used everywhere.
// Including other headers from libbase frequently results in inclusion of
// android-base/macros.h, which causes macro collisions.

// Container for a file descriptor that automatically closes the descriptor as
// it goes out of scope.
//
//      unique_fd ufd(open("/some/path", "r"));
//      if (ufd.get() == -1) return error;
//
//      // Do something useful, possibly including 'return'.
//
//      return 0; // Descriptor is closed for you.
//
// unique_fd is also known as ScopedFd/ScopedFD/scoped_fd; mentioned here to help
// you find this class if you're searching for one of those names.

#if defined(__BIONIC__)
#include <android/fdsan.h>
#endif

namespace android {
namespace base {

struct DefaultCloser {
#if defined(__BIONIC__)
  static void Tag(int fd, void* old_addr, void* new_addr) {
    if (android_fdsan_exchange_owner_tag) {
      uint64_t old_tag = android_fdsan_create_owner_tag(ANDROID_FDSAN_OWNER_TYPE_UNIQUE_FD,
                                                        reinterpret_cast<uint64_t>(old_addr));
      uint64_t new_tag = android_fdsan_create_owner_tag(ANDROID_FDSAN_OWNER_TYPE_UNIQUE_FD,
                                                        reinterpret_cast<uint64_t>(new_addr));
      android_fdsan_exchange_owner_tag(fd, old_tag, new_tag);
    }
  }
  static void Close(int fd, void* addr) {
    if (android_fdsan_close_with_tag) {
      uint64_t tag = android_fdsan_create_owner_tag(ANDROID_FDSAN_OWNER_TYPE_UNIQUE_FD,
                                                    reinterpret_cast<uint64_t>(addr));
      android_fdsan_close_with_tag(fd, tag);
    } else {
      close(fd);
    }
  }
#else
  static void Close(int fd) {
    // Even if close(2) fails with EINTR, the fd will have been closed.
    // Using TEMP_FAILURE_RETRY will either lead to EBADF or closing someone
    // else's fd.
    // http://lkml.indiana.edu/hypermail/linux/kernel/0509.1/0877.html
    ::close(fd);
  }
#endif
};

template <typename Closer>
class unique_fd_impl final {
 public:
  unique_fd_impl() {}

  explicit unique_fd_impl(int fd) { reset(fd); }
  ~unique_fd_impl() { reset(); }

  unique_fd_impl(const unique_fd_impl&) = delete;
  void operator=(const unique_fd_impl&) = delete;
  unique_fd_impl(unique_fd_impl&& other) noexcept { reset(other.release()); }
  unique_fd_impl& operator=(unique_fd_impl&& s) noexcept {
    int fd = s.fd_;
    s.fd_ = -1;
    reset(fd, &s);
    return *this;
  }

  [[clang::reinitializes]] void reset(int new_value = -1) { reset(new_value, nullptr); }

  int get() const { return fd_; }

#if !defined(ANDROID_BASE_UNIQUE_FD_DISABLE_IMPLICIT_CONVERSION)
  // unique_fd's operator int is dangerous, but we have way too much code that
  // depends on it, so make this opt-in at first.
  operator int() const { return get(); }  // NOLINT
#endif

  bool operator>=(int rhs) const { return get() >= rhs; }
  bool operator<(int rhs) const { return get() < rhs; }
  bool operator==(int rhs) const { return get() == rhs; }
  bool operator!=(int rhs) const { return get() != rhs; }
  bool operator==(const unique_fd_impl& rhs) const { return get() == rhs.get(); }
  bool operator!=(const unique_fd_impl& rhs) const { return get() != rhs.get(); }

  // Catch bogus error checks (i.e.: "!fd" instead of "fd != -1").
  bool operator!() const = delete;

  bool ok() const { return get() >= 0; }

  int release() __attribute__((warn_unused_result)) {
    tag(fd_, this, nullptr);
    int ret = fd_;
    fd_ = -1;
    return ret;
  }

 private:
  void reset(int new_value, void* previous_tag) {
    int previous_errno = errno;

    if (fd_ != -1) {
      close(fd_, this);
    }

    fd_ = new_value;
    if (new_value != -1) {
      tag(new_value, previous_tag, this);
    }

    errno = previous_errno;
  }

  int fd_ = -1;

  // Template magic to use Closer::Tag if available, and do nothing if not.
  // If Closer::Tag exists, this implementation is preferred, because int is a better match.
  // If not, this implementation is SFINAEd away, and the no-op below is the only one that exists.
  template <typename T = Closer>
  static auto tag(int fd, void* old_tag, void* new_tag)
      -> decltype(T::Tag(fd, old_tag, new_tag), void()) {
    T::Tag(fd, old_tag, new_tag);
  }

  template <typename T = Closer>
  static void tag(long, void*, void*) {
    // No-op.
  }

  // Same as above, to select between Closer::Close(int) and Closer::Close(int, void*).
  template <typename T = Closer>
  static auto close(int fd, void* tag_value) -> decltype(T::Close(fd, tag_value), void()) {
    T::Close(fd, tag_value);
  }

  template <typename T = Closer>
  static auto close(int fd, void*) -> decltype(T::Close(fd), void()) {
    T::Close(fd);
  }
};

using unique_fd = unique_fd_impl<DefaultCloser>;

#if !defined(_WIN32)

// Inline functions, so that they can be used header-only.
template <typename Closer>
inline bool Pipe(unique_fd_impl<Closer>* read, unique_fd_impl<Closer>* write,
                 int flags = O_CLOEXEC) {
  int pipefd[2];

#if defined(__linux__)
  if (pipe2(pipefd, flags) != 0) {
    return false;
  }
#else  // defined(__APPLE__)
  if (flags & ~(O_CLOEXEC | O_NONBLOCK)) {
    return false;
  }
  if (pipe(pipefd) != 0) {
    return false;
  }

  if (flags & O_CLOEXEC) {
    if (fcntl(pipefd[0], F_SETFD, FD_CLOEXEC) != 0 || fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) != 0) {
      close(pipefd[0]);
      close(pipefd[1]);
      return false;
    }
  }
  if (flags & O_NONBLOCK) {
    if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) != 0 || fcntl(pipefd[1], F_SETFL, O_NONBLOCK) != 0) {
      close(pipefd[0]);
      close(pipefd[1]);
      return false;
    }
  }
#endif

  read->reset(pipefd[0]);
  write->reset(pipefd[1]);
  return true;
}

template <typename Closer>
inline bool Socketpair(int domain, int type, int protocol, unique_fd_impl<Closer>* left,
                       unique_fd_impl<Closer>* right) {
  int sockfd[2];
  if (socketpair(domain, type, protocol, sockfd) != 0) {
    return false;
  }
  left->reset(sockfd[0]);
  right->reset(sockfd[1]);
  return true;
}

template <typename Closer>
inline bool Socketpair(int type, unique_fd_impl<Closer>* left, unique_fd_impl<Closer>* right) {
  return Socketpair(AF_UNIX, type, 0, left, right);
}

// Using fdopen with unique_fd correctly is more annoying than it should be,
// because fdopen doesn't close the file descriptor received upon failure.
inline FILE* Fdopen(unique_fd&& ufd, const char* mode) {
  int fd = ufd.release();
  FILE* file = fdopen(fd, mode);
  if (!file) {
    close(fd);
  }
  return file;
}

// Using fdopendir with unique_fd correctly is more annoying than it should be,
// because fdopen doesn't close the file descriptor received upon failure.
inline DIR* Fdopendir(unique_fd&& ufd) {
  int fd = ufd.release();
  DIR* dir = fdopendir(fd);
  if (dir == nullptr) {
    close(fd);
  }
  return dir;
}

#endif  // !defined(_WIN32)

// A wrapper type that can be implicitly constructed from either int or unique_fd.
struct borrowed_fd {
  /* implicit */ borrowed_fd(int fd) : fd_(fd) {}  // NOLINT
  template <typename T>
  /* implicit */ borrowed_fd(const unique_fd_impl<T>& ufd) : fd_(ufd.get()) {}  // NOLINT

  int get() const { return fd_; }

  bool operator>=(int rhs) const { return get() >= rhs; }
  bool operator<(int rhs) const { return get() < rhs; }
  bool operator==(int rhs) const { return get() == rhs; }
  bool operator!=(int rhs) const { return get() != rhs; }

 private:
  int fd_ = -1;
};
}  // namespace base
}  // namespace android

template <typename T>
int close(const android::base::unique_fd_impl<T>&)
    __attribute__((__unavailable__("close called on unique_fd")));

template <typename T>
FILE* fdopen(const android::base::unique_fd_impl<T>&, const char* mode)
    __attribute__((__unavailable__("fdopen takes ownership of the fd passed in; either dup the "
                                   "unique_fd, or use android::base::Fdopen to pass ownership")));

template <typename T>
DIR* fdopendir(const android::base::unique_fd_impl<T>&) __attribute__((
    __unavailable__("fdopendir takes ownership of the fd passed in; either dup the "
                    "unique_fd, or use android::base::Fdopendir to pass ownership")));
