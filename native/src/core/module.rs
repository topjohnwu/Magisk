use crate::consts::{MODULEMNT, MODULEROOT, WORKERDIR};
use crate::daemon::MagiskD;
use crate::ffi::{get_magisk_tmp, get_zygisk_lib_name};
use crate::load_prop_file;
use base::{
    Directory, FsPathBuilder, LoggedResult, OsResultStatic, ResultExt, Utf8CStr, Utf8CStrBuf,
    Utf8CString, WalkResult, clone_attr, cstr, debug, error, info, libc, warn,
};
use libc::{MS_RDONLY, O_CLOEXEC, O_CREAT, O_RDONLY};
use std::collections::BTreeMap;
use std::path::{Component, Path};

const MAGISK_BIN_INJECT_PARTITIONS: [&Utf8CStr; 4] = [
    cstr!("/system/"),
    cstr!("/vendor/"),
    cstr!("/product/"),
    cstr!("/system_ext/"),
];

const SECONDARY_READ_ONLY_PARTITIONS: [&Utf8CStr; 3] =
    [cstr!("/vendor"), cstr!("/product"), cstr!("/system_ext")];

type FsNodeMap = BTreeMap<String, FsNode>;

macro_rules! module_log {
    ($($args:tt)+) => {
        debug!("{:8}: {} <- {}", $($args)+)
    }
}

#[allow(unused_variables)]
fn bind_mount(reason: &str, src: &Utf8CStr, dest: &Utf8CStr, rec: bool) {
    module_log!(reason, dest, src);
    // Ignore any kind of error here. If a single bind mount fails due to selinux permissions or
    // kernel limitations, don't let it break module mount entirely.
    src.bind_mount_to(dest, rec).log_ok();
    dest.remount_mount_point_flags(MS_RDONLY).log_ok();
}

fn mount_dummy(reason: &str, src: &Utf8CStr, dest: &Utf8CStr, is_dir: bool) -> OsResultStatic<()> {
    if is_dir {
        dest.mkdir(0o000)?;
    } else {
        dest.create(O_CREAT | O_RDONLY | O_CLOEXEC, 0o000)?;
    }
    bind_mount(reason, src, dest, false);
    Ok(())
}

// File path that act like a stack, popping out the last element
// automatically when out of scope. Using Rust's lifetime mechanism,
// we can ensure the buffer will never be incorrectly copied or modified.
// After calling append or reborrow, the mutable reference's lifetime is
// "transferred" to the returned object, and the compiler will guarantee
// that the original mutable reference can only be reused if and only if
// the newly created instance is destroyed.
struct PathTracker<'a> {
    path: &'a mut dyn Utf8CStrBuf,
    len: usize,
}

impl PathTracker<'_> {
    fn from<'a>(path: &'a mut dyn Utf8CStrBuf) -> PathTracker<'a> {
        let len = path.len();
        PathTracker { path, len }
    }

    fn append(&mut self, name: &str) -> PathTracker {
        let len = self.path.len();
        self.path.append_path(name);
        PathTracker {
            path: self.path,
            len,
        }
    }

    fn reborrow(&mut self) -> PathTracker {
        Self::from(self.path)
    }
}

impl Drop for PathTracker<'_> {
    // Revert back to the original state after finish using the buffer
    fn drop(&mut self) {
        self.path.truncate(self.len);
    }
}

struct FilePaths<'a> {
    real: PathTracker<'a>,
    worker: PathTracker<'a>,
    module_mnt: PathTracker<'a>,
    module_root: PathTracker<'a>,
}

impl FilePaths<'_> {
    fn append(&mut self, name: &str) -> FilePaths {
        FilePaths {
            real: self.real.append(name),
            worker: self.worker.append(name),
            module_mnt: self.module_mnt.append(name),
            module_root: self.module_root.append(name),
        }
    }

    fn reborrow(&mut self) -> FilePaths {
        FilePaths {
            real: self.real.reborrow(),
            worker: self.worker.reborrow(),
            module_mnt: self.module_mnt.reborrow(),
            module_root: self.module_root.reborrow(),
        }
    }

    fn real(&self) -> &Utf8CStr {
        self.real.path
    }

    fn worker(&self) -> &Utf8CStr {
        self.worker.path
    }

    fn module_mnt(&self) -> &Utf8CStr {
        self.module_mnt.path
    }

    fn module(&self) -> &Utf8CStr {
        self.module_root.path
    }
}

enum FsNode {
    Directory { children: FsNodeMap },
    File { src: Utf8CString },
    Symlink { target: Utf8CString },
    MagiskLink,
    Whiteout,
}

impl FsNode {
    fn new_dir() -> FsNode {
        FsNode::Directory {
            children: BTreeMap::new(),
        }
    }

    fn collect(&mut self, mut paths: FilePaths) -> LoggedResult<()> {
        let FsNode::Directory { children } = self else {
            return Ok(());
        };
        let mut dir = Directory::open(paths.module())?;

        while let Some(entry) = dir.read()? {
            let entry_paths = paths.append(entry.name());
            let path = entry_paths.module();
            if entry.is_dir() {
                let node = children
                    .entry(entry.name().to_string())
                    .or_insert_with(FsNode::new_dir);
                node.collect(entry_paths)?;
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
                if entry_paths.real().exists() {
                    clone_attr(entry_paths.real(), path)?;
                }
                children
                    .entry(entry.name().to_string())
                    .or_insert_with(|| FsNode::File {
                        // Make sure to mount from module_mnt, not module
                        src: entry_paths.module_mnt().to_owned(),
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
            _ => true,
        }
    }

    fn children(&mut self) -> Option<&mut FsNodeMap> {
        match self {
            FsNode::Directory { children } => Some(children),
            _ => None,
        }
    }

    fn commit(&mut self, mut path: FilePaths, is_root_dir: bool) -> LoggedResult<()> {
        match self {
            FsNode::Directory { children } => {
                let mut is_tmpfs = false;

                // First determine whether tmpfs is required
                children.retain(|name, node| {
                    if name == ".replace" {
                        return if is_root_dir {
                            warn!("Unable to replace '{}', ignore request", path.real());
                            false
                        } else {
                            is_tmpfs = true;
                            true
                        };
                    }

                    let path = path.append(name);
                    if node.parent_should_be_tmpfs(path.real()) {
                        if is_root_dir {
                            // Ignore the unsupported child node
                            warn!("Unable to add '{}', skipped", path.real());
                            return false;
                        }
                        is_tmpfs = true;
                    }
                    true
                });

                if is_tmpfs {
                    self.commit_tmpfs(path.reborrow())?;
                    // Transitioning from non-tmpfs to tmpfs, we need to actually mount the
                    // worker dir to dest after all children are committed.
                    bind_mount("move", path.worker(), path.real(), true);
                } else {
                    for (name, node) in children {
                        let path = path.append(name);
                        node.commit(path, false)?;
                    }
                }
            }
            FsNode::File { src } => {
                bind_mount("mount", src, path.real(), false);
            }
            _ => {
                error!("Unable to handle '{}': parent should be tmpfs", path.real());
            }
        }

        Ok(())
    }

    fn commit_tmpfs(&mut self, mut path: FilePaths) -> LoggedResult<()> {
        match self {
            FsNode::Directory { children } => {
                path.worker().mkdirs(0o000)?;
                if path.real().exists() {
                    clone_attr(path.real(), path.worker())?;
                } else if let Some(p) = path.worker().parent_dir() {
                    let parent = Utf8CString::from(p);
                    clone_attr(&parent, path.worker())?;
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
                                mount_dummy(
                                    "mount",
                                    &src,
                                    path.worker(),
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
                if let Ok(mut dir) = Directory::open(path.real()) {
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
                            mount_dummy("mirror", path.real(), path.worker(), entry.is_dir())?;
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
                mount_dummy("mount", src, path.worker(), false)?;
            }
            FsNode::Symlink { target } => {
                module_log!("mklink", path.worker(), target);
                path.worker().create_symlink_to(target)?;
                if path.real().exists() {
                    clone_attr(path.real(), path.worker())?;
                }
            }
            FsNode::MagiskLink => {
                if let Some(name) = path.real().file_name()
                    && name == "supolicy"
                {
                    module_log!("mklink", path.worker(), "./magiskpolicy");
                    path.worker().create_symlink_to(cstr!("./magiskpolicy"))?;
                } else {
                    module_log!("mklink", path.worker(), "./magisk");
                    path.worker().create_symlink_to(cstr!("./magisk"))?;
                }
            }
            FsNode::Whiteout => {
                module_log!("delete", path.real(), "null");
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
        children.insert("su".to_string(), FsNode::MagiskLink);
        children.insert("resetprop".to_string(), FsNode::MagiskLink);
        children.insert("supolicy".to_string(), FsNode::MagiskLink);
    }

    // Strip /system prefix to insert correct node
    fn strip_system_prefix(orig_item: &str) -> String {
        match orig_item.strip_prefix("/system/") {
            Some(rest) => format!("/{rest}"),
            None => orig_item.to_string(),
        }
    }

    let path_env = get_path_env();
    let mut candidates = vec![];

    for orig_item in path_env.split(':') {
        // Filter not suitbale paths
        if !MAGISK_BIN_INJECT_PARTITIONS
            .iter()
            .any(|p| orig_item.starts_with(p.as_str()))
        {
            continue;
        }
        // Flatten apex path is not suitable too
        if orig_item.starts_with("/system/apex/") {
            continue;
        }

        // Override existing su first
        let su_path = Utf8CString::from(format!("{orig_item}/su"));
        if su_path.exists() {
            let item = strip_system_prefix(orig_item);
            candidates.push((item, 0));
            break;
        }

        let path = Utf8CString::from(orig_item);
        if let Ok(attr) = path.get_attr()
            && (attr.st.st_mode & 0x0001) != 0
            && let Ok(mut dir) = Directory::open(&path)
        {
            let mut count = 0;
            if dir
                .pre_order_walk(|e| {
                    if e.is_file() {
                        count += 1;
                    }
                    Ok(WalkResult::Continue)
                })
                .is_err()
            {
                // Skip, we cannot ensure the result is correct
                continue;
            }
            let item = strip_system_prefix(orig_item);
            candidates.push((item, count));
        }
    }

    // Sort by amount of files
    candidates.sort_by_key(|&(_, count)| count);

    'path_loop: for candidate in candidates {
        let components = Path::new(&candidate.0)
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

fn inject_zygisk_bins(system: &mut FsNode) {
    let name = get_zygisk_lib_name();

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

            // There are some devices that announce ABI as 64 bit only, but ship with linker
            // because they make use of a special 32 bit to 64 bit translator (such as tango).
            // In this case, magisk32 does not exist, so inserting it will cause bind mount
            // failure and affect module mount. Native bridge injection does not support these
            // kind of translators anyway, so simply check if magisk32 exists here.
            if bin_path.exists() {
                children.insert(
                    name.to_string(),
                    FsNode::File {
                        src: bin_path.to_owned(),
                    },
                );
            }
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

impl MagiskD {
    pub fn apply_modules(&self) {
        let mut system = FsNode::new_dir();

        // Build all the base "prefix" paths
        let mut root = cstr::buf::default().join_path("/");

        let mut module_dir = cstr::buf::default().join_path(MODULEROOT);

        let mut module_mnt = cstr::buf::default()
            .join_path(get_magisk_tmp())
            .join_path(MODULEMNT);

        let mut worker = cstr::buf::default()
            .join_path(get_magisk_tmp())
            .join_path(WORKERDIR);

        // Create a collection of all relevant paths
        let mut root_paths = FilePaths {
            real: PathTracker::from(&mut root),
            worker: PathTracker::from(&mut worker),
            module_mnt: PathTracker::from(&mut module_mnt),
            module_root: PathTracker::from(&mut module_dir),
        };

        // Step 1: Create virtual filesystem tree
        //
        // In this step, there is zero logic applied during tree construction; we simply collect and
        // record the union of all module filesystem trees under each of their /system directory.

        for info in self.module_list.get().iter().flat_map(|v| v.iter()) {
            let mut module_paths = root_paths.append(&info.name);
            {
                // Read props
                let prop = module_paths.append("system.prop");
                if prop.module().exists() {
                    // Do NOT go through property service as it could cause boot lock
                    load_prop_file(prop.module(), true);
                }
            }
            {
                // Check whether skip mounting
                let skip = module_paths.append("skip_mount");
                if skip.module().exists() {
                    continue;
                }
            }
            {
                // Double check whether the system folder exists
                let sys = module_paths.append("system");
                if sys.module().exists() {
                    info!("{}: loading module files", &info.name);
                    system.collect(sys).log_ok();
                }
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
        if self.zygisk_enabled() {
            inject_zygisk_bins(&mut system);
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

        for (dir, mut root) in roots {
            // Step 4: Convert virtual filesystem tree into concrete operations
            //
            // Compare the virtual filesystem tree we constructed against the real filesystem
            // structure on-device to generate a series of "operations".
            // The "core" of the logic is to decide which directories need to be rebuilt in the
            // tmpfs worker directory, and real sub-nodes need to be mirrored inside it.

            let path = root_paths.append(dir);
            root.commit(path, true).log_ok();
        }
    }
}
