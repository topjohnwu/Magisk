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

#define LOG_TAG "HidlLazyUtils"

#include <hidl/HidlLazyUtils.h>
#include <hidl/HidlTransportSupport.h>

#include <android-base/logging.h>

#include <android/hidl/manager/1.2/IClientCallback.h>
#include <android/hidl/manager/1.2/IServiceManager.h>

namespace android {
namespace hardware {
namespace details {

using ::android::hidl::base::V1_0::IBase;

class ClientCounterCallback : public ::android::hidl::manager::V1_2::IClientCallback {
  public:
    ClientCounterCallback() {}

    bool addRegisteredService(const sp<IBase>& service, const std::string& name);

    bool tryUnregisterLocked();

    void reRegisterLocked();

    void setActiveServicesCallback(const std::function<bool(bool)>& activeServicesCallback);

  protected:
    Return<void> onClients(const sp<IBase>& service, bool clients) override;

  private:
    struct Service {
        sp<IBase> service;
        std::string name;
        bool clients = false;
        // Used to keep track of unregistered services to allow re-registry
        bool registered = true;
    };

    /**
     * Looks up service that is guaranteed to be registered (service from
     * onClients).
     */
    Service& assertRegisteredServiceLocked(const sp<IBase>& service);

    /**
     * Registers or re-registers services. Returns whether successful.
     */
    bool registerServiceLocked(const sp<IBase>& service, const std::string& name);

    /**
     * Unregisters all services that we can. If we can't unregister all, re-register other
     * services.
     */
    void tryShutdownLocked();

    /**
     * For below.
     */
    std::mutex mMutex;

    /**
     * Number of services that have been registered.
     */
    std::vector<Service> mRegisteredServices;

    /**
     * Callback for reporting the number of services with clients.
     */
    std::function<bool(bool)> mActiveServicesCallback;

    /**
     * Previous value passed to the active services callback.
     */
    std::optional<bool> mPreviousHasClients;
};

class LazyServiceRegistrarImpl {
  public:
    LazyServiceRegistrarImpl() : mClientCallback(new ClientCounterCallback) {}

    status_t registerService(const sp<::android::hidl::base::V1_0::IBase>& service,
                             const std::string& name);
    bool tryUnregister();
    void reRegister();
    void setActiveServicesCallback(const std::function<bool(bool)>& activeServicesCallback);

  private:
    sp<ClientCounterCallback> mClientCallback;
};

bool ClientCounterCallback::addRegisteredService(const sp<IBase>& service,
                                                 const std::string& name) {
    std::lock_guard<std::mutex> lock(mMutex);
    bool success = registerServiceLocked(service, name);

    if (success) {
        mRegisteredServices.push_back({service, name});
    }

    return success;
}

ClientCounterCallback::Service& ClientCounterCallback::assertRegisteredServiceLocked(
        const sp<IBase>& service) {
    for (Service& registered : mRegisteredServices) {
        if (registered.service != service) continue;
        return registered;
    }
    LOG(FATAL) << "Got callback on service " << getDescriptor(service.get())
               << " which we did not register.";
    __builtin_unreachable();
}

bool ClientCounterCallback::registerServiceLocked(const sp<IBase>& service,
                                                  const std::string& name) {
    auto manager = hardware::defaultServiceManager1_2();

    const std::string descriptor = getDescriptor(service.get());

    LOG(INFO) << "Registering HAL: " << descriptor << " with name: " << name;

    status_t res = android::hardware::details::registerAsServiceInternal(service, name);
    if (res != android::OK) {
        LOG(ERROR) << "Failed to register as service.";
        return false;
    }

    bool ret = manager->registerClientCallback(getDescriptor(service.get()), name, service, this);
    if (!ret) {
        LOG(ERROR) << "Failed to add client callback.";
        return false;
    }

    return true;
}

Return<void> ClientCounterCallback::onClients(const sp<::android::hidl::base::V1_0::IBase>& service,
                                              bool clients) {
    std::lock_guard<std::mutex> lock(mMutex);
    Service& registered = assertRegisteredServiceLocked(service);
    if (registered.clients == clients) {
        LOG(FATAL) << "Process already thought " << getDescriptor(service.get()) << "/"
                   << registered.name << " had clients: " << registered.clients
                   << " but hwservicemanager has notified has clients: " << clients;
    }
    registered.clients = clients;

    size_t numWithClients = 0;
    for (const Service& registered : mRegisteredServices) {
        if (registered.clients) numWithClients++;
    }

    LOG(INFO) << "Process has " << numWithClients << " (of " << mRegisteredServices.size()
              << " available) client(s) in use after notification " << getDescriptor(service.get())
              << "/" << registered.name << " has clients: " << clients;

    bool handledInCallback = false;
    if (mActiveServicesCallback != nullptr) {
        bool hasClients = numWithClients != 0;
        if (hasClients != mPreviousHasClients) {
            handledInCallback = mActiveServicesCallback(hasClients);
            mPreviousHasClients = hasClients;
        }
    }

    // If there is no callback defined or the callback did not handle this
    // client count change event, try to shutdown the process if its services
    // have no clients.
    if (!handledInCallback && numWithClients == 0) {
        tryShutdownLocked();
    }

    return Status::ok();
}

bool ClientCounterCallback::tryUnregisterLocked() {
    auto manager = hardware::defaultServiceManager1_2();

    for (Service& entry : mRegisteredServices) {
        const std::string descriptor = getDescriptor(entry.service.get());
        bool success = manager->tryUnregister(descriptor, entry.name, entry.service);

        if (!success) {
            LOG(INFO) << "Failed to unregister HAL " << descriptor << "/" << entry.name;
            return false;
        }

        // Mark the entry unregistered, but do not remove it (may still be re-registered)
        entry.registered = false;
    }

    return true;
}

void ClientCounterCallback::reRegisterLocked() {
    for (Service& entry : mRegisteredServices) {
        // re-register entry if not already registered
        if (entry.registered) {
            continue;
        }

        if (!registerServiceLocked(entry.service, entry.name)) {
            // Must restart. Otherwise, clients will never be able to get ahold of this service.
            LOG(FATAL) << "Bad state: could not re-register " << getDescriptor(entry.service.get());
        }

        entry.registered = true;
    }
}

void ClientCounterCallback::tryShutdownLocked() {
    LOG(INFO) << "Trying to exit HAL. No clients in use for any service in process.";

    if (tryUnregisterLocked()) {
        LOG(INFO) << "Unregistered all clients and exiting";
        exit(EXIT_SUCCESS);
    }

    // At this point, we failed to unregister some of the services, leaving the
    // server in an inconsistent state. Re-register all services that were
    // unregistered by tryUnregisterLocked().
    reRegisterLocked();
}

void ClientCounterCallback::setActiveServicesCallback(
        const std::function<bool(bool)>& activeServicesCallback) {
    std::lock_guard<std::mutex> lock(mMutex);

    mActiveServicesCallback = activeServicesCallback;
}

status_t LazyServiceRegistrarImpl::registerService(
    const sp<::android::hidl::base::V1_0::IBase>& service, const std::string& name) {
    if (!mClientCallback->addRegisteredService(service, name)) {
        return ::android::UNKNOWN_ERROR;
    }

    return ::android::OK;
}

bool LazyServiceRegistrarImpl::tryUnregister() {
    // see comments in header, this should only be called from the active
    // services callback, see also b/191781736
    return mClientCallback->tryUnregisterLocked();
}

void LazyServiceRegistrarImpl::reRegister() {
    // see comments in header, this should only be called from the active
    // services callback, see also b/191781736
    mClientCallback->reRegisterLocked();
}

void LazyServiceRegistrarImpl::setActiveServicesCallback(
        const std::function<bool(bool)>& activeServicesCallback) {
    mClientCallback->setActiveServicesCallback(activeServicesCallback);
}

}  // namespace details

LazyServiceRegistrar::LazyServiceRegistrar() {
    mImpl = std::make_shared<details::LazyServiceRegistrarImpl>();
}

LazyServiceRegistrar& LazyServiceRegistrar::getInstance() {
    static auto registrarInstance = new LazyServiceRegistrar();
    return *registrarInstance;
}

status_t LazyServiceRegistrar::registerService(
    const sp<::android::hidl::base::V1_0::IBase>& service, const std::string& name) {
    return mImpl->registerService(service, name);
}

bool LazyServiceRegistrar::tryUnregister() {
    return mImpl->tryUnregister();
}

void LazyServiceRegistrar::reRegister() {
    mImpl->reRegister();
}

void LazyServiceRegistrar::setActiveServicesCallback(
        const std::function<bool(bool)>& activeServicesCallback) {
    mImpl->setActiveServicesCallback(activeServicesCallback);
}

}  // namespace hardware
}  // namespace android
