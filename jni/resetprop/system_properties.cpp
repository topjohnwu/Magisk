/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <new>

#include <linux/xattr.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/xattr.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include "_system_properties.h"
#include "system_properties.h"

// #include <async_safe/log.h>

#include "ErrnoRestorer.h"
#include "bionic_futex.h"
#include "bionic_lock.h"
#include "bionic_macros.h"

static constexpr int PROP_FILENAME_MAX = 1024;

static constexpr uint32_t PROP_AREA_MAGIC = 0x504f5250;
static constexpr uint32_t PROP_AREA_VERSION = 0xfc6ed0ab;

static constexpr size_t PA_SIZE = 128 * 1024;

#define SERIAL_DIRTY(serial) ((serial)&1)
#define SERIAL_VALUE_LEN(serial) ((serial) >> 24)

static const char property_service_socket[] = "/dev/socket/" PROP_SERVICE_NAME;
static const char* kServiceVersionPropertyName = "ro.property_service.version";

/*
 * Properties are stored in a hybrid trie/binary tree structure.
 * Each property's name is delimited at '.' characters, and the tokens are put
 * into a trie structure.  Siblings at each level of the trie are stored in a
 * binary tree.  For instance, "ro.secure"="1" could be stored as follows:
 *
 * +-----+   children    +----+   children    +--------+
 * |     |-------------->| ro |-------------->| secure |
 * +-----+               +----+               +--------+
 *                       /    \                /   |
 *                 left /      \ right   left /    |  prop   +===========+
 *                     v        v            v     +-------->| ro.secure |
 *                  +-----+   +-----+     +-----+            +-----------+
 *                  | net |   | sys |     | com |            |     1     |
 *                  +-----+   +-----+     +-----+            +===========+
 */

// This is a alternative implementation for async_safe_format_buffer
// A workaround to not include the async_safe header
static int async_safe_format_buffer(char * s, size_t n, const char * format, ...) {
  va_list vl;
  va_start(vl, format);
  int ret = vsnprintf(s, n, format, vl);
  va_end(vl);
  return ret;
}

// Represents a node in the trie.
struct prop_bt {
  uint32_t namelen;

  // The property trie is updated only by the init process (single threaded) which provides
  // property service. And it can be read by multiple threads at the same time.
  // As the property trie is not protected by locks, we use atomic_uint_least32_t types for the
  // left, right, children "pointers" in the trie node. To make sure readers who see the
  // change of "pointers" can also notice the change of prop_bt structure contents pointed by
  // the "pointers", we always use release-consume ordering pair when accessing these "pointers".

  // prop "points" to prop_info structure if there is a propery associated with the trie node.
  // Its situation is similar to the left, right, children "pointers". So we use
  // atomic_uint_least32_t and release-consume ordering to protect it as well.

  // We should also avoid rereading these fields redundantly, since not
  // all processor implementations ensure that multiple loads from the
  // same field are carried out in the right order.
  atomic_uint_least32_t prop;

  atomic_uint_least32_t left;
  atomic_uint_least32_t right;

  atomic_uint_least32_t children;

  char name[0];

  prop_bt(const char* name, const uint32_t name_length) {
    this->namelen = name_length;
    memcpy(this->name, name, name_length);
    this->name[name_length] = '\0';
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(prop_bt);
};

class prop_area {
 public:
  prop_area(const uint32_t magic, const uint32_t version) : magic_(magic), version_(version) {
    atomic_init(&serial_, 0);
    memset(reserved_, 0, sizeof(reserved_));
    // Allocate enough space for the root node.
    bytes_used_ = sizeof(prop_bt);
  }

  const prop_info* find(const char* name);
  bool del(const char *name);               // resetprop add
  bool add(const char* name, unsigned int namelen, const char* value, unsigned int valuelen);

  bool foreach (void (*propfn)(const prop_info* pi, void* cookie), void* cookie);

  atomic_uint_least32_t* serial() {
    return &serial_;
  }
  uint32_t magic() const {
    return magic_;
  }
  uint32_t version() const {
    return version_;
  }

 private:
  void* allocate_obj(const size_t size, uint_least32_t* const off);
  prop_bt* new_prop_bt(const char* name, uint32_t namelen, uint_least32_t* const off);
  prop_info* new_prop_info(const char* name, uint32_t namelen, const char* value, uint32_t valuelen,
                           uint_least32_t* const off);
  void* to_prop_obj(uint_least32_t off);
  prop_bt* to_prop_bt(atomic_uint_least32_t* off_p);
  prop_info* to_prop_info(atomic_uint_least32_t* off_p);

  prop_bt* root_node();

  prop_bt* find_prop_bt(prop_bt* const bt, const char* name, uint32_t namelen, bool alloc_if_needed);

  const prop_info* find_property(prop_bt* const trie, const char* name, uint32_t namelen,
                                 const char* value, uint32_t valuelen, bool alloc_if_needed);

  bool find_property_and_del(prop_bt *const trie, const char *name);    // resetprop add

  bool foreach_property(prop_bt* const trie, void (*propfn)(const prop_info* pi, void* cookie),
                        void* cookie);

  uint32_t bytes_used_;
  atomic_uint_least32_t serial_;
  uint32_t magic_;
  uint32_t version_;
  uint32_t reserved_[28];
  char data_[0];

  DISALLOW_COPY_AND_ASSIGN(prop_area);
};

struct prop_info {
  atomic_uint_least32_t serial;
  // we need to keep this buffer around because the property
  // value can be modified whereas name is constant.
  char value[PROP_VALUE_MAX];
  char name[0];

  prop_info(const char* name, uint32_t namelen, const char* value, uint32_t valuelen) {
    memcpy(this->name, name, namelen);
    this->name[namelen] = '\0';
    atomic_init(&this->serial, valuelen << 24);
    memcpy(this->value, value, valuelen);
    this->value[valuelen] = '\0';
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(prop_info);
};

// This is public because it was exposed in the NDK. As of 2017-01, ~60 apps reference this symbol.
// Change to static, we don't want to use the global libc reference
static prop_area* __system_property_area__ = nullptr;

static char property_filename[PROP_FILENAME_MAX] = PROP_FILENAME;
static size_t pa_data_size;
static size_t pa_size;
static bool initialized = false;

static prop_area* map_prop_area_rw(const char* filename, const char* context,
                                   bool* fsetxattr_failed) {
  /* dev is a tmpfs that we can use to carve a shared workspace
   * out of, so let's do that...
   */
  const int fd = open(filename, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC | O_EXCL, 0444);

  if (fd < 0) {
    if (errno == EACCES) {
      /* for consistency with the case where the process has already
       * mapped the page in and segfaults when trying to write to it
       */
      abort();
    }
    return nullptr;
  }

  if (context) {
    if (fsetxattr(fd, XATTR_NAME_SELINUX, context, strlen(context) + 1, 0) != 0) {
      // async_safe_format_log(ANDROID_LOG_ERROR, "libc",
      //                       "fsetxattr failed to set context (%s) for \"%s\"", context, filename);
      /*
       * fsetxattr() will fail during system properties tests due to selinux policy.
       * We do not want to create a custom policy for the tester, so we will continue in
       * this function but set a flag that an error has occurred.
       * Init, which is the only daemon that should ever call this function will abort
       * when this error occurs.
       * Otherwise, the tester will ignore it and continue, albeit without any selinux
       * property separation.
       */
      if (fsetxattr_failed) {
        *fsetxattr_failed = true;
      }
    }
  }

  if (ftruncate(fd, PA_SIZE) < 0) {
    close(fd);
    return nullptr;
  }

  pa_size = PA_SIZE;
  pa_data_size = pa_size - sizeof(prop_area);

  void* const memory_area = mmap(nullptr, pa_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (memory_area == MAP_FAILED) {
    close(fd);
    return nullptr;
  }

  prop_area* pa = new (memory_area) prop_area(PROP_AREA_MAGIC, PROP_AREA_VERSION);

  close(fd);
  return pa;
}

static prop_area* map_fd_ro(const int fd) {
  struct stat fd_stat;
  if (fstat(fd, &fd_stat) < 0) {
    return nullptr;
  }

  if ((fd_stat.st_uid != 0) || (fd_stat.st_gid != 0) ||
      ((fd_stat.st_mode & (S_IWGRP | S_IWOTH)) != 0) ||
      (fd_stat.st_size < static_cast<off_t>(sizeof(prop_area)))) {
    return nullptr;
  }

  pa_size = fd_stat.st_size;
  pa_data_size = pa_size - sizeof(prop_area);

  void* const map_result = mmap(nullptr, pa_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // resetprop: add PROT_WRITE
  if (map_result == MAP_FAILED) {
    return nullptr;
  }

  prop_area* pa = reinterpret_cast<prop_area*>(map_result);
  if ((pa->magic() != PROP_AREA_MAGIC) || (pa->version() != PROP_AREA_VERSION)) {
    munmap(pa, pa_size);
    return nullptr;
  }

  return pa;
}

static prop_area* map_prop_area(const char* filename) {
  int fd = open(filename, O_CLOEXEC | O_NOFOLLOW | O_RDWR);       // resetprop: O_RDONLY -> O_RDWR
  if (fd == -1) return nullptr;

  prop_area* map_result = map_fd_ro(fd);
  close(fd);

  return map_result;
}

void* prop_area::allocate_obj(const size_t size, uint_least32_t* const off) {
  const size_t aligned = BIONIC_ALIGN(size, sizeof(uint_least32_t));
  if (bytes_used_ + aligned > pa_data_size) {
    return nullptr;
  }

  *off = bytes_used_;
  bytes_used_ += aligned;
  return data_ + *off;
}

prop_bt* prop_area::new_prop_bt(const char* name, uint32_t namelen, uint_least32_t* const off) {
  uint_least32_t new_offset;
  void* const p = allocate_obj(sizeof(prop_bt) + namelen + 1, &new_offset);
  if (p != nullptr) {
    prop_bt* bt = new (p) prop_bt(name, namelen);
    *off = new_offset;
    return bt;
  }

  return nullptr;
}

prop_info* prop_area::new_prop_info(const char* name, uint32_t namelen, const char* value,
                                    uint32_t valuelen, uint_least32_t* const off) {
  uint_least32_t new_offset;
  void* const p = allocate_obj(sizeof(prop_info) + namelen + 1, &new_offset);
  if (p != nullptr) {
    prop_info* info = new (p) prop_info(name, namelen, value, valuelen);
    *off = new_offset;
    return info;
  }

  return nullptr;
}

void* prop_area::to_prop_obj(uint_least32_t off) {
  if (off > pa_data_size) return nullptr;

  return (data_ + off);
}

inline prop_bt* prop_area::to_prop_bt(atomic_uint_least32_t* off_p) {
  uint_least32_t off = atomic_load_explicit(off_p, memory_order_consume);
  return reinterpret_cast<prop_bt*>(to_prop_obj(off));
}

inline prop_info* prop_area::to_prop_info(atomic_uint_least32_t* off_p) {
  uint_least32_t off = atomic_load_explicit(off_p, memory_order_consume);
  return reinterpret_cast<prop_info*>(to_prop_obj(off));
}

inline prop_bt* prop_area::root_node() {
  return reinterpret_cast<prop_bt*>(to_prop_obj(0));
}

static int cmp_prop_name(const char* one, uint32_t one_len, const char* two, uint32_t two_len) {
  if (one_len < two_len)
    return -1;
  else if (one_len > two_len)
    return 1;
  else
    return strncmp(one, two, one_len);
}

prop_bt* prop_area::find_prop_bt(prop_bt* const bt, const char* name, uint32_t namelen,
                                 bool alloc_if_needed) {
  prop_bt* current = bt;
  while (true) {
    if (!current) {
      return nullptr;
    }

    const int ret = cmp_prop_name(name, namelen, current->name, current->namelen);
    if (ret == 0) {
      return current;
    }

    if (ret < 0) {
      uint_least32_t left_offset = atomic_load_explicit(&current->left, memory_order_relaxed);
      if (left_offset != 0) {
        current = to_prop_bt(&current->left);
      } else {
        if (!alloc_if_needed) {
          return nullptr;
        }

        uint_least32_t new_offset;
        prop_bt* new_bt = new_prop_bt(name, namelen, &new_offset);
        if (new_bt) {
          atomic_store_explicit(&current->left, new_offset, memory_order_release);
        }
        return new_bt;
      }
    } else {
      uint_least32_t right_offset = atomic_load_explicit(&current->right, memory_order_relaxed);
      if (right_offset != 0) {
        current = to_prop_bt(&current->right);
      } else {
        if (!alloc_if_needed) {
          return nullptr;
        }

        uint_least32_t new_offset;
        prop_bt* new_bt = new_prop_bt(name, namelen, &new_offset);
        if (new_bt) {
          atomic_store_explicit(&current->right, new_offset, memory_order_release);
        }
        return new_bt;
      }
    }
  }
}

const prop_info* prop_area::find_property(prop_bt* const trie, const char* name, uint32_t namelen,
                                          const char* value, uint32_t valuelen,
                                          bool alloc_if_needed) {
  if (!trie) return nullptr;

  const char* remaining_name = name;
  prop_bt* current = trie;
  while (true) {
    const char* sep = strchr(remaining_name, '.');
    const bool want_subtree = (sep != nullptr);
    const uint32_t substr_size = (want_subtree) ? sep - remaining_name : strlen(remaining_name);

    if (!substr_size) {
      return nullptr;
    }

    prop_bt* root = nullptr;
    uint_least32_t children_offset = atomic_load_explicit(&current->children, memory_order_relaxed);
    if (children_offset != 0) {
      root = to_prop_bt(&current->children);
    } else if (alloc_if_needed) {
      uint_least32_t new_offset;
      root = new_prop_bt(remaining_name, substr_size, &new_offset);
      if (root) {
        atomic_store_explicit(&current->children, new_offset, memory_order_release);
      }
    }

    if (!root) {
      return nullptr;
    }

    current = find_prop_bt(root, remaining_name, substr_size, alloc_if_needed);
    if (!current) {
      return nullptr;
    }

    if (!want_subtree) break;

    remaining_name = sep + 1;
  }

  uint_least32_t prop_offset = atomic_load_explicit(&current->prop, memory_order_relaxed);
  if (prop_offset != 0) {
    return to_prop_info(&current->prop);
  } else if (alloc_if_needed) {
    uint_least32_t new_offset;
    prop_info* new_info = new_prop_info(name, namelen, value, valuelen, &new_offset);
    if (new_info) {
      atomic_store_explicit(&current->prop, new_offset, memory_order_release);
    }

    return new_info;
  } else {
    return nullptr;
  }
}

bool prop_area::find_property_and_del(prop_bt* const trie, const char* name) {
  if (!trie) return false;

  const char* remaining_name = name;
  prop_bt* current = trie;
  while (true) {
    const char* sep = strchr(remaining_name, '.');
    const bool want_subtree = (sep != nullptr);
    const uint32_t substr_size = (want_subtree) ? sep - remaining_name : strlen(remaining_name);

    if (!substr_size) {
      return false;
    }

    prop_bt* root = nullptr;
    uint_least32_t children_offset = atomic_load_explicit(&current->children, memory_order_relaxed);
    if (children_offset != 0) {
      root = to_prop_bt(&current->children);
    }

    if (!root) {
      return false;
    }

    current = find_prop_bt(root, remaining_name, substr_size, false);
    if (!current) {
      return false;
    }

    if (!want_subtree) break;

    remaining_name = sep + 1;
  }

  uint_least32_t prop_offset = atomic_load_explicit(&current->prop, memory_order_relaxed);
  if (prop_offset != 0) {
    atomic_store_explicit(&current->prop, 0, memory_order_release);     // resetprop: nullify the offset to delete the prop
    return true;
  } else {
    return false;
  }
}

class PropertyServiceConnection {
 public:
  PropertyServiceConnection() : last_error_(0) {
    socket_ = ::socket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (socket_ == -1) {
      last_error_ = errno;
      return;
    }

    const size_t namelen = strlen(property_service_socket);
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    strlcpy(addr.sun_path, property_service_socket, sizeof(addr.sun_path));
    addr.sun_family = AF_LOCAL;
    socklen_t alen = namelen + offsetof(sockaddr_un, sun_path) + 1;

    if (TEMP_FAILURE_RETRY(connect(socket_, reinterpret_cast<sockaddr*>(&addr), alen)) == -1) {
      last_error_ = errno;
      close(socket_);
      socket_ = -1;
    }
  }

  bool IsValid() {
    return socket_ != -1;
  }

  int GetLastError() {
    return last_error_;
  }

  bool RecvInt32(int32_t* value) {
    int result = TEMP_FAILURE_RETRY(recv(socket_, value, sizeof(*value), MSG_WAITALL));
    return CheckSendRecvResult(result, sizeof(*value));
  }

  int socket() {
    return socket_;
  }

  ~PropertyServiceConnection() {
    if (socket_ != -1) {
      close(socket_);
    }
  }

 private:
  bool CheckSendRecvResult(int result, int expected_len) {
    if (result == -1) {
      last_error_ = errno;
    } else if (result != expected_len) {
      last_error_ = -1;
    } else {
      last_error_ = 0;
    }

    return last_error_ == 0;
  }

  int socket_;
  int last_error_;

  friend class SocketWriter;
};

class SocketWriter {
 public:
  explicit SocketWriter(PropertyServiceConnection* connection)
      : connection_(connection), iov_index_(0), uint_buf_index_(0)
  {}

  SocketWriter& WriteUint32(uint32_t value) {
    // CHECK(uint_buf_index_ < kUintBufSize);
    // CHECK(iov_index_ < kIovSize);
    uint32_t* ptr = uint_buf_ + uint_buf_index_;
    uint_buf_[uint_buf_index_++] = value;
    iov_[iov_index_].iov_base = ptr;
    iov_[iov_index_].iov_len = sizeof(*ptr);
    ++iov_index_;
    return *this;
  }

  SocketWriter& WriteString(const char* value) {
    uint32_t valuelen = strlen(value);
    WriteUint32(valuelen);
    if (valuelen == 0) {
      return *this;
    }

    // CHECK(iov_index_ < kIovSize);
    iov_[iov_index_].iov_base = const_cast<char*>(value);
    iov_[iov_index_].iov_len = valuelen;
    ++iov_index_;

    return *this;
  }

  bool Send() {
    if (!connection_->IsValid()) {
      return false;
    }

    if (writev(connection_->socket(), iov_, iov_index_) == -1) {
      connection_->last_error_ = errno;
      return false;
    }

    iov_index_ = uint_buf_index_ = 0;
    return true;
  }

 private:
  static constexpr size_t kUintBufSize = 8;
  static constexpr size_t kIovSize = 8;

  PropertyServiceConnection* connection_;
  iovec iov_[kIovSize];
  size_t iov_index_;
  uint32_t uint_buf_[kUintBufSize];
  size_t uint_buf_index_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(SocketWriter);
};

struct prop_msg {
  unsigned cmd;
  char name[PROP_NAME_MAX];
  char value[PROP_VALUE_MAX];
};

static int send_prop_msg(const prop_msg* msg) {
  PropertyServiceConnection connection;
  if (!connection.IsValid()) {
    return connection.GetLastError();
  }

  int result = -1;
  int s = connection.socket();

  const int num_bytes = TEMP_FAILURE_RETRY(send(s, msg, sizeof(prop_msg), 0));
  if (num_bytes == sizeof(prop_msg)) {
    // We successfully wrote to the property server but now we
    // wait for the property server to finish its work.  It
    // acknowledges its completion by closing the socket so we
    // poll here (on nothing), waiting for the socket to close.
    // If you 'adb shell setprop foo bar' you'll see the POLLHUP
    // once the socket closes.  Out of paranoia we cap our poll
    // at 250 ms.
    pollfd pollfds[1];
    pollfds[0].fd = s;
    pollfds[0].events = 0;
    const int poll_result = TEMP_FAILURE_RETRY(poll(pollfds, 1, 250 /* ms */));
    if (poll_result == 1 && (pollfds[0].revents & POLLHUP) != 0) {
      result = 0;
    } else {
      // Ignore the timeout and treat it like a success anyway.
      // The init process is single-threaded and its property
      // service is sometimes slow to respond (perhaps it's off
      // starting a child process or something) and thus this
      // times out and the caller thinks it failed, even though
      // it's still getting around to it.  So we fake it here,
      // mostly for ctl.* properties, but we do try and wait 250
      // ms so callers who do read-after-write can reliably see
      // what they've written.  Most of the time.
      // TODO: fix the system properties design.
      // async_safe_format_log(ANDROID_LOG_WARN, "libc",
      //                       "Property service has timed out while trying to set \"%s\" to \"%s\"",
      //                       msg->name, msg->value);
      result = 0;
    }
  }

  return result;
}

bool prop_area::foreach_property(prop_bt* const trie,
                                 void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  if (!trie) return false;

  uint_least32_t left_offset = atomic_load_explicit(&trie->left, memory_order_relaxed);
  if (left_offset != 0) {
    const int err = foreach_property(to_prop_bt(&trie->left), propfn, cookie);
    if (err < 0) return false;
  }
  uint_least32_t prop_offset = atomic_load_explicit(&trie->prop, memory_order_relaxed);
  if (prop_offset != 0) {
    prop_info* info = to_prop_info(&trie->prop);
    if (!info) return false;
    propfn(info, cookie);
  }
  uint_least32_t children_offset = atomic_load_explicit(&trie->children, memory_order_relaxed);
  if (children_offset != 0) {
    const int err = foreach_property(to_prop_bt(&trie->children), propfn, cookie);
    if (err < 0) return false;
  }
  uint_least32_t right_offset = atomic_load_explicit(&trie->right, memory_order_relaxed);
  if (right_offset != 0) {
    const int err = foreach_property(to_prop_bt(&trie->right), propfn, cookie);
    if (err < 0) return false;
  }

  return true;
}

const prop_info* prop_area::find(const char* name) {
  return find_property(root_node(), name, strlen(name), nullptr, 0, false);
}

bool prop_area::del(const char* name) {
  return find_property_and_del(root_node(), name);
}

bool prop_area::add(const char* name, unsigned int namelen, const char* value,
                    unsigned int valuelen) {
  return find_property(root_node(), name, namelen, value, valuelen, true);
}

bool prop_area::foreach (void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  return foreach_property(root_node(), propfn, cookie);
}

class context_node {
 public:
  context_node(context_node* next, const char* context, prop_area* pa)
      : next(next), context_(strdup(context)), pa_(pa), no_access_(false) {
    lock_.init(false);
  }
  ~context_node() {
    unmap();
    free(context_);
  }
  bool open(bool access_rw, bool* fsetxattr_failed);
  bool check_access_and_open();
  void reset_access();

  const char* context() const {
    return context_;
  }
  prop_area* pa() {
    return pa_;
  }

  context_node* next;

 private:
  bool check_access();
  void unmap();

  Lock lock_;
  char* context_;
  prop_area* pa_;
  bool no_access_;
};

struct prefix_node {
  prefix_node(struct prefix_node* next, const char* prefix, context_node* context)
      : prefix(strdup(prefix)), prefix_len(strlen(prefix)), context(context), next(next) {
  }
  ~prefix_node() {
    free(prefix);
  }
  char* prefix;
  const size_t prefix_len;
  context_node* context;
  struct prefix_node* next;
};

template <typename List, typename... Args>
static inline void list_add(List** list, Args... args) {
  *list = new List(*list, args...);
}

static void list_add_after_len(prefix_node** list, const char* prefix, context_node* context) {
  size_t prefix_len = strlen(prefix);

  auto next_list = list;

  while (*next_list) {
    if ((*next_list)->prefix_len < prefix_len || (*next_list)->prefix[0] == '*') {
      list_add(next_list, prefix, context);
      return;
    }
    next_list = &(*next_list)->next;
  }
  list_add(next_list, prefix, context);
}

template <typename List, typename Func>
static void list_foreach(List* list, Func func) {
  while (list) {
    func(list);
    list = list->next;
  }
}

template <typename List, typename Func>
static List* list_find(List* list, Func func) {
  while (list) {
    if (func(list)) {
      return list;
    }
    list = list->next;
  }
  return nullptr;
}

template <typename List>
static void list_free(List** list) {
  while (*list) {
    auto old_list = *list;
    *list = old_list->next;
    delete old_list;
  }
}

static prefix_node* prefixes = nullptr;
static context_node* contexts = nullptr;

/*
 * pthread_mutex_lock() calls into system_properties in the case of contention.
 * This creates a risk of dead lock if any system_properties functions
 * use pthread locks after system_property initialization.
 *
 * For this reason, the below three functions use a bionic Lock and static
 * allocation of memory for each filename.
 */

bool context_node::open(bool access_rw, bool* fsetxattr_failed) {
  lock_.lock();
  if (pa_) {
    lock_.unlock();
    return true;
  }

  char filename[PROP_FILENAME_MAX];
  int len = async_safe_format_buffer(filename, sizeof(filename), "%s/%s", property_filename,
                                     context_);
  if (len < 0 || len > PROP_FILENAME_MAX) {
    lock_.unlock();
    return false;
  }

  if (access_rw) {
    pa_ = map_prop_area_rw(filename, context_, fsetxattr_failed);
  } else {
    pa_ = map_prop_area(filename);
  }
  lock_.unlock();
  return pa_;
}

bool context_node::check_access_and_open() {
  if (!pa_ && !no_access_) {
    if (!check_access() || !open(false, nullptr)) {
      no_access_ = true;
    }
  }
  return pa_;
}

void context_node::reset_access() {
  if (!check_access()) {
    unmap();
    no_access_ = true;
  } else {
    no_access_ = false;
  }
}

bool context_node::check_access() {
  char filename[PROP_FILENAME_MAX];
  int len = async_safe_format_buffer(filename, sizeof(filename), "%s/%s", property_filename,
                                     context_);
  if (len < 0 || len > PROP_FILENAME_MAX) {
    return false;
  }

  return access(filename, R_OK) == 0;
}

void context_node::unmap() {
  if (!pa_) {
    return;
  }

  munmap(pa_, pa_size);
  if (pa_ == __system_property_area__) {
    __system_property_area__ = nullptr;
  }
  pa_ = nullptr;
}

static bool map_system_property_area(bool access_rw, bool* fsetxattr_failed) {
  char filename[PROP_FILENAME_MAX];
  int len =
      async_safe_format_buffer(filename, sizeof(filename), "%s/properties_serial",
                               property_filename);
  if (len < 0 || len > PROP_FILENAME_MAX) {
    __system_property_area__ = nullptr;
    return false;
  }

  if (access_rw) {
    __system_property_area__ =
        map_prop_area_rw(filename, "u:object_r:properties_serial:s0", fsetxattr_failed);
  } else {
    __system_property_area__ = map_prop_area(filename);
  }
  return __system_property_area__;
}

static prop_area* get_prop_area_for_name(const char* name) {
  auto entry = list_find(prefixes, [name](prefix_node* l) {
    return l->prefix[0] == '*' || !strncmp(l->prefix, name, l->prefix_len);
  });
  if (!entry) {
    return nullptr;
  }

  auto cnode = entry->context;
  if (!cnode->pa()) {
    /*
     * We explicitly do not check no_access_ in this case because unlike the
     * case of foreach(), we want to generate an selinux audit for each
     * non-permitted property access in this function.
     */
    cnode->open(false, nullptr);
  }
  return cnode->pa();
}

/*
 * The below two functions are duplicated from label_support.c in libselinux.
 * TODO: Find a location suitable for these functions such that both libc and
 * libselinux can share a common source file.
 */

/*
 * The read_spec_entries and read_spec_entry functions may be used to
 * replace sscanf to read entries from spec files. The file and
 * property services now use these.
 */

/* Read an entry from a spec file (e.g. file_contexts) */
static inline int read_spec_entry(char** entry, char** ptr, int* len) {
  *entry = nullptr;
  char* tmp_buf = nullptr;

  while (isspace(**ptr) && **ptr != '\0') (*ptr)++;

  tmp_buf = *ptr;
  *len = 0;

  while (!isspace(**ptr) && **ptr != '\0') {
    (*ptr)++;
    (*len)++;
  }

  if (*len) {
    *entry = strndup(tmp_buf, *len);
    if (!*entry) return -1;
  }

  return 0;
}

/*
 * line_buf - Buffer containing the spec entries .
 * num_args - The number of spec parameter entries to process.
 * ...      - A 'char **spec_entry' for each parameter.
 * returns  - The number of items processed.
 *
 * This function calls read_spec_entry() to do the actual string processing.
 */
static int read_spec_entries(char* line_buf, int num_args, ...) {
  char **spec_entry, *buf_p;
  int len, rc, items, entry_len = 0;
  va_list ap;

  len = strlen(line_buf);
  if (line_buf[len - 1] == '\n')
    line_buf[len - 1] = '\0';
  else
    /* Handle case if line not \n terminated by bumping
     * the len for the check below (as the line is NUL
     * terminated by getline(3)) */
    len++;

  buf_p = line_buf;
  while (isspace(*buf_p)) buf_p++;

  /* Skip comment lines and empty lines. */
  if (*buf_p == '#' || *buf_p == '\0') return 0;

  /* Process the spec file entries */
  va_start(ap, num_args);

  items = 0;
  while (items < num_args) {
    spec_entry = va_arg(ap, char**);

    if (len - 1 == buf_p - line_buf) {
      va_end(ap);
      return items;
    }

    rc = read_spec_entry(spec_entry, &buf_p, &entry_len);
    if (rc < 0) {
      va_end(ap);
      return rc;
    }
    if (entry_len) items++;
  }
  va_end(ap);
  return items;
}

static bool initialize_properties_from_file(const char* filename) {
  FILE* file = fopen(filename, "re");
  if (!file) {
    return false;
  }

  char* buffer = nullptr;
  size_t line_len;
  char* prop_prefix = nullptr;
  char* context = nullptr;

  while (getline(&buffer, &line_len, file) > 0) {
    int items = read_spec_entries(buffer, 2, &prop_prefix, &context);
    if (items <= 0) {
      continue;
    }
    if (items == 1) {
      free(prop_prefix);
      continue;
    }
    /*
     * init uses ctl.* properties as an IPC mechanism and does not write them
     * to a property file, therefore we do not need to create property files
     * to store them.
     */
    if (!strncmp(prop_prefix, "ctl.", 4)) {
      free(prop_prefix);
      free(context);
      continue;
    }

    auto old_context =
        list_find(contexts, [context](context_node* l) { return !strcmp(l->context(), context); });
    if (old_context) {
      list_add_after_len(&prefixes, prop_prefix, old_context);
    } else {
      list_add(&contexts, context, nullptr);
      list_add_after_len(&prefixes, prop_prefix, contexts);
    }
    free(prop_prefix);
    free(context);
  }

  free(buffer);
  fclose(file);

  return true;
}

static bool initialize_properties() {
  // If we do find /property_contexts, then this is being
  // run as part of the OTA updater on older release that had
  // /property_contexts - b/34370523
  if (initialize_properties_from_file("/property_contexts")) {
    return true;
  }

  // Use property_contexts from /system & /vendor, fall back to those from /
  if (access("/system/etc/selinux/plat_property_contexts", R_OK) != -1) {
    if (!initialize_properties_from_file("/system/etc/selinux/plat_property_contexts")) {
      return false;
    }
    if (!initialize_properties_from_file("/vendor/etc/selinux/nonplat_property_contexts")) {
      return false;
    }
  } else {
    if (!initialize_properties_from_file("/plat_property_contexts")) {
      return false;
    }
    if (!initialize_properties_from_file("/nonplat_property_contexts")) {
      return false;
    }
  }

  return true;
}

static bool is_dir(const char* pathname) {
  struct stat info;
  if (stat(pathname, &info) == -1) {
    return false;
  }
  return S_ISDIR(info.st_mode);
}

static void free_and_unmap_contexts() {
  list_free(&prefixes);
  list_free(&contexts);
  if (__system_property_area__) {
    munmap(__system_property_area__, pa_size);
    __system_property_area__ = nullptr;
  }
}

int __system_properties_init2() {
  // This is called from __libc_init_common, and should leave errno at 0 (http://b/37248982).
  ErrnoRestorer errno_restorer;

  if (initialized) {
    // list_foreach(contexts, [](context_node* l) { l->reset_access(); });    // resetprop remove
    return 0;
  }
  if (is_dir(property_filename)) {
    if (!initialize_properties()) {
      return -1;
    }
    if (!map_system_property_area(false, nullptr)) {
      free_and_unmap_contexts();
      return -1;
    }
  } else {
    __system_property_area__ = map_prop_area(property_filename);
    if (!__system_property_area__) {
      return -1;
    }
    list_add(&contexts, "legacy_system_prop_area", __system_property_area__);
    list_add_after_len(&prefixes, "*", contexts);
  }
  initialized = true;
  return 0;
}

int __system_property_set_filename2(const char* filename) {
  size_t len = strlen(filename);
  if (len >= sizeof(property_filename)) return -1;

  strcpy(property_filename, filename);
  return 0;
}

int __system_property_area_init2() {
  free_and_unmap_contexts();
  mkdir(property_filename, S_IRWXU | S_IXGRP | S_IXOTH);
  if (!initialize_properties()) {
    return -1;
  }
  bool open_failed = false;
  bool fsetxattr_failed = false;
  list_foreach(contexts, [&fsetxattr_failed, &open_failed](context_node* l) {
    if (!l->open(true, &fsetxattr_failed)) {
      open_failed = true;
    }
  });
  if (open_failed || !map_system_property_area(true, &fsetxattr_failed)) {
    free_and_unmap_contexts();
    return -1;
  }
  initialized = true;
  return fsetxattr_failed ? -2 : 0;
}

uint32_t __system_property_area_serial2() {
  prop_area* pa = __system_property_area__;
  if (!pa) {
    return -1;
  }
  // Make sure this read fulfilled before __system_property_serial
  return atomic_load_explicit(pa->serial(), memory_order_acquire);
}

const prop_info* __system_property_find2(const char* name) {
  if (!__system_property_area__) {
    return nullptr;
  }

  prop_area* pa = get_prop_area_for_name(name);
  if (!pa) {
    // async_safe_format_log(ANDROID_LOG_ERROR, "libc", "Access denied finding property \"%s\"", name);
    return nullptr;
  }

  return pa->find(name);
}

int __system_property_del(const char *name) {
  if (!__system_property_area__) {
    return 1;
  }

  prop_area* pa = get_prop_area_for_name(name);
  if (!pa) {
    return 1;
  }

  if (!pa->del(name))
    return 1;

  // We want to make sure that updates are visible to readers
  atomic_store_explicit(
      __system_property_area__->serial(),
      atomic_load_explicit(__system_property_area__->serial(), memory_order_relaxed) + 1,
      memory_order_release);
  __futex_wake(__system_property_area__->serial(), INT32_MAX);
  return 0;
}

// The C11 standard doesn't allow atomic loads from const fields,
// though C++11 does.  Fudge it until standards get straightened out.
static inline uint_least32_t load_const_atomic(const atomic_uint_least32_t* s, memory_order mo) {
  atomic_uint_least32_t* non_const_s = const_cast<atomic_uint_least32_t*>(s);
  return atomic_load_explicit(non_const_s, mo);
}

int __system_property_read2(const prop_info* pi, char* name, char* value) {
  while (true) {
    uint32_t serial = __system_property_serial2(pi);  // acquire semantics
    size_t len = SERIAL_VALUE_LEN(serial);
    memcpy(value, pi->value, len + 1);
    // TODO: Fix the synchronization scheme here.
    // There is no fully supported way to implement this kind
    // of synchronization in C++11, since the memcpy races with
    // updates to pi, and the data being accessed is not atomic.
    // The following fence is unintuitive, but would be the
    // correct one if memcpy used memory_order_relaxed atomic accesses.
    // In practice it seems unlikely that the generated code would
    // would be any different, so this should be OK.
    atomic_thread_fence(memory_order_acquire);
    if (serial == load_const_atomic(&(pi->serial), memory_order_relaxed)) {
      if (name != nullptr) {
        size_t namelen = strlcpy(name, pi->name, PROP_NAME_MAX);
        // if (namelen >= PROP_NAME_MAX) {
        //   async_safe_format_log(ANDROID_LOG_ERROR, "libc",
        //                         "The property name length for \"%s\" is >= %d;"
        //                         " please use __system_property_read_callback2"
        //                         " to read this property. (the name is truncated to \"%s\")",
        //                         pi->name, PROP_NAME_MAX - 1, name);
        // }
      }
      return len;
    }
  }
}

void __system_property_read_callback2(const prop_info* pi,
                                     void (*callback)(void* cookie,
                                                      const char* name,
                                                      const char* value,
                                                      uint32_t serial),
                                     void* cookie) {
  while (true) {
    uint32_t serial = __system_property_serial2(pi);  // acquire semantics
    size_t len = SERIAL_VALUE_LEN(serial);
    char value_buf[len + 1];

    memcpy(value_buf, pi->value, len);
    value_buf[len] = '\0';

    // TODO: see todo in __system_property_read function
    atomic_thread_fence(memory_order_acquire);
    if (serial == load_const_atomic(&(pi->serial), memory_order_relaxed)) {
      callback(cookie, pi->name, value_buf, serial);
      return;
    }
  }
}

int __system_property_get2(const char* name, char* value) {
  const prop_info* pi = __system_property_find2(name);

  if (pi != 0) {
    return __system_property_read2(pi, nullptr, value);
  } else {
    value[0] = 0;
    return 0;
  }
}

static constexpr uint32_t kProtocolVersion1 = 1;
static constexpr uint32_t kProtocolVersion2 = 2;  // current

static atomic_uint_least32_t g_propservice_protocol_version = 0;

static void detect_protocol_version() {
  char value[PROP_VALUE_MAX];
  if (__system_property_get2(kServiceVersionPropertyName, value) == 0) {
    g_propservice_protocol_version = kProtocolVersion1;
    // async_safe_format_log(ANDROID_LOG_WARN, "libc",
    //                       "Using old property service protocol (\"%s\" is not set)",
    //                       kServiceVersionPropertyName);
  } else {
    uint32_t version = static_cast<uint32_t>(atoll(value));
    if (version >= kProtocolVersion2) {
      g_propservice_protocol_version = kProtocolVersion2;
    } else {
      // async_safe_format_log(ANDROID_LOG_WARN, "libc",
      //                       "Using old property service protocol (\"%s\"=\"%s\")",
      //                       kServiceVersionPropertyName, value);
      g_propservice_protocol_version = kProtocolVersion1;
    }
  }
}

int __system_property_set2(const char* key, const char* value) {
  if (key == nullptr) return -1;
  if (value == nullptr) value = "";
  if (strlen(value) >= PROP_VALUE_MAX) return -1;

  if (g_propservice_protocol_version == 0) {
    detect_protocol_version();
  }

  if (g_propservice_protocol_version == kProtocolVersion1) {
    // Old protocol does not support long names
    if (strlen(key) >= PROP_NAME_MAX) return -1;

    prop_msg msg;
    memset(&msg, 0, sizeof msg);
    msg.cmd = PROP_MSG_SETPROP;
    strlcpy(msg.name, key, sizeof msg.name);
    strlcpy(msg.value, value, sizeof msg.value);

    return send_prop_msg(&msg);
  } else {
    // Use proper protocol
    PropertyServiceConnection connection;
    if (!connection.IsValid()) {
      // errno = connection.GetLastError();
      // async_safe_format_log(ANDROID_LOG_WARN,
      //                       "libc",
      //                       "Unable to set property \"%s\" to \"%s\": connection failed; errno=%d (%s)",
      //                       key,
      //                       value,
      //                       errno,
      //                       strerror(errno));
      return -1;
    }

    SocketWriter writer(&connection);
    if (!writer.WriteUint32(PROP_MSG_SETPROP2).WriteString(key).WriteString(value).Send()) {
      // errno = connection.GetLastError();
      // async_safe_format_log(ANDROID_LOG_WARN,
      //                       "libc",
      //                       "Unable to set property \"%s\" to \"%s\": write failed; errno=%d (%s)",
      //                       key,
      //                       value,
      //                       errno,
      //                       strerror(errno));
      return -1;
    }

    int result = -1;
    if (!connection.RecvInt32(&result)) {
      // errno = connection.GetLastError();
      // async_safe_format_log(ANDROID_LOG_WARN,
      //                       "libc",
      //                       "Unable to set property \"%s\" to \"%s\": recv failed; errno=%d (%s)",
      //                       key,
      //                       value,
      //                       errno,
      //                       strerror(errno));
      return -1;
    }

    if (result != PROP_SUCCESS) {
      // async_safe_format_log(ANDROID_LOG_WARN,
      //                       "libc",
      //                       "Unable to set property \"%s\" to \"%s\": error code: 0x%x",
      //                       key,
      //                       value,
      //                       result);
      return -1;
    }

    return 0;
  }
}

int __system_property_update2(prop_info* pi, const char* value, unsigned int len) {
  if (len >= PROP_VALUE_MAX) {
    return -1;
  }

  prop_area* pa = __system_property_area__;

  if (!pa) {
    return -1;
  }

  uint32_t serial = atomic_load_explicit(&pi->serial, memory_order_relaxed);
  serial |= 1;
  atomic_store_explicit(&pi->serial, serial, memory_order_relaxed);
  // The memcpy call here also races.  Again pretend it
  // used memory_order_relaxed atomics, and use the analogous
  // counterintuitive fence.
  atomic_thread_fence(memory_order_release);
  strlcpy(pi->value, value, len + 1);

  atomic_store_explicit(&pi->serial, (len << 24) | ((serial + 1) & 0xffffff), memory_order_release);
  __futex_wake(&pi->serial, INT32_MAX);

  atomic_store_explicit(pa->serial(), atomic_load_explicit(pa->serial(), memory_order_relaxed) + 1,
                        memory_order_release);
  __futex_wake(pa->serial(), INT32_MAX);

  return 0;
}

int __system_property_add2(const char* name, unsigned int namelen, const char* value,
                          unsigned int valuelen) {
  if (valuelen >= PROP_VALUE_MAX) {
    return -1;
  }

  if (namelen < 1) {
    return -1;
  }

  if (!__system_property_area__) {
    return -1;
  }

  prop_area* pa = get_prop_area_for_name(name);

  if (!pa) {
    // async_safe_format_log(ANDROID_LOG_ERROR, "libc", "Access denied adding property \"%s\"", name);
    return -1;
  }

  bool ret = pa->add(name, namelen, value, valuelen);
  if (!ret) {
    return -1;
  }

  // There is only a single mutator, but we want to make sure that
  // updates are visible to a reader waiting for the update.
  atomic_store_explicit(
      __system_property_area__->serial(),
      atomic_load_explicit(__system_property_area__->serial(), memory_order_relaxed) + 1,
      memory_order_release);
  __futex_wake(__system_property_area__->serial(), INT32_MAX);
  return 0;
}

// Wait for non-locked serial, and retrieve it with acquire semantics.
uint32_t __system_property_serial2(const prop_info* pi) {
  uint32_t serial = load_const_atomic(&pi->serial, memory_order_acquire);
  while (SERIAL_DIRTY(serial)) {
    __futex_wait(const_cast<_Atomic(uint_least32_t)*>(&pi->serial), serial, nullptr);
    serial = load_const_atomic(&pi->serial, memory_order_acquire);
  }
  return serial;
}

uint32_t __system_property_wait_any2(uint32_t old_serial) {
  uint32_t new_serial;
  __system_property_wait2(nullptr, old_serial, &new_serial, nullptr);
  return new_serial;
}

bool __system_property_wait2(const prop_info* pi,
                            uint32_t old_serial,
                            uint32_t* new_serial_ptr,
                            const timespec* relative_timeout) {
  // Are we waiting on the global serial or a specific serial?
  atomic_uint_least32_t* serial_ptr;
  if (pi == nullptr) {
    if (__system_property_area__ == nullptr) return -1;
    serial_ptr = __system_property_area__->serial();
  } else {
    serial_ptr = const_cast<atomic_uint_least32_t*>(&pi->serial);
  }

  uint32_t new_serial;
  do {
    int rc;
    if ((rc = __futex_wait(serial_ptr, old_serial, relative_timeout)) != 0 && rc == -ETIMEDOUT) {
      return false;
    }
    new_serial = load_const_atomic(serial_ptr, memory_order_acquire);
  } while (new_serial == old_serial);

  *new_serial_ptr = new_serial;
  return true;
}

const prop_info* __system_property_find_nth2(unsigned n) {
  struct find_nth {
    const uint32_t sought;
    uint32_t current;
    const prop_info* result;

    explicit find_nth(uint32_t n) : sought(n), current(0), result(nullptr) {}
    static void fn(const prop_info* pi, void* ptr) {
      find_nth* self = reinterpret_cast<find_nth*>(ptr);
      if (self->current++ == self->sought) self->result = pi;
    }
  } state(n);
  __system_property_foreach2(find_nth::fn, &state);
  return state.result;
}

int __system_property_foreach2(void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  if (!__system_property_area__) {
    return -1;
  }

  list_foreach(contexts, [propfn, cookie](context_node* l) {
    if (l->check_access_and_open()) {
      l->pa()->foreach(propfn, cookie);
    }
  });
  return 0;
}
