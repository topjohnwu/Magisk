#define AVB_VBMETA_IMAGE_HEADER_SIZE 256
#define AVB_VBMETA_IMAGE_BLOCK_SIZE  4096

struct vbmeta_img {
    // Memory map of the whole image
    uint8_t *map_addr;
    size_t map_size;

    vbmeta_img(const char *);
    ~vbmeta_img();
};