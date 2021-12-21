#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_ISERVICEMANAGER_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_ISERVICEMANAGER_H

#include <android/hidl/base/1.0/IBase.h>
#include <android/hidl/manager/1.1/IServiceManager.h>
#include <android/hidl/manager/1.2/IClientCallback.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_2 {

struct IServiceManager : public ::android::hidl::manager::V1_1::IServiceManager {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef ::android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "android.hidl.manager@1.2::IServiceManager"
     */
    static const char* descriptor;

    /**
     * Returns whether this object's implementation is outside of the current process.
     */
    virtual bool isRemote() const override { return false; }

    /**
     * Retrieve an existing service that supports the requested version.
     *
     * WARNING: This function is for libhidl/HwBinder use only. You are likely
     * looking for 'IMyInterface::getService("name")' instead.
     *
     * @param fqName   Fully-qualified interface name.
     * @param name     Instance name. Same as in IServiceManager::add.
     *
     * @return service Handle to requested service, same as provided in
     *                 IServiceManager::add. Will be nullptr if not available.
     */
    virtual ::android::hardware::Return<::android::sp<::android::hidl::base::V1_0::IBase>> get(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name) = 0;

    /**
     * Register a service. The service manager must retrieve the (inherited)
     * interfaces that this service implements, and register them along with
     * the service.
     *
     * Each interface must have its own namespace for instance names. If you
     * have two unrelated interfaces IFoo and IBar, it must be valid to call:
     *
     * add("my_instance", foo); // foo implements IFoo
     * add("my_instance", bar); // bar implements IBar
     *
     * WARNING: This function is for libhidl/HwBinder use only. You are likely
     * looking for 'INTERFACE::registerAsService("name")' instead.
     *
     * @param name           Instance name. Must also be used to retrieve service.
     * @param service        Handle to registering service.
     *
     * @return success       Whether or not the service was registered.
     *
     */
    virtual ::android::hardware::Return<bool> add(const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::base::V1_0::IBase>& service) = 0;

    /**
     * Get the transport of a service.
     *
     * @param fqName     Fully-qualified interface name.
     * @param name       Instance name. Same as in IServiceManager::add
     *
     * @return transport Transport of service if known.
     */
    virtual ::android::hardware::Return<::android::hidl::manager::V1_0::IServiceManager::Transport> getTransport(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name) = 0;

    /**
     * Return callback for list
     */
    using list_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& fqInstanceNames)>;
    /**
     * List all registered services. Must be sorted.
     *
     * @return fqInstanceNames List of fully-qualified instance names.
     */
    virtual ::android::hardware::Return<void> list(list_cb _hidl_cb) = 0;

    /**
     * Return callback for listByInterface
     */
    using listByInterface_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& instanceNames)>;
    /**
     * List all instances of a particular service. Must be sorted.
     *
     * @param fqName         Fully-qualified interface name.
     *
     * @return instanceNames List of instance names running the particular service.
     */
    virtual ::android::hardware::Return<void> listByInterface(const ::android::hardware::hidl_string& fqName, listByInterface_cb _hidl_cb) = 0;

    /**
     * Register for service notifications for a particular service. Must support
     * multiple registrations.
     *
     * onRegistration must be sent out for all services which support the
     * version provided in the fqName. For instance, if a client registers for
     * notifications from "android.hardware.foo@1.0", they must also get
     * notifications from "android.hardware.foo@1.1". If a matching service
     * is already registered, onRegistration must be sent out with preexisting
     * = true.
     *
     * @param fqName   Fully-qualified interface name.
     * @param name     Instance name. If name is empty, notifications must be
     *                 sent out for all names.
     * @param callback Client callback to recieve notifications.
     *
     * @return success Whether or not registration was successful.
     */
    virtual ::android::hardware::Return<bool> registerForNotifications(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification>& callback) = 0;

    /**
     * Return callback for debugDump
     */
    using debugDump_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo>& info)>;
    /**
     * Similar to list, but contains more information for each instance.
     * @return info a vector where each item contains debug information for each
     *         instance.
     */
    virtual ::android::hardware::Return<void> debugDump(debugDump_cb _hidl_cb) = 0;

    /**
     * When the passthrough service manager returns a service via
     * get(string, string), it must dispatch a registerPassthroughClient call
     * to the binderized service manager to indicate the current process has
     * called get(). Binderized service manager must record this PID, which can
     * be retrieved via debugDump.
     */
    virtual ::android::hardware::Return<void> registerPassthroughClient(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name) = 0;

    /**
     * Unregister for service notifications for a specific callback.
     *
     * @param fqName   Fully-qualified interface name. If empty, unregister for
     *                 all notifications the callback receives.
     * @param name     Instance name. If name is empty, unregister for all instance
     *                 names.
     * @param callback Client callback that was previously registered.
     *
     * @return success Whether or not deregistration was successful.
     */
    virtual ::android::hardware::Return<bool> unregisterForNotifications(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification>& callback) = 0;

    /**
     * Adds a callback that must be called when the specified server has no clients.
     *
     * If the service has clients at the time of registration, the callback is called with
     * hasClients true. After that, it is called based on the changes in clientele.
     *
     * @param fqName Fully-qualified interface name (used to register)
     * @param name Instance name (of the registered service)
     * @param server non-null service waiting to have no clients (previously registered)
     * @param cb non-null callback to call when there are no clients
     * @return success
     *     true on success
     *     false if either
     *      - the server or cb parameters are null
     *      - this is called by a process other than the server process
     */
    virtual ::android::hardware::Return<bool> registerClientCallback(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::base::V1_0::IBase>& server, const ::android::sp<::android::hidl::manager::V1_2::IClientCallback>& cb) = 0;

    /**
     * Removes a callback previously registered with registerClientCallback.
     *
     * If server is null, then this must remove the cb from all matching services.
     *
     * @param server service registered with registerClientCallback
     * @param cb non-null callback to remove
     * @return success
     *     true if the server(s) have been removed
     *     false if cb is null or if the client callback or server could not be found
     */
    virtual ::android::hardware::Return<bool> unregisterClientCallback(const ::android::sp<::android::hidl::base::V1_0::IBase>& server, const ::android::sp<::android::hidl::manager::V1_2::IClientCallback>& cb) = 0;

    /**
     * Exactly the same as @1.0::IServiceManager.add, but the interfaceChain of the service is
     * provided in the same call.
     *
     * @param name Instance name. Must also be used to retrieve service.
     * @param service Handle to registering service.
     * @param chain service->interfaceChain
     *
     * @return success Whether or not the service was registered.
     */
    virtual ::android::hardware::Return<bool> addWithChain(const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::base::V1_0::IBase>& service, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& chain) = 0;

    /**
     * Return callback for listManifestByInterface
     */
    using listManifestByInterface_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& instanceNames)>;
    /**
     * List all instances of a particular service from the manifest. Must be sorted. These HALs
     * may not be started if they are lazy.
     *
     * See also @1.0::IServiceManager's listByInterface function.
     *
     * @param fqName Fully-qualified interface name.
     *
     * @return instanceNames List of instance names running the particular service.
     */
    virtual ::android::hardware::Return<void> listManifestByInterface(const ::android::hardware::hidl_string& fqName, listManifestByInterface_cb _hidl_cb) = 0;

    /**
     * Unregisters a service if there are no clients for it. This must only unregister the
     * service if it is called from the same process that registered the service.
     *
     * This unregisters all instances of the service, even if they are under a different instance
     * name.
     *
     * Recommended usage is when creating a lazy process, try unregistering when IClientCallback's
     * onClients(*, false) is called. If this unregister is successful, then it is safe to exit. If
     * it is unsuccessful, then you can assume a client re-associated with the server. If a process
     * has multiple servers, and only some succeed in unregistration, then the unregistered services
     * can be re-registered.
     *
     * See also addWithChain and @1.0::IServiceManager's add.
     *
     * @param fqName Fully-qualified interface name.
     * @param name Instance name.
     * @param service Handle to registering service.
     *
     * @return success Whether the service was successfully unregistered.
     */
    virtual ::android::hardware::Return<bool> tryUnregister(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::base::V1_0::IBase>& service) = 0;

    /**
     * Return callback for interfaceChain
     */
    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    /*
     * Provides run-time type information for this object.
     * For example, for the following interface definition:
     *     package android.hardware.foo@1.0;
     *     interface IParent {};
     *     interface IChild extends IParent {};
     * Calling interfaceChain on an IChild object must yield the following:
     *     ["android.hardware.foo@1.0::IChild",
     *      "android.hardware.foo@1.0::IParent"
     *      "android.hidl.base@1.0::IBase"]
     *
     * @return descriptors a vector of descriptors of the run-time type of the
     *         object.
     */
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    /*
     * Emit diagnostic information to the given file.
     *
     * Optionally overriden.
     *
     * @param fd      File descriptor to dump data to.
     *                Must only be used for the duration of this call.
     * @param options Arguments for debugging.
     *                Must support empty for default debug information.
     */
    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    /**
     * Return callback for interfaceDescriptor
     */
    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    /*
     * Provides run-time type information for this object.
     * For example, for the following interface definition:
     *     package android.hardware.foo@1.0;
     *     interface IParent {};
     *     interface IChild extends IParent {};
     * Calling interfaceDescriptor on an IChild object must yield
     *     "android.hardware.foo@1.0::IChild"
     *
     * @return descriptor a descriptor of the run-time type of the
     *         object (the first element of the vector returned by
     *         interfaceChain())
     */
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    /**
     * Return callback for getHashChain
     */
    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    /*
     * Returns hashes of the source HAL files that define the interfaces of the
     * runtime type information on the object.
     * For example, for the following interface definition:
     *     package android.hardware.foo@1.0;
     *     interface IParent {};
     *     interface IChild extends IParent {};
     * Calling interfaceChain on an IChild object must yield the following:
     *     [(hash of IChild.hal),
     *      (hash of IParent.hal)
     *      (hash of IBase.hal)].
     *
     * SHA-256 is used as the hashing algorithm. Each hash has 32 bytes
     * according to SHA-256 standard.
     *
     * @return hashchain a vector of SHA-1 digests
     */
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    /*
     * This method trigger the interface to enable/disable instrumentation based
     * on system property hal.instrumentation.enable.
     */
    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    /*
     * Registers a death recipient, to be called when the process hosting this
     * interface dies.
     *
     * @param recipient a hidl_death_recipient callback object
     * @param cookie a cookie that must be returned with the callback
     * @return success whether the death recipient was registered successfully.
     */
    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    /*
     * Provides way to determine if interface is running without requesting
     * any functionality.
     */
    virtual ::android::hardware::Return<void> ping() override;

    /**
     * Return callback for getDebugInfo
     */
    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    /*
     * Get debug information on references on this interface.
     * @return info debugging information. See comments of DebugInfo.
     */
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    /*
     * This method notifies the interface that one or more system properties
     * have changed. The default implementation calls
     * (C++)  report_sysprop_change() in libcutils or
     * (Java) android.os.SystemProperties.reportSyspropChanged,
     * which in turn calls a set of registered callbacks (eg to update trace
     * tags).
     */
    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    /*
     * Unregisters the registered death recipient. If this service was registered
     * multiple times with the same exact death recipient, this unlinks the most
     * recently registered one.
     *
     * @param recipient a previously registered hidl_death_recipient callback
     * @return success whether the death recipient was unregistered successfully.
     */
    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;

    // cast static functions
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hidl::manager::V1_2::IServiceManager>> castFrom(const ::android::sp<::android::hidl::manager::V1_2::IServiceManager>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hidl::manager::V1_2::IServiceManager>> castFrom(const ::android::sp<::android::hidl::manager::V1_1::IServiceManager>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hidl::manager::V1_2::IServiceManager>> castFrom(const ::android::sp<::android::hidl::manager::V1_0::IServiceManager>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hidl::manager::V1_2::IServiceManager>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    // helper methods for interactions with the hwservicemanager
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is currently not available or not in the VINTF manifest on a Trebilized
     * device, this will return nullptr. This is useful when you don't want to block
     * during device boot. If getStub is true, this will try to return an unwrapped
     * passthrough implementation in the same process. This is useful when getting an
     * implementation from the same partition/compilation group.
     *
     * In general, prefer getService(std::string,bool)
     */
    static ::android::sp<IServiceManager> tryGetService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IServiceManager> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IServiceManager> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    /**
     * Calls tryGetService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IServiceManager> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is not in the VINTF manifest on a Trebilized device, this will return
     * nullptr. If the service is not available, this will wait for the service to
     * become available. If the service is a lazy service, this will start the service
     * and return when it becomes available. If getStub is true, this will try to
     * return an unwrapped passthrough implementation in the same process. This is
     * useful when getting an implementation from the same partition/compilation group.
     */
    static ::android::sp<IServiceManager> getService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IServiceManager> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IServiceManager> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    /**
     * Calls getService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IServiceManager> getService(bool getStub) { return getService("default", getStub); }
    /**
     * Registers a service with the service manager. For Trebilized devices, the service
     * must also be in the VINTF manifest.
     */
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    /**
     * Registers for notifications for when a service is registered.
     */
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

//
// type declarations for package
//

static inline std::string toString(const ::android::sp<::android::hidl::manager::V1_2::IServiceManager>& o);

//
// type header definitions for package
//

static inline std::string toString(const ::android::sp<::android::hidl::manager::V1_2::IServiceManager>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hidl::manager::V1_2::IServiceManager::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V1_2
}  // namespace manager
}  // namespace hidl
}  // namespace android

//
// global type declarations for package
//


#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_ISERVICEMANAGER_H
