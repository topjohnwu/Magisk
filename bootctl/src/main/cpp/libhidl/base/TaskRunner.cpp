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

#include <hidl/TaskRunner.h>

#include <utils/AndroidThreads.h>
#include "SynchronizedQueue.h"

#include <thread>

namespace android {
namespace hardware {
namespace details {

TaskRunner::TaskRunner() {
}

void TaskRunner::start(size_t limit) {
    mQueue = std::make_shared<SynchronizedQueue<Task>>(limit);
}

TaskRunner::~TaskRunner() {
    if (mQueue) {
        mQueue->push(nullptr);
    }
}

bool TaskRunner::push(const Task &t) {
    if (mQueue == nullptr || !t) {
        return false;
    }

    {
        std::unique_lock<std::mutex> lock = mQueue->lock();

        if (!mQueue->isInitializedLocked()) {
            // Allow the thread to continue running in background;
            // TaskRunner do not care about the std::thread object.
            std::thread{[q = mQueue] {
                androidSetThreadName("HIDL TaskRunner");

                Task nextTask;
                while (!!(nextTask = q->wait_pop())) {
                    nextTask();
                }
            }}.detach();

            mQueue->setInitializedLocked(true);
        }
    }

    return this->mQueue->push(t);
}

} // namespace details
} // namespace hardware
} // namespace android

