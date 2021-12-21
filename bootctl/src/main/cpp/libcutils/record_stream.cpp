/* libs/cutils/record_stream.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include <cutils/record_stream.h>

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#if defined(_WIN32)
#include <winsock2.h>   /* for ntohl */
#else
#include <netinet/in.h>
#endif

#define HEADER_SIZE 4

struct RecordStream {
    int fd;
    size_t maxRecordLen;

    unsigned char *buffer;

    unsigned char *unconsumed;
    unsigned char *read_end;
    unsigned char *buffer_end;
};


extern RecordStream *record_stream_new(int fd, size_t maxRecordLen)
{
    RecordStream *ret;

    assert (maxRecordLen <= 0xffff);

    ret = (RecordStream *)calloc(1, sizeof(RecordStream));

    ret->fd = fd;
    ret->maxRecordLen = maxRecordLen;
    ret->buffer = (unsigned char *)malloc (maxRecordLen + HEADER_SIZE);
    
    ret->unconsumed = ret->buffer;
    ret->read_end = ret->buffer;
    ret->buffer_end = ret->buffer + maxRecordLen + HEADER_SIZE;

    return ret;
}


extern void record_stream_free(RecordStream *rs)
{
    free(rs->buffer);
    free(rs);
}


/* returns NULL; if there isn't a full record in the buffer */
static unsigned char * getEndOfRecord (unsigned char *p_begin,
                                            unsigned char *p_end)
{
    size_t len;
    unsigned char * p_ret;

    if (p_end < p_begin + HEADER_SIZE) {
        return NULL;
    }

    //First four bytes are length
    len = ntohl(*((uint32_t *)p_begin));

    p_ret = p_begin + HEADER_SIZE + len;

    if (p_end < p_ret) {
        return NULL;
    }

    return p_ret;
}

static void *getNextRecord (RecordStream *p_rs, size_t *p_outRecordLen)
{
    unsigned char *record_start, *record_end;

    record_end = getEndOfRecord (p_rs->unconsumed, p_rs->read_end);

    if (record_end != NULL) {
        /* one full line in the buffer */
        record_start = p_rs->unconsumed + HEADER_SIZE;
        p_rs->unconsumed = record_end;

        *p_outRecordLen = record_end - record_start;

        return record_start;
    }

    return NULL;
}

/**
 * Reads the next record from stream fd
 * Records are prefixed by a 16-bit big endian length value
 * Records may not be larger than maxRecordLen
 *
 * Doesn't guard against EINTR
 *
 * p_outRecord and p_outRecordLen may not be NULL
 *
 * Return 0 on success, -1 on fail
 * Returns 0 with *p_outRecord set to NULL on end of stream
 * Returns -1 / errno = EAGAIN if it needs to read again
 */
int record_stream_get_next (RecordStream *p_rs, void ** p_outRecord, 
                                    size_t *p_outRecordLen)
{
    void *ret;

    ssize_t countRead;

    /* is there one record already in the buffer? */
    ret = getNextRecord (p_rs, p_outRecordLen);

    if (ret != NULL) {
        *p_outRecord = ret;
        return 0;
    }

    // if the buffer is full and we don't have a full record
    if (p_rs->unconsumed == p_rs->buffer 
        && p_rs->read_end == p_rs->buffer_end
    ) {
        // this should never happen
        //ALOGE("max record length exceeded\n");
        assert (0);
        errno = EFBIG;
        return -1;
    }

    if (p_rs->unconsumed != p_rs->buffer) {
        // move remainder to the beginning of the buffer
        size_t toMove;

        toMove = p_rs->read_end - p_rs->unconsumed;
        if (toMove) {
            memmove(p_rs->buffer, p_rs->unconsumed, toMove);
        }

        p_rs->read_end = p_rs->buffer + toMove;
        p_rs->unconsumed = p_rs->buffer;
    }

    countRead = read (p_rs->fd, p_rs->read_end, p_rs->buffer_end - p_rs->read_end);

    if (countRead <= 0) {
        /* note: end-of-stream drops through here too */
        *p_outRecord = NULL;
        return countRead;
    }

    p_rs->read_end += countRead;

    ret = getNextRecord (p_rs, p_outRecordLen);

    if (ret == NULL) {
        /* not enough of a buffer to for a whole command */
        errno = EAGAIN;
        return -1;
    }

    *p_outRecord = ret;        
    return 0;
}
