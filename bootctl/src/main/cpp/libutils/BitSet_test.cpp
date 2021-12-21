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

#define LOG_TAG "BitSet_test"

#include <unistd.h>

#include <android/log.h>
#include <gtest/gtest.h>
#include <utils/BitSet.h>

namespace android {

class BitSet32Test : public testing::Test {
protected:
    BitSet32 b1;
    BitSet32 b2;
    virtual void TearDown() {
        b1.clear();
        b2.clear();
    }
};


TEST_F(BitSet32Test, BitWiseOr) {
    b1.markBit(2);
    b2.markBit(4);

    BitSet32 tmp = b1 | b2;
    EXPECT_EQ(tmp.count(), 2u);
    EXPECT_TRUE(tmp.hasBit(2) && tmp.hasBit(4));
    // Check that the operator is symmetric
    EXPECT_TRUE((b2 | b1) == (b1 | b2));

    b1 |= b2;
    EXPECT_EQ(b1.count(), 2u);
    EXPECT_TRUE(b1.hasBit(2) && b1.hasBit(4));
    EXPECT_TRUE(b2.hasBit(4) && b2.count() == 1u);
}
TEST_F(BitSet32Test, BitWiseAnd_Disjoint) {
    b1.markBit(2);
    b1.markBit(4);
    b1.markBit(6);

    BitSet32 tmp = b1 & b2;
    EXPECT_TRUE(tmp.isEmpty());
    // Check that the operator is symmetric
    EXPECT_TRUE((b2 & b1) == (b1 & b2));

    b2 &= b1;
    EXPECT_TRUE(b2.isEmpty());
    EXPECT_EQ(b1.count(), 3u);
    EXPECT_TRUE(b1.hasBit(2) && b1.hasBit(4) && b1.hasBit(6));
}

TEST_F(BitSet32Test, BitWiseAnd_NonDisjoint) {
    b1.markBit(2);
    b1.markBit(4);
    b1.markBit(6);
    b2.markBit(3);
    b2.markBit(6);
    b2.markBit(9);

    BitSet32 tmp = b1 & b2;
    EXPECT_EQ(tmp.count(), 1u);
    EXPECT_TRUE(tmp.hasBit(6));
    // Check that the operator is symmetric
    EXPECT_TRUE((b2 & b1) == (b1 & b2));

    b1 &= b2;
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_EQ(b2.count(), 3u);
    EXPECT_TRUE(b2.hasBit(3) && b2.hasBit(6) && b2.hasBit(9));
}

TEST_F(BitSet32Test, MarkFirstUnmarkedBit) {
    b1.markBit(1);

    b1.markFirstUnmarkedBit();
    EXPECT_EQ(b1.count(), 2u);
    EXPECT_TRUE(b1.hasBit(0) && b1.hasBit(1));

    b1.markFirstUnmarkedBit();
    EXPECT_EQ(b1.count(), 3u);
    EXPECT_TRUE(b1.hasBit(0) && b1.hasBit(1) && b1.hasBit(2));
}

TEST_F(BitSet32Test, ClearFirstMarkedBit) {
    b1.markBit(0);
    b1.markBit(10);

    b1.clearFirstMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(10));

    b1.markBit(30);
    b1.clearFirstMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(30));
}

TEST_F(BitSet32Test, ClearLastMarkedBit) {
    b1.markBit(10);
    b1.markBit(31);

    b1.clearLastMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(10));

    b1.markBit(5);
    b1.clearLastMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(5));
}

TEST_F(BitSet32Test, FillAndClear) {
    EXPECT_TRUE(b1.isEmpty());
    for (size_t i = 0; i < 32; i++) {
        b1.markFirstUnmarkedBit();
    }
    EXPECT_TRUE(b1.isFull());
    b1.clear();
    EXPECT_TRUE(b1.isEmpty());
}

TEST_F(BitSet32Test, GetIndexOfBit) {
    b1.markBit(1);
    b1.markBit(4);
    EXPECT_EQ(0U, b1.getIndexOfBit(1));
    EXPECT_EQ(1U, b1.getIndexOfBit(4));
    b1.markFirstUnmarkedBit();
    EXPECT_EQ(1U, b1.getIndexOfBit(1));
    EXPECT_EQ(2U, b1.getIndexOfBit(4));
}

class BitSet64Test : public testing::Test {
protected:
    BitSet64 b1;
    BitSet64 b2;
    virtual void TearDown() {
        b1.clear();
        b2.clear();
    }
};


TEST_F(BitSet64Test, BitWiseOr) {
    b1.markBit(20);
    b2.markBit(40);

    BitSet64 tmp = b1 | b2;
    EXPECT_EQ(tmp.count(), 2u);
    EXPECT_TRUE(tmp.hasBit(20) && tmp.hasBit(40));
    // Check that the operator is symmetric
    EXPECT_TRUE((b2 | b1) == (b1 | b2));

    b1 |= b2;
    EXPECT_EQ(b1.count(), 2u);
    EXPECT_TRUE(b1.hasBit(20) && b1.hasBit(40));
    EXPECT_TRUE(b2.hasBit(40) && b2.count() == 1u);
}
TEST_F(BitSet64Test, BitWiseAnd_Disjoint) {
    b1.markBit(20);
    b1.markBit(40);
    b1.markBit(60);

    BitSet64 tmp = b1 & b2;
    EXPECT_TRUE(tmp.isEmpty());
    // Check that the operator is symmetric
    EXPECT_TRUE((b2 & b1) == (b1 & b2));

    b2 &= b1;
    EXPECT_TRUE(b2.isEmpty());
    EXPECT_EQ(b1.count(), 3u);
    EXPECT_TRUE(b1.hasBit(20) && b1.hasBit(40) && b1.hasBit(60));
}

TEST_F(BitSet64Test, BitWiseAnd_NonDisjoint) {
    b1.markBit(20);
    b1.markBit(40);
    b1.markBit(60);
    b2.markBit(30);
    b2.markBit(60);
    b2.markBit(63);

    BitSet64 tmp = b1 & b2;
    EXPECT_EQ(tmp.count(), 1u);
    EXPECT_TRUE(tmp.hasBit(60));
    // Check that the operator is symmetric
    EXPECT_TRUE((b2 & b1) == (b1 & b2));

    b1 &= b2;
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_EQ(b2.count(), 3u);
    EXPECT_TRUE(b2.hasBit(30) && b2.hasBit(60) && b2.hasBit(63));
}

TEST_F(BitSet64Test, MarkFirstUnmarkedBit) {
    b1.markBit(1);

    b1.markFirstUnmarkedBit();
    EXPECT_EQ(b1.count(), 2u);
    EXPECT_TRUE(b1.hasBit(0) && b1.hasBit(1));

    b1.markFirstUnmarkedBit();
    EXPECT_EQ(b1.count(), 3u);
    EXPECT_TRUE(b1.hasBit(0) && b1.hasBit(1) && b1.hasBit(2));
}

TEST_F(BitSet64Test, ClearFirstMarkedBit) {
    b1.markBit(0);
    b1.markBit(10);

    b1.clearFirstMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(10));

    b1.markBit(50);
    b1.clearFirstMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(50));
}

TEST_F(BitSet64Test, ClearLastMarkedBit) {
    b1.markBit(10);
    b1.markBit(63);

    b1.clearLastMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(10));

    b1.markBit(5);
    b1.clearLastMarkedBit();
    EXPECT_EQ(b1.count(), 1u);
    EXPECT_TRUE(b1.hasBit(5));
}

TEST_F(BitSet64Test, FillAndClear) {
    EXPECT_TRUE(b1.isEmpty());
    for (size_t i = 0; i < 64; i++) {
        b1.markFirstUnmarkedBit();
    }
    EXPECT_TRUE(b1.isFull());
    b1.clear();
    EXPECT_TRUE(b1.isEmpty());
}

TEST_F(BitSet64Test, GetIndexOfBit) {
    b1.markBit(10);
    b1.markBit(40);
    EXPECT_EQ(0U, b1.getIndexOfBit(10));
    EXPECT_EQ(1U, b1.getIndexOfBit(40));
    b1.markFirstUnmarkedBit();
    EXPECT_EQ(1U, b1.getIndexOfBit(10));
    EXPECT_EQ(2U, b1.getIndexOfBit(40));
}

} // namespace android
