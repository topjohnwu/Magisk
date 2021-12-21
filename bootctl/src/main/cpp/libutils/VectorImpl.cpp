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

#define LOG_TAG "Vector"

#include <utils/VectorImpl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>

#include "SharedBuffer.h"

/*****************************************************************************/


namespace android {

// ----------------------------------------------------------------------------

const size_t kMinVectorCapacity = 4;

static inline size_t max(size_t a, size_t b) {
    return a>b ? a : b;
}

// ----------------------------------------------------------------------------

VectorImpl::VectorImpl(size_t itemSize, uint32_t flags)
    : mStorage(nullptr), mCount(0), mFlags(flags), mItemSize(itemSize)
{
}

VectorImpl::VectorImpl(const VectorImpl& rhs)
    :   mStorage(rhs.mStorage), mCount(rhs.mCount),
        mFlags(rhs.mFlags), mItemSize(rhs.mItemSize)
{
    if (mStorage) {
        SharedBuffer::bufferFromData(mStorage)->acquire();
    }
}

VectorImpl::~VectorImpl()
{
    ALOGW_IF(mCount,
        "[%p] subclasses of VectorImpl must call finish_vector()"
        " in their destructor. Leaking %d bytes.",
        this, (int)(mCount*mItemSize));
    // We can't call _do_destroy() here because the vtable is already gone.
}

VectorImpl& VectorImpl::operator = (const VectorImpl& rhs)
{
    LOG_ALWAYS_FATAL_IF(mItemSize != rhs.mItemSize,
        "Vector<> have different types (this=%p, rhs=%p)", this, &rhs);
    if (this != &rhs) {
        release_storage();
        if (rhs.mCount) {
            mStorage = rhs.mStorage;
            mCount = rhs.mCount;
            SharedBuffer::bufferFromData(mStorage)->acquire();
        } else {
            mStorage = nullptr;
            mCount = 0;
        }
    }
    return *this;
}

void* VectorImpl::editArrayImpl()
{
    if (mStorage) {
        const SharedBuffer* sb = SharedBuffer::bufferFromData(mStorage);
        SharedBuffer* editable = sb->attemptEdit();
        if (editable == nullptr) {
            // If we're here, we're not the only owner of the buffer.
            // We must make a copy of it.
            editable = SharedBuffer::alloc(sb->size());
            // Fail instead of returning a pointer to storage that's not
            // editable. Otherwise we'd be editing the contents of a buffer
            // for which we're not the only owner, which is undefined behaviour.
            LOG_ALWAYS_FATAL_IF(editable == nullptr);
            _do_copy(editable->data(), mStorage, mCount);
            release_storage();
            mStorage = editable->data();
        }
    }
    return mStorage;
}

size_t VectorImpl::capacity() const
{
    if (mStorage) {
        return SharedBuffer::bufferFromData(mStorage)->size() / mItemSize;
    }
    return 0;
}

ssize_t VectorImpl::insertVectorAt(const VectorImpl& vector, size_t index)
{
    return insertArrayAt(vector.arrayImpl(), index, vector.size());
}

ssize_t VectorImpl::appendVector(const VectorImpl& vector)
{
    return insertVectorAt(vector, size());
}

ssize_t VectorImpl::insertArrayAt(const void* array, size_t index, size_t length)
{
    if (index > size())
        return BAD_INDEX;
    void* where = _grow(index, length);
    if (where) {
        _do_copy(where, array, length);
    }
    return where ? index : (ssize_t)NO_MEMORY;
}

ssize_t VectorImpl::appendArray(const void* array, size_t length)
{
    return insertArrayAt(array, size(), length);
}

ssize_t VectorImpl::insertAt(size_t index, size_t numItems)
{
    return insertAt(nullptr, index, numItems);
}

ssize_t VectorImpl::insertAt(const void* item, size_t index, size_t numItems)
{
    if (index > size())
        return BAD_INDEX;
    void* where = _grow(index, numItems);
    if (where) {
        if (item) {
            _do_splat(where, item, numItems);
        } else {
            _do_construct(where, numItems);
        }
    }
    return where ? index : (ssize_t)NO_MEMORY;
}

static int sortProxy(const void* lhs, const void* rhs, void* func)
{
    return (*(VectorImpl::compar_t)func)(lhs, rhs);
}

status_t VectorImpl::sort(VectorImpl::compar_t cmp)
{
    return sort(sortProxy, (void*)cmp);
}

status_t VectorImpl::sort(VectorImpl::compar_r_t cmp, void* state)
{
    // the sort must be stable. we're using insertion sort which
    // is well suited for small and already sorted arrays
    // for big arrays, it could be better to use mergesort
    const ssize_t count = size();
    if (count > 1) {
        void* array = const_cast<void*>(arrayImpl());
        void* temp = nullptr;
        ssize_t i = 1;
        while (i < count) {
            void* item = reinterpret_cast<char*>(array) + mItemSize*(i);
            void* curr = reinterpret_cast<char*>(array) + mItemSize*(i-1);
            if (cmp(curr, item, state) > 0) {

                if (!temp) {
                    // we're going to have to modify the array...
                    array = editArrayImpl();
                    if (!array) return NO_MEMORY;
                    temp = malloc(mItemSize);
                    if (!temp) return NO_MEMORY;
                    item = reinterpret_cast<char*>(array) + mItemSize*(i);
                    curr = reinterpret_cast<char*>(array) + mItemSize*(i-1);
                } else {
                    _do_destroy(temp, 1);
                }

                _do_copy(temp, item, 1);

                ssize_t j = i-1;
                void* next = reinterpret_cast<char*>(array) + mItemSize*(i);
                do {
                    _do_destroy(next, 1);
                    _do_copy(next, curr, 1);
                    next = curr;
                    --j;
                    curr = nullptr;
                    if (j >= 0) {
                        curr = reinterpret_cast<char*>(array) + mItemSize*(j);
                    }
                } while (j>=0 && (cmp(curr, temp, state) > 0));

                _do_destroy(next, 1);
                _do_copy(next, temp, 1);
            }
            i++;
        }

        if (temp) {
            _do_destroy(temp, 1);
            free(temp);
        }
    }
    return OK;
}

void VectorImpl::pop()
{
    if (size())
        removeItemsAt(size()-1, 1);
}

void VectorImpl::push()
{
    push(nullptr);
}

void VectorImpl::push(const void* item)
{
    insertAt(item, size());
}

ssize_t VectorImpl::add()
{
    return add(nullptr);
}

ssize_t VectorImpl::add(const void* item)
{
    return insertAt(item, size());
}

ssize_t VectorImpl::replaceAt(size_t index)
{
    return replaceAt(nullptr, index);
}

ssize_t VectorImpl::replaceAt(const void* prototype, size_t index)
{
    ALOG_ASSERT(index<size(),
        "[%p] replace: index=%d, size=%d", this, (int)index, (int)size());

    if (index >= size()) {
        return BAD_INDEX;
    }

    void* item = editItemLocation(index);
    if (item != prototype) {
        if (item == nullptr)
            return NO_MEMORY;
        _do_destroy(item, 1);
        if (prototype == nullptr) {
            _do_construct(item, 1);
        } else {
            _do_copy(item, prototype, 1);
        }
    }
    return ssize_t(index);
}

ssize_t VectorImpl::removeItemsAt(size_t index, size_t count)
{
    ALOG_ASSERT((index+count)<=size(),
        "[%p] remove: index=%d, count=%d, size=%d",
               this, (int)index, (int)count, (int)size());

    if ((index+count) > size())
        return BAD_VALUE;
   _shrink(index, count);
   return index;
}

void VectorImpl::finish_vector()
{
    release_storage();
    mStorage = nullptr;
    mCount = 0;
}

void VectorImpl::clear()
{
    _shrink(0, mCount);
}

void* VectorImpl::editItemLocation(size_t index)
{
    ALOG_ASSERT(index<capacity(),
        "[%p] editItemLocation: index=%d, capacity=%d, count=%d",
        this, (int)index, (int)capacity(), (int)mCount);

    if (index < capacity()) {
        void* buffer = editArrayImpl();
        if (buffer) {
            return reinterpret_cast<char*>(buffer) + index*mItemSize;
        }
    }
    return nullptr;
}

const void* VectorImpl::itemLocation(size_t index) const
{
    ALOG_ASSERT(index<capacity(),
        "[%p] itemLocation: index=%d, capacity=%d, count=%d",
        this, (int)index, (int)capacity(), (int)mCount);

    if (index < capacity()) {
        const  void* buffer = arrayImpl();
        if (buffer) {
            return reinterpret_cast<const char*>(buffer) + index*mItemSize;
        }
    }
    return nullptr;
}

ssize_t VectorImpl::setCapacity(size_t new_capacity)
{
    // The capacity must always be greater than or equal to the size
    // of this vector.
    if (new_capacity <= size()) {
        return capacity();
    }

    size_t new_allocation_size = 0;
    LOG_ALWAYS_FATAL_IF(__builtin_mul_overflow(new_capacity, mItemSize, &new_allocation_size));
    SharedBuffer* sb = SharedBuffer::alloc(new_allocation_size);
    if (sb) {
        void* array = sb->data();
        _do_copy(array, mStorage, size());
        release_storage();
        mStorage = const_cast<void*>(array);
    } else {
        return NO_MEMORY;
    }
    return new_capacity;
}

ssize_t VectorImpl::resize(size_t size) {
    ssize_t result = OK;
    if (size > mCount) {
        result = insertAt(mCount, size - mCount);
    } else if (size < mCount) {
        result = removeItemsAt(size, mCount - size);
    }
    return result < 0 ? result : size;
}

void VectorImpl::release_storage()
{
    if (mStorage) {
        const SharedBuffer* sb = SharedBuffer::bufferFromData(mStorage);
        if (sb->release(SharedBuffer::eKeepStorage) == 1) {
            _do_destroy(mStorage, mCount);
            SharedBuffer::dealloc(sb);
        }
    }
}

void* VectorImpl::_grow(size_t where, size_t amount)
{
//    ALOGV("_grow(this=%p, where=%d, amount=%d) count=%d, capacity=%d",
//        this, (int)where, (int)amount, (int)mCount, (int)capacity());

    ALOG_ASSERT(where <= mCount,
            "[%p] _grow: where=%d, amount=%d, count=%d",
            this, (int)where, (int)amount, (int)mCount); // caller already checked

    size_t new_size;
    LOG_ALWAYS_FATAL_IF(__builtin_add_overflow(mCount, amount, &new_size), "new_size overflow");

    if (capacity() < new_size) {
        // NOTE: This implementation used to resize vectors as per ((3*x + 1) / 2)
        // (sigh..). Also note, the " + 1" was necessary to handle the special case
        // where x == 1, where the resized_capacity will be equal to the old
        // capacity without the +1. The old calculation wouldn't work properly
        // if x was zero.
        //
        // This approximates the old calculation, using (x + (x/2) + 1) instead.
        size_t new_capacity = 0;
        LOG_ALWAYS_FATAL_IF(__builtin_add_overflow(new_size, (new_size / 2), &new_capacity),
                            "new_capacity overflow");
        LOG_ALWAYS_FATAL_IF(
                __builtin_add_overflow(new_capacity, static_cast<size_t>(1u), &new_capacity),
                "new_capacity overflow");
        new_capacity = max(kMinVectorCapacity, new_capacity);

        size_t new_alloc_size = 0;
        LOG_ALWAYS_FATAL_IF(__builtin_mul_overflow(new_capacity, mItemSize, &new_alloc_size),
                            "new_alloc_size overflow");

        // ALOGV("grow vector %p, new_capacity=%d", this, (int)new_capacity);
        if ((mStorage) &&
            (mCount==where) &&
            (mFlags & HAS_TRIVIAL_COPY) &&
            (mFlags & HAS_TRIVIAL_DTOR))
        {
            const SharedBuffer* cur_sb = SharedBuffer::bufferFromData(mStorage);
            SharedBuffer* sb = cur_sb->editResize(new_alloc_size);
            if (sb) {
                mStorage = sb->data();
            } else {
                return nullptr;
            }
        } else {
            SharedBuffer* sb = SharedBuffer::alloc(new_alloc_size);
            if (sb) {
                void* array = sb->data();
                if (where != 0) {
                    _do_copy(array, mStorage, where);
                }
                if (where != mCount) {
                    const void* from = reinterpret_cast<const uint8_t *>(mStorage) + where*mItemSize;
                    void* dest = reinterpret_cast<uint8_t *>(array) + (where+amount)*mItemSize;
                    _do_copy(dest, from, mCount-where);
                }
                release_storage();
                mStorage = const_cast<void*>(array);
            } else {
                return nullptr;
            }
        }
    } else {
        void* array = editArrayImpl();
        if (where != mCount) {
            const void* from = reinterpret_cast<const uint8_t *>(array) + where*mItemSize;
            void* to = reinterpret_cast<uint8_t *>(array) + (where+amount)*mItemSize;
            _do_move_forward(to, from, mCount - where);
        }
    }
    mCount = new_size;
    void* free_space = const_cast<void*>(itemLocation(where));
    return free_space;
}

void VectorImpl::_shrink(size_t where, size_t amount)
{
    if (!mStorage)
        return;

//    ALOGV("_shrink(this=%p, where=%d, amount=%d) count=%d, capacity=%d",
//        this, (int)where, (int)amount, (int)mCount, (int)capacity());

    ALOG_ASSERT(where + amount <= mCount,
            "[%p] _shrink: where=%d, amount=%d, count=%d",
            this, (int)where, (int)amount, (int)mCount); // caller already checked

    size_t new_size;
    LOG_ALWAYS_FATAL_IF(__builtin_sub_overflow(mCount, amount, &new_size));

    if (new_size < (capacity() / 2)) {
        // NOTE: (new_size * 2) is safe because capacity didn't overflow and
        // new_size < (capacity / 2)).
        const size_t new_capacity = max(kMinVectorCapacity, new_size * 2);

        // NOTE: (new_capacity * mItemSize), (where * mItemSize) and
        // ((where + amount) * mItemSize) beyond this point are safe because
        // we are always reducing the capacity of the underlying SharedBuffer.
        // In other words, (old_capacity * mItemSize) did not overflow, and
        // where < (where + amount) < new_capacity < old_capacity.
        if ((where == new_size) &&
            (mFlags & HAS_TRIVIAL_COPY) &&
            (mFlags & HAS_TRIVIAL_DTOR))
        {
            const SharedBuffer* cur_sb = SharedBuffer::bufferFromData(mStorage);
            SharedBuffer* sb = cur_sb->editResize(new_capacity * mItemSize);
            if (sb) {
                mStorage = sb->data();
            } else {
                return;
            }
        } else {
            SharedBuffer* sb = SharedBuffer::alloc(new_capacity * mItemSize);
            if (sb) {
                void* array = sb->data();
                if (where != 0) {
                    _do_copy(array, mStorage, where);
                }
                if (where != new_size) {
                    const void* from = reinterpret_cast<const uint8_t *>(mStorage) + (where+amount)*mItemSize;
                    void* dest = reinterpret_cast<uint8_t *>(array) + where*mItemSize;
                    _do_copy(dest, from, new_size - where);
                }
                release_storage();
                mStorage = const_cast<void*>(array);
            } else{
                return;
            }
        }
    } else {
        void* array = editArrayImpl();
        void* to = reinterpret_cast<uint8_t *>(array) + where*mItemSize;
        _do_destroy(to, amount);
        if (where != new_size) {
            const void* from = reinterpret_cast<uint8_t *>(array) + (where+amount)*mItemSize;
            _do_move_backward(to, from, new_size - where);
        }
    }
    mCount = new_size;
}

size_t VectorImpl::itemSize() const {
    return mItemSize;
}

void VectorImpl::_do_construct(void* storage, size_t num) const
{
    if (!(mFlags & HAS_TRIVIAL_CTOR)) {
        do_construct(storage, num);
    }
}

void VectorImpl::_do_destroy(void* storage, size_t num) const
{
    if (!(mFlags & HAS_TRIVIAL_DTOR)) {
        do_destroy(storage, num);
    }
}

void VectorImpl::_do_copy(void* dest, const void* from, size_t num) const
{
    if (!(mFlags & HAS_TRIVIAL_COPY)) {
        do_copy(dest, from, num);
    } else {
        memcpy(dest, from, num*itemSize());
    }
}

void VectorImpl::_do_splat(void* dest, const void* item, size_t num) const {
    do_splat(dest, item, num);
}

void VectorImpl::_do_move_forward(void* dest, const void* from, size_t num) const {
    do_move_forward(dest, from, num);
}

void VectorImpl::_do_move_backward(void* dest, const void* from, size_t num) const {
    do_move_backward(dest, from, num);
}

/*****************************************************************************/

SortedVectorImpl::SortedVectorImpl(size_t itemSize, uint32_t flags)
    : VectorImpl(itemSize, flags)
{
}

SortedVectorImpl::SortedVectorImpl(const VectorImpl& rhs)
: VectorImpl(rhs)
{
}

SortedVectorImpl::~SortedVectorImpl()
{
}

SortedVectorImpl& SortedVectorImpl::operator = (const SortedVectorImpl& rhs)
{
    return static_cast<SortedVectorImpl&>( VectorImpl::operator = (static_cast<const VectorImpl&>(rhs)) );
}

ssize_t SortedVectorImpl::indexOf(const void* item) const
{
    return _indexOrderOf(item);
}

size_t SortedVectorImpl::orderOf(const void* item) const
{
    size_t o;
    _indexOrderOf(item, &o);
    return o;
}

ssize_t SortedVectorImpl::_indexOrderOf(const void* item, size_t* order) const
{
    if (order) *order = 0;
    if (isEmpty()) {
        return NAME_NOT_FOUND;
    }
    // binary search
    ssize_t err = NAME_NOT_FOUND;
    ssize_t l = 0;
    ssize_t h = size()-1;
    ssize_t mid;
    const void* a = arrayImpl();
    const size_t s = itemSize();
    while (l <= h) {
        mid = l + (h - l)/2;
        const void* const curr = reinterpret_cast<const char *>(a) + (mid*s);
        const int c = do_compare(curr, item);
        if (c == 0) {
            err = l = mid;
            break;
        } else if (c < 0) {
            l = mid + 1;
        } else {
            h = mid - 1;
        }
    }
    if (order) *order = l;
    return err;
}

ssize_t SortedVectorImpl::add(const void* item)
{
    size_t order;
    ssize_t index = _indexOrderOf(item, &order);
    if (index < 0) {
        index = VectorImpl::insertAt(item, order, 1);
    } else {
        index = VectorImpl::replaceAt(item, index);
    }
    return index;
}

ssize_t SortedVectorImpl::merge(const VectorImpl& vector)
{
    // naive merge...
    if (!vector.isEmpty()) {
        const void* buffer = vector.arrayImpl();
        const size_t is = itemSize();
        size_t s = vector.size();
        for (size_t i=0 ; i<s ; i++) {
            ssize_t err = add( reinterpret_cast<const char*>(buffer) + i*is );
            if (err<0) {
                return err;
            }
        }
    }
    return OK;
}

ssize_t SortedVectorImpl::merge(const SortedVectorImpl& vector)
{
    // we've merging a sorted vector... nice!
    ssize_t err = OK;
    if (!vector.isEmpty()) {
        // first take care of the case where the vectors are sorted together
        if (do_compare(vector.itemLocation(vector.size()-1), arrayImpl()) <= 0) {
            err = VectorImpl::insertVectorAt(static_cast<const VectorImpl&>(vector), 0);
        } else if (do_compare(vector.arrayImpl(), itemLocation(size()-1)) >= 0) {
            err = VectorImpl::appendVector(static_cast<const VectorImpl&>(vector));
        } else {
            // this could be made a little better
            err = merge(static_cast<const VectorImpl&>(vector));
        }
    }
    return err;
}

ssize_t SortedVectorImpl::remove(const void* item)
{
    ssize_t i = indexOf(item);
    if (i>=0) {
        VectorImpl::removeItemsAt(i, 1);
    }
    return i;
}

/*****************************************************************************/

}; // namespace android
