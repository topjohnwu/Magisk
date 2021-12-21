/*
 * Copyright 2020 The Android Open Source Project
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

#include <memory.h>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/CallStack.h"

static constexpr int MAX_STRING_SIZE = 500;
static constexpr int MAX_IGNORE_DEPTH = 200;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    size_t ignoreDepth = dataProvider.ConsumeIntegralInRange<size_t>(0, MAX_IGNORE_DEPTH);
    int logPriority = dataProvider.ConsumeIntegral<int>();
    pid_t tid = dataProvider.ConsumeIntegral<pid_t>();
    std::string logTag = dataProvider.ConsumeRandomLengthString(MAX_STRING_SIZE);
    std::string prefix = dataProvider.ConsumeRandomLengthString(MAX_STRING_SIZE);

    const char* logTagChars = logTag.c_str();
    const char* prefixChars = prefix.c_str();

    android::CallStack::CallStackUPtr callStack = android::CallStack::getCurrent(ignoreDepth);
    android::CallStack* callstackPtr = callStack.get();
    android::CallStack::logStack(logTagChars, callstackPtr,
                                 static_cast<android_LogPriority>(logPriority));
    android::CallStack::stackToString(prefixChars);

    callstackPtr->log(logTagChars, static_cast<android_LogPriority>(logPriority), prefixChars);
    callstackPtr->clear();
    callstackPtr->getCurrent(ignoreDepth);
    callstackPtr->log(logTagChars, static_cast<android_LogPriority>(logPriority), prefixChars);
    callstackPtr->update(ignoreDepth, tid);
    callstackPtr->log(logTagChars, static_cast<android_LogPriority>(logPriority), prefixChars);

    return 0;
}
