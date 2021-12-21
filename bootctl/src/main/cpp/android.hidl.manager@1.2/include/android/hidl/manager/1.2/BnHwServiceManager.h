#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BNHWSERVICEMANAGER_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BNHWSERVICEMANAGER_H

#include <android/hidl/manager/1.2/IHwServiceManager.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_2 {

struct BnHwServiceManager : public ::android::hidl::base::V1_0::BnHwBase {
    explicit BnHwServiceManager(const ::android::sp<IServiceManager> &_hidl_impl);
    explicit BnHwServiceManager(const ::android::sp<IServiceManager> &_hidl_impl, const std::string& HidlInstrumentor_package, const std::string& HidlInstrumentor_interface);

    virtual ~BnHwServiceManager();

    ::android::status_t onTransact(
            uint32_t _hidl_code,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            uint32_t _hidl_flags = 0,
            TransactCallback _hidl_cb = nullptr) override;


    /**
     * The pure class is what this class wraps.
     */
    typedef IServiceManager Pure;

    /**
     * Type tag for use in template logic that indicates this is a 'native' class.
     */
    typedef ::android::hardware::details::bnhw_tag _hidl_tag;

    ::android::sp<IServiceManager> getImpl() { return _hidl_mImpl; }
    // Methods from ::android::hidl::manager::V1_2::IServiceManager follow.
    static ::android::status_t _hidl_registerClientCallback(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);


    static ::android::status_t _hidl_unregisterClientCallback(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);


    static ::android::status_t _hidl_addWithChain(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);


    static ::android::status_t _hidl_listManifestByInterface(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);


    static ::android::status_t _hidl_tryUnregister(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);



private:
    // Methods from ::android::hidl::manager::V1_0::IServiceManager follow.

    // Methods from ::android::hidl::manager::V1_1::IServiceManager follow.

    // Methods from ::android::hidl::manager::V1_2::IServiceManager follow.

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> ping();
    using getDebugInfo_cb = ::android::hidl::base::V1_0::IBase::getDebugInfo_cb;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb);

    ::android::sp<IServiceManager> _hidl_mImpl;
};

}  // namespace V1_2
}  // namespace manager
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_2_BNHWSERVICEMANAGER_H
