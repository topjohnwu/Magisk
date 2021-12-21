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

#include "android-base/file.h"
#include "android/log.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "utils/Printer.h"
#include "utils/String8.h"
static constexpr int MAX_STR_SIZE = 1000;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    android::String8 outStr = android::String8();
    // Line indent/formatting
    uint indent = dataProvider.ConsumeIntegral<uint>();
    std::string prefix = dataProvider.ConsumeRandomLengthString(MAX_STR_SIZE);
    std::string line = dataProvider.ConsumeRandomLengthString(MAX_STR_SIZE);

    // Misc properties
    std::string logTag = dataProvider.ConsumeRandomLengthString(MAX_STR_SIZE);
    android_LogPriority priority =
            static_cast<android_LogPriority>(dataProvider.ConsumeIntegral<int>());
    bool ignoreBlankLines = dataProvider.ConsumeBool();

    TemporaryFile tf;
    android::FdPrinter filePrinter = android::FdPrinter(tf.fd, indent, prefix.c_str());
    android::String8Printer stringPrinter = android::String8Printer(&outStr);
    android::PrefixPrinter printer = android::PrefixPrinter(stringPrinter, prefix.c_str());
    android::LogPrinter logPrinter =
            android::LogPrinter(logTag.c_str(), priority, prefix.c_str(), ignoreBlankLines);

    printer.printLine(line.c_str());
    printer.printFormatLine("%s", line.c_str());
    logPrinter.printLine(line.c_str());
    logPrinter.printFormatLine("%s", line.c_str());
    filePrinter.printLine(line.c_str());
    filePrinter.printFormatLine("%s", line.c_str());
    return 0;
}
