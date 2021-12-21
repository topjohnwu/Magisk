#ifndef HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_0_HWTYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_0_HWTYPES_H

#include <android/hardware/boot/1.0/types.h>


#include <hidl/Status.h>
#include <hwbinder/IBinder.h>
#include <hwbinder/Parcel.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_0 {
::android::status_t readEmbeddedFromParcel(
        const ::android::hardware::boot::V1_0::CommandResult &obj,
        const ::android::hardware::Parcel &parcel,
        size_t parentHandle,
        size_t parentOffset);

::android::status_t writeEmbeddedToParcel(
        const ::android::hardware::boot::V1_0::CommandResult &obj,
        ::android::hardware::Parcel *parcel,
        size_t parentHandle,
        size_t parentOffset);

}  // namespace V1_0
}  // namespace boot
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_0_HWTYPES_H
