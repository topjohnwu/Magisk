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

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <utils/String8.h>

#include <utils/Compat.h>
#include <utils/Log.h>
#include <utils/String16.h>

#include <ctype.h>

#include <string>

#include "SharedBuffer.h"

/*
 * Functions outside android is below the namespace android, since they use
 * functions and constants in android namespace.
 */

// ---------------------------------------------------------------------------

namespace android {

// Separator used by resource paths. This is not platform dependent contrary
// to OS_PATH_SEPARATOR.
#define RES_PATH_SEPARATOR '/'

static inline char* getEmptyString() {
    static SharedBuffer* gEmptyStringBuf = [] {
        SharedBuffer* buf = SharedBuffer::alloc(1);
        char* str = static_cast<char*>(buf->data());
        *str = 0;
        return buf;
    }();

    gEmptyStringBuf->acquire();
    return static_cast<char*>(gEmptyStringBuf->data());
}

// ---------------------------------------------------------------------------

static char* allocFromUTF8(const char* in, size_t len)
{
    if (len > 0) {
        if (len == SIZE_MAX) {
            return nullptr;
        }
        SharedBuffer* buf = SharedBuffer::alloc(len+1);
        ALOG_ASSERT(buf, "Unable to allocate shared buffer");
        if (buf) {
            char* str = (char*)buf->data();
            memcpy(str, in, len);
            str[len] = 0;
            return str;
        }
        return nullptr;
    }

    return getEmptyString();
}

static char* allocFromUTF16(const char16_t* in, size_t len)
{
    if (len == 0) return getEmptyString();

     // Allow for closing '\0'
    const ssize_t resultStrLen = utf16_to_utf8_length(in, len) + 1;
    if (resultStrLen < 1) {
        return getEmptyString();
    }

    SharedBuffer* buf = SharedBuffer::alloc(resultStrLen);
    ALOG_ASSERT(buf, "Unable to allocate shared buffer");
    if (!buf) {
        return getEmptyString();
    }

    char* resultStr = (char*)buf->data();
    utf16_to_utf8(in, len, resultStr, resultStrLen);
    return resultStr;
}

static char* allocFromUTF32(const char32_t* in, size_t len)
{
    if (len == 0) {
        return getEmptyString();
    }

    const ssize_t resultStrLen = utf32_to_utf8_length(in, len) + 1;
    if (resultStrLen < 1) {
        return getEmptyString();
    }

    SharedBuffer* buf = SharedBuffer::alloc(resultStrLen);
    ALOG_ASSERT(buf, "Unable to allocate shared buffer");
    if (!buf) {
        return getEmptyString();
    }

    char* resultStr = (char*) buf->data();
    utf32_to_utf8(in, len, resultStr, resultStrLen);

    return resultStr;
}

// ---------------------------------------------------------------------------

String8::String8()
    : mString(getEmptyString())
{
}

String8::String8(const String8& o)
    : mString(o.mString)
{
    SharedBuffer::bufferFromData(mString)->acquire();
}

String8::String8(const char* o)
    : mString(allocFromUTF8(o, strlen(o)))
{
    if (mString == nullptr) {
        mString = getEmptyString();
    }
}

String8::String8(const char* o, size_t len)
    : mString(allocFromUTF8(o, len))
{
    if (mString == nullptr) {
        mString = getEmptyString();
    }
}

String8::String8(const String16& o)
    : mString(allocFromUTF16(o.string(), o.size()))
{
}

String8::String8(const char16_t* o)
    : mString(allocFromUTF16(o, strlen16(o)))
{
}

String8::String8(const char16_t* o, size_t len)
    : mString(allocFromUTF16(o, len))
{
}

String8::String8(const char32_t* o)
    : mString(allocFromUTF32(o, std::char_traits<char32_t>::length(o))) {}

String8::String8(const char32_t* o, size_t len)
    : mString(allocFromUTF32(o, len))
{
}

String8::~String8()
{
    SharedBuffer::bufferFromData(mString)->release();
}

size_t String8::length() const
{
    return SharedBuffer::sizeFromData(mString)-1;
}

String8 String8::format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    String8 result(formatV(fmt, args));

    va_end(args);
    return result;
}

String8 String8::formatV(const char* fmt, va_list args)
{
    String8 result;
    result.appendFormatV(fmt, args);
    return result;
}

void String8::clear() {
    SharedBuffer::bufferFromData(mString)->release();
    mString = getEmptyString();
}

void String8::setTo(const String8& other)
{
    SharedBuffer::bufferFromData(other.mString)->acquire();
    SharedBuffer::bufferFromData(mString)->release();
    mString = other.mString;
}

status_t String8::setTo(const char* other)
{
    const char *newString = allocFromUTF8(other, strlen(other));
    SharedBuffer::bufferFromData(mString)->release();
    mString = newString;
    if (mString) return OK;

    mString = getEmptyString();
    return NO_MEMORY;
}

status_t String8::setTo(const char* other, size_t len)
{
    const char *newString = allocFromUTF8(other, len);
    SharedBuffer::bufferFromData(mString)->release();
    mString = newString;
    if (mString) return OK;

    mString = getEmptyString();
    return NO_MEMORY;
}

status_t String8::setTo(const char16_t* other, size_t len)
{
    const char *newString = allocFromUTF16(other, len);
    SharedBuffer::bufferFromData(mString)->release();
    mString = newString;
    if (mString) return OK;

    mString = getEmptyString();
    return NO_MEMORY;
}

status_t String8::setTo(const char32_t* other, size_t len)
{
    const char *newString = allocFromUTF32(other, len);
    SharedBuffer::bufferFromData(mString)->release();
    mString = newString;
    if (mString) return OK;

    mString = getEmptyString();
    return NO_MEMORY;
}

status_t String8::append(const String8& other)
{
    const size_t otherLen = other.bytes();
    if (bytes() == 0) {
        setTo(other);
        return OK;
    } else if (otherLen == 0) {
        return OK;
    }

    return real_append(other.string(), otherLen);
}

status_t String8::append(const char* other)
{
    return append(other, strlen(other));
}

status_t String8::append(const char* other, size_t otherLen)
{
    if (bytes() == 0) {
        return setTo(other, otherLen);
    } else if (otherLen == 0) {
        return OK;
    }

    return real_append(other, otherLen);
}

status_t String8::appendFormat(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    status_t result = appendFormatV(fmt, args);

    va_end(args);
    return result;
}

status_t String8::appendFormatV(const char* fmt, va_list args)
{
    int n, result = OK;
    va_list tmp_args;

    /* args is undefined after vsnprintf.
     * So we need a copy here to avoid the
     * second vsnprintf access undefined args.
     */
    va_copy(tmp_args, args);
    n = vsnprintf(nullptr, 0, fmt, tmp_args);
    va_end(tmp_args);

    if (n < 0) return UNKNOWN_ERROR;

    if (n > 0) {
        size_t oldLength = length();
        if (n > std::numeric_limits<size_t>::max() - 1 ||
            oldLength > std::numeric_limits<size_t>::max() - n - 1) {
            return NO_MEMORY;
        }
        char* buf = lockBuffer(oldLength + n);
        if (buf) {
            vsnprintf(buf + oldLength, n + 1, fmt, args);
        } else {
            result = NO_MEMORY;
        }
    }
    return result;
}

status_t String8::real_append(const char* other, size_t otherLen) {
    const size_t myLen = bytes();

    SharedBuffer* buf;
    size_t newLen;
    if (__builtin_add_overflow(myLen, otherLen, &newLen) ||
        __builtin_add_overflow(newLen, 1, &newLen) ||
        (buf = SharedBuffer::bufferFromData(mString)->editResize(newLen)) == nullptr) {
        return NO_MEMORY;
    }

    char* str = (char*)buf->data();
    mString = str;
    str += myLen;
    memcpy(str, other, otherLen);
    str[otherLen] = '\0';
    return OK;
}

char* String8::lockBuffer(size_t size)
{
    SharedBuffer* buf = SharedBuffer::bufferFromData(mString)
        ->editResize(size+1);
    if (buf) {
        char* str = (char*)buf->data();
        mString = str;
        return str;
    }
    return nullptr;
}

void String8::unlockBuffer()
{
    unlockBuffer(strlen(mString));
}

status_t String8::unlockBuffer(size_t size)
{
    if (size != this->size()) {
        SharedBuffer* buf = SharedBuffer::bufferFromData(mString)
            ->editResize(size+1);
        if (! buf) {
            return NO_MEMORY;
        }

        char* str = (char*)buf->data();
        str[size] = 0;
        mString = str;
    }

    return OK;
}

ssize_t String8::find(const char* other, size_t start) const
{
    size_t len = size();
    if (start >= len) {
        return -1;
    }
    const char* s = mString+start;
    const char* p = strstr(s, other);
    return p ? p-mString : -1;
}

bool String8::removeAll(const char* other) {
    ssize_t index = find(other);
    if (index < 0) return false;

    char* buf = lockBuffer(size());
    if (!buf) return false; // out of memory

    size_t skip = strlen(other);
    size_t len = size();
    size_t tail = index;
    while (size_t(index) < len) {
        ssize_t next = find(other, index + skip);
        if (next < 0) {
            next = len;
        }

        memmove(buf + tail, buf + index + skip, next - index - skip);
        tail += next - index - skip;
        index = next;
    }
    unlockBuffer(tail);
    return true;
}

void String8::toLower()
{
    const size_t length = size();
    if (length == 0) return;

    char* buf = lockBuffer(length);
    for (size_t i = length; i > 0; --i) {
        *buf = static_cast<char>(tolower(*buf));
        buf++;
    }
    unlockBuffer(length);
}

// ---------------------------------------------------------------------------
// Path functions

void String8::setPathName(const char* name)
{
    setPathName(name, strlen(name));
}

void String8::setPathName(const char* name, size_t len)
{
    char* buf = lockBuffer(len);

    memcpy(buf, name, len);

    // remove trailing path separator, if present
    if (len > 0 && buf[len-1] == OS_PATH_SEPARATOR)
        len--;

    buf[len] = '\0';

    unlockBuffer(len);
}

String8 String8::getPathLeaf(void) const
{
    const char* cp;
    const char*const buf = mString;

    cp = strrchr(buf, OS_PATH_SEPARATOR);
    if (cp == nullptr)
        return String8(*this);
    else
        return String8(cp+1);
}

String8 String8::getPathDir(void) const
{
    const char* cp;
    const char*const str = mString;

    cp = strrchr(str, OS_PATH_SEPARATOR);
    if (cp == nullptr)
        return String8("");
    else
        return String8(str, cp - str);
}

String8 String8::walkPath(String8* outRemains) const
{
    const char* cp;
    const char*const str = mString;
    const char* buf = str;

    cp = strchr(buf, OS_PATH_SEPARATOR);
    if (cp == buf) {
        // don't include a leading '/'.
        buf = buf+1;
        cp = strchr(buf, OS_PATH_SEPARATOR);
    }

    if (cp == nullptr) {
        String8 res = buf != str ? String8(buf) : *this;
        if (outRemains) *outRemains = String8("");
        return res;
    }

    String8 res(buf, cp-buf);
    if (outRemains) *outRemains = String8(cp+1);
    return res;
}

/*
 * Helper function for finding the start of an extension in a pathname.
 *
 * Returns a pointer inside mString, or NULL if no extension was found.
 */
char* String8::find_extension(void) const
{
    const char* lastSlash;
    const char* lastDot;
    const char* const str = mString;

    // only look at the filename
    lastSlash = strrchr(str, OS_PATH_SEPARATOR);
    if (lastSlash == nullptr)
        lastSlash = str;
    else
        lastSlash++;

    // find the last dot
    lastDot = strrchr(lastSlash, '.');
    if (lastDot == nullptr)
        return nullptr;

    // looks good, ship it
    return const_cast<char*>(lastDot);
}

String8 String8::getPathExtension(void) const
{
    char* ext;

    ext = find_extension();
    if (ext != nullptr)
        return String8(ext);
    else
        return String8("");
}

String8 String8::getBasePath(void) const
{
    char* ext;
    const char* const str = mString;

    ext = find_extension();
    if (ext == nullptr)
        return String8(*this);
    else
        return String8(str, ext - str);
}

String8& String8::appendPath(const char* name)
{
    // TODO: The test below will fail for Win32 paths. Fix later or ignore.
    if (name[0] != OS_PATH_SEPARATOR) {
        if (*name == '\0') {
            // nothing to do
            return *this;
        }

        size_t len = length();
        if (len == 0) {
            // no existing filename, just use the new one
            setPathName(name);
            return *this;
        }

        // make room for oldPath + '/' + newPath
        int newlen = strlen(name);

        char* buf = lockBuffer(len+1+newlen);

        // insert a '/' if needed
        if (buf[len-1] != OS_PATH_SEPARATOR)
            buf[len++] = OS_PATH_SEPARATOR;

        memcpy(buf+len, name, newlen+1);
        len += newlen;

        unlockBuffer(len);

        return *this;
    } else {
        setPathName(name);
        return *this;
    }
}

String8& String8::convertToResPath()
{
#if OS_PATH_SEPARATOR != RES_PATH_SEPARATOR
    size_t len = length();
    if (len > 0) {
        char * buf = lockBuffer(len);
        for (char * end = buf + len; buf < end; ++buf) {
            if (*buf == OS_PATH_SEPARATOR)
                *buf = RES_PATH_SEPARATOR;
        }
        unlockBuffer(len);
    }
#endif
    return *this;
}

}; // namespace android
