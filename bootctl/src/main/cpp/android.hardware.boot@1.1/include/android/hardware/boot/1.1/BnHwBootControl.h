#ifndef HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_BNHWBOOTCONTROL_H
#define HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_BNHWBOOTCONTROL_H

#include <android/hardware/boot/1.1/IHwBootControl.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_1 {

struct BnHwBootControl : public ::android::hidl::base::V1_0::BnHwBase {
    explicit BnHwBootControl(const ::android::sp<IBootControl> &_hidl_impl);
    explicit BnHwBootControl(const ::android::sp<IBootControl> &_hidl_impl, const std::string& HidlInstrumentor_package, const std::string& HidlInstrumentor_interface);

    virtual ~BnHwBootControl();

    ::android::status_t onTransact(
            uint32_t _hidl_code,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            uint32_t _hidl_flags = 0,
            TransactCallback _hidl_cb = nullptr) override;


    /**
     * The pure class is what this class wraps.
     */
    typedef IBootControl Pure;

    /**
     * Type tag for use in template logic that indicates this is a 'native' class.
     */
    typedef ::android::hardware::details::bnhw_tag _hidl_tag;

    ::android::sp<IBootControl> getImpl() { return _hidl_mImpl; }
    // Methods from ::android::hardware::boot::V1_1::IBootControl follow.
    static ::android::status_t _hidl_setSnapshotMergeStatus(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);


    static ::android::status_t _hidl_getSnapshotMergeStatus(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);



private:
    // Methods from ::android::hardware::boot::V1_0::IBootControl follow.

    // Methods from ::android::hardware::boot::V1_1::IBootControl follow.

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> ping();
    using getDebugInfo_cb = ::android::hidl::base::V1_0::IBase::getDebugInfo_cb;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb);

    ::android::sp<IBootControl> _hidl_mImpl;
};

}  // namespace V1_1
}  // namespace boot
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_BNHWBOOTCONTROL_H
