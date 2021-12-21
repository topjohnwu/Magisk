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
#include "fuzzer/FuzzedDataProvider.h"
#include "utils/Vector.h"
static constexpr uint16_t MAX_VEC_SIZE = 5000;

void runVectorFuzz(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    android::Vector<uint8_t> vec = android::Vector<uint8_t>();
    // We want to test handling of sizeof as well.
    android::Vector<uint32_t> vec32 = android::Vector<uint32_t>();

    // We're going to generate two vectors of this size
    size_t vectorSize = dataProvider.ConsumeIntegralInRange<size_t>(0, MAX_VEC_SIZE);
    vec.setCapacity(vectorSize);
    vec32.setCapacity(vectorSize);
    for (size_t i = 0; i < vectorSize; i++) {
        uint8_t count = dataProvider.ConsumeIntegralInRange<uint8_t>(1, 5);
        vec.insertAt((uint8_t)i, i, count);
        vec32.insertAt((uint32_t)i, i, count);
        vec.push_front(i);
        vec32.push(i);
    }

    // Now we'll perform some test operations with any remaining data
    // Index to perform operations at
    size_t index = dataProvider.ConsumeIntegralInRange<size_t>(0, vec.size());
    std::vector<uint8_t> remainingVec = dataProvider.ConsumeRemainingBytes<uint8_t>();
    // Insert an array and vector
    vec.insertArrayAt(remainingVec.data(), index, remainingVec.size());
    android::Vector<uint8_t> vecCopy = android::Vector<uint8_t>(vec);
    vec.insertVectorAt(vecCopy, index);
    // Same thing for 32 bit vector
    android::Vector<uint32_t> vec32Copy = android::Vector<uint32_t>(vec32);
    vec32.insertArrayAt(vec32Copy.array(), index, vec32.size());
    vec32.insertVectorAt(vec32Copy, index);
    // Replace single character
    if (remainingVec.size() > 0) {
        vec.replaceAt(remainingVec[0], index);
        vec32.replaceAt(static_cast<uint32_t>(remainingVec[0]), index);
    } else {
        vec.replaceAt(0, index);
        vec32.replaceAt(0, index);
    }
    // Add any remaining bytes
    for (uint8_t i : remainingVec) {
        vec.add(i);
        vec32.add(static_cast<uint32_t>(i));
    }
    // Shrink capactiy
    vec.setCapacity(remainingVec.size());
    vec32.setCapacity(remainingVec.size());
    // Iterate through each pointer
    size_t sum = 0;
    for (auto& it : vec) {
        sum += it;
    }
    for (auto& it : vec32) {
        sum += it;
    }
    // Cleanup
    vec.clear();
    vecCopy.clear();
    vec32.clear();
    vec32Copy.clear();
}
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    runVectorFuzz(data, size);
    return 0;
}
