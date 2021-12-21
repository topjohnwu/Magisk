/*
 * Copyright 2020 The Android Open Source Project
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

#define LOG_TAG "RefBaseFuzz"

#include <thread>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/Log.h"
#include "utils/RWLock.h"
#include "utils/RefBase.h"
#include "utils/StrongPointer.h"

using android::RefBase;
using android::RWLock;
using android::sp;
using android::wp;

static constexpr int kMaxOperations = 100;
static constexpr int kMaxThreads = 10;
struct RefBaseSubclass : public RefBase {
  public:
    RefBaseSubclass(bool* deletedCheck, RWLock& deletedMtx)
        : mDeleted(deletedCheck), mRwLock(deletedMtx) {
        RWLock::AutoWLock lock(mRwLock);
        *mDeleted = false;
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
    }

    virtual ~RefBaseSubclass() {
        RWLock::AutoWLock lock(mRwLock);
        *mDeleted = true;
    }

  private:
    bool* mDeleted;
    android::RWLock& mRwLock;
};

// A thread-specific state object for ref
struct RefThreadState {
    size_t strongCount = 0;
    size_t weakCount = 0;
};

RWLock gRefDeletedLock;
bool gRefDeleted = false;
bool gHasModifiedRefs = false;
RefBaseSubclass* ref;
RefBase::weakref_type* weakRefs;

// These operations don't need locks as they explicitly check per-thread counts before running
// they also have the potential to write to gRefDeleted, so must not be locked.
const std::vector<std::function<void(RefThreadState*)>> kUnlockedOperations = {
        [](RefThreadState* refState) -> void {
            if (refState->strongCount > 0) {
                ref->decStrong(nullptr);
                gHasModifiedRefs = true;
                refState->strongCount--;
            }
        },
        [](RefThreadState* refState) -> void {
            if (refState->weakCount > 0) {
                weakRefs->decWeak(nullptr);
                gHasModifiedRefs = true;
                refState->weakCount--;
            }
        },
};

const std::vector<std::function<void(RefThreadState*)>> kMaybeLockedOperations = {
        // Read-only operations
        [](RefThreadState*) -> void { ref->getStrongCount(); },
        [](RefThreadState*) -> void { weakRefs->getWeakCount(); },
        [](RefThreadState*) -> void { ref->printRefs(); },

        // Read/write operations
        [](RefThreadState* refState) -> void {
            ref->incStrong(nullptr);
            gHasModifiedRefs = true;
            refState->strongCount++;
        },
        [](RefThreadState* refState) -> void {
            ref->forceIncStrong(nullptr);
            gHasModifiedRefs = true;
            refState->strongCount++;
        },
        [](RefThreadState* refState) -> void {
            ref->createWeak(nullptr);
            gHasModifiedRefs = true;
            refState->weakCount++;
        },
        [](RefThreadState* refState) -> void {
            // This will increment weak internally, then attempt to
            // promote it to strong. If it fails, it decrements weak.
            // If it succeeds, the weak is converted to strong.
            // Both cases net no weak reference change.
            if (weakRefs->attemptIncStrong(nullptr)) {
                refState->strongCount++;
                gHasModifiedRefs = true;
            }
        },
        [](RefThreadState* refState) -> void {
            if (weakRefs->attemptIncWeak(nullptr)) {
                refState->weakCount++;
                gHasModifiedRefs = true;
            }
        },
        [](RefThreadState* refState) -> void {
            weakRefs->incWeak(nullptr);
            gHasModifiedRefs = true;
            refState->weakCount++;
        },
};

void loop(const std::vector<uint8_t>& fuzzOps) {
    RefThreadState state;
    uint8_t lockedOpSize = kMaybeLockedOperations.size();
    uint8_t totalOperationTypes = lockedOpSize + kUnlockedOperations.size();
    for (auto op : fuzzOps) {
        auto opVal = op % totalOperationTypes;
        if (opVal >= lockedOpSize) {
            kUnlockedOperations[opVal % lockedOpSize](&state);
        } else {
            // We only need to lock if we have no strong or weak count
            bool shouldLock = state.strongCount == 0 && state.weakCount == 0;
            if (shouldLock) {
                gRefDeletedLock.readLock();
                // If ref has deleted itself, we can no longer fuzz on this thread.
                if (gRefDeleted) {
                    // Unlock since we're exiting the loop here.
                    gRefDeletedLock.unlock();
                    return;
                }
            }
            // Execute the locked operation
            kMaybeLockedOperations[opVal](&state);
            // Unlock if we locked.
            if (shouldLock) {
                gRefDeletedLock.unlock();
            }
        }
    }

    // Instead of explicitly freeing this, we're going to remove our weak and
    // strong references.
    for (; state.weakCount > 0; state.weakCount--) {
        weakRefs->decWeak(nullptr);
    }

    // Clean up any strong references
    for (; state.strongCount > 0; state.strongCount--) {
        ref->decStrong(nullptr);
    }
}

void spawnThreads(FuzzedDataProvider* dataProvider) {
    std::vector<std::thread> threads = std::vector<std::thread>();

    // Get the number of threads to generate
    uint8_t count = dataProvider->ConsumeIntegralInRange<uint8_t>(1, kMaxThreads);
    // Generate threads
    for (uint8_t i = 0; i < count; i++) {
        uint8_t opCount = dataProvider->ConsumeIntegralInRange<uint8_t>(1, kMaxOperations);
        std::vector<uint8_t> threadOperations = dataProvider->ConsumeBytes<uint8_t>(opCount);
        std::thread tmpThread = std::thread(loop, threadOperations);
        threads.push_back(move(tmpThread));
    }

    for (auto& th : threads) {
        th.join();
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    gHasModifiedRefs = false;
    ref = new RefBaseSubclass(&gRefDeleted, gRefDeletedLock);
    weakRefs = ref->getWeakRefs();
    // Since we are modifying flags, (flags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK
    // is true. The destructor for RefBase should clean up weakrefs because of this.
    FuzzedDataProvider dataProvider(data, size);
    spawnThreads(&dataProvider);
    LOG_ALWAYS_FATAL_IF(!gHasModifiedRefs && gRefDeleted, "ref(%p) was prematurely deleted!", ref);
    // We need to explicitly delete this object
    // if no refs have been added or deleted.
    if (!gHasModifiedRefs && !gRefDeleted) {
        delete ref;
    }
    LOG_ALWAYS_FATAL_IF(gHasModifiedRefs && !gRefDeleted,
                        "ref(%p) should be deleted, is it leaking?", ref);
    return 0;
}
