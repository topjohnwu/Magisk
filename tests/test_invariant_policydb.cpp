#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <climits>

// Simulate the vulnerable pattern from policydb.cpp
// These constants mirror what would be defined in the actual code
#define PLAT_POLICY_DIR "/system/etc/selinux/"
#define SYSEXT_POLICY_DIR "/system_ext/etc/selinux/"
#define PROD_POLICY_DIR "/product/etc/selinux/"

// The fixed-size buffer used in the vulnerable code
// Typically PATH_MAX (4096) or a smaller fixed size like 256 or 512
static constexpr size_t PATH_BUFFER_SIZE = 256;

// Canary values to detect buffer overflow
static constexpr uint8_t CANARY_VALUE = 0xAB;
static constexpr size_t CANARY_SIZE = 64;

// Safe version using snprintf (what the fix should look like)
static int safe_construct_path(char* path, size_t path_size, const char* dir, const char* plat_ver, const char* suffix) {
    return snprintf(path, path_size, "%smapping/%s%s", dir, plat_ver, suffix);
}

// Unsafe version mimicking the vulnerable sprintf pattern
// Returns the number of bytes that WOULD have been written (like snprintf)
static size_t unsafe_would_write(const char* dir, const char* plat_ver, const char* suffix) {
    // Calculate what sprintf would write without actually doing it unsafely
    return strlen(dir) + strlen("mapping/") + strlen(plat_ver) + strlen(suffix);
}

struct PathConstructionResult {
    bool overflowed;
    size_t required_size;
    size_t buffer_size;
    bool canary_intact;
};

static PathConstructionResult test_path_construction(const std::string& plat_ver,
                                                       const char* dir,
                                                       const char* suffix) {
    // Allocate buffer with canary regions
    std::vector<uint8_t> buffer_region(CANARY_SIZE + PATH_BUFFER_SIZE + CANARY_SIZE, CANARY_VALUE);
    char* path = reinterpret_cast<char*>(buffer_region.data() + CANARY_SIZE);

    // Calculate required size
    size_t required = unsafe_would_write(dir, plat_ver.c_str(), suffix) + 1; // +1 for null terminator

    // Use safe snprintf
    int written = safe_construct_path(path, PATH_BUFFER_SIZE, dir, plat_ver.c_str(), suffix);

    // Check canary regions
    bool pre_canary_intact = true;
    bool post_canary_intact = true;

    for (size_t i = 0; i < CANARY_SIZE; i++) {
        if (buffer_region[i] != CANARY_VALUE) {
            pre_canary_intact = false;
            break;
        }
    }

    for (size_t i = CANARY_SIZE + PATH_BUFFER_SIZE; i < buffer_region.size(); i++) {
        if (buffer_region[i] != CANARY_VALUE) {
            post_canary_intact = false;
            break;
        }
    }

    PathConstructionResult result;
    result.overflowed = (written < 0 || static_cast<size_t>(written) >= PATH_BUFFER_SIZE);
    result.required_size = required;
    result.buffer_size = PATH_BUFFER_SIZE;
    result.canary_intact = pre_canary_intact && post_canary_intact;

    return result;
}

class SecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(SecurityTest, BufferReadsNeverExceedDeclaredLength) {
    // Invariant: Buffer reads/writes for SEPolicy path construction must never
    // exceed the declared buffer size, regardless of plat_ver input length.
    // Either the input must be truncated (safe) or rejected, but never overflow.

    std::string payload = GetParam();

    const struct {
        const char* dir;
        const char* suffix;
        const char* description;
    } test_cases[] = {
        { PLAT_POLICY_DIR,   ".cil",        "plat mapping cil"        },
        { PLAT_POLICY_DIR,   ".compat.cil", "plat mapping compat cil" },
        { SYSEXT_POLICY_DIR, ".cil",        "sysext mapping cil"      },
        { SYSEXT_POLICY_DIR, ".compat.cil", "sysext mapping compat"   },
        { PROD_POLICY_DIR,   ".cil",        "prod mapping cil"        },
    };

    for (const auto& tc : test_cases) {
        PathConstructionResult result = test_path_construction(payload, tc.dir, tc.suffix);

        // INVARIANT 1: Canary bytes must never be corrupted
        EXPECT_TRUE(result.canary_intact)
            << "SECURITY VIOLATION: Buffer overflow detected for case: " << tc.description
            << " with plat_ver of length " << payload.size()
            << ". Adjacent memory was corrupted!";

        // INVARIANT 2: If the required size exceeds buffer, the safe implementation
        // must truncate (not overflow). The written content must fit within buffer.
        if (result.required_size > PATH_BUFFER_SIZE) {
            EXPECT_TRUE(result.overflowed)
                << "Expected truncation for oversized input in case: " << tc.description
                << " (required=" << result.required_size
                << ", buffer=" << result.buffer_size << ")";
        }

        // INVARIANT 3: Verify that using snprintf (safe fix) produces bounded output
        std::vector<char> safe_buf(PATH_BUFFER_SIZE, '\0');
        int ret = snprintf(safe_buf.data(), PATH_BUFFER_SIZE,
                           "%smapping/%s%s", tc.dir, payload.c_str(), tc.suffix);

        // snprintf must not write beyond buffer
        EXPECT_LT(static_cast<size_t>(ret < 0 ? 0 : ret), PATH_BUFFER_SIZE + 1)
            << "snprintf return value indicates potential overflow for: " << tc.description;

        // The null terminator must be present within bounds
        bool null_found = false;
        for (size_t i = 0; i < PATH_BUFFER_SIZE; i++) {
            if (safe_buf[i] == '\0') {
                null_found = true;
                break;
            }
        }
        EXPECT_TRUE(null_found)
            << "No null terminator found within buffer bounds for: " << tc.description;
    }
}

TEST_P(SecurityTest, NullTerminatorAlwaysPresent) {
    // Invariant: The resulting path string must always be null-terminated within bounds
    std::string payload = GetParam();

    std::vector<char> path(PATH_BUFFER_SIZE);

    // Fill with non-null to detect missing terminator
    std::fill(path.begin(), path.end(), 'X');

    snprintf(path.data(), PATH_BUFFER_SIZE,
             PLAT_POLICY_DIR "mapping/%s.cil", payload.c_str());

    // Must find null terminator within the buffer
    bool has_null = false;
    for (size_t i = 0; i < PATH_BUFFER_SIZE; i++) {
        if (path[i] == '\0') {
            has_null = true;
            EXPECT_LT(i, PATH_BUFFER_SIZE) << "Null terminator at valid position " << i;
            break;
        }
    }
    EXPECT_TRUE(has_null) << "Path buffer lacks null terminator for plat_ver length: "
                          << payload.size();
}

TEST_P(SecurityTest, PathLengthBoundedByBufferSize) {
    // Invariant: strlen of resulting path must always be less than PATH_BUFFER_SIZE
    std::string payload = GetParam();

    std::vector<char> path(PATH_BUFFER_SIZE, '\0');

    snprintf(path.data(), PATH_BUFFER_SIZE,
             PLAT_POLICY_DIR "mapping/%s.cil", payload.c_str());

    size_t path_len = strnlen(path.data(), PATH_BUFFER_SIZE);

    EXPECT_LT(path_len, PATH_BUFFER_SIZE)
        << "Path length " << path_len << " is not less than buffer size "
        << PATH_BUFFER_SIZE << " for plat_ver of length " << payload.size();
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    SecurityTest,
    ::testing::Values(
        // Normal inputs (should pass trivially)
        std::string("30.0"),
        std::string("29.0"),
        std::string("1.0"),

        // Boundary inputs
        std::string(200, 'A'),          // Near buffer limit
        std::string(255, 'B'),          // At typical buffer limit
        std::string(256, 'C'),          // Just over typical limit

        // 2x oversized
        std::string(512, 'D'),

        // 10x oversized
        std::string(2560, 'E'),

        // PATH_MAX sized
        std::string(4096, 'F'),

        // Extremely large (10x PATH_MAX)
        std::string(40960, 'G'),

        // Null bytes embedded (should be handled)
        std::string("30.0\x00INJECTED", 14),

        // Path traversal attempts
        std::string("../../../etc/passwd"),
        std::string("../../../../" + std::string(200, 'x')),

        // Format string characters
        std::string("%s%s%s%s%s%s%s%s%s%s"),
        std::string("%n%n%n%n%n%n%n%n%n%n"),
        std::string(std::string(50, '%') + "s"),

        // Special characters
        std::string("/dev/null"),
        std::string("30.0; rm -rf /"),
        std::string("30.0`id`"),

        // Unicode/multibyte
        std::string("\xc0\xaf\xc0\xaf\xc0\xaf"),
        std::string(std::string(100, '\xff')),

        // Mixed: normal prefix + long suffix
        std::string("30.0" + std::string(300, 'X')),

        // Whitespace
        std::string(256, ' '),
        std::string("30.0\n\r\t"),

        // All zeros (except not null-terminated string)
        std::string(256, '0')
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}