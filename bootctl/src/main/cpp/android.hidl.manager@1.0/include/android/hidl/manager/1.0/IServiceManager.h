#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_0_ISERVICEMANAGER_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_0_ISERVICEMANAGER_H

#include <android/hidl/base/1.0/IBase.h>
#include <android/hidl/base/1.0/types.h>
#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_0 {

/**
 * Manages all the hidl hals on a device.
 *
 * All examples in this file assume that there is one service registered with
 * the service manager, "android.hidl.manager@1.0::IServiceManager/manager"
 *
 * Terminology:
 *   Package: "android.hidl.manager"
 *   Major version: "1"
 *   Minor version: "0"
 *   Version: "1.0"
 *   Interface name: "IServiceManager"
 *   Fully-qualified interface name: "android.hidl.manager@1.0::IServiceManager"
 *   Instance name: "manager"
 *   Fully-qualified instance name: "android.hidl.manager@1.0::IServiceManager/manager"
 */
struct IServiceManager : public ::android::hidl::base::V1_0::IBase {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef ::android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "android.hidl.manager@1.0::IServiceManager"
     */
    static const char* descriptor;

    // Forward declaration for forward reference support:
    enum class Transport : uint8_t;
    enum class PidConstant : int32_t;
    struct InstanceDebugInfo;

    enum class Transport : uint8_t {
        EMPTY = 0,
        HWBINDER = 1 /* ::android::hidl::manager::V1_0::IServiceManager::Transport.EMPTY implicitly + 1 */,
        PASSTHROUGH = 2 /* ::android::hidl::manager::V1_0::IServiceManager::Transport.HWBINDER implicitly + 1 */,
    };

    /**
     * Special values for InstanceDebugInfo pids.
     */
    enum class PidConstant : int32_t {
        NO_PID = -1 /* -1 */,
    };

    /**
     * Returned object for debugDump().
     */
    struct InstanceDebugInfo final {
        ::android::hardware::hidl_string interfaceName __attribute__ ((aligned(8)));
        ::android::hardware::hidl_string instanceName __attribute__ ((aligned(8)));
        int32_t pid __attribute__ ((aligned(4)));
        ::android::hardware::hidl_vec<int32_t> clientPids __attribute__ ((aligned(8)));
        ::android::hidl::base::V1_0::DebugInfo::Architecture arch __attribute__ ((aligned(4)));
    };

    static_assert(offsetof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo, interfaceName) == 0, "wrong offset");
    static_assert(offsetof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo, instanceName) == 16, "wrong offset");
    static_assert(offsetof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo, pid) == 32, "wrong offset");
    static_assert(offsetof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo, clientPids) == 40, "wrong offset");
    static_assert(offsetof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo, arch) == 56, "wrong offset");
    static_assert(sizeof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo) == 64, "wrong size");
    static_assert(__alignof(::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo) == 8, "wrong alignment");

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
    static ::android::hardware::Return<::android::sp<::android::hidl::manager::V1_0::IServiceManager>> castFrom(const ::android::sp<::android::hidl::manager::V1_0::IServiceManager>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hidl::manager::V1_0::IServiceManager>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

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

template<typename>
static inline std::string toString(uint8_t o);
static inline std::string toString(::android::hidl::manager::V1_0::IServiceManager::Transport o);
static inline void PrintTo(::android::hidl::manager::V1_0::IServiceManager::Transport o, ::std::ostream* os);
constexpr uint8_t operator|(const ::android::hidl::manager::V1_0::IServiceManager::Transport lhs, const ::android::hidl::manager::V1_0::IServiceManager::Transport rhs) {
    return static_cast<uint8_t>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}
constexpr uint8_t operator|(const uint8_t lhs, const ::android::hidl::manager::V1_0::IServiceManager::Transport rhs) {
    return static_cast<uint8_t>(lhs | static_cast<uint8_t>(rhs));
}
constexpr uint8_t operator|(const ::android::hidl::manager::V1_0::IServiceManager::Transport lhs, const uint8_t rhs) {
    return static_cast<uint8_t>(static_cast<uint8_t>(lhs) | rhs);
}
constexpr uint8_t operator&(const ::android::hidl::manager::V1_0::IServiceManager::Transport lhs, const ::android::hidl::manager::V1_0::IServiceManager::Transport rhs) {
    return static_cast<uint8_t>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}
constexpr uint8_t operator&(const uint8_t lhs, const ::android::hidl::manager::V1_0::IServiceManager::Transport rhs) {
    return static_cast<uint8_t>(lhs & static_cast<uint8_t>(rhs));
}
constexpr uint8_t operator&(const ::android::hidl::manager::V1_0::IServiceManager::Transport lhs, const uint8_t rhs) {
    return static_cast<uint8_t>(static_cast<uint8_t>(lhs) & rhs);
}
constexpr uint8_t &operator|=(uint8_t& v, const ::android::hidl::manager::V1_0::IServiceManager::Transport e) {
    v |= static_cast<uint8_t>(e);
    return v;
}
constexpr uint8_t &operator&=(uint8_t& v, const ::android::hidl::manager::V1_0::IServiceManager::Transport e) {
    v &= static_cast<uint8_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hidl::manager::V1_0::IServiceManager::PidConstant o);
static inline void PrintTo(::android::hidl::manager::V1_0::IServiceManager::PidConstant o, ::std::ostream* os);
constexpr int32_t operator|(const ::android::hidl::manager::V1_0::IServiceManager::PidConstant lhs, const ::android::hidl::manager::V1_0::IServiceManager::PidConstant rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hidl::manager::V1_0::IServiceManager::PidConstant rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hidl::manager::V1_0::IServiceManager::PidConstant lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hidl::manager::V1_0::IServiceManager::PidConstant lhs, const ::android::hidl::manager::V1_0::IServiceManager::PidConstant rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hidl::manager::V1_0::IServiceManager::PidConstant rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hidl::manager::V1_0::IServiceManager::PidConstant lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hidl::manager::V1_0::IServiceManager::PidConstant e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hidl::manager::V1_0::IServiceManager::PidConstant e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& o);
static inline void PrintTo(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& o, ::std::ostream*);
static inline bool operator==(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& lhs, const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& rhs);
static inline bool operator!=(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& lhs, const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& rhs);

static inline std::string toString(const ::android::sp<::android::hidl::manager::V1_0::IServiceManager>& o);

//
// type header definitions for package
//

template<>
inline std::string toString<::android::hidl::manager::V1_0::IServiceManager::Transport>(uint8_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hidl::manager::V1_0::IServiceManager::Transport> flipped = 0;
    bool first = true;
    if ((o & ::android::hidl::manager::V1_0::IServiceManager::Transport::EMPTY) == static_cast<uint8_t>(::android::hidl::manager::V1_0::IServiceManager::Transport::EMPTY)) {
        os += (first ? "" : " | ");
        os += "EMPTY";
        first = false;
        flipped |= ::android::hidl::manager::V1_0::IServiceManager::Transport::EMPTY;
    }
    if ((o & ::android::hidl::manager::V1_0::IServiceManager::Transport::HWBINDER) == static_cast<uint8_t>(::android::hidl::manager::V1_0::IServiceManager::Transport::HWBINDER)) {
        os += (first ? "" : " | ");
        os += "HWBINDER";
        first = false;
        flipped |= ::android::hidl::manager::V1_0::IServiceManager::Transport::HWBINDER;
    }
    if ((o & ::android::hidl::manager::V1_0::IServiceManager::Transport::PASSTHROUGH) == static_cast<uint8_t>(::android::hidl::manager::V1_0::IServiceManager::Transport::PASSTHROUGH)) {
        os += (first ? "" : " | ");
        os += "PASSTHROUGH";
        first = false;
        flipped |= ::android::hidl::manager::V1_0::IServiceManager::Transport::PASSTHROUGH;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hidl::manager::V1_0::IServiceManager::Transport o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hidl::manager::V1_0::IServiceManager::Transport::EMPTY) {
        return "EMPTY";
    }
    if (o == ::android::hidl::manager::V1_0::IServiceManager::Transport::HWBINDER) {
        return "HWBINDER";
    }
    if (o == ::android::hidl::manager::V1_0::IServiceManager::Transport::PASSTHROUGH) {
        return "PASSTHROUGH";
    }
    std::string os;
    os += toHexString(static_cast<uint8_t>(o));
    return os;
}

static inline void PrintTo(::android::hidl::manager::V1_0::IServiceManager::Transport o, ::std::ostream* os) {
    *os << toString(o);
}

template<>
inline std::string toString<::android::hidl::manager::V1_0::IServiceManager::PidConstant>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hidl::manager::V1_0::IServiceManager::PidConstant> flipped = 0;
    bool first = true;
    if ((o & ::android::hidl::manager::V1_0::IServiceManager::PidConstant::NO_PID) == static_cast<int32_t>(::android::hidl::manager::V1_0::IServiceManager::PidConstant::NO_PID)) {
        os += (first ? "" : " | ");
        os += "NO_PID";
        first = false;
        flipped |= ::android::hidl::manager::V1_0::IServiceManager::PidConstant::NO_PID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hidl::manager::V1_0::IServiceManager::PidConstant o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hidl::manager::V1_0::IServiceManager::PidConstant::NO_PID) {
        return "NO_PID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline void PrintTo(::android::hidl::manager::V1_0::IServiceManager::PidConstant o, ::std::ostream* os) {
    *os << toString(o);
}

static inline std::string toString(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".interfaceName = ";
    os += ::android::hardware::toString(o.interfaceName);
    os += ", .instanceName = ";
    os += ::android::hardware::toString(o.instanceName);
    os += ", .pid = ";
    os += ::android::hardware::toString(o.pid);
    os += ", .clientPids = ";
    os += ::android::hardware::toString(o.clientPids);
    os += ", .arch = ";
    os += ::android::hidl::base::V1_0::toString(o.arch);
    os += "}"; return os;
}

static inline void PrintTo(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& o, ::std::ostream* os) {
    *os << toString(o);
}

static inline bool operator==(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& lhs, const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& rhs) {
    if (lhs.interfaceName != rhs.interfaceName) {
        return false;
    }
    if (lhs.instanceName != rhs.instanceName) {
        return false;
    }
    if (lhs.pid != rhs.pid) {
        return false;
    }
    if (lhs.clientPids != rhs.clientPids) {
        return false;
    }
    if (lhs.arch != rhs.arch) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& lhs, const ::android::hidl::manager::V1_0::IServiceManager::InstanceDebugInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::sp<::android::hidl::manager::V1_0::IServiceManager>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hidl::manager::V1_0::IServiceManager::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V1_0
}  // namespace manager
}  // namespace hidl
}  // namespace android

//
// global type declarations for package
//

namespace android {
namespace hardware {
namespace details {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template<> inline constexpr std::array<::android::hidl::manager::V1_0::IServiceManager::Transport, 3> hidl_enum_values<::android::hidl::manager::V1_0::IServiceManager::Transport> = {
    ::android::hidl::manager::V1_0::IServiceManager::Transport::EMPTY,
    ::android::hidl::manager::V1_0::IServiceManager::Transport::HWBINDER,
    ::android::hidl::manager::V1_0::IServiceManager::Transport::PASSTHROUGH,
};
#pragma clang diagnostic pop
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template<> inline constexpr std::array<::android::hidl::manager::V1_0::IServiceManager::PidConstant, 1> hidl_enum_values<::android::hidl::manager::V1_0::IServiceManager::PidConstant> = {
    ::android::hidl::manager::V1_0::IServiceManager::PidConstant::NO_PID,
};
#pragma clang diagnostic pop
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_0_ISERVICEMANAGER_H
