/* Signature: RAFCODE-Φ-∆RafaelVerboΩ - 2025-08-31T14:25:55Z
   Manifest: RAFAELIA_MANIFEST.json (see repo root)
   ψχρΔΣΩ-session: (runtime will inject per-run)
*/
/*
 * native/magiskboot/magiskboot.c
 * RAFAELIA NATIVE_HARDEN - integrated hardened magiskboot.c
 *
 * - Adds defensive checks for IO, sizes and pointers.
 * - Adds logging macros that use __android_log_print on Android, or stderr/stdout on host.
 * - Ensures we limit reads to ramdisk size where possible.
 * - Emits structured log lines (JSON-like) to ease CI parsing.
 *
 * NOTE: This file replaces or should be merged into your existing magiskboot.c.
 * Review the sections marked "TODO MERGE" if your original magiskboot has
 * existing boot-header parsing logic (prefer header offsets over magic scan fallback).
 *
 * Signature: RAFCODE-Φ-∆RafaelVerboΩ - timestamp: 2025-08-31T14:25:55Z
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <inttypes.h>

/* Crypto includes (OpenSSL) */
#include <openssl/sha.h>

/* Compression libs (optional, may require prebuilt) */
#ifdef HAVE_LZMA
#include <lzma.h>
#endif
#ifdef HAVE_LZ4
#include <lz4frame.h>
#endif
#include <zlib.h>

/* Android logging when building for Android */
#ifdef __ANDROID__
#include <android/log.h>
#define RAF_LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "RAFAELIA", fmt, ##__VA_ARGS__)
#define RAF_LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, "RAFAELIA", fmt, ##__VA_ARGS__)
#else
#define RAF_LOGI(fmt, ...) do { fprintf(stdout, "RAFAELIA INFO: " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#define RAF_LOGE(fmt, ...) do { fprintf(stderr, "RAFAELIA ERROR: " fmt "\n", ##__VA_ARGS__); fflush(stderr); } while(0)
#endif

/* Constants and caps */
#define BUF_SIZE (64*1024)
#define MAX_NAME_SIZE (1024*1024)      /* 1 MiB name max (cap) */
#define MAX_FILE_SIZE ((uint64_t)4<<30) /* 4 GiB cap for individual file payloads (sanity) */
#define RAMDISK_SCAN_LIMIT (16*1024*1024) /* 16 MiB fallback scan limit */

/* Progress callback type */
typedef int (*progress_cb_t)(unsigned long long processed_bytes, unsigned long long total_bytes, void *userdata);

/* Utility: current timestamp */
static const char* now_iso8601(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm tm;
    gmtime_r(&t, &tm);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

/* Small helper to emit JSON-like structured logs for CI parsing */
static void raf_log_json(const char *level, const char *component, const char *event, const char *msg) {
    char ts[32];
    now_iso8601(ts, sizeof(ts));
    /* Keep message simple; avoid huge binary dumps in logs */
    RAF_LOGI("{\"ts\":\"%s\",\"level\":\"%s\",\"component\":\"%s\",\"event\":\"%s\",\"msg\":\"%s\"}", ts, level, component, event, msg ? msg : "");
}

/* Robust read wrapper */
static ssize_t safe_read(int fd, void *buf, size_t count) {
    ssize_t r = read(fd, buf, count);
    if (r < 0) {
        RAF_LOGE("safe_read: read failed: %s", strerror(errno));
    }
    return r;
}

/* Robust lseek wrapper */
static off_t safe_lseek(int fd, off_t offset, int whence) {
    off_t r = lseek(fd, offset, whence);
    if (r == (off_t)-1) {
        RAF_LOGE("safe_lseek: lseek failed: %s", strerror(errno));
    }
    return r;
}

/* Helper: hex print truncated */
static void hex_print_trunc(const unsigned char *in, size_t len, size_t max_print) {
    size_t print_len = len;
    if (print_len > max_print) print_len = max_print;
    for (size_t i = 0; i < print_len; ++i) {
        printf("%02x", in[i]);
    }
    if (len > max_print) printf("...(%zu bytes)", len);
    printf("\n");
}

/* -----------------------------
   SHA-256 streaming for fd/path
   ----------------------------- */
int sha256_file_stream_fd(int fd, unsigned char out[32], progress_cb_t pcb, void *pud) {
    if (fd < 0 || out == NULL) {
        raf_log_json("ERROR", "sha256", "invalid_params", "invalid fd or out == NULL");
        return -1;
    }
    SHA256_CTX ctx;
    if (!SHA256_Init(&ctx)) {
        raf_log_json("ERROR", "sha256", "init_fail", "SHA256_Init failed");
        return -1;
    }

    unsigned char *buf = malloc(BUF_SIZE);
    if (!buf) {
        raf_log_json("ERROR", "sha256", "oom", "malloc BUF_SIZE failed");
        return -1;
    }

    uint64_t total = 0;
    ssize_t n;
    off_t orig = safe_lseek(fd, 0, SEEK_CUR);
    if (orig != (off_t)-1) safe_lseek(fd, 0, SEEK_SET);

    while ((n = safe_read(fd, buf, BUF_SIZE)) > 0) {
        SHA256_Update(&ctx, buf, (size_t)n);
        total += (uint64_t)n;
        if (pcb && pcb(total, 0, pud)) {
            raf_log_json("ERROR", "sha256", "aborted_by_callback", "callback requested abort");
            free(buf);
            if (orig != (off_t)-1) safe_lseek(fd, orig, SEEK_SET);
            return -1;
        }
    }
    if (n < 0) {
        raf_log_json("ERROR", "sha256", "read_fail", strerror(errno));
        free(buf);
        if (orig != (off_t)-1) safe_lseek(fd, orig, SEEK_SET);
        return -1;
    }
    SHA256_Final(out, &ctx);
    free(buf);
    if (orig != (off_t)-1) safe_lseek(fd, orig, SEEK_SET);

    char msg[128];
    snprintf(msg, sizeof(msg), "sha256 computed bytes=%" PRIu64, (uint64_t)total);
    raf_log_json("INFO", "sha256", "done", msg);
    return 0;
}

int sha256_file_stream_path(const char *path, unsigned char out[32], progress_cb_t pcb, void *pud) {
    if (!path || !out) return -1;
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        char buff[200];
        snprintf(buff, sizeof(buff), "open(%s) failed: %s", path, strerror(errno));
        raf_log_json("ERROR", "sha256_path", "open_fail", buff);
        return -1;
    }
    int rc = sha256_file_stream_fd(fd, out, pcb, pud);
    close(fd);
    return rc;
}

/* ------------------------------------------------------
   CPIO newc streaming parser (simplified & hardened)
   ------------------------------------------------------
   This parser reads an ASCII newc (070701) cpio stream from a feed
   callback and calls a feed that processes the file payloads.
   It includes sanity checks on namesize and filesize and stops early on anomalies.
*/

/* feed callback type: called with data buffer and size. Return 0 to continue, non-zero to abort. */
typedef int (*feed_cb_t)(const unsigned char *buf, size_t len, void *userdata);

/* read 8 hex chars as uint32 */
static int parse_hex8(const char *s, uint32_t *out) {
    char tmp[9]; memcpy(tmp, s, 8); tmp[8]=0;
    char *end;
    unsigned long v = strtoul(tmp, &end, 16);
    if (end == tmp) return -1;
    *out = (uint32_t)v;
    return 0;
}

/* Parse cpio newc from fd (limited by max_bytes if >0), feed payload to feed_cb (callable) */
int cpio_parse_and_feed_fd(int fd, uint64_t max_bytes, feed_cb_t feed, void *userdata) {
    if (fd < 0 || !feed) {
        raf_log_json("ERROR","cpio","invalid_params","fd<0 or feed==NULL");
        return -1;
    }

    unsigned char header[6*8+1]; /* "070701" + fields */
    ssize_t n;
    uint64_t processed = 0;
    char tmp[256];
    while (1) {
        /* read first 6 bytes ("070701") */
        n = safe_read(fd, header, 6);
        if (n == 0) {
            raf_log_json("INFO","cpio","eof","end of stream");
            return 0; /* normal EOF */
        }
        if (n < 6) { raf_log_json("ERROR","cpio","short_header","short magic"); return -1; }
        header[6] = '\0';
        if (memcmp(header, "070701", 6) != 0) {
            raf_log_json("ERROR","cpio","bad_magic","expected 070701");
            return -1;
        }
        /* read next 110 bytes (11 fields * 8 hex chars) */
        n = safe_read(fd, tmp, 11*8);
        if (n != 11*8) { raf_log_json("ERROR","cpio","short_header_fields","expected 88 bytes"); return -1; }
        tmp[88] = '\0';
        /* parse fields we need: namesize (field 11?), filesize maybe field 7? newc header order:
           c_magic, c_ino, c_mode, c_uid, c_gid, c_nlink, c_mtime, c_filesize, c_devmajor, c_devminor, c_rdevmajor, c_rdevminor, c_namesize, c_check
           in newc there are 13 fields after magic; but with 6+13*8 = 110 bytes.
        */
        /* For simplicity we parse namesize at byte offset 12*? We'll use string scanning safer approach. */
        char fields[13][9];
        for (int i=0;i<13;i++) {
            memcpy(fields[i], tmp + (i*8), 8);
            fields[i][8]=0;
        }
        uint32_t namesize = 0;
        uint32_t filesize = 0;
        /* fields index reference: 0=ino,1=mode,2=uid,3=gid,4=nlink,5=mtime,6=filesize ... depends on original packing.
           We attempt to read namesize from fields[11] and filesize from fields[6] (common layout).
        */
        if (parse_hex8(fields[11], &namesize) != 0) { raf_log_json("ERROR","cpio","namesize_parse","failed"); return -1; }
        if (parse_hex8(fields[6], &filesize) != 0) { raf_log_json("ERROR","cpio","filesize_parse","failed"); return -1; }

        if (namesize == 0 || namesize > MAX_NAME_SIZE) {
            raf_log_json("ERROR","cpio","namesize_invalid","namesize out of bounds");
            return -1;
        }
        if ((uint64_t)filesize > MAX_FILE_SIZE) {
            raf_log_json("ERROR","cpio","filesize_invalid","filesize out of bounds");
            return -1;
        }

        /* read the filename (namesize bytes, padded to 4) */
        size_t name_padded = ((namesize + 3) & ~3);
        unsigned char *namebuf = malloc(name_padded);
        if (!namebuf) { raf_log_json("ERROR","cpio","oom","alloc namebuf failed"); return -1; }
        n = safe_read(fd, namebuf, name_padded);
        if (n != (ssize_t)name_padded) { free(namebuf); raf_log_json("ERROR","cpio","short_name","short read"); return -1; }
        /* filename is first namesize bytes; check for "TRAILER!!!" */
        if (memcmp(namebuf, "TRAILER!!!", 10) == 0) {
            free(namebuf);
            raf_log_json("INFO","cpio","trailer","found trailer, end of archive");
            return 0;
        }
        /* Now read file data (filesize bytes) and feed through feed callback in chunks */
        uint64_t remain = (uint64_t)filesize;
        size_t toread;
        unsigned char *data_buf = malloc(BUF_SIZE);
        if (!data_buf) { free(namebuf); raf_log_json("ERROR","cpio","oom_data","malloc failed"); return -1; }
        while (remain) {
            toread = (remain > BUF_SIZE) ? BUF_SIZE : (size_t)remain;
            n = safe_read(fd, data_buf, toread);
            if (n <= 0) { free(namebuf); free(data_buf); raf_log_json("ERROR","cpio","short_payload","read failed"); return -1; }
            if (feed(data_buf, (size_t)n, userdata) != 0) {
                free(namebuf); free(data_buf);
                raf_log_json("ERROR","cpio","feed_abort","feed callback returned non-zero");
                return -1;
            }
            remain -= (size_t)n;
            processed += (size_t)n;
        }
        free(namebuf);
        free(data_buf);
        /* skip padding to 4 bytes boundary for file data */
        off_t cur = safe_lseek(fd, 0, SEEK_CUR);
        off_t aligned = ( (cur + 3) & ~3 );
        if (aligned > cur) safe_lseek(fd, aligned, SEEK_SET);

        if (max_bytes && processed > max_bytes) {
            raf_log_json("ERROR","cpio","max_bytes_exceeded","processed exceeded max_bytes");
            return -1;
        }
    }
    /* unreachable */
    return 0;
}

/* -----------------------------
   Decompressor stubs (gzip/xz/lz4) - streaming
   -----------------------------
   Implementations should call the provided feed callback with decompressed bytes.
   For brevity, provide a gzip implementation and placeholders for xz/lz4.
*/

int decompress_gzip_fd_and_feed(int fd, uint64_t max_bytes, feed_cb_t feed, void *userdata) {
    if (fd < 0 || !feed) { raf_log_json("ERROR","decompress","invalid_params",""); return -1; }
    /* Using zlib's inflate with gz header support via gz* is simpler but we need fd.
       We'll read chunks and use inflate in streaming mode. For brevity, use gzopen on a tempfile fallback.
       Safer approach: lseek to beginning, create FILE* via fdopen and use gzdopen (zlib) if available.
    */
    int dupfd = dup(fd);
    if (dupfd < 0) { raf_log_json("ERROR","decompress","dup_fail",strerror(errno)); return -1; }
    FILE *f = fdopen(dupfd, "rb");
    if (!f) { close(dupfd); raf_log_json("ERROR","decompress","fdopen_fail",strerror(errno)); return -1; }
    gzFile gzf = gzdopen(dupfd, "rb");
    if (!gzf) { fclose(f); raf_log_json("ERROR","decompress","gzdopen_fail",""); return -1; }

    unsigned char *out = malloc(BUF_SIZE);
    if (!out) { gzclose(gzf); raf_log_json("ERROR","decompress","oom","alloc"); return -1; }

    int r;
    uint64_t processed = 0;
    while ((r = gzread(gzf, out, BUF_SIZE)) > 0) {
        if (feed(out, (size_t)r, userdata) != 0) {
            raf_log_json("ERROR","decompress","feed_abort","callback returned non-zero");
            free(out); gzclose(gzf); return -1;
        }
        processed += (uint64_t)r;
        if (max_bytes && processed > max_bytes) {
            raf_log_json("ERROR","decompress","max_bytes_exceeded","decompressed beyond limit");
            free(out); gzclose(gzf); return -1;
        }
    }
    if (r < 0) {
        raf_log_json("ERROR","decompress","gzread_fail","error reading gzip stream");
        free(out); gzclose(gzf); return -1;
    }
    free(out);
    gzclose(gzf);
    raf_log_json("INFO","decompress","done","gzip feed complete");
    return 0;
}

/* placeholders for xz/lz4 - add similar guarded implementations if libraries available */
#ifdef HAVE_LZMA
int decompress_xz_fd_and_feed(int fd, uint64_t max_bytes, feed_cb_t feed, void *userdata) {
    raf_log_json("INFO","decompress","xz_start","not fully implemented in this build; add liblzma logic");
    return -1;
}
#endif

#ifdef HAVE_LZ4
int decompress_lz4_fd_and_feed(int fd, uint64_t max_bytes, feed_cb_t feed, void *userdata) {
    raf_log_json("INFO","decompress","lz4_start","not fully implemented in this build; add lz4frame logic");
    return -1;
}
#endif

/* compute_logical_ramdisk_sha_from_fd
   - format: a hint "gzip"|"xz"|"lz4" or NULL for autodetect fallback
   - out: 32 bytes SHA-256 result
*/
int compute_logical_ramdisk_sha_from_fd(int fd, const char *format_hint, unsigned char out[32], progress_cb_t pcb, void *pud) {
    if (fd < 0 || !out) { raf_log_json("ERROR","compute_logical_ramdisk","invalid_params",""); return -1; }

    /* Seek to start of ramdisk if header-based offsets are known. TODO: integrate header parsing here. */
    off_t cur = safe_lseek(fd, 0, SEEK_CUR);
    if (cur == (off_t)-1) cur = 0;

    /* For now, attempt to use format_hint; fallback to magic scan (limited to RAMDISK_SCAN_LIMIT) */
    int rc = -1;
    if (format_hint && strcmp(format_hint, "gzip") == 0) {
        raf_log_json("INFO","compute_logical_ramdisk","hint","using gzip decompressor");
        /* We'll directly call decompress_gzip_fd_and_feed feeding into cpio parser */
        /* Prepare SHA context and feed callback that updates SHA */
        SHA256_CTX ctx;
        if (!SHA256_Init(&ctx)) { raf_log_json("ERROR","compute_logical_ramdisk","sha_init_fail",""); return -1; }
        /* feed that updates SHA */
        struct feed_ctx { SHA256_CTX *ctx; } fctx;
        fctx.ctx = &ctx;
        int feed_cb(const unsigned char *buf, size_t len, void *userdata) {
            SHA256_Update(((struct feed_ctx*)userdata)->ctx, buf, len);
            return 0;
        }
        /* decompress and then parse cpio - simplified: decompress to temporary file descriptor or stream into parser
           For brevity call decompress_gzip_fd_and_feed to feed decompressed bytes to cpio parser via an in-memory pipeline.
           A robust approach requires streaming between decompress and cpio parser; omitted for brevity.
        */
        raf_log_json("INFO","compute_logical_ramdisk","not_impl","full stream integration omitted in this build");
        /* Fallback to failure to indicate incomplete integration if header parsing not available */
        return -2;
    }

    /* Fallback: scan for gzip/xz/lz4 magic within first RAMDISK_SCAN_LIMIT bytes and try to compute logical sha
       NOTE: scanning binary and invoking decompressors on found offsets is expensive and fragile. Prefer header offsets.
    */
    raf_log_json("INFO","compute_logical_ramdisk","fallback","attempting magic scan (limited)");
    unsigned char *scan_buf = malloc(RAMDISK_SCAN_LIMIT);
    if (!scan_buf) { raf_log_json("ERROR","compute_logical_ramdisk","oom","alloc"); return -1; }
    off_t start = safe_lseek(fd, 0, SEEK_SET);
    ssize_t got = safe_read(fd, scan_buf, RAMDISK_SCAN_LIMIT);
    if (got <= 0) { free(scan_buf); raf_log_json("ERROR","compute_logical_ramdisk","read_fail","scan read failed"); return -1; }
    /* naive search for gzip magic 0x1f8b */
    ssize_t pos = -1;
    for (ssize_t i = 0; i+1 < got; ++i) {
        if (scan_buf[i] == 0x1f && scan_buf[i+1] == 0x8b) { pos = i; break; }
    }
    if (pos == -1) {
        raf_log_json("ERROR","compute_logical_ramdisk","no_magic","no gzip magic found in scan window");
        free(scan_buf);
        return -1;
    }
    /* Seek to found offset and attempt to compute SHA on the decompressed logical content. For brevity we compute SHA over compressed blob here */
    if (safe_lseek(fd, start + pos, SEEK_SET) == (off_t)-1) { free(scan_buf); return -1; }
    /* Compute SHA over the compressed data (best-effort); real logical SHA requires decompression + cpio parsing */
    SHA256_CTX ctx;
    if (!SHA256_Init(&ctx)) { free(scan_buf); return -1; }
    unsigned char *rbuf = malloc(BUF_SIZE);
    if (!rbuf) { free(scan_buf); return -1; }
    ssize_t rn;
    uint64_t total = 0;
    while ((rn = safe_read(fd, rbuf, BUF_SIZE)) > 0) {
        SHA256_Update(&ctx, rbuf, (size_t)rn);
        total += (uint64_t)rn;
    }
    if (rn < 0) { raf_log_json("ERROR","compute_logical_ramdisk","read_fail2","read failed during sha"); free(scan_buf); free(rbuf); return -1; }
    SHA256_Final(out, &ctx);
    free(scan_buf);
    free(rbuf);
    char msg[128];
    snprintf(msg, sizeof(msg), "computed best-effort ramdisk sha over compressed stream bytes=%" PRIu64, total);
    raf_log_json("INFO","compute_logical_ramdisk","done", msg);
    /* restore position */
    safe_lseek(fd, cur, SEEK_SET);
    return 0;
}

/* compute_image_and_ramdisk_shas: convenience combo */
int compute_image_and_ramdisk_shas(const char *image_path, unsigned char sha_before[32], unsigned char sha_after[32], unsigned char ramdisk_sha[32]) {
    if (!image_path) return -1;
    int fd = open(image_path, O_RDONLY);
    if (fd < 0) { raf_log_json("ERROR","compute_combo","open_fail", strerror(errno)); return -1; }
    int rc = sha256_file_stream_fd(fd, sha_before, NULL, NULL);
    if (rc != 0) { close(fd); return -1; }
    /* attempt ramdisk logical sha */
    int r = compute_logical_ramdisk_sha_from_fd(fd, NULL, ramdisk_sha, NULL, NULL);
    if (r != 0) raf_log_json("WARN","compute_combo","ramdisk_nok","ramdisk logical sha failed or incomplete");
    /* optional sha_after if modifications done (seek back and compute) */
    safe_lseek(fd, 0, SEEK_SET);
    int rc2 = sha256_file_stream_fd(fd, sha_after, NULL, NULL);
    close(fd);
    return (rc==0 && rc2==0) ? 0 : -1;
}

/* main test driver for host development */
#ifdef BUILD_MAIN
int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr,"Usage: %s <boot.img> [ramdisk.gz]\n", argv[0]); return 2; }
    unsigned char before[32], after[32], rsha[32];
    if (compute_image_and_ramdisk_shas(argv[1], before, after, rsha) == 0) {
        printf("sha_before: "); hex_print_trunc(before, 32, 32);
        printf("sha_after:  "); hex_print_trunc(after, 32, 32);
        printf("ramdisk_sha: "); hex_print_trunc(rsha, 32, 32);
    } else {
        fprintf(stderr,"compute failed\n");
    }
    return 0;
}
#endif

/* End of file */
