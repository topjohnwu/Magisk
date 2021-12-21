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

#include "android-base/test_utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include <android-base/file.h>
#include <android-base/logging.h>

CapturedStdFd::CapturedStdFd(int std_fd) : std_fd_(std_fd), old_fd_(-1) {
  Start();
}

CapturedStdFd::~CapturedStdFd() {
  if (old_fd_ != -1) {
    Stop();
  }
}

int CapturedStdFd::fd() const {
  return temp_file_.fd;
}

std::string CapturedStdFd::str() {
  std::string result;
  CHECK_EQ(0, TEMP_FAILURE_RETRY(lseek(fd(), 0, SEEK_SET)));
  android::base::ReadFdToString(fd(), &result);
  return result;
}

void CapturedStdFd::Reset() {
  // Do not reset while capturing.
  CHECK_EQ(-1, old_fd_);
  CHECK_EQ(0, TEMP_FAILURE_RETRY(lseek(fd(), 0, SEEK_SET)));
  CHECK_EQ(0, ftruncate(fd(), 0));
}

void CapturedStdFd::Start() {
#if defined(_WIN32)
  // On Windows, stderr is often buffered, so make sure it is unbuffered so
  // that we can immediately read back what was written to stderr.
  if (std_fd_ == STDERR_FILENO) CHECK_EQ(0, setvbuf(stderr, nullptr, _IONBF, 0));
#endif
  old_fd_ = dup(std_fd_);
  CHECK_NE(-1, old_fd_);
  CHECK_NE(-1, dup2(fd(), std_fd_));
}

void CapturedStdFd::Stop() {
  CHECK_NE(-1, old_fd_);
  CHECK_NE(-1, dup2(old_fd_, std_fd_));
  close(old_fd_);
  old_fd_ = -1;
  // Note: cannot restore prior setvbuf() setting.
}
