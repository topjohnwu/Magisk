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

#include <hidladapter/HidlBinderAdapter.h>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/hidl/base/1.0/IBase.h>
#include <android/hidl/manager/1.0/IServiceManager.h>
#include <hidl/HidlTransportSupport.h>

#include <unistd.h>
#include <iostream>
#include <map>
#include <string>

namespace android {
namespace hardware {
namespace details {

using android::base::SetProperty;
using android::base::WaitForProperty;

const static std::string kDeactivateProp = "test.hidl.adapters.deactivated";

int usage(const std::string& me) {
    std::cerr << "usage: " << me
              << " [-p|P] [-n instance-name] interface-name instance-name number-of-threads."
              << std::endl;
    std::cerr << "    -p: stop based on property " << kDeactivateProp << "and reset it."
              << std::endl;
    std::cerr << "    -P: stop based on interface specific property " << kDeactivateProp
              << ".<fq-name>.<instance-name>" << std::endl;
    std::cerr
        << "    -n instance-name: register as a different instance name (does not de-register)"
        << std::endl;
    return EINVAL;
}

enum class StopMethod {
    NONE,
    ALL,
    SPECIFIC,
};

struct Args {
    StopMethod stopMethod = StopMethod::NONE;
    std::string interface;     // e.x. IFoo
    std::string instanceName;  // e.x. default
    int threadNumber;
    std::string registerInstanceName;  // e.x. default
};

bool processArguments(int argc, char** argv, Args* args) {
    int c;
    while ((c = getopt(argc, argv, "pPn:")) != -1) {
        switch (c) {
            case 'p': {
                args->stopMethod = StopMethod::ALL;
                break;
            }
            case 'P': {
                args->stopMethod = StopMethod::SPECIFIC;
                break;
            }
            case 'n': {
                args->registerInstanceName = optarg;
                break;
            }
            default: { return false; }
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 3) {
        std::cerr << "ERROR: requires exactly three positional arguments for "
                     "interface, instance name, and number of threads, but "
                  << argc << " provided." << std::endl;
        return false;
    }

    args->interface = argv[0];
    args->instanceName = argv[1];
    args->threadNumber = std::stoi(argv[2]);

    if (args->threadNumber <= 0) {
        std::cerr << "ERROR: invalid thread number " << args->threadNumber
                  << " must be a positive integer." << std::endl;
        return false;
    }

    if (args->registerInstanceName.empty()) {
        args->registerInstanceName = args->instanceName;
    }

    return true;
}

// only applies for -p argument
void waitForAdaptersDeactivated(const std::string& property) {
    using std::literals::chrono_literals::operator""s;

    while (!WaitForProperty(property, "true", 30s)) {
        // Log this so that when using this option on testing devices, there is
        // a clear indication if adapters are not properly stopped
        LOG(WARNING) << "Adapter use in progress. Waiting for stop based on 'true' " << property;
    }

    SetProperty(property, "false");
}

int adapterMain(const std::string& package, int argc, char** argv,
                const AdaptersFactory& adapters) {
    using android::hardware::configureRpcThreadpool;
    using android::hidl::base::V1_0::IBase;
    using android::hidl::manager::V1_0::IServiceManager;

    const std::string& me = argc > 0 ? argv[0] : "(error)";

    Args args;
    if (!processArguments(argc, argv, &args)) {
        return usage(me);
    }

    std::string interfaceName = package + "::" + args.interface;

    auto it = adapters.find(interfaceName);
    if (it == adapters.end()) {
        std::cerr << "ERROR: could not resolve " << interfaceName << "." << std::endl;
        return 1;
    }

    std::cout << "Trying to adapt down " << interfaceName << "/" << args.instanceName << " to "
              << args.registerInstanceName << std::endl;

    configureRpcThreadpool(args.threadNumber, false /* callerWillJoin */);

    sp<IServiceManager> manager = IServiceManager::getService();
    if (manager == nullptr) {
        std::cerr << "ERROR: could not retrieve service manager." << std::endl;
        return 1;
    }

    sp<IBase> implementation = manager->get(interfaceName, args.instanceName).withDefault(nullptr);
    if (implementation == nullptr) {
        std::cerr << "ERROR: could not retrieve desired implementation" << std::endl;
        return 1;
    }

    sp<IBase> adapter = it->second(implementation);
    if (adapter == nullptr) {
        std::cerr << "ERROR: could not create adapter." << std::endl;
        return 1;
    }

    bool replaced = manager->add(args.registerInstanceName, adapter).withDefault(false);
    if (!replaced) {
        std::cerr << "ERROR: could not register the service with the service manager." << std::endl;
        return 1;
    }

    switch (args.stopMethod) {
        case StopMethod::NONE: {
            std::cout << "Press any key to disassociate adapter." << std::endl;
            getchar();
            break;
        };
        case StopMethod::SPECIFIC: {
            const std::string property =
                kDeactivateProp + "." + interfaceName + "." + args.registerInstanceName;
            std::cout << "Set " << property << " to true to deactivate." << std::endl;
            waitForAdaptersDeactivated(property);
            break;
        };
        case StopMethod::ALL: {
            std::cout << "Set " << kDeactivateProp << " to true to deactivate." << std::endl;
            waitForAdaptersDeactivated(kDeactivateProp);
            break;
        };
    }

    // automatically unregistered on process exit if it is a new instance name
    if (args.registerInstanceName == args.instanceName) {
        bool restored = manager->add(args.instanceName, implementation).withDefault(false);
        if (!restored) {
            std::cerr << "ERROR: could not re-register interface with the service manager."
                      << std::endl;
            return 1;
        }
    }

    std::cout << "Success." << std::endl;

    return 0;
}

// If an interface is adapted to 1.0, it can then not be adapted to 1.1 in the same process.
// This poses a problem in the following scenario:
// auto interface = new V1_1::implementation::IFoo;
// hidlObject1_0->foo(interface) // adaptation set at 1.0
// hidlObject1_1->bar(interface) // adaptation still is 1.0
// This could be solved by keeping a map of IBase,fqName -> IBase, but then you end up
// with multiple names for the same interface.
sp<IBase> adaptWithDefault(const sp<IBase>& something,
                           const std::function<sp<IBase>()>& makeDefault) {
    static std::map<sp<IBase>, sp<IBase>> sAdapterMap;

    if (something == nullptr) {
        return something;
    }

    auto it = sAdapterMap.find(something);
    if (it == sAdapterMap.end()) {
        it = sAdapterMap.insert(it, {something, makeDefault()});
    }

    return it->second;
}

}  // namespace details
}  // namespace hardware
}  // namespace android
