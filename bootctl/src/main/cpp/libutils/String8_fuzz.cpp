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
#include <iostream>
#include <memory>

#include "FuzzFormatTypes.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "utils/String8.h"

static constexpr int MAX_STRING_BYTES = 256;
static constexpr uint8_t MAX_OPERATIONS = 50;
// Interestingly, 2147483614 (INT32_MAX - 33) seems to be the max value that is handled for format
// flags. Unfortunately we need to use a smaller value so we avoid consuming too much memory.

void fuzzFormat(FuzzedDataProvider* dataProvider, android::String8* str1, bool shouldAppend);
std::vector<std::function<void(FuzzedDataProvider*, android::String8*, android::String8*)>>
        operations = {
                // Bytes and size
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->bytes();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->isEmpty();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->length();
                },

                // Casing
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->toLower();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8* str2) -> void {
                    str1->removeAll(str2->c_str());
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8* str2) -> void {
                    const android::String8& constRef(*str2);
                    str1->compare(constRef);
                },

                // Append and format
                [](FuzzedDataProvider*, android::String8* str1, android::String8* str2) -> void {
                    str1->append(str2->c_str());
                },
                [](FuzzedDataProvider* dataProvider, android::String8* str1, android::String8*)
                        -> void { fuzzFormat(dataProvider, str1, dataProvider->ConsumeBool()); },

                // Find operation
                [](FuzzedDataProvider* dataProvider, android::String8* str1,
                   android::String8* str2) -> void {
                    // We need to get a value from our fuzzer here.
                    int start_index = dataProvider->ConsumeIntegralInRange<int>(0, str1->size());
                    str1->find(str2->c_str(), start_index);
                },

                // Path handling
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->getBasePath();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->getPathExtension();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->getPathLeaf();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->getPathDir();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    str1->convertToResPath();
                },
                [](FuzzedDataProvider*, android::String8* str1, android::String8*) -> void {
                    std::shared_ptr<android::String8> path_out_str =
                            std::make_shared<android::String8>();
                    str1->walkPath(path_out_str.get());
                    path_out_str->clear();
                },
                [](FuzzedDataProvider* dataProvider, android::String8* str1,
                   android::String8*) -> void {
                    str1->setPathName(dataProvider->ConsumeBytesWithTerminator<char>(5).data());
                },
                [](FuzzedDataProvider* dataProvider, android::String8* str1,
                   android::String8*) -> void {
                    str1->appendPath(dataProvider->ConsumeBytesWithTerminator<char>(5).data());
                },
};

void fuzzFormat(FuzzedDataProvider* dataProvider, android::String8* str1, bool shouldAppend) {
    FormatChar formatType = dataProvider->ConsumeEnum<FormatChar>();

    std::string formatString("%");
    // Width specifier
    if (dataProvider->ConsumeBool()) {
        // Left pad with zeroes
        if (dataProvider->ConsumeBool()) {
            formatString.push_back('0');
        }
        // Right justify (or left justify if negative)
        int32_t justify = dataProvider->ConsumeIntegralInRange<int32_t>(-kMaxFormatFlagValue,
                                                                        kMaxFormatFlagValue);
        formatString += std::to_string(justify);
    }

    // The # specifier only works with o, x, X, a, A, e, E, f, F, g, and G
    if (canApplyFlag(formatType, '#') && dataProvider->ConsumeBool()) {
        formatString.push_back('#');
    }

    // Precision specifier
    if (canApplyFlag(formatType, '.') && dataProvider->ConsumeBool()) {
        formatString.push_back('.');
        formatString +=
                std::to_string(dataProvider->ConsumeIntegralInRange<int>(0, kMaxFormatFlagValue));
    }

    formatString.push_back(kFormatChars.at(static_cast<uint8_t>(formatType)));

    switch (formatType) {
        case SIGNED_DECIMAL: {
            int val = dataProvider->ConsumeIntegral<int>();
            if (shouldAppend) {
                str1->appendFormat(formatString.c_str(), val);
            } else {
                str1->format(formatString.c_str(), dataProvider->ConsumeIntegral<int>());
            }
            break;
        }

        case UNSIGNED_DECIMAL:
        case UNSIGNED_OCTAL:
        case UNSIGNED_HEX_LOWER:
        case UNSIGNED_HEX_UPPER: {
            // Unsigned integers for u, o, x, and X
            uint val = dataProvider->ConsumeIntegral<uint>();
            if (shouldAppend) {
                str1->appendFormat(formatString.c_str(), val);
            } else {
                str1->format(formatString.c_str(), val);
            }
            break;
        }

        case FLOAT_LOWER:
        case FLOAT_UPPER:
        case EXPONENT_LOWER:
        case EXPONENT_UPPER:
        case SHORT_EXP_LOWER:
        case SHORT_EXP_UPPER:
        case HEX_FLOAT_LOWER:
        case HEX_FLOAT_UPPER: {
            // Floating points for f, F, e, E, g, G, a, and A
            float val = dataProvider->ConsumeFloatingPoint<float>();
            if (shouldAppend) {
                str1->appendFormat(formatString.c_str(), val);
            } else {
                str1->format(formatString.c_str(), val);
            }
            break;
        }

        case CHAR: {
            char val = dataProvider->ConsumeIntegral<char>();
            if (shouldAppend) {
                str1->appendFormat(formatString.c_str(), val);
            } else {
                str1->format(formatString.c_str(), val);
            }
            break;
        }

        case STRING: {
            std::string val = dataProvider->ConsumeRandomLengthString(MAX_STRING_BYTES);
            if (shouldAppend) {
                str1->appendFormat(formatString.c_str(), val.c_str());
            } else {
                str1->format(formatString.c_str(), val.c_str());
            }
            break;
        }
        case POINTER: {
            uintptr_t val = dataProvider->ConsumeIntegral<uintptr_t>();
            if (shouldAppend) {
                str1->appendFormat(formatString.c_str(), val);
            } else {
                str1->format(formatString.c_str(), val);
            }
            break;
        }
    }
}

void callFunc(uint8_t index, FuzzedDataProvider* dataProvider, android::String8* str1,
              android::String8* str2) {
    operations[index](dataProvider, str1, str2);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    // Generate vector lengths
    const size_t kVecOneLen = dataProvider.ConsumeIntegralInRange<size_t>(1, MAX_STRING_BYTES);
    const size_t kVecTwoLen = dataProvider.ConsumeIntegralInRange<size_t>(1, MAX_STRING_BYTES);
    // Populate vectors
    std::vector<char> vec = dataProvider.ConsumeBytesWithTerminator<char>(kVecOneLen);
    std::vector<char> vec_two = dataProvider.ConsumeBytesWithTerminator<char>(kVecTwoLen);
    // Create UTF-8 pointers
    android::String8 str_one_utf8 = android::String8(vec.data());
    android::String8 str_two_utf8 = android::String8(vec_two.data());
    // Run operations against strings
    int opsRun = 0;
    while (dataProvider.remaining_bytes() > 0 && opsRun++ < MAX_OPERATIONS) {
        uint8_t op = dataProvider.ConsumeIntegralInRange<uint8_t>(0, operations.size() - 1);
        operations[op](&dataProvider, &str_one_utf8, &str_two_utf8);
    }
    // Just to be extra sure these can be freed, we're going to explicitly clear
    // them
    str_one_utf8.clear();
    str_two_utf8.clear();
    return 0;
}
