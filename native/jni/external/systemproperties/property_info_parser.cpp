//
// Copyright (C) 2017 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "property_info_parser/property_info_parser.h"

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace android {
namespace properties {

namespace {

// Binary search to find index of element in an array compared via f(search).
template <typename F>
int Find(uint32_t array_length, F&& f) {
  int bottom = 0;
  int top = array_length - 1;
  while (top >= bottom) {
    int search = (top + bottom) / 2;

    auto cmp = f(search);

    if (cmp == 0) return search;
    if (cmp < 0) bottom = search + 1;
    if (cmp > 0) top = search - 1;
  }
  return -1;
}

}  // namespace

// Binary search the list of contexts to find the index of a given context string.
// Only should be used for TrieSerializer to construct the Trie.
int PropertyInfoArea::FindContextIndex(const char* context) const {
  return Find(num_contexts(), [this, context](auto array_offset) {
    auto string_offset = uint32_array(contexts_array_offset())[array_offset];
    return strcmp(c_string(string_offset), context);
  });
}

// Binary search the list of types to find the index of a given type string.
// Only should be used for TrieSerializer to construct the Trie.
int PropertyInfoArea::FindTypeIndex(const char* type) const {
  return Find(num_types(), [this, type](auto array_offset) {
    auto string_offset = uint32_array(types_array_offset())[array_offset];
    return strcmp(c_string(string_offset), type);
  });
}

// Binary search the list of children nodes to find a TrieNode for a given property piece.
// Used to traverse the Trie in GetPropertyInfoIndexes().
bool TrieNode::FindChildForString(const char* name, uint32_t namelen, TrieNode* child) const {
  auto node_index = Find(trie_node_base_->num_child_nodes, [this, name, namelen](auto array_offset) {
    const char* child_name = child_node(array_offset).name();
    int cmp = strncmp(child_name, name, namelen);
    if (cmp == 0 && child_name[namelen] != '\0') {
      // We use strncmp() since name isn't null terminated, but we don't want to match only a
      // prefix of a child node's name, so we check here if we did only match a prefix and
      // return 1, to indicate to the binary search to search earlier in the array for the real
      // match.
      return 1;
    }
    return cmp;
  });

  if (node_index == -1) {
    return false;
  }
  *child = child_node(node_index);
  return true;
}

void PropertyInfoArea::CheckPrefixMatch(const char* remaining_name, const TrieNode& trie_node,
                                        uint32_t* context_index, uint32_t* type_index) const {
  const uint32_t remaining_name_size = strlen(remaining_name);
  for (uint32_t i = 0; i < trie_node.num_prefixes(); ++i) {
    auto prefix_len = trie_node.prefix(i)->namelen;
    if (prefix_len > remaining_name_size) continue;

    if (!strncmp(c_string(trie_node.prefix(i)->name_offset), remaining_name, prefix_len)) {
      if (trie_node.prefix(i)->context_index != ~0u) {
        *context_index = trie_node.prefix(i)->context_index;
      }
      if (trie_node.prefix(i)->type_index != ~0u) {
        *type_index = trie_node.prefix(i)->type_index;
      }
      return;
    }
  }
}

void PropertyInfoArea::GetPropertyInfoIndexes(const char* name, uint32_t* context_index,
                                              uint32_t* type_index) const {
  uint32_t return_context_index = ~0u;
  uint32_t return_type_index = ~0u;
  const char* remaining_name = name;
  auto trie_node = root_node();
  while (true) {
    const char* sep = strchr(remaining_name, '.');

    // Apply prefix match for prefix deliminated with '.'
    if (trie_node.context_index() != ~0u) {
      return_context_index = trie_node.context_index();
    }
    if (trie_node.type_index() != ~0u) {
      return_type_index = trie_node.type_index();
    }

    // Check prefixes at this node.  This comes after the node check since these prefixes are by
    // definition longer than the node itself.
    CheckPrefixMatch(remaining_name, trie_node, &return_context_index, &return_type_index);

    if (sep == nullptr) {
      break;
    }

    const uint32_t substr_size = sep - remaining_name;
    TrieNode child_node;
    if (!trie_node.FindChildForString(remaining_name, substr_size, &child_node)) {
      break;
    }

    trie_node = child_node;
    remaining_name = sep + 1;
  }

  // We've made it to a leaf node, so check contents and return appropriately.
  // Check exact matches
  for (uint32_t i = 0; i < trie_node.num_exact_matches(); ++i) {
    if (!strcmp(c_string(trie_node.exact_match(i)->name_offset), remaining_name)) {
      if (context_index != nullptr) {
        if (trie_node.exact_match(i)->context_index != ~0u) {
          *context_index = trie_node.exact_match(i)->context_index;
        } else {
          *context_index = return_context_index;
        }
      }
      if (type_index != nullptr) {
        if (trie_node.exact_match(i)->type_index != ~0u) {
          *type_index = trie_node.exact_match(i)->type_index;
        } else {
          *type_index = return_type_index;
        }
      }
      return;
    }
  }
  // Check prefix matches for prefixes not deliminated with '.'
  CheckPrefixMatch(remaining_name, trie_node, &return_context_index, &return_type_index);
  // Return previously found prefix match.
  if (context_index != nullptr) *context_index = return_context_index;
  if (type_index != nullptr) *type_index = return_type_index;
  return;
}

void PropertyInfoArea::GetPropertyInfo(const char* property, const char** context,
                                       const char** type) const {
  uint32_t context_index;
  uint32_t type_index;
  GetPropertyInfoIndexes(property, &context_index, &type_index);
  if (context != nullptr) {
    if (context_index == ~0u) {
      *context = nullptr;
    } else {
      *context = this->context(context_index);
    }
  }
  if (type != nullptr) {
    if (type_index == ~0u) {
      *type = nullptr;
    } else {
      *type = this->type(type_index);
    }
  }
}

bool PropertyInfoAreaFile::LoadDefaultPath() {
  return LoadPath("/dev/__properties__/property_info");
}

bool PropertyInfoAreaFile::LoadPath(const char* filename) {
  int fd = open(filename, O_CLOEXEC | O_NOFOLLOW | O_RDONLY);

  struct stat fd_stat;
  if (fstat(fd, &fd_stat) < 0) {
    close(fd);
    return false;
  }

  if ((fd_stat.st_uid != 0) || (fd_stat.st_gid != 0) ||
      ((fd_stat.st_mode & (S_IWGRP | S_IWOTH)) != 0) ||
      (fd_stat.st_size < static_cast<off_t>(sizeof(PropertyInfoArea)))) {
    close(fd);
    return false;
  }

  auto mmap_size = fd_stat.st_size;

  void* map_result = mmap(nullptr, mmap_size, PROT_READ, MAP_SHARED, fd, 0);
  if (map_result == MAP_FAILED) {
    close(fd);
    return false;
  }

  auto property_info_area = reinterpret_cast<PropertyInfoArea*>(map_result);
  if (property_info_area->minimum_supported_version() > 1 ||
      property_info_area->size() != mmap_size) {
    munmap(map_result, mmap_size);
    close(fd);
    return false;
  }

  close(fd);
  mmap_base_ = map_result;
  mmap_size_ = mmap_size;
  return true;
}

void PropertyInfoAreaFile::Reset() {
  if (mmap_size_ > 0) {
    munmap(mmap_base_, mmap_size_);
  }
  mmap_base_ = nullptr;
  mmap_size_ = 0;
}

}  // namespace properties
}  // namespace android
