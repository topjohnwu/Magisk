/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "Vector_test"

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <unistd.h>

#include <android/log.h>
#include <gtest/gtest.h>
#include <utils/Vector.h>

namespace android {

class VectorTest : public testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

public:
};


TEST_F(VectorTest, CopyOnWrite_CopyAndAddElements) {

    Vector<int> vector;
    Vector<int> other;
    vector.setCapacity(8);

    vector.add(1);
    vector.add(2);
    vector.add(3);

    EXPECT_EQ(3U, vector.size());

    // copy the vector
    other = vector;

    EXPECT_EQ(3U, other.size());

    // add an element to the first vector
    vector.add(4);

    // make sure the sizes are correct
    EXPECT_EQ(4U, vector.size());
    EXPECT_EQ(3U, other.size());

    // add an element to the copy
    other.add(5);

    // make sure the sizes are correct
    EXPECT_EQ(4U, vector.size());
    EXPECT_EQ(4U, other.size());

    // make sure the content of both vectors are correct
    EXPECT_EQ(vector[3], 4);
    EXPECT_EQ(other[3], 5);
}

TEST_F(VectorTest, SetCapacity_Overflow) {
  Vector<int> vector;
  EXPECT_DEATH(vector.setCapacity(SIZE_MAX / sizeof(int) + 1), "Assertion failed");
}

TEST_F(VectorTest, SetCapacity_ShrinkBelowSize) {
  Vector<int> vector;
  vector.add(1);
  vector.add(2);
  vector.add(3);
  vector.add(4);

  vector.setCapacity(8);
  ASSERT_EQ(8U, vector.capacity());
  vector.setCapacity(2);
  ASSERT_EQ(8U, vector.capacity());
}

TEST_F(VectorTest, _grow_OverflowSize) {
  Vector<int> vector;
  vector.add(1);

  // Checks that the size calculation (not the capacity calculation) doesn't
  // overflow : the size here will be (1 + SIZE_MAX).
  EXPECT_DEATH(vector.insertArrayAt(nullptr, 0, SIZE_MAX), "new_size overflow");
}

TEST_F(VectorTest, _grow_OverflowCapacityDoubling) {
  Vector<int> vector;

  // This should fail because the calculated capacity will overflow even though
  // the size of the vector doesn't.
  EXPECT_DEATH(vector.insertArrayAt(nullptr, 0, (SIZE_MAX - 1)), "new_capacity overflow");
}

TEST_F(VectorTest, _grow_OverflowBufferAlloc) {
  Vector<int> vector;
  // This should fail because the capacity * sizeof(int) overflows, even
  // though the capacity itself doesn't.
  EXPECT_DEATH(vector.insertArrayAt(nullptr, 0, (SIZE_MAX / 2)), "new_alloc_size overflow");
}

TEST_F(VectorTest, editArray_Shared) {
  Vector<int> vector1;
  vector1.add(1);
  vector1.add(2);
  vector1.add(3);
  vector1.add(4);

  Vector<int> vector2 = vector1;
  ASSERT_EQ(vector1.array(), vector2.array());
  // We must make a copy here, since we're not the exclusive owners
  // of this array.
  ASSERT_NE(vector1.editArray(), vector2.editArray());

  // Vector doesn't implement operator ==.
  ASSERT_EQ(vector1.size(), vector2.size());
  for (size_t i = 0; i < vector1.size(); ++i) {
    EXPECT_EQ(vector1[i], vector2[i]);
  }
}

} // namespace android
