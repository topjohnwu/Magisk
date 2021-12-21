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
#include <iostream>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/String16.h"
static constexpr int MAX_STRING_BYTES = 256;
static constexpr uint8_t MAX_OPERATIONS = 50;

std::vector<std::function<void(FuzzedDataProvider&, android::String16, android::String16)>>
        operations = {

                // Bytes and size
                ([](FuzzedDataProvider&, android::String16 str1, android::String16) -> void {
                    str1.string();
                }),
                ([](FuzzedDataProvider&, android::String16 str1, android::String16) -> void {
                    str1.isStaticString();
                }),
                ([](FuzzedDataProvider&, android::String16 str1, android::String16) -> void {
                    str1.size();
                }),

                // Comparison
                ([](FuzzedDataProvider&, android::String16 str1, android::String16 str2) -> void {
                    str1.startsWith(str2);
                }),
                ([](FuzzedDataProvider&, android::String16 str1, android::String16 str2) -> void {
                    str1.contains(str2.string());
                }),
                ([](FuzzedDataProvider&, android::String16 str1, android::String16 str2) -> void {
                    str1.compare(str2);
                }),

                // Append and format
                ([](FuzzedDataProvider&, android::String16 str1, android::String16 str2) -> void {
                    str1.append(str2);
                }),
                ([](FuzzedDataProvider& dataProvider, android::String16 str1,
                    android::String16 str2) -> void {
                    int pos = dataProvider.ConsumeIntegralInRange<int>(0, str1.size());
                    str1.insert(pos, str2.string());
                }),

                // Find and replace operations
                ([](FuzzedDataProvider& dataProvider, android::String16 str1,
                    android::String16) -> void {
                    char16_t findChar = dataProvider.ConsumeIntegral<char16_t>();
                    str1.findFirst(findChar);
                }),
                ([](FuzzedDataProvider& dataProvider, android::String16 str1,
                    android::String16) -> void {
                    char16_t findChar = dataProvider.ConsumeIntegral<char16_t>();
                    str1.findLast(findChar);
                }),
                ([](FuzzedDataProvider& dataProvider, android::String16 str1,
                    android::String16) -> void {
                    char16_t findChar = dataProvider.ConsumeIntegral<char16_t>();
                    char16_t replaceChar = dataProvider.ConsumeIntegral<char16_t>();
                    str1.replaceAll(findChar, replaceChar);
                }),
};

void callFunc(uint8_t index, FuzzedDataProvider& dataProvider, android::String16 str1,
              android::String16 str2) {
    operations[index](dataProvider, str1, str2);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    // We're generating two char vectors.
    // First, generate lengths.
    const size_t kVecOneLen = dataProvider.ConsumeIntegralInRange<size_t>(1, MAX_STRING_BYTES);
    const size_t kVecTwoLen = dataProvider.ConsumeIntegralInRange<size_t>(1, MAX_STRING_BYTES);

    // Next, populate the vectors
    std::vector<char> vec = dataProvider.ConsumeBytesWithTerminator<char>(kVecOneLen);
    std::vector<char> vec_two = dataProvider.ConsumeBytesWithTerminator<char>(kVecTwoLen);

    // Get pointers to their data
    char* char_one = vec.data();
    char* char_two = vec_two.data();

    // Create UTF16 representations
    android::String16 str_one_utf16 = android::String16(char_one);
    android::String16 str_two_utf16 = android::String16(char_two);

    // Run operations against strings
    int opsRun = 0;
    while (dataProvider.remaining_bytes() > 0 && opsRun++ < MAX_OPERATIONS) {
        uint8_t op = dataProvider.ConsumeIntegralInRange<uint8_t>(0, operations.size() - 1);
        callFunc(op, dataProvider, str_one_utf16, str_two_utf16);
    }

    return 0;
}
