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

#include <sys/select.h>

#include <iostream>

#include <utils/Looper.h>

#include "Looper_test_pipe.h"
#include "fuzzer/FuzzedDataProvider.h"

using android::Looper;
using android::sp;

// We don't want this to bog down fuzzing
static constexpr int MAX_POLL_DELAY = 50;
static constexpr int MAX_OPERATIONS = 500;

void doNothing() {}
void* doNothingPointer = reinterpret_cast<void*>(doNothing);

static int noopCallback(int, int, void*) {
    return 0;
}

std::vector<std::function<void(FuzzedDataProvider*, sp<Looper>, Pipe)>> operations = {
        [](FuzzedDataProvider* dataProvider, sp<Looper> looper, Pipe) -> void {
            looper->pollOnce(dataProvider->ConsumeIntegralInRange<int>(0, MAX_POLL_DELAY));
        },
        [](FuzzedDataProvider* dataProvider, sp<Looper> looper, Pipe) -> void {
            looper->pollAll(dataProvider->ConsumeIntegralInRange<int>(0, MAX_POLL_DELAY));
        },
        // events and callback are nullptr
        [](FuzzedDataProvider* dataProvider, sp<Looper> looper, Pipe pipeObj) -> void {
            looper->addFd(pipeObj.receiveFd, dataProvider->ConsumeIntegral<int>(),
                          dataProvider->ConsumeIntegral<int>(), nullptr, nullptr);
        },
        // Events is nullptr
        [](FuzzedDataProvider* dataProvider, sp<Looper> looper, Pipe pipeObj) -> void {
            looper->addFd(pipeObj.receiveFd, dataProvider->ConsumeIntegral<int>(),
                          dataProvider->ConsumeIntegral<int>(), noopCallback, nullptr);
        },
        // callback is nullptr
        [](FuzzedDataProvider* dataProvider, sp<Looper> looper, Pipe pipeObj) -> void {
            looper->addFd(pipeObj.receiveFd, dataProvider->ConsumeIntegral<int>(),
                          dataProvider->ConsumeIntegral<int>(), nullptr, doNothingPointer);
        },
        // callback and events both set
        [](FuzzedDataProvider* dataProvider, sp<Looper> looper, Pipe pipeObj) -> void {
            looper->addFd(pipeObj.receiveFd, dataProvider->ConsumeIntegral<int>(),
                          dataProvider->ConsumeIntegral<int>(), noopCallback, doNothingPointer);
        },

        [](FuzzedDataProvider*, sp<Looper> looper, Pipe) -> void { looper->wake(); },
        [](FuzzedDataProvider*, sp<Looper>, Pipe pipeObj) -> void { pipeObj.writeSignal(); }};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    Pipe pipeObj;
    FuzzedDataProvider dataProvider(data, size);
    sp<Looper> looper = new Looper(dataProvider.ConsumeBool());

    size_t opsRun = 0;
    while (dataProvider.remaining_bytes() > 0 && opsRun++ < MAX_OPERATIONS) {
        uint8_t op = dataProvider.ConsumeIntegralInRange<uint8_t>(0, operations.size() - 1);
        operations[op](&dataProvider, looper, pipeObj);
    }
    // Clear our pointer
    looper.clear();
    return 0;
}
