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

#include <errno.h>
#include <linux/fs.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <android-base/macros.h>
#include <android-base/unique_fd.h>
#include <cutils/ashmem.h>
#include <gtest/gtest.h>

using android::base::unique_fd;

void TestCreateRegion(size_t size, unique_fd &fd, int prot) {
    fd = unique_fd(ashmem_create_region(nullptr, size));
    ASSERT_TRUE(fd >= 0);
    ASSERT_TRUE(ashmem_valid(fd));
    ASSERT_EQ(size, static_cast<size_t>(ashmem_get_size_region(fd)));
    ASSERT_EQ(0, ashmem_set_prot_region(fd, prot));

    // We've been inconsistent historically about whether or not these file
    // descriptors were CLOEXEC. Make sure we're consistent going forward.
    // https://issuetracker.google.com/165667331
    ASSERT_EQ(FD_CLOEXEC, (fcntl(fd, F_GETFD) & FD_CLOEXEC));
}

void TestMmap(const unique_fd& fd, size_t size, int prot, void** region, off_t off = 0) {
    ASSERT_TRUE(fd >= 0);
    ASSERT_TRUE(ashmem_valid(fd));
    *region = mmap(nullptr, size, prot, MAP_SHARED, fd, off);
    ASSERT_NE(MAP_FAILED, *region);
}

void TestProtDenied(const unique_fd &fd, size_t size, int prot) {
    ASSERT_TRUE(fd >= 0);
    ASSERT_TRUE(ashmem_valid(fd));
    EXPECT_EQ(MAP_FAILED, mmap(nullptr, size, prot, MAP_SHARED, fd, 0));
}

void TestProtIs(const unique_fd& fd, int prot) {
    ASSERT_TRUE(fd >= 0);
    ASSERT_TRUE(ashmem_valid(fd));
    EXPECT_EQ(prot, ioctl(fd, ASHMEM_GET_PROT_MASK));
}

void FillData(uint8_t* data, size_t dataLen) {
    for (size_t i = 0; i < dataLen; i++) {
        data[i] = i & 0xFF;
    }
}

TEST(AshmemTest, BasicTest) {
    constexpr size_t size = PAGE_SIZE;
    uint8_t data[size];
    FillData(data, size);

    unique_fd fd;
    ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_READ | PROT_WRITE));

    void *region1;
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, size, PROT_READ | PROT_WRITE, &region1));

    memcpy(region1, &data, size);
    ASSERT_EQ(0, memcmp(region1, &data, size));

    EXPECT_EQ(0, munmap(region1, size));

    void *region2;
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, size, PROT_READ, &region2));
    ASSERT_EQ(0, memcmp(region2, &data, size));
    EXPECT_EQ(0, munmap(region2, size));
}

TEST(AshmemTest, ForkTest) {
    constexpr size_t size = PAGE_SIZE;
    uint8_t data[size];
    FillData(data, size);

    unique_fd fd;
    ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_READ | PROT_WRITE));

    void *region1;
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, size, PROT_READ | PROT_WRITE, &region1));

    memcpy(region1, &data, size);
    ASSERT_EQ(0, memcmp(region1, &data, size));
    EXPECT_EQ(0, munmap(region1, size));

    ASSERT_EXIT(
        {
            if (!ashmem_valid(fd)) {
                _exit(3);
            }
            void* region2 = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (region2 == MAP_FAILED) {
                _exit(1);
            }
            if (memcmp(region2, &data, size) != 0) {
                _exit(2);
            }
            memset(region2, 0, size);
            munmap(region2, size);
            _exit(0);
        },
        ::testing::ExitedWithCode(0), "");

    memset(&data, 0, size);
    void *region2;
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, size, PROT_READ | PROT_WRITE, &region2));
    ASSERT_EQ(0, memcmp(region2, &data, size));
    EXPECT_EQ(0, munmap(region2, size));
}

TEST(AshmemTest, FileOperationsTest) {
    unique_fd fd;
    void* region;

    // Allocate a 4-page buffer, but leave page-sized holes on either side
    constexpr size_t size = PAGE_SIZE * 4;
    constexpr size_t dataSize = PAGE_SIZE * 2;
    constexpr size_t holeSize = PAGE_SIZE;
    ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_READ | PROT_WRITE));
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, dataSize, PROT_READ | PROT_WRITE, &region, holeSize));

    uint8_t data[dataSize];
    FillData(data, dataSize);
    memcpy(region, data, dataSize);

    constexpr off_t dataStart = holeSize;
    constexpr off_t dataEnd = dataStart + dataSize;

    // The sequence of seeks below looks something like this:
    //
    // [    ][data][data][    ]
    // --^                          lseek(99, SEEK_SET)
    //   ------^                    lseek(dataStart, SEEK_CUR)
    // ------^                      lseek(0, SEEK_DATA)
    //       ------------^          lseek(dataStart, SEEK_HOLE)
    //                      ^--     lseek(-99, SEEK_END)
    //                ^------       lseek(-dataStart, SEEK_CUR)
    const struct {
        // lseek() parameters
        off_t offset;
        int whence;
        // Expected lseek() return value
        off_t ret;
    } seeks[] = {
        {99, SEEK_SET, 99},         {dataStart, SEEK_CUR, dataStart + 99},
        {0, SEEK_DATA, dataStart},  {dataStart, SEEK_HOLE, dataEnd},
        {-99, SEEK_END, size - 99}, {-dataStart, SEEK_CUR, dataEnd - 99},
    };
    for (const auto& cfg : seeks) {
        errno = 0;
        ASSERT_TRUE(ashmem_valid(fd));
        auto off = lseek(fd, cfg.offset, cfg.whence);
        ASSERT_EQ(cfg.ret, off) << "lseek(" << cfg.offset << ", " << cfg.whence << ") failed"
                                << (errno ? ": " : "") << (errno ? strerror(errno) : "");

        if (off >= dataStart && off < dataEnd) {
            off_t dataOff = off - dataStart;
            ssize_t readSize = dataSize - dataOff;
            uint8_t buf[readSize];

            ASSERT_EQ(readSize, TEMP_FAILURE_RETRY(read(fd, buf, readSize)));
            EXPECT_EQ(0, memcmp(buf, data + dataOff, readSize));
        }
    }

    EXPECT_EQ(0, munmap(region, dataSize));
}

TEST(AshmemTest, ProtTest) {
    unique_fd fd;
    constexpr size_t size = PAGE_SIZE;
    void *region;

    ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_READ));
    TestProtDenied(fd, size, PROT_WRITE);
    TestProtIs(fd, PROT_READ);
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, size, PROT_READ, &region));
    EXPECT_EQ(0, munmap(region, size));

    ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_WRITE));
    TestProtDenied(fd, size, PROT_READ);
    TestProtIs(fd, PROT_WRITE);
    ASSERT_NO_FATAL_FAILURE(TestMmap(fd, size, PROT_WRITE, &region));
    EXPECT_EQ(0, munmap(region, size));

    ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_READ | PROT_WRITE));
    TestProtIs(fd, PROT_READ | PROT_WRITE);
    ASSERT_EQ(0, ashmem_set_prot_region(fd, PROT_READ));
    errno = 0;
    ASSERT_EQ(-1, ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE))
        << "kernel shouldn't allow adding protection bits";
    EXPECT_EQ(EINVAL, errno);
    TestProtIs(fd, PROT_READ);
    TestProtDenied(fd, size, PROT_WRITE);
}

TEST(AshmemTest, ForkProtTest) {
    unique_fd fd;
    constexpr size_t size = PAGE_SIZE;

    int protFlags[] = { PROT_READ, PROT_WRITE };
    for (size_t i = 0; i < arraysize(protFlags); i++) {
        ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd, PROT_READ | PROT_WRITE));
        ASSERT_EXIT(
            {
                if (!ashmem_valid(fd)) {
                    _exit(3);
                } else if (ashmem_set_prot_region(fd, protFlags[i]) >= 0) {
                    _exit(0);
                } else {
                    _exit(1);
                }
            },
            ::testing::ExitedWithCode(0), "");
        ASSERT_NO_FATAL_FAILURE(TestProtDenied(fd, size, protFlags[1-i]));
    }
}

TEST(AshmemTest, ForkMultiRegionTest) {
    constexpr size_t size = PAGE_SIZE;
    uint8_t data[size];
    FillData(data, size);

    constexpr int nRegions = 16;
    unique_fd fd[nRegions];
    for (int i = 0; i < nRegions; i++) {
        ASSERT_NO_FATAL_FAILURE(TestCreateRegion(size, fd[i], PROT_READ | PROT_WRITE));
        void *region;
        ASSERT_NO_FATAL_FAILURE(TestMmap(fd[i], size, PROT_READ | PROT_WRITE, &region));
        memcpy(region, &data, size);
        ASSERT_EQ(0, memcmp(region, &data, size));
        EXPECT_EQ(0, munmap(region, size));
    }

    ASSERT_EXIT({
        for (int i = 0; i < nRegions; i++) {
            if (!ashmem_valid(fd[i])) {
                _exit(3);
            }
            void *region = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd[i], 0);
            if (region == MAP_FAILED) {
                _exit(1);
            }
            if (memcmp(region, &data, size) != 0) {
                munmap(region, size);
                _exit(2);
            }
            memset(region, 0, size);
            munmap(region, size);
        }
        _exit(0);
    }, ::testing::ExitedWithCode(0), "");

    memset(&data, 0, size);
    for (int i = 0; i < nRegions; i++) {
        void *region;
        ASSERT_NO_FATAL_FAILURE(TestMmap(fd[i], size, PROT_READ | PROT_WRITE, &region));
        ASSERT_EQ(0, memcmp(region, &data, size));
        EXPECT_EQ(0, munmap(region, size));
    }
}
