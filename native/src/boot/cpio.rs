#![allow(clippy::useless_conversion)]

use std::collections::BTreeMap;
use std::fmt::{Display, Formatter};
use std::fs::{metadata, read, DirBuilder, File};
use std::io::Write;
use std::mem::size_of;
use std::os::unix::fs::{symlink, DirBuilderExt, FileTypeExt, MetadataExt};
use std::path::Path;
use std::process::exit;
use std::str;

use argh::FromArgs;
use bytemuck::{from_bytes, Pod, Zeroable};
use num_traits::cast::AsPrimitive;
use size::{Base, Size, Style};

use crate::ffi::{unxz, xz};
use base::libc::{
    c_char, dev_t, gid_t, major, makedev, minor, mknod, mode_t, uid_t, S_IFBLK, S_IFCHR, S_IFDIR,
    S_IFLNK, S_IFMT, S_IFREG, S_IRGRP, S_IROTH, S_IRUSR, S_IWGRP, S_IWOTH, S_IWUSR, S_IXGRP,
    S_IXOTH, S_IXUSR,
};
use base::{
    log_err, map_args, EarlyExitExt, LoggedResult, MappedFile, ResultExt, Utf8CStr, WriteExt,
};

use crate::ramdisk::MagiskCpio;

#[derive(FromArgs)]
struct CpioCli {
    #[argh(positional)]
    file: String,
    #[argh(positional)]
    commands: Vec<String>,
}

#[derive(FromArgs)]
struct CpioCommand {
    #[argh(subcommand)]
    command: CpioSubCommand,
}

#[derive(FromArgs)]
#[argh(subcommand)]
enum CpioSubCommand {
    Test(Test),
    Restore(Restore),
    Patch(Patch),
    Exists(Exists),
    Backup(Backup),
    Remove(Remove),
    Move(Move),
    Extract(Extract),
    MakeDir(MakeDir),
    Link(Link),
    Add(Add),
    List(List),
}

#[derive(FromArgs)]
#[argh(subcommand, name = "test")]
struct Test {}

#[derive(FromArgs)]
#[argh(subcommand, name = "restore")]
struct Restore {}

#[derive(FromArgs)]
#[argh(subcommand, name = "patch")]
struct Patch {}

#[derive(FromArgs)]
#[argh(subcommand, name = "exists")]
struct Exists {
    #[argh(positional, arg_name = "entry")]
    path: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "backup")]
struct Backup {
    #[argh(positional, arg_name = "orig")]
    origin: String,
    #[argh(switch, short = 'n')]
    skip_compress: bool,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "rm")]
struct Remove {
    #[argh(positional, arg_name = "entry")]
    path: String,
    #[argh(switch, short = 'r')]
    recursive: bool,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "mv")]
struct Move {
    #[argh(positional, arg_name = "source")]
    from: String,
    #[argh(positional, arg_name = "dest")]
    to: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "extract")]
struct Extract {
    #[argh(positional, greedy)]
    paths: Vec<String>,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "mkdir")]
struct MakeDir {
    #[argh(positional, from_str_fn(parse_mode))]
    mode: mode_t,
    #[argh(positional, arg_name = "entry")]
    dir: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "ln")]
struct Link {
    #[argh(positional, arg_name = "entry")]
    src: String,
    #[argh(positional, arg_name = "target")]
    dst: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "add")]
struct Add {
    #[argh(positional, from_str_fn(parse_mode))]
    mode: mode_t,
    #[argh(positional, arg_name = "entry")]
    path: String,
    #[argh(positional, arg_name = "infile")]
    file: String,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "ls")]
struct List {
    #[argh(positional, default = r#"String::from("/")"#)]
    path: String,
    #[argh(switch, short = 'r')]
    recursive: bool,
}

fn print_cpio_usage() {
    eprintln!(
        r#"Usage: magiskboot cpio <incpio> [commands...]

Do cpio commands to <incpio> (modifications are done in-place).
Each command is a single argument; add quotes for each command.

Supported commands:
  exists ENTRY
    Return 0 if ENTRY exists, else return 1
  ls [-r] [PATH]
    List PATH ("/" by default); specify [-r] to list recursively
  rm [-r] ENTRY
    Remove ENTRY, specify [-r] to remove recursively
  mkdir MODE ENTRY
    Create directory ENTRY with permissions MODE
  ln TARGET ENTRY
    Create a symlink to TARGET with the name ENTRY
  mv SOURCE DEST
    Move SOURCE to DEST
  add MODE ENTRY INFILE
    Add INFILE as ENTRY with permissions MODE; replaces ENTRY if exists
  extract [ENTRY OUT]
    Extract ENTRY to OUT, or extract all entries to current directory
  test
    Test the cpio's status
    Return value is 0 or bitwise or-ed of following values:
    0x1:Magisk    0x2:unsupported    0x4:Sony
  patch
    Apply ramdisk patches
    Configure with env variables: KEEPVERITY KEEPFORCEENCRYPT
  backup ORIG [-n]
    Create ramdisk backups from ORIG, specify [-n] to skip compression
  restore
    Restore ramdisk from ramdisk backup stored within incpio
"#
    )
}

#[derive(Copy, Clone, Pod, Zeroable)]
#[repr(C, packed)]
struct CpioHeader {
    magic: [u8; 6],
    ino: [u8; 8],
    mode: [u8; 8],
    uid: [u8; 8],
    gid: [u8; 8],
    nlink: [u8; 8],
    mtime: [u8; 8],
    filesize: [u8; 8],
    devmajor: [u8; 8],
    devminor: [u8; 8],
    rdevmajor: [u8; 8],
    rdevminor: [u8; 8],
    namesize: [u8; 8],
    check: [u8; 8],
}

pub(crate) struct Cpio {
    pub(crate) entries: BTreeMap<String, Box<CpioEntry>>,
}

pub(crate) struct CpioEntry {
    pub(crate) mode: mode_t,
    pub(crate) uid: uid_t,
    pub(crate) gid: gid_t,
    pub(crate) rdevmajor: dev_t,
    pub(crate) rdevminor: dev_t,
    pub(crate) data: Vec<u8>,
}

impl Cpio {
    fn new() -> Self {
        Self {
            entries: BTreeMap::new(),
        }
    }

    fn load_from_data(data: &[u8]) -> LoggedResult<Self> {
        let mut cpio = Cpio::new();
        let mut pos = 0_usize;
        while pos < data.len() {
            let hdr_sz = size_of::<CpioHeader>();
            let hdr = from_bytes::<CpioHeader>(&data[pos..(pos + hdr_sz)]);
            if &hdr.magic != b"070701" {
                return Err(log_err!("invalid cpio magic"));
            }
            pos += hdr_sz;
            let name_sz = x8u(&hdr.namesize)? as usize;
            let name = Utf8CStr::from_bytes(&data[pos..(pos + name_sz)])?.to_string();
            pos += name_sz;
            pos = align_4(pos);
            if name == "." || name == ".." {
                continue;
            }
            if name == "TRAILER!!!" {
                match data[pos..].windows(6).position(|x| x == b"070701") {
                    Some(x) => pos += x,
                    None => break,
                }
                continue;
            }
            let file_sz = x8u(&hdr.filesize)? as usize;
            let entry = Box::new(CpioEntry {
                mode: x8u(&hdr.mode)?.as_(),
                uid: x8u(&hdr.uid)?.as_(),
                gid: x8u(&hdr.gid)?.as_(),
                rdevmajor: x8u(&hdr.rdevmajor)?.as_(),
                rdevminor: x8u(&hdr.rdevminor)?.as_(),
                data: data[pos..(pos + file_sz)].to_vec(),
            });
            pos += file_sz;
            cpio.entries.insert(name, entry);
            pos = align_4(pos);
        }
        Ok(cpio)
    }

    pub(crate) fn load_from_file(path: &Utf8CStr) -> LoggedResult<Self> {
        eprintln!("Loading cpio: [{}]", path);
        let file = MappedFile::open(path)?;
        Self::load_from_data(file.as_ref())
    }

    fn dump(&self, path: &str) -> LoggedResult<()> {
        eprintln!("Dumping cpio: [{}]", path);
        let mut file = File::create(path)?;
        let mut pos = 0usize;
        let mut inode = 300000i64;
        for (name, entry) in &self.entries {
            pos += file.write(
                format!(
                    "070701{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}",
                    inode,
                    entry.mode,
                    entry.uid,
                    entry.gid,
                    1,
                    0,
                    entry.data.len(),
                    0,
                    0,
                    entry.rdevmajor,
                    entry.rdevminor,
                    name.len() + 1,
                    0
                ).as_bytes(),
            )?;
            pos += file.write(name.as_bytes())?;
            pos += file.write(&[0])?;
            file.write_zeros(align_4(pos) - pos)?;
            pos = align_4(pos);
            pos += file.write(&entry.data)?;
            file.write_zeros(align_4(pos) - pos)?;
            pos = align_4(pos);
            inode += 1;
        }
        pos += file.write(
            format!("070701{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}{:08x}",
                inode, 0o755, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0
            ).as_bytes()
        )?;
        pos += file.write("TRAILER!!!\0".as_bytes())?;
        file.write_zeros(align_4(pos) - pos)?;
        Ok(())
    }

    pub(crate) fn rm(&mut self, path: &str, recursive: bool) {
        let path = norm_path(path);
        if self.entries.remove(&path).is_some() {
            eprintln!("Removed entry [{}]", path);
        }
        if recursive {
            let path = path + "/";
            self.entries.retain(|k, _| {
                if k.starts_with(&path) {
                    eprintln!("Removed entry [{}]", k);
                    false
                } else {
                    true
                }
            })
        }
    }

    fn extract_entry(&self, path: &str, out: &Path) -> LoggedResult<()> {
        let entry = self
            .entries
            .get(path)
            .ok_or_else(|| log_err!("No such file"))?;
        eprintln!("Extracting entry [{}] to [{}]", path, out.to_string_lossy());
        if let Some(parent) = out.parent() {
            DirBuilder::new()
                .mode(0o755)
                .recursive(true)
                .create(parent)?;
        }
        match entry.mode & S_IFMT {
            S_IFDIR => {
                DirBuilder::new()
                    .mode((entry.mode & 0o777).into())
                    .recursive(true) // avoid error if existing
                    .create(out)?;
            }
            S_IFREG => {
                let mut file = File::create(out)?;
                file.write_all(&entry.data)?;
            }
            S_IFLNK => {
                symlink(Path::new(&str::from_utf8(entry.data.as_slice())?), out)?;
            }
            S_IFBLK | S_IFCHR => {
                let dev = makedev(entry.rdevmajor.try_into()?, entry.rdevminor.try_into()?);
                unsafe {
                    mknod(
                        out.to_str().unwrap().as_ptr() as *const c_char,
                        entry.mode,
                        dev,
                    )
                };
            }
            _ => {
                return Err(log_err!("unknown entry type"));
            }
        }
        Ok(())
    }

    fn extract(&self, path: Option<&str>, out: Option<&str>) -> LoggedResult<()> {
        let path = path.map(norm_path);
        let out = out.map(Path::new);
        if let (Some(path), Some(out)) = (&path, &out) {
            return self.extract_entry(path, out);
        } else {
            for path in self.entries.keys() {
                if path == "." || path == ".." {
                    continue;
                }
                self.extract_entry(path, Path::new(path))?;
            }
        }
        Ok(())
    }

    pub(crate) fn exists(&self, path: &str) -> bool {
        self.entries.contains_key(&norm_path(path))
    }

    fn add(&mut self, mode: &mode_t, path: &str, file: &str) -> LoggedResult<()> {
        if path.ends_with('/') {
            return Err(log_err!("path cannot end with / for add"));
        }
        let file = Path::new(file);
        let content = read(file)?;
        let metadata = metadata(file)?;
        let mut rdevmajor: dev_t = 0;
        let mut rdevminor: dev_t = 0;
        let mode = if metadata.file_type().is_file() {
            mode | S_IFREG
        } else {
            rdevmajor = unsafe { major(metadata.rdev().try_into()?).try_into()? };
            rdevminor = unsafe { minor(metadata.rdev().try_into()?).try_into()? };
            if metadata.file_type().is_block_device() {
                mode | S_IFBLK
            } else if metadata.file_type().is_char_device() {
                mode | S_IFCHR
            } else {
                return Err(log_err!("unsupported file type"));
            }
        };
        self.entries.insert(
            norm_path(path),
            Box::new(CpioEntry {
                mode,
                uid: 0,
                gid: 0,
                rdevmajor,
                rdevminor,
                data: content,
            }),
        );
        eprintln!("Add file [{}] ({:04o})", path, mode);
        Ok(())
    }

    fn mkdir(&mut self, mode: &mode_t, dir: &str) {
        self.entries.insert(
            norm_path(dir),
            Box::new(CpioEntry {
                mode: *mode | S_IFDIR,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: vec![],
            }),
        );
        eprintln!("Create directory [{}] ({:04o})", dir, mode);
    }

    fn ln(&mut self, src: &str, dst: &str) {
        self.entries.insert(
            norm_path(dst),
            Box::new(CpioEntry {
                mode: S_IFLNK,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: norm_path(src).as_bytes().to_vec(),
            }),
        );
        eprintln!("Create symlink [{}] -> [{}]", dst, src);
    }

    fn mv(&mut self, from: &str, to: &str) -> LoggedResult<()> {
        let entry = self
            .entries
            .remove(&norm_path(from))
            .ok_or_else(|| log_err!("no such entry {}", from))?;
        self.entries.insert(norm_path(to), entry);
        eprintln!("Move [{}] -> [{}]", from, to);
        Ok(())
    }

    fn ls(&self, path: &str, recursive: bool) {
        let path = norm_path(path);
        let path = if path.is_empty() {
            path
        } else {
            "/".to_string() + path.as_str()
        };
        for (name, entry) in &self.entries {
            let p = "/".to_string() + name.as_str();
            if !p.starts_with(&path) {
                continue;
            }
            let p = p.strip_prefix(&path).unwrap();
            if !p.is_empty() && !p.starts_with('/') {
                continue;
            }
            if !recursive && !p.is_empty() && p.matches('/').count() > 1 {
                continue;
            }
            println!("{}\t{}", entry, name);
        }
    }
}

impl CpioEntry {
    pub(crate) fn compress(&mut self) -> bool {
        if self.mode & S_IFMT != S_IFREG {
            return false;
        }
        let mut compressed = Vec::new();
        if !xz(&self.data, &mut compressed) {
            eprintln!("xz compression failed");
            return false;
        }
        self.data = compressed;
        true
    }

    pub(crate) fn decompress(&mut self) -> bool {
        if self.mode & S_IFMT != S_IFREG {
            return false;
        }
        let mut decompressed = Vec::new();
        if !unxz(&self.data, &mut decompressed) {
            eprintln!("xz decompression failed");
            return false;
        }
        self.data = decompressed;
        true
    }
}

impl Display for CpioEntry {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}{}{}{}{}{}{}{}{}{}\t{}\t{}\t{}\t{}:{}",
            match self.mode & S_IFMT {
                S_IFDIR => "d",
                S_IFREG => "-",
                S_IFLNK => "l",
                S_IFBLK => "b",
                S_IFCHR => "c",
                _ => "?",
            },
            if self.mode & S_IRUSR != 0 { "r" } else { "-" },
            if self.mode & S_IWUSR != 0 { "w" } else { "-" },
            if self.mode & S_IXUSR != 0 { "x" } else { "-" },
            if self.mode & S_IRGRP != 0 { "r" } else { "-" },
            if self.mode & S_IWGRP != 0 { "w" } else { "-" },
            if self.mode & S_IXGRP != 0 { "x" } else { "-" },
            if self.mode & S_IROTH != 0 { "r" } else { "-" },
            if self.mode & S_IWOTH != 0 { "w" } else { "-" },
            if self.mode & S_IXOTH != 0 { "x" } else { "-" },
            self.uid,
            self.gid,
            Size::from_bytes(self.data.len())
                .format()
                .with_style(Style::Abbreviated)
                .with_base(Base::Base10)
                .to_string(),
            self.rdevmajor,
            self.rdevminor,
        )
    }
}

pub fn cpio_commands(argc: i32, argv: *const *const c_char) -> bool {
    fn inner(argc: i32, argv: *const *const c_char) -> LoggedResult<()> {
        if argc < 1 {
            return Err(log_err!("No arguments"));
        }

        let cmds = map_args(argc, argv)?;

        let mut cli =
            CpioCli::from_args(&["magiskboot", "cpio"], &cmds).on_early_exit(print_cpio_usage);

        let file = Utf8CStr::from_string(&mut cli.file);
        let mut cpio = if Path::new(file).exists() {
            Cpio::load_from_file(file)?
        } else {
            Cpio::new()
        };

        for cmd in cli.commands {
            if cmd.starts_with('#') {
                continue;
            }
            let mut cli = CpioCommand::from_args(
                &["magiskboot", "cpio", file],
                cmd.split(' ')
                    .filter(|x| !x.is_empty())
                    .collect::<Vec<_>>()
                    .as_slice(),
            )
            .on_early_exit(print_cpio_usage);

            match &mut cli.command {
                CpioSubCommand::Test(_) => exit(cpio.test()),
                CpioSubCommand::Restore(_) => cpio.restore()?,
                CpioSubCommand::Patch(_) => cpio.patch(),
                CpioSubCommand::Exists(Exists { path }) => {
                    if cpio.exists(path) {
                        exit(0);
                    } else {
                        exit(1);
                    }
                }
                CpioSubCommand::Backup(Backup {
                    origin,
                    skip_compress,
                }) => cpio.backup(Utf8CStr::from_string(origin), *skip_compress)?,
                CpioSubCommand::Remove(Remove { path, recursive }) => cpio.rm(path, *recursive),
                CpioSubCommand::Move(Move { from, to }) => cpio.mv(from, to)?,
                CpioSubCommand::MakeDir(MakeDir { mode, dir }) => cpio.mkdir(mode, dir),
                CpioSubCommand::Link(Link { src, dst }) => cpio.ln(src, dst),
                CpioSubCommand::Add(Add { mode, path, file }) => cpio.add(mode, path, file)?,
                CpioSubCommand::Extract(Extract { paths }) => {
                    if !paths.is_empty() && paths.len() != 2 {
                        return Err(log_err!("invalid arguments"));
                    }
                    cpio.extract(
                        paths.get(0).map(|x| x.as_str()),
                        paths.get(1).map(|x| x.as_str()),
                    )?;
                }
                CpioSubCommand::List(List { path, recursive }) => {
                    cpio.ls(path.as_str(), *recursive);
                    exit(0);
                }
            };
        }
        cpio.dump(file)?;
        Ok(())
    }
    inner(argc, argv)
        .log_with_msg(|w| w.write_str("Failed to process cpio"))
        .is_ok()
}

fn x8u(x: &[u8; 8]) -> LoggedResult<u32> {
    // parse hex
    let mut ret = 0u32;
    let s = str::from_utf8(x).log_with_msg(|w| w.write_str("bad cpio header"))?;
    for c in s.chars() {
        ret = ret * 16 + c.to_digit(16).ok_or_else(|| log_err!("bad cpio header"))?;
    }
    Ok(ret)
}

#[inline(always)]
fn align_4(x: usize) -> usize {
    (x + 3) & !3
}

#[inline(always)]
fn norm_path(path: &str) -> String {
    path.split('/')
        .filter(|x| !x.is_empty())
        .intersperse("/")
        .collect()
}

fn parse_mode(s: &str) -> Result<mode_t, String> {
    mode_t::from_str_radix(s, 8).map_err(|e| e.to_string())
}
