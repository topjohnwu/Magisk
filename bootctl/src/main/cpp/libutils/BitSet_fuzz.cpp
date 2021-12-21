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
#include <functional>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/BitSet.h"
static constexpr uint8_t MAX_OPERATIONS = 50;

// We need to handle both 32 and 64 bit bitsets, so we use a function template
// here. Sadly, std::function can't be generic, so we generate a vector of
// std::functions using this function.
template <typename T>
std::vector<std::function<void(T, uint32_t)>> getOperationsForType() {
    return {
            [](T bs, uint32_t val) -> void { bs.markBit(val); },
            [](T bs, uint32_t val) -> void { bs.valueForBit(val); },
            [](T bs, uint32_t val) -> void { bs.hasBit(val); },
            [](T bs, uint32_t val) -> void { bs.clearBit(val); },
            [](T bs, uint32_t val) -> void { bs.getIndexOfBit(val); },
            [](T bs, uint32_t) -> void { bs.clearFirstMarkedBit(); },
            [](T bs, uint32_t) -> void { bs.markFirstUnmarkedBit(); },
            [](T bs, uint32_t) -> void { bs.clearLastMarkedBit(); },
            [](T bs, uint32_t) -> void { bs.clear(); },
            [](T bs, uint32_t) -> void { bs.count(); },
            [](T bs, uint32_t) -> void { bs.isEmpty(); },
            [](T bs, uint32_t) -> void { bs.isFull(); },
            [](T bs, uint32_t) -> void { bs.firstMarkedBit(); },
            [](T bs, uint32_t) -> void { bs.lastMarkedBit(); },
    };
}

// Our operations for 32 and 64 bit bitsets
static const std::vector<std::function<void(android::BitSet32, uint32_t)>> thirtyTwoBitOps =
        getOperationsForType<android::BitSet32>();
static const std::vector<std::function<void(android::BitSet64, uint32_t)>> sixtyFourBitOps =
        getOperationsForType<android::BitSet64>();

void runOperationFor32Bit(android::BitSet32 bs, uint32_t bit, uint8_t operation) {
    thirtyTwoBitOps[operation](bs, bit);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    uint32_t thirty_two_base = dataProvider.ConsumeIntegral<uint32_t>();
    uint64_t sixty_four_base = dataProvider.ConsumeIntegral<uint64_t>();
    android::BitSet32 b1 = android::BitSet32(thirty_two_base);
    android::BitSet64 b2 = android::BitSet64(sixty_four_base);

    size_t opsRun = 0;
    while (dataProvider.remaining_bytes() > 0 && opsRun++ < MAX_OPERATIONS) {
        uint32_t bit = dataProvider.ConsumeIntegral<uint32_t>();
        uint8_t op = dataProvider.ConsumeIntegral<uint8_t>();
        thirtyTwoBitOps[op % thirtyTwoBitOps.size()](b1, bit);
        sixtyFourBitOps[op % sixtyFourBitOps.size()](b2, bit);
    }
    return 0;
}
