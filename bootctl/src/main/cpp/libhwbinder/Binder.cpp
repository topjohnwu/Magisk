/*
 * Copyright (C) 2005 The Android Open Source Project
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

#include <hwbinder/Binder.h>

#include <android-base/macros.h>
#include <cutils/android_filesystem_config.h>
#include <cutils/multiuser.h>
#include <hwbinder/BpHwBinder.h>
#include <hwbinder/IInterface.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/Parcel.h>
#include <utils/Log.h>
#include <utils/misc.h>

#include <linux/sched.h>
#include <stdio.h>

#include <atomic>

namespace android {
namespace hardware {

// ---------------------------------------------------------------------------

IBinder::IBinder()
    : RefBase()
{
}

IBinder::~IBinder()
{
}

// ---------------------------------------------------------------------------

BHwBinder* IBinder::localBinder()
{
    return nullptr;
}

BpHwBinder* IBinder::remoteBinder()
{
    return nullptr;
}

bool IBinder::checkSubclass(const void* /*subclassID*/) const
{
    return false;
}

// ---------------------------------------------------------------------------

class BHwBinder::Extras
{
public:
    // unlocked objects
    bool mRequestingSid = false;

    // for below objects
    Mutex mLock;
    BpHwBinder::ObjectManager mObjects;
};

// ---------------------------------------------------------------------------

BHwBinder::BHwBinder() : mSchedPolicy(SCHED_NORMAL), mSchedPriority(0), mExtras(nullptr)
{
}

int BHwBinder::getMinSchedulingPolicy() {
    return mSchedPolicy;
}

int BHwBinder::getMinSchedulingPriority() {
    return mSchedPriority;
}

bool BHwBinder::isRequestingSid() {
    Extras* e = mExtras.load(std::memory_order_acquire);

    return e && e->mRequestingSid;
}

void BHwBinder::setRequestingSid(bool requestingSid) {
    Extras* e = mExtras.load(std::memory_order_acquire);

    if (!e) {
        // default is false. Most things don't need sids, so avoiding allocations when possible.
        if (!requestingSid) {
            return;
        }

        e = getOrCreateExtras();
        if (!e) return; // out of memory
    }

    e->mRequestingSid = requestingSid;
}

status_t BHwBinder::transact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags, TransactCallback callback)
{
    data.setDataPosition(0);

    if (reply != nullptr && (flags & FLAG_CLEAR_BUF)) {
        reply->markSensitive();
    }

    // extra comment to try to force running all tests
    if (UNLIKELY(code == HIDL_DEBUG_TRANSACTION)) {
        uid_t uid = IPCThreadState::self()->getCallingUid();
        if (multiuser_get_app_id(uid) >= AID_APP_START) {
            ALOGE("Can not call IBase::debug from apps");
            return PERMISSION_DENIED;
        }
    }

    status_t err = NO_ERROR;
    switch (code) {
        default:
            err = onTransact(code, data, reply, flags,
                    [&](auto &replyParcel) {
                        replyParcel.setDataPosition(0);
                        if (callback != nullptr) {
                            callback(replyParcel);
                        }
                    });
            break;
    }

    return err;
}

status_t BHwBinder::linkToDeath(
    const sp<DeathRecipient>& /*recipient*/, void* /*cookie*/,
    uint32_t /*flags*/)
{
    return INVALID_OPERATION;
}

status_t BHwBinder::unlinkToDeath(
    const wp<DeathRecipient>& /*recipient*/, void* /*cookie*/,
    uint32_t /*flags*/, wp<DeathRecipient>* /*outRecipient*/)
{
    return INVALID_OPERATION;
}

void BHwBinder::attachObject(
    const void* objectID, void* object, void* cleanupCookie,
    object_cleanup_func func)
{
    Extras* e = getOrCreateExtras();
    if (!e) return; // out of memory

    AutoMutex _l(e->mLock);
    e->mObjects.attach(objectID, object, cleanupCookie, func);
}

void* BHwBinder::findObject(const void* objectID) const
{
    Extras* e = mExtras.load(std::memory_order_acquire);
    if (!e) return nullptr;

    AutoMutex _l(e->mLock);
    return e->mObjects.find(objectID);
}

void BHwBinder::detachObject(const void* objectID)
{
    Extras* e = mExtras.load(std::memory_order_acquire);
    if (!e) return;

    AutoMutex _l(e->mLock);
    e->mObjects.detach(objectID);
}

BHwBinder* BHwBinder::localBinder()
{
    return this;
}

BHwBinder::~BHwBinder()
{
    Extras* e = mExtras.load(std::memory_order_relaxed);
    if (e) delete e;
}


status_t BHwBinder::onTransact(
    uint32_t /*code*/, const Parcel& /*data*/, Parcel* /*reply*/, uint32_t /*flags*/,
    TransactCallback /*callback*/)
{
    return UNKNOWN_TRANSACTION;
}

BHwBinder::Extras* BHwBinder::getOrCreateExtras()
{
    Extras* e = mExtras.load(std::memory_order_acquire);

    if (!e) {
        e = new Extras;
        Extras* expected = nullptr;
        if (!mExtras.compare_exchange_strong(expected, e,
                                             std::memory_order_release,
                                             std::memory_order_acquire)) {
            delete e;
            e = expected;  // Filled in by CAS
        }
        if (e == nullptr) return nullptr; // out of memory
    }

    return e;
}

// ---------------------------------------------------------------------------

enum {
    // This is used to transfer ownership of the remote binder from
    // the BpHwRefBase object holding it (when it is constructed), to the
    // owner of the BpHwRefBase object when it first acquires that BpHwRefBase.
    kRemoteAcquired = 0x00000001
};

BpHwRefBase::BpHwRefBase(const sp<IBinder>& o)
    : mRemote(o.get()), mRefs(nullptr), mState(0)
{
    if (mRemote) {
        mRemote->incStrong(this);           // Removed on first IncStrong().
    }
}

BpHwRefBase::~BpHwRefBase()
{
    if (mRemote) {
        if (!(mState.load(std::memory_order_relaxed)&kRemoteAcquired)) {
            mRemote->decStrong(this);
        }
    }
}

void BpHwRefBase::onFirstRef()
{
    mState.fetch_or(kRemoteAcquired, std::memory_order_relaxed);
}

void BpHwRefBase::onLastStrongRef(const void* /*id*/)
{
    if (mRemote) {
        mRemote->decStrong(this);
    }
}

bool BpHwRefBase::onIncStrongAttempted(uint32_t /*flags*/, const void* /*id*/)
{
    return false;
}

// ---------------------------------------------------------------------------

} // namespace hardware
} // namespace android
