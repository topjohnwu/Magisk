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

#include <android-base/logging.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <gtest/gtest.h>
#include <hidl/GtestPrinter.h>
#include <hidl/ServiceManagement.h>
#include <hidlmemory/mapping.h>

using ::android::sp;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;

#define ASSERT_OK(ret) ASSERT_TRUE((ret).isOk())
#define EXPECT_OK(ret) EXPECT_TRUE((ret).isOk())

class AllocatorHidlTest : public ::testing::TestWithParam<std::string> {
  public:
    virtual void SetUp() override {
        allocator = IAllocator::getService(GetParam());
        ASSERT_NE(allocator, nullptr);
    }

    sp<IMemory> expectAllocateSuccess(size_t size) {
        sp<IMemory> memory;
        EXPECT_OK(allocator->allocate(size, [&](bool success, const hidl_memory& mem) {
            ASSERT_TRUE(success) << "Allocate failed for size: " << size;
            EXPECT_EQ(mem.size(), size)
                << "Allocated " << size << " but got hidl_memory with size " << mem.size();
            memory = mapMemory(mem);
        }));
        EXPECT_NE(nullptr, memory.get());
        EXPECT_EQ(memory->getSize(), size)
            << "Allocated " << size << " but got IMemory with size " << memory->getSize();
        return memory;
    }

    std::vector<sp<IMemory>> expectBatchAllocateSuccess(size_t size, size_t count) {
        std::vector<sp<IMemory>> memories;
        memories.reserve(count);
        EXPECT_OK(allocator->batchAllocate(
            size, count, [&](bool success, const hidl_vec<hidl_memory>& mems) {
                EXPECT_EQ(count, mems.size());

                for (const hidl_memory& mem : mems) {
                    ASSERT_TRUE(success) << "Allocate failed for size: " << size;
                    EXPECT_EQ(mem.size(), size)
                        << "Allocated " << size << " but got hidl_memory with size " << mem.size();
                    memories.push_back(mapMemory(mem));
                }
            }));
        for (const sp<IMemory>& memory : memories) {
            EXPECT_NE(nullptr, memory.get());
            EXPECT_EQ(memory->getSize(), size)
                << "Allocated " << size << " but got IMemory with size " << memory->getSize();
        }
        return memories;
    }

    sp<IAllocator> allocator;
};

TEST_P(AllocatorHidlTest, TestAllocateSizes) {
    for (size_t size : {1, 1023, 1024, 1025, 4096}) {
        expectAllocateSuccess(size);
    }
}

TEST_P(AllocatorHidlTest, TestBatchAllocateSizes) {
    for (size_t count : {1, 1, 2, 3, 10}) {
        for (size_t size : {1, 1023, 1024, 1025, 4096}) {
            expectBatchAllocateSuccess(size, count);
        }
    }
}

TEST_P(AllocatorHidlTest, TestCommit) {
    constexpr size_t kSize = 1337;

    sp<IMemory> memory = expectAllocateSuccess(kSize);
    for (int i = 0; i < 100; i++) {
        EXPECT_OK(memory->read());
        EXPECT_OK(memory->update());
        EXPECT_OK(memory->commit());

        EXPECT_OK(memory->read());
        EXPECT_OK(memory->commit());

        EXPECT_OK(memory->update());
        EXPECT_OK(memory->commit());
    }

    for (int i = 0; i < kSize; i++) {
        EXPECT_OK(memory->readRange(i, kSize));
        EXPECT_OK(memory->updateRange(i, kSize));
        EXPECT_OK(memory->commit());

        EXPECT_OK(memory->readRange(i, kSize));
        EXPECT_OK(memory->commit());

        EXPECT_OK(memory->updateRange(i, kSize));
        EXPECT_OK(memory->commit());
    }
}

INSTANTIATE_TEST_SUITE_P(
        PerInstance, AllocatorHidlTest,
        testing::ValuesIn(android::hardware::getAllHalInstanceNames(IAllocator::descriptor)),
        android::hardware::PrintInstanceNameToString);
