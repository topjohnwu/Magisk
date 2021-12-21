#ifndef HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_IBOOTCONTROL_H
#define HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_IBOOTCONTROL_H

#include <android/hardware/boot/1.0/IBootControl.h>
#include <android/hardware/boot/1.1/types.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_1 {

struct IBootControl : public ::android::hardware::boot::V1_0::IBootControl {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef ::android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "android.hardware.boot@1.1::IBootControl"
     */
    static const char* descriptor;

    /**
     * Returns whether this object's implementation is outside of the current process.
     */
    virtual bool isRemote() const override { return false; }

    /**
     * getNumberSlots() returns the number of available slots.
     * For instance, a system with a single set of partitions must return
     * 1, a system with A/B must return 2, A/B/C -> 3 and so on. A system with
     * less than two slots doesn't support background updates, for example if
     * running from a virtual machine with only one copy of each partition for the
     * purpose of testing.
     */
    virtual ::android::hardware::Return<uint32_t> getNumberSlots() = 0;

    /**
     * getCurrentSlot() returns the slot number of that the current boot is booted
     * from, for example slot number 0 (Slot A). It is assumed that if the current
     * slot is A, then the block devices underlying B can be accessed directly
     * without any risk of corruption.
     * The returned value is always guaranteed to be strictly less than the
     * value returned by getNumberSlots. Slots start at 0 and finish at
     * getNumberSlots() - 1. The value returned here must match the suffix passed
     * from the bootloader, regardless of which slot is active or successful.
     */
    virtual ::android::hardware::Return<uint32_t> getCurrentSlot() = 0;

    /**
     * Return callback for markBootSuccessful
     */
    using markBootSuccessful_cb = std::function<void(const ::android::hardware::boot::V1_0::CommandResult& error)>;
    /**
     * markBootSuccessful() marks the current slot as having booted successfully.
     *
     * Returns whether the command succeeded.
     */
    virtual ::android::hardware::Return<void> markBootSuccessful(markBootSuccessful_cb _hidl_cb) = 0;

    /**
     * Return callback for setActiveBootSlot
     */
    using setActiveBootSlot_cb = std::function<void(const ::android::hardware::boot::V1_0::CommandResult& error)>;
    /**
     * setActiveBootSlot() marks the slot passed in parameter as the active boot
     * slot (see getCurrentSlot for an explanation of the "slot" parameter). This
     * overrides any previous call to setSlotAsUnbootable.
     * Returns whether the command succeeded.
     */
    virtual ::android::hardware::Return<void> setActiveBootSlot(uint32_t slot, setActiveBootSlot_cb _hidl_cb) = 0;

    /**
     * Return callback for setSlotAsUnbootable
     */
    using setSlotAsUnbootable_cb = std::function<void(const ::android::hardware::boot::V1_0::CommandResult& error)>;
    /**
     * setSlotAsUnbootable() marks the slot passed in parameter as
     * an unbootable. This can be used while updating the contents of the slot's
     * partitions, so that the system must not attempt to boot a known bad set up.
     * Returns whether the command succeeded.
     */
    virtual ::android::hardware::Return<void> setSlotAsUnbootable(uint32_t slot, setSlotAsUnbootable_cb _hidl_cb) = 0;

    /**
     * isSlotBootable() returns if the slot passed in parameter is bootable. Note
     * that slots can be made unbootable by both the bootloader and by the OS
     * using setSlotAsUnbootable.
     * Returns TRUE if the slot is bootable, FALSE if it's not, and INVALID_SLOT
     * if slot does not exist.
     */
    virtual ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> isSlotBootable(uint32_t slot) = 0;

    /**
     * isSlotMarkedSucessful() returns if the slot passed in parameter has been
     * marked as successful using markBootSuccessful. Note that only the current
     * slot can be marked as successful but any slot can be queried.
     * Returns TRUE if the slot has been marked as successful, FALSE if it has
     * not, and INVALID_SLOT if the slot does not exist.
     */
    virtual ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> isSlotMarkedSuccessful(uint32_t slot) = 0;

    /**
     * Return callback for getSuffix
     */
    using getSuffix_cb = std::function<void(const ::android::hardware::hidl_string& slotSuffix)>;
    /**
     * getSuffix() returns the string suffix used by partitions that correspond to
     * the slot number passed in as a parameter. The bootloader must pass the
     * suffix of the currently active slot either through a kernel command line
     * property at androidboot.slot_suffix, or the device tree at
     * /firmware/android/slot_suffix.
     * Returns the empty string "" if slot does not match an existing slot.
     */
    virtual ::android::hardware::Return<void> getSuffix(uint32_t slot, getSuffix_cb _hidl_cb) = 0;

    /**
     * Sets whether a snapshot-merge of any dynamic partition is in progress.
     *
     * After the merge status is set to a given value, subsequent calls to
     * getSnapshotMergeStatus must return the set value.
     *
     * The merge status must be persistent across reboots. That is, getSnapshotMergeStatus
     * must return the same value after a reboot if the merge status is not altered in any way
     * (e.g. set by setSnapshotMergeStatus or set to CANCELLED by bootloader).
     *
     * Read/write access to the merge status must be atomic. When the HAL is processing a
     * setSnapshotMergeStatus call, all subsequent calls to getSnapshotMergeStatus must block until
     * setSnapshotMergeStatus has returned.
     *
     * A MERGING state indicates that dynamic partitions are partially comprised by blocks in the
     * userdata partition.
     *
     * When the merge status is set to MERGING, the following operations must be prohibited from the
     * bootloader:
     *  - Flashing or erasing "userdata" or "metadata".
     *
     * The following operations may be prohibited when the status is set to MERGING. If not
     * prohibited, it is recommended that the user receive a warning.
     *  - Changing the active slot (e.g. via "fastboot set_active")
     *
     * @param status Merge status.
     *
     * @return success True on success, false otherwise.
     */
    virtual ::android::hardware::Return<bool> setSnapshotMergeStatus(::android::hardware::boot::V1_1::MergeStatus status) = 0;

    /**
     * Returns whether a snapshot-merge of any dynamic partition is in progress.
     *
     * This function must return the merge status set by the last setSnapshotMergeStatus call and
     * recorded by the bootloader with one exception. If the partitions are being flashed from the
     * bootloader such that the pending merge must be canceled (for example, if the super partition
     * is being flashed), this function must return CANCELLED.
     *
     * @return success True if the merge status is read successfully, false otherwise.
     * @return status Merge status.
     */
    virtual ::android::hardware::Return<::android::hardware::boot::V1_1::MergeStatus> getSnapshotMergeStatus() = 0;

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
    static ::android::hardware::Return<::android::sp<::android::hardware::boot::V1_1::IBootControl>> castFrom(const ::android::sp<::android::hardware::boot::V1_1::IBootControl>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hardware::boot::V1_1::IBootControl>> castFrom(const ::android::sp<::android::hardware::boot::V1_0::IBootControl>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::android::hardware::boot::V1_1::IBootControl>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

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
    static ::android::sp<IBootControl> tryGetService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IBootControl> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IBootControl> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    /**
     * Calls tryGetService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IBootControl> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is not in the VINTF manifest on a Trebilized device, this will return
     * nullptr. If the service is not available, this will wait for the service to
     * become available. If the service is a lazy service, this will start the service
     * and return when it becomes available. If getStub is true, this will try to
     * return an unwrapped passthrough implementation in the same process. This is
     * useful when getting an implementation from the same partition/compilation group.
     */
    static ::android::sp<IBootControl> getService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IBootControl> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IBootControl> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    /**
     * Calls getService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IBootControl> getService(bool getStub) { return getService("default", getStub); }
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

static inline std::string toString(const ::android::sp<::android::hardware::boot::V1_1::IBootControl>& o);

//
// type header definitions for package
//

static inline std::string toString(const ::android::sp<::android::hardware::boot::V1_1::IBootControl>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hardware::boot::V1_1::IBootControl::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V1_1
}  // namespace boot
}  // namespace hardware
}  // namespace android

//
// global type declarations for package
//


#endif  // HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_IBOOTCONTROL_H
