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

#include "system_properties/prop_area.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <new>

#include <async_safe/log.h>

constexpr size_t PA_SIZE = 128 * 1024;
constexpr uint32_t PROP_AREA_MAGIC = 0x504f5250;
constexpr uint32_t PROP_AREA_VERSION = 0xfc6ed0ab;

size_t prop_area::pa_size_ = 0;
size_t prop_area::pa_data_size_ = 0;

prop_area* prop_area::map_prop_area_rw(const char* filename, const char* context,
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
      async_safe_format_log(ANDROID_LOG_ERROR, "libc",
                            "fsetxattr failed to set context (%s) for \"%s\"", context, filename);
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

  pa_size_ = PA_SIZE;
  pa_data_size_ = pa_size_ - sizeof(prop_area);

  void* const memory_area = mmap(nullptr, pa_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (memory_area == MAP_FAILED) {
    close(fd);
    return nullptr;
  }

  prop_area* pa = new (memory_area) prop_area(PROP_AREA_MAGIC, PROP_AREA_VERSION);

  close(fd);
  return pa;
}

/* resetprop: map_fd_ro -> map_fd_rw */
prop_area* prop_area::map_fd_rw(const int fd) {
  struct stat fd_stat;
  if (fstat(fd, &fd_stat) < 0) {
    return nullptr;
  }

  if ((fd_stat.st_uid != 0) || (fd_stat.st_gid != 0) ||
      ((fd_stat.st_mode & (S_IWGRP | S_IWOTH)) != 0) ||
      (fd_stat.st_size < static_cast<off_t>(sizeof(prop_area)))) {
    return nullptr;
  }

  pa_size_ = fd_stat.st_size;
  pa_data_size_ = pa_size_ - sizeof(prop_area);

  /* resetprop: add PROT_WRITE */
  void* const map_result = mmap(nullptr, pa_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map_result == MAP_FAILED) {
    return nullptr;
  }

  prop_area* pa = reinterpret_cast<prop_area*>(map_result);
  if ((pa->magic() != PROP_AREA_MAGIC) || (pa->version() != PROP_AREA_VERSION)) {
    munmap(pa, pa_size_);
    return nullptr;
  }

  return pa;
}

prop_area* prop_area::map_prop_area(const char* filename) {
  /* resetprop: O_RDONLY -> O_RDWR */
  int fd = open(filename, O_CLOEXEC | O_NOFOLLOW | O_RDWR);
  if (fd == -1) return nullptr;

  prop_area* map_result = map_fd_rw(fd);
  close(fd);

  return map_result;
}

void* prop_area::allocate_obj(const size_t size, uint_least32_t* const off) {
  const size_t aligned = __BIONIC_ALIGN(size, sizeof(uint_least32_t));
  if (bytes_used_ + aligned > pa_data_size_) {
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
  if (p == nullptr) return nullptr;

  prop_info* info;
  if (valuelen >= PROP_VALUE_MAX) {
    uint32_t long_value_offset = 0;
    char* long_location = reinterpret_cast<char*>(allocate_obj(valuelen + 1, &long_value_offset));
    if (!long_location) return nullptr;

    memcpy(long_location, value, valuelen);
    long_location[valuelen] = '\0';

    // Both new_offset and long_value_offset are offsets based off of data_, however prop_info
    // does not know what data_ is, so we change this offset to be an offset from the prop_info
    // pointer that contains it.
    long_value_offset -= new_offset;

    info = new (p) prop_info(name, namelen, long_value_offset);
  } else {
    info = new (p) prop_info(name, namelen, value, valuelen);
  }
  *off = new_offset;
  return info;
}

void* prop_area::to_prop_obj(uint_least32_t off) {
  if (off > pa_data_size_) return nullptr;

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

prop_bt *prop_area::find_prop_bt(prop_bt *const trie, const char *name, bool alloc_if_needed) {
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
  return current;
}

const prop_info* prop_area::find_property(prop_bt* const trie, const char* name, uint32_t namelen,
                                          const char* value, uint32_t valuelen,
                                          bool alloc_if_needed) {
  const char* remaining_name = name;
  prop_bt* current = find_prop_bt(trie, name, alloc_if_needed);
  if (!current)
	  return nullptr;

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

bool prop_area::add(const char* name, unsigned int namelen, const char* value,
                    unsigned int valuelen) {
  return find_property(root_node(), name, namelen, value, valuelen, true);
}

bool prop_area::del(const char *name) {
  prop_bt* node = find_prop_bt(root_node(), name, false);
  if (!node)
    return false;
  atomic_store_explicit(&node->prop, 0, memory_order_release);
  return true;
}

bool prop_area::foreach (void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  return foreach_property(root_node(), propfn, cookie);
}
