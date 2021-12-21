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

#pragma once

#include <sys/stat.h>
#include <sys/types.h>

#include <type_traits>
#include <vector>

#include <android-base/collections.h>
#include <android-base/macros.h>
#include <android-base/unique_fd.h>

namespace android {
namespace base {

#if !defined(_WIN32)

// Helpers for sending and receiving file descriptors across Unix domain sockets.
//
// The cmsg(3) API is very hard to get right, with multiple landmines that can
// lead to death. Almost all of the uses of cmsg in Android make at least one of
// the following mistakes:
//
//   - not aligning the cmsg buffer
//   - leaking fds if more fds are received than expected
//   - blindly dereferencing CMSG_DATA without checking the header
//   - using CMSG_SPACE instead of CMSG_LEN for .cmsg_len
//   - using CMSG_LEN instead of CMSG_SPACE for .msg_controllen
//   - using a length specified in number of fds instead of bytes
//
// These functions wrap the hard-to-use cmsg API with an easier to use abstraction.

// Send file descriptors across a Unix domain socket.
//
// Note that the write can return short if the socket type is SOCK_STREAM. When
// this happens, file descriptors are still sent to the other end, but with
// truncated data. For this reason, using SOCK_SEQPACKET or SOCK_DGRAM is recommended.
ssize_t SendFileDescriptorVector(borrowed_fd sock, const void* data, size_t len,
                                 const std::vector<int>& fds);

// Receive file descriptors from a Unix domain socket.
//
// If more FDs (or bytes, for datagram sockets) are received than expected,
// -1 is returned with errno set to EMSGSIZE, and all received FDs are thrown away.
ssize_t ReceiveFileDescriptorVector(borrowed_fd sock, void* data, size_t len, size_t max_fds,
                                    std::vector<android::base::unique_fd>* fds);

// Helper for SendFileDescriptorVector that constructs a std::vector for you, e.g.:
//   SendFileDescriptors(sock, "foo", 3, std::move(fd1), std::move(fd2))
template <typename... Args>
ssize_t SendFileDescriptors(borrowed_fd sock, const void* data, size_t len, Args&&... sent_fds) {
  // Do not allow implicit conversion to int: people might try to do something along the lines of:
  //   SendFileDescriptors(..., std::move(a_unique_fd))
  // and be surprised when the unique_fd isn't closed afterwards.
  AssertType<int>(std::forward<Args>(sent_fds)...);
  std::vector<int> fds;
  Append(fds, std::forward<Args>(sent_fds)...);
  return SendFileDescriptorVector(sock, data, len, fds);
}

// Helper for ReceiveFileDescriptorVector that receives an exact number of file descriptors.
// If more file descriptors are received than requested, -1 is returned with errno set to EMSGSIZE.
// If fewer file descriptors are received than requested, -1 is returned with errno set to ENOMSG.
// In both cases, all arguments are cleared and any received FDs are thrown away.
template <typename... Args>
ssize_t ReceiveFileDescriptors(borrowed_fd sock, void* data, size_t len, Args&&... received_fds) {
  std::vector<unique_fd*> fds;
  Append(fds, std::forward<Args>(received_fds)...);

  std::vector<unique_fd> result;
  ssize_t rc = ReceiveFileDescriptorVector(sock, data, len, fds.size(), &result);
  if (rc == -1 || result.size() != fds.size()) {
    int err = rc == -1 ? errno : ENOMSG;
    for (unique_fd* fd : fds) {
      fd->reset();
    }
    errno = err;
    return -1;
  }

  for (size_t i = 0; i < fds.size(); ++i) {
    *fds[i] = std::move(result[i]);
  }
  return rc;
}

#endif

}  // namespace base
}  // namespace android
