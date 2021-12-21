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

#include <gtest/gtest.h>

#include <utils/StrongPointer.h>
#include <utils/RefBase.h>

using namespace android;

class SPFoo : virtual public RefBase {
  public:
    explicit SPFoo(bool* deleted_check) : mDeleted(deleted_check) {
        *mDeleted = false;
    }

    ~SPFoo() {
        *mDeleted = true;
    }

  private:
    bool* mDeleted;
};

class SPLightFoo : virtual public VirtualLightRefBase {
  public:
    explicit SPLightFoo(bool* deleted_check) : mDeleted(deleted_check) { *mDeleted = false; }

    ~SPLightFoo() { *mDeleted = true; }

  private:
    bool* mDeleted;
};

template <typename T>
class StrongPointer : public ::testing::Test {};

using RefBaseTypes = ::testing::Types<SPFoo, SPLightFoo>;
TYPED_TEST_CASE(StrongPointer, RefBaseTypes);

TYPED_TEST(StrongPointer, move) {
    bool isDeleted;
    sp<TypeParam> sp1 = sp<TypeParam>::make(&isDeleted);
    TypeParam* foo = sp1.get();
    ASSERT_EQ(1, foo->getStrongCount());
    {
        sp<TypeParam> sp2 = std::move(sp1);
        ASSERT_EQ(1, foo->getStrongCount()) << "std::move failed, incremented refcnt";
        ASSERT_EQ(nullptr, sp1.get()) << "std::move failed, sp1 is still valid";
        // The strong count isn't increasing, let's double check the old object
        // is properly reset and doesn't early delete
        sp1 = std::move(sp2);
    }
    ASSERT_FALSE(isDeleted) << "deleted too early! still has a reference!";
    {
        // Now let's double check it deletes on time
        sp<TypeParam> sp2 = std::move(sp1);
    }
    ASSERT_TRUE(isDeleted) << "foo was leaked!";
}

TYPED_TEST(StrongPointer, NullptrComparison) {
    sp<TypeParam> foo;
    ASSERT_EQ(foo, nullptr);
    ASSERT_EQ(nullptr, foo);
}

TYPED_TEST(StrongPointer, PointerComparison) {
    bool isDeleted;
    sp<TypeParam> foo = sp<TypeParam>::make(&isDeleted);
    ASSERT_EQ(foo.get(), foo);
    ASSERT_EQ(foo, foo.get());
    ASSERT_NE(nullptr, foo);
    ASSERT_NE(foo, nullptr);
}

TYPED_TEST(StrongPointer, Deleted) {
    bool isDeleted;
    sp<TypeParam> foo = sp<TypeParam>::make(&isDeleted);

    auto foo2 = sp<TypeParam>::fromExisting(foo.get());

    EXPECT_FALSE(isDeleted);
    foo = nullptr;
    EXPECT_FALSE(isDeleted);
    foo2 = nullptr;
    EXPECT_TRUE(isDeleted);
}

TYPED_TEST(StrongPointer, AssertStrongRefExists) {
    bool isDeleted;
    TypeParam* foo = new TypeParam(&isDeleted);
    EXPECT_DEATH(sp<TypeParam>::fromExisting(foo), "");
    delete foo;
}
