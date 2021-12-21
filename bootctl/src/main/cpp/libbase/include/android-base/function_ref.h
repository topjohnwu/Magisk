/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <stdint.h>
#include <type_traits>
#include <utility>

namespace android::base {

//
// function_ref<> - a class that stores a reference to a callable object,
//      similar to string_view for strings.
//
// We need to pass around lots of callbacks. The standard way of doing it
// is via std::function<> class, and it usually works OK. But there are some
// noticeable drawbacks:
//
//  1. std::function<> in most STLs performs heap allocation for all callables
//     bigger than a single poiner to a function.
//  2. std::function<> goes through at least two pointers + a vptr call to call
//     the stored function.
//  3. std::function<> copies the passed object inside at least once; this also
//     means it can't work with non-copyable functors.
//
// function_ref is an alternative way of passing functors around. Instead of
// storing a copy of the functor inside, it follows the path of string_view and
// merely captures a pointer to the object to call. This allows for a simple,
// fast and lightweight wrapper design; it also dictates the limitations:
//
//  1. function_ref<> stores a pointer to outside functor. That functor _must_
//     outlive the ref.
//  2. function_ref<> has two calls through a function pointer in its call
//     operator. That's still better than std::function<>, but slower compared
//     to a raw function pointer call with a "void* opaque" context parameter.
//
// Limitation #1 dictates the best use case: a function parameter type for some
// generic callback which doesn't get stored inside an object field but only
// gets called in this call. E.g.:
//
//  void someLongOperation(function_ref<void(int progress)> onProgress) {
//      firstStep(onProgress);
//      ...
//      onProgress(50);
//      ...
//      lastStep(onProgress);
//      onProgress(100);
//  }
//
// In this code std::function<> is an overkill as the whole use of |onProgresss|
// callback is scoped and easy to track. An alternative design - making it a
// template parameter (template <class Callback> ... (Callback onProgress))
// forces one to put someLongOperation() + some private functions into the
// header. function_ref<> is the choice then.
//
// NOTE: Beware of passing temporary functions via function_ref<>! Temporaries
//  live until the end of full expression (usually till the next semicolon), and
//  having a function_ref<> that refers to a dangling pointer is a bug that's
//  hard to debug. E.g.:
//      function_ref<...> v = [](){};                     // this is fine
//      function_ref<...> v = std::function<...>([](){}); // this will kill you
//
// NOTE2: function_ref<> should not have an empty state, but it doesn't have a
//  runtime check against that. Don't construct it from a null function!

template <class Signature>
class function_ref;

template <class Ret, class... Args>
class function_ref<Ret(Args...)> final {
 public:
  constexpr function_ref() noexcept = delete;
  constexpr function_ref(const function_ref& other) noexcept = default;
  constexpr function_ref& operator=(const function_ref&) noexcept = default;

  template <class Callable, class = std::enable_if_t<
                                std::is_invocable_r<Ret, Callable, Args...>::value &&
                                !std::is_same_v<function_ref, std::remove_reference_t<Callable>>>>
  function_ref(Callable&& c) noexcept
      : mTypeErasedFunction([](const function_ref* self, Args... args) -> Ret {
          // Generate a lambda that remembers the type of the passed
          // |Callable|.
          return (*reinterpret_cast<std::remove_reference_t<Callable>*>(self->mCallable))(
              std::forward<Args>(args)...);
        }),
        mCallable(reinterpret_cast<intptr_t>(&c)) {}

  template <class Callable, class = std::enable_if_t<
                                std::is_invocable_r<Ret, Callable, Args...>::value &&
                                !std::is_same_v<function_ref, std::remove_reference_t<Callable>>>>
  function_ref& operator=(Callable&& c) noexcept {
    mTypeErasedFunction = [](const function_ref* self, Args... args) -> Ret {
      // Generate a lambda that remembers the type of the passed
      // |Callable|.
      return (*reinterpret_cast<std::remove_reference_t<Callable>*>(self->mCallable))(
          std::forward<Args>(args)...);
    };
    mCallable = reinterpret_cast<intptr_t>(&c);
    return *this;
  }

  Ret operator()(Args... args) const {
    return mTypeErasedFunction(this, std::forward<Args>(args)...);
  }

 private:
  using TypeErasedFunc = Ret(const function_ref*, Args...);
  TypeErasedFunc* mTypeErasedFunction;
  intptr_t mCallable;
};

}  // namespace android::base
