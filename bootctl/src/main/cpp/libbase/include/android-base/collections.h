/*
 * Copyright (C) 2019 The Android Open Source Project
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

#pragma once

#include <utility>

namespace android {
namespace base {

// Helpers for converting a variadic template parameter pack to a homogeneous collection.
// Parameters must be implictly convertible to the contained type (including via move/copy ctors).
//
// Use as follows:
//
//   template <typename... Args>
//   std::vector<int> CreateVector(Args&&... args) {
//     std::vector<int> result;
//     Append(result, std::forward<Args>(args)...);
//     return result;
//   }
template <typename CollectionType, typename T>
void Append(CollectionType& collection, T&& arg) {
  collection.push_back(std::forward<T>(arg));
}

template <typename CollectionType, typename T, typename... Args>
void Append(CollectionType& collection, T&& arg, Args&&... args) {
  collection.push_back(std::forward<T>(arg));
  return Append(collection, std::forward<Args>(args)...);
}

// Assert that all of the arguments in a variadic template parameter pack are of a given type
// after std::decay.
template <typename T, typename Arg, typename... Args>
void AssertType(Arg&&) {
  static_assert(std::is_same<T, typename std::decay<Arg>::type>::value);
}

template <typename T, typename Arg, typename... Args>
void AssertType(Arg&&, Args&&... args) {
  static_assert(std::is_same<T, typename std::decay<Arg>::type>::value);
  AssertType<T>(std::forward<Args>(args)...);
}

}  // namespace base
}  // namespace android
