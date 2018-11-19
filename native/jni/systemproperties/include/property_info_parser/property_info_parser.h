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

#ifndef PROPERTY_INFO_PARSER_H
#define PROPERTY_INFO_PARSER_H

#include <stdint.h>
#include <stdlib.h>

namespace android {
namespace properties {

// The below structs intentionally do not end with char name[0] or other tricks to allocate
// with a dynamic size, such that they can be added onto in the future without breaking
// backwards compatibility.
struct PropertyEntry {
  uint32_t name_offset;
  uint32_t namelen;

  // This is the context match for this node_; ~0u if it doesn't correspond to any.
  uint32_t context_index;
  // This is the type for this node_; ~0u if it doesn't correspond to any.
  uint32_t type_index;
};

struct TrieNodeInternal {
  // This points to a property entry struct, which includes the name for this node
  uint32_t property_entry;

  // Children are a sorted list of child nodes_; binary search them.
  uint32_t num_child_nodes;
  uint32_t child_nodes;

  // Prefixes are terminating prefix matches at this node, sorted longest to smallest
  // Take the first match sequentially found with StartsWith().
  uint32_t num_prefixes;
  uint32_t prefix_entries;

  // Exact matches are a sorted list of exact matches at this node_; binary search them.
  uint32_t num_exact_matches;
  uint32_t exact_match_entries;
};

struct PropertyInfoAreaHeader {
  // The current version of this data as created by property service.
  uint32_t current_version;
  // The lowest version of libc that can properly parse this data.
  uint32_t minimum_supported_version;
  uint32_t size;
  uint32_t contexts_offset;
  uint32_t types_offset;
  uint32_t root_offset;
};

class SerializedData {
 public:
  uint32_t size() const {
    return reinterpret_cast<const PropertyInfoAreaHeader*>(data_base_)->size;
  }

  const char* c_string(uint32_t offset) const {
    if (offset != 0 && offset > size()) return nullptr;
    return static_cast<const char*>(data_base_ + offset);
  }

  const uint32_t* uint32_array(uint32_t offset) const {
    if (offset != 0 && offset > size()) return nullptr;
    return reinterpret_cast<const uint32_t*>(data_base_ + offset);
  }

  uint32_t uint32(uint32_t offset) const {
    if (offset != 0 && offset > size()) return ~0u;
    return *reinterpret_cast<const uint32_t*>(data_base_ + offset);
  }

  const char* data_base() const { return data_base_; }

 private:
  const char data_base_[0];
};

class TrieNode {
 public:
  TrieNode() : serialized_data_(nullptr), trie_node_base_(nullptr) {}
  TrieNode(const SerializedData* data_base, const TrieNodeInternal* trie_node_base)
      : serialized_data_(data_base), trie_node_base_(trie_node_base) {}

  const char* name() const {
    return serialized_data_->c_string(node_property_entry()->name_offset);
  }

  uint32_t context_index() const { return node_property_entry()->context_index; }
  uint32_t type_index() const { return node_property_entry()->type_index; }

  uint32_t num_child_nodes() const { return trie_node_base_->num_child_nodes; }
  TrieNode child_node(int n) const {
    uint32_t child_node_offset = serialized_data_->uint32_array(trie_node_base_->child_nodes)[n];
    const TrieNodeInternal* trie_node_base =
        reinterpret_cast<const TrieNodeInternal*>(serialized_data_->data_base() + child_node_offset);
    return TrieNode(serialized_data_, trie_node_base);
  }

  bool FindChildForString(const char* input, uint32_t namelen, TrieNode* child) const;

  uint32_t num_prefixes() const { return trie_node_base_->num_prefixes; }
  const PropertyEntry* prefix(int n) const {
    uint32_t prefix_entry_offset =
        serialized_data_->uint32_array(trie_node_base_->prefix_entries)[n];
    return reinterpret_cast<const PropertyEntry*>(serialized_data_->data_base() +
                                                  prefix_entry_offset);
  }

  uint32_t num_exact_matches() const { return trie_node_base_->num_exact_matches; }
  const PropertyEntry* exact_match(int n) const {
    uint32_t exact_match_entry_offset =
        serialized_data_->uint32_array(trie_node_base_->exact_match_entries)[n];
    return reinterpret_cast<const PropertyEntry*>(serialized_data_->data_base() +
                                                  exact_match_entry_offset);
  }

 private:
  const PropertyEntry* node_property_entry() const {
    return reinterpret_cast<const PropertyEntry*>(serialized_data_->data_base() +
                                                  trie_node_base_->property_entry);
  }

  const SerializedData* serialized_data_;
  const TrieNodeInternal* trie_node_base_;
};

class PropertyInfoArea : private SerializedData {
 public:
  void GetPropertyInfoIndexes(const char* name, uint32_t* context_index, uint32_t* type_index) const;
  void GetPropertyInfo(const char* property, const char** context, const char** type) const;

  int FindContextIndex(const char* context) const;
  int FindTypeIndex(const char* type) const;

  const char* context(uint32_t index) const {
    uint32_t context_array_size_offset = contexts_offset();
    const uint32_t* context_array = uint32_array(context_array_size_offset + sizeof(uint32_t));
    return data_base() + context_array[index];
  }

  const char* type(uint32_t index) const {
    uint32_t type_array_size_offset = types_offset();
    const uint32_t* type_array = uint32_array(type_array_size_offset + sizeof(uint32_t));
    return data_base() + type_array[index];
  }

  uint32_t current_version() const { return header()->current_version; }
  uint32_t minimum_supported_version() const { return header()->minimum_supported_version; }

  uint32_t size() const { return SerializedData::size(); }

  uint32_t num_contexts() const { return uint32_array(contexts_offset())[0]; }
  uint32_t num_types() const { return uint32_array(types_offset())[0]; }

  TrieNode root_node() const { return trie(header()->root_offset); }

 private:
  void CheckPrefixMatch(const char* remaining_name, const TrieNode& trie_node,
                        uint32_t* context_index, uint32_t* type_index) const;

  const PropertyInfoAreaHeader* header() const {
    return reinterpret_cast<const PropertyInfoAreaHeader*>(data_base());
  }
  uint32_t contexts_offset() const { return header()->contexts_offset; }
  uint32_t contexts_array_offset() const { return contexts_offset() + sizeof(uint32_t); }
  uint32_t types_offset() const { return header()->types_offset; }
  uint32_t types_array_offset() const { return types_offset() + sizeof(uint32_t); }

  TrieNode trie(uint32_t offset) const {
    if (offset != 0 && offset > size()) return TrieNode();
    const TrieNodeInternal* trie_node_base =
        reinterpret_cast<const TrieNodeInternal*>(data_base() + offset);
    return TrieNode(this, trie_node_base);
  }
};

// This is essentially a smart pointer for read only mmap region for property contexts.
class PropertyInfoAreaFile {
 public:
  PropertyInfoAreaFile() : mmap_base_(nullptr), mmap_size_(0) {}
  ~PropertyInfoAreaFile() { Reset(); }

  PropertyInfoAreaFile(const PropertyInfoAreaFile&) = delete;
  void operator=(const PropertyInfoAreaFile&) = delete;
  PropertyInfoAreaFile(PropertyInfoAreaFile&&) = default;
  PropertyInfoAreaFile& operator=(PropertyInfoAreaFile&&) = default;

  bool LoadDefaultPath();
  bool LoadPath(const char* filename);

  const PropertyInfoArea* operator->() const {
    return reinterpret_cast<const PropertyInfoArea*>(mmap_base_);
  }

  explicit operator bool() const { return mmap_base_ != nullptr; }

  void Reset();

 private:
  void* mmap_base_;
  size_t mmap_size_;
};

}  // namespace properties
}  // namespace android

#endif
