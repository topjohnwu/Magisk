#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <climits>

// Simulate the vulnerable pattern from policydb.cpp
// The fixed buffer size used in the original code context
// We test that any safe implementation must validate input length
// before performing sprintf with these suffixes.

static const char* SUFFIXES[] = {
    ".plat_and_mapping.sha256",
    ".plat_sepolicy_and_mapping.sha256",
    ".product_sepolicy_and_mapping.sha256",
    ".system_ext_sepolicy_and_mapping.sha256",
    nullptr
};

// The longest suffix is ".system_ext_sepolicy_and_mapping.sha256" = 39 chars
// ".product_sepolicy_and_mapping.sha256" = 36 chars
// A typical PATH_MAX is 4096, but the original code likely uses a smaller buffer
// Based on context, assume a 256-byte stack buffer (common pattern)
static const size_t ASSUMED_BUFFER_SIZE = 256;

// Safe version that checks bounds before sprintf
static bool safe_build_sha_path(char* out, size_t out_size, const char* precompiled, const char* suffix) {
    if (!out || !precompiled || !suffix || out_size == 0) {
        return false;
    }
    size_t precompiled_len = strlen(precompiled);
    size_t suffix_len = strlen(suffix);
    // Check that precompiled + suffix + null terminator fits in buffer
    if (precompiled_len + suffix_len + 1 > out_size) {
        return false;
    }
    int written = snprintf(out, out_size, "%s%s", precompiled, suffix);
    return (written >= 0 && (size_t)written < out_size);
}

// Compute the maximum safe precompiled path length for a given suffix and buffer size
static size_t max_safe_precompiled_len(const char* suffix, size_t buffer_size) {
    size_t suffix_len = strlen(suffix);
    if (suffix_len + 1 >= buffer_size) {
        return 0;
    }
    return buffer_size - suffix_len - 1;
}

class SecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(SecurityTest, BufferReadNeverExceedsDeclaredLength) {
    // Invariant: sprintf calls building sha256 paths must never write beyond
    // the declared buffer size, regardless of input length.
    // Any safe implementation must reject or truncate inputs that would overflow.

    std::string payload = GetParam();
    const char* precompiled = payload.c_str();

    char compiled_sha[ASSUMED_BUFFER_SIZE];
    memset(compiled_sha, 0xAA, sizeof(compiled_sha)); // canary pattern

    for (int i = 0; SUFFIXES[i] != nullptr; i++) {
        const char* suffix = SUFFIXES[i];
        size_t precompiled_len = strlen(precompiled);
        size_t suffix_len = strlen(suffix);
        size_t required_len = precompiled_len + suffix_len + 1;

        // INVARIANT: If the combined length exceeds buffer size,
        // the operation must be rejected (return false / not write)
        bool result = safe_build_sha_path(compiled_sha, ASSUMED_BUFFER_SIZE, precompiled, suffix);

        if (required_len > ASSUMED_BUFFER_SIZE) {
            // Must be rejected - buffer overflow would occur with naive sprintf
            EXPECT_FALSE(result)
                << "Expected rejection for oversized input: precompiled_len="
                << precompiled_len << " suffix_len=" << suffix_len
                << " required=" << required_len
                << " buffer=" << ASSUMED_BUFFER_SIZE
                << " suffix=" << suffix;
        } else {
            // Must succeed and produce correct output
            EXPECT_TRUE(result)
                << "Expected success for safe-sized input: precompiled_len="
                << precompiled_len << " suffix=" << suffix;

            if (result) {
                // Verify the output is null-terminated within bounds
                size_t out_len = strnlen(compiled_sha, ASSUMED_BUFFER_SIZE);
                EXPECT_LT(out_len, ASSUMED_BUFFER_SIZE)
                    << "Output not null-terminated within buffer bounds";

                // Verify content correctness
                std::string expected = payload + suffix;
                EXPECT_EQ(std::string(compiled_sha), expected)
                    << "Output content mismatch for suffix=" << suffix;
            }
        }

        // Verify canary bytes beyond buffer are untouched
        // (In a real overflow scenario these would be corrupted)
        // We use a separate canary buffer to detect writes past end
        char canary_buffer[ASSUMED_BUFFER_SIZE + 16];
        memset(canary_buffer, 0xBB, sizeof(canary_buffer));
        char* safe_region = canary_buffer + ASSUMED_BUFFER_SIZE;

        // Simulate what would happen with raw sprintf on oversized input
        // We verify our safe version never touches beyond ASSUMED_BUFFER_SIZE
        bool safe_result = safe_build_sha_path(canary_buffer, ASSUMED_BUFFER_SIZE, precompiled, suffix);

        // Check canary region is untouched
        for (size_t j = 0; j < 16; j++) {
            EXPECT_EQ((unsigned char)safe_region[j], 0xBB)
                << "Buffer overflow detected: byte at offset "
                << (ASSUMED_BUFFER_SIZE + j) << " was modified!"
                << " precompiled_len=" << precompiled_len
                << " suffix=" << suffix;
        }

        (void)safe_result;
    }
}

TEST_P(SecurityTest, MaxSafeInputLengthRespected) {
    // Invariant: For each suffix, inputs at exactly the maximum safe length
    // must succeed, and inputs one byte over must fail.
    std::string payload = GetParam();

    for (int i = 0; SUFFIXES[i] != nullptr; i++) {
        const char* suffix = SUFFIXES[i];
        size_t max_safe = max_safe_precompiled_len(suffix, ASSUMED_BUFFER_SIZE);

        // Test at exactly max safe length
        std::string exact_safe(max_safe, 'A');
        char buf1[ASSUMED_BUFFER_SIZE + 64];
        memset(buf1, 0xCC, sizeof(buf1));
        bool result_exact = safe_build_sha_path(buf1, ASSUMED_BUFFER_SIZE, exact_safe.c_str(), suffix);
        EXPECT_TRUE(result_exact)
            << "Should succeed at exactly max safe length=" << max_safe
            << " for suffix=" << suffix;

        // Verify no overflow into guard region
        for (size_t j = ASSUMED_BUFFER_SIZE; j < sizeof(buf1); j++) {
            EXPECT_EQ((unsigned char)buf1[j], 0xCC)
                << "Overflow at position " << j << " for suffix=" << suffix;
        }

        // Test one byte over max safe length
        if (max_safe < SIZE_MAX) {
            std::string one_over(max_safe + 1, 'B');
            char buf2[ASSUMED_BUFFER_SIZE + 64];
            memset(buf2, 0xDD, sizeof(buf2));
            bool result_over = safe_build_sha_path(buf2, ASSUMED_BUFFER_SIZE, one_over.c_str(), suffix);
            EXPECT_FALSE(result_over)
                << "Should reject input one byte over max safe length="
                << (max_safe + 1) << " for suffix=" << suffix;

            // Verify no overflow into guard region
            for (size_t j = ASSUMED_BUFFER_SIZE; j < sizeof(buf2); j++) {
                EXPECT_EQ((unsigned char)buf2[j], 0xDD)
                    << "Overflow at position " << j << " for suffix=" << suffix;
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    SecurityTest,
    ::testing::Values(
        // Normal/safe inputs
        std::string("/system/etc/selinux/precompiled_sepolicy"),
        std::string("/vendor/etc/selinux/precompiled_sepolicy"),
        std::string("/a"),
        std::string(""),

        // Boundary inputs - exactly at buffer limit minus longest suffix
        // longest suffix ".system_ext_sepolicy_and_mapping.sha256" = 39 chars
        // max safe = 256 - 39 - 1 = 216 chars
        std::string(216, 'X'),
        std::string(217, 'X'),  // one over for longest suffix

        // 2x buffer size
        std::string(512, '/'),
        std::string(512, 'A'),

        // 10x buffer size
        std::string(2560, 'B'),
        std::string(2560, '/'),

        // PATH_MAX sized input
        std::string(4096, 'C'),

        // Very large input
        std::string(65536, 'D'),

        // Inputs with path separators and special chars
        std::string(300, '/'),
        std::string(300, '.'),
        std::string(300, '\x01'),

        // Null-byte adjacent (string stops at first null in C, but test boundary)
        std::string(255, 'E'),
        std::string(256, 'F'),
        std::string(257, 'G'),

        // Realistic long paths
        std::string("/very/long/path/that/exceeds/normal/limits/") + std::string(200, 'H'),
        std::string("/system/etc/selinux/") + std::string(250, 'I'),

        // Unicode-like byte sequences (high bytes)
        std::string(300, '\xFF'),
        std::string(300, '\xFE'),

        // Mixed content
        std::string(128, 'A') + std::string(128, 'B') + std::string(128, 'C')
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}