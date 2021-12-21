#define LOG_TAG "android.hardware.boot@1.2::BootControl"

#include <log/log.h>
#include <cutils/trace.h>
#include <hidl/HidlTransportSupport.h>

#include <hidl/Static.h>
#include <hwbinder/ProcessState.h>
#include <utils/Trace.h>
#include <android/hidl/manager/1.0/IServiceManager.h>
#include <android/hardware/boot/1.2/BpHwBootControl.h>
#include <android/hardware/boot/1.2/BnHwBootControl.h>
#include <android/hardware/boot/1.2/BsBootControl.h>
#include <android/hardware/boot/1.1/BpHwBootControl.h>
#include <android/hardware/boot/1.0/BpHwBootControl.h>
#include <android/hidl/base/1.0/BpHwBase.h>
#include <hidl/ServiceManagement.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_2 {

const char* IBootControl::descriptor("android.hardware.boot@1.2::IBootControl");

__attribute__((constructor)) static void static_constructor() {
    ::android::hardware::details::getBnConstructorMap().set(IBootControl::descriptor,
            [](void *iIntf) -> ::android::sp<::android::hardware::IBinder> {
                return new BnHwBootControl(static_cast<IBootControl *>(iIntf));
            });
    ::android::hardware::details::getBsConstructorMap().set(IBootControl::descriptor,
            [](void *iIntf) -> ::android::sp<::android::hidl::base::V1_0::IBase> {
                return new BsBootControl(static_cast<IBootControl *>(iIntf));
            });
}

__attribute__((destructor))static void static_destructor() {
    ::android::hardware::details::getBnConstructorMap().erase(IBootControl::descriptor);
    ::android::hardware::details::getBsConstructorMap().erase(IBootControl::descriptor);
}

// Methods from ::android::hardware::boot::V1_0::IBootControl follow.
// no default implementation for: ::android::hardware::Return<uint32_t> IBootControl::getNumberSlots()
// no default implementation for: ::android::hardware::Return<uint32_t> IBootControl::getCurrentSlot()
// no default implementation for: ::android::hardware::Return<void> IBootControl::markBootSuccessful(markBootSuccessful_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<void> IBootControl::setActiveBootSlot(uint32_t slot, setActiveBootSlot_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<void> IBootControl::setSlotAsUnbootable(uint32_t slot, setSlotAsUnbootable_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> IBootControl::isSlotBootable(uint32_t slot)
// no default implementation for: ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> IBootControl::isSlotMarkedSuccessful(uint32_t slot)
// no default implementation for: ::android::hardware::Return<void> IBootControl::getSuffix(uint32_t slot, getSuffix_cb _hidl_cb)

// Methods from ::android::hardware::boot::V1_1::IBootControl follow.
// no default implementation for: ::android::hardware::Return<bool> IBootControl::setSnapshotMergeStatus(::android::hardware::boot::V1_1::MergeStatus status)
// no default implementation for: ::android::hardware::Return<::android::hardware::boot::V1_1::MergeStatus> IBootControl::getSnapshotMergeStatus()

// Methods from ::android::hardware::boot::V1_2::IBootControl follow.
// no default implementation for: ::android::hardware::Return<uint32_t> IBootControl::getActiveBootSlot()

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IBootControl::interfaceChain(interfaceChain_cb _hidl_cb){
    _hidl_cb({
        ::android::hardware::boot::V1_2::IBootControl::descriptor,
        ::android::hardware::boot::V1_1::IBootControl::descriptor,
        ::android::hardware::boot::V1_0::IBootControl::descriptor,
        ::android::hidl::base::V1_0::IBase::descriptor,
    });
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IBootControl::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IBootControl::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    _hidl_cb(::android::hardware::boot::V1_2::IBootControl::descriptor);
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IBootControl::getHashChain(getHashChain_cb _hidl_cb){
    _hidl_cb({
        (uint8_t[32]){103,99,221,34,115,177,180,127,58,198,138,249,182,104,112,40,126,186,51,251,91,77,102,232,254,29,48,174,24,206,36,203} /* 6763dd2273b1b47f3ac68af9b66870287eba33fb5b4d66e8fe1d30ae18ce24cb */,
        (uint8_t[32]){7,208,162,82,178,216,250,53,136,121,8,169,150,186,57,92,243,146,150,131,149,252,48,175,171,121,31,70,224,194,42,82} /* 07d0a252b2d8fa35887908a996ba395cf392968395fc30afab791f46e0c22a52 */,
        (uint8_t[32]){113,146,215,86,174,186,0,171,163,47,69,4,152,29,248,23,47,252,168,62,33,12,72,56,218,191,41,94,83,233,53,144} /* 7192d756aeba00aba32f4504981df8172ffca83e210c4838dabf295e53e93590 */,
        (uint8_t[32]){236,127,215,158,208,45,250,133,188,73,148,38,173,174,62,190,35,239,5,36,243,205,105,87,19,147,36,184,59,24,202,76} /* ec7fd79ed02dfa85bc499426adae3ebe23ef0524f3cd6957139324b83b18ca4c */});
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IBootControl::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IBootControl::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IBootControl::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IBootControl::getDebugInfo(getDebugInfo_cb _hidl_cb){
    ::android::hidl::base::V1_0::DebugInfo info = {};
    info.pid = -1;
    info.ptr = 0;
    info.arch = 
    #if defined(__LP64__)
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_64BIT
    #else
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_32BIT
    #endif
    ;
    _hidl_cb(info);
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IBootControl::notifySyspropsChanged(){
    ::android::report_sysprop_change();
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IBootControl::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


::android::hardware::Return<::android::sp<::android::hardware::boot::V1_2::IBootControl>> IBootControl::castFrom(const ::android::sp<::android::hardware::boot::V1_2::IBootControl>& parent, bool /* emitError */) {
    return parent;
}

::android::hardware::Return<::android::sp<::android::hardware::boot::V1_2::IBootControl>> IBootControl::castFrom(const ::android::sp<::android::hardware::boot::V1_1::IBootControl>& parent, bool emitError) {
    return ::android::hardware::details::castInterface<IBootControl, ::android::hardware::boot::V1_1::IBootControl, BpHwBootControl>(
            parent, "android.hardware.boot@1.2::IBootControl", emitError);
}

::android::hardware::Return<::android::sp<::android::hardware::boot::V1_2::IBootControl>> IBootControl::castFrom(const ::android::sp<::android::hardware::boot::V1_0::IBootControl>& parent, bool emitError) {
    return ::android::hardware::details::castInterface<IBootControl, ::android::hardware::boot::V1_0::IBootControl, BpHwBootControl>(
            parent, "android.hardware.boot@1.2::IBootControl", emitError);
}

::android::hardware::Return<::android::sp<::android::hardware::boot::V1_2::IBootControl>> IBootControl::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::hardware::details::castInterface<IBootControl, ::android::hidl::base::V1_0::IBase, BpHwBootControl>(
            parent, "android.hardware.boot@1.2::IBootControl", emitError);
}

BpHwBootControl::BpHwBootControl(const ::android::sp<::android::hardware::IBinder> &_hidl_impl)
        : BpInterface<IBootControl>(_hidl_impl),
          ::android::hardware::details::HidlInstrumentor("android.hardware.boot@1.2", "IBootControl") {
}

void BpHwBootControl::onLastStrongRef(const void* id) {
    {
        std::unique_lock<std::mutex> lock(_hidl_mMutex);
        _hidl_mDeathRecipients.clear();
    }

    BpInterface<IBootControl>::onLastStrongRef(id);
}
// Methods from ::android::hardware::boot::V1_2::IBootControl follow.
::android::hardware::Return<uint32_t> BpHwBootControl::_hidl_getActiveBootSlot(::android::hardware::IInterface *_hidl_this, ::android::hardware::details::HidlInstrumentor *_hidl_this_instrumentor) {
    #ifdef __ANDROID_DEBUGGABLE__
    bool mEnableInstrumentation = _hidl_this_instrumentor->isInstrumentationEnabled();
    const auto &mInstrumentationCallbacks = _hidl_this_instrumentor->getInstrumentationCallbacks();
    #else
    (void) _hidl_this_instrumentor;
    #endif // __ANDROID_DEBUGGABLE__
    ::android::ScopedTrace PASTE(___tracer, __LINE__) (ATRACE_TAG_HAL, "HIDL::IBootControl::getActiveBootSlot::client");
    #ifdef __ANDROID_DEBUGGABLE__
    if (UNLIKELY(mEnableInstrumentation)) {
        std::vector<void *> _hidl_args;
        for (const auto &callback: mInstrumentationCallbacks) {
            callback(InstrumentationEvent::CLIENT_API_ENTRY, "android.hardware.boot", "1.2", "IBootControl", "getActiveBootSlot", &_hidl_args);
        }
    }
    #endif // __ANDROID_DEBUGGABLE__

    ::android::hardware::Parcel _hidl_data;
    ::android::hardware::Parcel _hidl_reply;
    ::android::status_t _hidl_err;
    ::android::status_t _hidl_transact_err;
    ::android::hardware::Status _hidl_status;

    uint32_t _hidl_out_slot;

    _hidl_err = _hidl_data.writeInterfaceToken(BpHwBootControl::descriptor);
    if (_hidl_err != ::android::OK) { goto _hidl_error; }

    _hidl_transact_err = ::android::hardware::IInterface::asBinder(_hidl_this)->transact(11 /* getActiveBootSlot */, _hidl_data, &_hidl_reply, 0 /* flags */);
    if (_hidl_transact_err != ::android::OK) 
    {
        _hidl_err = _hidl_transact_err;
        goto _hidl_error;
    }

    _hidl_err = ::android::hardware::readFromParcel(&_hidl_status, _hidl_reply);
    if (_hidl_err != ::android::OK) { goto _hidl_error; }

    if (!_hidl_status.isOk()) { return _hidl_status; }

    _hidl_err = _hidl_reply.readUint32(&_hidl_out_slot);
    if (_hidl_err != ::android::OK) { goto _hidl_error; }

    #ifdef __ANDROID_DEBUGGABLE__
    if (UNLIKELY(mEnableInstrumentation)) {
        std::vector<void *> _hidl_args;
        _hidl_args.push_back((void *)&_hidl_out_slot);
        for (const auto &callback: mInstrumentationCallbacks) {
            callback(InstrumentationEvent::CLIENT_API_EXIT, "android.hardware.boot", "1.2", "IBootControl", "getActiveBootSlot", &_hidl_args);
        }
    }
    #endif // __ANDROID_DEBUGGABLE__

    return ::android::hardware::Return<uint32_t>(_hidl_out_slot);

_hidl_error:
    _hidl_status.setFromStatusT(_hidl_err);
    return ::android::hardware::Return<uint32_t>(_hidl_status);
}


// Methods from ::android::hardware::boot::V1_0::IBootControl follow.
::android::hardware::Return<uint32_t> BpHwBootControl::getNumberSlots(){
    ::android::hardware::Return<uint32_t>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_getNumberSlots(this, this);

    return _hidl_out;
}

::android::hardware::Return<uint32_t> BpHwBootControl::getCurrentSlot(){
    ::android::hardware::Return<uint32_t>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_getCurrentSlot(this, this);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::markBootSuccessful(markBootSuccessful_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_markBootSuccessful(this, this, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::setActiveBootSlot(uint32_t slot, setActiveBootSlot_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_setActiveBootSlot(this, this, slot, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::setSlotAsUnbootable(uint32_t slot, setSlotAsUnbootable_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_setSlotAsUnbootable(this, this, slot, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> BpHwBootControl::isSlotBootable(uint32_t slot){
    ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_isSlotBootable(this, this, slot);

    return _hidl_out;
}

::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult> BpHwBootControl::isSlotMarkedSuccessful(uint32_t slot){
    ::android::hardware::Return<::android::hardware::boot::V1_0::BoolResult>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_isSlotMarkedSuccessful(this, this, slot);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::getSuffix(uint32_t slot, getSuffix_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hardware::boot::V1_0::BpHwBootControl::_hidl_getSuffix(this, this, slot, _hidl_cb);

    return _hidl_out;
}


// Methods from ::android::hardware::boot::V1_1::IBootControl follow.
::android::hardware::Return<bool> BpHwBootControl::setSnapshotMergeStatus(::android::hardware::boot::V1_1::MergeStatus status){
    ::android::hardware::Return<bool>  _hidl_out = ::android::hardware::boot::V1_1::BpHwBootControl::_hidl_setSnapshotMergeStatus(this, this, status);

    return _hidl_out;
}

::android::hardware::Return<::android::hardware::boot::V1_1::MergeStatus> BpHwBootControl::getSnapshotMergeStatus(){
    ::android::hardware::Return<::android::hardware::boot::V1_1::MergeStatus>  _hidl_out = ::android::hardware::boot::V1_1::BpHwBootControl::_hidl_getSnapshotMergeStatus(this, this);

    return _hidl_out;
}


// Methods from ::android::hardware::boot::V1_2::IBootControl follow.
::android::hardware::Return<uint32_t> BpHwBootControl::getActiveBootSlot(){
    ::android::hardware::Return<uint32_t>  _hidl_out = ::android::hardware::boot::V1_2::BpHwBootControl::_hidl_getActiveBootSlot(this, this);

    return _hidl_out;
}


// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> BpHwBootControl::interfaceChain(interfaceChain_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_interfaceChain(this, this, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_debug(this, this, fd, options);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_interfaceDescriptor(this, this, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::getHashChain(getHashChain_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_getHashChain(this, this, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::setHALInstrumentation(){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_setHALInstrumentation(this, this);

    return _hidl_out;
}

::android::hardware::Return<bool> BpHwBootControl::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    ::android::hardware::ProcessState::self()->startThreadPool();
    ::android::hardware::hidl_binder_death_recipient *binder_recipient = new ::android::hardware::hidl_binder_death_recipient(recipient, cookie, this);
    std::unique_lock<std::mutex> lock(_hidl_mMutex);
    _hidl_mDeathRecipients.push_back(binder_recipient);
    return (remote()->linkToDeath(binder_recipient) == ::android::OK);
}

::android::hardware::Return<void> BpHwBootControl::ping(){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_ping(this, this);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::getDebugInfo(getDebugInfo_cb _hidl_cb){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_getDebugInfo(this, this, _hidl_cb);

    return _hidl_out;
}

::android::hardware::Return<void> BpHwBootControl::notifySyspropsChanged(){
    ::android::hardware::Return<void>  _hidl_out = ::android::hidl::base::V1_0::BpHwBase::_hidl_notifySyspropsChanged(this, this);

    return _hidl_out;
}

::android::hardware::Return<bool> BpHwBootControl::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    std::unique_lock<std::mutex> lock(_hidl_mMutex);
    for (auto it = _hidl_mDeathRecipients.rbegin();it != _hidl_mDeathRecipients.rend();++it) {
        if ((*it)->getRecipient() == recipient) {
            ::android::status_t status = remote()->unlinkToDeath(*it);
            _hidl_mDeathRecipients.erase(it.base()-1);
            return status == ::android::OK;
        }
    }
    return false;
}


BnHwBootControl::BnHwBootControl(const ::android::sp<IBootControl> &_hidl_impl)
        : ::android::hidl::base::V1_0::BnHwBase(_hidl_impl, "android.hardware.boot@1.2", "IBootControl") { 
            _hidl_mImpl = _hidl_impl;
            auto prio = ::android::hardware::getMinSchedulerPolicy(_hidl_impl);
            mSchedPolicy = prio.sched_policy;
            mSchedPriority = prio.prio;
            setRequestingSid(::android::hardware::getRequestingSid(_hidl_impl));
}

BnHwBootControl::~BnHwBootControl() {
    ::android::hardware::details::gBnMap->eraseIfEqual(_hidl_mImpl.get(), this);
}

// Methods from ::android::hardware::boot::V1_2::IBootControl follow.
::android::status_t BnHwBootControl::_hidl_getActiveBootSlot(
        ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
        const ::android::hardware::Parcel &_hidl_data,
        ::android::hardware::Parcel *_hidl_reply,
        TransactCallback _hidl_cb) {
    #ifdef __ANDROID_DEBUGGABLE__
    bool mEnableInstrumentation = _hidl_this->isInstrumentationEnabled();
    const auto &mInstrumentationCallbacks = _hidl_this->getInstrumentationCallbacks();
    #endif // __ANDROID_DEBUGGABLE__

    ::android::status_t _hidl_err = ::android::OK;
    if (!_hidl_data.enforceInterface(BnHwBootControl::Pure::descriptor)) {
        _hidl_err = ::android::BAD_TYPE;
        return _hidl_err;
    }

    atrace_begin(ATRACE_TAG_HAL, "HIDL::IBootControl::getActiveBootSlot::server");
    #ifdef __ANDROID_DEBUGGABLE__
    if (UNLIKELY(mEnableInstrumentation)) {
        std::vector<void *> _hidl_args;
        for (const auto &callback: mInstrumentationCallbacks) {
            callback(InstrumentationEvent::SERVER_API_ENTRY, "android.hardware.boot", "1.2", "IBootControl", "getActiveBootSlot", &_hidl_args);
        }
    }
    #endif // __ANDROID_DEBUGGABLE__

    uint32_t _hidl_out_slot = static_cast<IBootControl*>(_hidl_this->getImpl().get())->getActiveBootSlot();

    ::android::hardware::writeToParcel(::android::hardware::Status::ok(), _hidl_reply);

    _hidl_err = _hidl_reply->writeUint32(_hidl_out_slot);
    if (_hidl_err != ::android::OK) { goto _hidl_error; }

_hidl_error:
    atrace_end(ATRACE_TAG_HAL);
    #ifdef __ANDROID_DEBUGGABLE__
    if (UNLIKELY(mEnableInstrumentation)) {
        std::vector<void *> _hidl_args;
        _hidl_args.push_back((void *)&_hidl_out_slot);
        for (const auto &callback: mInstrumentationCallbacks) {
            callback(InstrumentationEvent::SERVER_API_EXIT, "android.hardware.boot", "1.2", "IBootControl", "getActiveBootSlot", &_hidl_args);
        }
    }
    #endif // __ANDROID_DEBUGGABLE__

    if (_hidl_err != ::android::OK) { return _hidl_err; }
    _hidl_cb(*_hidl_reply);
    return _hidl_err;
}


// Methods from ::android::hardware::boot::V1_0::IBootControl follow.

// Methods from ::android::hardware::boot::V1_1::IBootControl follow.

// Methods from ::android::hardware::boot::V1_2::IBootControl follow.

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> BnHwBootControl::ping() {
    return ::android::hardware::Void();
}
::android::hardware::Return<void> BnHwBootControl::getDebugInfo(getDebugInfo_cb _hidl_cb) {
    ::android::hidl::base::V1_0::DebugInfo info = {};
    info.pid = ::android::hardware::details::getPidIfSharable();
    info.ptr = ::android::hardware::details::debuggable()? reinterpret_cast<uint64_t>(this) : 0;
    info.arch = 
    #if defined(__LP64__)
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_64BIT
    #else
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_32BIT
    #endif
    ;
    _hidl_cb(info);
    return ::android::hardware::Void();
}

::android::status_t BnHwBootControl::onTransact(
        uint32_t _hidl_code,
        const ::android::hardware::Parcel &_hidl_data,
        ::android::hardware::Parcel *_hidl_reply,
        uint32_t _hidl_flags,
        TransactCallback _hidl_cb) {
    ::android::status_t _hidl_err = ::android::OK;

    switch (_hidl_code) {
        case 1 /* getNumberSlots */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_getNumberSlots(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 2 /* getCurrentSlot */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_getCurrentSlot(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 3 /* markBootSuccessful */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_markBootSuccessful(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 4 /* setActiveBootSlot */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_setActiveBootSlot(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 5 /* setSlotAsUnbootable */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_setSlotAsUnbootable(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 6 /* isSlotBootable */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_isSlotBootable(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 7 /* isSlotMarkedSuccessful */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_isSlotMarkedSuccessful(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 8 /* getSuffix */:
        {
            _hidl_err = ::android::hardware::boot::V1_0::BnHwBootControl::_hidl_getSuffix(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 9 /* setSnapshotMergeStatus */:
        {
            _hidl_err = ::android::hardware::boot::V1_1::BnHwBootControl::_hidl_setSnapshotMergeStatus(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 10 /* getSnapshotMergeStatus */:
        {
            _hidl_err = ::android::hardware::boot::V1_1::BnHwBootControl::_hidl_getSnapshotMergeStatus(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        case 11 /* getActiveBootSlot */:
        {
            _hidl_err = ::android::hardware::boot::V1_2::BnHwBootControl::_hidl_getActiveBootSlot(this, _hidl_data, _hidl_reply, _hidl_cb);
            break;
        }

        default:
        {
            return ::android::hidl::base::V1_0::BnHwBase::onTransact(
                    _hidl_code, _hidl_data, _hidl_reply, _hidl_flags, _hidl_cb);
        }
    }

    if (_hidl_err == ::android::UNEXPECTED_NULL) {
        _hidl_err = ::android::hardware::writeToParcel(
                ::android::hardware::Status::fromExceptionCode(::android::hardware::Status::EX_NULL_POINTER),
                _hidl_reply);
    }return _hidl_err;
}

BsBootControl::BsBootControl(const ::android::sp<::android::hardware::boot::V1_2::IBootControl> impl) : ::android::hardware::details::HidlInstrumentor("android.hardware.boot@1.2", "IBootControl"), mImpl(impl) {
    mOnewayQueue.start(3000 /* similar limit to binderized */);
}

::android::hardware::Return<void> BsBootControl::addOnewayTask(std::function<void(void)> fun) {
    if (!mOnewayQueue.push(fun)) {
        return ::android::hardware::Status::fromExceptionCode(
                ::android::hardware::Status::EX_TRANSACTION_FAILED,
                "Passthrough oneway function queue exceeds maximum size.");
    }
    return ::android::hardware::Status();
}

::android::sp<IBootControl> IBootControl::tryGetService(const std::string &serviceName, const bool getStub) {
    return ::android::hardware::details::getServiceInternal<BpHwBootControl>(serviceName, false, getStub);
}

::android::sp<IBootControl> IBootControl::getService(const std::string &serviceName, const bool getStub) {
    return ::android::hardware::details::getServiceInternal<BpHwBootControl>(serviceName, true, getStub);
}

::android::status_t IBootControl::registerAsService(const std::string &serviceName) {
    return ::android::hardware::details::registerAsServiceInternal(this, serviceName);
}

bool IBootControl::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    const ::android::sp<::android::hidl::manager::V1_0::IServiceManager> sm
            = ::android::hardware::defaultServiceManager();
    if (sm == nullptr) {
        return false;
    }
    ::android::hardware::Return<bool> success =
            sm->registerForNotifications("android.hardware.boot@1.2::IBootControl",
                    serviceName, notification);
    return success.isOk() && success;
}

static_assert(sizeof(::android::hardware::MQDescriptor<char, ::android::hardware::kSynchronizedReadWrite>) == 32, "wrong size");
static_assert(sizeof(::android::hardware::hidl_handle) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_memory) == 40, "wrong size");
static_assert(sizeof(::android::hardware::hidl_string) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_vec<char>) == 16, "wrong size");

}  // namespace V1_2
}  // namespace boot
}  // namespace hardware
}  // namespace android
