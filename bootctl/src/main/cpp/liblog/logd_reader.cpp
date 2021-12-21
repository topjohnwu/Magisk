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

#include "logd_reader.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include <android-base/parseint.h>
#include <private/android_logger.h>

#include "logger.h"

// Connects to /dev/socket/<name> and returns the associated fd or returns -1 on error.
// O_CLOEXEC is always set.
static int socket_local_client(const std::string& name, int type, bool timeout) {
  sockaddr_un addr = {.sun_family = AF_LOCAL};

  std::string path = "/dev/socket/" + name;
  if (path.size() + 1 > sizeof(addr.sun_path)) {
    return -1;
  }
  strlcpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path));

  int fd = socket(AF_LOCAL, type | SOCK_CLOEXEC, 0);
  if (fd == -1) {
    return -1;
  }

  if (timeout) {
    // Sending and receiving messages should be instantaneous, but we don't want to wait forever if
    // logd is hung, so we set a gracious 2s timeout.
    struct timeval t = {2, 0};
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) == -1) {
      return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1) {
      return -1;
    }
  }

  if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

/* worker for sending the command to the logger */
ssize_t SendLogdControlMessage(char* buf, size_t buf_size) {
  ssize_t ret;
  size_t len;
  char* cp;
  int errno_save = 0;
  int sock = socket_local_client("logd", SOCK_STREAM, true);
  if (sock < 0) {
    return sock;
  }

  len = strlen(buf) + 1;
  ret = TEMP_FAILURE_RETRY(write(sock, buf, len));
  if (ret <= 0) {
    goto done;
  }

  len = buf_size;
  cp = buf;
  while ((ret = TEMP_FAILURE_RETRY(read(sock, cp, len))) > 0) {
    struct pollfd p;

    if (((size_t)ret == len) || (buf_size < PAGE_SIZE)) {
      break;
    }

    len -= ret;
    cp += ret;

    memset(&p, 0, sizeof(p));
    p.fd = sock;
    p.events = POLLIN;

    /* Give other side 20ms to refill pipe */
    ret = TEMP_FAILURE_RETRY(poll(&p, 1, 20));

    if (ret <= 0) {
      break;
    }

    if (!(p.revents & POLLIN)) {
      ret = 0;
      break;
    }
  }

  if (ret >= 0) {
    ret += buf_size - len;
  }

done:
  if ((ret == -1) && errno) {
    errno_save = errno;
  }
  close(sock);
  if (errno_save) {
    errno = errno_save;
  }
  return ret;
}

static int check_log_success(char* buf, ssize_t ret) {
  if (ret < 0) {
    return ret;
  }

  if (strncmp(buf, "success", 7)) {
    errno = EINVAL;
    return -1;
  }

  return 0;
}

int android_logger_clear(struct logger* logger) {
  if (!android_logger_is_logd(logger)) {
    return -EINVAL;
  }
  uint32_t log_id = android_logger_get_id(logger);
  char buf[512];
  snprintf(buf, sizeof(buf), "clear %" PRIu32, log_id);

  return check_log_success(buf, SendLogdControlMessage(buf, sizeof(buf)));
}

enum class LogSizeType : uint32_t {
  kAllotted = 0,
  kReadable,
  kConsumed,
};

static long GetLogSize(struct logger* logger, LogSizeType type) {
  if (!android_logger_is_logd(logger)) {
    return -EINVAL;
  }

  uint32_t log_id = android_logger_get_id(logger);
  char buf[512];
  switch (type) {
    case LogSizeType::kAllotted:
      snprintf(buf, sizeof(buf), "getLogSize %" PRIu32, log_id);
      break;
    case LogSizeType::kReadable:
      snprintf(buf, sizeof(buf), "getLogSizeReadable %" PRIu32, log_id);
      break;
    case LogSizeType::kConsumed:
      snprintf(buf, sizeof(buf), "getLogSizeUsed %" PRIu32, log_id);
      break;
    default:
      abort();
  }

  ssize_t ret = SendLogdControlMessage(buf, sizeof(buf));
  if (ret < 0) {
    return ret;
  }

  long size;
  if (!android::base::ParseInt(buf, &size)) {
    return -1;
  }

  return size;
}

long android_logger_get_log_size(struct logger* logger) {
  return GetLogSize(logger, LogSizeType::kAllotted);
}

long android_logger_get_log_readable_size(struct logger* logger) {
  return GetLogSize(logger, LogSizeType::kReadable);
}

long android_logger_get_log_consumed_size(struct logger* logger) {
  return GetLogSize(logger, LogSizeType::kConsumed);
}

int android_logger_set_log_size(struct logger* logger, unsigned long size) {
  if (!android_logger_is_logd(logger)) {
    return -EINVAL;
  }

  uint32_t log_id = android_logger_get_id(logger);
  char buf[512];
  snprintf(buf, sizeof(buf), "setLogSize %" PRIu32 " %lu", log_id, size);

  return check_log_success(buf, SendLogdControlMessage(buf, sizeof(buf)));
}

int android_logger_get_log_version(struct logger*) {
  return 4;
}

ssize_t android_logger_get_statistics(struct logger_list* logger_list, char* buf, size_t len) {
  if (logger_list->mode & ANDROID_LOG_PSTORE) {
    return -EINVAL;
  }

  char* cp = buf;
  size_t remaining = len;
  size_t n;

  n = snprintf(cp, remaining, "getStatistics");
  n = MIN(n, remaining);
  remaining -= n;
  cp += n;

  for (size_t log_id = 0; log_id < LOG_ID_MAX; ++log_id) {
    if ((1 << log_id) & logger_list->log_mask) {
      n = snprintf(cp, remaining, " %zu", log_id);
      n = MIN(n, remaining);
      remaining -= n;
      cp += n;
    }
  }

  if (logger_list->pid) {
    snprintf(cp, remaining, " pid=%u", logger_list->pid);
  }

  return SendLogdControlMessage(buf, len);
}
ssize_t android_logger_get_prune_list(struct logger_list* logger_list, char* buf, size_t len) {
  if (logger_list->mode & ANDROID_LOG_PSTORE) {
    return -EINVAL;
  }

  snprintf(buf, len, "getPruneList");
  return SendLogdControlMessage(buf, len);
}

int android_logger_set_prune_list(struct logger_list* logger_list, const char* buf, size_t len) {
  if (logger_list->mode & ANDROID_LOG_PSTORE) {
    return -EINVAL;
  }

  std::string cmd = "setPruneList " + std::string{buf, len};

  return check_log_success(cmd.data(), SendLogdControlMessage(cmd.data(), cmd.size()));
}

static int logdOpen(struct logger_list* logger_list) {
  char buffer[256], *cp, c;
  int ret, remaining, sock;

  sock = atomic_load(&logger_list->fd);
  if (sock > 0) {
    return sock;
  }

  sock = socket_local_client("logdr", SOCK_SEQPACKET, false);
  if (sock <= 0) {
    if ((sock == -1) && errno) {
      return -errno;
    }
    return sock;
  }

  strcpy(buffer, (logger_list->mode & ANDROID_LOG_NONBLOCK) ? "dumpAndClose" : "stream");
  cp = buffer + strlen(buffer);

  strcpy(cp, " lids");
  cp += 5;
  c = '=';
  remaining = sizeof(buffer) - (cp - buffer);

  for (size_t log_id = 0; log_id < LOG_ID_MAX; ++log_id) {
    if ((1 << log_id) & logger_list->log_mask) {
      ret = snprintf(cp, remaining, "%c%zu", c, log_id);
      ret = MIN(ret, remaining);
      remaining -= ret;
      cp += ret;
      c = ',';
    }
  }

  if (logger_list->tail) {
    ret = snprintf(cp, remaining, " tail=%u", logger_list->tail);
    ret = MIN(ret, remaining);
    remaining -= ret;
    cp += ret;
  }

  if (logger_list->start.tv_sec || logger_list->start.tv_nsec) {
    if (logger_list->mode & ANDROID_LOG_WRAP) {
      // ToDo: alternate API to allow timeout to be adjusted.
      ret = snprintf(cp, remaining, " timeout=%u", ANDROID_LOG_WRAP_DEFAULT_TIMEOUT);
      ret = MIN(ret, remaining);
      remaining -= ret;
      cp += ret;
    }
    ret = snprintf(cp, remaining, " start=%" PRIu32 ".%09" PRIu32, logger_list->start.tv_sec,
                   logger_list->start.tv_nsec);
    ret = MIN(ret, remaining);
    remaining -= ret;
    cp += ret;
  }

  if (logger_list->pid) {
    ret = snprintf(cp, remaining, " pid=%u", logger_list->pid);
    ret = MIN(ret, remaining);
    cp += ret;
  }

  ret = TEMP_FAILURE_RETRY(write(sock, buffer, cp - buffer));
  int write_errno = errno;

  if (ret <= 0) {
    close(sock);
    if (ret == -1) {
      return -write_errno;
    }
    if (ret == 0) {
      return -EIO;
    }
    return ret;
  }

  ret = atomic_exchange(&logger_list->fd, sock);
  if ((ret > 0) && (ret != sock)) {
    close(ret);
  }
  return sock;
}

/* Read from the selected logs */
int LogdRead(struct logger_list* logger_list, struct log_msg* log_msg) {
  int ret = logdOpen(logger_list);
  if (ret < 0) {
    return ret;
  }

  /* NOTE: SOCK_SEQPACKET guarantees we read exactly one full entry */
  ret = TEMP_FAILURE_RETRY(recv(ret, log_msg, LOGGER_ENTRY_MAX_LEN, 0));
  if ((logger_list->mode & ANDROID_LOG_NONBLOCK) && ret == 0) {
    return -EAGAIN;
  }

  if (ret == -1) {
    return -errno;
  }
  return ret;
}

/* Close all the logs */
void LogdClose(struct logger_list* logger_list) {
  int sock = atomic_exchange(&logger_list->fd, -1);
  if (sock > 0) {
    close(sock);
  }
}
