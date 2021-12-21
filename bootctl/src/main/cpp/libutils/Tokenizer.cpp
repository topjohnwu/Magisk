/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "Tokenizer"

#include <utils/Tokenizer.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utils/Log.h>

// Enables debug output for the tokenizer.
#define DEBUG_TOKENIZER 0


namespace android {

static inline bool isDelimiter(char ch, const char* delimiters) {
    return strchr(delimiters, ch) != nullptr;
}

Tokenizer::Tokenizer(const String8& filename, FileMap* fileMap, char* buffer,
        bool ownBuffer, size_t length) :
        mFilename(filename), mFileMap(fileMap),
        mBuffer(buffer), mOwnBuffer(ownBuffer), mLength(length),
        mCurrent(buffer), mLineNumber(1) {
}

Tokenizer::~Tokenizer() {
    delete mFileMap;
    if (mOwnBuffer) {
        delete[] mBuffer;
    }
}

status_t Tokenizer::open(const String8& filename, Tokenizer** outTokenizer) {
    *outTokenizer = nullptr;

    int result = OK;
    int fd = ::open(filename.string(), O_RDONLY);
    if (fd < 0) {
        result = -errno;
        ALOGE("Error opening file '%s': %s", filename.string(), strerror(errno));
    } else {
        struct stat stat;
        if (fstat(fd, &stat)) {
            result = -errno;
            ALOGE("Error getting size of file '%s': %s", filename.string(), strerror(errno));
        } else {
            size_t length = size_t(stat.st_size);

            FileMap* fileMap = new FileMap();
            bool ownBuffer = false;
            char* buffer;
            if (fileMap->create(nullptr, fd, 0, length, true)) {
                fileMap->advise(FileMap::SEQUENTIAL);
                buffer = static_cast<char*>(fileMap->getDataPtr());
            } else {
                delete fileMap;
                fileMap = nullptr;

                // Fall back to reading into a buffer since we can't mmap files in sysfs.
                // The length we obtained from stat is wrong too (it will always be 4096)
                // so we must trust that read will read the entire file.
                buffer = new char[length];
                ownBuffer = true;
                ssize_t nrd = read(fd, buffer, length);
                if (nrd < 0) {
                    result = -errno;
                    ALOGE("Error reading file '%s': %s", filename.string(), strerror(errno));
                    delete[] buffer;
                    buffer = nullptr;
                } else {
                    length = size_t(nrd);
                }
            }

            if (!result) {
                *outTokenizer = new Tokenizer(filename, fileMap, buffer, ownBuffer, length);
            }
        }
        close(fd);
    }
    return result;
}

status_t Tokenizer::fromContents(const String8& filename,
        const char* contents, Tokenizer** outTokenizer) {
    *outTokenizer = new Tokenizer(filename, nullptr,
            const_cast<char*>(contents), false, strlen(contents));
    return OK;
}

String8 Tokenizer::getLocation() const {
    String8 result;
    result.appendFormat("%s:%d", mFilename.string(), mLineNumber);
    return result;
}

String8 Tokenizer::peekRemainderOfLine() const {
    const char* end = getEnd();
    const char* eol = mCurrent;
    while (eol != end) {
        char ch = *eol;
        if (ch == '\n') {
            break;
        }
        eol += 1;
    }
    return String8(mCurrent, eol - mCurrent);
}

String8 Tokenizer::nextToken(const char* delimiters) {
#if DEBUG_TOKENIZER
    ALOGD("nextToken");
#endif
    const char* end = getEnd();
    const char* tokenStart = mCurrent;
    while (mCurrent != end) {
        char ch = *mCurrent;
        if (ch == '\n' || isDelimiter(ch, delimiters)) {
            break;
        }
        mCurrent += 1;
    }
    return String8(tokenStart, mCurrent - tokenStart);
}

void Tokenizer::nextLine() {
#if DEBUG_TOKENIZER
    ALOGD("nextLine");
#endif
    const char* end = getEnd();
    while (mCurrent != end) {
        char ch = *(mCurrent++);
        if (ch == '\n') {
            mLineNumber += 1;
            break;
        }
    }
}

void Tokenizer::skipDelimiters(const char* delimiters) {
#if DEBUG_TOKENIZER
    ALOGD("skipDelimiters");
#endif
    const char* end = getEnd();
    while (mCurrent != end) {
        char ch = *mCurrent;
        if (ch == '\n' || !isDelimiter(ch, delimiters)) {
            break;
        }
        mCurrent += 1;
    }
}

} // namespace android
