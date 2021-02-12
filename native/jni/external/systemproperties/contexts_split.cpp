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

#include "system_properties/contexts_split.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <async_safe/log.h>

#include "system_properties/context_node.h"
#include "system_properties/system_properties.h"

class ContextListNode : public ContextNode {
 public:
  ContextListNode(ContextListNode* next, const char* context, const char* filename)
      : ContextNode(strdup(context), filename), next(next) {
  }

  ~ContextListNode() {
    free(const_cast<char*>(context()));
  }

  ContextListNode* next;
};

struct PrefixNode {
  PrefixNode(struct PrefixNode* next, const char* prefix, ContextListNode* context)
      : prefix(strdup(prefix)), prefix_len(strlen(prefix)), context(context), next(next) {
  }
  ~PrefixNode() {
    free(prefix);
  }
  char* prefix;
  const size_t prefix_len;
  ContextListNode* context;
  PrefixNode* next;
};

template <typename List, typename... Args>
static inline void ListAdd(List** list, Args... args) {
  *list = new List(*list, args...);
}

static void ListAddAfterLen(PrefixNode** list, const char* prefix, ContextListNode* context) {
  size_t prefix_len = strlen(prefix);

  auto next_list = list;

  while (*next_list) {
    if ((*next_list)->prefix_len < prefix_len || (*next_list)->prefix[0] == '*') {
      ListAdd(next_list, prefix, context);
      return;
    }
    next_list = &(*next_list)->next;
  }
  ListAdd(next_list, prefix, context);
}

template <typename List, typename Func>
static void ListForEach(List* list, Func func) {
  while (list) {
    func(list);
    list = list->next;
  }
}

template <typename List, typename Func>
static List* ListFind(List* list, Func func) {
  while (list) {
    if (func(list)) {
      return list;
    }
    list = list->next;
  }
  return nullptr;
}

template <typename List>
static void ListFree(List** list) {
  while (*list) {
    auto old_list = *list;
    *list = old_list->next;
    delete old_list;
  }
}

// The below two functions are duplicated from label_support.c in libselinux.
// TODO: Find a location suitable for these functions such that both libc and
// libselinux can share a common source file.

// The read_spec_entries and read_spec_entry functions may be used to
// replace sscanf to read entries from spec files. The file and
// property services now use these.

// Read an entry from a spec file (e.g. file_contexts)
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

// line_buf - Buffer containing the spec entries .
// num_args - The number of spec parameter entries to process.
// ...      - A 'char **spec_entry' for each parameter.
// returns  - The number of items processed.
//
// This function calls read_spec_entry() to do the actual string processing.
static int read_spec_entries(char* line_buf, int num_args, ...) {
  char **spec_entry, *buf_p;
  int len, rc, items, entry_len = 0;
  va_list ap;

  len = strlen(line_buf);
  if (line_buf[len - 1] == '\n')
    line_buf[len - 1] = '\0';
  else
    // Handle case if line not \n terminated by bumping
    // the len for the check below (as the line is NUL
    // terminated by getline(3))
    len++;

  buf_p = line_buf;
  while (isspace(*buf_p)) buf_p++;

  // Skip comment lines and empty lines.
  if (*buf_p == '#' || *buf_p == '\0') return 0;

  // Process the spec file entries
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

bool ContextsSplit::MapSerialPropertyArea(bool access_rw, bool* fsetxattr_failed) {
  char filename[PROP_FILENAME_MAX];
  int len = async_safe_format_buffer(filename, sizeof(filename), "%s/properties_serial", filename_);
  if (len < 0 || len >= PROP_FILENAME_MAX) {
    serial_prop_area_ = nullptr;
    return false;
  }

  if (access_rw) {
    serial_prop_area_ =
        prop_area::map_prop_area_rw(filename, "u:object_r:properties_serial:s0", fsetxattr_failed);
  } else {
    serial_prop_area_ = prop_area::map_prop_area(filename);
  }
  return serial_prop_area_;
}

bool ContextsSplit::InitializePropertiesFromFile(const char* filename) {
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

    // init uses ctl.* properties as an IPC mechanism and does not write them
    // to a property file, therefore we do not need to create property files
    // to store them.
    if (!strncmp(prop_prefix, "ctl.", 4)) {
      free(prop_prefix);
      free(context);
      continue;
    }

    auto old_context = ListFind(
        contexts_, [context](ContextListNode* l) { return !strcmp(l->context(), context); });
    if (old_context) {
      ListAddAfterLen(&prefixes_, prop_prefix, old_context);
    } else {
      ListAdd(&contexts_, context, filename_);
      ListAddAfterLen(&prefixes_, prop_prefix, contexts_);
    }
    free(prop_prefix);
    free(context);
  }

  free(buffer);
  fclose(file);

  return true;
}

bool ContextsSplit::InitializeProperties() {
  // If we do find /property_contexts, then this is being
  // run as part of the OTA updater on older release that had
  // /property_contexts - b/34370523
  if (InitializePropertiesFromFile("/property_contexts")) {
    return true;
  }

  // Use property_contexts from /system & /vendor, fall back to those from /
  if (access("/system/etc/selinux/plat_property_contexts", R_OK) != -1) {
    if (!InitializePropertiesFromFile("/system/etc/selinux/plat_property_contexts")) {
      return false;
    }
    // Don't check for failure here, so we always have a sane list of properties.
    // E.g. In case of recovery, the vendor partition will not have mounted and we
    // still need the system / platform properties to function.
    if (access("/vendor/etc/selinux/vendor_property_contexts", R_OK) != -1) {
      InitializePropertiesFromFile("/vendor/etc/selinux/vendor_property_contexts");
    } else {
      // Fallback to nonplat_* if vendor_* doesn't exist.
      InitializePropertiesFromFile("/vendor/etc/selinux/nonplat_property_contexts");
    }
  } else {
    if (!InitializePropertiesFromFile("/plat_property_contexts")) {
      return false;
    }
    if (access("/vendor_property_contexts", R_OK) != -1) {
      InitializePropertiesFromFile("/vendor_property_contexts");
    } else {
      // Fallback to nonplat_* if vendor_* doesn't exist.
      InitializePropertiesFromFile("/nonplat_property_contexts");
    }
  }

  return true;
}

bool ContextsSplit::Initialize(bool writable, const char* filename, bool* fsetxattr_failed) {
  filename_ = filename;
  if (!InitializeProperties()) {
    return false;
  }

  if (writable) {
    mkdir(filename_, S_IRWXU | S_IXGRP | S_IXOTH);
    bool open_failed = false;
    if (fsetxattr_failed) {
      *fsetxattr_failed = false;
    }

    ListForEach(contexts_, [&fsetxattr_failed, &open_failed](ContextListNode* l) {
      if (!l->Open(true, fsetxattr_failed)) {
        open_failed = true;
      }
    });
    if (open_failed || !MapSerialPropertyArea(true, fsetxattr_failed)) {
      FreeAndUnmap();
      return false;
    }
  } else {
    if (!MapSerialPropertyArea(false, nullptr)) {
      FreeAndUnmap();
      return false;
    }
  }
  return true;
}

prop_area* ContextsSplit::GetPropAreaForName(const char* name) {
  auto entry = ListFind(prefixes_, [name](PrefixNode* l) {
    return l->prefix[0] == '*' || !strncmp(l->prefix, name, l->prefix_len);
  });
  if (!entry) {
    return nullptr;
  }

  auto cnode = entry->context;
  if (!cnode->pa()) {
    // We explicitly do not check no_access_ in this case because unlike the
    // case of foreach(), we want to generate an selinux audit for each
    // non-permitted property access in this function.
    cnode->Open(false, nullptr);
  }
  return cnode->pa();
}

void ContextsSplit::ForEach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  ListForEach(contexts_, [propfn, cookie](ContextListNode* l) {
    if (l->CheckAccessAndOpen()) {
      l->pa()->foreach (propfn, cookie);
    }
  });
}

void ContextsSplit::ResetAccess() {
  ListForEach(contexts_, [](ContextListNode* l) { l->ResetAccess(); });
}

void ContextsSplit::FreeAndUnmap() {
  ListFree(&prefixes_);
  ListFree(&contexts_);
  prop_area::unmap_prop_area(&serial_prop_area_);
}
