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

#include "pmsg_reader.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <cutils/list.h>
#include <private/android_logger.h>

#include "logger.h"

int PmsgRead(struct logger_list* logger_list, struct log_msg* log_msg) {
  ssize_t ret;
  off_t current, next;
  struct __attribute__((__packed__)) {
    android_pmsg_log_header_t p;
    android_log_header_t l;
    uint8_t prio;
  } buf;
  static uint8_t preread_count;

  memset(log_msg, 0, sizeof(*log_msg));

  if (atomic_load(&logger_list->fd) <= 0) {
    int i, fd = open("/sys/fs/pstore/pmsg-ramoops-0", O_RDONLY | O_CLOEXEC);

    if (fd < 0) {
      return -errno;
    }
    if (fd == 0) { /* Argggg */
      fd = open("/sys/fs/pstore/pmsg-ramoops-0", O_RDONLY | O_CLOEXEC);
      close(0);
      if (fd < 0) {
        return -errno;
      }
    }
    i = atomic_exchange(&logger_list->fd, fd);
    if ((i > 0) && (i != fd)) {
      close(i);
    }
    preread_count = 0;
  }

  while (1) {
    int fd;

    if (preread_count < sizeof(buf)) {
      fd = atomic_load(&logger_list->fd);
      if (fd <= 0) {
        return -EBADF;
      }
      ret = TEMP_FAILURE_RETRY(read(fd, &buf.p.magic + preread_count, sizeof(buf) - preread_count));
      if (ret < 0) {
        return -errno;
      }
      preread_count += ret;
    }
    if (preread_count != sizeof(buf)) {
      return preread_count ? -EIO : -EAGAIN;
    }
    if ((buf.p.magic != LOGGER_MAGIC) || (buf.p.len <= sizeof(buf)) ||
        (buf.p.len > (sizeof(buf) + LOGGER_ENTRY_MAX_PAYLOAD)) || (buf.l.id >= LOG_ID_MAX) ||
        (buf.l.realtime.tv_nsec >= NS_PER_SEC) ||
        ((buf.l.id != LOG_ID_EVENTS) && (buf.l.id != LOG_ID_SECURITY) &&
         ((buf.prio == ANDROID_LOG_UNKNOWN) || (buf.prio == ANDROID_LOG_DEFAULT) ||
          (buf.prio >= ANDROID_LOG_SILENT)))) {
      do {
        memmove(&buf.p.magic, &buf.p.magic + 1, --preread_count);
      } while (preread_count && (buf.p.magic != LOGGER_MAGIC));
      continue;
    }
    preread_count = 0;

    if ((logger_list->log_mask & (1 << buf.l.id)) &&
        ((!logger_list->start.tv_sec && !logger_list->start.tv_nsec) ||
         ((logger_list->start.tv_sec <= buf.l.realtime.tv_sec) &&
          ((logger_list->start.tv_sec != buf.l.realtime.tv_sec) ||
           (logger_list->start.tv_nsec <= buf.l.realtime.tv_nsec)))) &&
        (!logger_list->pid || (logger_list->pid == buf.p.pid))) {
      char* msg = reinterpret_cast<char*>(&log_msg->entry) + sizeof(log_msg->entry);
      *msg = buf.prio;
      fd = atomic_load(&logger_list->fd);
      if (fd <= 0) {
        return -EBADF;
      }
      ret = TEMP_FAILURE_RETRY(read(fd, msg + sizeof(buf.prio), buf.p.len - sizeof(buf)));
      if (ret < 0) {
        return -errno;
      }
      if (ret != (ssize_t)(buf.p.len - sizeof(buf))) {
        return -EIO;
      }

      log_msg->entry.len = buf.p.len - sizeof(buf) + sizeof(buf.prio);
      log_msg->entry.hdr_size = sizeof(log_msg->entry);
      log_msg->entry.pid = buf.p.pid;
      log_msg->entry.tid = buf.l.tid;
      log_msg->entry.sec = buf.l.realtime.tv_sec;
      log_msg->entry.nsec = buf.l.realtime.tv_nsec;
      log_msg->entry.lid = buf.l.id;
      log_msg->entry.uid = buf.p.uid;

      return ret + sizeof(buf.prio) + log_msg->entry.hdr_size;
    }

    fd = atomic_load(&logger_list->fd);
    if (fd <= 0) {
      return -EBADF;
    }
    current = TEMP_FAILURE_RETRY(lseek(fd, (off_t)0, SEEK_CUR));
    if (current < 0) {
      return -errno;
    }
    fd = atomic_load(&logger_list->fd);
    if (fd <= 0) {
      return -EBADF;
    }
    next = TEMP_FAILURE_RETRY(lseek(fd, (off_t)(buf.p.len - sizeof(buf)), SEEK_CUR));
    if (next < 0) {
      return -errno;
    }
    if ((next - current) != (ssize_t)(buf.p.len - sizeof(buf))) {
      return -EIO;
    }
  }
}

void PmsgClose(struct logger_list* logger_list) {
  int fd = atomic_exchange(&logger_list->fd, 0);
  if (fd > 0) {
    close(fd);
  }
}

static void* realloc_or_free(void* ptr, size_t new_size) {
  void* result = realloc(ptr, new_size);
  if (!result) {
    free(ptr);
  }
  return result;
}

ssize_t __android_log_pmsg_file_read(log_id_t logId, char prio, const char* prefix,
                                     __android_log_pmsg_file_read_fn fn, void* arg) {
  ssize_t ret;
  struct logger_list logger_list;
  struct content {
    struct listnode node;
    struct logger_entry entry;
  } * content;
  struct names {
    struct listnode node;
    struct listnode content;
    log_id_t id;
    char prio;
    char name[];
  } * names;
  struct listnode name_list;
  struct listnode *node, *n;
  size_t len, prefix_len;

  if (!fn) {
    return -EINVAL;
  }

  /* Add just enough clues in logger_list and transp to make API function */
  memset(&logger_list, 0, sizeof(logger_list));

  logger_list.mode = ANDROID_LOG_PSTORE | ANDROID_LOG_NONBLOCK;
  logger_list.log_mask = (unsigned)-1;
  if (logId != LOG_ID_ANY) {
    logger_list.log_mask = (1 << logId);
  }
  logger_list.log_mask &= ~((1 << LOG_ID_KERNEL) | (1 << LOG_ID_EVENTS) | (1 << LOG_ID_SECURITY));
  if (!logger_list.log_mask) {
    return -EINVAL;
  }

  /* Initialize name list */
  list_init(&name_list);

  ret = SSIZE_MAX;

  /* Validate incoming prefix, shift until it contains only 0 or 1 : or / */
  prefix_len = 0;
  if (prefix) {
    const char *prev = NULL, *last = NULL, *cp = prefix;
    while ((cp = strpbrk(cp, "/:"))) {
      prev = last;
      last = cp;
      cp = cp + 1;
    }
    if (prev) {
      prefix = prev + 1;
    }
    prefix_len = strlen(prefix);
  }

  /* Read the file content */
  log_msg log_msg;
  while (PmsgRead(&logger_list, &log_msg) > 0) {
    const char* cp;
    size_t hdr_size = log_msg.entry.hdr_size;

    char* msg = (char*)&log_msg + hdr_size;
    const char* split = NULL;

    if (hdr_size != sizeof(log_msg.entry)) {
      continue;
    }
    /* Check for invalid sequence number */
    if (log_msg.entry.nsec % ANDROID_LOG_PMSG_FILE_SEQUENCE ||
        (log_msg.entry.nsec / ANDROID_LOG_PMSG_FILE_SEQUENCE) >=
            ANDROID_LOG_PMSG_FILE_MAX_SEQUENCE) {
      continue;
    }

    /* Determine if it has <dirbase>:<filebase> format for tag */
    len = log_msg.entry.len - sizeof(prio);
    for (cp = msg + sizeof(prio); *cp && isprint(*cp) && !isspace(*cp) && --len; ++cp) {
      if (*cp == ':') {
        if (split) {
          break;
        }
        split = cp;
      }
    }
    if (*cp || !split) {
      continue;
    }

    /* Filters */
    if (prefix_len && strncmp(msg + sizeof(prio), prefix, prefix_len)) {
      size_t offset;
      /*
       *   Allow : to be a synonym for /
       * Things we do dealing with const char * and do not alloc
       */
      split = strchr(prefix, ':');
      if (split) {
        continue;
      }
      split = strchr(prefix, '/');
      if (!split) {
        continue;
      }
      offset = split - prefix;
      if ((msg[offset + sizeof(prio)] != ':') || strncmp(msg + sizeof(prio), prefix, offset)) {
        continue;
      }
      ++offset;
      if ((prefix_len > offset) &&
          strncmp(&msg[offset + sizeof(prio)], split + 1, prefix_len - offset)) {
        continue;
      }
    }

    if ((prio != ANDROID_LOG_ANY) && (*msg < prio)) {
      continue;
    }

    /* check if there is an existing entry */
    list_for_each(node, &name_list) {
      names = node_to_item(node, struct names, node);
      if (!strcmp(names->name, msg + sizeof(prio)) && names->id == log_msg.entry.lid &&
          names->prio == *msg) {
        break;
      }
    }

    /* We do not have an existing entry, create and add one */
    if (node == &name_list) {
      static const char numbers[] = "0123456789";
      unsigned long long nl;

      len = strlen(msg + sizeof(prio)) + 1;
      names = static_cast<struct names*>(calloc(1, sizeof(*names) + len));
      if (!names) {
        ret = -ENOMEM;
        break;
      }
      strcpy(names->name, msg + sizeof(prio));
      names->id = static_cast<log_id_t>(log_msg.entry.lid);
      names->prio = *msg;
      list_init(&names->content);
      /*
       * Insert in reverse numeric _then_ alpha sorted order as
       * representative of log rotation:
       *
       *   log.10
       *   klog.10
       *   . . .
       *   log.2
       *   klog.2
       *   log.1
       *   klog.1
       *   log
       *   klog
       *
       * thus when we present the content, we are provided the oldest
       * first, which when 'refreshed' could spill off the end of the
       * pmsg FIFO but retaining the newest data for last with best
       * chances to survive.
       */
      nl = 0;
      cp = strpbrk(names->name, numbers);
      if (cp) {
        nl = strtoull(cp, NULL, 10);
      }
      list_for_each_reverse(node, &name_list) {
        struct names* a_name = node_to_item(node, struct names, node);
        const char* r = a_name->name;
        int compare = 0;

        unsigned long long nr = 0;
        cp = strpbrk(r, numbers);
        if (cp) {
          nr = strtoull(cp, NULL, 10);
        }
        if (nr != nl) {
          compare = (nl > nr) ? 1 : -1;
        }
        if (compare == 0) {
          compare = strcmp(names->name, r);
        }
        if (compare <= 0) {
          break;
        }
      }
      list_add_head(node, &names->node);
    }

    /* Remove any file fragments that match our sequence number */
    list_for_each_safe(node, n, &names->content) {
      content = node_to_item(node, struct content, node);
      if (log_msg.entry.nsec == content->entry.nsec) {
        list_remove(&content->node);
        free(content);
      }
    }

    /* Add content */
    content = static_cast<struct content*>(
        calloc(1, sizeof(content->node) + hdr_size + log_msg.entry.len));
    if (!content) {
      ret = -ENOMEM;
      break;
    }
    memcpy(&content->entry, &log_msg.entry, hdr_size + log_msg.entry.len);

    /* Insert in sequence number sorted order, to ease reconstruction */
    list_for_each_reverse(node, &names->content) {
      if ((node_to_item(node, struct content, node))->entry.nsec < log_msg.entry.nsec) {
        break;
      }
    }
    list_add_head(node, &content->node);
  }
  PmsgClose(&logger_list);

  /* Progress through all the collected files */
  list_for_each_safe(node, n, &name_list) {
    struct listnode *content_node, *m;
    char* buf;
    size_t sequence, tag_len;

    names = node_to_item(node, struct names, node);

    /* Construct content into a linear buffer */
    buf = NULL;
    len = 0;
    sequence = 0;
    tag_len = strlen(names->name) + sizeof(char); /* tag + nul */
    list_for_each_safe(content_node, m, &names->content) {
      ssize_t add_len;

      content = node_to_item(content_node, struct content, node);
      add_len = content->entry.len - tag_len - sizeof(prio);
      if (add_len <= 0) {
        list_remove(content_node);
        free(content);
        continue;
      }

      if (!buf) {
        buf = static_cast<char*>(malloc(sizeof(char)));
        if (!buf) {
          ret = -ENOMEM;
          list_remove(content_node);
          free(content);
          continue;
        }
        *buf = '\0';
      }

      /* Missing sequence numbers */
      while (sequence < content->entry.nsec) {
        /* plus space for enforced nul */
        buf = static_cast<char*>(realloc_or_free(buf, len + sizeof(char) + sizeof(char)));
        if (!buf) {
          break;
        }
        buf[len] = '\f'; /* Mark missing content with a form feed */
        buf[++len] = '\0';
        sequence += ANDROID_LOG_PMSG_FILE_SEQUENCE;
      }
      if (!buf) {
        ret = -ENOMEM;
        list_remove(content_node);
        free(content);
        continue;
      }
      /* plus space for enforced nul */
      buf = static_cast<char*>(realloc_or_free(buf, len + add_len + sizeof(char)));
      if (!buf) {
        ret = -ENOMEM;
        list_remove(content_node);
        free(content);
        continue;
      }
      memcpy(buf + len, (char*)&content->entry + content->entry.hdr_size + tag_len + sizeof(prio),
             add_len);
      len += add_len;
      buf[len] = '\0'; /* enforce trailing hidden nul */
      sequence = content->entry.nsec + ANDROID_LOG_PMSG_FILE_SEQUENCE;

      list_remove(content_node);
      free(content);
    }
    if (buf) {
      if (len) {
        /* Buffer contains enforced trailing nul just beyond length */
        ssize_t r;
        *strchr(names->name, ':') = '/'; /* Convert back to filename */
        r = (*fn)(names->id, names->prio, names->name, buf, len, arg);
        if ((ret >= 0) && (r > 0)) {
          if (ret == SSIZE_MAX) {
            ret = r;
          } else {
            ret += r;
          }
        } else if (r < ret) {
          ret = r;
        }
      }
      free(buf);
    }
    list_remove(node);
    free(names);
  }
  return (ret == SSIZE_MAX) ? -ENOENT : ret;
}
