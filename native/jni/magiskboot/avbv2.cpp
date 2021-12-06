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
    const uint8_t* header = boot.buf;
    const uint8_t* footer = boot.buf + (boot.sz - 64);
    AvbFooter boot_footer;
    AvbVBMetaImageHeader vbmeta_header;
    size_t vbmeta_size;
    const uint8_t* auth;
    const uint8_t* hash;
    const uint8_t* sig;
    const uint8_t* aux;
    const uint8_t* pubkey;
    AvbHashDescriptor boot_desc;
    const uint8_t* boot_salt;
    const uint8_t* boot_digest;
    bool mismatch = false;
    uint8_t actual_boot_digest[SHA256_DIGEST_LENGTH];
    uint8_t actual_hash[SHA256_DIGEST_LENGTH];

    /* Ensure magic is correct. */
    if (avb_safe_memcmp(footer, AVB_FOOTER_MAGIC, AVB_FOOTER_MAGIC_LEN) == 0) {
        if (avb_footer_validate_and_byteswap((AvbFooter*)footer, &boot_footer)) {
            header = boot.buf + boot_footer.vbmeta_offset;
        }
    }

    /* Ensure magic is correct. */
    if (avb_safe_memcmp(header, AVB_MAGIC, AVB_MAGIC_LEN) != 0) {
        fprintf(stderr, "! Header magic is incorrect\n");
        return AVB2_INVALID;
    }
    avb_vbmeta_image_header_to_host_byte_order((AvbVBMetaImageHeader*)(header), &vbmeta_header);

    vbmeta_size = sizeof(AvbVBMetaImageHeader) + vbmeta_header.authentication_data_block_size + vbmeta_header.auxiliary_data_block_size;

    auth = header + sizeof(AvbVBMetaImageHeader);
    aux = auth + vbmeta_header.authentication_data_block_size;
    if (vbmeta_header.algorithm_type != AVB_ALGORITHM_TYPE_SHA256_RSA4096) {
        fprintf(stderr, "! Unsupported algorithm type\n");
        return AVB2_UNSUPPORTED;
    }
    hash = auth + vbmeta_header.hash_offset;
    sig = auth + vbmeta_header.signature_offset;
    pubkey = aux + vbmeta_header.public_key_offset;
    if (vbmeta_header.public_key_size != ANDROID_PUBKEY_ENCODED_SIZE) {
        fprintf(stderr, "! Unsupported pubkey size\n");
        return AVB2_UNSUPPORTED;
    }

    size_t num_descriptors;
    size_t n;
    bool boot_found = false;
    const AvbDescriptor** descriptors = descriptors = avb_descriptor_get_all(header, vbmeta_size, &num_descriptors);
    for (n = 0; n < num_descriptors; n++) {
        AvbDescriptor desc;
        if (!avb_descriptor_validate_and_byteswap(descriptors[n], &desc)) {
            fprintf(stderr, "! Descriptor is invalid\n");
            return AVB2_INVALID;
        }
        switch (desc.tag) {
            case AVB_DESCRIPTOR_TAG_HASH: {
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
                    if (avb_strcmp((const char*)boot_desc.hash_algorithm, "sha256") != 0) {
                        fprintf(stderr, "! Unsupported hash algorithm\n");
                        return AVB2_UNSUPPORTED;
                    }
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

    SHA256_CTX boot_ctx;
    SHA256_Init(&boot_ctx);
    SHA256_Update(&boot_ctx, boot_salt, boot_desc.salt_len);
    SHA256_Update(&boot_ctx, boot.buf, boot_desc.image_size);
    SHA256_Final(actual_boot_digest, &boot_ctx);

    if (boot_desc.digest_len != SHA256_DIGEST_LENGTH) {
        fprintf(stderr, "! Digest length mismatch\n");
        return AVB2_INVALID;
    }
    if (avb_memcmp(boot_digest, actual_boot_digest, SHA256_DIGEST_LENGTH) != 0) {
        if (!rw) {
            fprintf(stderr, "Boot digest mismatch\n");
            fprintf(stderr, "expected digest: %s\n", mem_to_hexstring(boot_digest, SHA256_DIGEST_LENGTH).c_str());
            fprintf(stderr, "actual digest:   %s\n", mem_to_hexstring(actual_boot_digest, SHA256_DIGEST_LENGTH).c_str());
        }
        mismatch = true;
    } else if (!rw) {
        printf("Boot digest verified\n");
    }

    SHA256_CTX auth_ctx;
    SHA256_Init(&auth_ctx);
    SHA256_Update(&auth_ctx, header, sizeof(AvbVBMetaImageHeader));
    SHA256_Update(&auth_ctx, aux, vbmeta_header.auxiliary_data_block_size);
    SHA256_Final(actual_hash, &auth_ctx);

    if (vbmeta_header.hash_size != SHA256_DIGEST_LENGTH) {
        fprintf(stderr, "! Hash size mismatch\n");
        return AVB2_INVALID;
    }
    if (avb_memcmp(hash, actual_hash, SHA256_DIGEST_LENGTH) != 0) {
        if (!rw) {
            fprintf(stderr, "Hash mismatch\n");
            fprintf(stderr, "expected hash: %s\n", mem_to_hexstring(hash, SHA256_DIGEST_LENGTH).c_str());
            fprintf(stderr, "actual hash:   %s\n", mem_to_hexstring(actual_hash, SHA256_DIGEST_LENGTH).c_str());
        }
        mismatch = true;
    } else if (!rw) {
        printf("Hash verified\n");
    }

    ScopedRSA decoded(pubkey, vbmeta_header.public_key_size);
    uint32_t signature_size = decoded.size();

    if (vbmeta_header.signature_size != signature_size) {
        fprintf(stderr, "! Signature size mismatch\n");
        return AVB2_INVALID;
    }
    if (!decoded.verify(
        hash, SHA256_DIGEST_LENGTH,
        sig, vbmeta_header.signature_size
    )) {
        if (!rw) {
            fprintf(stderr, "Signature mismatch\n");
        }
        mismatch = true;
    } else if (!rw) {
        printf("Signature verified\n");
    }

    if (mismatch && rw) {
        ScopedRSA scoped_privkey(testkey_rsa4096.c_str());

        if (scoped_privkey.size() != signature_size) {
            fprintf(stderr, "! Key size mismatch\n");
            return AVB2_INVALID;
        }

        uint8_t encoded_key[ANDROID_PUBKEY_ENCODED_SIZE];
        scoped_privkey.encode(encoded_key, ANDROID_PUBKEY_ENCODED_SIZE);
        if (avb_memcmp(pubkey, encoded_key, ANDROID_PUBKEY_ENCODED_SIZE) != 0) {
            avb_memcpy((void *)pubkey, encoded_key, ANDROID_PUBKEY_ENCODED_SIZE);
        }

        if (avb_memcmp(boot_digest, actual_boot_digest, SHA256_DIGEST_LENGTH) != 0) {
            avb_memcpy((void *)boot_digest, actual_boot_digest, SHA256_DIGEST_LENGTH);
        }

        SHA256_Init(&auth_ctx);
        SHA256_Update(&auth_ctx, header, sizeof(AvbVBMetaImageHeader));
        SHA256_Update(&auth_ctx, aux, vbmeta_header.auxiliary_data_block_size);
        SHA256_Final(actual_hash, &auth_ctx);

        uint8_t signature[signature_size];
        scoped_privkey.sign(actual_hash, SHA256_DIGEST_LENGTH, signature);

        avb_memcpy((void *)hash, actual_hash, SHA256_DIGEST_LENGTH);

        avb_memcpy((void *)sig, signature, signature_size);
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

std::string mem_to_hexstring(const uint8_t* data, size_t len) {
  std::string ret;
  char digits[17] = "0123456789abcdef";
  for (size_t n = 0; n < len; n++) {
    ret.push_back(digits[data[n] >> 4]);
    ret.push_back(digits[data[n] & 0x0f]);
  }
  return ret;
}
