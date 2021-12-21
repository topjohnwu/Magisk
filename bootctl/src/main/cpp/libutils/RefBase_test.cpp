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

#include <gtest/gtest.h>

#include <utils/StrongPointer.h>
#include <utils/RefBase.h>

#include <thread>
#include <atomic>
#include <sched.h>
#include <errno.h>

// Enhanced version of StrongPointer_test, but using RefBase underneath.

using namespace android;

static constexpr int NITERS = 1000000;

static constexpr int INITIAL_STRONG_VALUE = 1 << 28;  // Mirroring RefBase definition.

class Foo : public RefBase {
public:
    Foo(bool* deleted_check) : mDeleted(deleted_check) {
        *mDeleted = false;
    }

    ~Foo() {
        *mDeleted = true;
    }
private:
    bool* mDeleted;
};

// A version of Foo that ensures that all objects are allocated at the same
// address. No more than one can be allocated at a time. Thread-hostile.
class FooFixedAlloc : public RefBase {
public:
    static void* operator new(size_t size) {
        if (mAllocCount != 0) {
            abort();
        }
        mAllocCount = 1;
        if (theMemory == nullptr) {
            theMemory = malloc(size);
        }
        return theMemory;
    }

    static void operator delete(void *p) {
        if (mAllocCount != 1 || p != theMemory) {
            abort();
        }
        mAllocCount = 0;
    }

    FooFixedAlloc(bool* deleted_check) : mDeleted(deleted_check) {
        *mDeleted = false;
    }

    ~FooFixedAlloc() {
        *mDeleted = true;
    }
private:
    bool* mDeleted;
    static int mAllocCount;
    static void* theMemory;
};

int FooFixedAlloc::mAllocCount(0);
void* FooFixedAlloc::theMemory(nullptr);

TEST(RefBase, StrongMoves) {
    bool isDeleted;
    Foo* foo = new Foo(&isDeleted);
    ASSERT_EQ(INITIAL_STRONG_VALUE, foo->getStrongCount());
    ASSERT_FALSE(isDeleted) << "Already deleted...?";
    sp<Foo> sp1(foo);
    wp<Foo> wp1(sp1);
    ASSERT_EQ(1, foo->getStrongCount());
    // Weak count includes both strong and weak references.
    ASSERT_EQ(2, foo->getWeakRefs()->getWeakCount());
    {
        sp<Foo> sp2 = std::move(sp1);
        ASSERT_EQ(1, foo->getStrongCount())
                << "std::move failed, incremented refcnt";
        ASSERT_EQ(nullptr, sp1.get()) << "std::move failed, sp1 is still valid";
        // The strong count isn't increasing, let's double check the old object
        // is properly reset and doesn't early delete
        sp1 = std::move(sp2);
    }
    ASSERT_FALSE(isDeleted) << "deleted too early! still has a reference!";
    {
        // Now let's double check it deletes on time
        sp<Foo> sp2 = std::move(sp1);
    }
    ASSERT_TRUE(isDeleted) << "foo was leaked!";
    ASSERT_TRUE(wp1.promote().get() == nullptr);
}

TEST(RefBase, WeakCopies) {
    bool isDeleted;
    Foo* foo = new Foo(&isDeleted);
    EXPECT_EQ(0, foo->getWeakRefs()->getWeakCount());
    ASSERT_FALSE(isDeleted) << "Foo (weak) already deleted...?";
    wp<Foo> wp1(foo);
    EXPECT_EQ(1, foo->getWeakRefs()->getWeakCount());
    {
        wp<Foo> wp2 = wp1;
        ASSERT_EQ(2, foo->getWeakRefs()->getWeakCount());
    }
    EXPECT_EQ(1, foo->getWeakRefs()->getWeakCount());
    ASSERT_FALSE(isDeleted) << "deleted too early! still has a reference!";
    wp1 = nullptr;
    ASSERT_FALSE(isDeleted) << "Deletion on wp destruction should no longer occur";
}

TEST(RefBase, Comparisons) {
    bool isDeleted, isDeleted2, isDeleted3;
    Foo* foo = new Foo(&isDeleted);
    Foo* foo2 = new Foo(&isDeleted2);
    sp<Foo> sp1(foo);
    sp<Foo> sp2(foo2);
    wp<Foo> wp1(sp1);
    wp<Foo> wp2(sp1);
    wp<Foo> wp3(sp2);
    ASSERT_TRUE(wp1 == wp2);
    ASSERT_TRUE(wp1 == sp1);
    ASSERT_TRUE(wp3 == sp2);
    ASSERT_TRUE(wp1 != sp2);
    ASSERT_TRUE(wp1 <= wp2);
    ASSERT_TRUE(wp1 >= wp2);
    ASSERT_FALSE(wp1 != wp2);
    ASSERT_FALSE(wp1 > wp2);
    ASSERT_FALSE(wp1 < wp2);
    ASSERT_FALSE(sp1 == sp2);
    ASSERT_TRUE(sp1 != sp2);
    bool sp1_smaller = sp1 < sp2;
    wp<Foo>wp_smaller = sp1_smaller ? wp1 : wp3;
    wp<Foo>wp_larger = sp1_smaller ? wp3 : wp1;
    ASSERT_TRUE(wp_smaller < wp_larger);
    ASSERT_TRUE(wp_smaller != wp_larger);
    ASSERT_TRUE(wp_smaller <= wp_larger);
    ASSERT_FALSE(wp_smaller == wp_larger);
    ASSERT_FALSE(wp_smaller > wp_larger);
    ASSERT_FALSE(wp_smaller >= wp_larger);
    sp2 = nullptr;
    ASSERT_TRUE(isDeleted2);
    ASSERT_FALSE(isDeleted);
    ASSERT_FALSE(wp3 == sp2);
    // Comparison results on weak pointers should not be affected.
    ASSERT_TRUE(wp_smaller < wp_larger);
    ASSERT_TRUE(wp_smaller != wp_larger);
    ASSERT_TRUE(wp_smaller <= wp_larger);
    ASSERT_FALSE(wp_smaller == wp_larger);
    ASSERT_FALSE(wp_smaller > wp_larger);
    ASSERT_FALSE(wp_smaller >= wp_larger);
    wp2 = nullptr;
    ASSERT_FALSE(wp1 == wp2);
    ASSERT_TRUE(wp1 != wp2);
    wp1.clear();
    ASSERT_TRUE(wp1 == wp2);
    ASSERT_FALSE(wp1 != wp2);
    wp3.clear();
    ASSERT_TRUE(wp1 == wp3);
    ASSERT_FALSE(wp1 != wp3);
    ASSERT_FALSE(isDeleted);
    sp1.clear();
    ASSERT_TRUE(isDeleted);
    ASSERT_TRUE(sp1 == sp2);
    // Try to check that null pointers are properly initialized.
    {
        // Try once with non-null, to maximize chances of getting junk on the
        // stack.
        sp<Foo> sp3(new Foo(&isDeleted3));
        wp<Foo> wp4(sp3);
        wp<Foo> wp5;
        ASSERT_FALSE(wp4 == wp5);
        ASSERT_TRUE(wp4 != wp5);
        ASSERT_FALSE(sp3 == wp5);
        ASSERT_FALSE(wp5 == sp3);
        ASSERT_TRUE(sp3 != wp5);
        ASSERT_TRUE(wp5 != sp3);
        ASSERT_TRUE(sp3 == wp4);
    }
    {
        sp<Foo> sp3;
        wp<Foo> wp4(sp3);
        wp<Foo> wp5;
        ASSERT_TRUE(wp4 == wp5);
        ASSERT_FALSE(wp4 != wp5);
        ASSERT_TRUE(sp3 == wp5);
        ASSERT_TRUE(wp5 == sp3);
        ASSERT_FALSE(sp3 != wp5);
        ASSERT_FALSE(wp5 != sp3);
        ASSERT_TRUE(sp3 == wp4);
    }
}

// Check whether comparison against dead wp works, even if the object referenced
// by the new wp happens to be at the same address.
TEST(RefBase, ReplacedComparison) {
    bool isDeleted, isDeleted2;
    FooFixedAlloc* foo = new FooFixedAlloc(&isDeleted);
    sp<FooFixedAlloc> sp1(foo);
    wp<FooFixedAlloc> wp1(sp1);
    ASSERT_TRUE(wp1 == sp1);
    sp1.clear();  // Deallocates the object.
    ASSERT_TRUE(isDeleted);
    FooFixedAlloc* foo2 = new FooFixedAlloc(&isDeleted2);
    ASSERT_FALSE(isDeleted2);
    ASSERT_EQ(foo, foo2);  // Not technically a legal comparison, but ...
    sp<FooFixedAlloc> sp2(foo2);
    wp<FooFixedAlloc> wp2(sp2);
    ASSERT_TRUE(sp2 == wp2);
    ASSERT_FALSE(sp2 != wp2);
    ASSERT_TRUE(sp2 != wp1);
    ASSERT_FALSE(sp2 == wp1);
    ASSERT_FALSE(sp2 == sp1);  // sp1 is null.
    ASSERT_FALSE(wp1 == wp2);  // wp1 refers to old object.
    ASSERT_TRUE(wp1 != wp2);
    ASSERT_TRUE(wp1 > wp2 || wp1 < wp2);
    ASSERT_TRUE(wp1 >= wp2 || wp1 <= wp2);
    ASSERT_FALSE(wp1 >= wp2 && wp1 <= wp2);
    ASSERT_FALSE(wp1 == nullptr);
    wp1 = sp2;
    ASSERT_TRUE(wp1 == wp2);
    ASSERT_FALSE(wp1 != wp2);
}

TEST(RefBase, AssertWeakRefExistsSuccess) {
    bool isDeleted;
    sp<Foo> foo = sp<Foo>::make(&isDeleted);
    wp<Foo> weakFoo = foo;

    EXPECT_EQ(weakFoo, wp<Foo>::fromExisting(foo.get()));
    EXPECT_EQ(weakFoo.unsafe_get(), wp<Foo>::fromExisting(foo.get()).unsafe_get());

    EXPECT_FALSE(isDeleted);
    foo = nullptr;
    EXPECT_TRUE(isDeleted);
}

TEST(RefBase, AssertWeakRefExistsDeath) {
    // uses some other refcounting method, or none at all
    bool isDeleted;
    Foo* foo = new Foo(&isDeleted);

    // can only get a valid wp<> object when you construct it from an sp<>
    EXPECT_DEATH(wp<Foo>::fromExisting(foo), "");

    delete foo;
}

// Set up a situation in which we race with visit2AndRremove() to delete
// 2 strong references.  Bar destructor checks that there are no early
// deletions and prior updates are visible to destructor.
class Bar : public RefBase {
public:
    Bar(std::atomic<int>* delete_count) : mVisited1(false), mVisited2(false),
            mDeleteCount(delete_count) {
    }

    ~Bar() {
        EXPECT_TRUE(mVisited1);
        EXPECT_TRUE(mVisited2);
        (*mDeleteCount)++;
    }
    bool mVisited1;
    bool mVisited2;
private:
    std::atomic<int>* mDeleteCount;
};

static sp<Bar> buffer;
static std::atomic<bool> bufferFull(false);

// Wait until bufferFull has value val.
static inline void waitFor(bool val) {
    while (bufferFull != val) {}
}

cpu_set_t otherCpus;

// Divide the cpus we're allowed to run on into myCpus and otherCpus.
// Set origCpus to the processors we were originally allowed to run on.
// Return false if origCpus doesn't include at least processors 0 and 1.
static bool setExclusiveCpus(cpu_set_t* origCpus /* out */,
        cpu_set_t* myCpus /* out */, cpu_set_t* otherCpus) {
    if (sched_getaffinity(0, sizeof(cpu_set_t), origCpus) != 0) {
        return false;
    }
    if (!CPU_ISSET(0,  origCpus) || !CPU_ISSET(1, origCpus)) {
        return false;
    }
    CPU_ZERO(myCpus);
    CPU_ZERO(otherCpus);
    CPU_OR(myCpus, myCpus, origCpus);
    CPU_OR(otherCpus, otherCpus, origCpus);
    for (unsigned i = 0; i < CPU_SETSIZE; ++i) {
        // I get the even cores, the other thread gets the odd ones.
        if (i & 1) {
            CPU_CLR(i, myCpus);
        } else {
            CPU_CLR(i, otherCpus);
        }
    }
    return true;
}

static void visit2AndRemove() {
    if (sched_setaffinity(0, sizeof(cpu_set_t), &otherCpus) != 0) {
        FAIL() << "setaffinity returned:" << errno;
    }
    for (int i = 0; i < NITERS; ++i) {
        waitFor(true);
        buffer->mVisited2 = true;
        buffer = nullptr;
        bufferFull = false;
    }
}

TEST(RefBase, RacingDestructors) {
    cpu_set_t origCpus;
    cpu_set_t myCpus;
    // Restrict us and the helper thread to disjoint cpu sets.
    // This prevents us from getting scheduled against each other,
    // which would be atrociously slow.
    if (setExclusiveCpus(&origCpus, &myCpus, &otherCpus)) {
        std::thread t(visit2AndRemove);
        std::atomic<int> deleteCount(0);
        if (sched_setaffinity(0, sizeof(cpu_set_t), &myCpus) != 0) {
            FAIL() << "setaffinity returned:" << errno;
        }
        for (int i = 0; i < NITERS; ++i) {
            waitFor(false);
            Bar* bar = new Bar(&deleteCount);
            sp<Bar> sp3(bar);
            buffer = sp3;
            bufferFull = true;
            ASSERT_TRUE(bar->getStrongCount() >= 1);
            // Weak count includes strong count.
            ASSERT_TRUE(bar->getWeakRefs()->getWeakCount() >= 1);
            sp3->mVisited1 = true;
            sp3 = nullptr;
        }
        t.join();
        if (sched_setaffinity(0, sizeof(cpu_set_t), &origCpus) != 0) {
            FAIL();
        }
        ASSERT_EQ(NITERS, deleteCount) << "Deletions missed!";
    }  // Otherwise this is slow and probably pointless on a uniprocessor.
}

static wp<Bar> wpBuffer;
static std::atomic<bool> wpBufferFull(false);

// Wait until wpBufferFull has value val.
static inline void wpWaitFor(bool val) {
    while (wpBufferFull != val) {}
}

static void visit3AndRemove() {
    if (sched_setaffinity(0, sizeof(cpu_set_t), &otherCpus) != 0) {
        FAIL() << "setaffinity returned:" << errno;
    }
    for (int i = 0; i < NITERS; ++i) {
        wpWaitFor(true);
        {
            sp<Bar> sp1 = wpBuffer.promote();
            // We implicitly check that sp1 != NULL
            sp1->mVisited2 = true;
        }
        wpBuffer = nullptr;
        wpBufferFull = false;
    }
}

TEST(RefBase, RacingPromotions) {
    cpu_set_t origCpus;
    cpu_set_t myCpus;
    // Restrict us and the helper thread to disjoint cpu sets.
    // This prevents us from getting scheduled against each other,
    // which would be atrociously slow.
    if (setExclusiveCpus(&origCpus, &myCpus, &otherCpus)) {
        std::thread t(visit3AndRemove);
        std::atomic<int> deleteCount(0);
        if (sched_setaffinity(0, sizeof(cpu_set_t), &myCpus) != 0) {
            FAIL() << "setaffinity returned:" << errno;
        }
        for (int i = 0; i < NITERS; ++i) {
            Bar* bar = new Bar(&deleteCount);
            wp<Bar> wp1(bar);
            bar->mVisited1 = true;
            if (i % (NITERS / 10) == 0) {
                // Do this rarely, since it generates a log message.
                wp1 = nullptr;  // No longer destroys the object.
                wp1 = bar;
            }
            wpBuffer = wp1;
            ASSERT_EQ(bar->getWeakRefs()->getWeakCount(), 2);
            wpBufferFull = true;
            // Promotion races with that in visit3AndRemove.
            // This may or may not succeed, but it shouldn't interfere with
            // the concurrent one.
            sp<Bar> sp1 = wp1.promote();
            wpWaitFor(false);  // Waits for other thread to drop strong pointer.
            sp1 = nullptr;
            // No strong pointers here.
            sp1 = wp1.promote();
            ASSERT_EQ(sp1.get(), nullptr) << "Dead wp promotion succeeded!";
        }
        t.join();
        if (sched_setaffinity(0, sizeof(cpu_set_t), &origCpus) != 0) {
            FAIL();
        }
        ASSERT_EQ(NITERS, deleteCount) << "Deletions missed!";
    }  // Otherwise this is slow and probably pointless on a uniprocessor.
}
