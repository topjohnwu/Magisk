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

/*
 * DEPRECATED.  DO NOT USE FOR NEW CODE.
 */

#ifndef ANDROID_SHARED_BUFFER_H
#define ANDROID_SHARED_BUFFER_H

#include <atomic>
#include <stdint.h>
#include <sys/types.h>

// ---------------------------------------------------------------------------

namespace android {

class SharedBuffer
{
public:

    /* flags to use with release() */
    enum {
        eKeepStorage = 0x00000001
    };

    /*! allocate a buffer of size 'size' and acquire() it.
     *  call release() to free it.
     */
    static          SharedBuffer*           alloc(size_t size);
    
    /*! free the memory associated with the SharedBuffer.
     * Fails if there are any users associated with this SharedBuffer.
     * In other words, the buffer must have been release by all its
     * users.
     */
    static          void                    dealloc(const SharedBuffer* released);

    //! access the data for read
    inline          const void*             data() const;
    
    //! access the data for read/write
    inline          void*                   data();

    //! get size of the buffer
    inline          size_t                  size() const;
 
    //! get back a SharedBuffer object from its data
    static  inline  SharedBuffer*           bufferFromData(void* data);
    
    //! get back a SharedBuffer object from its data
    static  inline  const SharedBuffer*     bufferFromData(const void* data);

    //! get the size of a SharedBuffer object from its data
    static  inline  size_t                  sizeFromData(const void* data);
    
    //! edit the buffer (get a writtable, or non-const, version of it)
                    SharedBuffer*           edit() const;

    //! edit the buffer, resizing if needed
                    SharedBuffer*           editResize(size_t size) const;

    //! like edit() but fails if a copy is required
                    SharedBuffer*           attemptEdit() const;
    
    //! resize and edit the buffer, loose it's content.
                    SharedBuffer*           reset(size_t size) const;

    //! acquire/release a reference on this buffer
                    void                    acquire() const;
                    
    /*! release a reference on this buffer, with the option of not
     * freeing the memory associated with it if it was the last reference
     * returns the previous reference count
     */     
                    int32_t                 release(uint32_t flags = 0) const;
    
    //! returns wether or not we're the only owner
    inline          bool                    onlyOwner() const;
    

private:
        inline SharedBuffer() { }
        inline ~SharedBuffer() { }
        SharedBuffer(const SharedBuffer&);
        SharedBuffer& operator = (const SharedBuffer&);
 
        // Must be sized to preserve correct alignment.
        mutable std::atomic<int32_t>        mRefs;
                size_t                      mSize;
                uint32_t                    mReserved;
public:
        // mClientMetadata is reserved for client use.  It is initialized to 0
        // and the clients can do whatever they want with it.  Note that this is
        // placed last so that it is adjcent to the buffer allocated.
                uint32_t                    mClientMetadata;
};

static_assert(sizeof(SharedBuffer) % 8 == 0
        && (sizeof(size_t) > 4 || sizeof(SharedBuffer) == 16),
        "SharedBuffer has unexpected size");

// ---------------------------------------------------------------------------

const void* SharedBuffer::data() const {
    return this + 1;
}

void* SharedBuffer::data() {
    return this + 1;
}

size_t SharedBuffer::size() const {
    return mSize;
}

SharedBuffer* SharedBuffer::bufferFromData(void* data) {
    return data ? static_cast<SharedBuffer *>(data)-1 : nullptr;
}
    
const SharedBuffer* SharedBuffer::bufferFromData(const void* data) {
    return data ? static_cast<const SharedBuffer *>(data)-1 : nullptr;
}

size_t SharedBuffer::sizeFromData(const void* data) {
    return data ? bufferFromData(data)->mSize : 0;
}

bool SharedBuffer::onlyOwner() const {
    return (mRefs.load(std::memory_order_acquire) == 1);
}

}  // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_VECTOR_H
