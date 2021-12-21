/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "HybridInterface"

#include <utils/Log.h>
#include <hidl/HybridInterface.h>
#include <hidl/HidlSupport.h>
#include <android/hidl/token/1.0/ITokenManager.h>

namespace android {

using ::android::hidl::token::V1_0::ITokenManager;

namespace {

std::mutex gTokenManagerLock;
sp<ITokenManager> gTokenManager = nullptr;

struct TokenManagerDeathRecipient : public hardware::hidl_death_recipient {
    void serviceDied(uint64_t, const wp<HInterface>&) {
        std::lock_guard<std::mutex> lock(gTokenManagerLock);
        gTokenManager = nullptr;
    }
};

sp<TokenManagerDeathRecipient> gTokenManagerDeathRecipient =
    new TokenManagerDeathRecipient();

bool isBadTokenManager() {
    if (gTokenManager != nullptr) {
        return false;
    }
    gTokenManager = ITokenManager::getService();
    if (gTokenManager == nullptr) {
        ALOGE("Cannot retrieve TokenManager.");
        return true;
    }
    auto transaction = gTokenManager->linkToDeath(
            gTokenManagerDeathRecipient, 0);
    if (!transaction.isOk()) {
        ALOGE("Cannot observe TokenManager's death.");
        gTokenManager = nullptr;
        return true;
    }
    return false;
}

template <typename ReturnType>
bool isBadTransaction(hardware::Return<ReturnType>& transaction) {
    if (transaction.isOk()) {
        return false;
    }
    ALOGE("TokenManager's transaction error: %s",
            transaction.description().c_str());
    gTokenManager->unlinkToDeath(gTokenManagerDeathRecipient).isOk();
    gTokenManager = nullptr;
    return true;
}

} // unnamed namespace

sp<HInterface> retrieveHalInterface(const HalToken& token) {
    hardware::Return<sp<HInterface> > transaction(nullptr);
    {
        std::lock_guard<std::mutex> lock(gTokenManagerLock);
        if (isBadTokenManager()) {
            return nullptr;
        }
        transaction = gTokenManager->get(token);
        if (isBadTransaction(transaction)) {
            return nullptr;
        }
    }
    return static_cast<sp<HInterface> >(transaction);
}

bool createHalToken(const sp<HInterface>& interface, HalToken* token) {
    hardware::Return<void> transaction;
    {
        std::lock_guard<std::mutex> lock(gTokenManagerLock);
        if (isBadTokenManager()) {
            return false;
        }
        transaction = gTokenManager->createToken(interface, [&](const HalToken &newToken) {
            *token = newToken;
        });
    }
    return !isBadTransaction(transaction);
}

bool deleteHalToken(const HalToken& token) {
    hardware::Return<bool> transaction(false);
    {
        std::lock_guard<std::mutex> lock(gTokenManagerLock);
        if (isBadTokenManager()) {
            return false;
        }
        transaction = gTokenManager->unregister(token);
        if (isBadTransaction(transaction)) {
            return false;
        }
    }
    return static_cast<bool>(transaction);
}

}  // namespace android
