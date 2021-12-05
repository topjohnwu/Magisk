#include <functional>
#include <memory>

#include <cstring>
#include <inttypes.h>
#include <openssl/sha.h>
#include <libavb/libavb.h>
#include <utils.hpp>

#include "avb2.hpp"
#include "scopedrsa.hpp"
#include "android_pubkey.h"

int avb2_commands(int argc, char *argv[]) {
    char *boot = argv[0];
    ++argv;
    --argc;

    std::string_view action(argv[0]);
    if (action == "verify") {
        return avb2_verify_sign(boot, nullptr);
    } else if (argc > 1 && action == "sign") {
        return avb2_verify_sign(boot, argv[1]);
    } else {
        return 1;
    }
}

int avb2_verify_sign(const char *image, const char *privkey) {
    bool rw = false;
    if (privkey != nullptr) {
        rw = true;
    }
    mmap_data boot(image, rw);
    const uint8_t* header = boot.buf;
    const uint8_t* footer = boot.buf + (boot.sz - 64);
    AvbFooter boot_footer;
    AvbVBMetaImageHeader vbmeta_header;
    size_t vbmeta_size;
    const AvbAlgorithmData* algorithm;
    const uint8_t* auth;
    const uint8_t* hash;
    const uint8_t* sig;
    uint8_t sig_digest[SHA_DIGEST_LENGTH];
    const uint8_t* aux;
    const uint8_t* pubkey;
    uint8_t pubkey_digest[SHA_DIGEST_LENGTH];
    AvbHashDescriptor boot_desc;
    const uint8_t* boot_salt;
    const uint8_t* boot_digest;
    bool mismatch = false;
    uint8_t actual_boot_digest[SHA256_DIGEST_LENGTH];
    uint8_t actual_hash[SHA256_DIGEST_LENGTH];

    /* Ensure magic is correct. */
    if (avb_safe_memcmp(footer, AVB_FOOTER_MAGIC, AVB_FOOTER_MAGIC_LEN) == 0) {
        bool valid = avb_footer_validate_and_byteswap((AvbFooter*)footer, &boot_footer);

        if (valid) {
            printf("\n### Footer ###\n\n");
            printf("magic: %.4s\n", (char *)boot_footer.magic);
            printf("version_major: %u\n", boot_footer.version_major);
            printf("version_minor: %u\n", boot_footer.version_minor);
            printf("original_image_size: %" PRIu64 "\n", boot_footer.original_image_size);
            printf("vbmeta_offset: %" PRIu64 "\n", boot_footer.vbmeta_offset);
            printf("vbmeta_size: %" PRIu64 "\n", boot_footer.vbmeta_size);
            header = boot.buf + boot_footer.vbmeta_offset;
        } else {
            printf("Footer is invalid.\n");
        }
    } else {
        printf("Footer magic is incorrect.\n");
    }

    printf("\n### Header ###\n\n");

    /* Ensure magic is correct. */
    if (avb_safe_memcmp(header, AVB_MAGIC, AVB_MAGIC_LEN) != 0) {
        printf("Header magic is incorrect.\n");
        return AVB2_INVALID;
    }
    avb_vbmeta_image_header_to_host_byte_order((AvbVBMetaImageHeader*)(header), &vbmeta_header);

    vbmeta_size = sizeof(AvbVBMetaImageHeader) + vbmeta_header.authentication_data_block_size + vbmeta_header.auxiliary_data_block_size;

    printf("magic: %.4s\n", (char *)vbmeta_header.magic);
    printf("required_libavb_version_major: %u\n", vbmeta_header.required_libavb_version_major);
    printf("required_libavb_version_minor: %u\n", vbmeta_header.required_libavb_version_minor);
    auth = header + sizeof(AvbVBMetaImageHeader);
    printf("authentication_data_block_size: %" PRIu64 "\n", vbmeta_header.authentication_data_block_size);
    aux = auth + vbmeta_header.authentication_data_block_size;
    printf("auxiliary_data_block_size: %" PRIu64 "\n", vbmeta_header.auxiliary_data_block_size);
    if (vbmeta_header.algorithm_type != AVB_ALGORITHM_TYPE_SHA256_RSA4096) {
        printf("algorithm_type: %u\n", vbmeta_header.algorithm_type);
        printf("Unsupported algorithm type.\n");
        return AVB2_UNSUPPORTED;
    }
    printf("algorithm_type: AVB_ALGORITHM_TYPE_SHA256_RSA4096\n");
    algorithm = avb_get_algorithm_data((AvbAlgorithmType)vbmeta_header.algorithm_type);
    if (!algorithm) {
        printf("Invalid or unknown algorithm.\n");
        return AVB2_INVALID;
    }
    printf("hash_offset: %" PRIu64 "\n", vbmeta_header.hash_offset);
    hash = auth + vbmeta_header.hash_offset;
    printf("hash_size: %" PRIu64 "\n", vbmeta_header.hash_size);
    printf("hash: %s\n", mem_to_hexstring(hash, vbmeta_header.hash_size).c_str());
    printf("signature_offset: %" PRIu64 "\n", vbmeta_header.signature_offset);
    sig = auth + vbmeta_header.signature_offset;
    printf("signature_size: %" PRIu64 "\n", vbmeta_header.signature_size);
    SHA1(sig, vbmeta_header.signature_size, sig_digest);
    printf("signature (SHA1): %s\n", mem_to_hexstring(sig_digest, SHA_DIGEST_LENGTH).c_str());
    printf("public_key_offset: %" PRIu64 "\n", vbmeta_header.public_key_offset);
    pubkey = aux + vbmeta_header.public_key_offset;
    printf("public_key_size: %" PRIu64 "\n", vbmeta_header.public_key_size);
    if (vbmeta_header.public_key_size != ANDROID_PUBKEY_ENCODED_SIZE) {
        printf("Unsupported pubkey size.\n");
        return AVB2_UNSUPPORTED;
    }
    SHA1(pubkey, vbmeta_header.public_key_size, pubkey_digest);
    printf("public_key (SHA1): %s\n", mem_to_hexstring(pubkey_digest, SHA_DIGEST_LENGTH).c_str());
    printf("public_key_metadata_offset: %" PRIu64 "\n", vbmeta_header.public_key_metadata_offset);
    printf("public_key_metadata_size: %" PRIu64 "\n", vbmeta_header.public_key_metadata_size);
    printf("descriptors_offset: %" PRIu64 "\n", vbmeta_header.descriptors_offset);
    printf("descriptors_size: %" PRIu64 "\n", vbmeta_header.descriptors_size);
    printf("rollback_index: %" PRIu64 "\n", vbmeta_header.rollback_index);
    printf("flags: %u\n", vbmeta_header.flags);
    printf("rollback_index_location: %u\n", vbmeta_header.rollback_index_location);
    printf("release_string: %s\n", (char *)vbmeta_header.release_string);

    printf("\n### boot ###\n\n");

    size_t num_descriptors;
    size_t n;
    bool boot_found = false;
    const AvbDescriptor** descriptors = descriptors = avb_descriptor_get_all(header, vbmeta_size, &num_descriptors);
    for (n = 0; n < num_descriptors; n++) {
        AvbDescriptor desc;
        if (!avb_descriptor_validate_and_byteswap(descriptors[n], &desc)) {
            printf("Descriptor is invalid.\n");
            return AVB2_INVALID;
        }
        switch (desc.tag) {
            case AVB_DESCRIPTOR_TAG_HASH: {
                AvbHashDescriptor hash_desc;
                const uint8_t* desc_partition_name;

                if (!avb_hash_descriptor_validate_and_byteswap((AvbHashDescriptor*)descriptors[n], &hash_desc)) {
                    printf("Hash descriptor is invalid.\n");
                    return AVB2_INVALID;
                }

                desc_partition_name = (const uint8_t*)descriptors[n] + sizeof(AvbHashDescriptor);

                if (hash_desc.partition_name_len == 4 && strncmp((const char*)desc_partition_name, "boot", hash_desc.partition_name_len) == 0) {
                    boot_desc = hash_desc;
                    boot_found = true;

                    boot_salt = desc_partition_name + hash_desc.partition_name_len;
                    boot_digest = boot_salt + hash_desc.salt_len;
                    printf("hash_algorithm: %s\n", (const char*)boot_desc.hash_algorithm);

                    if (avb_strcmp((const char*)boot_desc.hash_algorithm, "sha256") == 0) {
                        printf("salt: %s\n", mem_to_hexstring(boot_salt, boot_desc.salt_len).c_str());
                        printf("digest: %s\n", mem_to_hexstring(boot_digest, boot_desc.digest_len).c_str());
                    } else if (avb_strcmp((const char*)hash_desc.hash_algorithm, "sha512") == 0) {
                        printf("Unsupported hash algorithm.\n");
                        return AVB2_UNSUPPORTED;
                    } else {
                        printf("Unsupported hash algorithm.\n");
                        return AVB2_UNSUPPORTED;
                    }
                } else {
                    printf("Unexpected descriptor.\n");
                    return AVB2_UNSUPPORTED;
                }
            } break;
            case AVB_DESCRIPTOR_TAG_PROPERTY: {} break;
            default: {
                printf("Unexpected descriptor.\n");
                return AVB2_UNSUPPORTED;
            } break;
        }
    }
    if (!boot_found) {
        printf("boot descriptor missing\n");
        return AVB2_INVALID;
    }

    printf("\n### Tests ###\n\n");

    SHA256_CTX boot_ctx;
    SHA256_Init(&boot_ctx);
    SHA256_Update(&boot_ctx, boot_salt, boot_desc.salt_len);
    SHA256_Update(&boot_ctx, boot.buf, boot_desc.image_size);
    SHA256_Final(actual_boot_digest, &boot_ctx);

    if (boot_desc.digest_len != SHA256_DIGEST_LENGTH) {
        printf("Digest length mismatch.\n");
        return AVB2_INVALID;
    }
    if (avb_memcmp(boot_digest, actual_boot_digest, SHA256_DIGEST_LENGTH) != 0) {
        printf("Boot digest mismatch.\n");
        printf("expected digest: %s\n", mem_to_hexstring(boot_digest, SHA256_DIGEST_LENGTH).c_str());
        printf("actual digest:   %s\n", mem_to_hexstring(actual_boot_digest, SHA256_DIGEST_LENGTH).c_str());
        mismatch = true;
    } else {
        printf("Boot digest verified.\n");
    }

    SHA256_CTX auth_ctx;
    SHA256_Init(&auth_ctx);
    SHA256_Update(&auth_ctx, header, sizeof(AvbVBMetaImageHeader));
    SHA256_Update(&auth_ctx, aux, vbmeta_header.auxiliary_data_block_size);
    SHA256_Final(actual_hash, &auth_ctx);

    if (vbmeta_header.hash_size != SHA256_DIGEST_LENGTH) {
        printf("Hash size mismatch.\n");
        return AVB2_INVALID;
    }
    if (avb_memcmp(hash, actual_hash, SHA256_DIGEST_LENGTH) != 0) {
        printf("Hash mismatch.\n");
        printf("expected hash: %s\n", mem_to_hexstring(hash, SHA256_DIGEST_LENGTH).c_str());
        printf("actual hash:   %s\n", mem_to_hexstring(actual_hash, SHA256_DIGEST_LENGTH).c_str());
        mismatch = true;
    } else {
        printf("Hash verified.\n");
    }

    ScopedRSA decoded(pubkey, vbmeta_header.public_key_size);
    uint32_t signature_size = decoded.size();

    if (vbmeta_header.signature_size != signature_size) {
        printf("Signature size mismatch.\n");
        return AVB2_INVALID;
    }
    if (!decoded.verify(
        hash, SHA256_DIGEST_LENGTH,
        sig, vbmeta_header.signature_size
    )) {
        printf("Signature mismatch.\n");
        mismatch = true;
    } else {
        printf("Signature verified.\n");
    }

    if (mismatch && rw) {
        printf("\n### Patches ###\n\n");

        ScopedRSA scoped_privkey(privkey);

        if (scoped_privkey.size() != signature_size) {
            printf("Key size mismatch.\n");
            return AVB2_INVALID;
        }

        uint8_t encoded_key[ANDROID_PUBKEY_ENCODED_SIZE];
        scoped_privkey.encode(encoded_key, ANDROID_PUBKEY_ENCODED_SIZE);
        if (avb_memcmp(pubkey, encoded_key, ANDROID_PUBKEY_ENCODED_SIZE) != 0) {
            avb_memcpy((void *)pubkey, encoded_key, ANDROID_PUBKEY_ENCODED_SIZE);
            printf("Pubkey patched.\n");
        }

        if (avb_memcmp(boot_digest, actual_boot_digest, SHA256_DIGEST_LENGTH) != 0) {
            avb_memcpy((void *)boot_digest, actual_boot_digest, SHA256_DIGEST_LENGTH);
            printf("boot digest patched.\n");
        }

        SHA256_Init(&auth_ctx);
        SHA256_Update(&auth_ctx, header, sizeof(AvbVBMetaImageHeader));
        SHA256_Update(&auth_ctx, aux, vbmeta_header.auxiliary_data_block_size);
        SHA256_Final(actual_hash, &auth_ctx);

        uint8_t signature[signature_size];
        scoped_privkey.sign(actual_hash, SHA256_DIGEST_LENGTH, signature);

        avb_memcpy((void *)hash, actual_hash, SHA256_DIGEST_LENGTH);
        printf("Hash patched.\n");

        avb_memcpy((void *)sig, signature, signature_size);
        printf("Signature patched.\n");

        mismatch = false;
    } else if (!mismatch) {
        printf("\nAll tests passed.\n");
    }
    printf("\n");

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
