/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "libc_logging.h" // Relative path so we can #include this .cpp file for testing.

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include "_system_properties.h"

static pthread_mutex_t g_abort_msg_lock = PTHREAD_MUTEX_INITIALIZER;

__LIBC_HIDDEN__ abort_msg_t** __abort_message_ptr; // Accessible to __libc_init_common.

// Must be kept in sync with frameworks/base/core/java/android/util/EventLog.java.
enum AndroidEventLogType {
  EVENT_TYPE_INT      = 0,
  EVENT_TYPE_LONG     = 1,
  EVENT_TYPE_STRING   = 2,
  EVENT_TYPE_LIST     = 3,
  EVENT_TYPE_FLOAT    = 4,
};

struct BufferOutputStream {
 public:
  BufferOutputStream(char* buffer, size_t size) : total(0) {
    buffer_ = buffer;
    end_ = buffer + size - 1;
    pos_ = buffer_;
    pos_[0] = '\0';
  }

  ~BufferOutputStream() {
  }

  void Send(const char* data, int len) {
    if (len < 0) {
      len = strlen(data);
    }

    total += len;

    while (len > 0) {
      int avail = end_ - pos_;
      if (avail == 0) {
        return;
      }
      if (avail > len) {
        avail = len;
      }
      memcpy(pos_, data, avail);
      pos_ += avail;
      pos_[0] = '\0';
      len -= avail;
    }
  }

  size_t total;

 private:
  char* buffer_;
  char* pos_;
  char* end_;
};

struct FdOutputStream {
 public:
  FdOutputStream(int fd) : total(0), fd_(fd) {
  }

  void Send(const char* data, int len) {
    if (len < 0) {
      len = strlen(data);
    }

    total += len;

    while (len > 0) {
      int rc = TEMP_FAILURE_RETRY(write(fd_, data, len));
      if (rc == -1) {
        return;
      }
      data += rc;
      len -= rc;
    }
  }

  size_t total;

 private:
  int fd_;
};

/*** formatted output implementation
 ***/

/* Parse a decimal string from 'format + *ppos',
 * return the value, and writes the new position past
 * the decimal string in '*ppos' on exit.
 *
 * NOTE: Does *not* handle a sign prefix.
 */
static unsigned parse_decimal(const char *format, int *ppos) {
    const char* p = format + *ppos;
    unsigned result = 0;

    for (;;) {
        int ch = *p;
        unsigned d = static_cast<unsigned>(ch - '0');

        if (d >= 10U) {
            break;
        }

        result = result*10 + d;
        p++;
    }
    *ppos = p - format;
    return result;
}

// Writes number 'value' in base 'base' into buffer 'buf' of size 'buf_size' bytes.
// Assumes that buf_size > 0.
static void format_unsigned(char* buf, size_t buf_size, uint64_t value, int base, bool caps) {
  char* p = buf;
  char* end = buf + buf_size - 1;

  // Generate digit string in reverse order.
  while (value) {
    unsigned d = value % base;
    value /= base;
    if (p != end) {
      char ch;
      if (d < 10) {
        ch = '0' + d;
      } else {
        ch = (caps ? 'A' : 'a') + (d - 10);
      }
      *p++ = ch;
    }
  }

  // Special case for 0.
  if (p == buf) {
    if (p != end) {
      *p++ = '0';
    }
  }
  *p = '\0';

  // Reverse digit string in-place.
  size_t length = p - buf;
  for (size_t i = 0, j = length - 1; i < j; ++i, --j) {
    char ch = buf[i];
    buf[i] = buf[j];
    buf[j] = ch;
  }
}

static void format_integer(char* buf, size_t buf_size, uint64_t value, char conversion) {
  // Decode the conversion specifier.
  int is_signed = (conversion == 'd' || conversion == 'i' || conversion == 'o');
  int base = 10;
  if (conversion == 'x' || conversion == 'X') {
    base = 16;
  } else if (conversion == 'o') {
    base = 8;
  }
  bool caps = (conversion == 'X');

  if (is_signed && static_cast<int64_t>(value) < 0) {
    buf[0] = '-';
    buf += 1;
    buf_size -= 1;
    value = static_cast<uint64_t>(-static_cast<int64_t>(value));
  }
  format_unsigned(buf, buf_size, value, base, caps);
}

template <typename Out>
static void SendRepeat(Out& o, char ch, int count) {
  char pad[8];
  memset(pad, ch, sizeof(pad));

  const int pad_size = static_cast<int>(sizeof(pad));
  while (count > 0) {
    int avail = count;
    if (avail > pad_size) {
      avail = pad_size;
    }
    o.Send(pad, avail);
    count -= avail;
  }
}

/* Perform formatted output to an output target 'o' */
template <typename Out>
static void out_vformat(Out& o, const char* format, va_list args) {
    int nn = 0;

    for (;;) {
        int mm;
        int padZero = 0;
        int padLeft = 0;
        char sign = '\0';
        int width = -1;
        int prec  = -1;
        size_t bytelen = sizeof(int);
        int slen;
        char buffer[32];  /* temporary buffer used to format numbers */

        char  c;

        /* first, find all characters that are not 0 or '%' */
        /* then send them to the output directly */
        mm = nn;
        do {
            c = format[mm];
            if (c == '\0' || c == '%')
                break;
            mm++;
        } while (1);

        if (mm > nn) {
            o.Send(format+nn, mm-nn);
            nn = mm;
        }

        /* is this it ? then exit */
        if (c == '\0')
            break;

        /* nope, we are at a '%' modifier */
        nn++;  // skip it

        /* parse flags */
        for (;;) {
            c = format[nn++];
            if (c == '\0') {  /* single trailing '%' ? */
                c = '%';
                o.Send(&c, 1);
                return;
            }
            else if (c == '0') {
                padZero = 1;
                continue;
            }
            else if (c == '-') {
                padLeft = 1;
                continue;
            }
            else if (c == ' ' || c == '+') {
                sign = c;
                continue;
            }
            break;
        }

        /* parse field width */
        if ((c >= '0' && c <= '9')) {
            nn --;
            width = static_cast<int>(parse_decimal(format, &nn));
            c = format[nn++];
        }

        /* parse precision */
        if (c == '.') {
            prec = static_cast<int>(parse_decimal(format, &nn));
            c = format[nn++];
        }

        /* length modifier */
        switch (c) {
        case 'h':
            bytelen = sizeof(short);
            if (format[nn] == 'h') {
                bytelen = sizeof(char);
                nn += 1;
            }
            c = format[nn++];
            break;
        case 'l':
            bytelen = sizeof(long);
            if (format[nn] == 'l') {
                bytelen = sizeof(long long);
                nn += 1;
            }
            c = format[nn++];
            break;
        case 'z':
            bytelen = sizeof(size_t);
            c = format[nn++];
            break;
        case 't':
            bytelen = sizeof(ptrdiff_t);
            c = format[nn++];
            break;
        default:
            ;
        }

        /* conversion specifier */
        const char* str = buffer;
        if (c == 's') {
            /* string */
            str = va_arg(args, const char*);
            if (str == NULL) {
                str = "(null)";
            }
        } else if (c == 'c') {
            /* character */
            /* NOTE: char is promoted to int when passed through the stack */
            buffer[0] = static_cast<char>(va_arg(args, int));
            buffer[1] = '\0';
        } else if (c == 'p') {
            uint64_t  value = reinterpret_cast<uintptr_t>(va_arg(args, void*));
            buffer[0] = '0';
            buffer[1] = 'x';
            format_integer(buffer + 2, sizeof(buffer) - 2, value, 'x');
        } else if (c == 'd' || c == 'i' || c == 'o' || c == 'u' || c == 'x' || c == 'X') {
            /* integers - first read value from stack */
            uint64_t value;
            int is_signed = (c == 'd' || c == 'i' || c == 'o');

            /* NOTE: int8_t and int16_t are promoted to int when passed
             *       through the stack
             */
            switch (bytelen) {
            case 1: value = static_cast<uint8_t>(va_arg(args, int)); break;
            case 2: value = static_cast<uint16_t>(va_arg(args, int)); break;
            case 4: value = va_arg(args, uint32_t); break;
            case 8: value = va_arg(args, uint64_t); break;
            default: return;  /* should not happen */
            }

            /* sign extension, if needed */
            if (is_signed) {
                int shift = 64 - 8*bytelen;
                value = static_cast<uint64_t>((static_cast<int64_t>(value << shift)) >> shift);
            }

            /* format the number properly into our buffer */
            format_integer(buffer, sizeof(buffer), value, c);
        } else if (c == '%') {
            buffer[0] = '%';
            buffer[1] = '\0';
        } else {
            __assert(__FILE__, __LINE__, "conversion specifier unsupported");
        }

        /* if we are here, 'str' points to the content that must be
         * outputted. handle padding and alignment now */

        slen = strlen(str);

        if (sign != '\0' || prec != -1) {
            __assert(__FILE__, __LINE__, "sign/precision unsupported");
        }

        if (slen < width && !padLeft) {
            char padChar = padZero ? '0' : ' ';
            SendRepeat(o, padChar, width - slen);
        }

        o.Send(str, slen);

        if (slen < width && padLeft) {
            char padChar = padZero ? '0' : ' ';
            SendRepeat(o, padChar, width - slen);
        }
    }
}

int __libc_format_buffer(char* buffer, size_t buffer_size, const char* format, ...) {
  BufferOutputStream os(buffer, buffer_size);
  va_list args;
  va_start(args, format);
  out_vformat(os, format, args);
  va_end(args);
  return os.total;
}

int __libc_format_fd(int fd, const char* format, ...) {
  FdOutputStream os(fd);
  va_list args;
  va_start(args, format);
  out_vformat(os, format, args);
  va_end(args);
  return os.total;
}

static int __libc_write_stderr(const char* tag, const char* msg) {
  int fd = TEMP_FAILURE_RETRY(open("/dev/stderr", O_CLOEXEC | O_WRONLY | O_APPEND));
  if (fd == -1) {
    return -1;
  }

  iovec vec[4];
  vec[0].iov_base = const_cast<char*>(tag);
  vec[0].iov_len = strlen(tag);
  vec[1].iov_base = const_cast<char*>(": ");
  vec[1].iov_len = 2;
  vec[2].iov_base = const_cast<char*>(msg);
  vec[2].iov_len = strlen(msg);
  vec[3].iov_base = const_cast<char*>("\n");
  vec[3].iov_len = 1;

  int result = TEMP_FAILURE_RETRY(writev(fd, vec, 4));
  close(fd);
  return result;
}

static int __libc_open_log_socket() {
  // ToDo: Ideally we want this to fail if the gid of the current
  // process is AID_LOGD, but will have to wait until we have
  // registered this in private/android_filesystem_config.h. We have
  // found that all logd crashes thus far have had no problem stuffing
  // the UNIX domain socket and moving on so not critical *today*.

  int log_fd = TEMP_FAILURE_RETRY(socket(PF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0));
  if (log_fd < 0) {
    return -1;
  }

  if (fcntl(log_fd, F_SETFL, O_NONBLOCK) == -1) {
    close(log_fd);
    return -1;
  }

  union {
    struct sockaddr    addr;
    struct sockaddr_un addrUn;
  } u;
  memset(&u, 0, sizeof(u));
  u.addrUn.sun_family = AF_UNIX;
  strlcpy(u.addrUn.sun_path, "/dev/socket/logdw", sizeof(u.addrUn.sun_path));

  if (TEMP_FAILURE_RETRY(connect(log_fd, &u.addr, sizeof(u.addrUn))) != 0) {
    close(log_fd);
    return -1;
  }

  return log_fd;
}

struct cache {
  const prop_info* pinfo;
  uint32_t serial;
  char c;
};

static void refresh_cache(struct cache *cache, const char *key)
{
  if (!cache->pinfo) {
    cache->pinfo = __system_property_find(key);
    if (!cache->pinfo) {
      return;
    }
  }
  uint32_t serial = __system_property_serial(cache->pinfo);
  if (serial == cache->serial) {
    return;
  }
  cache->serial = serial;

  char buf[PROP_VALUE_MAX];
  __system_property_read(cache->pinfo, 0, buf);
  cache->c = buf[0];
}

// Timestamp state generally remains constant, since a change is
// rare, we can accept a trylock failure gracefully.
static pthread_mutex_t lock_clockid = PTHREAD_MUTEX_INITIALIZER;

static clockid_t __android_log_clockid()
{
  static struct cache r_time_cache = { NULL, static_cast<uint32_t>(-1), 0 };
  static struct cache p_time_cache = { NULL, static_cast<uint32_t>(-1), 0 };
  char c;

  if (pthread_mutex_trylock(&lock_clockid)) {
    // We are willing to accept some race in this context
    if (!(c = p_time_cache.c)) {
      c = r_time_cache.c;
    }
  } else {
    static uint32_t serial;
    uint32_t current_serial = __system_property_area_serial();
    if (current_serial != serial) {
      refresh_cache(&r_time_cache, "ro.logd.timestamp");
      refresh_cache(&p_time_cache, "persist.logd.timestamp");
      serial = current_serial;
    }
    if (!(c = p_time_cache.c)) {
      c = r_time_cache.c;
    }

    pthread_mutex_unlock(&lock_clockid);
  }

  return (tolower(c) == 'm') ? CLOCK_MONOTONIC : CLOCK_REALTIME;
}

struct log_time { // Wire format
  uint32_t tv_sec;
  uint32_t tv_nsec;
};

int __libc_write_log(int priority, const char* tag, const char* msg) {
  int main_log_fd = __libc_open_log_socket();
  if (main_log_fd == -1) {
    // Try stderr instead.
    return __libc_write_stderr(tag, msg);
  }

  iovec vec[6];
  char log_id = (priority == ANDROID_LOG_FATAL) ? LOG_ID_CRASH : LOG_ID_MAIN;
  vec[0].iov_base = &log_id;
  vec[0].iov_len = sizeof(log_id);
  uint16_t tid = gettid();
  vec[1].iov_base = &tid;
  vec[1].iov_len = sizeof(tid);
  timespec ts;
  clock_gettime(__android_log_clockid(), &ts);
  log_time realtime_ts;
  realtime_ts.tv_sec = ts.tv_sec;
  realtime_ts.tv_nsec = ts.tv_nsec;
  vec[2].iov_base = &realtime_ts;
  vec[2].iov_len = sizeof(realtime_ts);

  vec[3].iov_base = &priority;
  vec[3].iov_len = 1;
  vec[4].iov_base = const_cast<char*>(tag);
  vec[4].iov_len = strlen(tag) + 1;
  vec[5].iov_base = const_cast<char*>(msg);
  vec[5].iov_len = strlen(msg) + 1;

  int result = TEMP_FAILURE_RETRY(writev(main_log_fd, vec, sizeof(vec) / sizeof(vec[0])));
  close(main_log_fd);
  return result;
}

int __libc_format_log_va_list(int priority, const char* tag, const char* format, va_list args) {
  char buffer[1024];
  BufferOutputStream os(buffer, sizeof(buffer));
  out_vformat(os, format, args);
  return __libc_write_log(priority, tag, buffer);
}

int __libc_format_log(int priority, const char* tag, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int result = __libc_format_log_va_list(priority, tag, format, args);
  va_end(args);
  return result;
}

static int __libc_android_log_event(int32_t tag, char type, const void* payload, size_t len) {
  iovec vec[6];
  char log_id = LOG_ID_EVENTS;
  vec[0].iov_base = &log_id;
  vec[0].iov_len = sizeof(log_id);
  uint16_t tid = gettid();
  vec[1].iov_base = &tid;
  vec[1].iov_len = sizeof(tid);
  timespec ts;
  clock_gettime(__android_log_clockid(), &ts);
  log_time realtime_ts;
  realtime_ts.tv_sec = ts.tv_sec;
  realtime_ts.tv_nsec = ts.tv_nsec;
  vec[2].iov_base = &realtime_ts;
  vec[2].iov_len = sizeof(realtime_ts);

  vec[3].iov_base = &tag;
  vec[3].iov_len = sizeof(tag);
  vec[4].iov_base = &type;
  vec[4].iov_len = sizeof(type);
  vec[5].iov_base = const_cast<void*>(payload);
  vec[5].iov_len = len;

  int event_log_fd = __libc_open_log_socket();

  if (event_log_fd == -1) {
    return -1;
  }
  int result = TEMP_FAILURE_RETRY(writev(event_log_fd, vec, sizeof(vec) / sizeof(vec[0])));
  close(event_log_fd);
  return result;
}

void __libc_android_log_event_int(int32_t tag, int value) {
  __libc_android_log_event(tag, EVENT_TYPE_INT, &value, sizeof(value));
}

void __libc_android_log_event_uid(int32_t tag) {
  __libc_android_log_event_int(tag, getuid());
}

void __fortify_chk_fail(const char* msg, uint32_t tag) {
  if (tag != 0) {
    __libc_android_log_event_uid(tag);
  }
  __libc_fatal("FORTIFY: %s", msg);
}

void __libc_fatal_no_abort(const char* format, ...) {
  va_list args;
  va_start(args, format);
  __libc_fatal(format, args);
  va_end(args);
}

void __libc_fatal(const char* format, ...) {
  va_list args;
  va_start(args, format);
  __libc_fatal(format, args);
  va_end(args);
  abort();
}