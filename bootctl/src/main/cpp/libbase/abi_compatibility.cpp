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

#include <memory>

#include "android-base/cmsg.h"
#include "android-base/file.h"
#include "android-base/mapped_file.h"
#include "android-base/unique_fd.h"

namespace android {
namespace base {

// These ABI-compatibility shims are in a separate file for two reasons:
//   1. If they were in the file with the actual functions, it prevents calls to
//      those functions by other functions in the file, due to ambiguity.
//   2. We will hopefully be able to delete these quickly.

#if !defined(_WIN32)
ssize_t SendFileDescriptorVector(int sockfd, const void* data, size_t len,
                                 const std::vector<int>& fds) {
  return SendFileDescriptorVector(borrowed_fd(sockfd), data, len, fds);
}

ssize_t ReceiveFileDescriptorVector(int sockfd, void* data, size_t len, size_t max_fds,
                                    std::vector<unique_fd>* fds) {
  return ReceiveFileDescriptorVector(borrowed_fd(sockfd), data, len, max_fds, fds);
}
#endif

bool ReadFdToString(int fd, std::string* content) {
  return ReadFdToString(borrowed_fd(fd), content);
}

bool WriteStringToFd(const std::string& content, int fd) {
  return WriteStringToFd(content, borrowed_fd(fd));
}

bool ReadFully(int fd, void* data, size_t byte_count) {
  return ReadFully(borrowed_fd(fd), data, byte_count);
}

bool ReadFullyAtOffset(int fd, void* data, size_t byte_count, off64_t offset) {
  return ReadFullyAtOffset(borrowed_fd(fd), data, byte_count, offset);
}

bool WriteFully(int fd, const void* data, size_t byte_count) {
  return WriteFully(borrowed_fd(fd), data, byte_count);
}

#if defined(__LP64__)
#define MAPPEDFILE_FROMFD _ZN10MappedFile6FromFdEilmi
#else
#define MAPPEDFILE_FROMFD _ZN10MappedFile6FromFdEixmi
#endif

#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C" std::unique_ptr<MappedFile> MAPPEDFILE_FROMFD(int fd, off64_t offset, size_t length,
                                                         int prot) {
  return MappedFile::FromFd(fd, offset, length, prot);
}

}  // namespace base
}  // namespace android
