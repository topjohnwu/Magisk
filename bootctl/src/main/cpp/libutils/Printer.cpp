/*
 * Copyright (C) 2013 The Android Open Source Project
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

#define LOG_TAG "Printer"
// #define LOG_NDEBUG 0

#include <utils/Printer.h>
#include <utils/String8.h>
#include <utils/Log.h>

#include <stdlib.h>

namespace android {

/*
 * Implementation of Printer
 */
Printer::Printer() {
    // Intentionally left empty
}

Printer::~Printer() {
    // Intentionally left empty
}

void Printer::printFormatLine(const char* format, ...) {
    va_list arglist;
    va_start(arglist, format);

    char* formattedString;

#ifndef _WIN32
    if (vasprintf(&formattedString, format, arglist) < 0) { // returns -1 on error
        ALOGE("%s: Failed to format string", __FUNCTION__);
        va_end(arglist);
        return;
    }
#else
    va_end(arglist);
    return;
#endif

    va_end(arglist);

    printLine(formattedString);
    free(formattedString);
}

/*
 * Implementation of LogPrinter
 */
LogPrinter::LogPrinter(const char* logtag,
                       android_LogPriority priority,
                       const char* prefix,
                       bool ignoreBlankLines) :
        mLogTag(logtag),
        mPriority(priority),
        mPrefix(prefix ?: ""),
        mIgnoreBlankLines(ignoreBlankLines) {
}

void LogPrinter::printLine(const char* string) {
    if (string == nullptr) {
        ALOGW("%s: NULL string passed in", __FUNCTION__);
        return;
    }

    if (mIgnoreBlankLines || (*string)) {
        // Simple case: Line is not blank, or we don't care about printing blank lines
        printRaw(string);
    } else {
        // Force logcat to print empty lines by adding prefixing with a space
        printRaw(" ");
    }
}

void LogPrinter::printRaw(const char* string) {
    __android_log_print(mPriority, mLogTag, "%s%s", mPrefix, string);
}


/*
 * Implementation of FdPrinter
 */
FdPrinter::FdPrinter(int fd, unsigned int indent, const char* prefix) :
        mFd(fd), mIndent(indent), mPrefix(prefix ?: "") {

    if (fd < 0) {
        ALOGW("%s: File descriptor out of range (%d)", __FUNCTION__, fd);
    }

    // <indent><prefix><line> -- e.g. '%-4s%s\n' for indent=4
    snprintf(mFormatString, sizeof(mFormatString), "%%-%us%%s\n", mIndent);
}

void FdPrinter::printLine(const char* string) {
    if (string == nullptr) {
        ALOGW("%s: NULL string passed in", __FUNCTION__);
        return;
    } else if (mFd < 0) {
        ALOGW("%s: File descriptor out of range (%d)", __FUNCTION__, mFd);
        return;
    }

#ifndef _WIN32
    dprintf(mFd, mFormatString, mPrefix, string);
#endif
}

/*
 * Implementation of String8Printer
 */
String8Printer::String8Printer(String8* target, const char* prefix) :
        mTarget(target),
        mPrefix(prefix ?: "") {

    if (target == nullptr) {
        ALOGW("%s: Target string was NULL", __FUNCTION__);
    }
}

void String8Printer::printLine(const char* string) {
    if (string == nullptr) {
        ALOGW("%s: NULL string passed in", __FUNCTION__);
        return;
    } else if (mTarget == nullptr) {
        ALOGW("%s: Target string was NULL", __FUNCTION__);
        return;
    }

    mTarget->append(mPrefix);
    mTarget->append(string);
    mTarget->append("\n");
}

/*
 * Implementation of PrefixPrinter
 */
PrefixPrinter::PrefixPrinter(Printer& printer, const char* prefix) :
        mPrinter(printer), mPrefix(prefix ?: "") {
}

void PrefixPrinter::printLine(const char* string) {
    mPrinter.printFormatLine("%s%s", mPrefix, string);
}

}; //namespace android
