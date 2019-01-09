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

#ifndef ERRNO_RESTORER_H
#define ERRNO_RESTORER_H

#include <errno.h>

#include "bionic_macros.h"

class ErrnoRestorer {
 public:
  explicit ErrnoRestorer() : saved_errno_(errno) {
  }

  ~ErrnoRestorer() {
    errno = saved_errno_;
  }

  void override(int new_errno) {
    saved_errno_ = new_errno;
  }

 private:
  int saved_errno_;

  DISALLOW_COPY_AND_ASSIGN(ErrnoRestorer);
};

#endif // ERRNO_RESTORER_H
