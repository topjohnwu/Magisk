use crate::consts::{APP_PACKAGE_NAME, MAGISK_VER_CODE};
use crate::daemon::{AID_APP_END, AID_APP_START, AID_USER_OFFSET, MagiskD, to_app_id};
use crate::ffi::{DbEntryKey, get_magisk_tmp, install_apk, uninstall_pkg};
use base::WalkResult::{Abort, Continue, Skip};
use base::libc::{O_CLOEXEC, O_CREAT, O_RDONLY, O_TRUNC, O_WRONLY};
use base::{
    BufReadExt, Directory, FsPathBuilder, LoggedResult, ReadExt, ResultExt, Utf8CStrBuf,
    Utf8CString, cstr, error, fd_get_attr, warn,
};
use bit_set::BitSet;
use cxx::CxxString;
use std::collections::BTreeMap;
use std::fs::File;
use std::io;
use std::io::{Cursor, Read, Seek, SeekFrom};
use std::os::fd::AsRawFd;
use std::pin::Pin;
use std::time::Duration;

const EOCD_MAGIC: u32 = 0x06054B50;
const APK_SIGNING_BLOCK_MAGIC: [u8; 16] = *b"APK Sig Block 42";
const SIGNATURE_SCHEME_V2_MAGIC: u32 = 0x7109871A;
const PACKAGES_XML: &str = "/data/system/packages.xml";

macro_rules! bad_apk {
    ($msg:literal) => {
        io::Error::new(io::ErrorKind::InvalidData, concat!("cert: ", $msg))
    };
}

/*
 * A v2/v3 signed APK has the format as following
 *
 * +---------------+
 * | zip content   |
 * +---------------+
 * | signing block |
 * +---------------+
 * | central dir   |
 * +---------------+
 * | EOCD          |
 * +---------------+
 *
 * Scan from end of file to find EOCD, and figure our way back to the
 * offset of the signing block. Next, directly extract the certificate
 * from the v2 signature block.
 *
 * All structures above are mostly just for documentation purpose.
 *
 * This method extracts the first certificate of the first signer
 * within the APK v2 signature block.
 */
fn read_certificate(apk: &mut File, version: i32) -> Vec<u8> {
    let res: io::Result<Vec<u8>> = try {
        let mut u32_val = 0u32;
        let mut u64_val = 0u64;

        // Find EOCD
        for i in 0u16.. {
            let mut comment_sz = 0u16;
            apk.seek(SeekFrom::End(-(size_of_val(&comment_sz) as i64) - i as i64))?;
            apk.read_pod(&mut comment_sz)?;

            if comment_sz == i {
                apk.seek(SeekFrom::Current(-22))?;
                let mut magic = 0u32;
                apk.read_pod(&mut magic)?;
                if magic == EOCD_MAGIC {
                    break;
                }
            }
            if i == 0xffff {
                Err(bad_apk!("invalid APK format"))?;
            }
        }

        // We are now at EOCD + sizeof(magic)
        // Seek and read central_dir_off to find the start of the central directory
        let mut central_dir_off = 0u32;
        apk.seek(SeekFrom::Current(12))?;
        apk.read_pod(&mut central_dir_off)?;

        // Code for parse APK comment to get version code
        if version >= 0 {
            let mut comment_sz = 0u16;
            apk.read_pod(&mut comment_sz)?;
            let mut comment = vec![0u8; comment_sz as usize];
            apk.read_exact(&mut comment)?;
            let mut comment = Cursor::new(&comment);
            let mut apk_ver = 0;
            comment.foreach_props(|k, v| {
                if k == "versionCode" {
                    apk_ver = v.parse::<i32>().unwrap_or(0);
                    false
                } else {
                    true
                }
            });
            if version > apk_ver {
                Err(bad_apk!("APK version too low"))?;
            }
        }

        // Next, find the start of the APK signing block
        apk.seek(SeekFrom::Start((central_dir_off - 24) as u64))?;
        apk.read_pod(&mut u64_val)?; // u64_value = block_sz_
        let mut magic = [0u8; 16];
        apk.read_exact(&mut magic)?;
        if magic != APK_SIGNING_BLOCK_MAGIC {
            Err(bad_apk!("invalid signing block magic"))?;
        }
        let mut signing_blk_sz = 0u64;
        apk.seek(SeekFrom::Current(
            -(u64_val as i64) - (size_of_val(&signing_blk_sz) as i64),
        ))?;
        apk.read_pod(&mut signing_blk_sz)?;
        if signing_blk_sz != u64_val {
            Err(bad_apk!("invalid signing block size"))?;
        }

        // Finally, we are now at the beginning of the id-value pair sequence
        loop {
            apk.read_pod(&mut u64_val)?; // id-value pair length
            if u64_val == signing_blk_sz {
                Err(bad_apk!("cannot find certificate"))?;
            }

            let mut id = 0u32;
            apk.read_pod(&mut id)?;
            if id == SIGNATURE_SCHEME_V2_MAGIC {
                // Skip [signer sequence length] + [1st signer length] + [signed data length]
                apk.seek(SeekFrom::Current((size_of_val(&u32_val) * 3) as i64))?;

                apk.read_pod(&mut u32_val)?; // digest sequence length
                apk.seek(SeekFrom::Current(u32_val as i64))?; // skip all digests

                apk.seek(SeekFrom::Current(size_of_val(&u32_val) as i64))?; // cert sequence length
                apk.read_pod(&mut u32_val)?; // 1st cert length

                let mut cert = vec![0; u32_val as usize];
                apk.read_exact(cert.as_mut())?;
                break cert;
            } else {
                // Skip this id-value pair
                apk.seek(SeekFrom::Current(
                    u64_val as i64 - (size_of_val(&id) as i64),
                ))?;
            }
        }
    };
    res.log().unwrap_or(vec![])
}

fn find_apk_path(pkg: &str) -> LoggedResult<Utf8CString> {
    let mut buf = cstr::buf::default();
    Directory::open(cstr!("/data/app"))?.pre_order_walk(|e| {
        if !e.is_dir() {
            return Ok(Skip);
        }
        let name_bytes = e.name().as_bytes();
        if name_bytes.starts_with(pkg.as_bytes()) && name_bytes[pkg.len()] == b'-' {
            // Found the APK path, we can abort now
            e.resolve_path(&mut buf)?;
            return Ok(Abort);
        }
        if name_bytes.starts_with(b"~~") {
            return Ok(Continue);
        }
        Ok(Skip)
    })?;
    if !buf.is_empty() {
        buf.push_str("/base.apk");
    }
    Ok(buf.to_owned())
}

enum Status {
    Installed,
    NotInstalled,
    CertMismatch,
}

pub struct ManagerInfo {
    stub_apk_fd: Option<File>,
    trusted_cert: Vec<u8>,
    repackaged_app_id: i32,
    repackaged_pkg: String,
    repackaged_cert: Vec<u8>,
    tracked_files: BTreeMap<i32, TrackedFile>,
}

impl Default for ManagerInfo {
    fn default() -> Self {
        ManagerInfo {
            stub_apk_fd: None,
            trusted_cert: Vec::new(),
            repackaged_app_id: -1,
            repackaged_pkg: String::new(),
            repackaged_cert: Vec::new(),
            tracked_files: BTreeMap::new(),
        }
    }
}

#[derive(Default)]
struct TrackedFile {
    path: Utf8CString,
    timestamp: Duration,
}

impl TrackedFile {
    fn new(path: Utf8CString) -> TrackedFile {
        let attr = match path.get_attr() {
            Ok(attr) => attr,
            Err(_) => return TrackedFile::default(),
        };
        let timestamp = Duration::new(attr.st.st_ctime as u64, attr.st.st_ctime_nsec as u32);
        TrackedFile { path, timestamp }
    }

    fn is_same(&self) -> bool {
        if self.path.is_empty() {
            return false;
        }
        let attr = match self.path.get_attr() {
            Ok(attr) => attr,
            Err(_) => return false,
        };
        let timestamp = Duration::new(attr.st.st_ctime as u64, attr.st.st_ctime_nsec as u32);
        timestamp == self.timestamp
    }
}

impl ManagerInfo {
    fn check_dyn(&mut self, daemon: &MagiskD, user: i32, pkg: &str) -> Status {
        let apk = cstr::buf::default()
            .join_path(daemon.app_data_dir())
            .join_path_fmt(user)
            .join_path(pkg)
            .join_path("dyn")
            .join_path("current.apk");
        let uid: i32;
        let cert = match apk.open(O_RDONLY | O_CLOEXEC) {
            Ok(mut fd) => {
                uid = fd_get_attr(fd.as_raw_fd())
                    .map(|attr| attr.st.st_uid as i32)
                    .unwrap_or(-1);
                read_certificate(&mut fd, MAGISK_VER_CODE)
            }
            Err(_) => {
                warn!("pkg: no dyn APK, ignore");
                return Status::NotInstalled;
            }
        };

        if cert.is_empty() || cert != self.trusted_cert {
            error!("pkg: dyn APK signature mismatch: {}", apk);
            #[cfg(all(feature = "check-signature", not(debug_assertions)))]
            {
                return Status::CertMismatch;
            }
        }

        self.repackaged_app_id = to_app_id(uid);
        self.tracked_files
            .insert(user, TrackedFile::new(apk.to_owned()));
        Status::Installed
    }

    fn check_stub(&mut self, user: i32, pkg: &str) -> Status {
        let Ok(apk) = find_apk_path(pkg) else {
            return Status::NotInstalled;
        };

        let cert = match apk.open(O_RDONLY | O_CLOEXEC) {
            Ok(mut fd) => read_certificate(&mut fd, -1),
            Err(_) => return Status::NotInstalled,
        };

        if cert.is_empty() || (pkg == self.repackaged_pkg && cert != self.repackaged_cert) {
            error!("pkg: repackaged APK signature invalid: {}", apk);
            uninstall_pkg(&apk);
            return Status::CertMismatch;
        }

        self.repackaged_pkg.clear();
        self.repackaged_pkg.push_str(pkg);
        self.repackaged_cert = cert;
        self.tracked_files.insert(user, TrackedFile::new(apk));
        Status::Installed
    }

    fn check_orig(&mut self, user: i32) -> Status {
        let Ok(apk) = find_apk_path(APP_PACKAGE_NAME) else {
            return Status::NotInstalled;
        };

        let cert = match apk.open(O_RDONLY | O_CLOEXEC) {
            Ok(mut fd) => read_certificate(&mut fd, MAGISK_VER_CODE),
            Err(_) => return Status::NotInstalled,
        };

        if cert.is_empty() || cert != self.trusted_cert {
            error!("pkg: APK signature mismatch: {}", apk);
            #[cfg(all(feature = "check-signature", not(debug_assertions)))]
            {
                uninstall_pkg(cstr!(APP_PACKAGE_NAME));
                return Status::CertMismatch;
            }
        }

        self.tracked_files.insert(user, TrackedFile::new(apk));
        Status::Installed
    }

    fn install_stub(&mut self) {
        if let Some(ref mut stub_fd) = self.stub_apk_fd {
            // Copy the stub APK
            let tmp_apk = cstr!("/data/stub.apk");
            let result: LoggedResult<()> = try {
                {
                    let mut tmp_apk_file =
                        tmp_apk.create(O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0o600)?;
                    io::copy(stub_fd, &mut tmp_apk_file)?;
                }
                // Seek the fd back to start
                stub_fd.seek(SeekFrom::Start(0))?;
            };
            if result.is_ok() {
                install_apk(tmp_apk);
            }
        }
    }

    fn get_manager(&mut self, daemon: &MagiskD, user: i32, mut install: bool) -> (i32, &str) {
        let db_pkg = daemon.get_db_string(DbEntryKey::SuManager);

        // If database changed, always re-check files
        if db_pkg != self.repackaged_pkg {
            self.tracked_files.remove(&user);
        }

        if let Some(file) = self.tracked_files.get(&user)
            && file.is_same()
        {
            // no APK
            if &file.path == PACKAGES_XML {
                if install && !daemon.is_emulator {
                    self.install_stub();
                }
                return (-1, "");
            }
            // dyn APK is still the same
            if file.path.starts_with(daemon.app_data_dir().as_str()) {
                return (
                    user * AID_USER_OFFSET + self.repackaged_app_id,
                    &self.repackaged_pkg,
                );
            }
            // stub APK is still the same
            if !self.repackaged_pkg.is_empty() {
                return if matches!(
                    self.check_dyn(daemon, user, self.repackaged_pkg.clone().as_str()),
                    Status::Installed
                ) {
                    (
                        user * AID_USER_OFFSET + self.repackaged_app_id,
                        &self.repackaged_pkg,
                    )
                } else {
                    (-1, "")
                };
            }
            // orig APK is still the same
            let uid = daemon.get_package_uid(user, APP_PACKAGE_NAME);
            return if uid < 0 {
                (-1, "")
            } else {
                (uid, APP_PACKAGE_NAME)
            };
        }

        if !db_pkg.is_empty() {
            match self.check_stub(user, &db_pkg) {
                Status::Installed => {
                    return if matches!(self.check_dyn(daemon, user, &db_pkg), Status::Installed) {
                        (
                            user * AID_USER_OFFSET + self.repackaged_app_id,
                            &self.repackaged_pkg,
                        )
                    } else {
                        (-1, "")
                    };
                }
                Status::NotInstalled => {
                    daemon.rm_db_string(DbEntryKey::SuManager).ok();
                }
                Status::CertMismatch => {
                    install = true;
                    daemon.rm_db_string(DbEntryKey::SuManager).ok();
                }
            }
        }

        self.repackaged_pkg.clear();
        self.repackaged_cert.clear();

        match self.check_orig(user) {
            Status::Installed => {
                let uid = daemon.get_package_uid(user, APP_PACKAGE_NAME);
                return if uid < 0 {
                    (-1, "")
                } else {
                    (uid, APP_PACKAGE_NAME)
                };
            }
            Status::CertMismatch => install = true,
            Status::NotInstalled => {}
        }

        // If we cannot find any manager, track packages.xml for new package installs
        self.tracked_files
            .insert(user, TrackedFile::new(PACKAGES_XML.into()));

        if install && !daemon.is_emulator {
            self.install_stub();
        }
        (-1, "")
    }
}

impl MagiskD {
    fn get_package_uid(&self, user: i32, pkg: &str) -> i32 {
        let path = cstr::buf::default()
            .join_path(self.app_data_dir())
            .join_path_fmt(user)
            .join_path(pkg);
        path.get_attr()
            .map(|attr| attr.st.st_uid as i32)
            .unwrap_or(-1)
    }

    pub fn preserve_stub_apk(&self) {
        let mut info = self.manager_info.lock().unwrap();

        let apk = cstr::buf::default()
            .join_path(get_magisk_tmp())
            .join_path("stub.apk");

        if let Ok(mut fd) = apk.open(O_RDONLY | O_CLOEXEC) {
            info.trusted_cert = read_certificate(&mut fd, MAGISK_VER_CODE);
            // Seek the fd back to start
            fd.seek(SeekFrom::Start(0)).log_ok();
            info.stub_apk_fd = Some(fd);
        }

        apk.remove().log_ok();
    }

    pub fn get_manager_uid(&self, user: i32) -> i32 {
        let mut info = self.manager_info.lock().unwrap();
        let (uid, _) = info.get_manager(self, user, false);
        uid
    }

    pub fn get_manager(&self, user: i32, install: bool) -> (i32, String) {
        let mut info = self.manager_info.lock().unwrap();
        let (uid, pkg) = info.get_manager(self, user, install);
        (uid, pkg.to_string())
    }

    pub fn ensure_manager(&self) {
        let mut info = self.manager_info.lock().unwrap();
        let _ = info.get_manager(self, 0, true);
    }

    pub unsafe fn get_manager_for_cxx(&self, user: i32, ptr: *mut CxxString, install: bool) -> i32 {
        unsafe {
            let mut info = self.manager_info.lock().unwrap();
            let (uid, pkg) = info.get_manager(self, user, install);
            if let Some(str) = ptr.as_mut() {
                let mut str = Pin::new_unchecked(str);
                str.as_mut().clear();
                str.push_str(pkg);
            }
            uid
        }
    }

    // app_id = app_no + AID_APP_START
    // app_no range: [0, 9999]
    pub fn get_app_no_list(&self) -> BitSet {
        let mut list = BitSet::new();
        let _: LoggedResult<()> = try {
            let mut app_data_dir = Directory::open(self.app_data_dir())?;
            // For each user
            loop {
                let entry = match app_data_dir.read()? {
                    None => break,
                    Some(e) => e,
                };
                let mut user_dir = match entry.open_as_dir() {
                    Err(_) => continue,
                    Ok(dir) => dir,
                };
                // For each package
                loop {
                    match user_dir.read()? {
                        None => break,
                        Some(e) => {
                            let mut entry_path = cstr::buf::default();
                            e.resolve_path(&mut entry_path)?;
                            let attr = entry_path.get_attr()?;
                            let app_id = to_app_id(attr.st.st_uid as i32);
                            if (AID_APP_START..=AID_APP_END).contains(&app_id) {
                                let app_no = app_id - AID_APP_START;
                                list.insert(app_no as usize);
                            }
                        }
                    }
                }
            }
        };
        list
    }
}
