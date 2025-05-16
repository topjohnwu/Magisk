use crate::consts::{MODULEMNT, WORKERDIR};
use crate::ffi::{ModuleInfo, get_magisk_tmp};
use crate::load_prop_file;
use base::{
    Directory, FsPathBuilder, LoggedResult, OsResultStatic, ResultExt, Utf8CStr, Utf8CStrBuf,
    Utf8CString, clone_attr, cstr, debug, error, info, libc, warn,
};
use libc::{MS_RDONLY, O_CLOEXEC, O_CREAT, O_RDONLY};
use std::collections::BTreeMap;
use std::path::{Component, Path};

const SECONDARY_READ_ONLY_PARTITIONS: [&Utf8CStr; 3] =
    [cstr!("/vendor"), cstr!("/product"), cstr!("/system_ext")];

type FsNodeMap = BTreeMap<String, FsNode>;

macro_rules! module_log {
    ($($args:tt)+) => {
        debug!("{:8}: {} <- {}", $($args)+)
    }
}

#[allow(unused_variables)]
fn bind_mount(reason: &str, src: &Utf8CStr, dest: &Utf8CStr, rec: bool) -> OsResultStatic<()> {
    module_log!(reason, dest, src);
    src.bind_mount_to(dest, rec)?;
    dest.remount_mount_point_flags(MS_RDONLY)?;
    Ok(())
}

fn mount_dummy(reason: &str, src: &Utf8CStr, dest: &Utf8CStr, is_dir: bool) -> OsResultStatic<()> {
    if is_dir {
        dest.mkdir(0o000)?;
    } else {
        dest.create(O_CREAT | O_RDONLY | O_CLOEXEC, 0o000)?;
    }
    bind_mount(reason, src, dest, false)
}

// File paths that act like a stack, popping out the last element
// automatically when out of scope. Using Rust's lifetime mechanism,
// we can ensure the buffer will never be incorrectly copied or modified.
// After calling append or clone, the mutable reference's lifetime is
// "transferred" to the returned object, and the compiler will guarantee
// that the original mutable reference can only be reused if and only if
// the newly created instance is destroyed.
struct PathTracker<'a> {
    real: &'a mut dyn Utf8CStrBuf,
    tmp: &'a mut dyn Utf8CStrBuf,
    real_len: usize,
    tmp_len: usize,
}

impl PathTracker<'_> {
    fn from<'a>(real: &'a mut dyn Utf8CStrBuf, tmp: &'a mut dyn Utf8CStrBuf) -> PathTracker<'a> {
        let real_len = real.len();
        let tmp_len = tmp.len();
        PathTracker {
            real,
            tmp,
            real_len,
            tmp_len,
        }
    }

    fn append(&mut self, name: &str) -> PathTracker {
        let real_len = self.real.len();
        let tmp_len = self.tmp.len();
        self.real.append_path(name);
        self.tmp.append_path(name);
        PathTracker {
            real: self.real,
            tmp: self.tmp,
            real_len,
            tmp_len,
        }
    }

    fn clone(&mut self) -> PathTracker {
        Self::from(self.real, self.tmp)
    }
}

impl Drop for PathTracker<'_> {
    // Revert back to the original state after finish using the buffer
    fn drop(&mut self) {
        self.real.truncate(self.real_len);
        self.tmp.truncate(self.tmp_len);
    }
}

enum FsNode {
    Directory { children: FsNodeMap },
    File { src: Utf8CString },
    Symlink { target: Utf8CString },
    Whiteout,
}

impl FsNode {
    fn new_dir() -> FsNode {
        FsNode::Directory {
            children: BTreeMap::new(),
        }
    }

    fn build_from_path(&mut self, path: &mut dyn Utf8CStrBuf) -> LoggedResult<()> {
        let FsNode::Directory { children } = self else {
            return Ok(());
        };
        let mut dir = Directory::open(path)?;
        let path_len = path.len();

        while let Some(entry) = dir.read()? {
            path.truncate(path_len);
            path.append_path(entry.name());
            if entry.is_dir() {
                let node = children
                    .entry(entry.name().to_string())
                    .or_insert_with(FsNode::new_dir);
                node.build_from_path(path)?;
            } else if entry.is_symlink() {
                let mut link = cstr::buf::default();
                path.read_link(&mut link)?;
                children
                    .entry(entry.name().to_string())
                    .or_insert_with(|| FsNode::Symlink {
                        target: link.to_owned(),
                    });
            } else {
                if entry.is_char_device() {
                    let attr = path.get_attr()?;
                    if attr.is_whiteout() {
                        children
                            .entry(entry.name().to_string())
                            .or_insert_with(|| FsNode::Whiteout);
                        continue;
                    }
                }
                children
                    .entry(entry.name().to_string())
                    .or_insert_with(|| FsNode::File {
                        src: path.to_owned(),
                    });
            }
        }

        Ok(())
    }

    // The parent node has to be tmpfs if:
    // - Target does not exist
    // - Source or target is a symlink (since we cannot bind mount symlink)
    // - Source is whiteout (used for removal)
    fn parent_should_be_tmpfs(&self, target_path: &Utf8CStr) -> bool {
        match self {
            FsNode::Directory { .. } | FsNode::File { .. } => {
                if let Ok(attr) = target_path.get_attr() {
                    attr.is_symlink()
                } else {
                    true
                }
            }
            FsNode::Symlink { .. } | FsNode::Whiteout => true,
        }
    }

    fn children(&mut self) -> Option<&mut FsNodeMap> {
        match self {
            FsNode::Directory { children } => Some(children),
            _ => None,
        }
    }

    fn commit(&mut self, mut path: PathTracker, is_root_dir: bool) -> LoggedResult<()> {
        match self {
            FsNode::Directory { children } => {
                let mut is_tmpfs = false;

                // First determine whether tmpfs is required
                children.retain(|name, node| {
                    if name == ".replace" {
                        return if is_root_dir {
                            warn!("Unable to replace '{}', ignore request", path.real);
                            false
                        } else {
                            is_tmpfs = true;
                            true
                        };
                    }

                    let path = path.append(name);
                    if node.parent_should_be_tmpfs(path.real) {
                        if is_root_dir {
                            // Ignore the unsupported child node
                            warn!("Unable to add '{}', skipped", path.real);
                            return false;
                        }
                        is_tmpfs = true;
                    }
                    true
                });

                if is_tmpfs {
                    self.commit_tmpfs(path.clone())?;
                    // Transitioning from non-tmpfs to tmpfs, we need to actually mount the
                    // worker dir to dest after all children are committed.
                    bind_mount("move", path.tmp, path.real, true)?;
                } else {
                    for (name, node) in children {
                        let path = path.append(name);
                        node.commit(path, false)?;
                    }
                }
            }
            FsNode::File { src } => {
                clone_attr(path.real, src)?;
                bind_mount("mount", src, path.real, false)?;
            }
            FsNode::Symlink { .. } | FsNode::Whiteout => {
                error!("Unable to handle '{}': parent should be tmpfs", path.real);
            }
        }

        Ok(())
    }

    fn commit_tmpfs(&mut self, mut path: PathTracker) -> LoggedResult<()> {
        match self {
            FsNode::Directory { children } => {
                path.tmp.mkdirs(0o000)?;
                if path.real.exists() {
                    clone_attr(path.real, path.tmp)?;
                } else if let Some(p) = path.tmp.parent_dir() {
                    let parent = Utf8CString::from(p);
                    clone_attr(&parent, path.tmp)?;
                }

                // Check whether a file name '.replace' exist
                if let Some(FsNode::File { src }) = children.remove(".replace")
                    && let Some(base_dir) = src.parent_dir()
                {
                    for (name, node) in children {
                        let path = path.append(name);
                        match node {
                            FsNode::Directory { .. } | FsNode::File { .. } => {
                                let src = Utf8CString::from(base_dir).join_path(name);
                                bind_mount(
                                    "mount",
                                    &src,
                                    path.real,
                                    matches!(node, FsNode::Directory { .. }),
                                )?;
                            }
                            _ => node.commit_tmpfs(path)?,
                        }
                    }

                    // If performing replace, we skip mirroring
                    return Ok(());
                }

                // Traverse the real directory and mount mirrors
                if let Ok(mut dir) = Directory::open(path.real) {
                    while let Ok(Some(entry)) = dir.read() {
                        if children.contains_key(entry.name().as_str()) {
                            // Should not be mirrored, next
                            continue;
                        }

                        let path = path.append(entry.name());

                        if entry.is_symlink() {
                            // Add the symlink into children and handle it later
                            let mut link = cstr::buf::default();
                            entry.read_link(&mut link).log_ok();
                            children.insert(
                                entry.name().to_string(),
                                FsNode::Symlink {
                                    target: link.to_owned(),
                                },
                            );
                        } else {
                            mount_dummy("mirror", path.real, path.tmp, entry.is_dir())?;
                        }
                    }
                }

                // Finally, commit children
                for (name, node) in children {
                    let path = path.append(name);
                    node.commit_tmpfs(path)?;
                }
            }
            FsNode::File { src } => {
                if path.real.exists() {
                    clone_attr(path.real, src)?;
                }
                mount_dummy("mount", src, path.tmp, false)?;
            }
            FsNode::Symlink { target } => {
                module_log!("mklink", path.tmp, target);
                path.tmp.create_symlink_to(target)?;
                if path.real.exists() {
                    clone_attr(path.real, path.tmp)?;
                }
            }
            FsNode::Whiteout => {
                module_log!("delete", path.real, "null");
            }
        }
        Ok(())
    }
}

fn get_path_env() -> String {
    std::env::var_os("PATH")
        .and_then(|s| s.into_string().ok())
        .unwrap_or_default()
}

fn inject_magisk_bins(system: &mut FsNode) {
    fn inject(children: &mut FsNodeMap) {
        let mut path = cstr::buf::default().join_path(get_magisk_tmp());

        // Inject binaries

        let len = path.len();
        path.append_path("magisk");
        children.insert(
            "magisk".to_string(),
            FsNode::File {
                src: path.to_owned(),
            },
        );

        path.truncate(len);
        path.append_path("magiskpolicy");
        children.insert(
            "magiskpolicy".to_string(),
            FsNode::File {
                src: path.to_owned(),
            },
        );

        // Inject applet symlinks

        children.insert(
            "su".to_string(),
            FsNode::Symlink {
                target: Utf8CString::from("./magisk"),
            },
        );
        children.insert(
            "resetprop".to_string(),
            FsNode::Symlink {
                target: Utf8CString::from("./magisk"),
            },
        );
        children.insert(
            "supolicy".to_string(),
            FsNode::Symlink {
                target: Utf8CString::from("./magiskpolicy"),
            },
        );
    }

    // First find whether /system/bin exists
    let bin = system.children().and_then(|c| c.get_mut("bin"));
    if let Some(FsNode::Directory { children }) = bin {
        inject(children);
        return;
    }

    // If /system/bin node does not exist, use the first suitable directory in PATH
    let path_env = get_path_env();
    let bin_paths = path_env.split(':').filter_map(|path| {
        if SECONDARY_READ_ONLY_PARTITIONS
            .iter()
            .any(|p| path.starts_with(p.as_str()))
        {
            let path = Utf8CString::from(path);
            if let Ok(attr) = path.get_attr()
                && (attr.st.st_mode & 0x0001) != 0
            {
                return Some(path);
            }
        }
        None
    });
    'path_loop: for path in bin_paths {
        let components = Path::new(&path)
            .components()
            .filter(|c| matches!(c, Component::Normal(_)))
            .filter_map(|c| c.as_os_str().to_str());

        let mut curr = match system {
            FsNode::Directory { children } => children,
            _ => continue,
        };

        for dir in components {
            let node = curr.entry(dir.to_owned()).or_insert_with(FsNode::new_dir);
            match node {
                FsNode::Directory { children } => curr = children,
                _ => continue 'path_loop,
            }
        }

        // Found a suitable path, done
        inject(curr);
        return;
    }

    // If still not found, directly inject into /system/bin
    let node = system
        .children()
        .map(|c| c.entry("bin".to_string()).or_insert_with(FsNode::new_dir));
    if let Some(FsNode::Directory { children }) = node {
        inject(children)
    }
}

fn inject_zygisk_bins(system: &mut FsNode, name: &str) {
    #[cfg(target_pointer_width = "64")]
    let has_32_bit = cstr!("/system/bin/linker").exists();

    #[cfg(target_pointer_width = "32")]
    let has_32_bit = true;

    if has_32_bit {
        let lib = system
            .children()
            .map(|c| c.entry("lib".to_string()).or_insert_with(FsNode::new_dir));
        if let Some(FsNode::Directory { children }) = lib {
            let mut bin_path = cstr::buf::default().join_path(get_magisk_tmp());

            #[cfg(target_pointer_width = "64")]
            bin_path.append_path("magisk32");

            #[cfg(target_pointer_width = "32")]
            bin_path.append_path("magisk");

            children.insert(
                name.to_string(),
                FsNode::File {
                    src: bin_path.to_owned(),
                },
            );
        }
    }

    #[cfg(target_pointer_width = "64")]
    if cstr!("/system/bin/linker64").exists() {
        let lib64 = system
            .children()
            .map(|c| c.entry("lib64".to_string()).or_insert_with(FsNode::new_dir));
        if let Some(FsNode::Directory { children }) = lib64 {
            let bin_path = cstr::buf::default()
                .join_path(get_magisk_tmp())
                .join_path("magisk");

            children.insert(
                name.to_string(),
                FsNode::File {
                    src: bin_path.to_owned(),
                },
            );
        }
    }
}

pub fn load_modules(module_list: &[ModuleInfo], zygisk_name: &str) {
    let mut system = FsNode::new_dir();

    // Step 1: Create virtual filesystem tree
    //
    // In this step, there is zero logic applied during tree construction; we simply collect
    // and record the union of all module filesystem trees under each of their /system directory.

    let mut path = cstr::buf::default()
        .join_path(get_magisk_tmp())
        .join_path(MODULEMNT);
    let len = path.len();
    for info in module_list {
        path.truncate(len);
        path.append_path(&info.name);

        // Read props
        let module_path_len = path.len();
        path.append_path("system.prop");
        if path.exists() {
            // Do NOT go through property service as it could cause boot lock
            load_prop_file(&path, true);
        }

        // Check whether skip mounting
        path.truncate(module_path_len);
        path.append_path("skip_mount");
        if path.exists() {
            continue;
        }

        // Double check whether the system folder exists
        path.truncate(module_path_len);
        path.append_path("system");
        if path.exists() {
            info!("{}: loading module files", &info.name);
            system.build_from_path(&mut path).log_ok();
        }
    }

    // Step 2: Inject custom files
    //
    // Magisk provides some built-in functionality that requires augmenting the filesystem.
    // We expose several cmdline tools (e.g. su) into PATH, and the zygisk shared library
    // has to also be added into the default LD_LIBRARY_PATH for code injection.
    // We directly inject file nodes into the virtual filesystem tree we built in the previous
    // step, treating Magisk just like a special "module".

    if get_magisk_tmp() != "/sbin" || get_path_env().split(":").all(|s| s != "/sbin") {
        inject_magisk_bins(&mut system);
    }
    if !zygisk_name.is_empty() {
        inject_zygisk_bins(&mut system, zygisk_name);
    }

    // Step 3: Extract all supported read-only partition roots
    //
    // For simplicity and backwards compatibility on older Android versions, when constructing
    // Magisk modules, we always assume that there is only a single read-only partition mounted
    // at /system. However, on modern Android there are actually multiple read-only partitions
    // mounted at their respective paths. We need to extract these subtrees out of the main
    // tree and treat them as individual trees.

    let mut roots = BTreeMap::new(); /* mapOf(partition_name -> FsNode) */
    if let FsNode::Directory { children } = &mut system {
        for dir in SECONDARY_READ_ONLY_PARTITIONS {
            // Only treat these nodes as root iff it is actually a directory in rootdir
            if let Ok(attr) = dir.get_attr()
                && attr.is_dir()
            {
                let name = dir.trim_start_matches('/');
                if let Some(root) = children.remove(name) {
                    roots.insert(name, root);
                }
            }
        }
    }
    roots.insert("system", system);

    // Reuse the path buffer
    path.clear();
    path.push_str("/");

    // Build the base worker directory path
    let mut tmp = cstr::buf::default()
        .join_path(get_magisk_tmp())
        .join_path(WORKERDIR);

    let mut tracker = PathTracker::from(&mut path, &mut tmp);
    for (dir, mut root) in roots {
        // Step 4: Convert virtual filesystem tree into concrete operations
        //
        // Compare the virtual filesystem tree we constructed against the real filesystem
        // structure on-device to generate a series of "operations".
        // The "core" of the logic is to decide which directories need to be rebuilt in the
        // tmpfs worker directory, and real sub-nodes need to be mirrored inside it.

        let path = tracker.append(dir);
        root.commit(path, true).log_ok();
    }
}
