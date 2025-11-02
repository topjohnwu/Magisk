 url=https://github.com/rafaelmeloreisnovo/Magisk_Rafaelia/blob/main/native/tests/test_sha.c
/*
 * native/tests/test_sha.c
 * Improved harness with argument checks and descriptive output.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

extern int sha256_file_stream_path(const char *path, unsigned char out[32], int (*pcb)(unsigned long long, unsigned long long, void*), void *pud);
extern int compute_logical_ramdisk_sha_from_fd(int fd, const char *format, unsigned char out[32], int (*pcb)(unsigned long long, unsigned long long, void*), void *pud);

static void print_hex(const unsigned char *in, size_t len) {
    for (size_t i=0;i<len;++i) printf("%02x", in[i]);
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr,"Usage: %s <fixture_boot.img> [ramdisk.gz]\n", argv[0]); return 2; }
    unsigned char digest[32];
    int rc = sha256_file_stream_path(argv[1], digest, NULL, NULL);
    if (rc != 0) { fprintf(stderr, "sha256_file_stream_path failed\n"); return 3; }
    printf("sha256(%s): ", argv[1]); print_hex(digest,32);

    if (argc >= 3) {
        int fd = open(argv[2], O_RDONLY);
        if (fd < 0) { perror("open ramdisk"); return 4; }
        const char *fmt = "gzip";
        unsigned char rsha[32];
        int r = compute_logical_ramdisk_sha_from_fd(fd, fmt, rsha, NULL, NULL);
        close(fd);
        if (r == 0) { printf("logical ramdisk sha(%s): ", argv[2]); print_hex(rsha,32); }
        else { fprintf(stderr,"logical ramdisk sha failed (%d)\n", r); }
    }
    printf("native tests done\n");
    return 0;
}
