#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_1_BPHWSERVICEMANAGER_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_1_BPHWSERVICEMANAGER_H

#include <hidl/HidlTransportSupport.h>

#include <android/hidl/manager/1.1/IHwServiceManager.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_1 {

struct BpHwServiceManager : public ::android::hardware::BpInterface<IServiceManager>, public ::android::hardware::details::HidlInstrumentor {
    explicit BpHwServiceManager(const ::android::sp<::android::hardware::IBinder> &_hidl_impl);

    /**
     * The pure class is what this class wraps.
     */
    typedef IServiceManager Pure;

    /**
     * Type tag for use in template logic that indicates this is a 'proxy' class.
     */
    typedef ::android::hardware::details::bphw_tag _hidl_tag;

    virtual bool isRemote() const override { return true; }

    void onLastStrongRef(const void* id) override;

    // Methods from ::android::hidl::manager::V1_1::IServiceManager follow.
    static ::android::hardware::Return<bool>  _hidl_unregisterForNotifications(::android::hardware::IInterface* _hidl_this, ::android::hardware::details::HidlInstrumentor *_hidl_this_instrumentor, const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification>& callback);

    // Methods from ::android::hidl::manager::V1_0::IServiceManager follow.
    ::android::hardware::Return<::android::sp<::android::hidl::base::V1_0::IBase>> get(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name) override;
    ::android::hardware::Return<bool> add(const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::base::V1_0::IBase>& service) override;
    ::android::hardware::Return<::android::hidl::manager::V1_0::IServiceManager::Transport> getTransport(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name) override;
    ::android::hardware::Return<void> list(list_cb _hidl_cb) override;
    ::android::hardware::Return<void> listByInterface(const ::android::hardware::hidl_string& fqName, listByInterface_cb _hidl_cb) override;
    ::android::hardware::Return<bool> registerForNotifications(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification>& callback) override;
    ::android::hardware::Return<void> debugDump(debugDump_cb _hidl_cb) override;
    ::android::hardware::Return<void> registerPassthroughClient(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name) override;

    // Methods from ::android::hidl::manager::V1_1::IServiceManager follow.
    ::android::hardware::Return<bool> unregisterForNotifications(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification>& callback) override;

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
}  // namespace manager
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_1_BPHWSERVICEMANAGER_H
