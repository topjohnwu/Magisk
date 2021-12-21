/*
 * Copyright (C) 2007-2016 The Android Open Source Project
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

#include "pmsg_writer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <log/log_properties.h>
#include <private/android_logger.h>

#include "logger.h"
#include "uio.h"

static atomic_int pmsg_fd;

static void GetPmsgFd() {
  // Note if open() fails and returns -1, that value is stored into pmsg_fd as an indication that
  // pmsg is not available and open() should not be retried.
  if (pmsg_fd != 0) {
    return;
  }

  int new_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));

  // Unlikely that new_fd is 0, but that is synonymous with our uninitialized value, and we'd prefer
  // STDIN_FILENO != stdin, so we call open() to get a new fd value in this case.
  if (new_fd == 0) {
    new_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));
    close(0);
  }

  // pmsg_fd should only be opened once.  If we see that pmsg_fd is uninitialized, we open
  // "/dev/pmsg0" then attempt to compare/exchange it into pmsg_fd.  If the compare/exchange was
  // successful, then that will be the fd used for the duration of the program, otherwise a
  // different thread has already opened and written the fd to the atomic, so close the new fd and
  // return.
  int uninitialized_value = 0;
  if (!pmsg_fd.compare_exchange_strong(uninitialized_value, new_fd)) {
    if (new_fd != -1) {
      close(new_fd);
    }
  }
}

void PmsgClose() {
  if (pmsg_fd > 0) {
    close(pmsg_fd);
  }
  pmsg_fd = 0;
}

int PmsgWrite(log_id_t logId, struct timespec* ts, struct iovec* vec, size_t nr) {
  static const unsigned headerLength = 2;
  struct iovec newVec[nr + headerLength];
  android_log_header_t header;
  android_pmsg_log_header_t pmsgHeader;
  size_t i, payloadSize;
  ssize_t ret;

  if (!__android_log_is_debuggable()) {
    if (logId != LOG_ID_EVENTS && logId != LOG_ID_SECURITY) {
      return -1;
    }

    if (logId == LOG_ID_EVENTS) {
      if (vec[0].iov_len < 4) {
        return -EINVAL;
      }

      if (SNET_EVENT_LOG_TAG != *static_cast<uint32_t*>(vec[0].iov_base)) {
        return -EPERM;
      }
    }
  }

  GetPmsgFd();

  if (pmsg_fd <= 0) {
    return -EBADF;
  }

  /*
   *  struct {
   *      // what we provide to pstore
   *      android_pmsg_log_header_t pmsgHeader;
   *      // what we provide to file
   *      android_log_header_t header;
   *      // caller provides
   *      union {
   *          struct {
   *              char     prio;
   *              char     payload[];
   *          } string;
   *          struct {
   *              uint32_t tag
   *              char     payload[];
   *          } binary;
   *      };
   *  };
   */

  pmsgHeader.magic = LOGGER_MAGIC;
  pmsgHeader.len = sizeof(pmsgHeader) + sizeof(header);
  pmsgHeader.uid = getuid();
  pmsgHeader.pid = getpid();

  header.id = logId;
  header.tid = gettid();
  header.realtime.tv_sec = ts->tv_sec;
  header.realtime.tv_nsec = ts->tv_nsec;

  newVec[0].iov_base = (unsigned char*)&pmsgHeader;
  newVec[0].iov_len = sizeof(pmsgHeader);
  newVec[1].iov_base = (unsigned char*)&header;
  newVec[1].iov_len = sizeof(header);

  for (payloadSize = 0, i = headerLength; i < nr + headerLength; i++) {
    newVec[i].iov_base = vec[i - headerLength].iov_base;
    payloadSize += newVec[i].iov_len = vec[i - headerLength].iov_len;

    if (payloadSize > LOGGER_ENTRY_MAX_PAYLOAD) {
      newVec[i].iov_len -= payloadSize - LOGGER_ENTRY_MAX_PAYLOAD;
      if (newVec[i].iov_len) {
        ++i;
      }
      payloadSize = LOGGER_ENTRY_MAX_PAYLOAD;
      break;
    }
  }
  pmsgHeader.len += payloadSize;

  ret = TEMP_FAILURE_RETRY(writev(pmsg_fd, newVec, i));
  if (ret < 0) {
    ret = errno ? -errno : -ENOTCONN;
  }

  if (ret > (ssize_t)(sizeof(header) + sizeof(pmsgHeader))) {
    ret -= sizeof(header) - sizeof(pmsgHeader);
  }

  return ret;
}

/*
 * Virtual pmsg filesystem
 *
 * Payload will comprise the string "<basedir>:<basefile>\0<content>" to a
 * maximum of LOGGER_ENTRY_MAX_PAYLOAD, but scaled to the last newline in the
 * file.
 *
 * Will hijack the header.realtime.tv_nsec field for a sequence number in usec.
 */

static inline const char* strnrchr(const char* buf, size_t len, char c) {
  const char* cp = buf + len;
  while ((--cp > buf) && (*cp != c))
    ;
  if (cp <= buf) {
    return buf + len;
  }
  return cp;
}

/* Write a buffer as filename references (tag = <basedir>:<basename>) */
ssize_t __android_log_pmsg_file_write(log_id_t logId, char prio, const char* filename,
                                      const char* buf, size_t len) {
  size_t length, packet_len;
  const char* tag;
  char *cp, *slash;
  struct timespec ts;
  struct iovec vec[3];

  /* Make sure the logId value is not a bad idea */
  if ((logId == LOG_ID_KERNEL) ||   /* Verbotten */
      (logId == LOG_ID_EVENTS) ||   /* Do not support binary content */
      (logId == LOG_ID_SECURITY) || /* Bad idea to allow */
      ((unsigned)logId >= 32)) {    /* fit within logMask on arch32 */
    return -EINVAL;
  }

  clock_gettime(CLOCK_REALTIME, &ts);

  cp = strdup(filename);
  if (!cp) {
    return -ENOMEM;
  }

  tag = cp;
  slash = strrchr(cp, '/');
  if (slash) {
    *slash = ':';
    slash = strrchr(cp, '/');
    if (slash) {
      tag = slash + 1;
    }
  }

  length = strlen(tag) + 1;
  packet_len = LOGGER_ENTRY_MAX_PAYLOAD - sizeof(char) - length;

  vec[0].iov_base = &prio;
  vec[0].iov_len = sizeof(char);
  vec[1].iov_base = (unsigned char*)tag;
  vec[1].iov_len = length;

  for (ts.tv_nsec = 0, length = len; length; ts.tv_nsec += ANDROID_LOG_PMSG_FILE_SEQUENCE) {
    ssize_t ret;
    size_t transfer;

    if ((ts.tv_nsec / ANDROID_LOG_PMSG_FILE_SEQUENCE) >= ANDROID_LOG_PMSG_FILE_MAX_SEQUENCE) {
      len -= length;
      break;
    }

    transfer = length;
    if (transfer > packet_len) {
      transfer = strnrchr(buf, packet_len - 1, '\n') - buf;
      if ((transfer < length) && (buf[transfer] == '\n')) {
        ++transfer;
      }
    }

    vec[2].iov_base = (unsigned char*)buf;
    vec[2].iov_len = transfer;

    ret = PmsgWrite(logId, &ts, vec, sizeof(vec) / sizeof(vec[0]));

    if (ret <= 0) {
      free(cp);
      return ret ? ret : (len - length);
    }
    length -= transfer;
    buf += transfer;
  }
  free(cp);
  return len;
}
