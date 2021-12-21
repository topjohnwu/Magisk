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

#define LOG_TAG "hw-Parcel"
//#define LOG_NDEBUG 0

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>

#include <hwbinder/Binder.h>
#include <hwbinder/BpHwBinder.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/Parcel.h>
#include <hwbinder/ProcessState.h>

#include <cutils/ashmem.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <utils/String8.h>
#include <utils/String16.h>

#include "binder_kernel.h"
#include <hwbinder/Static.h>
#include "TextOutput.h"
#include "Utils.h"

#include <atomic>

#define LOG_REFS(...)
//#define LOG_REFS(...) ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG_ALLOC(...)
//#define LOG_ALLOC(...) ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG_BUFFER(...)
// #define LOG_BUFFER(...) ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// ---------------------------------------------------------------------------

// This macro should never be used at runtime, as a too large value
// of s could cause an integer overflow. Instead, you should always
// use the wrapper function pad_size()
#define PAD_SIZE_UNSAFE(s) (((s)+3)&~3)

static size_t pad_size(size_t s) {
    if (s > (std::numeric_limits<size_t>::max() - 3)) {
        LOG_ALWAYS_FATAL("pad size too big %zu", s);
    }
    return PAD_SIZE_UNSAFE(s);
}

// Note: must be kept in sync with android/os/StrictMode.java's PENALTY_GATHER
#define STRICT_MODE_PENALTY_GATHER (0x40 << 16)

namespace android {
namespace hardware {

static std::atomic<size_t> gParcelGlobalAllocCount;
static std::atomic<size_t> gParcelGlobalAllocSize;

static size_t gMaxFds = 0;

void acquire_binder_object(const sp<ProcessState>& proc,
    const flat_binder_object& obj, const void* who)
{
    switch (obj.hdr.type) {
        case BINDER_TYPE_BINDER:
            if (obj.binder) {
                LOG_REFS("Parcel %p acquiring reference on local %p", who, obj.cookie);
                reinterpret_cast<IBinder*>(obj.cookie)->incStrong(who);
            }
            return;
        case BINDER_TYPE_WEAK_BINDER:
            if (obj.binder)
                reinterpret_cast<RefBase::weakref_type*>(obj.binder)->incWeak(who);
            return;
        case BINDER_TYPE_HANDLE: {
            const sp<IBinder> b = proc->getStrongProxyForHandle(obj.handle);
            if (b != nullptr) {
                LOG_REFS("Parcel %p acquiring reference on remote %p", who, b.get());
                b->incStrong(who);
            }
            return;
        }
        case BINDER_TYPE_WEAK_HANDLE: {
            const wp<IBinder> b = proc->getWeakProxyForHandle(obj.handle);
            if (b != nullptr) b.get_refs()->incWeak(who);
            return;
        }
    }

    ALOGD("Invalid object type 0x%08x", obj.hdr.type);
}

void acquire_object(const sp<ProcessState>& proc, const binder_object_header& obj,
        const void *who) {
    switch (obj.type) {
        case BINDER_TYPE_BINDER:
        case BINDER_TYPE_WEAK_BINDER:
        case BINDER_TYPE_HANDLE:
        case BINDER_TYPE_WEAK_HANDLE: {
            const flat_binder_object& fbo = reinterpret_cast<const flat_binder_object&>(obj);
            acquire_binder_object(proc, fbo, who);
            break;
        }
    }
}

void release_object(const sp<ProcessState>& proc,
    const flat_binder_object& obj, const void* who)
{
    switch (obj.hdr.type) {
        case BINDER_TYPE_BINDER:
            if (obj.binder) {
                LOG_REFS("Parcel %p releasing reference on local %p", who, obj.cookie);
                reinterpret_cast<IBinder*>(obj.cookie)->decStrong(who);
            }
            return;
        case BINDER_TYPE_WEAK_BINDER:
            if (obj.binder)
                reinterpret_cast<RefBase::weakref_type*>(obj.binder)->decWeak(who);
            return;
        case BINDER_TYPE_HANDLE: {
            const sp<IBinder> b = proc->getStrongProxyForHandle(obj.handle);
            if (b != nullptr) {
                LOG_REFS("Parcel %p releasing reference on remote %p", who, b.get());
                b->decStrong(who);
            }
            return;
        }
        case BINDER_TYPE_WEAK_HANDLE: {
            const wp<IBinder> b = proc->getWeakProxyForHandle(obj.handle);
            if (b != nullptr) b.get_refs()->decWeak(who);
            return;
        }
        case BINDER_TYPE_FD: {
            if (obj.cookie != 0) { // owned
                close(obj.handle);
            }
            return;
        }
        case BINDER_TYPE_PTR: {
            // The relevant buffer is part of the transaction buffer and will be freed that way
            return;
        }
        case BINDER_TYPE_FDA: {
            // The enclosed file descriptors are closed in the kernel
            return;
        }
    }

    ALOGE("Invalid object type 0x%08x", obj.hdr.type);
}

inline static status_t finish_flatten_binder(
    const sp<IBinder>& /*binder*/, const flat_binder_object& flat, Parcel* out)
{
    return out->writeObject(flat);
}

status_t flatten_binder(const sp<ProcessState>& /*proc*/,
    const sp<IBinder>& binder, Parcel* out)
{
    flat_binder_object obj = {};

    if (binder != nullptr) {
        BHwBinder *local = binder->localBinder();
        if (!local) {
            BpHwBinder *proxy = binder->remoteBinder();
            if (proxy == nullptr) {
                ALOGE("null proxy");
            }
            const int32_t handle = proxy ? proxy->handle() : 0;
            obj.hdr.type = BINDER_TYPE_HANDLE;
            obj.flags = FLAT_BINDER_FLAG_ACCEPTS_FDS;
            obj.binder = 0; /* Don't pass uninitialized stack data to a remote process */
            obj.handle = handle;
            obj.cookie = 0;
        } else {
            // Get policy and convert it
            int policy = local->getMinSchedulingPolicy();
            int priority = local->getMinSchedulingPriority();

            obj.flags = priority & FLAT_BINDER_FLAG_PRIORITY_MASK;
            obj.flags |= FLAT_BINDER_FLAG_ACCEPTS_FDS | FLAT_BINDER_FLAG_INHERIT_RT;
            obj.flags |= (policy & 3) << FLAT_BINDER_FLAG_SCHED_POLICY_SHIFT;
            if (local->isRequestingSid()) {
                obj.flags |= FLAT_BINDER_FLAG_TXN_SECURITY_CTX;
            }
            obj.hdr.type = BINDER_TYPE_BINDER;
            obj.binder = reinterpret_cast<uintptr_t>(local->getWeakRefs());
            obj.cookie = reinterpret_cast<uintptr_t>(local);
        }
    } else {
        obj.hdr.type = BINDER_TYPE_BINDER;
        obj.binder = 0;
        obj.cookie = 0;
    }

    return finish_flatten_binder(binder, obj, out);
}

inline static status_t finish_unflatten_binder(
    BpHwBinder* /*proxy*/, const flat_binder_object& /*flat*/,
    const Parcel& /*in*/)
{
    return NO_ERROR;
}

status_t unflatten_binder(const sp<ProcessState>& proc,
    const Parcel& in, sp<IBinder>* out)
{
    const flat_binder_object* flat = in.readObject<flat_binder_object>();

    if (flat) {
        switch (flat->hdr.type) {
            case BINDER_TYPE_BINDER:
                *out = reinterpret_cast<IBinder*>(flat->cookie);
                return finish_unflatten_binder(nullptr, *flat, in);
            case BINDER_TYPE_HANDLE:
                *out = proc->getStrongProxyForHandle(flat->handle);
                return finish_unflatten_binder(
                    static_cast<BpHwBinder*>(out->get()), *flat, in);
        }
    }
    return BAD_TYPE;
}

// ---------------------------------------------------------------------------

Parcel::Parcel()
{
    LOG_ALLOC("Parcel %p: constructing", this);
    initState();
}

Parcel::~Parcel()
{
    freeDataNoInit();
    LOG_ALLOC("Parcel %p: destroyed", this);
}

size_t Parcel::getGlobalAllocSize() {
    return gParcelGlobalAllocSize.load();
}

size_t Parcel::getGlobalAllocCount() {
    return gParcelGlobalAllocCount.load();
}

const uint8_t* Parcel::data() const
{
    return mData;
}

size_t Parcel::dataSize() const
{
    return (mDataSize > mDataPos ? mDataSize : mDataPos);
}

size_t Parcel::dataAvail() const
{
    size_t result = dataSize() - dataPosition();
    if (result > INT32_MAX) {
        LOG_ALWAYS_FATAL("result too big: %zu", result);
    }
    return result;
}

size_t Parcel::dataPosition() const
{
    return mDataPos;
}

size_t Parcel::dataCapacity() const
{
    return mDataCapacity;
}

status_t Parcel::setDataSize(size_t size)
{
    if (size > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    status_t err;
    err = continueWrite(size);
    if (err == NO_ERROR) {
        mDataSize = size;
        ALOGV("setDataSize Setting data size of %p to %zu", this, mDataSize);
    }
    return err;
}

void Parcel::setDataPosition(size_t pos) const
{
    if (pos > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        LOG_ALWAYS_FATAL("pos too big: %zu", pos);
    }

    mDataPos = pos;
    mNextObjectHint = 0;
}

status_t Parcel::setDataCapacity(size_t size)
{
    if (size > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    if (size > mDataCapacity) return continueWrite(size);
    return NO_ERROR;
}

status_t Parcel::setData(const uint8_t* buffer, size_t len)
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    status_t err = restartWrite(len);
    if (err == NO_ERROR) {
        memcpy(const_cast<uint8_t*>(data()), buffer, len);
        mDataSize = len;
        mFdsKnown = false;
    }
    return err;
}

void Parcel::markSensitive() const
{
    mDeallocZero = true;
}

// Write RPC headers.  (previously just the interface token)
status_t Parcel::writeInterfaceToken(const char* interface)
{
    // currently the interface identification token is just its name as a string
    return writeCString(interface);
}

bool Parcel::enforceInterface(const char* interface) const
{
    const char* str = readCString();
    if (str != nullptr && strcmp(str, interface) == 0) {
        return true;
    } else {
        ALOGW("**** enforceInterface() expected '%s' but read '%s'",
                interface, (str ? str : "<empty string>"));
        return false;
    }
}

const binder_size_t* Parcel::objects() const
{
    return mObjects;
}

size_t Parcel::objectsCount() const
{
    return mObjectsSize;
}

status_t Parcel::errorCheck() const
{
    return mError;
}

void Parcel::setError(status_t err)
{
    mError = err;
}

status_t Parcel::finishWrite(size_t len)
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    //printf("Finish write of %d\n", len);
    mDataPos += len;
    ALOGV("finishWrite Setting data pos of %p to %zu", this, mDataPos);
    if (mDataPos > mDataSize) {
        mDataSize = mDataPos;
        ALOGV("finishWrite Setting data size of %p to %zu", this, mDataSize);
    }
    //printf("New pos=%d, size=%d\n", mDataPos, mDataSize);
    return NO_ERROR;
}

status_t Parcel::writeUnpadded(const void* data, size_t len)
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    size_t end = mDataPos + len;
    if (end < mDataPos) {
        // integer overflow
        return BAD_VALUE;
    }

    if (end <= mDataCapacity) {
restart_write:
        memcpy(mData+mDataPos, data, len);
        return finishWrite(len);
    }

    status_t err = growData(len);
    if (err == NO_ERROR) goto restart_write;
    return err;
}

status_t Parcel::write(const void* data, size_t len)
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    void* const d = writeInplace(len);
    if (d) {
        memcpy(d, data, len);
        return NO_ERROR;
    }
    return mError;
}

void* Parcel::writeInplace(size_t len)
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return nullptr;
    }

    const size_t padded = pad_size(len);

    // validate for integer overflow
    if (mDataPos+padded < mDataPos) {
        return nullptr;
    }

    if ((mDataPos+padded) <= mDataCapacity) {
restart_write:
        //printf("Writing %ld bytes, padded to %ld\n", len, padded);
        uint8_t* const data = mData+mDataPos;

        // Need to pad at end?
        if (padded != len) {
#if BYTE_ORDER == BIG_ENDIAN
            static const uint32_t mask[4] = {
                0x00000000, 0xffffff00, 0xffff0000, 0xff000000
            };
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
            static const uint32_t mask[4] = {
                0x00000000, 0x00ffffff, 0x0000ffff, 0x000000ff
            };
#endif
            //printf("Applying pad mask: %p to %p\n", (void*)mask[padded-len],
            //    *reinterpret_cast<void**>(data+padded-4));
            *reinterpret_cast<uint32_t*>(data+padded-4) &= mask[padded-len];
        }

        finishWrite(padded);
        return data;
    }

    status_t err = growData(padded);
    if (err == NO_ERROR) goto restart_write;
    return nullptr;
}

status_t Parcel::writeInt8(int8_t val)
{
    return write(&val, sizeof(val));
}

status_t Parcel::writeUint8(uint8_t val)
{
    return write(&val, sizeof(val));
}

status_t Parcel::writeInt16(int16_t val)
{
    return write(&val, sizeof(val));
}

status_t Parcel::writeUint16(uint16_t val)
{
    return write(&val, sizeof(val));
}

status_t Parcel::writeInt32(int32_t val)
{
    return writeAligned(val);
}

status_t Parcel::writeUint32(uint32_t val)
{
    return writeAligned(val);
}

status_t Parcel::writeBool(bool val)
{
    return writeInt8(int8_t(val));
}
status_t Parcel::writeInt64(int64_t val)
{
    return writeAligned(val);
}

status_t Parcel::writeUint64(uint64_t val)
{
    return writeAligned(val);
}

status_t Parcel::writePointer(uintptr_t val)
{
    return writeAligned<binder_uintptr_t>(val);
}

status_t Parcel::writeFloat(float val)
{
    return writeAligned(val);
}

#if defined(__mips__) && defined(__mips_hard_float)

status_t Parcel::writeDouble(double val)
{
    union {
        double d;
        unsigned long long ll;
    } u;
    u.d = val;
    return writeAligned(u.ll);
}

#else

status_t Parcel::writeDouble(double val)
{
    return writeAligned(val);
}

#endif

status_t Parcel::writeCString(const char* str)
{
    return write(str, strlen(str)+1);
}
status_t Parcel::writeString16(const std::unique_ptr<String16>& str)
{
    if (!str) {
        return writeInt32(-1);
    }

    return writeString16(*str);
}

status_t Parcel::writeString16(const String16& str)
{
    return writeString16(str.string(), str.size());
}

status_t Parcel::writeString16(const char16_t* str, size_t len)
{
    if (str == nullptr) return writeInt32(-1);

    status_t err = writeInt32(len);
    if (err == NO_ERROR) {
        len *= sizeof(char16_t);
        uint8_t* data = (uint8_t*)writeInplace(len+sizeof(char16_t));
        if (data) {
            memcpy(data, str, len);
            *reinterpret_cast<char16_t*>(data+len) = 0;
            return NO_ERROR;
        }
        err = mError;
    }
    return err;
}
status_t Parcel::writeStrongBinder(const sp<IBinder>& val)
{
    return flatten_binder(ProcessState::self(), val, this);
}

template <typename T>
status_t Parcel::writeObject(const T& val)
{
    const bool enoughData = (mDataPos+sizeof(val)) <= mDataCapacity;
    const bool enoughObjects = mObjectsSize < mObjectsCapacity;
    if (enoughData && enoughObjects) {
restart_write:
        *reinterpret_cast<T*>(mData+mDataPos) = val;

        const binder_object_header* hdr = reinterpret_cast<binder_object_header*>(mData+mDataPos);
        switch (hdr->type) {
            case BINDER_TYPE_BINDER:
            case BINDER_TYPE_WEAK_BINDER:
            case BINDER_TYPE_HANDLE:
            case BINDER_TYPE_WEAK_HANDLE: {
                const flat_binder_object *fbo = reinterpret_cast<const flat_binder_object*>(hdr);
                if (fbo->binder != 0) {
                    mObjects[mObjectsSize++] = mDataPos;
                    acquire_binder_object(ProcessState::self(), *fbo, this);
                }
                break;
            }
            case BINDER_TYPE_FD: {
                // remember if it's a file descriptor
                if (!mAllowFds) {
                    // fail before modifying our object index
                    return FDS_NOT_ALLOWED;
                }
                mHasFds = mFdsKnown = true;
                mObjects[mObjectsSize++] = mDataPos;
                break;
            }
            case BINDER_TYPE_FDA:
                mObjects[mObjectsSize++] = mDataPos;
                break;
            case BINDER_TYPE_PTR: {
                const binder_buffer_object *buffer_obj = reinterpret_cast<
                    const binder_buffer_object*>(hdr);
                if ((void *)buffer_obj->buffer != nullptr) {
                    mObjects[mObjectsSize++] = mDataPos;
                }
                break;
            }
            default: {
                ALOGE("writeObject: unknown type %d", hdr->type);
                break;
            }
        }
        return finishWrite(sizeof(val));
    }

    if (!enoughData) {
        const status_t err = growData(sizeof(val));
        if (err != NO_ERROR) return err;
    }
    if (!enoughObjects) {
        if (mObjectsSize > SIZE_MAX - 2) return NO_MEMORY; // overflow
        if (mObjectsSize + 2 > SIZE_MAX / 3) return NO_MEMORY; // overflow
        size_t newSize = ((mObjectsSize+2)*3)/2;
        if (newSize > SIZE_MAX / sizeof(binder_size_t)) return NO_MEMORY; // overflow
        binder_size_t* objects = (binder_size_t*)realloc(mObjects, newSize*sizeof(binder_size_t));
        if (objects == nullptr) return NO_MEMORY;
        mObjects = objects;
        mObjectsCapacity = newSize;
    }

    goto restart_write;
}

template status_t Parcel::writeObject<flat_binder_object>(const flat_binder_object& val);
template status_t Parcel::writeObject<binder_fd_object>(const binder_fd_object& val);
template status_t Parcel::writeObject<binder_buffer_object>(const binder_buffer_object& val);
template status_t Parcel::writeObject<binder_fd_array_object>(const binder_fd_array_object& val);

bool Parcel::validateBufferChild(size_t child_buffer_handle,
                                 size_t child_offset) const {
    if (child_buffer_handle >= mObjectsSize)
        return false;
    binder_buffer_object *child = reinterpret_cast<binder_buffer_object*>
            (mData + mObjects[child_buffer_handle]);
    if (child->hdr.type != BINDER_TYPE_PTR || child_offset > child->length) {
        // Parent object not a buffer, or not large enough
        LOG_BUFFER("writeEmbeddedReference found weird child. "
                   "child_offset = %zu, child->length = %zu",
                   child_offset, (size_t)child->length);
        return false;
    }
    return true;
}

bool Parcel::validateBufferParent(size_t parent_buffer_handle,
                                  size_t parent_offset) const {
    if (parent_buffer_handle >= mObjectsSize)
        return false;
    binder_buffer_object *parent = reinterpret_cast<binder_buffer_object*>
            (mData + mObjects[parent_buffer_handle]);
    if (parent->hdr.type != BINDER_TYPE_PTR ||
            sizeof(binder_uintptr_t) > parent->length ||
            parent_offset > parent->length - sizeof(binder_uintptr_t)) {
        // Parent object not a buffer, or not large enough
        return false;
    }
    return true;
}
status_t Parcel::writeEmbeddedBuffer(
        const void *buffer, size_t length, size_t *handle,
        size_t parent_buffer_handle, size_t parent_offset) {
    LOG_BUFFER("writeEmbeddedBuffer(%p, %zu, parent = (%zu, %zu)) -> %zu",
        buffer, length, parent_buffer_handle,
         parent_offset, mObjectsSize);
    if(!validateBufferParent(parent_buffer_handle, parent_offset))
        return BAD_VALUE;
    binder_buffer_object obj = {
        .hdr = { .type = BINDER_TYPE_PTR },
        .flags = BINDER_BUFFER_FLAG_HAS_PARENT,
        .buffer = reinterpret_cast<binder_uintptr_t>(buffer),
        .length = length,
        .parent = parent_buffer_handle,
        .parent_offset = parent_offset,
    };
    if (handle != nullptr) {
        // We use an index into mObjects as a handle
        *handle = mObjectsSize;
    }
    return writeObject(obj);
}

status_t Parcel::writeBuffer(const void *buffer, size_t length, size_t *handle)
{
    LOG_BUFFER("writeBuffer(%p, %zu) -> %zu",
        buffer, length, mObjectsSize);
    binder_buffer_object obj {
        .hdr = { .type = BINDER_TYPE_PTR },
        .flags = 0,
        .buffer = reinterpret_cast<binder_uintptr_t>(buffer),
        .length = length,
    };
    if (handle != nullptr) {
        // We use an index into mObjects as a handle
        *handle = mObjectsSize;
    }
    return writeObject(obj);
}

void Parcel::clearCache() const {
    LOG_BUFFER("clearing cache.");
    mBufCachePos = 0;
    mBufCache.clear();
}

void Parcel::updateCache() const {
    if(mBufCachePos == mObjectsSize)
        return;
    LOG_BUFFER("updating cache from %zu to %zu", mBufCachePos, mObjectsSize);
    for(size_t i = mBufCachePos; i < mObjectsSize; i++) {
        binder_size_t dataPos = mObjects[i];
        binder_buffer_object *obj =
            reinterpret_cast<binder_buffer_object*>(mData+dataPos);
        if(obj->hdr.type != BINDER_TYPE_PTR)
            continue;
        BufferInfo ifo;
        ifo.index = i;
        ifo.buffer = obj->buffer;
        ifo.bufend = obj->buffer + obj->length;
        mBufCache.push_back(ifo);
    }
    mBufCachePos = mObjectsSize;
}

/* O(n) (n=#buffers) to find a buffer that contains the given addr */
status_t Parcel::findBuffer(const void *ptr, size_t length, bool *found,
                        size_t *handle, size_t *offset) const {
    if(found == nullptr)
        return UNKNOWN_ERROR;
    updateCache();
    binder_uintptr_t ptrVal = reinterpret_cast<binder_uintptr_t>(ptr);
    // true if the pointer is in some buffer, but the length is too big
    // so that ptr + length doesn't fit into the buffer.
    bool suspectRejectBadPointer = false;
    LOG_BUFFER("findBuffer examining %zu objects.", mObjectsSize);
    for(auto entry = mBufCache.rbegin(); entry != mBufCache.rend(); ++entry ) {
        if(entry->buffer <= ptrVal && ptrVal < entry->bufend) {
            // might have found it.
            if(ptrVal + length <= entry->bufend) {
                *found = true;
                if(handle != nullptr) *handle = entry->index;
                if(offset != nullptr) *offset = ptrVal - entry->buffer;
                LOG_BUFFER("    findBuffer has a match at %zu!", entry->index);
                return OK;
            } else {
                suspectRejectBadPointer = true;
            }
        }
    }
    LOG_BUFFER("findBuffer did not find for ptr = %p.", ptr);
    *found = false;
    return suspectRejectBadPointer ? BAD_VALUE : OK;
}

/* findBuffer with the assumption that ptr = .buffer (so it points to top
 * of the buffer, aka offset 0).
 *  */
status_t Parcel::quickFindBuffer(const void *ptr, size_t *handle) const {
    updateCache();
    binder_uintptr_t ptrVal = reinterpret_cast<binder_uintptr_t>(ptr);
    LOG_BUFFER("quickFindBuffer examining %zu objects.", mObjectsSize);
    for(auto entry = mBufCache.rbegin(); entry != mBufCache.rend(); ++entry ) {
        if(entry->buffer == ptrVal) {
            if(handle != nullptr) *handle = entry->index;
            return OK;
        }
    }
    LOG_BUFFER("quickFindBuffer did not find for ptr = %p.", ptr);
    return NO_INIT;
}

status_t Parcel::writeNativeHandleNoDup(const native_handle_t *handle,
                                        bool embedded,
                                        size_t parent_buffer_handle,
                                        size_t parent_offset)
{
    size_t buffer_handle;
    status_t status = OK;

    if (handle == nullptr) {
        status = writeUint64(0);
        return status;
    }

    size_t native_handle_size = sizeof(native_handle_t)
                + handle->numFds * sizeof(int) + handle->numInts * sizeof(int);
    writeUint64(native_handle_size);

    if (embedded) {
        status = writeEmbeddedBuffer((void*) handle,
                native_handle_size, &buffer_handle,
                parent_buffer_handle, parent_offset);
    } else {
        status = writeBuffer((void*) handle, native_handle_size, &buffer_handle);
    }

    if (status != OK) {
        return status;
    }

    struct binder_fd_array_object fd_array {
        .hdr = { .type = BINDER_TYPE_FDA },
        .num_fds = static_cast<binder_size_t>(handle->numFds),
        .parent = buffer_handle,
        .parent_offset = offsetof(native_handle_t, data),
    };

    return writeObject(fd_array);
}

status_t Parcel::writeNativeHandleNoDup(const native_handle_t *handle)
{
    return writeNativeHandleNoDup(handle, false /* embedded */);
}

status_t Parcel::writeEmbeddedNativeHandle(const native_handle_t *handle,
                                           size_t parent_buffer_handle,
                                           size_t parent_offset)
{
    return writeNativeHandleNoDup(handle, true /* embedded */,
                                  parent_buffer_handle, parent_offset);
}

status_t Parcel::read(void* outData, size_t len) const
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    if ((mDataPos+pad_size(len)) >= mDataPos && (mDataPos+pad_size(len)) <= mDataSize
            && len <= pad_size(len)) {
        memcpy(outData, mData+mDataPos, len);
        mDataPos += pad_size(len);
        ALOGV("read Setting data pos of %p to %zu", this, mDataPos);
        return NO_ERROR;
    }
    return NOT_ENOUGH_DATA;
}

const void* Parcel::readInplace(size_t len) const
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return nullptr;
    }

    if ((mDataPos+pad_size(len)) >= mDataPos && (mDataPos+pad_size(len)) <= mDataSize
            && len <= pad_size(len)) {
        const void* data = mData+mDataPos;
        mDataPos += pad_size(len);
        ALOGV("readInplace Setting data pos of %p to %zu", this, mDataPos);
        return data;
    }
    return nullptr;
}

template<class T>
status_t Parcel::readAligned(T *pArg) const {
    static_assert(PAD_SIZE_UNSAFE(sizeof(T)) == sizeof(T));

    if ((mDataPos+sizeof(T)) <= mDataSize) {
        const void* data = mData+mDataPos;
        mDataPos += sizeof(T);
        *pArg =  *reinterpret_cast<const T*>(data);
        return NO_ERROR;
    } else {
        return NOT_ENOUGH_DATA;
    }
}

template<class T>
T Parcel::readAligned() const {
    T result;
    if (readAligned(&result) != NO_ERROR) {
        result = 0;
    }

    return result;
}

template<class T>
status_t Parcel::writeAligned(T val) {
    static_assert(PAD_SIZE_UNSAFE(sizeof(T)) == sizeof(T));

    if ((mDataPos+sizeof(val)) <= mDataCapacity) {
restart_write:
        *reinterpret_cast<T*>(mData+mDataPos) = val;
        return finishWrite(sizeof(val));
    }

    status_t err = growData(sizeof(val));
    if (err == NO_ERROR) goto restart_write;
    return err;
}

status_t Parcel::readInt8(int8_t *pArg) const
{
    return read(pArg, sizeof(*pArg));
}

status_t Parcel::readUint8(uint8_t *pArg) const
{
    return read(pArg, sizeof(*pArg));
}

status_t Parcel::readInt16(int16_t *pArg) const
{
    return read(pArg, sizeof(*pArg));
}

status_t Parcel::readUint16(uint16_t *pArg) const
{
    return read(pArg, sizeof(*pArg));
}

status_t Parcel::readInt32(int32_t *pArg) const
{
    return readAligned(pArg);
}

int32_t Parcel::readInt32() const
{
    return readAligned<int32_t>();
}

status_t Parcel::readUint32(uint32_t *pArg) const
{
    return readAligned(pArg);
}

uint32_t Parcel::readUint32() const
{
    return readAligned<uint32_t>();
}

status_t Parcel::readInt64(int64_t *pArg) const
{
    return readAligned(pArg);
}

int64_t Parcel::readInt64() const
{
    return readAligned<int64_t>();
}

status_t Parcel::readUint64(uint64_t *pArg) const
{
    return readAligned(pArg);
}

uint64_t Parcel::readUint64() const
{
    return readAligned<uint64_t>();
}

status_t Parcel::readPointer(uintptr_t *pArg) const
{
    status_t ret;
    binder_uintptr_t ptr;
    ret = readAligned(&ptr);
    if (!ret)
        *pArg = ptr;
    return ret;
}

uintptr_t Parcel::readPointer() const
{
    return readAligned<binder_uintptr_t>();
}


status_t Parcel::readFloat(float *pArg) const
{
    return readAligned(pArg);
}


float Parcel::readFloat() const
{
    return readAligned<float>();
}

#if defined(__mips__) && defined(__mips_hard_float)

status_t Parcel::readDouble(double *pArg) const
{
    union {
      double d;
      unsigned long long ll;
    } u;
    u.d = 0;
    status_t status;
    status = readAligned(&u.ll);
    *pArg = u.d;
    return status;
}

double Parcel::readDouble() const
{
    union {
      double d;
      unsigned long long ll;
    } u;
    u.ll = readAligned<unsigned long long>();
    return u.d;
}

#else

status_t Parcel::readDouble(double *pArg) const
{
    return readAligned(pArg);
}

double Parcel::readDouble() const
{
    return readAligned<double>();
}

#endif

status_t Parcel::readBool(bool *pArg) const
{
    int8_t tmp;
    status_t ret = readInt8(&tmp);
    *pArg = (tmp != 0);
    return ret;
}

bool Parcel::readBool() const
{
    int8_t tmp;
    status_t err = readInt8(&tmp);

    if (err != OK) {
        return 0;
    }

    return tmp != 0;
}

const char* Parcel::readCString() const
{
    if (mDataPos < mDataSize) {
        const size_t avail = mDataSize-mDataPos;
        const char* str = reinterpret_cast<const char*>(mData+mDataPos);
        // is the string's trailing NUL within the parcel's valid bounds?
        const char* eos = reinterpret_cast<const char*>(memchr(str, 0, avail));
        if (eos) {
            const size_t len = eos - str;
            mDataPos += pad_size(len+1);
            ALOGV("readCString Setting data pos of %p to %zu", this, mDataPos);
            return str;
        }
    }
    return nullptr;
}
String16 Parcel::readString16() const
{
    size_t len;
    const char16_t* str = readString16Inplace(&len);
    if (str) return String16(str, len);
    ALOGE("Reading a NULL string not supported here.");
    return String16();
}

status_t Parcel::readString16(std::unique_ptr<String16>* pArg) const
{
    const int32_t start = dataPosition();
    int32_t size;
    status_t status = readInt32(&size);
    pArg->reset();

    if (status != OK || size < 0) {
        return status;
    }

    setDataPosition(start);
    pArg->reset(new (std::nothrow) String16());

    status = readString16(pArg->get());

    if (status != OK) {
        pArg->reset();
    }

    return status;
}

status_t Parcel::readString16(String16* pArg) const
{
    size_t len;
    const char16_t* str = readString16Inplace(&len);
    if (str) {
        pArg->setTo(str, len);
        return 0;
    } else {
        *pArg = String16();
        return UNEXPECTED_NULL;
    }
}

const char16_t* Parcel::readString16Inplace(size_t* outLen) const
{
    int32_t size = readInt32();
    // watch for potential int overflow from size+1
    if (size >= 0 && size < INT32_MAX) {
        *outLen = size;
        const char16_t* str = (const char16_t*)readInplace((size+1)*sizeof(char16_t));
        if (str != nullptr) {
            return str;
        }
    }
    *outLen = 0;
    return nullptr;
}
status_t Parcel::readStrongBinder(sp<IBinder>* val) const
{
    status_t status = readNullableStrongBinder(val);
    if (status == OK && !val->get()) {
        status = UNEXPECTED_NULL;
    }
    return status;
}

status_t Parcel::readNullableStrongBinder(sp<IBinder>* val) const
{
    return unflatten_binder(ProcessState::self(), *this, val);
}

sp<IBinder> Parcel::readStrongBinder() const
{
    sp<IBinder> val;
    // Note that a lot of code in Android reads binders by hand with this
    // method, and that code has historically been ok with getting nullptr
    // back (while ignoring error codes).
    readNullableStrongBinder(&val);
    return val;
}

template<typename T>
const T* Parcel::readObject(size_t *objects_offset) const
{
    const size_t DPOS = mDataPos;
    if (objects_offset != nullptr) {
        *objects_offset = 0;
    }

    if ((DPOS+sizeof(T)) <= mDataSize) {
        const T* obj = reinterpret_cast<const T*>(mData+DPOS);
        mDataPos = DPOS + sizeof(T);
        const binder_object_header *hdr = reinterpret_cast<const binder_object_header*>(obj);
        switch (hdr->type) {
            case BINDER_TYPE_BINDER:
            case BINDER_TYPE_WEAK_BINDER:
            case BINDER_TYPE_HANDLE:
            case BINDER_TYPE_WEAK_HANDLE: {
                const flat_binder_object *flat_obj =
                    reinterpret_cast<const flat_binder_object*>(hdr);
                if (flat_obj->cookie == 0 && flat_obj->binder == 0) {
                    // When transferring a NULL binder object, we don't write it into
                    // the object list, so we don't want to check for it when
                    // reading.
                    ALOGV("readObject Setting data pos of %p to %zu", this, mDataPos);
                    return obj;
                }
                break;
            }
            case BINDER_TYPE_FD:
            case BINDER_TYPE_FDA:
                // fd (-arrays) must always appear in the meta-data list (eg touched by the kernel)
                break;
            case BINDER_TYPE_PTR: {
                const binder_buffer_object *buffer_obj =
                    reinterpret_cast<const binder_buffer_object*>(hdr);
                if ((void *)buffer_obj->buffer == nullptr) {
                    // null pointers can be returned directly - they're not written in the
                    // object list. All non-null buffers must appear in the objects list.
                    return obj;
                }
                break;
            }
        }
        // Ensure that this object is valid...
        binder_size_t* const OBJS = mObjects;
        const size_t N = mObjectsSize;
        size_t opos = mNextObjectHint;

        if (N > 0) {
            ALOGV("Parcel %p looking for obj at %zu, hint=%zu",
                 this, DPOS, opos);

            // Start at the current hint position, looking for an object at
            // the current data position.
            if (opos < N) {
                while (opos < (N-1) && OBJS[opos] < DPOS) {
                    opos++;
                }
            } else {
                opos = N-1;
            }
            if (OBJS[opos] == DPOS) {
                // Found it!
                ALOGV("Parcel %p found obj %zu at index %zu with forward search",
                     this, DPOS, opos);
                mNextObjectHint = opos+1;
                ALOGV("readObject Setting data pos of %p to %zu", this, mDataPos);
                if (objects_offset != nullptr) {
                    *objects_offset = opos;
                }
                return obj;
            }

            // Look backwards for it...
            while (opos > 0 && OBJS[opos] > DPOS) {
                opos--;
            }
            if (OBJS[opos] == DPOS) {
                // Found it!
                ALOGV("Parcel %p found obj %zu at index %zu with backward search",
                     this, DPOS, opos);
                mNextObjectHint = opos+1;
                ALOGV("readObject Setting data pos of %p to %zu", this, mDataPos);
                if (objects_offset != nullptr) {
                    *objects_offset = opos;
                }
                return obj;
            }
        }
        ALOGW("Attempt to read object from Parcel %p at offset %zu that is not in the object list",
             this, DPOS);
    }
    return nullptr;
}

template const flat_binder_object* Parcel::readObject<flat_binder_object>(size_t *objects_offset) const;

template const binder_fd_object* Parcel::readObject<binder_fd_object>(size_t *objects_offset) const;

template const binder_buffer_object* Parcel::readObject<binder_buffer_object>(size_t *objects_offset) const;

template const binder_fd_array_object* Parcel::readObject<binder_fd_array_object>(size_t *objects_offset) const;

bool Parcel::verifyBufferObject(const binder_buffer_object *buffer_obj,
                                size_t size, uint32_t flags, size_t parent,
                                size_t parentOffset) const {
    if (buffer_obj->length != size) {
        ALOGE("Buffer length %" PRIu64 " does not match expected size %zu.",
              static_cast<uint64_t>(buffer_obj->length), size);
        return false;
    }

    if (buffer_obj->flags != flags) {
        ALOGE("Buffer flags 0x%02X do not match expected flags 0x%02X.", buffer_obj->flags, flags);
        return false;
    }

    if (flags & BINDER_BUFFER_FLAG_HAS_PARENT) {
        if (buffer_obj->parent != parent) {
            ALOGE("Buffer parent %" PRIu64 " does not match expected parent %zu.",
                  static_cast<uint64_t>(buffer_obj->parent), parent);
            return false;
        }
        if (buffer_obj->parent_offset != parentOffset) {
              ALOGE("Buffer parent offset %" PRIu64 " does not match expected offset %zu.",
                  static_cast<uint64_t>(buffer_obj->parent_offset), parentOffset);
            return false;
        }

        binder_buffer_object *parentBuffer =
            reinterpret_cast<binder_buffer_object*>(mData + mObjects[parent]);
        void* bufferInParent = *reinterpret_cast<void**>(
            reinterpret_cast<uint8_t*>(parentBuffer->buffer) + parentOffset);
        void* childBuffer = reinterpret_cast<void*>(buffer_obj->buffer);

        if (bufferInParent != childBuffer) {
              ALOGE("Buffer in parent %p differs from embedded buffer %p",
                    bufferInParent, childBuffer);
              android_errorWriteLog(0x534e4554, "179289794");
              return false;
        }
    }

    return true;
}

status_t Parcel::readBuffer(size_t buffer_size, size_t *buffer_handle,
                            uint32_t flags, size_t parent, size_t parentOffset,
                            const void **buffer_out) const {

    const binder_buffer_object* buffer_obj = readObject<binder_buffer_object>(buffer_handle);

    if (buffer_obj == nullptr || buffer_obj->hdr.type != BINDER_TYPE_PTR) {
        return BAD_VALUE;
    }

    if (!verifyBufferObject(buffer_obj, buffer_size, flags, parent, parentOffset)) {
        return BAD_VALUE;
    }

    // in read side, always use .buffer and .length.
    *buffer_out = reinterpret_cast<void*>(buffer_obj->buffer);

    return OK;
}

status_t Parcel::readNullableBuffer(size_t buffer_size, size_t *buffer_handle,
                                    const void **buffer_out) const
{
    return readBuffer(buffer_size, buffer_handle,
                      0 /* flags */, 0 /* parent */, 0 /* parentOffset */,
                      buffer_out);
}

status_t Parcel::readBuffer(size_t buffer_size, size_t *buffer_handle,
                            const void **buffer_out) const
{
    status_t status = readNullableBuffer(buffer_size, buffer_handle, buffer_out);
    if (status == OK && *buffer_out == nullptr) {
        return UNEXPECTED_NULL;
    }
    return status;
}


status_t Parcel::readEmbeddedBuffer(size_t buffer_size,
                                    size_t *buffer_handle,
                                    size_t parent_buffer_handle,
                                    size_t parent_offset,
                                    const void **buffer_out) const
{
    status_t status = readNullableEmbeddedBuffer(buffer_size, buffer_handle,
                                                 parent_buffer_handle,
                                                 parent_offset, buffer_out);
    if (status == OK && *buffer_out == nullptr) {
        return UNEXPECTED_NULL;
    }
    return status;
}

status_t Parcel::readNullableEmbeddedBuffer(size_t buffer_size,
                                            size_t *buffer_handle,
                                            size_t parent_buffer_handle,
                                            size_t parent_offset,
                                            const void **buffer_out) const
{
    return readBuffer(buffer_size, buffer_handle, BINDER_BUFFER_FLAG_HAS_PARENT,
                      parent_buffer_handle, parent_offset, buffer_out);
}

status_t Parcel::readEmbeddedNativeHandle(size_t parent_buffer_handle,
                                          size_t parent_offset,
                                          const native_handle_t **handle) const
{
    status_t status = readNullableEmbeddedNativeHandle(parent_buffer_handle, parent_offset, handle);
    if (status == OK && *handle == nullptr) {
        return UNEXPECTED_NULL;
    }
    return status;
}

status_t Parcel::readNullableNativeHandleNoDup(const native_handle_t **handle,
                                               bool embedded,
                                               size_t parent_buffer_handle,
                                               size_t parent_offset) const
{
    uint64_t nativeHandleSize;
    status_t status = readUint64(&nativeHandleSize);
    if (status != OK) {
        return BAD_VALUE;
    }

    if (nativeHandleSize == 0) {
        // If !embedded, then parent_* vars are 0 and don't actually correspond
        // to anything. In that case, we're actually reading this data into
        // writable memory, and the handle returned from here will actually be
        // used (rather than be ignored).
        if (embedded) {
            binder_buffer_object *parentBuffer =
                reinterpret_cast<binder_buffer_object*>(mData + mObjects[parent_buffer_handle]);

            void* bufferInParent = *reinterpret_cast<void**>(
                reinterpret_cast<uint8_t*>(parentBuffer->buffer) + parent_offset);

            if (bufferInParent != nullptr) {
                  ALOGE("Buffer in (handle) parent %p is not nullptr.", bufferInParent);
                  android_errorWriteLog(0x534e4554, "179289794");
                  return BAD_VALUE;
            }
        }

        *handle = nullptr;
        return status;
    }

    if (nativeHandleSize < sizeof(native_handle_t)) {
        ALOGE("Received a native_handle_t size that was too small.");
        return BAD_VALUE;
    }

    size_t fdaParent;
    if (embedded) {
        status = readNullableEmbeddedBuffer(nativeHandleSize, &fdaParent,
                                            parent_buffer_handle, parent_offset,
                                            reinterpret_cast<const void**>(handle));
    } else {
        status = readNullableBuffer(nativeHandleSize, &fdaParent,
                                    reinterpret_cast<const void**>(handle));
    }

    if (status != OK) {
        return status;
    }

    if (*handle == nullptr) {
        // null handle already read above
        ALOGE("Expecting non-null handle buffer");
        return BAD_VALUE;
    }

    int numFds = (*handle)->numFds;
    int numInts = (*handle)->numInts;

    if (numFds < 0 || numFds > NATIVE_HANDLE_MAX_FDS) {
        ALOGE("Received native_handle with invalid number of fds.");
        return BAD_VALUE;
    }

    if (numInts < 0 || numInts > NATIVE_HANDLE_MAX_INTS) {
        ALOGE("Received native_handle with invalid number of ints.");
        return BAD_VALUE;
    }

    if (nativeHandleSize != (sizeof(native_handle_t) + ((numFds + numInts) * sizeof(int)))) {
        ALOGE("Size of native_handle doesn't match.");
        return BAD_VALUE;
    }

    const binder_fd_array_object* fd_array_obj = readObject<binder_fd_array_object>();

    if (fd_array_obj == nullptr || fd_array_obj->hdr.type != BINDER_TYPE_FDA) {
        ALOGE("Can't find file-descriptor array object.");
        return BAD_VALUE;
    }

    if (static_cast<int>(fd_array_obj->num_fds) != numFds) {
        ALOGE("Number of native handles does not match.");
        return BAD_VALUE;
    }

    if (fd_array_obj->parent != fdaParent) {
        ALOGE("Parent handle of file-descriptor array not correct.");
        return BAD_VALUE;
    }

    if (fd_array_obj->parent_offset != offsetof(native_handle_t, data)) {
        ALOGE("FD array object not properly offset in parent.");
        return BAD_VALUE;
    }

    return OK;
}

status_t Parcel::readNullableEmbeddedNativeHandle(size_t parent_buffer_handle,
                                                  size_t parent_offset,
                                                  const native_handle_t **handle) const
{
    return readNullableNativeHandleNoDup(handle, true /* embedded */, parent_buffer_handle,
                                         parent_offset);
}

status_t Parcel::readNativeHandleNoDup(const native_handle_t **handle) const
{
    status_t status = readNullableNativeHandleNoDup(handle);
    if (status == OK && *handle == nullptr) {
        return UNEXPECTED_NULL;
    }
    return status;
}

status_t Parcel::readNullableNativeHandleNoDup(const native_handle_t **handle) const
{
    return readNullableNativeHandleNoDup(handle, false /* embedded */);
}

void Parcel::closeFileDescriptors()
{
    size_t i = mObjectsSize;
    if (i > 0) {
        //ALOGI("Closing file descriptors for %zu objects...", i);
    }
    while (i > 0) {
        i--;
        const flat_binder_object* flat
            = reinterpret_cast<flat_binder_object*>(mData+mObjects[i]);
        if (flat->hdr.type == BINDER_TYPE_FD) {
            //ALOGI("Closing fd: %ld", flat->handle);
            close(flat->handle);
        }
    }
}

uintptr_t Parcel::ipcData() const
{
    return reinterpret_cast<uintptr_t>(mData);
}

size_t Parcel::ipcDataSize() const
{
    return mDataSize > mDataPos ? mDataSize : mDataPos;
}

uintptr_t Parcel::ipcObjects() const
{
    return reinterpret_cast<uintptr_t>(mObjects);
}

size_t Parcel::ipcObjectsCount() const
{
    return mObjectsSize;
}

#define BUFFER_ALIGNMENT_BYTES 8
size_t Parcel::ipcBufferSize() const
{
    size_t totalBuffersSize = 0;
    // Add size for BINDER_TYPE_PTR
    size_t i = mObjectsSize;
    while (i > 0) {
        i--;
        const binder_buffer_object* buffer
            = reinterpret_cast<binder_buffer_object*>(mData+mObjects[i]);
        if (buffer->hdr.type == BINDER_TYPE_PTR) {
            /* The binder kernel driver requires each buffer to be 8-byte
             * aligned */
            size_t alignedSize = (buffer->length + (BUFFER_ALIGNMENT_BYTES - 1))
                    & ~(BUFFER_ALIGNMENT_BYTES - 1);
            if (alignedSize > SIZE_MAX - totalBuffersSize) {
                ALOGE("ipcBuffersSize(): invalid buffer sizes.");
                return 0;
            }
            totalBuffersSize += alignedSize;
        }
    }
    return totalBuffersSize;
}

void Parcel::ipcSetDataReference(const uint8_t* data, size_t dataSize,
    const binder_size_t* objects, size_t objectsCount, release_func relFunc, void* relCookie)
{
    binder_size_t minOffset = 0;
    freeDataNoInit();
    mError = NO_ERROR;
    mData = const_cast<uint8_t*>(data);
    mDataSize = mDataCapacity = dataSize;
    //ALOGI("setDataReference Setting data size of %p to %lu (pid=%d)", this, mDataSize, getpid());
    mDataPos = 0;
    ALOGV("setDataReference Setting data pos of %p to %zu", this, mDataPos);
    mObjects = const_cast<binder_size_t*>(objects);
    mObjectsSize = mObjectsCapacity = objectsCount;
    mNextObjectHint = 0;
    clearCache();
    mOwner = relFunc;
    mOwnerCookie = relCookie;
    for (size_t i = 0; i < mObjectsSize; i++) {
        binder_size_t offset = mObjects[i];
        if (offset < minOffset) {
            ALOGE("%s: bad object offset %" PRIu64 " < %" PRIu64 "\n",
                  __func__, (uint64_t)offset, (uint64_t)minOffset);
            mObjectsSize = 0;
            break;
        }
        minOffset = offset + sizeof(flat_binder_object);
    }
    scanForFds();
}

void Parcel::print(TextOutput& to, uint32_t /*flags*/) const
{
    to << "Parcel(";

    if (errorCheck() != NO_ERROR) {
        const status_t err = errorCheck();
        to << "Error: " << (void*)(intptr_t)err << " \"" << strerror(-err) << "\"";
    } else if (dataSize() > 0) {
        const uint8_t* DATA = data();
        to << indent << HexDump(DATA, dataSize()) << dedent;
        const binder_size_t* OBJS = objects();
        const size_t N = objectsCount();
        for (size_t i=0; i<N; i++) {
            const flat_binder_object* flat
                = reinterpret_cast<const flat_binder_object*>(DATA+OBJS[i]);
            if (flat->hdr.type == BINDER_TYPE_PTR) {
                const binder_buffer_object* buffer
                    = reinterpret_cast<const binder_buffer_object*>(DATA+OBJS[i]);
                HexDump bufferDump((const uint8_t*)buffer->buffer, (size_t)buffer->length);
                bufferDump.setSingleLineCutoff(0);
                to << endl << "Object #" << i << " @ " << (void*)OBJS[i] << " (buffer size " << buffer->length << "):";
                to << indent << bufferDump << dedent;
            } else {
                to << endl << "Object #" << i << " @ " << (void*)OBJS[i] << ": "
                    << TypeCode(flat->hdr.type & 0x7f7f7f00)
                    << " = " << flat->binder;
            }
        }
    } else {
        to << "NULL";
    }

    to << ")";
}

void Parcel::releaseObjects()
{
    const sp<ProcessState> proc(ProcessState::self());
    size_t i = mObjectsSize;
    uint8_t* const data = mData;
    binder_size_t* const objects = mObjects;
    while (i > 0) {
        i--;
        const flat_binder_object* flat
            = reinterpret_cast<flat_binder_object*>(data+objects[i]);
        release_object(proc, *flat, this);
    }
}

void Parcel::acquireObjects()
{
    const sp<ProcessState> proc(ProcessState::self());
    size_t i = mObjectsSize;
    uint8_t* const data = mData;
    binder_size_t* const objects = mObjects;
    while (i > 0) {
        i--;
        const binder_object_header* flat
            = reinterpret_cast<binder_object_header*>(data+objects[i]);
        acquire_object(proc, *flat, this);
    }
}

void Parcel::freeData()
{
    freeDataNoInit();
    initState();
}

void Parcel::freeDataNoInit()
{
    if (mOwner) {
        LOG_ALLOC("Parcel %p: freeing other owner data", this);
        //ALOGI("Freeing data ref of %p (pid=%d)", this, getpid());
        mOwner(this, mData, mDataSize, mObjects, mObjectsSize, mOwnerCookie);
    } else {
        LOG_ALLOC("Parcel %p: freeing allocated data", this);
        releaseObjects();
        if (mData) {
            LOG_ALLOC("Parcel %p: freeing with %zu capacity", this, mDataCapacity);
            gParcelGlobalAllocSize -= mDataCapacity;
            gParcelGlobalAllocCount--;
            if (mDeallocZero) {
                zeroMemory(mData, mDataSize);
            }
            free(mData);
        }
        if (mObjects) free(mObjects);
    }
}

status_t Parcel::growData(size_t len)
{
    if (len > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }
    if (len > SIZE_MAX - mDataSize) return NO_MEMORY; // overflow
    if (mDataSize + len > SIZE_MAX / 3) return NO_MEMORY; // overflow
    size_t newSize = ((mDataSize+len)*3)/2;
    return continueWrite(newSize);
}

static uint8_t* reallocZeroFree(uint8_t* data, size_t oldCapacity, size_t newCapacity, bool zero) {
    if (!zero) {
        return (uint8_t*)realloc(data, newCapacity);
    }
    uint8_t* newData = (uint8_t*)malloc(newCapacity);
    if (!newData) {
        return nullptr;
    }

    memcpy(newData, data, std::min(oldCapacity, newCapacity));
    zeroMemory(data, oldCapacity);
    free(data);
    return newData;
}

status_t Parcel::restartWrite(size_t desired)
{
    if (desired > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    if (mOwner) {
        freeData();
        return continueWrite(desired);
    }

    uint8_t* data = reallocZeroFree(mData, mDataCapacity, desired, mDeallocZero);
    if (!data && desired > mDataCapacity) {
        mError = NO_MEMORY;
        return NO_MEMORY;
    }

    releaseObjects();

    if (data || desired == 0) {
        LOG_ALLOC("Parcel %p: restart from %zu to %zu capacity", this, mDataCapacity, desired);
        if (mDataCapacity > desired) {
            gParcelGlobalAllocSize -= (mDataCapacity - desired);
        } else {
            gParcelGlobalAllocSize += (desired - mDataCapacity);
        }

        if (!mData) {
            gParcelGlobalAllocCount++;
        }
        mData = data;
        mDataCapacity = desired;
    }

    mDataSize = mDataPos = 0;
    ALOGV("restartWrite Setting data size of %p to %zu", this, mDataSize);
    ALOGV("restartWrite Setting data pos of %p to %zu", this, mDataPos);

    free(mObjects);
    mObjects = nullptr;
    mObjectsSize = mObjectsCapacity = 0;
    mNextObjectHint = 0;
    mHasFds = false;
    clearCache();
    mFdsKnown = true;
    mAllowFds = true;

    return NO_ERROR;
}

status_t Parcel::continueWrite(size_t desired)
{
    if (desired > INT32_MAX) {
        // don't accept size_t values which may have come from an
        // inadvertent conversion from a negative int.
        return BAD_VALUE;
    }

    // If shrinking, first adjust for any objects that appear
    // after the new data size.
    size_t objectsSize = mObjectsSize;
    if (desired < mDataSize) {
        if (desired == 0) {
            objectsSize = 0;
        } else {
            while (objectsSize > 0) {
                if (mObjects[objectsSize-1] < desired)
                    break;
                objectsSize--;
            }
        }
    }

    if (mOwner) {
        // If the size is going to zero, just release the owner's data.
        if (desired == 0) {
            freeData();
            return NO_ERROR;
        }

        // If there is a different owner, we need to take
        // posession.
        uint8_t* data = (uint8_t*)malloc(desired);
        if (!data) {
            mError = NO_MEMORY;
            return NO_MEMORY;
        }
        binder_size_t* objects = nullptr;

        if (objectsSize) {
            objects = (binder_size_t*)calloc(objectsSize, sizeof(binder_size_t));
            if (!objects) {
                free(data);

                mError = NO_MEMORY;
                return NO_MEMORY;
            }

            // Little hack to only acquire references on objects
            // we will be keeping.
            size_t oldObjectsSize = mObjectsSize;
            mObjectsSize = objectsSize;
            acquireObjects();
            mObjectsSize = oldObjectsSize;
        }

        if (mData) {
            memcpy(data, mData, mDataSize < desired ? mDataSize : desired);
        }
        if (objects && mObjects) {
            memcpy(objects, mObjects, objectsSize*sizeof(binder_size_t));
        }
        //ALOGI("Freeing data ref of %p (pid=%d)", this, getpid());
        mOwner(this, mData, mDataSize, mObjects, mObjectsSize, mOwnerCookie);
        mOwner = nullptr;

        LOG_ALLOC("Parcel %p: taking ownership of %zu capacity", this, desired);
        gParcelGlobalAllocSize += desired;
        gParcelGlobalAllocCount++;

        mData = data;
        mObjects = objects;
        mDataSize = (mDataSize < desired) ? mDataSize : desired;
        ALOGV("continueWrite Setting data size of %p to %zu", this, mDataSize);
        mDataCapacity = desired;
        mObjectsSize = mObjectsCapacity = objectsSize;
        mNextObjectHint = 0;

        clearCache();
    } else if (mData) {
        if (objectsSize < mObjectsSize) {
            // Need to release refs on any objects we are dropping.
            const sp<ProcessState> proc(ProcessState::self());
            for (size_t i=objectsSize; i<mObjectsSize; i++) {
                const flat_binder_object* flat
                    = reinterpret_cast<flat_binder_object*>(mData+mObjects[i]);
                if (flat->hdr.type == BINDER_TYPE_FD) {
                    // will need to rescan because we may have lopped off the only FDs
                    mFdsKnown = false;
                }
                release_object(proc, *flat, this);
            }

            if (objectsSize == 0) {
                free(mObjects);
                mObjects = nullptr;
            } else {
                binder_size_t* objects =
                    (binder_size_t*)realloc(mObjects, objectsSize*sizeof(binder_size_t));
                if (objects) {
                    mObjects = objects;
                }
            }
            mObjectsSize = objectsSize;
            mNextObjectHint = 0;

            clearCache();
        }

        // We own the data, so we can just do a realloc().
        if (desired > mDataCapacity) {
            uint8_t* data = reallocZeroFree(mData, mDataCapacity, desired, mDeallocZero);
            if (data) {
                LOG_ALLOC("Parcel %p: continue from %zu to %zu capacity", this, mDataCapacity,
                        desired);
                gParcelGlobalAllocSize += desired;
                gParcelGlobalAllocSize -= mDataCapacity;
                mData = data;
                mDataCapacity = desired;
            } else {
                mError = NO_MEMORY;
                return NO_MEMORY;
            }
        } else {
            if (mDataSize > desired) {
                mDataSize = desired;
                ALOGV("continueWrite Setting data size of %p to %zu", this, mDataSize);
            }
            if (mDataPos > desired) {
                mDataPos = desired;
                ALOGV("continueWrite Setting data pos of %p to %zu", this, mDataPos);
            }
        }

    } else {
        // This is the first data.  Easy!
        uint8_t* data = (uint8_t*)malloc(desired);
        if (!data) {
            mError = NO_MEMORY;
            return NO_MEMORY;
        }

        if(!(mDataCapacity == 0 && mObjects == nullptr
             && mObjectsCapacity == 0)) {
            ALOGE("continueWrite: %zu/%p/%zu/%zu", mDataCapacity, mObjects, mObjectsCapacity, desired);
        }

        LOG_ALLOC("Parcel %p: allocating with %zu capacity", this, desired);
        gParcelGlobalAllocSize += desired;
        gParcelGlobalAllocCount++;

        mData = data;
        mDataSize = mDataPos = 0;
        ALOGV("continueWrite Setting data size of %p to %zu", this, mDataSize);
        ALOGV("continueWrite Setting data pos of %p to %zu", this, mDataPos);
        mDataCapacity = desired;
    }

    return NO_ERROR;
}

void Parcel::initState()
{
    LOG_ALLOC("Parcel %p: initState", this);
    mError = NO_ERROR;
    mData = nullptr;
    mDataSize = 0;
    mDataCapacity = 0;
    mDataPos = 0;
    ALOGV("initState Setting data size of %p to %zu", this, mDataSize);
    ALOGV("initState Setting data pos of %p to %zu", this, mDataPos);
    mObjects = nullptr;
    mObjectsSize = 0;
    mObjectsCapacity = 0;
    mNextObjectHint = 0;
    mHasFds = false;
    mFdsKnown = true;
    mAllowFds = true;
    mDeallocZero = false;
    mOwner = nullptr;
    clearCache();

    // racing multiple init leads only to multiple identical write
    if (gMaxFds == 0) {
        struct rlimit result;
        if (!getrlimit(RLIMIT_NOFILE, &result)) {
            gMaxFds = (size_t)result.rlim_cur;
            //ALOGI("parcel fd limit set to %zu", gMaxFds);
        } else {
            ALOGW("Unable to getrlimit: %s", strerror(errno));
            gMaxFds = 1024;
        }
    }
}

void Parcel::scanForFds() const
{
    bool hasFds = false;
    for (size_t i=0; i<mObjectsSize; i++) {
        const flat_binder_object* flat
            = reinterpret_cast<const flat_binder_object*>(mData + mObjects[i]);
        if (flat->hdr.type == BINDER_TYPE_FD) {
            hasFds = true;
            break;
        }
    }
    mHasFds = hasFds;
    mFdsKnown = true;
}

} // namespace hardware
} // namespace android
