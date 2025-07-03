#![allow(clippy::useless_conversion)]

use std::cmp::Ordering;
use std::collections::{BTreeMap, HashMap};
use std::fmt::{Display, Formatter};
use std::fs::File;
use std::io::{Read, Write};
use std::mem::size_of;
use std::process::exit;
use std::str;

use argh::FromArgs;
use bytemuck::{Pod, Zeroable, from_bytes};
use num_traits::cast::AsPrimitive;
use size::{Base, Size, Style};

use base::libc::{
    O_CLOEXEC, O_CREAT, O_RDONLY, O_TRUNC, O_WRONLY, S_IFBLK, S_IFCHR, S_IFDIR, S_IFLNK, S_IFMT,
    S_IFREG, S_IRGRP, S_IROTH, S_IRUSR, S_IWGRP, S_IWOTH, S_IWUSR, S_IXGRP, S_IXOTH, S_IXUSR,
    c_char, dev_t, gid_t, major, makedev, minor, mknod, mode_t, uid_t,
};
use base::{
    BytesExt, EarlyExitExt, LoggedResult, MappedFile, ResultExt, Utf8CStr, Utf8CStrBuf, WriteExt,
    cstr, log_err, map_args,
};

use crate::check_env;
use crate::compress::{get_decoder, get_encoder};
use crate::ffi::FileFormat;
use crate::patch::{patch_encryption, patch_verity};

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
    action: CpioAction,
}

#[derive(FromArgs)]
#[argh(subcommand)]
enum CpioAction {
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
    Test the cpio's status. Return values:
    0:stock    1:Magisk    2:unsupported
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

struct Cpio {
    entries: BTreeMap<String, Box<CpioEntry>>,
}

struct CpioEntry {
    mode: mode_t,
    uid: uid_t,
    gid: gid_t,
    rdevmajor: dev_t,
    rdevminor: dev_t,
    data: Vec<u8>,
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
                match data[pos..].find(b"070701") {
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

    fn load_from_file(path: &Utf8CStr) -> LoggedResult<Self> {
        eprintln!("Loading cpio: [{path}]");
        let file = MappedFile::open(path)?;
        Self::load_from_data(file.as_ref())
    }

    fn dump(&self, path: &str) -> LoggedResult<()> {
        eprintln!("Dumping cpio: [{path}]");
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

    fn rm(&mut self, path: &str, recursive: bool) {
        let path = norm_path(path);
        if self.entries.remove(&path).is_some() {
            eprintln!("Removed entry [{path}]");
        }
        if recursive {
            let path = path + "/";
            self.entries.retain(|k, _| {
                if k.starts_with(&path) {
                    eprintln!("Removed entry [{k}]");
                    false
                } else {
                    true
                }
            })
        }
    }

    fn extract_entry(&self, path: &str, out: &mut String) -> LoggedResult<()> {
        let entry = self
            .entries
            .get(path)
            .ok_or_else(|| log_err!("No such file"))?;
        eprintln!("Extracting entry [{path}] to [{out}]");

        let out = Utf8CStr::from_string(out);

        let mut buf = cstr::buf::default();

        // Make sure its parent directories exist
        if let Some(dir) = out.parent_dir() {
            buf.push_str(dir);
            buf.mkdirs(0o755)?;
        }

        let mode: mode_t = (entry.mode & 0o777).into();

        match entry.mode & S_IFMT {
            S_IFDIR => out.mkdir(mode)?,
            S_IFREG => {
                let mut file = out.create(O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, mode)?;
                file.write_all(&entry.data)?;
            }
            S_IFLNK => {
                buf.clear();
                buf.push_str(str::from_utf8(entry.data.as_slice())?);
                out.create_symlink_to(&buf)?;
            }
            S_IFBLK | S_IFCHR => {
                let dev = makedev(entry.rdevmajor.try_into()?, entry.rdevminor.try_into()?);
                unsafe { mknod(out.as_ptr().cast(), entry.mode, dev) };
            }
            _ => {
                return Err(log_err!("unknown entry type"));
            }
        }
        Ok(())
    }

    fn extract(&self, path: Option<&mut String>, out: Option<&mut String>) -> LoggedResult<()> {
        let path = path.map(|s| norm_path(s.as_str()));
        if let (Some(path), Some(out)) = (&path, out) {
            return self.extract_entry(path, out);
        } else {
            for path in self.entries.keys() {
                if path == "." || path == ".." {
                    continue;
                }
                self.extract_entry(path, &mut path.clone())?;
            }
        }
        Ok(())
    }

    fn exists(&self, path: &str) -> bool {
        self.entries.contains_key(&norm_path(path))
    }

    fn add(&mut self, mode: mode_t, path: &str, file: &mut String) -> LoggedResult<()> {
        if path.ends_with('/') {
            return Err(log_err!("path cannot end with / for add"));
        }
        let file = Utf8CStr::from_string(file);
        let attr = file.get_attr()?;

        let mut content = Vec::<u8>::new();
        let rdevmajor: dev_t;
        let rdevminor: dev_t;

        // Treat symlinks as regular files as symlinks are created by the 'ln TARGET ENTRY' command
        let mode = if attr.is_file() || attr.is_symlink() {
            rdevmajor = 0;
            rdevminor = 0;
            file.open(O_RDONLY | O_CLOEXEC)?.read_to_end(&mut content)?;
            mode | S_IFREG
        } else {
            rdevmajor = major(attr.st.st_rdev.as_()).as_();
            rdevminor = minor(attr.st.st_rdev.as_()).as_();
            if attr.is_block_device() {
                mode | S_IFBLK
            } else if attr.is_char_device() {
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
        eprintln!("Add file [{path}] ({mode:04o})");
        Ok(())
    }

    fn mkdir(&mut self, mode: mode_t, dir: &str) {
        self.entries.insert(
            norm_path(dir),
            Box::new(CpioEntry {
                mode: mode | S_IFDIR,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: vec![],
            }),
        );
        eprintln!("Create directory [{dir}] ({mode:04o})");
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
        eprintln!("Create symlink [{dst}] -> [{src}]");
    }

    fn mv(&mut self, from: &str, to: &str) -> LoggedResult<()> {
        let entry = self
            .entries
            .remove(&norm_path(from))
            .ok_or_else(|| log_err!("no such entry {}", from))?;
        self.entries.insert(norm_path(to), entry);
        eprintln!("Move [{from}] -> [{to}]");
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
            println!("{entry}\t{name}");
        }
    }
}

const MAGISK_PATCHED: i32 = 1 << 0;
const UNSUPPORTED_CPIO: i32 = 1 << 1;

impl Cpio {
    fn patch(&mut self) {
        let keep_verity = check_env("KEEPVERITY");
        let keep_force_encrypt = check_env("KEEPFORCEENCRYPT");
        eprintln!(
            "Patch with flag KEEPVERITY=[{keep_verity}] KEEPFORCEENCRYPT=[{keep_force_encrypt}]"
        );
        self.entries.retain(|name, entry| {
            let fstab = (!keep_verity || !keep_force_encrypt)
                && entry.mode & S_IFMT == S_IFREG
                && !name.starts_with(".backup")
                && !name.starts_with("twrp")
                && !name.starts_with("recovery")
                && name.starts_with("fstab");
            if !keep_verity {
                if fstab {
                    eprintln!("Found fstab file [{name}]");
                    let len = patch_verity(entry.data.as_mut_slice());
                    if len != entry.data.len() {
                        entry.data.resize(len, 0);
                    }
                } else if name == "verity_key" {
                    return false;
                }
            }
            if !keep_force_encrypt && fstab {
                let len = patch_encryption(entry.data.as_mut_slice());
                if len != entry.data.len() {
                    entry.data.resize(len, 0);
                }
            }
            true
        });
    }

    fn test(&self) -> i32 {
        for file in [
            "sbin/launch_daemonsu.sh",
            "sbin/su",
            "init.xposed.rc",
            "boot/sbin/launch_daemonsu.sh",
        ] {
            if self.exists(file) {
                return UNSUPPORTED_CPIO;
            }
        }
        for file in [
            ".backup/.magisk",
            "init.magisk.rc",
            "overlay/init.magisk.rc",
        ] {
            if self.exists(file) {
                return MAGISK_PATCHED;
            }
        }
        0
    }

    fn restore(&mut self) -> LoggedResult<()> {
        let mut backups = HashMap::<String, Box<CpioEntry>>::new();
        let mut rm_list = String::new();
        self.entries
            .extract_if(|name, _| name.starts_with(".backup/"))
            .for_each(|(name, mut entry)| {
                if name == ".backup/.rmlist" {
                    if let Ok(data) = str::from_utf8(&entry.data) {
                        rm_list.push_str(data);
                    }
                } else if name != ".backup/.magisk" {
                    let new_name = if name.ends_with(".xz") && entry.decompress() {
                        &name[8..name.len() - 3]
                    } else {
                        &name[8..]
                    };
                    eprintln!("Restore [{name}] -> [{new_name}]");
                    backups.insert(new_name.to_string(), entry);
                }
            });
        self.rm(".backup", false);
        if rm_list.is_empty() && backups.is_empty() {
            self.entries.clear();
            return Ok(());
        }
        for rm in rm_list.split('\0') {
            if !rm.is_empty() {
                self.rm(rm, false);
            }
        }
        self.entries.extend(backups);

        Ok(())
    }

    fn backup(&mut self, origin: &mut String, skip_compress: bool) -> LoggedResult<()> {
        let mut backups = HashMap::<String, Box<CpioEntry>>::new();
        let mut rm_list = String::new();
        backups.insert(
            ".backup".to_string(),
            Box::new(CpioEntry {
                mode: S_IFDIR,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: vec![],
            }),
        );
        let origin = Utf8CStr::from_string(origin);
        let mut o = Cpio::load_from_file(origin)?;
        o.rm(".backup", true);
        self.rm(".backup", true);

        let mut lhs = o.entries.into_iter().peekable();
        let mut rhs = self.entries.iter().peekable();

        loop {
            enum Action<'a> {
                Backup(String, Box<CpioEntry>),
                Record(&'a String),
                Noop,
            }
            let action = match (lhs.peek(), rhs.peek()) {
                (Some((l, _)), Some((r, re))) => match l.as_str().cmp(r.as_str()) {
                    Ordering::Less => {
                        let (l, le) = lhs.next().unwrap();
                        Action::Backup(l, le)
                    }
                    Ordering::Greater => Action::Record(rhs.next().unwrap().0),
                    Ordering::Equal => {
                        let (l, le) = lhs.next().unwrap();
                        let action = if re.data != le.data {
                            Action::Backup(l, le)
                        } else {
                            Action::Noop
                        };
                        rhs.next();
                        action
                    }
                },
                (Some(_), None) => {
                    let (l, le) = lhs.next().unwrap();
                    Action::Backup(l, le)
                }
                (None, Some(_)) => Action::Record(rhs.next().unwrap().0),
                (None, None) => {
                    break;
                }
            };
            match action {
                Action::Backup(name, mut entry) => {
                    let backup = if !skip_compress && entry.compress() {
                        format!(".backup/{name}.xz")
                    } else {
                        format!(".backup/{name}")
                    };
                    eprintln!("Backup [{name}] -> [{backup}]");
                    backups.insert(backup, entry);
                }
                Action::Record(name) => {
                    eprintln!("Record new entry: [{name}] -> [.backup/.rmlist]");
                    rm_list.push_str(&format!("{name}\0"));
                }
                Action::Noop => {}
            }
        }
        if !rm_list.is_empty() {
            backups.insert(
                ".backup/.rmlist".to_string(),
                Box::new(CpioEntry {
                    mode: S_IFREG,
                    uid: 0,
                    gid: 0,
                    rdevmajor: 0,
                    rdevminor: 0,
                    data: rm_list.as_bytes().to_vec(),
                }),
            );
        }
        self.entries.extend(backups);

        Ok(())
    }
}

impl CpioEntry {
    pub(crate) fn compress(&mut self) -> bool {
        if self.mode & S_IFMT != S_IFREG {
            return false;
        }
        let mut encoder = get_encoder(FileFormat::XZ, Vec::new());
        let Ok(data): std::io::Result<Vec<u8>> = (try {
            encoder.write_all(&self.data)?;
            encoder.finish()?
        }) else {
            eprintln!("xz compression failed");
            return false;
        };

        self.data = data;
        true
    }

    pub(crate) fn decompress(&mut self) -> bool {
        if self.mode & S_IFMT != S_IFREG {
            return false;
        }

        let mut decoder = get_decoder(FileFormat::XZ, Vec::new());
        let Ok(data): std::io::Result<Vec<u8>> = (try {
            decoder.write_all(&self.data)?;
            decoder.finish()?
        }) else {
            eprintln!("xz compression failed");
            return false;
        };

        self.data = data;
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
                .with_base(Base::Base10),
            self.rdevmajor,
            self.rdevminor,
        )
    }
}

pub fn cpio_commands(argc: i32, argv: *const *const c_char) -> bool {
    let res: LoggedResult<()> = try {
        if argc < 1 {
            Err(log_err!("No arguments"))?;
        }

        let cmds = map_args(argc, argv)?;

        let mut cli =
            CpioCli::from_args(&["magiskboot", "cpio"], &cmds).on_early_exit(print_cpio_usage);

        let file = Utf8CStr::from_string(&mut cli.file);
        let mut cpio = if file.exists() {
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

            match &mut cli.action {
                CpioAction::Test(_) => exit(cpio.test()),
                CpioAction::Restore(_) => cpio.restore()?,
                CpioAction::Patch(_) => cpio.patch(),
                CpioAction::Exists(Exists { path }) => {
                    if cpio.exists(path) {
                        exit(0);
                    } else {
                        exit(1);
                    }
                }
                CpioAction::Backup(Backup {
                    origin,
                    skip_compress,
                }) => cpio.backup(origin, *skip_compress)?,
                CpioAction::Remove(Remove { path, recursive }) => cpio.rm(path, *recursive),
                CpioAction::Move(Move { from, to }) => cpio.mv(from, to)?,
                CpioAction::MakeDir(MakeDir { mode, dir }) => cpio.mkdir(*mode, dir),
                CpioAction::Link(Link { src, dst }) => cpio.ln(src, dst),
                CpioAction::Add(Add { mode, path, file }) => cpio.add(*mode, path, file)?,
                CpioAction::Extract(Extract { paths }) => {
                    if !paths.is_empty() && paths.len() != 2 {
                        Err(log_err!("invalid arguments"))?;
                    }
                    let mut it = paths.iter_mut();
                    cpio.extract(it.next(), it.next())?;
                }
                CpioAction::List(List { path, recursive }) => {
                    cpio.ls(path.as_str(), *recursive);
                    exit(0);
                }
            };
        }
        cpio.dump(file)?;
    };
    res.log_with_msg(|w| w.write_str("Failed to process cpio"))
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
