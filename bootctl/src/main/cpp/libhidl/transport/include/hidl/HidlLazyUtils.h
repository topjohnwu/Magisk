/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <functional>

#include <android/hidl/base/1.0/IBase.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>

namespace android {
namespace hardware {
namespace details {
class LazyServiceRegistrarImpl;
}  // namespace details

/**
 * Exits when all HALs registered through this object have 0 clients
 *
 * In order to use this class, it's expected that your service:
 * - registers all services in the process with this API
 * - configures services as oneshot + disabled in init .rc files
 * - uses 'interface' declarations in init .rc files
 *
 * For more information on init .rc configuration, see system/core/init/README.md
 **/
class LazyServiceRegistrar {
   public:
     static LazyServiceRegistrar& getInstance();
     status_t registerService(const sp<::android::hidl::base::V1_0::IBase>& service,
                              const std::string& name = "default");
     /**
      * Set a callback that is invoked when the active HAL count (i.e. HALs with clients)
      * registered with this process drops to zero (or becomes nonzero).
      * The callback takes a boolean argument, which is 'true' if there is
      * at least one HAL with clients.
      *
      * Callback return value:
      * - false: Default behavior for lazy HALs (shut down the process if there
      *          are no clients).
      * - true:  Don't shut down the process even if there are no clients.
      *
      * This callback gives a chance to:
      * 1 - Perform some additional operations before exiting;
      * 2 - Prevent the process from exiting by returning "true" from the
      *     callback.
      *
      * This method should be called before 'registerService' to avoid races.
      */
     void setActiveServicesCallback(const std::function<bool(bool)>& activeServicesCallback);

     /**
      * Try to unregister all services previously registered with 'registerService'.
      * Returns 'true' if successful. This should only be called within
      * the callback registered by setActiveServicesCallback.
      */
     bool tryUnregister();

     /**
      * Re-register services that were unregistered by 'tryUnregister'.
      * This method should be called in the case 'tryUnregister' fails
      * (and should be called on the same thread).
      */
     void reRegister();

   private:
     std::shared_ptr<details::LazyServiceRegistrarImpl> mImpl;
     LazyServiceRegistrar();
};

}  // namespace hardware
}  // namespace android
