#include <base.hpp>

#include "boot-rs.hpp"
#include "magiskboot.hpp"

using namespace std;

static void print_formats() {
    for (int fmt = +FileFormat::GZIP; fmt < +FileFormat::LZOP; ++fmt) {
        fprintf(stderr, "%s ", fmt2name[(FileFormat) fmt]);
    }
}

static void usage(char *arg0) {
    fprintf(stderr,
R"EOF(MagiskBoot - Boot Image Modification Tool

Usage: %s <action> [args...]

Supported actions:
  unpack [-n] [-h] <bootimg>
    Unpack <bootimg> to its individual components, each component to
    a file with its corresponding file name in the current directory.
    Supported components: kernel, kernel_dtb, ramdisk.cpio, second,
    dtb, extra, and recovery_dtbo.
    By default, each component will be decompressed on-the-fly.
    If '-n' is provided, all decompression operations will be skipped;
    each component will remain untouched, dumped in its original format.
    If '-h' is provided, the boot image header information will be
    dumped to the file 'header', which can be used to modify header
    configurations during repacking.
    Return values:
    0:valid    1:error    2:chromeos

  repack [-n] <origbootimg> [outbootimg]
    Repack boot image components using files from the current directory
    to [outbootimg], or 'new-boot.img' if not specified. Current directory
    should only contain required files for [outbootimg], or incorrect
    [outbootimg] may be produced.
    <origbootimg> is the original boot image used to unpack the components.
    By default, each component will be automatically compressed using its
    corresponding format detected in <origbootimg>. If a component file
    in the current directory is already compressed, then no addition
    compression will be performed for that specific component.
    If '-n' is provided, all compression operations will be skipped.
    If env variable PATCHVBMETAFLAG is set to true, all disable flags in
    the boot image's vbmeta header will be set.

  verify <bootimg> [x509.pem]
    Check whether the boot image is signed with AVB 1.0 signature.
    Optionally provide a certificate to verify whether the image is
    signed by the public key certificate.
    Return value:
    0:valid    1:error

  sign <bootimg> [name] [x509.pem pk8]
    Sign <bootimg> with AVB 1.0 signature.
    Optionally provide the name of the image (default: '/boot').
    Optionally provide the certificate/private key pair for signing.
    If the certificate/private key pair is not provided, the AOSP
    verity key bundled in the executable will be used.

  extract <payload.bin> [partition] [outfile]
    Extract [partition] from <payload.bin> to [outfile].
    If [outfile] is not specified, then output to '[partition].img'.
    If [partition] is not specified, then attempt to extract either
    'init_boot' or 'boot'. Which partition was chosen can be determined
    by whichever 'init_boot.img' or 'boot.img' exists.
    <payload.bin> can be '-' to be STDIN.

  hexpatch <file> <hexpattern1> <hexpattern2>
    Search <hexpattern1> in <file>, and replace it with <hexpattern2>

  cpio <incpio> [commands...]
    Do cpio commands to <incpio> (modifications are done in-place).
    Each command is a single argument; add quotes for each command.
    See "cpio --help" for supported commands.

  dtb <file> <action> [args...]
    Do dtb related actions to <file>.
    See "dtb --help" for supported actions.

  split [-n] <file>
    Split image.*-dtb into kernel + kernel_dtb.
    If '-n' is provided, decompression operations will be skipped;
    the kernel will remain untouched, split in its original format.

  sha1 <file>
    Print the SHA1 checksum for <file>

  cleanup
    Cleanup the current working directory

  compress[=format] <infile> [outfile]
    Compress <infile> with [format] to [outfile].
    <infile>/[outfile] can be '-' to be STDIN/STDOUT.
    If [format] is not specified, then gzip will be used.
    If [outfile] is not specified, then <infile> will be replaced
    with another file suffixed with a matching file extension.
    Supported formats: )EOF", arg0);

    print_formats();

    fprintf(stderr, R"EOF(

  decompress <infile> [outfile]
    Detect format and decompress <infile> to [outfile].
    <infile>/[outfile] can be '-' to be STDIN/STDOUT.
    If [outfile] is not specified, then <infile> will be replaced
    with another file removing its archive format file extension.
    Supported formats: )EOF");

    print_formats();

    fprintf(stderr, "\n\n");
    exit(1);
}

static void decompress(char *infile, const char *outfile) {
    bool in_std = infile == "-"sv;
    bool rm_in = false;

    int in_fd = in_std ? STDIN_FILENO : xopen(infile, O_RDONLY);
    int out_fd = -1;

    uint8_t buf[4096];
    size_t len = read(in_fd, buf, sizeof(buf));
    FileFormat type = check_fmt(buf, len);

    fprintf(stderr, "Detected format: [%s]\n", fmt2name[type]);

    if (!COMPRESSED(type))
        LOGE("Input file is not a supported compressed type!\n");

    // If user does not provide outfile, infile has to be either
    // <path>.[ext], or '-'. Outfile will be either <path> or '-'.
    // If the input does not have proper format, abort.

    char *ext = nullptr;
    if (outfile == nullptr) {
        outfile = infile;
        if (!in_std) {
            ext = strrchr(infile, '.');
            if (ext == nullptr || strcmp(ext, fmt2ext[type]) != 0)
                LOGE("Input file is not a supported type!\n");

            // Strip out extension and remove input
            *ext = '\0';
            rm_in = true;
            fprintf(stderr, "Decompressing to [%s]\n", outfile);
        }
    }

    out_fd = outfile == "-"sv ?
             STDOUT_FILENO :
             xopen(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (ext) *ext = '.';

    decompress_bytes_fd(type, byte_view{ buf, len }, in_fd, out_fd);

    if (in_fd != STDIN_FILENO) close(in_fd);
    if (out_fd != STDOUT_FILENO) close(out_fd);

    if (rm_in)
        unlink(infile);
}

static void compress(const char *method, const char *infile, const char *outfile) {
    FileFormat fmt = name2fmt[method];
    if (fmt == FileFormat::UNKNOWN)
        LOGE("Unknown compression method: [%s]\n", method);

    bool in_std = infile == "-"sv;
    bool rm_in = false;

    int in_fd = in_std ? STDIN_FILENO : xopen(infile, O_RDONLY);
    int out_fd = -1;

    if (outfile == nullptr) {
        if (in_std) {
            out_fd = STDOUT_FILENO;
        } else {
            // If user does not provide outfile and infile is not
            // STDIN, output to <infile>.[ext]
            string tmp(infile);
            tmp += fmt2ext[fmt];
            out_fd = xopen(tmp.data(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            fprintf(stderr, "Compressing to [%s]\n", tmp.data());
            rm_in = true;
        }
    } else {
        out_fd = outfile == "-"sv ?
                 STDOUT_FILENO :
                 xopen(outfile,  O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    compress_fd(fmt, in_fd, out_fd);

    if (in_fd != STDIN_FILENO) close(in_fd);
    if (out_fd != STDOUT_FILENO) close(out_fd);

    if (rm_in)
        unlink(infile);
}

int main(int argc, char *argv[]) {
    cmdline_logging();
    umask(0);

    if (argc < 2)
        usage(argv[0]);

    // Skip '--' for backwards compatibility
    string_view action(argv[1]);
    if (str_starts(action, "--"))
        action = argv[1] + 2;

    if (action == "cleanup") {
        fprintf(stderr, "Cleaning up...\n");
        unlink(HEADER_FILE);
        unlink(KERNEL_FILE);
        unlink(RAMDISK_FILE);
        unlink(SECOND_FILE);
        unlink(KER_DTB_FILE);
        unlink(EXTRA_FILE);
        unlink(RECV_DTBO_FILE);
        unlink(DTB_FILE);
        unlink(BOOTCONFIG_FILE);
        rm_rf(VND_RAMDISK_DIR);
    } else if (argc > 2 && action == "sha1") {
        uint8_t sha1[20];
        {
            mmap_data m(argv[2]);
            sha1_hash(m, byte_data(sha1, sizeof(sha1)));
        }
        for (uint8_t i : sha1)
            printf("%02x", i);
        printf("\n");
    } else if (argc > 2 && action == "split") {
        if (argv[2] == "-n"sv) {
            if (argc == 3)
                usage(argv[0]);
            return split_image_dtb(argv[3], true);
        } else {
            return split_image_dtb(argv[2]);
        }
    } else if (argc > 2 && action == "unpack") {
        int idx = 2;
        bool nodecomp = false;
        bool hdr = false;
        for (;;) {
            if (idx >= argc)
                usage(argv[0]);
            if (argv[idx][0] != '-')
                break;
            for (char *flag = &argv[idx][1]; *flag; ++flag) {
                if (*flag == 'n')
                    nodecomp = true;
                else if (*flag == 'h')
                    hdr = true;
                else
                    usage(argv[0]);
            }
            ++idx;
        }
        return unpack(argv[idx], nodecomp, hdr);
    } else if (argc > 2 && action == "repack") {
        if (argv[2] == "-n"sv) {
            if (argc == 3)
                usage(argv[0]);
            repack(argv[3], argv[4] ? argv[4] : NEW_BOOT, true);
        } else {
            repack(argv[2], argv[3] ? argv[3] : NEW_BOOT);
        }
    } else if (argc > 2 && action == "verify") {
        return verify(argv[2], argv[3]);
    } else if (argc > 2 && action == "sign") {
        if (argc == 5) usage(argv[0]);
        return sign(
                argv[2],
                argc > 3 ? argv[3] : "/boot",
                argc > 5 ? argv[4] : nullptr,
                argc > 5 ? argv[5] : nullptr);
    } else if (argc > 2 && action == "decompress") {
        decompress(argv[2], argv[3]);
    } else if (argc > 2 && str_starts(action, "compress")) {
        compress(action[8] == '=' ? &action[9] : "gzip", argv[2], argv[3]);
    } else if (argc > 4 && action == "hexpatch") {
        return hexpatch(byte_view(argv[2]), byte_view(argv[3]), byte_view(argv[4])) ? 0 : 1;
    } else if (argc > 2 && action == "cpio") {
        return rust::cpio_commands(argc - 2, argv + 2) ? 0 : 1;
    } else if (argc > 2 && action == "dtb") {
        return rust::dtb_commands(argc - 2, argv + 2) ? 0 : 1;
    } else if (argc > 2 && action == "extract") {
        return rust::extract_boot_from_payload(
                argv[2],
                argc > 3 ? argv[3] : "",
                argc > 4 ? argv[4] : ""
                ) ? 0 : 1;
    } else {
        usage(argv[0]);
    }

    return 0;
}
