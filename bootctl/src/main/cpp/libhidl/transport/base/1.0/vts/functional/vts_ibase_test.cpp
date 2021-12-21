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
#define LOG_TAG "vts_ibase_test"

#include <algorithm>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <android/hidl/base/1.0/IBase.h>
#include <android/hidl/manager/1.0/IServiceManager.h>
#include <gtest/gtest.h>
#include <hidl-util/FqInstance.h>
#include <hidl/HidlBinderSupport.h>
#include <hidl/ServiceManagement.h>
#include <init-test-utils/service_utils.h>

using android::FqInstance;
using android::FQName;
using android::sp;
using android::wp;
using android::base::Result;
using android::hardware::hidl_array;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_handle;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::hardware::IBinder;
using android::hardware::toBinder;
using android::hidl::base::V1_0::IBase;
using android::hidl::manager::V1_0::IServiceManager;
using android::init::ServiceInterfacesMap;
using PidInterfacesMap = std::map<pid_t, std::set<FqInstance>>;

template <typename T>
static inline ::testing::AssertionResult isOk(const ::android::hardware::Return<T>& ret) {
    return ret.isOk() ? (::testing::AssertionSuccess() << ret.description())
                      : (::testing::AssertionFailure() << ret.description());
}
#define ASSERT_OK(__ret__) ASSERT_TRUE(isOk(__ret__))
#define EXPECT_OK(__ret__) EXPECT_TRUE(isOk(__ret__))

struct Hal {
    sp<IBase> service;
    std::string name;  // space separated list of android.hidl.foo@1.0::IFoo/instance-name
    FqInstance fq_instance;
};

template <typename T>
std::string FqInstancesToString(const T& instances) {
    std::set<std::string> instance_strings;
    for (const FqInstance& instance : instances) {
        instance_strings.insert(instance.string());
    }
    return android::base::Join(instance_strings, "\n");
}

pid_t GetServiceDebugPid(const std::string& service) {
    return android::base::GetIntProperty("init.svc_debug_pid." + service, 0);
}

std::map<std::string, std::vector<Hal>> gDeclaredServiceHalMap;
std::mutex gDeclaredServiceHalMapMutex;

void GetHal(const std::string& service, const FqInstance& instance) {
    if (instance.getFqName().string() == IBase::descriptor) {
        return;
    }

    sp<IBase> hal = android::hardware::details::getRawServiceInternal(
            instance.getFqName().string(), instance.getInstance(), true /*retry*/,
            false /*getStub*/);
    // Add to gDeclaredServiceHalMap if getRawServiceInternal() returns (even if
    // the returned HAL is null). getRawServiceInternal() won't return if the
    // HAL is in the VINTF but unable to start.
    std::lock_guard<std::mutex> guard(gDeclaredServiceHalMapMutex);
    gDeclaredServiceHalMap[service].push_back(Hal{.service = hal, .fq_instance = instance});
}

class VtsHalBaseV1_0TargetTest : public ::testing::Test {
   public:
    virtual void SetUp() override {
        default_manager_ = ::android::hardware::defaultServiceManager();

        ASSERT_NE(default_manager_, nullptr)
            << "Failed to get default service manager." << std::endl;

        ASSERT_OK(default_manager_->list([&](const auto& list) {
            for (const auto& name : list) {
                const std::string strName = name;
                auto loc = strName.find_first_of('/');
                if (loc == std::string::npos) {
                    ADD_FAILURE() << "Invalid FQName: " << strName;
                    continue;
                }
                const std::string fqName = strName.substr(0, loc);
                const std::string instance = strName.substr(loc + 1);

                sp<IBase> service = default_manager_->get(fqName, instance);
                if (service == nullptr) {
                    ADD_FAILURE() << "Null service for " << name << " " << fqName << " "
                                  << instance;
                    continue;
                }

                sp<IBinder> binder = toBinder(service);
                if (binder == nullptr) {
                    ADD_FAILURE() << "Null binder for " << name;
                    continue;
                }

                auto iter = all_hals_.find(binder);
                if (iter != all_hals_.end()) {
                    // include all the names this is registered as for error messages
                    iter->second.name += " " + strName;
                } else {
                    all_hals_.insert(iter, {binder, Hal{.service = service, .name = strName}});
                }
            }
        }));

        ASSERT_FALSE(all_hals_.empty());  // sanity
    }

    void EachHal(const std::function<void(const Hal&)>& check) {
        for (auto iter = all_hals_.begin(); iter != all_hals_.end(); ++iter) {
            check(iter->second);
        }
    }

    PidInterfacesMap GetPidInterfacesMap() {
        PidInterfacesMap result;
        EXPECT_OK(default_manager_->debugDump([&result](const auto& list) {
            for (const auto& debug_info : list) {
                if (debug_info.pid != static_cast<int32_t>(IServiceManager::PidConstant::NO_PID)) {
                    FQName fqName;
                    ASSERT_TRUE(fqName.setTo(debug_info.interfaceName.c_str()))
                            << "Unable to parse interface: '" << debug_info.interfaceName.c_str();
                    FqInstance fqInstance;
                    ASSERT_TRUE(fqInstance.setTo(fqName, debug_info.instanceName.c_str()));
                    if (fqInstance.getFqName().string() != IBase::descriptor) {
                        result[debug_info.pid].insert(fqInstance);
                    }
                }
            }
        }));
        return result;
    }

    // default service manager
    sp<IServiceManager> default_manager_;

    // map from underlying instance to actual instance
    //
    // this prevents calling the same service twice since the same service
    // will get registered multiple times for its entire inheritance
    // hierarchy (or perhaps as different instance names)
    std::map<sp<IBinder>, Hal> all_hals_;
};

TEST_F(VtsHalBaseV1_0TargetTest, CanPing) {
    EachHal(
        [&](const Hal& base) { EXPECT_OK(base.service->ping()) << "Cannot ping " << base.name; });
}

TEST_F(VtsHalBaseV1_0TargetTest, InterfaceChain) {
    EachHal([&](const Hal& base) {
        EXPECT_OK(base.service->interfaceChain([&](const auto& interfaceChain) {
            // must include IBase + subclasses
            EXPECT_GT(interfaceChain.size(), 1u) << "Invalid instance name " << base.name;
        })) << base.name;
    });
}

TEST_F(VtsHalBaseV1_0TargetTest, Descriptor) {
    EachHal([&](const Hal& base) {
        EXPECT_OK(base.service->interfaceDescriptor([&](const auto& descriptor) {
            // must include IBase + subclasses
            EXPECT_GT(descriptor.size(), 0u) << base.name;
            EXPECT_NE(IBase::descriptor, descriptor) << base.name;
        })) << base.name;
    });
}

TEST_F(VtsHalBaseV1_0TargetTest, Death) {
    struct HidlDeathRecipient : hidl_death_recipient {
        virtual void serviceDied(uint64_t /* cookie */, const wp<IBase>& /* who */){};
    };
    sp<hidl_death_recipient> recipient = new HidlDeathRecipient;

    EachHal([&](const Hal& base) {
        EXPECT_OK(base.service->linkToDeath(recipient, 0 /* cookie */))
            << "Register death recipient " << base.name;
        EXPECT_OK(base.service->unlinkToDeath(recipient)) << "Unlink death recipient " << base.name;
    });
}

TEST_F(VtsHalBaseV1_0TargetTest, Debug) {
    EachHal([&](const Hal& base) {
        // normally one is passed, but this is tested by dumpstate
        EXPECT_OK(base.service->debug(hidl_handle(), {}))
            << "Handle empty debug handle " << base.name;
    });
}

TEST_F(VtsHalBaseV1_0TargetTest, HashChain) {
    EachHal([&](const Hal& base) {
        EXPECT_OK(base.service->getHashChain([&](const auto& hashChain) {
            // must include IBase + subclasses
            EXPECT_NE(0u, hashChain.size()) << "Invalid hash chain " << base.name;
        })) << base.name;
    });
}

TEST_F(VtsHalBaseV1_0TargetTest, ServiceProvidesAndDeclaresTheSameInterfaces) {
    const Result<ServiceInterfacesMap> service_interfaces_map =
            android::init::GetOnDeviceServiceInterfacesMap();
    ASSERT_RESULT_OK(service_interfaces_map);

    std::map<std::string, std::set<FqInstance>> hidl_interfaces_map;

    // Attempt to get handles to all known declared interfaces. This will cause
    // any non-running lazy HALs to start up.
    // Results are saved in gDeclaredServiceHalMap.
    for (const auto& [service, declared_interfaces] : *service_interfaces_map) {
        if (declared_interfaces.empty()) {
            LOG(INFO) << "Service '" << service << "' does not declare any interfaces.";
        }
        for (const auto& interface : declared_interfaces) {
            if (interface.find("aidl/") == 0) {
                LOG(INFO) << "Not testing '" << service << "' AIDL interface: " << interface;
            } else {
                FqInstance fqInstance;
                ASSERT_TRUE(fqInstance.setTo(interface))
                        << "Unable to parse interface: '" << interface << "'";

                std::thread(GetHal, service, fqInstance).detach();
                hidl_interfaces_map[service].insert(fqInstance);
            }
        }
    }
    // Allow the threads 5 seconds to attempt to get each HAL. Any HAL whose
    // thread is stuck during retrieval is excluded from this test.
    sleep(5);

    std::lock_guard<std::mutex> guard(gDeclaredServiceHalMapMutex);
    PidInterfacesMap pid_interfaces_map = GetPidInterfacesMap();

    // For each service that had at least one thread return from attempting to
    // retrieve a HAL:
    for (const auto& [service, hals] : gDeclaredServiceHalMap) {
        // Assert that the service is running.
        pid_t pid = GetServiceDebugPid(service);
        ASSERT_NE(pid, 0) << "Service '" << service << "' is not running.";

        std::set<FqInstance> declared_interfaces;
        for (const auto& hal : hals) {
            declared_interfaces.insert(hal.fq_instance);
        }

        // Warn for any threads that were stuck when attempting to retrieve a
        // HAL.
        std::vector<FqInstance> missing_declared_interfaces;
        std::set_difference(hidl_interfaces_map[service].begin(),
                            hidl_interfaces_map[service].end(), declared_interfaces.begin(),
                            declared_interfaces.end(),
                            std::back_inserter(missing_declared_interfaces));
        if (!missing_declared_interfaces.empty()) {
            LOG(WARNING)
                    << "Service '" << service
                    << "' declares interfaces that are present in the VINTF but unable to start:"
                    << std::endl
                    << FqInstancesToString(missing_declared_interfaces);
        }

        // Expect that the set of interfaces running at this PID is the same as
        // the set of interfaces declared by this service.
        std::set<FqInstance> served_interfaces = pid_interfaces_map[pid];
        std::vector<FqInstance> served_declared_diff;
        std::set_symmetric_difference(declared_interfaces.begin(), declared_interfaces.end(),
                                      served_interfaces.begin(), served_interfaces.end(),
                                      std::back_inserter(served_declared_diff));

        EXPECT_TRUE(served_declared_diff.empty())
                << "Service '" << service << "' serves and declares different interfaces."
                << std::endl
                << "  Served:" << std::endl
                << FqInstancesToString(served_interfaces) << std::endl
                << "  Declared: " << std::endl
                << FqInstancesToString(declared_interfaces) << std::endl
                << "  Difference: " << std::endl
                << FqInstancesToString(served_declared_diff);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
