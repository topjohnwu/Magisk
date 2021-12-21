/*
 * Copyright (C) 2005 The Android Open Source Project
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

#include "TextOutput.h"

#include <hwbinder/Debug.h>

#include <utils/String8.h>
#include <utils/String16.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace android {
namespace hardware {

// ---------------------------------------------------------------------------

TextOutput::TextOutput() {
}

TextOutput::~TextOutput() {
}

// ---------------------------------------------------------------------------

static void textOutputPrinter(void* cookie, const char* txt)
{
    ((TextOutput*)cookie)->print(txt, strlen(txt));
}

TextOutput& operator<<(TextOutput& to, const TypeCode& val)
{
    printTypeCode(val.typeCode(), textOutputPrinter, (void*)&to);
    return to;
}

HexDump::HexDump(const void *buf, size_t size, size_t bytesPerLine)
    : mBuffer(buf)
    , mSize(size)
    , mBytesPerLine(bytesPerLine)
    , mSingleLineCutoff(16)
    , mAlignment(4)
    , mCArrayStyle(false)
{
    if (bytesPerLine >= 16) mAlignment = 4;
    else if (bytesPerLine >= 8) mAlignment = 2;
    else mAlignment = 1;
}

TextOutput& operator<<(TextOutput& to, const HexDump& val)
{
    printHexData(0, val.buffer(), val.size(), val.bytesPerLine(),
        val.singleLineCutoff(), val.alignment(), val.carrayStyle(),
        textOutputPrinter, (void*)&to);
    return to;
}

} // namespace hardware
} // namespace android
