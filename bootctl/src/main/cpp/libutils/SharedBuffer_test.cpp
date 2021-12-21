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

#define __STDC_LIMIT_MACROS

#include <gtest/gtest.h>

#include <memory>
#include <stdint.h>

#include "SharedBuffer.h"

extern "C" void __hwasan_init() __attribute__((weak));
#define SKIP_WITH_HWASAN \
    if (&__hwasan_init != 0) GTEST_SKIP()

TEST(SharedBufferTest, alloc_death) {
    EXPECT_DEATH(android::SharedBuffer::alloc(SIZE_MAX), "");
    EXPECT_DEATH(android::SharedBuffer::alloc(SIZE_MAX - sizeof(android::SharedBuffer)), "");
}

TEST(SharedBufferTest, alloc_max) {
    SKIP_WITH_HWASAN;  // hwasan has a 2GiB allocation limit.

    android::SharedBuffer* buf =
            android::SharedBuffer::alloc(SIZE_MAX - sizeof(android::SharedBuffer) - 1);
    if (buf != nullptr) {
        EXPECT_NE(nullptr, buf->data());
        buf->release();
    }
}

TEST(SharedBufferTest, alloc_big) {
    SKIP_WITH_HWASAN;  // hwasan has a 2GiB allocation limit.

    android::SharedBuffer* buf = android::SharedBuffer::alloc(SIZE_MAX / 2);
    if (buf != nullptr) {
        EXPECT_NE(nullptr, buf->data());
        buf->release();
    }
}

TEST(SharedBufferTest, alloc_zero_size) {
    android::SharedBuffer* buf = android::SharedBuffer::alloc(0);
    ASSERT_NE(nullptr, buf);
    ASSERT_EQ(0U, buf->size());
    buf->release();
}

TEST(SharedBufferTest, editResize_death) {
    android::SharedBuffer* buf = android::SharedBuffer::alloc(10);
    EXPECT_DEATH(buf->editResize(SIZE_MAX - sizeof(android::SharedBuffer)), "");
    buf = android::SharedBuffer::alloc(10);
    EXPECT_DEATH(buf->editResize(SIZE_MAX), "");
}

TEST(SharedBufferTest, editResize_null) {
    // Big enough to fail, not big enough to abort.
    SKIP_WITH_HWASAN;  // hwasan has a 2GiB allocation limit.
    android::SharedBuffer* buf = android::SharedBuffer::alloc(10);
    android::SharedBuffer* buf2 = buf->editResize(SIZE_MAX / 2);
    if (buf2 == nullptr) {
        buf->release();
    } else {
        EXPECT_NE(nullptr, buf2->data());
        buf2->release();
    }
}

TEST(SharedBufferTest, editResize_zero_size) {
    android::SharedBuffer* buf = android::SharedBuffer::alloc(10);
    buf = buf->editResize(0);
    ASSERT_EQ(0U, buf->size());
    buf->release();
}
