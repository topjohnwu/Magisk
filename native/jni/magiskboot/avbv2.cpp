#include <functional>
#include <memory>

#include <cstring>
#include <inttypes.h>
#include <openssl/sha.h>
#include <libavb/libavb.h>
#include <utils.hpp>

#include "avbv2.hpp"
#include "scopedrsa.hpp"
#include "android_pubkey.h"

int avbv2_commands(int argc, char *argv[]) {
    char *boot = argv[0];
    ++argv;
    --argc;

    std::string_view action(argv[0]);
    if (action == "verify") {
        return avbv2_verify_sign(boot, false);
    } else if (action == "sign") {
        return avbv2_verify_sign(boot, true);
    } else {
        return 1;
    }
}

int avbv2_verify_sign(const char *image, bool rw) {
    mmap_data boot(image, rw);
    const uint8_t* header_block = boot.buf;
    const uint8_t* footer = boot.buf + (boot.sz - 64);
    AvbFooter boot_footer;
    AvbVBMetaImageHeader vbmeta_header;
    size_t vbmeta_length;
    const uint8_t* authentication_block;
    const uint8_t* hash;
    const uint8_t* signature;
    const uint8_t* auxiliary_block;
    const uint8_t* public_key_data;
    AvbHashDescriptor boot_desc;
    const uint8_t* boot_salt;
    const uint8_t* boot_digest;
    bool mismatch = false;
    uint8_t actual_boot_digest[SHA512_DIGEST_LENGTH];
    uint32_t actual_boot_digest_length;
    uint8_t actual_hash[SHA512_DIGEST_LENGTH];
    uint32_t actual_hash_length;
    uint8_t actual_signature[AVB_RSA8192_NUM_BYTES];
    uint32_t actual_signature_length;

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_slot_verify.c#661
    /* Ensure magic is correct. */
    if (avb_safe_memcmp(footer, AVB_FOOTER_MAGIC, AVB_FOOTER_MAGIC_LEN) == 0) {
        if (avb_footer_validate_and_byteswap((AvbFooter*)footer, &boot_footer)) {
            header_block = boot.buf + boot_footer.vbmeta_offset;
        }
    }

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_vbmeta_image.c#63
    /* Ensure magic is correct. */
    if (avb_safe_memcmp(header_block, AVB_MAGIC, AVB_MAGIC_LEN) != 0) {
        fprintf(stderr, "! Header magic is incorrect\n");
        return AVB2_INVALID;
    }
    avb_vbmeta_image_header_to_host_byte_order((AvbVBMetaImageHeader*)(header_block), &vbmeta_header);

    vbmeta_length = sizeof(AvbVBMetaImageHeader) + vbmeta_header.authentication_data_block_size + vbmeta_header.auxiliary_data_block_size;

    authentication_block = header_block + sizeof(AvbVBMetaImageHeader);
    auxiliary_block = authentication_block + vbmeta_header.authentication_data_block_size;
    if (vbmeta_header.algorithm_type == AVB_ALGORITHM_TYPE_NONE || vbmeta_header.algorithm_type > AVB_ALGORITHM_TYPE_SHA512_RSA8192) {
        fprintf(stderr, "! Invalid algorithm type\n");
        return AVB2_INVALID;
    }
    hash = authentication_block + vbmeta_header.hash_offset;
    signature = authentication_block + vbmeta_header.signature_offset;
    public_key_data = auxiliary_block + vbmeta_header.public_key_offset;

    ScopedRSA pubkey(public_key_data, vbmeta_header.public_key_size);
    if (!pubkey.is_initialized()) {
        fprintf(stderr, "! Invalid public_key_data size\n");
        return AVB2_INVALID;
    }

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_slot_verify.c#940
    size_t num_descriptors;
    size_t n;
    bool boot_found = false;
    const AvbDescriptor** descriptors = descriptors = avb_descriptor_get_all(header_block, vbmeta_length, &num_descriptors);
    for (n = 0; n < num_descriptors; n++) {
        // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_hash_descriptor.c#34
        AvbDescriptor desc;
        if (!avb_descriptor_validate_and_byteswap(descriptors[n], &desc)) {
            fprintf(stderr, "! Descriptor is invalid\n");
            return AVB2_INVALID;
        }
        switch (desc.tag) {
            case AVB_DESCRIPTOR_TAG_HASH: {
                // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_slot_verify.c#302
                AvbHashDescriptor hash_desc;
                const uint8_t* desc_partition_name;

                if (!avb_hash_descriptor_validate_and_byteswap((AvbHashDescriptor*)descriptors[n], &hash_desc)) {
                    fprintf(stderr, "! Hash descriptor is invalid\n");
                    return AVB2_INVALID;
                }

                desc_partition_name = (const uint8_t*)descriptors[n] + sizeof(AvbHashDescriptor);

                if (hash_desc.partition_name_len == 4 && strncmp((const char*)desc_partition_name, "boot", hash_desc.partition_name_len) == 0) {
                    boot_desc = hash_desc;
                    boot_found = true;

                    boot_salt = desc_partition_name + hash_desc.partition_name_len;
                    boot_digest = boot_salt + hash_desc.salt_len;
                } else {
                    fprintf(stderr, "! Unexpected descriptor\n");
                    return AVB2_UNSUPPORTED;
                }
            } break;
            case AVB_DESCRIPTOR_TAG_PROPERTY: {} break;
            default: {
                fprintf(stderr, "! Unexpected descriptor\n");
                return AVB2_UNSUPPORTED;
            } break;
        }
    }
    if (!boot_found) {
        fprintf(stderr, "! Boot descriptor missing\n");
        return AVB2_INVALID;
    }

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_slot_verify.c#388
    SHA256_CTX sha256_ctx;
    SHA512_CTX sha512_ctx;
    if (avb_strcmp((const char*)boot_desc.hash_algorithm, "sha256") == 0) {
        SHA256_Init(&sha256_ctx);
        SHA256_Update(&sha256_ctx, boot_salt, boot_desc.salt_len);
        SHA256_Update(&sha256_ctx, boot.buf, boot_desc.image_size);
        SHA256_Final(actual_boot_digest, &sha256_ctx);
        actual_boot_digest_length = SHA256_DIGEST_LENGTH;
    } else  if (avb_strcmp((const char*)boot_desc.hash_algorithm, "sha512") != 0) {
        SHA512_Init(&sha512_ctx);
        SHA512_Update(&sha512_ctx, boot_salt, boot_desc.salt_len);
        SHA512_Update(&sha512_ctx, boot.buf, boot_desc.image_size);
        SHA512_Final(actual_boot_digest, &sha512_ctx);
        actual_boot_digest_length = SHA512_DIGEST_LENGTH;
    } else {
        fprintf(stderr, "! Invalid hash algorithm\n");
        return AVB2_INVALID;
    }

    if (boot_desc.digest_len != actual_boot_digest_length) {
        fprintf(stderr, "! Digest length mismatch\n");
        return AVB2_INVALID;
    }
    if (avb_memcmp(boot_digest, actual_boot_digest, actual_boot_digest_length) != 0) {
        if (!rw) {
            fprintf(stderr, "Boot digest mismatch\n");
            fprintf(stderr, "expected digest: %s\n", mem_to_hexstring(boot_digest, boot_desc.digest_len).c_str());
            fprintf(stderr, "actual digest:   %s\n", mem_to_hexstring(actual_boot_digest, actual_boot_digest_length).c_str());
        }
        mismatch = true;
    } else if (!rw) {
        printf("Boot digest verified\n");
    }

    // https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_vbmeta_image.c#176
    switch (vbmeta_header.algorithm_type) {
        case AVB_ALGORITHM_TYPE_SHA256_RSA2048:
        case AVB_ALGORITHM_TYPE_SHA256_RSA4096:
        case AVB_ALGORITHM_TYPE_SHA256_RSA8192:
            SHA256_Init(&sha256_ctx);
            SHA256_Update(&sha256_ctx, header_block, sizeof(AvbVBMetaImageHeader));
            SHA256_Update(&sha256_ctx, auxiliary_block, vbmeta_header.auxiliary_data_block_size);
            SHA256_Final(actual_hash, &sha256_ctx);
            actual_hash_length = SHA256_DIGEST_LENGTH;
            break;
        case AVB_ALGORITHM_TYPE_SHA512_RSA2048:
        case AVB_ALGORITHM_TYPE_SHA512_RSA4096:
        case AVB_ALGORITHM_TYPE_SHA512_RSA8192:
            SHA512_Init(&sha512_ctx);
            SHA512_Update(&sha512_ctx, header_block, sizeof(AvbVBMetaImageHeader));
            SHA512_Update(&sha512_ctx, auxiliary_block, vbmeta_header.auxiliary_data_block_size);
            SHA512_Final(actual_hash, &sha512_ctx);
            actual_hash_length = SHA512_DIGEST_LENGTH;
            break;
        default:
            fprintf(stderr, "! Unknown algorithm\n");
            return AVB2_INVALID;
    }

    if (vbmeta_header.hash_size != actual_hash_length) {
        fprintf(stderr, "! Hash size mismatch\n");
        return AVB2_INVALID;
    }
    if (avb_memcmp(hash, actual_hash, actual_hash_length) != 0) {
        if (!rw) {
            fprintf(stderr, "Hash mismatch\n");
            fprintf(stderr, "expected hash: %s\n", mem_to_hexstring(hash, vbmeta_header.hash_size).c_str());
            fprintf(stderr, "actual hash:   %s\n", mem_to_hexstring(actual_hash, actual_hash_length).c_str());
        }
        mismatch = true;
    } else if (!rw) {
        printf("Hash verified\n");
    }

    actual_signature_length = pubkey.size();

    if (vbmeta_header.signature_size != actual_signature_length) {
        fprintf(stderr, "! Signature size mismatch\n");
        return AVB2_INVALID;
    }
    if (!pubkey.verify(
        hash, vbmeta_header.hash_size,
        signature, vbmeta_header.signature_size
    )) {
        if (!rw) {
            fprintf(stderr, "Signature mismatch\n");
        }
        mismatch = true;
    } else if (!rw) {
        printf("Signature verified\n");
    }

    if (mismatch && rw) {
        std::string privkey_string;
        int hash_nid;

        switch (vbmeta_header.algorithm_type) {
            case AVB_ALGORITHM_TYPE_SHA256_RSA2048:
            case AVB_ALGORITHM_TYPE_SHA512_RSA2048:
                privkey_string = testkey_rsa2048;
                break;
            case AVB_ALGORITHM_TYPE_SHA256_RSA4096:
            case AVB_ALGORITHM_TYPE_SHA512_RSA4096:
                privkey_string = testkey_rsa4096;
                break;
            case AVB_ALGORITHM_TYPE_SHA256_RSA8192:
            case AVB_ALGORITHM_TYPE_SHA512_RSA8192:
                privkey_string = testkey_rsa8192;
                break;
        }


        ScopedRSA privkey(privkey_string.c_str());

        if (privkey.size() != actual_signature_length) {
            fprintf(stderr, "! Key size mismatch\n");
            return AVB2_INVALID;
        }

        uint8_t new_public_key_data[privkey.encoded_size()];
        privkey.encode(new_public_key_data, privkey.encoded_size());
        if (avb_memcmp(public_key_data, new_public_key_data, privkey.encoded_size()) != 0) {
            avb_memcpy((void *)public_key_data, new_public_key_data, privkey.encoded_size());
        }

        if (avb_memcmp(boot_digest, actual_boot_digest, actual_boot_digest_length) != 0) {
            avb_memcpy((void *)boot_digest, actual_boot_digest, actual_boot_digest_length);
        }

        switch (vbmeta_header.algorithm_type) {
            case AVB_ALGORITHM_TYPE_SHA256_RSA2048:
            case AVB_ALGORITHM_TYPE_SHA256_RSA4096:
            case AVB_ALGORITHM_TYPE_SHA256_RSA8192:
                SHA256_Init(&sha256_ctx);
                SHA256_Update(&sha256_ctx, header_block, sizeof(AvbVBMetaImageHeader));
                SHA256_Update(&sha256_ctx, auxiliary_block, vbmeta_header.auxiliary_data_block_size);
                SHA256_Final(actual_hash, &sha256_ctx);
                hash_nid = NID_sha256;
                break;
            case AVB_ALGORITHM_TYPE_SHA512_RSA2048:
            case AVB_ALGORITHM_TYPE_SHA512_RSA4096:
            case AVB_ALGORITHM_TYPE_SHA512_RSA8192:
                SHA512_Init(&sha512_ctx);
                SHA512_Update(&sha512_ctx, header_block, sizeof(AvbVBMetaImageHeader));
                SHA512_Update(&sha512_ctx, auxiliary_block, vbmeta_header.auxiliary_data_block_size);
                SHA512_Final(actual_hash, &sha512_ctx);
                hash_nid = NID_sha512;
                break;
        }

        privkey.sign(hash_nid, actual_hash, actual_hash_length, actual_signature);

        avb_memcpy((void *)hash, actual_hash, actual_hash_length);

        avb_memcpy((void *)signature, actual_signature, actual_signature_length);
        printf("- Boot AVBv2 signed\n");

        mismatch = false;
    } else if (!mismatch) {
        if (!rw) {
            printf("\nAll tests passed\n");
        } else {
            printf("- Boot already AVBv2 signed\n");
        }
    }

    return mismatch ? AVB2_INVALID : 0;
}

// https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/test/avb_unittest_util.cc#29
std::string mem_to_hexstring(const uint8_t* data, size_t len) {
  std::string ret;
  char digits[17] = "0123456789abcdef";
  for (size_t n = 0; n < len; n++) {
    ret.push_back(digits[data[n] >> 4]);
    ret.push_back(digits[data[n] & 0x0f]);
  }
  return ret;
}
