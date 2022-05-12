#include <utils.hpp>

using namespace std;

// Top-level block container
struct signing_block {
    uint64_t len;

    struct id_value_pair {
        uint64_t len;
        struct /* v2_signature */ {
            uint32_t id;
            uint8_t value[0];  // size = (len - 4)
        };
    } id_value_pair_sequence[0];

    uint64_t block_len;   // *MUST* be same as len
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

// The structures above are just for documentation purpose
// The real parsing logic is the following

string read_certificate(int fd) {
    string certificate;
    uint32_t size4;
    uint64_t size8, size_of_block;

    for (int i = 0;; i++) {
        unsigned short n;
        lseek(fd, -i - 2, SEEK_END);
        read(fd, &n, 2);
        if (n == i) {
            lseek(fd, -22, SEEK_CUR);
            read(fd, &size4, 4);
            if (size4 == 0x6054b50u) { // central directory end magic
                break;
            }
        }
        if (i == 0xffff) {
            return certificate;
        }
    }

    lseek(fd, 12, SEEK_CUR);

    read(fd, &size4, 0x4);
    lseek(fd, (off_t) (size4 - 0x18), SEEK_SET);

    read(fd, &size8, 0x8);
    char magic[0x10] = {0};
    read(fd, magic, sizeof(magic));
    if (memcmp(magic, "APK Sig Block 42", sizeof(magic)) != 0) {
        return certificate;
    }

    lseek(fd, (off_t) (size4 - (size8 + 0x8)), SEEK_SET);
    read(fd, &size_of_block, 0x8);
    if (size_of_block != size8) {
        return certificate;
    }

    for (;;) {
        uint32_t id;
        uint32_t offset;
        read(fd, &size8, 0x8); // sequence length
        if (size8 == size_of_block) {
            break;
        }
        read(fd, &id, 0x4); // id
        offset = 4;

        if (id == 0x7109871au) {
            read(fd, &size4, 0x4); // signer-sequence length
            read(fd, &size4, 0x4); // signer length
            read(fd, &size4, 0x4); // signed data length
            offset += 0x4 * 3;

            read(fd, &size4, 0x4); // digests-sequence length
            lseek(fd, (off_t) (size4), SEEK_CUR);// skip digests
            offset += 0x4 + size4;

            read(fd, &size4, 0x4); // certificates length
            read(fd, &size4, 0x4); // certificate length
            offset += 0x4 * 2;

            certificate.resize(size4);
            read(fd, certificate.data(), size4);

            offset += size4;
        }
        lseek(fd, (off_t) (size8 - offset), SEEK_CUR);
    }
    return certificate;
}
