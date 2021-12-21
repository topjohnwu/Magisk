#ifndef HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_0_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace boot {
namespace V1_0 {

// Forward declaration for forward reference support:
struct CommandResult;
enum class BoolResult : int32_t;

/**
 * A command result encapsulating whether the command succeeded and
 * an error string.
 */
struct CommandResult final {
    bool success __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string errMsg __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::boot::V1_0::CommandResult, success) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::boot::V1_0::CommandResult, errMsg) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::boot::V1_0::CommandResult) == 24, "wrong size");
static_assert(__alignof(::android::hardware::boot::V1_0::CommandResult) == 8, "wrong alignment");

/**
 * An identifier for a slot number.
 */
typedef uint32_t Slot;

/**
 * A result encapsulating whether a function returned true, false or
 * failed due to an invalid slot number
 */
enum class BoolResult : int32_t {
    FALSE = 0,
    TRUE = 1,
    INVALID_SLOT = -1 /* -1 */,
};

//
// type declarations for package
//

static inline std::string toString(const ::android::hardware::boot::V1_0::CommandResult& o);
static inline void PrintTo(const ::android::hardware::boot::V1_0::CommandResult& o, ::std::ostream*);
static inline bool operator==(const ::android::hardware::boot::V1_0::CommandResult& lhs, const ::android::hardware::boot::V1_0::CommandResult& rhs);
static inline bool operator!=(const ::android::hardware::boot::V1_0::CommandResult& lhs, const ::android::hardware::boot::V1_0::CommandResult& rhs);

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::boot::V1_0::BoolResult o);
static inline void PrintTo(::android::hardware::boot::V1_0::BoolResult o, ::std::ostream* os);
constexpr int32_t operator|(const ::android::hardware::boot::V1_0::BoolResult lhs, const ::android::hardware::boot::V1_0::BoolResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::boot::V1_0::BoolResult rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::boot::V1_0::BoolResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::boot::V1_0::BoolResult lhs, const ::android::hardware::boot::V1_0::BoolResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::boot::V1_0::BoolResult rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::boot::V1_0::BoolResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::boot::V1_0::BoolResult e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::boot::V1_0::BoolResult e) {
    v &= static_cast<int32_t>(e);
    return v;
}

//
// type header definitions for package
//

static inline std::string toString(const ::android::hardware::boot::V1_0::CommandResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".success = ";
    os += ::android::hardware::toString(o.success);
    os += ", .errMsg = ";
    os += ::android::hardware::toString(o.errMsg);
    os += "}"; return os;
}

static inline void PrintTo(const ::android::hardware::boot::V1_0::CommandResult& o, ::std::ostream* os) {
    *os << toString(o);
}

static inline bool operator==(const ::android::hardware::boot::V1_0::CommandResult& lhs, const ::android::hardware::boot::V1_0::CommandResult& rhs) {
    if (lhs.success != rhs.success) {
        return false;
    }
    if (lhs.errMsg != rhs.errMsg) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::boot::V1_0::CommandResult& lhs, const ::android::hardware::boot::V1_0::CommandResult& rhs){
    return !(lhs == rhs);
}

template<>
inline std::string toString<::android::hardware::boot::V1_0::BoolResult>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::boot::V1_0::BoolResult> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::boot::V1_0::BoolResult::FALSE) == static_cast<int32_t>(::android::hardware::boot::V1_0::BoolResult::FALSE)) {
        os += (first ? "" : " | ");
        os += "FALSE";
        first = false;
        flipped |= ::android::hardware::boot::V1_0::BoolResult::FALSE;
    }
    if ((o & ::android::hardware::boot::V1_0::BoolResult::TRUE) == static_cast<int32_t>(::android::hardware::boot::V1_0::BoolResult::TRUE)) {
        os += (first ? "" : " | ");
        os += "TRUE";
        first = false;
        flipped |= ::android::hardware::boot::V1_0::BoolResult::TRUE;
    }
    if ((o & ::android::hardware::boot::V1_0::BoolResult::INVALID_SLOT) == static_cast<int32_t>(::android::hardware::boot::V1_0::BoolResult::INVALID_SLOT)) {
        os += (first ? "" : " | ");
        os += "INVALID_SLOT";
        first = false;
        flipped |= ::android::hardware::boot::V1_0::BoolResult::INVALID_SLOT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::boot::V1_0::BoolResult o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::boot::V1_0::BoolResult::FALSE) {
        return "FALSE";
    }
    if (o == ::android::hardware::boot::V1_0::BoolResult::TRUE) {
        return "TRUE";
    }
    if (o == ::android::hardware::boot::V1_0::BoolResult::INVALID_SLOT) {
        return "INVALID_SLOT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline void PrintTo(::android::hardware::boot::V1_0::BoolResult o, ::std::ostream* os) {
    *os << toString(o);
}


}  // namespace V1_0
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
template<> inline constexpr std::array<::android::hardware::boot::V1_0::BoolResult, 3> hidl_enum_values<::android::hardware::boot::V1_0::BoolResult> = {
    ::android::hardware::boot::V1_0::BoolResult::FALSE,
    ::android::hardware::boot::V1_0::BoolResult::TRUE,
    ::android::hardware::boot::V1_0::BoolResult::INVALID_SLOT,
};
#pragma clang diagnostic pop
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_ANDROID_HARDWARE_BOOT_V1_0_TYPES_H
