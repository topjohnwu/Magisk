use std::collections::BTreeMap;
use std::ffi::CStr;
use std::fmt::{Display, Formatter};
use std::fs::{metadata, read, DirBuilder, File};
use std::io::Write;
use std::mem::size_of;
use std::os::fd::AsRawFd;
use std::os::unix::fs::{symlink, DirBuilderExt, FileTypeExt, MetadataExt};
use std::path::Path;
use std::process::exit;

use anyhow::{anyhow, Context};
use clap::{Parser, Subcommand};
use size::{Base, Size, Style};

use base::libc::{
    c_char, dev_t, gid_t, major, makedev, minor, mknod, mmap, mode_t, munmap, uid_t, MAP_FAILED,
    MAP_PRIVATE, PROT_READ, S_IFBLK, S_IFCHR, S_IFDIR, S_IFLNK, S_IFMT, S_IFREG, S_IRGRP, S_IROTH,
    S_IRUSR, S_IWGRP, S_IWOTH, S_IWUSR, S_IXGRP, S_IXOTH, S_IXUSR,
};
use base::{ptr_to_str_result, ResultExt, WriteExt};

use crate::ramdisk::MagiskCpio;

#[derive(Parser)]
struct CpioCli {
    #[command(subcommand)]
    command: CpioCommands,
}

#[derive(Subcommand)]
enum CpioCommands {
    Test {},
    Restore {},
    Patch {},
    Exists {
        path: String,
    },
    Backup {
        origin: String,
    },
    Rm {
        path: String,
        #[arg(short)]
        recursive: bool,
    },
    Mv {
        from: String,
        to: String,
    },
    Extract {
        path: Option<String>,
        out: Option<String>,
    },
    Mkdir {
        #[arg(value_parser=parse_mode)]
        mode: mode_t,
        dir: String,
    },
    Ln {
        src: String,
        dst: String,
    },
    Add {
        #[arg(value_parser=parse_mode)]
        mode: mode_t,
        path: String,
        file: String,
    },
    Ls {
        path: Option<String>,
        #[arg(short)]
        recursive: bool,
    },
}

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
    pub(crate) entries: BTreeMap<String, CpioEntry>,
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

    fn load_from_data(data: &[u8]) -> anyhow::Result<Self> {
        let mut cpio = Cpio::new();
        let mut pos = 0usize;
        while pos < data.len() {
            let hdr = unsafe { &*(data.as_ptr().add(pos) as *const CpioHeader) };
            if &hdr.magic != b"070701" {
                return Err(anyhow!("invalid cpio magic"));
            }
            pos += size_of::<CpioHeader>();
            let name = CStr::from_bytes_until_nul(&data[pos..])?
                .to_str()?
                .to_string();
            pos += x8u::<usize>(&hdr.namesize)?;
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
            let file_size = x8u::<usize>(&hdr.filesize)?;
            let entry = CpioEntry {
                mode: x8u(&hdr.mode)?,
                uid: x8u(&hdr.uid)?,
                gid: x8u(&hdr.gid)?,
                rdevmajor: x8u(&hdr.rdevmajor)?,
                rdevminor: x8u(&hdr.rdevminor)?,
                data: data[pos..pos + file_size].to_vec(),
            };
            pos += file_size;
            cpio.entries.insert(name, entry);
            pos = align_4(pos);
        }
        Ok(cpio)
    }

    pub(crate) fn load_from_file(path: &str) -> anyhow::Result<Self> {
        eprintln!("Loading cpio: [{}]", path);
        let file = File::open(path)?;
        let len = file.metadata()?.len() as usize;
        let mmap = unsafe {
            mmap(
                std::ptr::null_mut(),
                len,
                PROT_READ,
                MAP_PRIVATE,
                file.as_raw_fd(),
                0,
            )
        };
        if mmap == MAP_FAILED {
            return Err(anyhow!("mmap failed"));
        }
        let data = unsafe { std::slice::from_raw_parts(mmap as *const u8, len) };
        let cpio = Self::load_from_data(data)?;
        unsafe {
            if munmap(mmap, len) != 0 {
                return Err(anyhow!("munmap failed"));
            }
        }
        Ok(cpio)
    }

    fn dump(&self, path: &str) -> anyhow::Result<()> {
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
        if let Some(_) = self.entries.remove(&path) {
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

    fn extract_entry(&self, path: &str, out: &Path) -> anyhow::Result<()> {
        let entry = self.entries.get(path).ok_or(anyhow!("No such file"))?;
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
                    .create(out)?;
            }
            S_IFREG => {
                let mut file = File::create(out)?;
                file.write_all(&entry.data)?;
            }
            S_IFLNK => {
                symlink(Path::new(&std::str::from_utf8(entry.data.as_slice())?), out)?;
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
                return Err(anyhow!("unknown entry type"));
            }
        }
        Ok(())
    }

    fn extract(&self, path: Option<&str>, out: Option<&str>) -> anyhow::Result<()> {
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

    fn add(&mut self, mode: &mode_t, path: &str, file: &str) -> anyhow::Result<()> {
        if path.ends_with('/') {
            return Err(anyhow!("path cannot end with / for add"));
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
                return Err(anyhow!("unsupported file type"));
            }
        };
        self.entries.insert(
            norm_path(path),
            CpioEntry {
                mode,
                uid: 0,
                gid: 0,
                rdevmajor,
                rdevminor,
                data: content,
            },
        );
        eprintln!("Add file [{}] ({:04o})", path, mode);
        Ok(())
    }

    fn mkdir(&mut self, mode: &mode_t, dir: &str) {
        self.entries.insert(
            norm_path(dir),
            CpioEntry {
                mode: *mode | S_IFDIR,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: vec![],
            },
        );
        eprintln!("Create directory [{}] ({:04o})", dir, mode);
    }

    fn ln(&mut self, src: &str, dst: &str) {
        self.entries.insert(
            norm_path(dst),
            CpioEntry {
                mode: S_IFLNK,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: norm_path(src).as_bytes().to_vec(),
            },
        );
        eprintln!("Create symlink [{}] -> [{}]", dst, src);
    }

    fn mv(&mut self, from: &str, to: &str) -> anyhow::Result<()> {
        let entry = self
            .entries
            .remove(&norm_path(from))
            .ok_or(anyhow!("no such entry {}", from))?;
        self.entries.insert(norm_path(to), entry);
        eprintln!("Move [{}] -> [{}]", from, to);
        Ok(())
    }

    fn ls(&self, path: Option<&str>, recursive: bool) {
        let path = path
            .map(norm_path)
            .map(|p| "/".to_owned() + p.as_str())
            .unwrap_or("".to_string());
        for (name, entry) in &self.entries {
            let p = "/".to_owned() + name.as_str();
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
    fn inner(argc: i32, argv: *const *const c_char) -> anyhow::Result<()> {
        let mut cmds = Vec::new();
        if argc < 1 {
            return Err(anyhow!("no arguments"));
        }
        for i in 0..argc {
            let arg = unsafe { ptr_to_str_result(*argv.offset(i as isize)) };
            match arg {
                Ok(arg) => cmds.push(arg),
                Err(e) => Err(e)?,
            }
        }
        let file = cmds[0];

        let mut cpio = if Path::new(file).exists() {
            Cpio::load_from_file(file)?
        } else {
            Cpio::new()
        };
        for cmd in &cmds[1..] {
            if cmd.starts_with('#') {
                continue;
            }
            let cmd = "magiskboot ".to_string() + cmd;
            let cli = CpioCli::try_parse_from(cmd.split(' ').filter(|x| !x.is_empty()))?;
            match &cli.command {
                CpioCommands::Test {} => exit(cpio.test()),
                CpioCommands::Restore {} => cpio.restore()?,
                CpioCommands::Patch {} => cpio.patch(),
                CpioCommands::Exists { path } => {
                    if cpio.exists(path) {
                        exit(0);
                    } else {
                        exit(1);
                    }
                }
                CpioCommands::Backup { origin } => cpio.backup(origin)?,
                CpioCommands::Rm { path, recursive } => cpio.rm(path, *recursive),
                CpioCommands::Mv { from, to } => cpio.mv(from, to)?,
                CpioCommands::Extract { path, out } => {
                    cpio.extract(path.as_deref(), out.as_deref())?
                }
                CpioCommands::Mkdir { mode, dir } => cpio.mkdir(mode, dir),
                CpioCommands::Ln { src, dst } => cpio.ln(src, dst),
                CpioCommands::Add { mode, path, file } => cpio.add(mode, path, file)?,
                CpioCommands::Ls { path, recursive } => {
                    cpio.ls(path.as_deref(), *recursive);
                    exit(0);
                }
            }
        }
        cpio.dump(file)?;
        Ok(())
    }
    inner(argc, argv)
        .context("Failed to process cpio")
        .log()
        .is_ok()
}

fn x8u<U: TryFrom<u32>>(x: &[u8; 8]) -> anyhow::Result<U> {
    // parse hex
    let mut ret = 0u32;
    for i in x {
        let c = *i as char;
        let v = c.to_digit(16).ok_or(anyhow!("bad cpio header"))?;
        ret = ret * 16 + v;
    }
    ret.try_into().map_err(|_| anyhow!("bad cpio header"))
}

#[inline(always)]
fn align_4(x: usize) -> usize {
    (x + 3) & !3
}

#[inline(always)]
fn norm_path(path: &str) -> String {
    let path = path.strip_prefix('/').unwrap_or(path);
    path.strip_suffix('/').unwrap_or(path).to_string()
}

fn parse_mode(s: &str) -> Result<mode_t, String> {
    mode_t::from_str_radix(s, 8).map_err(|e| e.to_string())
}
