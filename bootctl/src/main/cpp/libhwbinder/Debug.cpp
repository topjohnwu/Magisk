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

#include <hwbinder/Debug.h>
#include <hwbinder/ProcessState.h>

#include <utils/misc.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

namespace android {
namespace hardware {

// ---------------------------------------------------------------------

static const char indentStr[] =
"                                                                            "
"                                                                            ";

const char* stringForIndent(int32_t indentLevel)
{
    ssize_t off = sizeof(indentStr)-1-(indentLevel*2);
    return indentStr + (off < 0 ? 0 : off);
}

// ---------------------------------------------------------------------

static void defaultPrintFunc(void* /*cookie*/, const char* txt)
{
    printf("%s", txt);
}

// ---------------------------------------------------------------------

static inline int isident(int c)
{
    return isalnum(c) || c == '_';
}

static inline bool isasciitype(char c)
{
    if( c >= ' ' && c < 127 && c != '\'' && c != '\\' ) return true;
    return false;
}

static inline char makehexdigit(uint32_t val)
{
    return "0123456789abcdef"[val&0xF];
}

static char* appendhexnum(uint32_t val, char* out)
{
    for( int32_t i=28; i>=0; i-=4 ) {
        *out++ = makehexdigit( val>>i );
    }
    *out = 0;
    return out;
}

static char* appendcharornum(char c, char* out, bool skipzero = true)
{
    if (skipzero && c == 0) return out;

    if (isasciitype(c)) {
        *out++ = c;
        return out;
    }

    *out++ = '\\';
    *out++ = 'x';
    *out++ = makehexdigit(c>>4);
    *out++ = makehexdigit(c);
    return out;
}

static char* typetostring(uint32_t type, char* out,
                          bool fullContext = true,
                          bool strict = false)
{
    char* pos = out;
    char c[4];
    c[0] = (char)((type>>24)&0xFF);
    c[1] = (char)((type>>16)&0xFF);
    c[2] = (char)((type>>8)&0xFF);
    c[3] = (char)(type&0xFF);
    bool valid;
    if( !strict ) {
        // now even less strict!
        // valid = isasciitype(c[3]);
        valid = true;
        int32_t i = 0;
        bool zero = true;
        while (valid && i<3) {
            if (c[i] == 0) {
                if (!zero) valid = false;
            } else {
                zero = false;
                //if (!isasciitype(c[i])) valid = false;
            }
            i++;
        }
        // if all zeros, not a valid type code.
        if (zero) valid = false;
    } else {
        valid = isident(c[3]) ? true : false;
        int32_t i = 0;
        bool zero = true;
        while (valid && i<3) {
            if (c[i] == 0) {
                if (!zero) valid = false;
            } else {
                zero = false;
                if (!isident(c[i])) valid = false;
            }
            i++;
        }
    }
    if( valid && (!fullContext || c[0] != '0' || c[1] != 'x') ) {
        if( fullContext ) *pos++ = '\'';
        pos = appendcharornum(c[0], pos);
        pos = appendcharornum(c[1], pos);
        pos = appendcharornum(c[2], pos);
        pos = appendcharornum(c[3], pos);
        if( fullContext ) *pos++ = '\'';
        *pos = 0;
        return pos;
    }

    if( fullContext ) {
        *pos++ = '0';
        *pos++ = 'x';
    }
    return appendhexnum(type, pos);
}

void printTypeCode(uint32_t typeCode, debugPrintFunc func, void* cookie)
{
    char buffer[32];
    char* end = typetostring(typeCode, buffer);
    *end = 0;
    func ? (*func)(cookie, buffer) : defaultPrintFunc(cookie, buffer);
}

void printHexData(int32_t indent, const void *buf, size_t length,
    size_t bytesPerLine, int32_t singleLineBytesCutoff,
    size_t alignment, bool cStyle,
    debugPrintFunc func, void* cookie)
{
    if (alignment == 0) {
        if (bytesPerLine >= 16) alignment = 4;
        else if (bytesPerLine >= 8) alignment = 2;
        else alignment = 1;
    }
    if (func == nullptr) func = defaultPrintFunc;

    size_t offset;

    unsigned char *pos = (unsigned char *)buf;

    if (pos == nullptr) {
        if (singleLineBytesCutoff < 0) func(cookie, "\n");
        func(cookie, "(NULL)");
        return;
    }

    if (length == 0) {
        if (singleLineBytesCutoff < 0) func(cookie, "\n");
        func(cookie, "(empty)");
        return;
    }

    if ((int32_t)length < 0) {
        if (singleLineBytesCutoff < 0) func(cookie, "\n");
        char buf[64];
        sprintf(buf, "(bad length: %zu)", length);
        func(cookie, buf);
        return;
    }

    char buffer[256];
    static const size_t maxBytesPerLine = (sizeof(buffer)-1-11-4)/(3+1);

    if (bytesPerLine > maxBytesPerLine) bytesPerLine = maxBytesPerLine;

    const bool oneLine = (int32_t)length <= singleLineBytesCutoff;
    bool newLine = false;
    if (cStyle) {
        indent++;
        func(cookie, "{\n");
        newLine = true;
    } else if (!oneLine) {
        func(cookie, "\n");
        newLine = true;
    }

    for (offset = 0; ; offset += bytesPerLine, pos += bytesPerLine) {
        long remain = length;

        char* c = buffer;
        if (!oneLine && !cStyle) {
            sprintf(c, "0x%08x: ", (int)offset);
            c += 12;
        }

        size_t index;
        size_t word;

        for (word = 0; word < bytesPerLine; ) {

            size_t align_offset = alignment-(alignment?1:0);
            if (remain > 0 && (size_t)remain <= align_offset) {
                align_offset = remain - 1;
            }
            const size_t startIndex = word+align_offset;

            for (index = 0; index < alignment || (alignment == 0 && index < bytesPerLine); index++) {

                if (!cStyle) {
                    if (index == 0 && word > 0 && alignment > 0) {
                        *c++ = ' ';
                    }

                    if (remain-- > 0) {
                        const unsigned char val = *(pos+startIndex-index);
                        *c++ = makehexdigit(val>>4);
                        *c++ = makehexdigit(val);
                    } else if (!oneLine) {
                        *c++ = ' ';
                        *c++ = ' ';
                    }
                } else {
                    if (remain > 0) {
                        if (index == 0 && word > 0) {
                            *c++ = ',';
                            *c++ = ' ';
                        }
                        if (index == 0) {
                            *c++ = '0';
                            *c++ = 'x';
                        }
                        const unsigned char val = *(pos+startIndex-index);
                        *c++ = makehexdigit(val>>4);
                        *c++ = makehexdigit(val);
                        remain--;
                    }
                }
            }

            word += index;
        }

        if (!cStyle) {
            remain = length;
            *c++ = ' ';
            *c++ = '\'';
            for (index = 0; index < bytesPerLine; index++) {

                if (remain-- > 0) {
                    const unsigned char val = pos[index];
                    *c++ = (val >= ' ' && val < 127) ? val : '.';
                } else if (!oneLine) {
                    *c++ = ' ';
                }
            }

            *c++ = '\'';
            if (length > bytesPerLine) *c++ = '\n';
        } else {
            if (remain > 0) *c++ = ',';
            *c++ = '\n';
        }

        if (newLine && indent) func(cookie, stringForIndent(indent));
        *c = 0;
        func(cookie, buffer);
        newLine = true;

        if (length <= bytesPerLine) break;
        length -= bytesPerLine;
    }

    if (cStyle) {
        if (indent > 0) func(cookie, stringForIndent(indent-1));
        func(cookie, "};");
    }
}

ssize_t getHWBinderKernelReferences(size_t count, uintptr_t* buf) {
    sp<ProcessState> proc = ProcessState::selfOrNull();
    if (proc.get() == nullptr) {
        return 0;
    }

    return proc->getKernelReferences(count, buf);
}

} // namespace hardware
} // namespace android

