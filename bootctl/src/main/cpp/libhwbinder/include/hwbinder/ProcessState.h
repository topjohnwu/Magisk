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

#ifndef ANDROID_HARDWARE_PROCESS_STATE_H
#define ANDROID_HARDWARE_PROCESS_STATE_H

#include <hwbinder/IBinder.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/String16.h>

#include <utils/threads.h>

#include <pthread.h>

// WARNING: this code is part of libhwbinder, a fork of libbinder. Generally,
// this means that it is only relevant to HIDL. Any AIDL- or libbinder-specific
// code should not try to use these things.

// ---------------------------------------------------------------------------
namespace android {
namespace hardware {

class IPCThreadState;

class ProcessState : public virtual RefBase
{
public:
    static  sp<ProcessState>    self();
    static  sp<ProcessState>    selfOrNull();
    // Note: don't call self() or selfOrNull() before initWithMmapSize()
    // with '0' as an argument, this is the same as selfOrNull
    static  sp<ProcessState>    initWithMmapSize(size_t mmapSize); // size in bytes

            void                startThreadPool();

            sp<IBinder>         getContextObject(const sp<IBinder>& /*caller*/);
                                // only call once, without creating a pool
            void                becomeContextManager();

            sp<IBinder>         getStrongProxyForHandle(int32_t handle);
            wp<IBinder>         getWeakProxyForHandle(int32_t handle);
            void                expungeHandle(int32_t handle, IBinder* binder);

            void                spawnPooledThread(bool isMain);

            status_t            setThreadPoolConfiguration(size_t maxThreads, bool callerJoinsPool);
            status_t            enableOnewaySpamDetection(bool enable);
            size_t              getMaxThreads();
            void                giveThreadPoolName();

            ssize_t             getKernelReferences(size_t count, uintptr_t* buf);
                                // This refcount includes:
                                // 1. Strong references to the node by this  and other processes
                                // 2. Temporary strong references held by the kernel during a
                                //    transaction on the node.
                                // It does NOT include local strong references to the node
            ssize_t             getStrongRefCountForNodeByHandle(int32_t handle);
            size_t              getMmapSize();

            enum class CallRestriction {
                // all calls okay
                NONE,
                // log when calls are blocking
                ERROR_IF_NOT_ONEWAY,
                // abort process on blocking calls
                FATAL_IF_NOT_ONEWAY,
            };
            // Sets calling restrictions for all transactions in this process. This must be called
            // before any threads are spawned.
            void setCallRestriction(CallRestriction restriction);

private:
    static  sp<ProcessState>    init(size_t mmapSize, bool requireMmapSize);

    friend class IPCThreadState;
            explicit            ProcessState(size_t mmapSize);
                                ~ProcessState();

                                ProcessState(const ProcessState& o);
            ProcessState&       operator=(const ProcessState& o);
            String8             makeBinderThreadName();

            struct handle_entry {
                IBinder* binder;
                RefBase::weakref_type* refs;
            };

            handle_entry*       lookupHandleLocked(int32_t handle);

            int                 mDriverFD;
            void*               mVMStart;

            // Protects thread count variable below.
            pthread_mutex_t     mThreadCountLock;
            // Number of binder threads current executing a command.
            size_t              mExecutingThreadsCount;
            // Maximum number for binder threads allowed for this process.
            size_t              mMaxThreads;
            // Time when thread pool was emptied
            int64_t             mStarvationStartTimeMs;

    mutable Mutex               mLock;  // protects everything below.

            Vector<handle_entry>mHandleToObject;

            String8             mRootDir;
            bool                mThreadPoolStarted;
            bool                mSpawnThreadOnStart;
    volatile int32_t            mThreadPoolSeq;
            const size_t        mMmapSize;

            CallRestriction     mCallRestriction;
};

} // namespace hardware
} // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_HARDWARE_PROCESS_STATE_H
