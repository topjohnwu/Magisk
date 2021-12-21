#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BNHWCLIENTCALLBACK_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BNHWCLIENTCALLBACK_H

#include <android/hidl/manager/1.2/IHwClientCallback.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_2 {

struct BnHwClientCallback : public ::android::hidl::base::V1_0::BnHwBase {
    explicit BnHwClientCallback(const ::android::sp<IClientCallback> &_hidl_impl);
    explicit BnHwClientCallback(const ::android::sp<IClientCallback> &_hidl_impl, const std::string& HidlInstrumentor_package, const std::string& HidlInstrumentor_interface);

    virtual ~BnHwClientCallback();

    ::android::status_t onTransact(
            uint32_t _hidl_code,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            uint32_t _hidl_flags = 0,
            TransactCallback _hidl_cb = nullptr) override;


    /**
     * The pure class is what this class wraps.
     */
    typedef IClientCallback Pure;

    /**
     * Type tag for use in template logic that indicates this is a 'native' class.
     */
    typedef ::android::hardware::details::bnhw_tag _hidl_tag;

    ::android::sp<IClientCallback> getImpl() { return _hidl_mImpl; }
    // Methods from ::android::hidl::manager::V1_2::IClientCallback follow.
    static ::android::status_t _hidl_onClients(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);



private:
    // Methods from ::android::hidl::manager::V1_2::IClientCallback follow.

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> ping();
    using getDebugInfo_cb = ::android::hidl::base::V1_0::IBase::getDebugInfo_cb;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb);

    ::android::sp<IClientCallback> _hidl_mImpl;
};

}  // namespace V1_2
}  // namespace manager
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BNHWCLIENTCALLBACK_H
