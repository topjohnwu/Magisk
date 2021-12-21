#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BPHWCLIENTCALLBACK_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BPHWCLIENTCALLBACK_H

#include <hidl/HidlTransportSupport.h>

#include <android/hidl/manager/1.2/IHwClientCallback.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_2 {

struct BpHwClientCallback : public ::android::hardware::BpInterface<IClientCallback>, public ::android::hardware::details::HidlInstrumentor {
    explicit BpHwClientCallback(const ::android::sp<::android::hardware::IBinder> &_hidl_impl);

    /**
     * The pure class is what this class wraps.
     */
    typedef IClientCallback Pure;

    /**
     * Type tag for use in template logic that indicates this is a 'proxy' class.
     */
    typedef ::android::hardware::details::bphw_tag _hidl_tag;

    virtual bool isRemote() const override { return true; }

    void onLastStrongRef(const void* id) override;

    // Methods from ::android::hidl::manager::V1_2::IClientCallback follow.
    static ::android::hardware::Return<void>  _hidl_onClients(::android::hardware::IInterface* _hidl_this, ::android::hardware::details::HidlInstrumentor *_hidl_this_instrumentor, const ::android::sp<::android::hidl::base::V1_0::IBase>& registered, bool hasClients);

    // Methods from ::android::hidl::manager::V1_2::IClientCallback follow.
    ::android::hardware::Return<void> onClients(const ::android::sp<::android::hidl::base::V1_0::IBase>& registered, bool hasClients) override;

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

}  // namespace V1_2
}  // namespace manager
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BPHWCLIENTCALLBACK_H
