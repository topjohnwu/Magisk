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

#include <atomic>
#include <thread>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/ProcessCallStack.h"
using android::ProcessCallStack;

static constexpr int MAX_NAME_SIZE = 1000;
static constexpr int MAX_LOG_META_SIZE = 1000;
static constexpr uint8_t MAX_THREADS = 10;

std::atomic_bool ranCallStackUpdate(false);
void loop() {
    while (!ranCallStackUpdate.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void spawnThreads(FuzzedDataProvider* dataProvider) {
    std::vector<std::thread> threads = std::vector<std::thread>();

    // Get the number of threads to generate
    uint8_t count = dataProvider->ConsumeIntegralInRange<uint8_t>(1, MAX_THREADS);

    // Generate threads
    for (uint8_t i = 0; i < count; i++) {
        std::string threadName =
                dataProvider->ConsumeRandomLengthString(MAX_NAME_SIZE).append(std::to_string(i));
        std::thread th = std::thread(loop);
        pthread_setname_np(th.native_handle(), threadName.c_str());
        threads.push_back(move(th));
    }

    // Collect thread information
    ProcessCallStack callStack = ProcessCallStack();
    callStack.update();

    // Tell our patiently waiting threads they can be done now.
    ranCallStackUpdate.store(true);

    std::string logTag = dataProvider->ConsumeRandomLengthString(MAX_LOG_META_SIZE);
    std::string prefix = dataProvider->ConsumeRandomLengthString(MAX_LOG_META_SIZE);
    // Both of these, along with dump, all call print() under the hood,
    // Which is covered by the Printer fuzzer.
    callStack.log(logTag.c_str());
    callStack.toString(prefix.c_str());

    // Check size
    callStack.size();

    // wait for any remaining threads
    for (auto& thread : threads) {
        thread.join();
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    spawnThreads(&dataProvider);
    return 0;
}
