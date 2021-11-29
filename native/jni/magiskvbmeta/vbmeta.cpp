#include <functional>
#include <memory>

#include <inttypes.h>
#include <utils.hpp>

#include "vbmeta.hpp"
#include "bootimg.hpp"
#include "magiskvbmeta.hpp"

vbmeta_img::vbmeta_img(const char *image) {
    mmap_ro(image, map_addr, map_size);
}

vbmeta_img::~vbmeta_img() {
    munmap(map_addr, map_size);
}

// https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/avbtool.py#203
uint64_t round_to_multiple(uint64_t number, uint16_t size) {
    uint16_t remainder = number % size;
    if (remainder == 0) {
        return number;
    }
    return number + size - remainder;
}

// https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/libavb/avb_vbmeta_image.h#66
// https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/avbtool.py#2332
// https://android.googlesource.com/platform/external/avb/+/refs/tags/android-12.0.0_r12/avbtool.py#732
uint64_t calculate_size(AvbVBMetaImageHeader *header) {
    auto authentication_data_block_size = __builtin_bswap64(header->authentication_data_block_size);
    auto auxiliary_data_block_size = __builtin_bswap64(header->auxiliary_data_block_size);
    return round_to_multiple(AVB_VBMETA_IMAGE_HEADER_SIZE + authentication_data_block_size + auxiliary_data_block_size, AVB_VBMETA_IMAGE_BLOCK_SIZE);
}

int test(const char *image) {
    vbmeta_img vbmeta(image);
    auto header = reinterpret_cast<AvbVBMetaImageHeader*>(vbmeta.map_addr);
    if (memcmp(header->magic, AVB_MAGIC, AVB_MAGIC_LEN) == 0) {
        auto flags = __builtin_bswap32(header->flags);
        if (flags == 0) {
        } else if (flags == 3) {
            return PATCHED;
        } else {
            return OTHER;
        }
    } else {
        return INVALID;
    }
    return 0;
}

int size(const char *image) {
    vbmeta_img vbmeta(image);
    auto header = reinterpret_cast<AvbVBMetaImageHeader*>(vbmeta.map_addr);
    if (memcmp(header->magic, AVB_MAGIC, AVB_MAGIC_LEN) == 0) {
        auto vbmeta_size = calculate_size(header);
        printf("%" PRIu64 "\n", vbmeta_size);
    } else {
        return INVALID;
    }
    return 0;
}

int patch(const char *src_img, const char *out_img) {
    vbmeta_img vbmeta(src_img);
    auto header = reinterpret_cast<AvbVBMetaImageHeader*>(vbmeta.map_addr);
    if (memcmp(header->magic, AVB_MAGIC, AVB_MAGIC_LEN) == 0) {
        auto flags = __builtin_bswap32(header->flags);
        if (flags == 0) {
            fprintf(stderr, "Patch to vbmeta image: [%s]\n", out_img);
            header->flags = __builtin_bswap32(3);

            auto vbmeta_size = calculate_size(header);

            // Create new image
            int fd = creat(out_img, 0644);
            xwrite(fd, header, vbmeta_size);
            close(fd);
        } else if (flags == 3) {
            return PATCHED;
        } else {
            return OTHER;
        }
    } else {
        return INVALID;
    }
    return 0;
}
