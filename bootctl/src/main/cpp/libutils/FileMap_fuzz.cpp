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

#include "android-base/file.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "utils/FileMap.h"

static constexpr uint16_t MAX_STR_SIZE = 256;
static constexpr uint8_t MAX_FILENAME_SIZE = 32;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    TemporaryFile tf;
    // Generate file contents
    std::string contents = dataProvider.ConsumeRandomLengthString(MAX_STR_SIZE);
    // If we have string contents, dump them into the file.
    // Otherwise, just leave it as an empty file.
    if (contents.length() > 0) {
        const char* bytes = contents.c_str();
        android::base::WriteStringToFd(bytes, tf.fd);
    }
    android::FileMap m;
    // Generate create() params
    std::string orig_name = dataProvider.ConsumeRandomLengthString(MAX_FILENAME_SIZE);
    size_t length = dataProvider.ConsumeIntegralInRange<size_t>(1, SIZE_MAX);
    off64_t offset = dataProvider.ConsumeIntegralInRange<off64_t>(1, INT64_MAX);
    bool read_only = dataProvider.ConsumeBool();
    m.create(orig_name.c_str(), tf.fd, offset, length, read_only);
    m.getDataOffset();
    m.getFileName();
    m.getDataLength();
    m.getDataPtr();
    int enum_index = dataProvider.ConsumeIntegral<int>();
    m.advise(static_cast<android::FileMap::MapAdvice>(enum_index));
    return 0;
}
