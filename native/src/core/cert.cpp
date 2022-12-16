#include <base.hpp>

using namespace std;

#define APK_SIGNING_BLOCK_MAGIC     "APK Sig Block 42"
#define SIGNATURE_SCHEME_V2_MAGIC   0x7109871a
#define EOCD_MAGIC                  0x6054b50

// Top-level block container
struct signing_block {
    uint64_t block_sz;

    struct id_value_pair {
        uint64_t len;
        struct /* v2_signature */ {
            uint32_t id;
            uint8_t value[0];  // size = (len - 4)
        };
    } id_value_pair_sequence[0];

    uint64_t block_sz_;   // *MUST* be same as block_sz
    char magic[16];       // "APK Sig Block 42"
};

struct len_prefixed {
    uint32_t len;
};

// Generic length prefixed raw data
struct len_prefixed_value : public len_prefixed {
    uint8_t value[0];
};

// V2 Signature Block
struct v2_signature {
    uint32_t id;  // 0x7109871a
    uint32_t signer_sequence_len;
    struct signer : public len_prefixed {
        struct signed_data : public len_prefixed {
            uint32_t digest_sequence_len;
            struct : public len_prefixed {
                uint32_t algorithm;
                len_prefixed_value digest;
            } digest_sequence[0];

            uint32_t certificate_sequence_len;
            len_prefixed_value certificate_sequence[0];

            uint32_t attribute_sequence_len;
            struct attribute : public len_prefixed {
                uint32_t id;
                uint8_t value[0];  // size = (len - 4)
            } attribute_sequence[0];
        } signed_data;

        uint32_t signature_sequence_len;
        struct : public len_prefixed {
            uint32_t id;
            len_prefixed_value signature;
        } signature_sequence[0];

        len_prefixed_value public_key;
    } signer_sequence[0];
};

// End of central directory record
struct EOCD {
    uint32_t magic;            // 0x6054b50
    uint8_t pad[8];            // 8 bytes of irrelevant data
    uint32_t central_dir_sz;   // size of central directory
    uint32_t central_dir_off;  // offset of central directory
    uint16_t comment_sz;       // size of comment
    char comment[0];
} __attribute__((packed));

/*
 * A v2/v3 signed APK has the format as following
 *
 * +---------------+
 * | zip content   |
 * +---------------+
 * | signing block |
 * +---------------+
 * | central dir   |
 * +---------------+
 * | EOCD          |
 * +---------------+
 *
 * Scan from end of file to find EOCD, and figure our way back to the
 * offset of the signing block. Next, directly extract the certificate
 * from the v2 signature block.
 *
 * All structures above are mostly just for documentation purpose.
 *
 * This method extracts the first certificate of the first signer
 * within the APK v2 signature block.
 */
string read_certificate(int fd, int version) {
    uint32_t u32;
    uint64_t u64;

    // Find EOCD
    for (int i = 0;; i++) {
        // i is the absolute offset to end of file
        uint16_t comment_sz = 0;
        xlseek64(fd, -static_cast<off64_t>(sizeof(comment_sz)) - i, SEEK_END);
        xxread(fd, &comment_sz, sizeof(comment_sz));
        if (comment_sz == i) {
            // Double check if we actually found the structure
            xlseek64(fd, -static_cast<off64_t>(sizeof(EOCD)), SEEK_CUR);
            uint32_t magic = 0;
            xxread(fd, &magic, sizeof(magic));
            if (magic == EOCD_MAGIC) {
                break;
            }
        }
        if (i == 0xffff) {
            // Comments cannot be longer than 0xffff (overflow), abort
            LOGE("cert: invalid APK format\n");
            return {};
        }
    }

    // We are now at EOCD + sizeof(magic)
    // Seek and read central_dir_off to find start of central directory
    uint32_t central_dir_off = 0;
    {
        constexpr off64_t off = offsetof(EOCD, central_dir_off) - sizeof(EOCD::magic);
        xlseek64(fd, off, SEEK_CUR);
    }
    xxread(fd, &central_dir_off, sizeof(central_dir_off));

    // Parse APK comment to get version code
    if (version >= 0) {
        xlseek64(fd, sizeof(EOCD::comment_sz), SEEK_CUR);
        FILE *fp = fdopen(fd, "r");  // DO NOT close this file pointer
        int apk_ver = -1;
        parse_prop_file(fp, [&](string_view key, string_view value) -> bool {
            if (key == "versionCode") {
                apk_ver = parse_int(value);
                return false;
            }
            return true;
        });
        if (version > apk_ver) {
            // Enforce the magisk app to always be newer than magiskd
            LOGE("cert: APK version too low\n");
            return {};
        }
    }

    // Next, find the start of the APK signing block
    {
        constexpr int off = sizeof(signing_block::block_sz_) + sizeof(signing_block::magic);
        xlseek64(fd, (off64_t) (central_dir_off - off), SEEK_SET);
    }
    xxread(fd, &u64, sizeof(u64));  // u64 = block_sz_
    char magic[sizeof(signing_block::magic)] = {0};
    xxread(fd, magic, sizeof(magic));
    if (memcmp(magic, APK_SIGNING_BLOCK_MAGIC, sizeof(magic)) != 0) {
        // Invalid signing block magic, abort
        LOGE("cert: invalid signing block magic\n");
        return {};
    }
    uint64_t signing_blk_sz = 0;
    xlseek64(fd, -static_cast<off64_t>(u64 + sizeof(signing_blk_sz)), SEEK_CUR);
    xxread(fd, &signing_blk_sz, sizeof(signing_blk_sz));
    if (signing_blk_sz != u64) {
        // block_sz != block_sz_, invalid signing block format, abort
        LOGE("cert: invalid signing block format\n");
        return {};
    }

    // Finally, we are now at the beginning of the id-value pair sequence

    for (;;) {
        xxread(fd, &u64, sizeof(u64)); // id-value pair length
        if (u64 == signing_blk_sz) {
            // Outside of the id-value pair sequence; actually reading block_sz_
            break;
        }

        uint32_t id;
        xxread(fd, &id, sizeof(id));
        if (id == SIGNATURE_SCHEME_V2_MAGIC) {
            // Skip [signer sequence length] + [1st signer length] + [signed data length]
            xlseek64(fd, sizeof(uint32_t) * 3, SEEK_CUR);

            xxread(fd, &u32, sizeof(u32)); // digest sequence length
            xlseek64(fd, u32, SEEK_CUR);     // skip all digests

            xlseek64(fd, sizeof(uint32_t), SEEK_CUR); // cert sequence length
            xxread(fd, &u32, sizeof(u32));          // 1st cert length

            string cert;
            cert.resize(u32);
            xxread(fd, cert.data(), u32);

            return cert;
        } else {
            // Skip this id-value pair
            xlseek64(fd, u64 - sizeof(id), SEEK_CUR);
        }
    }

    LOGE("cert: cannot find certificate\n");
    return {};
}
