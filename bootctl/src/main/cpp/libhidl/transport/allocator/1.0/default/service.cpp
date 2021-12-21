#define LOG_TAG "android.hidl.allocator@1.0-service"

#include "AshmemAllocator.h"

#include <android-base/logging.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <hidl/HidlTransportSupport.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hidl::allocator::V1_0::IAllocator;
using android::hidl::allocator::V1_0::implementation::AshmemAllocator;
using android::sp;
using android::status_t;

int main() {
    configureRpcThreadpool(1, true /* callerWillJoin */);

    sp<IAllocator> allocator = new AshmemAllocator();

    status_t status = allocator->registerAsService("ashmem");

    if (android::OK != status) {
        LOG(FATAL) << "Unable to register allocator service: " << status;
    }

    joinRpcThreadpool();

    return -1;
}
