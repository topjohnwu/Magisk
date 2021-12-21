/*
 * Copyright (C) 2006 The Android Open Source Project
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

#define LOG_TAG "hw-BufferedTextOutput"

#include <hwbinder/Debug.h>

#include <cutils/atomic.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>

#include "BufferedTextOutput.h"
#include <hwbinder/Static.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------

namespace android {
namespace hardware {

struct BufferedTextOutput::BufferState : public RefBase
{
    explicit BufferState(int32_t _seq)
        : seq(_seq)
        , buffer(nullptr)
        , bufferPos(0)
        , bufferSize(0)
        , atFront(true)
        , indent(0)
        , bundle(0) {
    }
    ~BufferState() {
        free(buffer);
    }

    status_t append(const char* txt, size_t len) {
        if (len > SIZE_MAX - bufferPos) return NO_MEMORY; // overflow
        if ((len+bufferPos) > bufferSize) {
            if ((len + bufferPos) > SIZE_MAX / 3) return NO_MEMORY; // overflow
            size_t newSize = ((len+bufferPos)*3)/2;
            void* b = realloc(buffer, newSize);
            if (!b) return NO_MEMORY;
            buffer = (char*)b;
            bufferSize = newSize;
        }
        memcpy(buffer+bufferPos, txt, len);
        bufferPos += len;
        return NO_ERROR;
    }

    void restart() {
        bufferPos = 0;
        atFront = true;
        if (bufferSize > 256) {
            void* b = realloc(buffer, 256);
            if (b) {
                buffer = (char*)b;
                bufferSize = 256;
            }
        }
    }

    const int32_t seq;
    char* buffer;
    size_t bufferPos;
    size_t bufferSize;
    bool atFront;
    int32_t indent;
    int32_t bundle;
};

struct BufferedTextOutput::ThreadState
{
    Vector<sp<BufferedTextOutput::BufferState> > states;
};

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

static volatile int32_t gSequence = 0;

static volatile int32_t gFreeBufferIndex = -1;

static int32_t allocBufferIndex()
{
    int32_t res = -1;

    pthread_mutex_lock(&gMutex);

    if (gFreeBufferIndex >= 0) {
        res = gFreeBufferIndex;
        gFreeBufferIndex = gTextBuffers[res];
        gTextBuffers.editItemAt(res) = -1;

    } else {
        res = gTextBuffers.size();
        gTextBuffers.add(-1);
    }

    pthread_mutex_unlock(&gMutex);

    return res;
}

static void freeBufferIndex(int32_t idx)
{
    pthread_mutex_lock(&gMutex);
    gTextBuffers.editItemAt(idx) = gFreeBufferIndex;
    gFreeBufferIndex = idx;
    pthread_mutex_unlock(&gMutex);
}

// ---------------------------------------------------------------------------

BufferedTextOutput::BufferedTextOutput(uint32_t flags)
    : mFlags(flags)
    , mSeq(android_atomic_inc(&gSequence))
    , mIndex(allocBufferIndex())
{
    mGlobalState = new BufferState(mSeq);
    if (mGlobalState) mGlobalState->incStrong(this);
}

BufferedTextOutput::~BufferedTextOutput()
{
    if (mGlobalState) mGlobalState->decStrong(this);
    freeBufferIndex(mIndex);
}

status_t BufferedTextOutput::print(const char* txt, size_t len)
{
    AutoMutex _l(mLock);
    BufferState* b = getBuffer();
    const char* const end = txt+len;
    status_t err;

    while (txt < end) {
        // Find the next line.
        const char* first = txt;
        while (txt < end && *txt != '\n') txt++;

        // Include this and all following empty lines.
        while (txt < end && *txt == '\n') txt++;

        // Special cases for first data on a line.
        if (b->atFront) {
            if (b->indent > 0) {
                // If this is the start of a line, add the indent.
                const char* prefix = stringForIndent(b->indent);
                err = b->append(prefix, strlen(prefix));
                if (err != NO_ERROR) return err;
            } else if (*(txt-1) == '\n' && !b->bundle) {
                // Fast path: if we are not indenting or bundling, and
                // have been given one or more complete lines, just write
                // them out without going through the buffer.

                // Slurp up all of the lines.
                const char* lastLine = txt+1;
                while (txt < end) {
                    if (*txt++ == '\n') lastLine = txt;
                }
                struct iovec vec;
                vec.iov_base = (void*)first;
                vec.iov_len = lastLine-first;
                //printf("Writing %d bytes of data!\n", vec.iov_len);
                writeLines(vec, 1);
                txt = lastLine;
                continue;
            }
        }

        // Append the new text to the buffer.
        err = b->append(first, txt-first);
        if (err != NO_ERROR) return err;
        b->atFront = *(txt-1) == '\n';

        // If we have finished a line and are not bundling, write
        // it out.
        //printf("Buffer is now %d bytes\n", b->bufferPos);
        if (b->atFront && !b->bundle) {
            struct iovec vec;
            vec.iov_base = b->buffer;
            vec.iov_len = b->bufferPos;
            //printf("Writing %d bytes of data!\n", vec.iov_len);
            writeLines(vec, 1);
            b->restart();
        }
    }

    return NO_ERROR;
}

void BufferedTextOutput::moveIndent(int delta)
{
    AutoMutex _l(mLock);
    BufferState* b = getBuffer();
    b->indent += delta;
    if (b->indent < 0) b->indent = 0;
}

void BufferedTextOutput::pushBundle()
{
    AutoMutex _l(mLock);
    BufferState* b = getBuffer();
    b->bundle++;
}

void BufferedTextOutput::popBundle()
{
    AutoMutex _l(mLock);
    BufferState* b = getBuffer();
    b->bundle--;
    LOG_FATAL_IF(b->bundle < 0,
        "TextOutput::popBundle() called more times than pushBundle()");
    if (b->bundle < 0) b->bundle = 0;

    if (b->bundle == 0) {
        // Last bundle, write out data if it is complete.  If it is not
        // complete, don't write until the last line is done... this may
        // or may not be the write thing to do, but it's the easiest.
        if (b->bufferPos > 0 && b->atFront) {
            struct iovec vec;
            vec.iov_base = b->buffer;
            vec.iov_len = b->bufferPos;
            writeLines(vec, 1);
            b->restart();
        }
    }
}

BufferedTextOutput::BufferState* BufferedTextOutput::getBuffer() const
{
    if ((mFlags&MULTITHREADED) != 0) {
        thread_local ThreadState ts;
        while (ts.states.size() <= (size_t)mIndex) ts.states.add(nullptr);
        BufferState* bs = ts.states[mIndex].get();
        if (bs != nullptr && bs->seq == mSeq) return bs;

        ts.states.editItemAt(mIndex) = new BufferState(mIndex);
        bs = ts.states[mIndex].get();
        if (bs != nullptr) return bs;
    }

    return mGlobalState;
}

} // namespace hardware
} // namespace android
