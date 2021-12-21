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

#ifndef ANDROID_HARDWARE_BPHWBINDER_H
#define ANDROID_HARDWARE_BPHWBINDER_H

#include <hwbinder/IBinder.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>

// WARNING: this code is part of libhwbinder, a fork of libbinder. Generally,
// this means that it is only relevant to HIDL. Any AIDL- or libbinder-specific
// code should not try to use these things.

// ---------------------------------------------------------------------------
namespace android {
namespace hardware {

class BpHwBinder : public IBinder
{
public:
                        BpHwBinder(int32_t handle);

    inline  int32_t     handle() const { return mHandle; }

    virtual status_t    transact(   uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0,
                                    TransactCallback callback = nullptr);

    virtual status_t    linkToDeath(const sp<DeathRecipient>& recipient,
                                    void* cookie = nullptr,
                                    uint32_t flags = 0);
    virtual status_t    unlinkToDeath(  const wp<DeathRecipient>& recipient,
                                        void* cookie = nullptr,
                                        uint32_t flags = 0,
                                        wp<DeathRecipient>* outRecipient = nullptr);

    virtual void        attachObject(   const void* objectID,
                                        void* object,
                                        void* cleanupCookie,
                                        object_cleanup_func func);
    virtual void*       findObject(const void* objectID) const;
    virtual void        detachObject(const void* objectID);

    virtual BpHwBinder*   remoteBinder();

            void        sendObituary();
                        // This refcount includes:
                        // 1. Strong references to the node by this and other processes
                        // 2. Temporary strong references held by the kernel during a
                        //    transaction on the node.
                        // It does NOT include local strong references to the node
            ssize_t     getNodeStrongRefCount();
    class ObjectManager
    {
    public:
                    ObjectManager();
                    ~ObjectManager();

        void        attach( const void* objectID,
                            void* object,
                            void* cleanupCookie,
                            IBinder::object_cleanup_func func);
        void*       find(const void* objectID) const;
        void        detach(const void* objectID);

        void        kill();

    private:
                    ObjectManager(const ObjectManager&);
        ObjectManager& operator=(const ObjectManager&);

        struct entry_t
        {
            void* object;
            void* cleanupCookie;
            IBinder::object_cleanup_func func;
        };

        KeyedVector<const void*, entry_t> mObjects;
    };

protected:
    virtual             ~BpHwBinder();
    virtual void        onFirstRef();
    virtual void        onLastStrongRef(const void* id);
    virtual bool        onIncStrongAttempted(uint32_t flags, const void* id);

private:
    const   int32_t             mHandle;

    struct Obituary {
        wp<DeathRecipient> recipient;
        void* cookie;
        uint32_t flags;
    };

            void                reportOneDeath(const Obituary& obit);
            bool                isDescriptorCached() const;

    mutable Mutex               mLock;
            volatile int32_t    mAlive;
            volatile int32_t    mObitsSent;
            Vector<Obituary>*   mObituaries;
            ObjectManager       mObjects;
    mutable String16            mDescriptorCache;
};

} // namespace hardware
} // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_HARDWARE_BPHWBINDER_H
