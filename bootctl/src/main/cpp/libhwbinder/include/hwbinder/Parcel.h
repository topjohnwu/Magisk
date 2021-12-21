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

#ifndef ANDROID_HARDWARE_PARCEL_H
#define ANDROID_HARDWARE_PARCEL_H

#include <string>
#include <vector>

#include <cutils/native_handle.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String16.h>

#include <hwbinder/IInterface.h>

// WARNING: this code is part of libhwbinder, a fork of libbinder. Generally,
// this means that it is only relevant to HIDL. Any AIDL- or libbinder-specific
// code should not try to use these things.

struct binder_buffer_object;
struct flat_binder_object;

// ---------------------------------------------------------------------------
namespace android {
namespace hardware {

#ifdef BINDER_IPC_32BIT
typedef unsigned int binder_size_t;
typedef unsigned int binder_uintptr_t;
#else
typedef unsigned long long binder_size_t;
typedef unsigned long long binder_uintptr_t;
#endif

class IBinder;
class IPCThreadState;
class ProcessState;
class TextOutput;

class Parcel {
    friend class IPCThreadState;
public:

                        Parcel();
                        ~Parcel();

    const uint8_t*      data() const;
    size_t              dataSize() const;
    size_t              dataAvail() const;
    size_t              dataPosition() const;
    size_t              dataCapacity() const;

    status_t            setDataSize(size_t size);
    void                setDataPosition(size_t pos) const;
    status_t            setDataCapacity(size_t size);

    status_t            setData(const uint8_t* buffer, size_t len);

    // Zeros data when reallocating. Other mitigations may be added
    // in the future.
    //
    // WARNING: some read methods may make additional copies of data.
    // In order to verify this, heap dumps should be used.
    void                markSensitive() const;

    // Writes the RPC header.
    status_t            writeInterfaceToken(const char* interface);

    // Parses the RPC header, returning true if the interface name
    // in the header matches the expected interface from the caller.
    bool                enforceInterface(const char* interface) const;

    void                freeData();

private:
    const binder_size_t* objects() const;

public:
    size_t              objectsCount() const;

    status_t            errorCheck() const;
    void                setError(status_t err);

    status_t            write(const void* data, size_t len);
    void*               writeInplace(size_t len);
    status_t            writeUnpadded(const void* data, size_t len);
    status_t            writeInt8(int8_t val);
    status_t            writeUint8(uint8_t val);
    status_t            writeInt16(int16_t val);
    status_t            writeUint16(uint16_t val);
    status_t            writeInt32(int32_t val);
    status_t            writeUint32(uint32_t val);
    status_t            writeInt64(int64_t val);
    status_t            writeUint64(uint64_t val);
    status_t            writeFloat(float val);
    status_t            writeDouble(double val);
    status_t            writeCString(const char* str);
    status_t            writeString16(const String16& str);
    status_t            writeString16(const std::unique_ptr<String16>& str);
    status_t            writeString16(const char16_t* str, size_t len);
    status_t            writeStrongBinder(const sp<IBinder>& val);
    status_t            writeBool(bool val);

    template<typename T>
    status_t            writeObject(const T& val);

    status_t            writeBuffer(const void *buffer, size_t length, size_t *handle);
    status_t            writeEmbeddedBuffer(const void *buffer, size_t length, size_t *handle,
                            size_t parent_buffer_handle, size_t parent_offset);
public:
    status_t            writeEmbeddedNativeHandle(const native_handle_t *handle,
                            size_t parent_buffer_handle, size_t parent_offset);
    status_t            writeNativeHandleNoDup(const native_handle* handle, bool embedded,
                                               size_t parent_buffer_handle = 0,
                                               size_t parent_offset = 0);
    status_t            writeNativeHandleNoDup(const native_handle* handle);

    status_t            read(void* outData, size_t len) const;
    const void*         readInplace(size_t len) const;
    status_t            readInt8(int8_t *pArg) const;
    status_t            readUint8(uint8_t *pArg) const;
    status_t            readInt16(int16_t *pArg) const;
    status_t            readUint16(uint16_t *pArg) const;
    int32_t             readInt32() const;
    status_t            readInt32(int32_t *pArg) const;
    uint32_t            readUint32() const;
    status_t            readUint32(uint32_t *pArg) const;
    int64_t             readInt64() const;
    status_t            readInt64(int64_t *pArg) const;
    uint64_t            readUint64() const;
    status_t            readUint64(uint64_t *pArg) const;
    float               readFloat() const;
    status_t            readFloat(float *pArg) const;
    double              readDouble() const;
    status_t            readDouble(double *pArg) const;

    bool                readBool() const;
    status_t            readBool(bool *pArg) const;
    const char*         readCString() const;
    String16            readString16() const;
    status_t            readString16(String16* pArg) const;
    status_t            readString16(std::unique_ptr<String16>* pArg) const;
    const char16_t*     readString16Inplace(size_t* outLen) const;
    sp<IBinder>         readStrongBinder() const;
    status_t            readStrongBinder(sp<IBinder>* val) const;
    status_t            readNullableStrongBinder(sp<IBinder>* val) const;

    template<typename T>
    const T*            readObject(size_t *objects_offset = nullptr) const;

    status_t            readBuffer(size_t buffer_size, size_t *buffer_handle,
                                   const void **buffer_out) const;
    status_t            readNullableBuffer(size_t buffer_size, size_t *buffer_handle,
                                           const void **buffer_out) const;
    status_t            readEmbeddedBuffer(size_t buffer_size, size_t *buffer_handle,
                                           size_t parent_buffer_handle, size_t parent_offset,
                                           const void **buffer_out) const;
    status_t            readNullableEmbeddedBuffer(size_t buffer_size,
                                                   size_t *buffer_handle,
                                                   size_t parent_buffer_handle,
                                                   size_t parent_offset,
                                                   const void **buffer_out) const;

    status_t            readEmbeddedNativeHandle(size_t parent_buffer_handle,
                           size_t parent_offset, const native_handle_t **handle) const;
    status_t            readNullableEmbeddedNativeHandle(size_t parent_buffer_handle,
                           size_t parent_offset, const native_handle_t **handle) const;
    status_t            readNativeHandleNoDup(const native_handle_t **handle) const;
    status_t            readNullableNativeHandleNoDup(const native_handle_t **handle) const;

    // Explicitly close all file descriptors in the parcel.
    void                closeFileDescriptors();

    // Debugging: get metrics on current allocations.
    static size_t       getGlobalAllocSize();
    static size_t       getGlobalAllocCount();

private:
    // Below is a cache that records some information about all actual buffers
    // in this parcel.
    struct BufferInfo {
        size_t index;
        binder_uintptr_t buffer;
        binder_uintptr_t bufend; // buffer + length
    };
    // value of mObjectSize when mBufCache is last updated.
    mutable size_t                  mBufCachePos;
    mutable std::vector<BufferInfo> mBufCache;
    // clear mBufCachePos and mBufCache.
    void                clearCache() const;
    // update mBufCache for all objects between mBufCachePos and mObjectsSize
    void                updateCache() const;

    bool                verifyBufferObject(const binder_buffer_object *buffer_obj,
                                           size_t size, uint32_t flags, size_t parent,
                                           size_t parentOffset) const;

    status_t            readBuffer(size_t buffer_size, size_t *buffer_handle,
                                   uint32_t flags, size_t parent, size_t parentOffset,
                                   const void **buffer_out) const;

    status_t            readNullableNativeHandleNoDup(const native_handle_t **handle,
                                                      bool embedded,
                                                      size_t parent_buffer_handle = 0,
                                                      size_t parent_offset = 0) const;
public:

    // The following two methods attempt to find if a chunk of memory ("buffer")
    // is written / read before (by (read|write)(Embedded)?Buffer methods. )
    // 1. Call findBuffer if the chunk of memory could be a small part of a larger
    //    buffer written before (for example, an element of a hidl_vec). The
    //    method will also ensure that the end address (ptr + length) is also
    //    within the buffer.
    // 2. Call quickFindBuffer if the buffer could only be written previously
    //    by itself (for example, the mBuffer field of a hidl_vec). No lengths
    //    are checked.
    status_t            findBuffer(const void *ptr,
                                   size_t length,
                                   bool *found,
                                   size_t *handle,
                                   size_t *offset // valid if found
                                  ) const;
    status_t            quickFindBuffer(const void *ptr,
                                        size_t *handle // valid if found
                                       ) const;

private:
    bool                validateBufferChild(size_t child_buffer_handle,
                                            size_t child_offset) const;
    bool                validateBufferParent(size_t parent_buffer_handle,
                                             size_t parent_offset) const;

private:
    typedef void        (*release_func)(Parcel* parcel,
                                        const uint8_t* data, size_t dataSize,
                                        const binder_size_t* objects, size_t objectsSize,
                                        void* cookie);

    uintptr_t           ipcData() const;
    size_t              ipcDataSize() const;
    uintptr_t           ipcObjects() const;
    size_t              ipcObjectsCount() const;
    size_t              ipcBufferSize() const;
    void                ipcSetDataReference(const uint8_t* data, size_t dataSize,
                                            const binder_size_t* objects, size_t objectsCount,
                                            release_func relFunc, void* relCookie);

public:
    void                print(TextOutput& to, uint32_t flags = 0) const;

private:
                        Parcel(const Parcel& o);
    Parcel&             operator=(const Parcel& o);

    status_t            finishWrite(size_t len);
    void                releaseObjects();
    void                acquireObjects();
    status_t            growData(size_t len);
    status_t            restartWrite(size_t desired);
    status_t            continueWrite(size_t desired);
    status_t            writePointer(uintptr_t val);
    status_t            readPointer(uintptr_t *pArg) const;
    uintptr_t           readPointer() const;
    void                freeDataNoInit();
    void                initState();
    void                scanForFds() const;

    template<class T>
    status_t            readAligned(T *pArg) const;

    template<class T>   T readAligned() const;

    template<class T>
    status_t            writeAligned(T val);

    status_t            mError;
    uint8_t*            mData;
    size_t              mDataSize;
    size_t              mDataCapacity;
    mutable size_t      mDataPos;
    binder_size_t*      mObjects;
    size_t              mObjectsSize;
    size_t              mObjectsCapacity;
    mutable size_t      mNextObjectHint;

    [[deprecated]] size_t mNumRef;

    mutable bool        mFdsKnown;
    mutable bool        mHasFds;
    bool                mAllowFds;

    // if this parcelable is involved in a secure transaction, force the
    // data to be overridden with zero when deallocated
    mutable bool        mDeallocZero;

    release_func        mOwner;
    void*               mOwnerCookie;
};
// ---------------------------------------------------------------------------

inline TextOutput& operator<<(TextOutput& to, const Parcel& parcel)
{
    parcel.print(to);
    return to;
}

// ---------------------------------------------------------------------------

// Generic acquire and release of objects.
void acquire_object(const sp<ProcessState>& proc,
                    const flat_binder_object& obj, const void* who);
void release_object(const sp<ProcessState>& proc,
                    const flat_binder_object& obj, const void* who);

void flatten_binder(const sp<ProcessState>& proc,
                    const sp<IBinder>& binder, flat_binder_object* out);
void flatten_binder(const sp<ProcessState>& proc,
                    const wp<IBinder>& binder, flat_binder_object* out);
status_t unflatten_binder(const sp<ProcessState>& proc,
                          const flat_binder_object& flat, sp<IBinder>* out);
status_t unflatten_binder(const sp<ProcessState>& proc,
                          const flat_binder_object& flat, wp<IBinder>* out);

} // namespace hardware
} // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_HARDWARE_PARCEL_H
