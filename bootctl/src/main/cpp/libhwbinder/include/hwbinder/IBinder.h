/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_IBINDER_H
#define ANDROID_HARDWARE_IBINDER_H

#include <functional>

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String16.h>

// WARNING: this code is part of libhwbinder, a fork of libbinder. Generally,
// this means that it is only relevant to HIDL. Any AIDL- or libbinder-specific
// code should not try to use these things.

// ---------------------------------------------------------------------------
namespace android {
namespace hardware {

class BHwBinder;
class BpHwBinder;
class IInterface;
class Parcel;

/**
 * Base class and low-level protocol for a remotable object.
 * You can derive from this class to create an object for which other
 * processes can hold references to it.  Communication between processes
 * (method calls, property get and set) is down through a low-level
 * protocol implemented on top of the transact() API.
 */
class IBinder : public virtual RefBase
{
public:
    using TransactCallback = std::function<void(Parcel&)>;

    enum {
        /* It is very important that these values NEVER change. These values
         * must remain unchanged over the lifetime of android. This is
         * because the framework on a device will be updated independently of
         * the hals on a device. If the hals are compiled with one set of
         * transaction values, and the framework with another, then the
         * interface between them will be destroyed, and the device will not
         * work.
         */
        /////////////////// User defined transactions
        FIRST_CALL_TRANSACTION  = 0x00000001,
        LAST_CALL_TRANSACTION   = 0x0effffff,
        /////////////////// HIDL reserved
#define B_PACK_CHARS_USER(c1, c2, c3, c4) \
         ((((c1)<<24)) | (((c2)<<16)) | (((c3)<<8)) | (c4))
        FIRST_HIDL_TRANSACTION  = 0x0f000000,
        HIDL_PING_TRANSACTION                     = B_PACK_CHARS_USER(0x0f, 'P', 'N', 'G'),
        HIDL_DESCRIPTOR_CHAIN_TRANSACTION         = B_PACK_CHARS_USER(0x0f, 'C', 'H', 'N'),
        HIDL_GET_DESCRIPTOR_TRANSACTION           = B_PACK_CHARS_USER(0x0f, 'D', 'S', 'C'),
        HIDL_SYSPROPS_CHANGED_TRANSACTION         = B_PACK_CHARS_USER(0x0f, 'S', 'Y', 'S'),
        HIDL_LINK_TO_DEATH_TRANSACTION            = B_PACK_CHARS_USER(0x0f, 'L', 'T', 'D'),
        HIDL_UNLINK_TO_DEATH_TRANSACTION          = B_PACK_CHARS_USER(0x0f, 'U', 'T', 'D'),
        HIDL_SET_HAL_INSTRUMENTATION_TRANSACTION  = B_PACK_CHARS_USER(0x0f, 'I', 'N', 'T'),
        HIDL_GET_REF_INFO_TRANSACTION             = B_PACK_CHARS_USER(0x0f, 'R', 'E', 'F'),
        HIDL_DEBUG_TRANSACTION                    = B_PACK_CHARS_USER(0x0f, 'D', 'B', 'G'),
        HIDL_HASH_CHAIN_TRANSACTION               = B_PACK_CHARS_USER(0x0f, 'H', 'S', 'H'),
#undef B_PACK_CHARS_USER
        LAST_HIDL_TRANSACTION   = 0x0fffffff,

        // Corresponds to TF_ONE_WAY -- an asynchronous call.
        FLAG_ONEWAY             = 0x00000001,

        // Corresponds to TF_CLEAR_BUF -- clear transaction buffers after call
        // is made
        FLAG_CLEAR_BUF          = 0x00000020,
    };

                          IBinder();

    virtual status_t        transact(   uint32_t code,
                                        const Parcel& data,
                                        Parcel* reply,
                                        uint32_t flags = 0,
                                        TransactCallback callback = nullptr) = 0;

    class DeathRecipient : public virtual RefBase
    {
    public:
        virtual void binderDied(const wp<IBinder>& who) = 0;
    };

    /**
     * Register the @a recipient for a notification if this binder
     * goes away.  If this binder object unexpectedly goes away
     * (typically because its hosting process has been killed),
     * then DeathRecipient::binderDied() will be called with a reference
     * to this.
     *
     * The @a cookie is optional -- if non-NULL, it should be a
     * memory address that you own (that is, you know it is unique).
     *
     * @note You will only receive death notifications for remote binders,
     * as local binders by definition can't die without you dying as well.
     * Trying to use this function on a local binder will result in an
     * INVALID_OPERATION code being returned and nothing happening.
     *
     * @note This link always holds a weak reference to its recipient.
     *
     * @note You will only receive a weak reference to the dead
     * binder.  You should not try to promote this to a strong reference.
     * (Nor should you need to, as there is nothing useful you can
     * directly do with it now that it has passed on.)
     */
    virtual status_t        linkToDeath(const sp<DeathRecipient>& recipient,
                                        void* cookie = nullptr,
                                        uint32_t flags = 0) = 0;

    /**
     * Remove a previously registered death notification.
     * The @a recipient will no longer be called if this object
     * dies.  The @a cookie is optional.  If non-NULL, you can
     * supply a NULL @a recipient, and the recipient previously
     * added with that cookie will be unlinked.
     */
    virtual status_t        unlinkToDeath(  const wp<DeathRecipient>& recipient,
                                            void* cookie = nullptr,
                                            uint32_t flags = 0,
                                            wp<DeathRecipient>* outRecipient = nullptr) = 0;

    virtual bool            checkSubclass(const void* subclassID) const;

    typedef void (*object_cleanup_func)(const void* id, void* obj, void* cleanupCookie);

    /**
     * This object is attached for the lifetime of this binder object. When
     * this binder object is destructed, the cleanup function of all attached
     * objects are invoked with their respective objectID, object, and
     * cleanupCookie. Access to these APIs can be made from multiple threads,
     * but calls from different threads are allowed to be interleaved.
     */
    virtual void            attachObject(   const void* objectID,
                                            void* object,
                                            void* cleanupCookie,
                                            object_cleanup_func func) = 0;
    /**
     * Returns object attached with attachObject.
     */
    virtual void*           findObject(const void* objectID) const = 0;
    /**
     * WARNING: this API does not call the cleanup function for legacy reasons.
     * It also does not return void* for legacy reasons. If you need to detach
     * an object and destroy it, there are two options:
     * - if you can, don't call detachObject and instead wait for the destructor
     *   to clean it up.
     * - manually retrieve and destruct the object (if multiple of your threads
     *   are accessing these APIs, you must guarantee that attachObject isn't
     *   called after findObject and before detachObject is called).
     */
    virtual void            detachObject(const void* objectID) = 0;

    virtual BHwBinder*        localBinder();
    virtual BpHwBinder*       remoteBinder();

protected:
    virtual          ~IBinder();

private:
};

} // namespace hardware
} // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_HARDWARE_IBINDER_H
