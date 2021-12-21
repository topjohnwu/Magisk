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

#define LOG_TAG "hw-BpHwBinder"
//#define LOG_NDEBUG 0

#include <hwbinder/BpHwBinder.h>

#include <hwbinder/IPCThreadState.h>
#include <utils/Log.h>

#include <stdio.h>

//#undef ALOGV
//#define ALOGV(...) fprintf(stderr, __VA_ARGS__)

namespace android {
namespace hardware {

// ---------------------------------------------------------------------------

BpHwBinder::ObjectManager::ObjectManager()
{
}

BpHwBinder::ObjectManager::~ObjectManager()
{
    kill();
}

void BpHwBinder::ObjectManager::attach(
    const void* objectID, void* object, void* cleanupCookie,
    IBinder::object_cleanup_func func)
{
    entry_t e;
    e.object = object;
    e.cleanupCookie = cleanupCookie;
    e.func = func;

    if (mObjects.indexOfKey(objectID) >= 0) {
        ALOGE("Trying to attach object ID %p to binder ObjectManager %p with object %p, but object ID already in use",
                objectID, this,  object);
        return;
    }

    mObjects.add(objectID, e);
}

void* BpHwBinder::ObjectManager::find(const void* objectID) const
{
    const ssize_t i = mObjects.indexOfKey(objectID);
    if (i < 0) return nullptr;
    return mObjects.valueAt(i).object;
}

void BpHwBinder::ObjectManager::detach(const void* objectID)
{
    mObjects.removeItem(objectID);
}

void BpHwBinder::ObjectManager::kill()
{
    const size_t N = mObjects.size();
    ALOGV("Killing %zu objects in manager %p", N, this);
    for (size_t i=0; i<N; i++) {
        const entry_t& e = mObjects.valueAt(i);
        if (e.func != nullptr) {
            e.func(mObjects.keyAt(i), e.object, e.cleanupCookie);
        }
    }

    mObjects.clear();
}

// ---------------------------------------------------------------------------

BpHwBinder::BpHwBinder(int32_t handle)
    : mHandle(handle)
    , mAlive(1)
    , mObitsSent(0)
    , mObituaries(nullptr)
{
    ALOGV("Creating BpHwBinder %p handle %d\n", this, mHandle);

    extendObjectLifetime(OBJECT_LIFETIME_WEAK);
    IPCThreadState::self()->incWeakHandle(handle, this);
}

status_t BpHwBinder::transact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags, TransactCallback callback)
{
    // Once a binder has died, it will never come back to life.
    if (mAlive) {
        status_t status = IPCThreadState::self()->transact(
            mHandle, code, data, reply, flags);

        if (status == ::android::OK && callback != nullptr) {
            callback(*reply);
        }

        if (status == DEAD_OBJECT) mAlive = 0;
        return status;
    }

    return DEAD_OBJECT;
}

status_t BpHwBinder::linkToDeath(
    const sp<DeathRecipient>& recipient, void* cookie, uint32_t flags)
{
    Obituary ob;
    ob.recipient = recipient;
    ob.cookie = cookie;
    ob.flags = flags;

    LOG_ALWAYS_FATAL_IF(recipient == nullptr,
                        "linkToDeath(): recipient must be non-NULL");

    {
        AutoMutex _l(mLock);

        if (!mObitsSent) {
            if (!mObituaries) {
                mObituaries = new Vector<Obituary>;
                if (!mObituaries) {
                    return NO_MEMORY;
                }
                ALOGV("Requesting death notification: %p handle %d\n", this, mHandle);
                getWeakRefs()->incWeak(this);
                IPCThreadState* self = IPCThreadState::self();
                self->requestDeathNotification(mHandle, this);
                self->flushCommands();
            }
            ssize_t res = mObituaries->add(ob);
            return res >= (ssize_t)NO_ERROR ? (status_t)NO_ERROR : res;
        }
    }

    return DEAD_OBJECT;
}

status_t BpHwBinder::unlinkToDeath(
    const wp<DeathRecipient>& recipient, void* cookie, uint32_t flags,
    wp<DeathRecipient>* outRecipient)
{
    AutoMutex _l(mLock);

    if (mObitsSent) {
        return DEAD_OBJECT;
    }

    const size_t N = mObituaries ? mObituaries->size() : 0;
    for (size_t i=0; i<N; i++) {
        const Obituary& obit = mObituaries->itemAt(i);
        if ((obit.recipient == recipient
                    || (recipient == nullptr && obit.cookie == cookie))
                && obit.flags == flags) {
            if (outRecipient != nullptr) {
                *outRecipient = mObituaries->itemAt(i).recipient;
            }
            mObituaries->removeAt(i);
            if (mObituaries->size() == 0) {
                ALOGV("Clearing death notification: %p handle %d\n", this, mHandle);
                IPCThreadState* self = IPCThreadState::self();
                self->clearDeathNotification(mHandle, this);
                self->flushCommands();
                delete mObituaries;
                mObituaries = nullptr;
            }
            return NO_ERROR;
        }
    }

    return NAME_NOT_FOUND;
}

void BpHwBinder::sendObituary()
{
    ALOGV("Sending obituary for proxy %p handle %d, mObitsSent=%s\n",
        this, mHandle, mObitsSent ? "true" : "false");

    mAlive = 0;
    if (mObitsSent) return;

    mLock.lock();
    Vector<Obituary>* obits = mObituaries;
    if(obits != nullptr) {
        ALOGV("Clearing sent death notification: %p handle %d\n", this, mHandle);
        IPCThreadState* self = IPCThreadState::self();
        self->clearDeathNotification(mHandle, this);
        self->flushCommands();
        mObituaries = nullptr;
    }
    mObitsSent = 1;
    mLock.unlock();

    ALOGV("Reporting death of proxy %p for %zu recipients\n",
        this, obits ? obits->size() : 0U);

    if (obits != nullptr) {
        const size_t N = obits->size();
        for (size_t i=0; i<N; i++) {
            reportOneDeath(obits->itemAt(i));
        }

        delete obits;
    }
}

// Returns the strong refcount on the object this proxy points to, or
// -1 in case of failure.
ssize_t BpHwBinder::getNodeStrongRefCount()
{
    return ProcessState::self()->getStrongRefCountForNodeByHandle(mHandle);
}

void BpHwBinder::reportOneDeath(const Obituary& obit)
{
    sp<DeathRecipient> recipient = obit.recipient.promote();
    ALOGV("Reporting death to recipient: %p\n", recipient.get());
    if (recipient == nullptr) return;

    recipient->binderDied(this);
}


void BpHwBinder::attachObject(
    const void* objectID, void* object, void* cleanupCookie,
    object_cleanup_func func)
{
    AutoMutex _l(mLock);
    ALOGV("Attaching object %p to binder %p (manager=%p)", object, this, &mObjects);
    mObjects.attach(objectID, object, cleanupCookie, func);
}

void* BpHwBinder::findObject(const void* objectID) const
{
    AutoMutex _l(mLock);
    return mObjects.find(objectID);
}

void BpHwBinder::detachObject(const void* objectID)
{
    AutoMutex _l(mLock);
    mObjects.detach(objectID);
}

BpHwBinder* BpHwBinder::remoteBinder()
{
    return this;
}

BpHwBinder::~BpHwBinder()
{
    ALOGV("Destroying BpHwBinder %p handle %d\n", this, mHandle);

    IPCThreadState* ipc = IPCThreadState::self();

    if (ipc) {
        ipc->expungeHandle(mHandle, this);
        ipc->decWeakHandle(mHandle);
    }
}

void BpHwBinder::onFirstRef()
{
    ALOGV("onFirstRef BpHwBinder %p handle %d\n", this, mHandle);
    IPCThreadState* ipc = IPCThreadState::self();
    if (ipc) ipc->incStrongHandle(mHandle, this);
}

void BpHwBinder::onLastStrongRef(const void* /*id*/)
{
    ALOGV("onLastStrongRef BpHwBinder %p handle %d\n", this, mHandle);
    IF_ALOGV() {
        printRefs();
    }
    IPCThreadState* ipc = IPCThreadState::self();
    if (ipc) {
        ipc->decStrongHandle(mHandle);
        ipc->flushCommands();
    }

    mLock.lock();
    Vector<Obituary>* obits = mObituaries;
    if(obits != nullptr) {
        if (!obits->isEmpty()) {
            ALOGI("onLastStrongRef automatically unlinking death recipients");
        }

        if (ipc) ipc->clearDeathNotification(mHandle, this);
        mObituaries = nullptr;
    }
    mLock.unlock();

    if (obits != nullptr) {
        // XXX Should we tell any remaining DeathRecipient
        // objects that the last strong ref has gone away, so they
        // are no longer linked?
        delete obits;
        obits = nullptr;
    }
}

bool BpHwBinder::onIncStrongAttempted(uint32_t /*flags*/, const void* /*id*/)
{
    ALOGV("onIncStrongAttempted BpHwBinder %p handle %d\n", this, mHandle);
    IPCThreadState* ipc = IPCThreadState::self();
    return ipc ? ipc->attemptIncStrongHandle(mHandle) == NO_ERROR : false;
}

// ---------------------------------------------------------------------------

} // namespace hardware
} // namespace android
