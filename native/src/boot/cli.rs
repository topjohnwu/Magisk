use crate::cpio::{cpio_commands, print_cpio_usage};
use crate::dtb::{dtb_commands, print_dtb_usage, DtbAction};
use crate::ffi::{
    cleanup, compress, decompress_raw, formats, repack, sign, split_image_dtb, unpack, verify
};
use crate::patch::hexpatch;
use crate::payload::extract_boot_from_payload;
use crate::sign::sha1_hash;
use argh::FromArgs;
use base::{
    cmdline_logging, libc::umask, log_err, map_args, raw_cstr, EarlyExitExt, LoggedResult,
    MappedFile, ResultExt, Utf8CStr,
};
use std::ffi::c_char;

#[derive(FromArgs)]
struct Cli {
    #[argh(subcommand)]
    action: Action,
}

#[derive(FromArgs)]
#[argh(subcommand)]
enum Action {
    Unpack(Unpack),
    Repack(Repack),
    Verify(Verify),
    Sign(Sign),
    Extract(Extract),
    HexPatch(HexPatch),
    Cpio(Cpio),
    Dtb(Dtb),
    Split(Split),
    Sha1(Sha1),
    Cleanup(Cleanup),
    Compress(Compress),
    Decompress(Decompress),
}

#[derive(FromArgs)]
#[argh(subcommand, name = "unpack")]
struct Unpack {
    #[argh(switch, short = 'n')]
    no_decompress: bool,
    #[argh(switch, short = 'h')]
    dump_header: bool,
    #[argh(positional)]
    img: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "repack")]
struct Repack {
    #[argh(switch, short = 'n')]
    no_compress: bool,
    #[argh(positional)]
    img: String,
    #[argh(positional, default = r#""new-boot.img".to_string()"#)]
    out: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "verify")]
struct Verify {
    #[argh(positional)]
    img: String,
    #[argh(positional)]
    cert: Option<String>,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "sign")]
struct Sign {
    #[argh(positional)]
    img: String,
    #[argh(positional)]
    args: Vec<String>,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "extract")]
struct Extract {
    #[argh(positional)]
    payload: String,
    #[argh(positional)]
    args: Vec<String>,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "hexpatch")]
struct HexPatch {
    #[argh(positional)]
    file: String,
    #[argh(positional)]
    src: String,
    #[argh(positional)]
    dest: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "cpio")]
struct Cpio {
    #[argh(positional)]
    file: String,
    #[argh(positional)]
    cmds: Vec<String>,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "dtb")]
struct Dtb {
    #[argh(positional)]
    file: String,
    #[argh(subcommand)]
    action: DtbAction,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "split")]
struct Split {
    #[argh(switch, short = 'n')]
    no_decompress: bool,
    #[argh(positional)]
    file: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "sha1")]
struct Sha1 {
    #[argh(positional)]
    file: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "cleanup")]
struct Cleanup {}

#[derive(FromArgs)]
#[argh(subcommand, name = "compress")]
struct Compress {
    #[argh(option, short = 'f', default = r#""gzip".to_string()"#)]
    format: String,
    #[argh(positional)]
    file: String,
    #[argh(positional)]
    out: Option<String>,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "decompress")]
struct Decompress {
    #[argh(positional)]
    file: String,
    #[argh(positional)]
    out: Option<String>,
}

fn print_usage(cmd: &str) {
    eprintln!(
        r#"MagiskBoot - Boot Image Modification Tool

Usage: {} <action> [args...]

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
    Supported formats:

    {1}

  decompress <infile> [outfile]
    Detect format and decompress <infile> to [outfile].
    <infile>/[outfile] can be '-' to be STDIN/STDOUT.
    If [outfile] is not specified, then <infile> will be replaced
    with another file removing its archive format file extension.
    Supported formats:

    {1}
"#,
        cmd,
        formats()
    );
}

#[no_mangle]
pub unsafe extern "C" fn main(
    argc: i32,
    argv: *const *const c_char,
    _envp: *const *const c_char,
) -> i32 {
    cmdline_logging();
    umask(0);
    let res: LoggedResult<()> = try {
        let mut cmds = map_args(argc, argv)?;
        if argc < 2 {
            print_usage(cmds.first().unwrap_or(&"magiskboot"));
            return 1;
        }

        if cmds[1].starts_with("--") {
            cmds[1] = &cmds[1][2..];
        }

        if let Some(fmt) = str::strip_prefix(cmds[1], "compress=") {
            cmds.insert(1, "compress");
            cmds.insert(2, "-f");
            cmds[3] = fmt;
        }

        let mut cli = Cli::from_args(&[cmds[0]], &cmds[1..]).on_early_exit(|| match cmds.get(1) {
            Some(&"dtb") => print_dtb_usage(),
            Some(&"cpio") => print_cpio_usage(),
            _ => print_usage(cmds[0]),
        });
        match cli.action {
            Action::Unpack(Unpack {
                no_decompress,
                dump_header,
                ref mut img,
            }) => {
                return unpack(
                    Utf8CStr::from_string(img).as_ptr(),
                    no_decompress,
                    dump_header,
                );
            }
            Action::Repack(Repack {
                no_compress,
                ref mut img,
                ref mut out,
            }) => {
                repack(
                    Utf8CStr::from_string(img).as_ptr(),
                    Utf8CStr::from_string(out).as_ptr(),
                    no_compress,
                );
            }
            Action::Verify(Verify {
                ref mut img,
                ref mut cert,
            }) => {
                return verify(Utf8CStr::from_string(img).as_ptr(), cert.as_mut().map(|x| Utf8CStr::from_string(x).as_ptr()).unwrap_or(std::ptr::null()));
            }
            Action::Sign(Sign {
                ref mut img,
                ref mut args,
            }) => {
                let (pem, pk8) = match args.get_mut(1..=2) {
                    Some([pem,pk8]) => (Utf8CStr::from_string(pem).as_ptr(), Utf8CStr::from_string(pk8).as_ptr()),
                    _ => (std::ptr::null(), std::ptr::null()),
                };
                return sign(
                    Utf8CStr::from_string(img).as_ptr(),
                    args.first_mut()
                        .map(|x| Utf8CStr::from_string(x).as_ptr())
                        .unwrap_or(raw_cstr!("/boot")),
                    pem, pk8
                )
            }
            Action::Extract(Extract {
                ref payload,
                ref args,
            }) => {
                if args.len() > 2 {
                    Err(log_err!("Too many arguments"))?;
                }
                extract_boot_from_payload(
                    payload,
                    args.first().map(|x| x.as_str()),
                    args.get(1).map(|x| x.as_str()),
                )
                .log_with_msg(|w| w.write_str("Failed to extract from payload"))?;
            }
            Action::HexPatch(HexPatch {
                ref mut file,
                ref mut src,
                ref mut dest,
            }) => {
                if !hexpatch(file, Utf8CStr::from_string(src), Utf8CStr::from_string(dest)) {
                    Err(log_err!("Failed to patch"))?;
                }
            }
            Action::Cpio(Cpio {
                ref mut file,
                ref mut cmds,
            }) => {
                return if cpio_commands(file, cmds)
                    .log_with_msg(|w| w.write_str("Failed to process cpio"))
                    .is_ok()
                {
                    0
                } else {
                    1
                }
            }
            Action::Dtb(Dtb {
                ref mut file,
                ref action,
            }) => {
                return if dtb_commands(file, action)
                    .log_with_msg(|w| w.write_str("Failed to process dtb"))
                    .unwrap_or(false)
                {
                    0
                } else {
                    1
                }
            }
            Action::Split(Split {
                no_decompress,
                ref mut file,
            }) => {
                return split_image_dtb(Utf8CStr::from_string(file).as_ptr(), no_decompress);
            }
            Action::Sha1(Sha1 { ref mut file }) => {
                let file = MappedFile::open(Utf8CStr::from_string(file))?;
                let mut sha1 = [0u8; 20];
                sha1_hash(file.as_ref(), &mut sha1);
                for byte in &sha1 {
                    print!("{:02x}", byte);
                }
                println!();
            }
            Action::Cleanup(_) => {
                eprintln!("Cleaning up...");
                cleanup();
            }
            Action::Decompress(Decompress {
                ref mut file,
                ref mut out,
            }) => {
                decompress_raw(
                    Utf8CStr::from_string(file).as_mut_ptr(),
                    out.as_mut()
                        .map(|x| Utf8CStr::from_string(x).as_ptr())
                        .unwrap_or(std::ptr::null()),
                );
            }
            Action::Compress(Compress {
                ref mut file,
                ref mut format,
                ref mut out,
            }) => {
                compress(
                    Utf8CStr::from_string(format).as_ptr(),
                    Utf8CStr::from_string(file).as_ptr(),
                    out.as_mut()
                        .map(|x| Utf8CStr::from_string(x).as_ptr())
                        .unwrap_or(std::ptr::null()),
                );
            }
        }
    };
    if res.is_ok() {
        0
    } else {
        1
    }
}
