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

#include <utils/Mutex.h>

#include <gtest/gtest.h>

static android::Mutex mLock;
static int i GUARDED_BY(mLock);

void modifyLockedVariable() REQUIRES(mLock) {
    i = 1;
}

TEST(Mutex, compile) {
    android::Mutex::Autolock _l(mLock);
    i = 0;
    modifyLockedVariable();
}

TEST(Mutex, tryLock) {
    if (mLock.tryLock() != 0) {
        return;
    }
    mLock.unlock();
}

#if defined(__ANDROID__)
TEST(Mutex, timedLock) {
    if (mLock.timedLock(1) != 0) {
        return;
    }
    mLock.unlock();
}
#endif
