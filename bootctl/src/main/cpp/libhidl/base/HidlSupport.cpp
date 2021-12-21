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
#define LOG_TAG "HidlSupport"

#include <hidl/HidlSupport.h>

#include <unordered_map>

#include <android-base/logging.h>
#include <android-base/parseint.h>

namespace android {
namespace hardware {

namespace details {
bool debuggable() {
#ifdef LIBHIDL_TARGET_DEBUGGABLE
    return true;
#else
    return false;
#endif
}
}  // namespace details

hidl_handle::hidl_handle() : mHandle(nullptr), mOwnsHandle(false) {
    memset(mPad, 0, sizeof(mPad));
}

hidl_handle::~hidl_handle() {
    freeHandle();
}

hidl_handle::hidl_handle(const native_handle_t* handle) : hidl_handle() {
    mHandle = handle;
    mOwnsHandle = false;
}

// copy constructor.
hidl_handle::hidl_handle(const hidl_handle& other) : hidl_handle() {
    mOwnsHandle = false;
    *this = other;
}

// move constructor.
hidl_handle::hidl_handle(hidl_handle&& other) noexcept : hidl_handle() {
    mOwnsHandle = false;
    *this = std::move(other);
}

// assignment operators
hidl_handle &hidl_handle::operator=(const hidl_handle &other) {
    if (this == &other) {
        return *this;
    }
    freeHandle();
    if (other.mHandle != nullptr) {
        mHandle = native_handle_clone(other.mHandle);
        if (mHandle == nullptr) {
            PLOG(FATAL) << "Failed to clone native_handle in hidl_handle";
        }
        mOwnsHandle = true;
    } else {
        mHandle = nullptr;
        mOwnsHandle = false;
    }
    return *this;
}

hidl_handle &hidl_handle::operator=(const native_handle_t *native_handle) {
    freeHandle();
    mHandle = native_handle;
    mOwnsHandle = false;
    return *this;
}

hidl_handle& hidl_handle::operator=(hidl_handle&& other) noexcept {
    if (this != &other) {
        freeHandle();
        mHandle = other.mHandle;
        mOwnsHandle = other.mOwnsHandle;
        other.mHandle = nullptr;
        other.mOwnsHandle = false;
    }
    return *this;
}

void hidl_handle::setTo(native_handle_t* handle, bool shouldOwn) {
    freeHandle();
    mHandle = handle;
    mOwnsHandle = shouldOwn;
}

const native_handle_t* hidl_handle::operator->() const {
    return mHandle;
}

// implicit conversion to const native_handle_t*
hidl_handle::operator const native_handle_t *() const {
    return mHandle;
}

// explicit conversion
const native_handle_t *hidl_handle::getNativeHandle() const {
    return mHandle;
}

void hidl_handle::freeHandle() {
    if (mOwnsHandle && mHandle != nullptr) {
        // This can only be true if:
        // 1. Somebody called setTo() with shouldOwn=true, so we know the handle
        //    wasn't const to begin with.
        // 2. Copy/assignment from another hidl_handle, in which case we have
        //    cloned the handle.
        // 3. Move constructor from another hidl_handle, in which case the original
        //    hidl_handle must have been non-const as well.
        native_handle_t *handle = const_cast<native_handle_t*>(
                static_cast<const native_handle_t*>(mHandle));
        native_handle_close(handle);
        native_handle_delete(handle);
        mHandle = nullptr;
    }
}

static const char *const kEmptyString = "";

hidl_string::hidl_string() : mBuffer(kEmptyString), mSize(0), mOwnsBuffer(false) {
    memset(mPad, 0, sizeof(mPad));
}

hidl_string::~hidl_string() {
    clear();
}

hidl_string::hidl_string(const char *s) : hidl_string() {
    if (s == nullptr) {
        return;
    }

    copyFrom(s, strlen(s));
}

hidl_string::hidl_string(const char *s, size_t length) : hidl_string() {
    copyFrom(s, length);
}

hidl_string::hidl_string(const hidl_string &other): hidl_string() {
    copyFrom(other.c_str(), other.size());
}

hidl_string::hidl_string(const std::string &s) : hidl_string() {
    copyFrom(s.c_str(), s.size());
}

hidl_string::hidl_string(hidl_string&& other) noexcept : hidl_string() {
    moveFrom(std::forward<hidl_string>(other));
}

hidl_string& hidl_string::operator=(hidl_string&& other) noexcept {
    if (this != &other) {
        clear();
        moveFrom(std::forward<hidl_string>(other));
    }
    return *this;
}

hidl_string &hidl_string::operator=(const hidl_string &other) {
    if (this != &other) {
        clear();
        copyFrom(other.c_str(), other.size());
    }

    return *this;
}

hidl_string &hidl_string::operator=(const char *s) {
    clear();

    if (s == nullptr) {
        return *this;
    }

    copyFrom(s, strlen(s));
    return *this;
}

hidl_string &hidl_string::operator=(const std::string &s) {
    clear();
    copyFrom(s.c_str(), s.size());
    return *this;
}

hidl_string::operator std::string() const {
    return std::string(mBuffer, mSize);
}

std::ostream& operator<<(std::ostream& os, const hidl_string& str) {
    os << str.c_str();
    return os;
}

void hidl_string::copyFrom(const char *data, size_t size) {
    // assume my resources are freed.

    if (size >= UINT32_MAX) {
        LOG(FATAL) << "string size can't exceed 2^32 bytes: " << size;
    }

    if (size == 0) {
        mBuffer = kEmptyString;
        mSize = 0;
        mOwnsBuffer = false;
        return;
    }

    char *buf = (char *)malloc(size + 1);
    memcpy(buf, data, size);
    buf[size] = '\0';
    mBuffer = buf;

    mSize = static_cast<uint32_t>(size);
    mOwnsBuffer = true;
}

void hidl_string::moveFrom(hidl_string &&other) {
    // assume my resources are freed.

    mBuffer = std::move(other.mBuffer);
    mSize = other.mSize;
    mOwnsBuffer = other.mOwnsBuffer;

    other.mOwnsBuffer = false;
    other.clear();
}

void hidl_string::clear() {
    if (mOwnsBuffer && (mBuffer != kEmptyString)) {
        free(const_cast<char *>(static_cast<const char *>(mBuffer)));
    }

    mBuffer = kEmptyString;
    mSize = 0;
    mOwnsBuffer = false;
}

void hidl_string::setToExternal(const char *data, size_t size) {
    if (size > UINT32_MAX) {
        LOG(FATAL) << "string size can't exceed 2^32 bytes: " << size;
    }

    // When the binder driver copies this data into its buffer, it must
    // have a zero byte there because the remote process will have a pointer
    // directly into the read-only binder buffer. If we manually copy the
    // data now to add a zero, then we lose the efficiency of this method.
    // Checking here (it's also checked in the parceling code later).
    CHECK(data[size] == '\0');

    clear();

    mBuffer = data;
    mSize = static_cast<uint32_t>(size);
    mOwnsBuffer = false;
}

const char *hidl_string::c_str() const {
    return mBuffer;
}

size_t hidl_string::size() const {
    return mSize;
}

bool hidl_string::empty() const {
    return mSize == 0;
}

sp<HidlMemory> HidlMemory::getInstance(const hidl_memory& mem) {
    sp<HidlMemory> instance = new HidlMemory();
    instance->hidl_memory::operator=(mem);
    return instance;
}

sp<HidlMemory> HidlMemory::getInstance(hidl_memory&& mem) {
    sp<HidlMemory> instance = new HidlMemory();
    instance->hidl_memory::operator=(std::move(mem));
    return instance;
}

sp<HidlMemory> HidlMemory::getInstance(const hidl_string& name, int fd, uint64_t size) {
    native_handle_t* handle = native_handle_create(1, 0);
    if (!handle) {
        close(fd);
        LOG(ERROR) << "native_handle_create fails";
        return new HidlMemory();
    }
    handle->data[0] = fd;

    hidl_handle hidlHandle;
    hidlHandle.setTo(handle, true /* shouldOwn */);

    sp<HidlMemory> instance = new HidlMemory(name, std::move(hidlHandle), size);
    return instance;
}

HidlMemory::HidlMemory() : hidl_memory() {}

HidlMemory::HidlMemory(const hidl_string& name, hidl_handle&& handle, size_t size)
        : hidl_memory(name, std::move(handle), size) {}

// it's required to have at least one out-of-line method to avoid weak vtable
HidlMemory::~HidlMemory() {}

}  // namespace hardware
}  // namespace android


