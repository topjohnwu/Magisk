/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "HidlServiceManagement"

#ifdef __ANDROID__
#include <android/dlext.h>
#endif  // __ANDROID__

#include <condition_variable>
#include <dlfcn.h>
#include <dirent.h>
#include <fstream>
#include <pthread.h>
#include <unistd.h>

#include <mutex>
#include <regex>
#include <set>

#include <hidl/HidlBinderSupport.h>
#include <hidl/HidlInternal.h>
#include <hidl/HidlTransportUtils.h>
#include <hidl/ServiceManagement.h>
#include <hidl/Status.h>
#include <utils/SystemClock.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/Parcel.h>
#if !defined(__ANDROID_RECOVERY__) && defined(__ANDROID__)
#include <vndksupport/linker.h>
#endif

#include <android/hidl/manager/1.2/BnHwServiceManager.h>
#include <android/hidl/manager/1.2/BpHwServiceManager.h>
#include <android/hidl/manager/1.2/IServiceManager.h>

using ::android::hidl::base::V1_0::IBase;
using IServiceManager1_0 = android::hidl::manager::V1_0::IServiceManager;
using IServiceManager1_1 = android::hidl::manager::V1_1::IServiceManager;
using IServiceManager1_2 = android::hidl::manager::V1_2::IServiceManager;
using ::android::hidl::manager::V1_0::IServiceNotification;

namespace android {
namespace hardware {

#if defined(__ANDROID_RECOVERY__)
static constexpr bool kIsRecovery = true;
#else
static constexpr bool kIsRecovery = false;
#endif

static void waitForHwServiceManager() {
    // TODO(b/31559095): need bionic host so that we can use 'prop_info' returned
    // from WaitForProperty
#ifdef __ANDROID__
    static const char* kHwServicemanagerReadyProperty = "hwservicemanager.ready";

    using std::literals::chrono_literals::operator""s;

    using android::base::WaitForProperty;
    while (!WaitForProperty(kHwServicemanagerReadyProperty, "true", 1s)) {
        LOG(WARNING) << "Waited for hwservicemanager.ready for a second, waiting another...";
    }
#endif  // __ANDROID__
}

static std::string binaryName() {
    std::ifstream ifs("/proc/self/cmdline");
    std::string cmdline;
    if (!ifs) {
        return "";
    }
    ifs >> cmdline;

    size_t idx = cmdline.rfind('/');
    if (idx != std::string::npos) {
        cmdline = cmdline.substr(idx + 1);
    }

    return cmdline;
}

static std::string packageWithoutVersion(const std::string& packageAndVersion) {
    size_t at = packageAndVersion.find('@');
    if (at == std::string::npos) return packageAndVersion;
    return packageAndVersion.substr(0, at);
}

__attribute__((noinline)) static void tryShortenProcessName(const std::string& descriptor) {
    const static std::string kTasks = "/proc/self/task/";

    // make sure that this binary name is in the same package
    std::string processName = binaryName();

    // e.x. android.hardware.foo is this package
    if (!base::StartsWith(packageWithoutVersion(processName), packageWithoutVersion(descriptor))) {
        return;
    }

    // e.x. android.hardware.module.foo@1.2::IFoo -> foo@1.2
    size_t lastDot = descriptor.rfind('.');
    if (lastDot == std::string::npos) return;
    size_t secondDot = descriptor.rfind('.', lastDot - 1);
    if (secondDot == std::string::npos) return;

    std::string newName = processName.substr(secondDot + 1, std::string::npos);
    ALOGI("Removing namespace from process name %s to %s.", processName.c_str(), newName.c_str());

    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(kTasks.c_str()), closedir);
    if (dir == nullptr) return;

    dirent* dp;
    while ((dp = readdir(dir.get())) != nullptr) {
        if (dp->d_type != DT_DIR) continue;
        if (dp->d_name[0] == '.') continue;

        std::fstream fs(kTasks + dp->d_name + "/comm");
        if (!fs) {
            ALOGI("Could not rename process, failed read comm for %s.", dp->d_name);
            continue;
        }

        std::string oldComm;
        if (!(fs >> oldComm)) continue;

        // don't rename if it already has an explicit name
        if (base::StartsWith(descriptor, oldComm)) {
            if (!fs.seekg(0, fs.beg)) continue;
            fs << newName;
        }
    }
}

namespace details {

#ifdef ENFORCE_VINTF_MANIFEST
static constexpr bool kEnforceVintfManifest = true;
#else
static constexpr bool kEnforceVintfManifest = false;
#endif

static bool* getTrebleTestingOverridePtr() {
    static bool gTrebleTestingOverride = false;
    return &gTrebleTestingOverride;
}

void setTrebleTestingOverride(bool testingOverride) {
    *getTrebleTestingOverridePtr() = testingOverride;
}

static bool isDebuggable() {
    static bool debuggable = base::GetBoolProperty("ro.debuggable", false);
    return debuggable;
}

static inline bool isTrebleTestingOverride() {
    if (kEnforceVintfManifest && !isDebuggable()) {
        // don't allow testing override in production
        return false;
    }

    return *getTrebleTestingOverridePtr();
}

static void onRegistrationImpl(const std::string& descriptor, const std::string& instanceName) {
    LOG(INFO) << "Registered " << descriptor << "/" << instanceName;
    tryShortenProcessName(descriptor);
}

// only used by prebuilts - should be able to remove
void onRegistration(const std::string& packageName, const std::string& interfaceName,
                    const std::string& instanceName) {
    return onRegistrationImpl(packageName + "::" + interfaceName, instanceName);
}

}  // details

sp<IServiceManager1_0> defaultServiceManager() {
    return defaultServiceManager1_2();
}
sp<IServiceManager1_1> defaultServiceManager1_1() {
    return defaultServiceManager1_2();
}
sp<IServiceManager1_2> defaultServiceManager1_2() {
    using android::hidl::manager::V1_2::BnHwServiceManager;
    using android::hidl::manager::V1_2::BpHwServiceManager;

    static std::mutex& gDefaultServiceManagerLock = *new std::mutex;
    static sp<IServiceManager1_2>& gDefaultServiceManager = *new sp<IServiceManager1_2>;

    {
        std::lock_guard<std::mutex> _l(gDefaultServiceManagerLock);
        if (gDefaultServiceManager != nullptr) {
            return gDefaultServiceManager;
        }

        if (access("/dev/hwbinder", F_OK|R_OK|W_OK) != 0) {
            // HwBinder not available on this device or not accessible to
            // this process.
            return nullptr;
        }

        waitForHwServiceManager();

        while (gDefaultServiceManager == nullptr) {
            gDefaultServiceManager =
                fromBinder<IServiceManager1_2, BpHwServiceManager, BnHwServiceManager>(
                    ProcessState::self()->getContextObject(nullptr));
            if (gDefaultServiceManager == nullptr) {
                LOG(ERROR) << "Waited for hwservicemanager, but got nullptr.";
                sleep(1);
            }
        }
    }

    return gDefaultServiceManager;
}

static std::vector<std::string> findFiles(const std::string& path, const std::string& prefix,
                                          const std::string& suffix) {
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path.c_str()), closedir);
    if (!dir) return {};

    std::vector<std::string> results{};

    dirent* dp;
    while ((dp = readdir(dir.get())) != nullptr) {
        std::string name = dp->d_name;

        if (base::StartsWith(name, prefix) && base::EndsWith(name, suffix)) {
            results.push_back(name);
        }
    }

    return results;
}

static bool matchPackageName(const std::string& lib, std::string* matchedName,
                             std::string* implName) {
#define RE_COMPONENT "[a-zA-Z_][a-zA-Z_0-9]*"
#define RE_PATH RE_COMPONENT "(?:[.]" RE_COMPONENT ")*"
    static const std::regex gLibraryFileNamePattern("(" RE_PATH "@[0-9]+[.][0-9]+)-impl(.*?).so");
#undef RE_PATH
#undef RE_COMPONENT

    std::smatch match;
    if (std::regex_match(lib, match, gLibraryFileNamePattern)) {
        *matchedName = match.str(1) + "::I*";
        *implName = match.str(2);
        return true;
    }
    return false;
}

static void registerReference(const hidl_string &interfaceName, const hidl_string &instanceName) {
    if (kIsRecovery) {
        // No hwservicemanager in recovery.
        return;
    }

    sp<IServiceManager1_0> binderizedManager = defaultServiceManager();
    if (binderizedManager == nullptr) {
        LOG(WARNING) << "Could not registerReference for "
                     << interfaceName << "/" << instanceName
                     << ": null binderized manager.";
        return;
    }
    auto ret = binderizedManager->registerPassthroughClient(interfaceName, instanceName);
    if (!ret.isOk()) {
        LOG(WARNING) << "Could not registerReference for "
                     << interfaceName << "/" << instanceName
                     << ": " << ret.description();
        return;
    }
    LOG(VERBOSE) << "Successfully registerReference for "
                 << interfaceName << "/" << instanceName;
}

using InstanceDebugInfo = hidl::manager::V1_0::IServiceManager::InstanceDebugInfo;
static inline void fetchPidsForPassthroughLibraries(
    std::map<std::string, InstanceDebugInfo>* infos) {
    static const std::string proc = "/proc/";

    std::map<std::string, std::set<pid_t>> pids;
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(proc.c_str()), closedir);
    if (!dir) return;
    dirent* dp;
    while ((dp = readdir(dir.get())) != nullptr) {
        pid_t pid = strtoll(dp->d_name, nullptr, 0);
        if (pid == 0) continue;
        std::string mapsPath = proc + dp->d_name + "/maps";
        std::ifstream ifs{mapsPath};
        if (!ifs.is_open()) continue;

        for (std::string line; std::getline(ifs, line);) {
            // The last token of line should look like
            // vendor/lib64/hw/android.hardware.foo@1.0-impl-extra.so
            // Use some simple filters to ignore bad lines before extracting libFileName
            // and checking the key in info to make parsing faster.
            if (line.back() != 'o') continue;
            if (line.rfind('@') == std::string::npos) continue;

            auto spacePos = line.rfind(' ');
            if (spacePos == std::string::npos) continue;
            auto libFileName = line.substr(spacePos + 1);
            auto it = infos->find(libFileName);
            if (it == infos->end()) continue;
            pids[libFileName].insert(pid);
        }
    }
    for (auto& pair : *infos) {
        pair.second.clientPids =
            std::vector<pid_t>{pids[pair.first].begin(), pids[pair.first].end()};
    }
}

struct PassthroughServiceManager : IServiceManager1_1 {
    static void openLibs(
        const std::string& fqName,
        const std::function<bool /* continue */ (void* /* handle */, const std::string& /* lib */,
                                                 const std::string& /* sym */)>& eachLib) {
        //fqName looks like android.hardware.foo@1.0::IFoo
        size_t idx = fqName.find("::");

        if (idx == std::string::npos ||
                idx + strlen("::") + 1 >= fqName.size()) {
            LOG(ERROR) << "Invalid interface name passthrough lookup: " << fqName;
            return;
        }

        std::string packageAndVersion = fqName.substr(0, idx);
        std::string ifaceName = fqName.substr(idx + strlen("::"));

        const std::string prefix = packageAndVersion + "-impl";
        const std::string sym = "HIDL_FETCH_" + ifaceName;

        constexpr int dlMode = RTLD_LAZY;
        void* handle = nullptr;

        dlerror(); // clear

        static std::string halLibPathVndkSp = details::getVndkSpHwPath();
        std::vector<std::string> paths = {
            HAL_LIBRARY_PATH_ODM, HAL_LIBRARY_PATH_VENDOR, halLibPathVndkSp,
#ifndef __ANDROID_VNDK__
            HAL_LIBRARY_PATH_SYSTEM,
#endif
        };

        if (details::isTrebleTestingOverride()) {
            // Load HAL implementations that are statically linked
            handle = dlopen(nullptr, dlMode);
            if (handle == nullptr) {
                const char* error = dlerror();
                LOG(ERROR) << "Failed to dlopen self: "
                           << (error == nullptr ? "unknown error" : error);
            } else if (!eachLib(handle, "SELF", sym)) {
                return;
            }
        }

        for (const std::string& path : paths) {
            std::vector<std::string> libs = findFiles(path, prefix, ".so");

            for (const std::string &lib : libs) {
                const std::string fullPath = path + lib;

                if (kIsRecovery || path == HAL_LIBRARY_PATH_SYSTEM) {
                    handle = dlopen(fullPath.c_str(), dlMode);
                } else {
#if !defined(__ANDROID_RECOVERY__) && defined(__ANDROID__)
                    handle = android_load_sphal_library(fullPath.c_str(), dlMode);
#endif
                }

                if (handle == nullptr) {
                    const char* error = dlerror();
                    LOG(ERROR) << "Failed to dlopen " << lib << ": "
                               << (error == nullptr ? "unknown error" : error);
                    continue;
                }

                if (!eachLib(handle, lib, sym)) {
                    return;
                }
            }
        }
    }

    Return<sp<IBase>> get(const hidl_string& fqName,
                          const hidl_string& name) override {
        sp<IBase> ret = nullptr;

        openLibs(fqName, [&](void* handle, const std::string &lib, const std::string &sym) {
            IBase* (*generator)(const char* name);
            *(void **)(&generator) = dlsym(handle, sym.c_str());
            if(!generator) {
                const char* error = dlerror();
                LOG(ERROR) << "Passthrough lookup opened " << lib << " but could not find symbol "
                           << sym << ": " << (error == nullptr ? "unknown error" : error)
                           << ". Keeping library open.";

                // dlclose too problematic in multi-threaded environment
                // dlclose(handle);

                return true;  // continue
            }

            ret = (*generator)(name.c_str());

            if (ret == nullptr) {
                LOG(ERROR) << "Could not find instance '" << name.c_str() << "' in library " << lib
                           << ". Keeping library open.";

                // dlclose too problematic in multi-threaded environment
                // dlclose(handle);

                // this module doesn't provide this particular instance
                return true;  // continue
            }

            // Actual fqname might be a subclass.
            // This assumption is tested in vts_treble_vintf_test
            using ::android::hardware::details::getDescriptor;
            std::string actualFqName = getDescriptor(ret.get());
            CHECK(actualFqName.size() > 0);
            registerReference(actualFqName, name);
            return false;
        });

        return ret;
    }

    Return<bool> add(const hidl_string& /* name */,
                     const sp<IBase>& /* service */) override {
        LOG(FATAL) << "Cannot register services with passthrough service manager.";
        return false;
    }

    Return<Transport> getTransport(const hidl_string& /* fqName */,
                                   const hidl_string& /* name */) {
        LOG(FATAL) << "Cannot getTransport with passthrough service manager.";
        return Transport::EMPTY;
    }

    Return<void> list(list_cb /* _hidl_cb */) override {
        LOG(FATAL) << "Cannot list services with passthrough service manager.";
        return Void();
    }
    Return<void> listByInterface(const hidl_string& /* fqInstanceName */,
                                 listByInterface_cb /* _hidl_cb */) override {
        // TODO: add this functionality
        LOG(FATAL) << "Cannot list services with passthrough service manager.";
        return Void();
    }

    Return<bool> registerForNotifications(const hidl_string& /* fqName */,
                                          const hidl_string& /* name */,
                                          const sp<IServiceNotification>& /* callback */) override {
        // This makes no sense.
        LOG(FATAL) << "Cannot register for notifications with passthrough service manager.";
        return false;
    }

    Return<void> debugDump(debugDump_cb _hidl_cb) override {
        using Arch = ::android::hidl::base::V1_0::DebugInfo::Architecture;
        using std::literals::string_literals::operator""s;
        static std::string halLibPathVndkSp64 = details::getVndkSpHwPath("lib64");
        static std::string halLibPathVndkSp32 = details::getVndkSpHwPath("lib");
        static std::vector<std::pair<Arch, std::vector<const char*>>> sAllPaths{
            {Arch::IS_64BIT,
             {
                 HAL_LIBRARY_PATH_ODM_64BIT, HAL_LIBRARY_PATH_VENDOR_64BIT,
                 halLibPathVndkSp64.c_str(),
#ifndef __ANDROID_VNDK__
                 HAL_LIBRARY_PATH_SYSTEM_64BIT,
#endif
             }},
            {Arch::IS_32BIT,
             {
                 HAL_LIBRARY_PATH_ODM_32BIT, HAL_LIBRARY_PATH_VENDOR_32BIT,
                 halLibPathVndkSp32.c_str(),
#ifndef __ANDROID_VNDK__
                 HAL_LIBRARY_PATH_SYSTEM_32BIT,
#endif
             }}};
        std::map<std::string, InstanceDebugInfo> map;
        for (const auto &pair : sAllPaths) {
            Arch arch = pair.first;
            for (const auto &path : pair.second) {
                std::vector<std::string> libs = findFiles(path, "", ".so");
                for (const std::string &lib : libs) {
                    std::string matchedName;
                    std::string implName;
                    if (matchPackageName(lib, &matchedName, &implName)) {
                        std::string instanceName{"* ("s + path + ")"s};
                        if (!implName.empty()) instanceName += " ("s + implName + ")"s;
                        map.emplace(path + lib, InstanceDebugInfo{.interfaceName = matchedName,
                                                                  .instanceName = instanceName,
                                                                  .clientPids = {},
                                                                  .arch = arch});
                    }
                }
            }
        }
        fetchPidsForPassthroughLibraries(&map);
        hidl_vec<InstanceDebugInfo> vec;
        vec.resize(map.size());
        size_t idx = 0;
        for (auto&& pair : map) {
            vec[idx++] = std::move(pair.second);
        }
        _hidl_cb(vec);
        return Void();
    }

    Return<void> registerPassthroughClient(const hidl_string &, const hidl_string &) override {
        // This makes no sense.
        LOG(FATAL) << "Cannot call registerPassthroughClient on passthrough service manager. "
                   << "Call it on defaultServiceManager() instead.";
        return Void();
    }

    Return<bool> unregisterForNotifications(const hidl_string& /* fqName */,
                                            const hidl_string& /* name */,
                                            const sp<IServiceNotification>& /* callback */) override {
        // This makes no sense.
        LOG(FATAL) << "Cannot unregister for notifications with passthrough service manager.";
        return false;
    }

};

sp<IServiceManager1_0> getPassthroughServiceManager() {
    return getPassthroughServiceManager1_1();
}
sp<IServiceManager1_1> getPassthroughServiceManager1_1() {
    static sp<PassthroughServiceManager> manager(new PassthroughServiceManager());
    return manager;
}

std::vector<std::string> getAllHalInstanceNames(const std::string& descriptor) {
    std::vector<std::string> ret;
    auto sm = defaultServiceManager1_2();
    sm->listManifestByInterface(descriptor, [&](const auto& instances) {
        ret.reserve(instances.size());
        for (const auto& i : instances) {
            ret.push_back(i);
        }
    });
    return ret;
}

namespace details {

void preloadPassthroughService(const std::string &descriptor) {
    PassthroughServiceManager::openLibs(descriptor,
        [&](void* /* handle */, const std::string& /* lib */, const std::string& /* sym */) {
            // do nothing
            return true; // open all libs
        });
}

struct Waiter : IServiceNotification {
    Waiter(const std::string& interface, const std::string& instanceName,
           const sp<IServiceManager1_1>& sm) : mInterfaceName(interface),
                                               mInstanceName(instanceName), mSm(sm) {
    }

    void onFirstRef() override {
        // If this process only has one binder thread, and we're calling wait() from
        // that thread, it will block forever because we hung up the one and only
        // binder thread on a condition variable that can only be notified by an
        // incoming binder call.
        if (IPCThreadState::self()->isOnlyBinderThread()) {
            LOG(WARNING) << "Can't efficiently wait for " << mInterfaceName << "/"
                         << mInstanceName << ", because we are called from "
                         << "the only binder thread in this process.";
            return;
        }

        Return<bool> ret = mSm->registerForNotifications(mInterfaceName, mInstanceName, this);

        if (!ret.isOk()) {
            LOG(ERROR) << "Transport error, " << ret.description()
                       << ", during notification registration for " << mInterfaceName << "/"
                       << mInstanceName << ".";
            return;
        }

        if (!ret) {
            LOG(ERROR) << "Could not register for notifications for " << mInterfaceName << "/"
                       << mInstanceName << ".";
            return;
        }

        mRegisteredForNotifications = true;
    }

    ~Waiter() {
        if (!mDoneCalled) {
            LOG(FATAL)
                << "Waiter still registered for notifications, call done() before dropping ref!";
        }
    }

    Return<void> onRegistration(const hidl_string& /* fqName */,
                                const hidl_string& /* name */,
                                bool /* preexisting */) override {
        std::unique_lock<std::mutex> lock(mMutex);
        if (mRegistered) {
            return Void();
        }
        mRegistered = true;
        lock.unlock();

        mCondition.notify_one();
        return Void();
    }

    void wait(bool timeout) {
        using std::literals::chrono_literals::operator""s;

        if (!mRegisteredForNotifications) {
            // As an alternative, just sleep for a second and return
            LOG(WARNING) << "Waiting one second for " << mInterfaceName << "/" << mInstanceName;
            sleep(1);
            return;
        }

        std::unique_lock<std::mutex> lock(mMutex);
        do {
            mCondition.wait_for(lock, 1s, [this]{
                return mRegistered;
            });

            if (mRegistered) {
                break;
            }

            LOG(WARNING) << "Waited one second for " << mInterfaceName << "/" << mInstanceName;
        } while (!timeout);
    }

    // Be careful when using this; after calling reset(), you must always try to retrieve
    // the corresponding service before blocking on the waiter; otherwise, you might run
    // into a race-condition where the service has just (re-)registered, you clear the state
    // here, and subsequently calling waiter->wait() will block forever.
    void reset() {
        std::unique_lock<std::mutex> lock(mMutex);
        mRegistered = false;
    }

    // done() must be called before dropping the last strong ref to the Waiter, to make
    // sure we can properly unregister with hwservicemanager.
    void done() {
        if (mRegisteredForNotifications) {
            if (!mSm->unregisterForNotifications(mInterfaceName, mInstanceName, this)
                     .withDefault(false)) {
                LOG(ERROR) << "Could not unregister service notification for " << mInterfaceName
                           << "/" << mInstanceName << ".";
            } else {
                mRegisteredForNotifications = false;
            }
        }
        mDoneCalled = true;
    }

   private:
    const std::string mInterfaceName;
    const std::string mInstanceName;
    sp<IServiceManager1_1> mSm;
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mRegistered = false;
    bool mRegisteredForNotifications = false;
    bool mDoneCalled = false;
};

void waitForHwService(
        const std::string &interface, const std::string &instanceName) {
    sp<Waiter> waiter = new Waiter(interface, instanceName, defaultServiceManager1_1());
    waiter->wait(false /* timeout */);
    waiter->done();
}

// Prints relevant error/warning messages for error return values from
// details::canCastInterface(), both transaction errors (!castReturn.isOk())
// as well as actual cast failures (castReturn.isOk() && castReturn = false).
// Returns 'true' if the error is non-fatal and it's useful to retry
bool handleCastError(const Return<bool>& castReturn, const std::string& descriptor,
                     const std::string& instance) {
    if (castReturn.isOk()) {
        if (castReturn) {
            details::logAlwaysFatal("Successful cast value passed into handleCastError.");
        }
        // This should never happen, and there's not really a point in retrying.
        ALOGE("getService: received incompatible service (bug in hwservicemanager?) for "
            "%s/%s.", descriptor.c_str(), instance.c_str());
        return false;
    }
    if (castReturn.isDeadObject()) {
        ALOGW("getService: found dead hwbinder service for %s/%s.", descriptor.c_str(),
              instance.c_str());
        return true;
    }
    // This can happen due to:
    // 1) No SELinux permissions
    // 2) Other transaction failure (no buffer space, kernel error)
    // The first isn't recoverable, but the second is.
    // Since we can't yet differentiate between the two, and clients depend
    // on us not blocking in case 1), treat this as a fatal error for now.
    ALOGW("getService: unable to call into hwbinder service for %s/%s.",
          descriptor.c_str(), instance.c_str());
    return false;
}

sp<::android::hidl::base::V1_0::IBase> getRawServiceInternal(const std::string& descriptor,
                                                             const std::string& instance,
                                                             bool retry, bool getStub) {
    using Transport = IServiceManager1_0::Transport;
    sp<Waiter> waiter;

    sp<IServiceManager1_1> sm;
    Transport transport = Transport::EMPTY;
    if (kIsRecovery) {
        transport = Transport::PASSTHROUGH;
    } else {
        sm = defaultServiceManager1_1();
        if (sm == nullptr) {
            ALOGE("getService: defaultServiceManager() is null");
            return nullptr;
        }

        Return<Transport> transportRet = sm->getTransport(descriptor, instance);

        if (!transportRet.isOk()) {
            ALOGE("getService: defaultServiceManager()->getTransport returns %s",
                  transportRet.description().c_str());
            return nullptr;
        }
        transport = transportRet;
    }

    const bool vintfHwbinder = (transport == Transport::HWBINDER);
    const bool vintfPassthru = (transport == Transport::PASSTHROUGH);
    const bool trebleTestingOverride = isTrebleTestingOverride();
    const bool allowLegacy = !kEnforceVintfManifest || (trebleTestingOverride && isDebuggable());
    const bool vintfLegacy = (transport == Transport::EMPTY) && allowLegacy;

    if (!kEnforceVintfManifest) {
        ALOGE("getService: Potential race detected. The VINTF manifest is not being enforced. If "
              "a HAL server has a delay in starting and it is not in the manifest, it will not be "
              "retrieved. Please make sure all HALs on this device are in the VINTF manifest and "
              "enable PRODUCT_ENFORCE_VINTF_MANIFEST on this device (this is also enabled by "
              "PRODUCT_FULL_TREBLE). PRODUCT_ENFORCE_VINTF_MANIFEST will ensure that no race "
              "condition is possible here.");
        sleep(1);
    }

    for (int tries = 0; !getStub && (vintfHwbinder || vintfLegacy); tries++) {
        if (waiter == nullptr && tries > 0) {
            waiter = new Waiter(descriptor, instance, sm);
        }
        if (waiter != nullptr) {
            waiter->reset();  // don't reorder this -- see comments on reset()
        }
        Return<sp<IBase>> ret = sm->get(descriptor, instance);
        if (!ret.isOk()) {
            ALOGE("getService: defaultServiceManager()->get returns %s for %s/%s.",
                  ret.description().c_str(), descriptor.c_str(), instance.c_str());
            break;
        }
        sp<IBase> base = ret;
        if (base != nullptr) {
            Return<bool> canCastRet =
                details::canCastInterface(base.get(), descriptor.c_str(), true /* emitError */);

            if (canCastRet.isOk() && canCastRet) {
                if (waiter != nullptr) {
                    waiter->done();
                }
                return base; // still needs to be wrapped by Bp class.
            }

            if (!handleCastError(canCastRet, descriptor, instance)) break;
        }

        // In case of legacy or we were not asked to retry, don't.
        if (vintfLegacy || !retry) break;

        if (waiter != nullptr) {
            ALOGI("getService: Trying again for %s/%s...", descriptor.c_str(), instance.c_str());
            waiter->wait(true /* timeout */);
        }
    }

    if (waiter != nullptr) {
        waiter->done();
    }

    if (getStub || vintfPassthru || vintfLegacy) {
        const sp<IServiceManager1_0> pm = getPassthroughServiceManager();
        if (pm != nullptr) {
            sp<IBase> base = pm->get(descriptor, instance).withDefault(nullptr);
            if (!getStub || trebleTestingOverride) {
                base = wrapPassthrough(base);
            }
            return base;
        }
    }

    return nullptr;
}

status_t registerAsServiceInternal(const sp<IBase>& service, const std::string& name) {
    if (service == nullptr) {
        return UNEXPECTED_NULL;
    }

    sp<IServiceManager1_2> sm = defaultServiceManager1_2();
    if (sm == nullptr) {
        return INVALID_OPERATION;
    }

    const std::string descriptor = getDescriptor(service.get());

    if (kEnforceVintfManifest && !isTrebleTestingOverride()) {
        using Transport = IServiceManager1_0::Transport;
        Return<Transport> transport = sm->getTransport(descriptor, name);

        if (!transport.isOk()) {
            LOG(ERROR) << "Could not get transport for " << descriptor << "/" << name << ": "
                       << transport.description();
            return UNKNOWN_ERROR;
        }

        if (transport != Transport::HWBINDER) {
            LOG(ERROR) << "Service " << descriptor << "/" << name
                       << " must be in VINTF manifest in order to register/get.";
            return UNKNOWN_ERROR;
        }
    }

    bool registered = false;
    Return<void> ret = service->interfaceChain([&](const auto& chain) {
        registered = sm->addWithChain(name.c_str(), service, chain).withDefault(false);
    });

    if (!ret.isOk()) {
        LOG(ERROR) << "Could not retrieve interface chain: " << ret.description();
    }

    if (registered) {
        onRegistrationImpl(descriptor, name);
    }

    return registered ? OK : UNKNOWN_ERROR;
}

} // namespace details

} // namespace hardware
} // namespace android
