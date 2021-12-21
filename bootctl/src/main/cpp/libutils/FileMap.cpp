/*
 * Copyright (C) 2006 The Android Open Source Project
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

//
// Shared file mapping class.
//

#define LOG_TAG "filemap"

#include <utils/FileMap.h>
#include <utils/Log.h>

#if defined(__MINGW32__) && !defined(__USE_MINGW_ANSI_STDIO)
# define PRId32 "I32d"
# define PRIx32 "I32x"
# define PRId64 "I64d"
#else
#include <inttypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#if !defined(__MINGW32__)
#include <sys/mman.h>
#endif

#include <string.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>

using namespace android;

/*static*/ long FileMap::mPageSize = -1;

// Constructor.  Create an empty object.
FileMap::FileMap(void)
    : mFileName(nullptr),
      mBasePtr(nullptr),
      mBaseLength(0),
      mDataPtr(nullptr),
      mDataLength(0)
#if defined(__MINGW32__)
      ,
      mFileHandle(INVALID_HANDLE_VALUE),
      mFileMapping(NULL)
#endif
{
}

// Move Constructor.
FileMap::FileMap(FileMap&& other) noexcept
    : mFileName(other.mFileName),
      mBasePtr(other.mBasePtr),
      mBaseLength(other.mBaseLength),
      mDataOffset(other.mDataOffset),
      mDataPtr(other.mDataPtr),
      mDataLength(other.mDataLength)
#if defined(__MINGW32__)
      ,
      mFileHandle(other.mFileHandle),
      mFileMapping(other.mFileMapping)
#endif
{
    other.mFileName = nullptr;
    other.mBasePtr = nullptr;
    other.mDataPtr = nullptr;
#if defined(__MINGW32__)
    other.mFileHandle = INVALID_HANDLE_VALUE;
    other.mFileMapping = NULL;
#endif
}

// Move assign operator.
FileMap& FileMap::operator=(FileMap&& other) noexcept {
    mFileName = other.mFileName;
    mBasePtr = other.mBasePtr;
    mBaseLength = other.mBaseLength;
    mDataOffset = other.mDataOffset;
    mDataPtr = other.mDataPtr;
    mDataLength = other.mDataLength;
    other.mFileName = nullptr;
    other.mBasePtr = nullptr;
    other.mDataPtr = nullptr;
#if defined(__MINGW32__)
    mFileHandle = other.mFileHandle;
    mFileMapping = other.mFileMapping;
    other.mFileHandle = INVALID_HANDLE_VALUE;
    other.mFileMapping = NULL;
#endif
    return *this;
}

// Destructor.
FileMap::~FileMap(void)
{
    if (mFileName != nullptr) {
        free(mFileName);
    }
#if defined(__MINGW32__)
    if (mBasePtr && UnmapViewOfFile(mBasePtr) == 0) {
        ALOGD("UnmapViewOfFile(%p) failed, error = %lu\n", mBasePtr,
              GetLastError() );
    }
    if (mFileMapping != NULL) {
        CloseHandle(mFileMapping);
    }
#else
    if (mBasePtr && munmap(mBasePtr, mBaseLength) != 0) {
        ALOGD("munmap(%p, %zu) failed\n", mBasePtr, mBaseLength);
    }
#endif
}


// Create a new mapping on an open file.
//
// Closing the file descriptor does not unmap the pages, so we don't
// claim ownership of the fd.
//
// Returns "false" on failure.
bool FileMap::create(const char* origFileName, int fd, off64_t offset, size_t length,
        bool readOnly)
{
#if defined(__MINGW32__)
    int     adjust;
    off64_t adjOffset;
    size_t  adjLength;

    if (mPageSize == -1) {
        SYSTEM_INFO  si;

        GetSystemInfo( &si );
        mPageSize = si.dwAllocationGranularity;
    }

    DWORD  protect = readOnly ? PAGE_READONLY : PAGE_READWRITE;

    mFileHandle  = (HANDLE) _get_osfhandle(fd);
    mFileMapping = CreateFileMapping( mFileHandle, NULL, protect, 0, 0, NULL);
    if (mFileMapping == NULL) {
        ALOGE("CreateFileMapping(%p, %lx) failed with error %lu\n",
              mFileHandle, protect, GetLastError() );
        return false;
    }

    adjust    = offset % mPageSize;
    adjOffset = offset - adjust;
    adjLength = length + adjust;

    mBasePtr = MapViewOfFile( mFileMapping,
                              readOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
                              0,
                              (DWORD)(adjOffset),
                              adjLength );
    if (mBasePtr == NULL) {
        ALOGE("MapViewOfFile(%" PRId64 ", %zu) failed with error %lu\n",
              adjOffset, adjLength, GetLastError() );
        CloseHandle(mFileMapping);
        mFileMapping = NULL;
        return false;
    }
#else // !defined(__MINGW32__)
    assert(fd >= 0);
    assert(offset >= 0);
    assert(length > 0);

    // init on first use
    if (mPageSize == -1) {
        mPageSize = sysconf(_SC_PAGESIZE);
        if (mPageSize == -1) {
            ALOGE("could not get _SC_PAGESIZE\n");
            return false;
        }
    }

    int adjust = offset % mPageSize;
    off64_t adjOffset = offset - adjust;
    size_t adjLength;
    if (__builtin_add_overflow(length, adjust, &adjLength)) {
        ALOGE("adjusted length overflow: length %zu adjust %d", length, adjust);
        return false;
    }

    int flags = MAP_SHARED;
    int prot = PROT_READ;
    if (!readOnly) prot |= PROT_WRITE;

    void* ptr = mmap64(nullptr, adjLength, prot, flags, fd, adjOffset);
    if (ptr == MAP_FAILED) {
        if (errno == EINVAL && length == 0) {
            ptr = nullptr;
            adjust = 0;
        } else {
            ALOGE("mmap(%lld,%zu) failed: %s\n", (long long)adjOffset, adjLength, strerror(errno));
            return false;
        }
    }
    mBasePtr = ptr;
#endif // !defined(__MINGW32__)

    mFileName = origFileName != nullptr ? strdup(origFileName) : nullptr;
    mBaseLength = adjLength;
    mDataOffset = offset;
    mDataPtr = (char*) mBasePtr + adjust;
    mDataLength = length;

    ALOGV("MAP: base %p/%zu data %p/%zu\n",
        mBasePtr, mBaseLength, mDataPtr, mDataLength);

    return true;
}

// Provide guidance to the system.
#if !defined(_WIN32)
int FileMap::advise(MapAdvice advice)
{
    int cc, sysAdvice;

    switch (advice) {
        case NORMAL:        sysAdvice = MADV_NORMAL;        break;
        case RANDOM:        sysAdvice = MADV_RANDOM;        break;
        case SEQUENTIAL:    sysAdvice = MADV_SEQUENTIAL;    break;
        case WILLNEED:      sysAdvice = MADV_WILLNEED;      break;
        case DONTNEED:      sysAdvice = MADV_DONTNEED;      break;
        default:
                            assert(false);
                            return -1;
    }

    cc = madvise(mBasePtr, mBaseLength, sysAdvice);
    if (cc != 0)
        ALOGW("madvise(%d) failed: %s\n", sysAdvice, strerror(errno));
    return cc;
}

#else
int FileMap::advise(MapAdvice /* advice */)
{
    return -1;
}
#endif
