use crate::compress::{compress_cmd, decompress_cmd};
use crate::cpio::{cpio_commands, print_cpio_usage};
use crate::dtb::{DtbAction, dtb_commands, print_dtb_usage};
use crate::ffi::{BootImage, FileFormat, cleanup, repack, split_image_dtb, unpack};
use crate::patch::hexpatch;
use crate::payload::extract_boot_from_payload;
use crate::sign::{sha1_hash, sign_boot_image};
use argh::{CommandInfo, EarlyExit, FromArgs, SubCommand};
use base::{
    CmdArgs, EarlyExitExt, LoggedResult, MappedFile, PositionalArgParser, ResultExt, Utf8CStr,
    WriteExt, cmdline_logging, cstr, libc, libc::umask, log_err,
};
use std::ffi::c_char;
use std::io::{Seek, SeekFrom, Write};
use std::str::FromStr;

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

struct Extract {
    payload: String,
    partition: Option<String>,
    outfile: Option<String>,
}

impl FromArgs for Extract {
    fn from_args(_command_name: &[&str], args: &[&str]) -> Result<Self, EarlyExit> {
        let mut parse = PositionalArgParser(args.iter());
        Ok(Extract {
            payload: parse.required("payload.bin")?,
            partition: parse.optional(),
            outfile: parse.last_optional()?,
        })
    }
}

impl SubCommand for Extract {
    const COMMAND: &'static CommandInfo = &CommandInfo {
        name: "extract",
        description: "",
    };
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

struct Compress {
    format: FileFormat,
    file: String,
    out: Option<String>,
}

impl FromArgs for Compress {
    fn from_args(command_name: &[&str], args: &[&str]) -> Result<Self, EarlyExit> {
        let cmd = command_name.last().copied().unwrap_or_default();
        let fmt = cmd.strip_prefix("compress=").unwrap_or("gzip");

        let Ok(fmt) = FileFormat::from_str(fmt) else {
            return Err(EarlyExit::from(format!(
                "Unsupported or unknown compression format: {fmt}\n"
            )));
        };

        let mut iter = PositionalArgParser(args.iter());
        Ok(Compress {
            format: fmt,
            file: iter.required("infile")?,
            out: iter.last_optional()?,
        })
    }
}

impl SubCommand for Compress {
    const COMMAND: &'static CommandInfo = &CommandInfo {
        name: "compress",
        description: "",
    };
}

struct Decompress {
    file: String,
    out: Option<String>,
}

impl FromArgs for Decompress {
    fn from_args(_command_name: &[&str], args: &[&str]) -> Result<Self, EarlyExit> {
        let mut iter = PositionalArgParser(args.iter());
        Ok(Decompress {
            file: iter.required("infile")?,
            out: iter.last_optional()?,
        })
    }
}

impl SubCommand for Decompress {
    const COMMAND: &'static CommandInfo = &CommandInfo {
        name: "decompress",
        description: "",
    };
}

fn print_usage(cmd: &str) {
    eprintln!(
        r#"MagiskBoot - Boot Image Modification Tool

Usage: {0} <action> [args...]

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
        FileFormat::formats()
    );
}

fn verify_cmd(image: &Utf8CStr, cert: Option<&Utf8CStr>) -> bool {
    let image = BootImage::new(image);
    match cert {
        None => {
            // Boot image parsing already checks if the image is signed
            image.is_signed()
        }
        Some(_) => {
            // Provide a custom certificate and re-verify
            image.verify(cert).is_ok()
        }
    }
}

fn sign_cmd(
    image: &Utf8CStr,
    name: Option<&Utf8CStr>,
    cert: Option<&Utf8CStr>,
    key: Option<&Utf8CStr>,
) -> LoggedResult<()> {
    let img = BootImage::new(image);
    let name = name.unwrap_or(cstr!("/boot"));
    let sig = sign_boot_image(img.payload(), name, cert, key)?;
    let tail_off = img.tail_off();
    drop(img);
    let mut fd = image.open(libc::O_WRONLY | libc::O_CLOEXEC)?;
    fd.seek(SeekFrom::Start(tail_off))?;
    fd.write_all(&sig)?;
    let current = fd.stream_position()?;
    let eof = fd.seek(SeekFrom::End(0))?;
    if eof > current {
        // Zero out rest of the file
        fd.seek(SeekFrom::Start(current))?;
        fd.write_zeros((eof - current) as usize)?;
    }
    Ok(())
}

fn boot_main(cmds: CmdArgs) -> LoggedResult<i32> {
    let mut cmds = cmds.0;
    if cmds.len() < 2 {
        print_usage(cmds.first().unwrap_or(&"magiskboot"));
        return log_err!();
    }

    if cmds[1].starts_with("--") {
        cmds[1] = &cmds[1][2..];
    }

    let mut cli = if cmds[1].starts_with("compress=") {
        // Skip the main parser, directly parse the subcommand
        Compress::from_args(&cmds[..2], &cmds[2..]).map(|compress| Cli {
            action: Action::Compress(compress),
        })
    } else {
        Cli::from_args(&[cmds[0]], &cmds[1..])
    }
    .on_early_exit(|| match cmds[1] {
        "dtb" => print_dtb_usage(),
        "cpio" => print_cpio_usage(),
        _ => print_usage(cmds[0]),
    });

    match cli.action {
        Action::Unpack(Unpack {
            no_decompress,
            dump_header,
            ref mut img,
        }) => {
            return Ok(unpack(
                Utf8CStr::from_string(img),
                no_decompress,
                dump_header,
            ));
        }
        Action::Repack(Repack {
            no_compress,
            mut img,
            mut out,
        }) => {
            repack(
                Utf8CStr::from_string(&mut img),
                Utf8CStr::from_string(&mut out),
                no_compress,
            );
        }
        Action::Verify(Verify { mut img, mut cert }) => {
            if !verify_cmd(
                Utf8CStr::from_string(&mut img),
                cert.as_mut().map(Utf8CStr::from_string),
            ) {
                return log_err!();
            }
        }
        Action::Sign(Sign { mut img, mut args }) => {
            let mut iter = args.iter_mut();
            sign_cmd(
                Utf8CStr::from_string(&mut img),
                iter.next().map(Utf8CStr::from_string),
                iter.next().map(Utf8CStr::from_string),
                iter.next().map(Utf8CStr::from_string),
            )?;
        }
        Action::Extract(Extract {
            payload,
            partition,
            outfile,
        }) => {
            extract_boot_from_payload(&payload, partition.as_deref(), outfile.as_deref())
                .log_with_msg(|w| w.write_str("Failed to extract from payload"))?;
        }
        Action::HexPatch(HexPatch {
            mut file,
            mut src,
            mut dest,
        }) => {
            if !hexpatch(
                &mut file,
                Utf8CStr::from_string(&mut src),
                Utf8CStr::from_string(&mut dest),
            ) {
                log_err!("Failed to patch")?;
            }
        }
        Action::Cpio(Cpio { mut file, mut cmds }) => {
            cpio_commands(Utf8CStr::from_string(&mut file), &mut cmds)
                .log_with_msg(|w| w.write_str("Failed to process cpio"))?;
        }
        Action::Dtb(Dtb { mut file, action }) => {
            return dtb_commands(Utf8CStr::from_string(&mut file), &action)
                .map(|b| if b { 0 } else { 1 })
                .log_with_msg(|w| w.write_str("Failed to process dtb"));
        }
        Action::Split(Split {
            no_decompress,
            mut file,
        }) => {
            return Ok(split_image_dtb(
                Utf8CStr::from_string(&mut file),
                no_decompress,
            ));
        }
        Action::Sha1(Sha1 { mut file }) => {
            let file = MappedFile::open(Utf8CStr::from_string(&mut file))?;
            let mut sha1 = [0u8; 20];
            sha1_hash(file.as_ref(), &mut sha1);
            for byte in &sha1 {
                print!("{byte:02x}");
            }
            println!();
        }
        Action::Cleanup(_) => {
            eprintln!("Cleaning up...");
            cleanup();
        }
        Action::Decompress(Decompress { mut file, mut out }) => {
            decompress_cmd(&mut file, out.as_mut())?;
        }
        Action::Compress(Compress {
            format,
            mut file,
            mut out,
        }) => {
            compress_cmd(format, &mut file, out.as_mut())?;
        }
    }
    Ok(0)
}

#[unsafe(no_mangle)]
pub extern "C" fn main(argc: i32, argv: *const *const c_char, _envp: *const *const c_char) -> i32 {
    cmdline_logging();
    unsafe { umask(0) };
    let cmds = CmdArgs::new(argc, argv);
    boot_main(cmds).unwrap_or(1)
}
