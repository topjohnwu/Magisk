#ifndef HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_BPHWBOOTCONTROL_H
#define HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_BPHWBOOTCONTROL_H

#include <hidl/HidlTransportSupport.h>

#include <android/hardware/boot/1.1/IHwBootControl.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_1 {

struct BpHwBootControl : public ::android::hardware::BpInterface<IBootControl>, public ::android::hardware::details::HidlInstrumentor {
    explicit BpHwBootControl(const ::android::sp<::android::hardware::IBinder> &_hidl_impl);

    /**
     * The pure class is what this class wraps.
     */
    typedef IBootControl Pure;

    /**
     * Type tag for use in template logic that indicates this is a 'proxy' class.
     */
    typedef ::android::hardware::details::bphw_tag _hidl_tag;

    virtual bool isRemote() const override { return true; }

    void onLastStrongRef(const void* id) override;

    // Methods from ::android::hardware::boot::V1_1::IBootControl follow.
    static ::android::hardware::Return<bool>  _hidl_setSnapshotMergeStatus(::android::hardware::IInterface* _hidl_this, ::android::hardware::details::HidlInstrumentor *_hidl_this_instrumentor, ::android::hardware::boot::V1_1::MergeStatus status);
    static ::android::hardware::Return<::android::hardware::boot::V1_1::MergeStatus>  _hidl_getSnapshotMergeStatus(::android::hardware::IInterface* _hidl_this, ::android::hardware::details::HidlInstrumentor *_hidl_this_instrumentor);

    // Methods from ::android::hardware::boot::V1_0::IBootControl follow.
    ::android::hardware::Return<uint32_t> getNumberSlots() override;
    ::android::hardware::Return<uint32_t> getCurrentSlot() override;
    ::android::hardware::Return<void> markBootSuccessful(markBootSuccessful_cb _hidl_cb) override;
    ::android::hardware::Return<void> setActiveBootSlot(uint32_t slot, setActiveBootSlot_cb _hidl_cb) override;
    ::android::hardware::Return<void> setSlotAsUnbootable(uint32_t slot, setSlotAsUnbootable_cb _hidl_cb) override;
    ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> isSlotBootable(uint32_t slot) override;
    ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> isSlotMarkedSuccessful(uint32_t slot) override;
    ::android::hardware::Return<void> getSuffix(uint32_t slot, getSuffix_cb _hidl_cb) override;

    // Methods from ::android::hardware::boot::V1_1::IBootControl follow.
    ::android::hardware::Return<bool> setSnapshotMergeStatus(::android::hardware::boot::V1_1::MergeStatus status) override;
    ::android::hardware::Return<::android::hardware::boot::V1_1::MergeStatus> getSnapshotMergeStatus() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;
    ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;
    ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> setHALInstrumentation() override;
    ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;
    ::android::hardware::Return<void> ping() override;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;
    ::android::hardware::Return<void> notifySyspropsChanged() override;
    ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;

private:
    std::mutex _hidl_mMutex;
    std::vector<::android::sp<::android::hardware::hidl_binder_death_recipient>> _hidl_mDeathRecipients;
};

}  // namespace V1_1
}  // namespace boot
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_BPHWBOOTCONTROL_H
