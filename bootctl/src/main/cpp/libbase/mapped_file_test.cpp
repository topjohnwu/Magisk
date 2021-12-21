/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "android-base/mapped_file.h"

#include <gtest/gtest.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#include "android-base/file.h"

TEST(mapped_file, smoke) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);
  ASSERT_TRUE(android::base::WriteStringToFd("hello world", tf.fd));

  auto m = android::base::MappedFile::FromFd(tf.fd, 3, 2, PROT_READ);
  ASSERT_EQ(2u, m->size());
  ASSERT_EQ('l', m->data()[0]);
  ASSERT_EQ('o', m->data()[1]);
}

TEST(mapped_file, zero_length_mapping) {
  // http://b/119818070 "app crashes when reading asset of zero length".
  // mmap fails with EINVAL for a zero length region.
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);

  auto m = android::base::MappedFile::FromFd(tf.fd, 4096, 0, PROT_READ);
  EXPECT_EQ(0u, m->size());
  EXPECT_NE(nullptr, m->data());
}
