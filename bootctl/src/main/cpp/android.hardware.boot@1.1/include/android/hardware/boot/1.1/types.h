#ifndef HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_1 {

// Forward declaration for forward reference support:
enum class MergeStatus : int32_t;

enum class MergeStatus : int32_t {
    /**
     * No snapshot or merge is in progress.
     */
    NONE = 0,
    /**
     * The merge status could not be determined.
     */
    UNKNOWN = 1 /* ::android::hardware::boot::V1_1::MergeStatus.NONE implicitly + 1 */,
    /**
     * Partitions are being snapshotted, but no merge has been started.
     */
    SNAPSHOTTED = 2 /* ::android::hardware::boot::V1_1::MergeStatus.UNKNOWN implicitly + 1 */,
    /**
     * At least one partition has merge is in progress.
     */
    MERGING = 3 /* ::android::hardware::boot::V1_1::MergeStatus.SNAPSHOTTED implicitly + 1 */,
    /**
     * A merge was in progress, but it was canceled by the bootloader.
     */
    CANCELLED = 4 /* ::android::hardware::boot::V1_1::MergeStatus.MERGING implicitly + 1 */,
};

//
// type declarations for package
//

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::boot::V1_1::MergeStatus o);
static inline void PrintTo(::android::hardware::boot::V1_1::MergeStatus o, ::std::ostream* os);
constexpr int32_t operator|(const ::android::hardware::boot::V1_1::MergeStatus lhs, const ::android::hardware::boot::V1_1::MergeStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::boot::V1_1::MergeStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::boot::V1_1::MergeStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::boot::V1_1::MergeStatus lhs, const ::android::hardware::boot::V1_1::MergeStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::boot::V1_1::MergeStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::boot::V1_1::MergeStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::boot::V1_1::MergeStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::boot::V1_1::MergeStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

//
// type header definitions for package
//

template<>
inline std::string toString<::android::hardware::boot::V1_1::MergeStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::boot::V1_1::MergeStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::boot::V1_1::MergeStatus::NONE) == static_cast<int32_t>(::android::hardware::boot::V1_1::MergeStatus::NONE)) {
        os += (first ? "" : " | ");
        os += "NONE";
        first = false;
        flipped |= ::android::hardware::boot::V1_1::MergeStatus::NONE;
    }
    if ((o & ::android::hardware::boot::V1_1::MergeStatus::UNKNOWN) == static_cast<int32_t>(::android::hardware::boot::V1_1::MergeStatus::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::android::hardware::boot::V1_1::MergeStatus::UNKNOWN;
    }
    if ((o & ::android::hardware::boot::V1_1::MergeStatus::SNAPSHOTTED) == static_cast<int32_t>(::android::hardware::boot::V1_1::MergeStatus::SNAPSHOTTED)) {
        os += (first ? "" : " | ");
        os += "SNAPSHOTTED";
        first = false;
        flipped |= ::android::hardware::boot::V1_1::MergeStatus::SNAPSHOTTED;
    }
    if ((o & ::android::hardware::boot::V1_1::MergeStatus::MERGING) == static_cast<int32_t>(::android::hardware::boot::V1_1::MergeStatus::MERGING)) {
        os += (first ? "" : " | ");
        os += "MERGING";
        first = false;
        flipped |= ::android::hardware::boot::V1_1::MergeStatus::MERGING;
    }
    if ((o & ::android::hardware::boot::V1_1::MergeStatus::CANCELLED) == static_cast<int32_t>(::android::hardware::boot::V1_1::MergeStatus::CANCELLED)) {
        os += (first ? "" : " | ");
        os += "CANCELLED";
        first = false;
        flipped |= ::android::hardware::boot::V1_1::MergeStatus::CANCELLED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::boot::V1_1::MergeStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::boot::V1_1::MergeStatus::NONE) {
        return "NONE";
    }
    if (o == ::android::hardware::boot::V1_1::MergeStatus::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::android::hardware::boot::V1_1::MergeStatus::SNAPSHOTTED) {
        return "SNAPSHOTTED";
    }
    if (o == ::android::hardware::boot::V1_1::MergeStatus::MERGING) {
        return "MERGING";
    }
    if (o == ::android::hardware::boot::V1_1::MergeStatus::CANCELLED) {
        return "CANCELLED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline void PrintTo(::android::hardware::boot::V1_1::MergeStatus o, ::std::ostream* os) {
    *os << toString(o);
}


}  // namespace V1_1
}  // namespace boot
}  // namespace hardware
}  // namespace android

//
// global type declarations for package
//

namespace android {
namespace hardware {
namespace details {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
template<> inline constexpr std::array<::android::hardware::boot::V1_1::MergeStatus, 5> hidl_enum_values<::android::hardware::boot::V1_1::MergeStatus> = {
    ::android::hardware::boot::V1_1::MergeStatus::NONE,
    ::android::hardware::boot::V1_1::MergeStatus::UNKNOWN,
    ::android::hardware::boot::V1_1::MergeStatus::SNAPSHOTTED,
    ::android::hardware::boot::V1_1::MergeStatus::MERGING,
    ::android::hardware::boot::V1_1::MergeStatus::CANCELLED,
};
#pragma clang diagnostic pop
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_1_TYPES_H
