/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <android-base/cmsg.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/user.h>

#include <memory>

#include <android-base/logging.h>

namespace android {
namespace base {

ssize_t SendFileDescriptorVector(borrowed_fd sockfd, const void* data, size_t len,
                                 const std::vector<int>& fds) {
  static const size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t cmsg_space = CMSG_SPACE(sizeof(int) * fds.size());
  size_t cmsg_len = CMSG_LEN(sizeof(int) * fds.size());
  if (cmsg_space >= page_size) {
    errno = ENOMEM;
    return -1;
  }

  alignas(struct cmsghdr) char cmsg_buf[cmsg_space];
  iovec iov = {.iov_base = const_cast<void*>(data), .iov_len = len};
  msghdr msg = {
      .msg_name = nullptr,
      .msg_namelen = 0,
      .msg_iov = &iov,
      .msg_iovlen = 1,
      .msg_control = cmsg_buf,
      // We can't cast to the actual type of the field, because it's different across platforms.
      .msg_controllen = static_cast<unsigned int>(cmsg_space),
      .msg_flags = 0,
  };

  struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = cmsg_len;

  int* cmsg_fds = reinterpret_cast<int*>(CMSG_DATA(cmsg));
  for (size_t i = 0; i < fds.size(); ++i) {
    cmsg_fds[i] = fds[i];
  }

#if defined(__linux__)
  int flags = MSG_NOSIGNAL;
#else
  int flags = 0;
#endif

  return TEMP_FAILURE_RETRY(sendmsg(sockfd.get(), &msg, flags));
}

ssize_t ReceiveFileDescriptorVector(borrowed_fd sockfd, void* data, size_t len, size_t max_fds,
                                    std::vector<unique_fd>* fds) {
  fds->clear();

  static const size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t cmsg_space = CMSG_SPACE(sizeof(int) * max_fds);
  if (cmsg_space >= page_size) {
    errno = ENOMEM;
    return -1;
  }

  alignas(struct cmsghdr) char cmsg_buf[cmsg_space];
  iovec iov = {.iov_base = const_cast<void*>(data), .iov_len = len};
  msghdr msg = {
      .msg_name = nullptr,
      .msg_namelen = 0,
      .msg_iov = &iov,
      .msg_iovlen = 1,
      .msg_control = cmsg_buf,
      // We can't cast to the actual type of the field, because it's different across platforms.
      .msg_controllen = static_cast<unsigned int>(cmsg_space),
      .msg_flags = 0,
  };

  int flags = MSG_TRUNC | MSG_CTRUNC;
#if defined(__linux__)
  flags |= MSG_CMSG_CLOEXEC | MSG_NOSIGNAL;
#endif

  ssize_t rc = TEMP_FAILURE_RETRY(recvmsg(sockfd.get(), &msg, flags));

  if (rc == -1) {
    return -1;
  }

  int error = 0;
  if ((msg.msg_flags & MSG_TRUNC)) {
    LOG(ERROR) << "message was truncated when receiving file descriptors";
    error = EMSGSIZE;
  } else if ((msg.msg_flags & MSG_CTRUNC)) {
    LOG(ERROR) << "control message was truncated when receiving file descriptors";
    error = EMSGSIZE;
  }

  std::vector<unique_fd> received_fds;
  struct cmsghdr* cmsg;
  for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
    if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
      LOG(ERROR) << "received unexpected cmsg: [" << cmsg->cmsg_level << ", " << cmsg->cmsg_type
                 << "]";
      error = EBADMSG;
      continue;
    }

    // There isn't a macro that does the inverse of CMSG_LEN, so hack around it ourselves, with
    // some asserts to ensure that CMSG_LEN behaves as we expect.
#if defined(__linux__)
#define CMSG_ASSERT static_assert
#else
// CMSG_LEN is somehow not constexpr on darwin.
#define CMSG_ASSERT CHECK
#endif
    CMSG_ASSERT(CMSG_LEN(0) + 1 * sizeof(int) == CMSG_LEN(1 * sizeof(int)));
    CMSG_ASSERT(CMSG_LEN(0) + 2 * sizeof(int) == CMSG_LEN(2 * sizeof(int)));
    CMSG_ASSERT(CMSG_LEN(0) + 3 * sizeof(int) == CMSG_LEN(3 * sizeof(int)));
    CMSG_ASSERT(CMSG_LEN(0) + 4 * sizeof(int) == CMSG_LEN(4 * sizeof(int)));

    if (cmsg->cmsg_len % sizeof(int) != 0) {
      LOG(FATAL) << "cmsg_len(" << cmsg->cmsg_len << ") not aligned to sizeof(int)";
    } else if (cmsg->cmsg_len <= CMSG_LEN(0)) {
      LOG(FATAL) << "cmsg_len(" << cmsg->cmsg_len << ") not long enough to hold any data";
    }

    int* cmsg_fds = reinterpret_cast<int*>(CMSG_DATA(cmsg));
    size_t cmsg_fdcount = static_cast<size_t>(cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
    for (size_t i = 0; i < cmsg_fdcount; ++i) {
#if !defined(__linux__)
      // Linux uses MSG_CMSG_CLOEXEC instead of doing this manually.
      fcntl(cmsg_fds[i], F_SETFD, FD_CLOEXEC);
#endif
      received_fds.emplace_back(cmsg_fds[i]);
    }
  }

  if (error != 0) {
    errno = error;
    return -1;
  }

  if (received_fds.size() > max_fds) {
    LOG(ERROR) << "received too many file descriptors, expected " << fds->size() << ", received "
               << received_fds.size();
    errno = EMSGSIZE;
    return -1;
  }

  *fds = std::move(received_fds);
  return rc;
}

}  // namespace base
}  // namespace android
