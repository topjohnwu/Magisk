use crate::consts::{MODULEMNT, WORKERDIR};
use crate::ffi::{ModuleInfo, get_magisk_tmp};
use base::{
    Directory, FsPath, FsPathBuf, LoggedResult, ResultExt, Utf8CStr, Utf8CStrBuf, Utf8CString,
    WalkResult::Continue, clone_attr, cstr, debug, error, info, libc::O_RDONLY, path, warn,
};
use cxx::CxxString;
use std::collections::{
    BTreeMap, BTreeSet, btree_map::Entry as MapEntry, btree_set::Entry as SetEntry,
};

enum Node {
    Dir {
        children: BTreeMap<Utf8CString, Node>,
        exist: bool,
        replace: bool,
    },
    File {
        src: Option<Utf8CString>,
        is_link: bool,
    },
}

impl Node {
    fn mount(&mut self, dest: &Utf8CStr) -> LoggedResult<()> {
        if let Node::Dir { exist, .. } = self {
            *exist = true;
        }
        self.do_mount(dest, false)
    }

    fn do_mount(&self, path: &Utf8CStr, parent_tmpfs: bool) -> LoggedResult<()> {
        // todo: do mount
        match &self {
            Node::Dir {
                children,
                exist,
                replace,
            } => {
                let mut worker = if !*exist || *replace {
                    let mut worker = get_magisk_tmp().to_owned();
                    worker.push_str("/");
                    worker.push_str(WORKERDIR);
                    let len = worker.len();
                    worker.push_str(path);
                    if *replace {
                        debug!("replace : {}", worker);
                    } else if parent_tmpfs {
                        debug!("mkdir   : {}", worker);
                    } else {
                        debug!("tmpfs   : {}", worker);
                    }
                    FsPath::from(&worker).mkdirs(0o000)?;
                    let mut buf = path.to_owned();
                    while !FsPath::from(&buf).exists() {
                        let mut tmp = Utf8CString::default();
                        FsPath::from(&buf).parent(&mut tmp);
                        buf = tmp.to_owned();
                    }
                    clone_attr(FsPath::from(&buf), FsPath::from(&worker)).ok();
                    worker.resize(len);
                    Some(worker)
                } else {
                    None
                };

                let mut buf = path.to_owned();
                let len = buf.len();
                for (name, child) in children {
                    buf.push_str("/");
                    buf.push_str(name);
                    child.do_mount(&buf, worker.is_some())?;
                    buf.resize(len);
                }
                if !*replace
                    && let Some(worker) = &mut worker
                    && let Ok(mut dest) = Directory::open(&buf)
                {
                    let len = worker.len();
                    dest.pre_order_walk(|f| {
                        if let Ok(name) = Utf8CStr::from_cstr(f.name())
                            && !children.contains_key(&name.to_owned())
                        {
                            f.path(&mut buf)?;
                            worker.push_str(&buf);
                            if f.is_dir() {
                                debug!("mkdir   : {}", &worker);
                                let worker = FsPath::from(worker);
                                worker.mkdir(0o00)?;
                                clone_attr(FsPath::from(&buf), worker)?;
                            } else if f.is_symlink() {
                                debug!("cp_link : {} <- {}", &worker, buf);
                                let attr = f.get_attr()?;
                                let mut link = Utf8CString::default();
                                f.read_link(&mut link)?;
                                let worker = FsPath::from(worker);
                                FsPath::from(&link).symlink_to(worker)?;
                                worker.set_attr(&attr)?;
                            } else {
                                debug!("mirror  : {} <- {}", &worker, buf);
                                let worker = FsPath::from(worker);
                                worker.create(O_RDONLY, 0o000)?;
                                worker.bind_mount_to(FsPath::from(&buf), true)?;
                            }
                            worker.resize(len);
                        };
                        Ok(Continue)
                    })?;
                }
                if !parent_tmpfs && let Some(mut worker) = worker {
                    worker.push_str(path);
                    debug!("move    : {} <- {}", path, worker);
                    FsPath::from(path).bind_mount_to(FsPath::from(&worker), true)?;
                }
            }
            Node::File { src, is_link } => match (src, is_link) {
                (None, _) => {
                    debug!("delete  : {}", path);
                }
                (Some(src), false) => {
                    if parent_tmpfs {
                        let mut worker = get_magisk_tmp().to_owned();
                        worker.push_str("/");
                        worker.push_str(WORKERDIR);
                        worker.push_str(path);
                        debug!("module  : {} <- {}", worker, src);
                        let worker = FsPath::from(&worker);
                        worker.create(O_RDONLY, 0o000)?;
                        worker.bind_mount_to(FsPath::from(src), true)?;
                    } else {
                        debug!("module  : {} <- {}", path, src);
                        let src = FsPath::from(src);
                        let path = FsPath::from(path);
                        clone_attr(path, src).ok();
                        path.bind_mount_to(src, true)?;
                    }
                }
                (Some(src), true) => {
                    if parent_tmpfs {
                        let mut worker = get_magisk_tmp().to_owned();
                        worker.push_str("/");
                        worker.push_str(WORKERDIR);
                        worker.push_str(path);
                        debug!("symlink : {} <- {}", worker, src);
                        FsPath::from(src).symlink_to(FsPath::from(&worker))?;
                    } else {
                        unreachable!()
                    }
                }
            },
        }
        Ok(())
    }

    fn extract(&mut self, path: &Utf8CStr) -> Option<Node> {
        let path = path.to_owned();
        match self {
            Node::Dir { children, .. } => children.remove_entry(&path).map(|(_, v)| v),
            Node::File { .. } => None,
        }
    }
}

enum ModuleEntry {
    Dir {
        name: Utf8CString,
        iter: ModuleIterator,
    },
    File {
        name: Utf8CString,
        path: Option<Utf8CString>,
        is_link: bool,
    },
}

enum CustomModule {
    Dir {
        children: BTreeMap<Utf8CString, CustomModule>,
    },
    File {
        path: Utf8CString,
        is_link: bool,
    },
}

trait CustomModuleExt {
    fn insert_entry(&mut self, path: &Utf8CStr, target: &Utf8CStr, is_link: bool);
}

impl CustomModuleExt for BTreeMap<Utf8CString, CustomModule> {
    fn insert_entry(&mut self, path: &Utf8CStr, target: &Utf8CStr, is_link: bool) {
        match path.split_once('/') {
            Some((dir, rest)) => {
                if dir.is_empty() {
                    return self.insert_entry(&Utf8CString::from(rest.to_owned()), target, is_link);
                }
                match self
                    .entry(Utf8CString::from(dir.to_owned()))
                    .or_insert_with(|| CustomModule::Dir {
                        children: BTreeMap::new(),
                    }) {
                    CustomModule::Dir { children } => {
                        children.insert_entry(&Utf8CString::from(rest.to_owned()), target, is_link);
                    }
                    CustomModule::File { .. } => {
                        warn!("Duplicate entry: {}", dir);
                    }
                }
            }
            _ => match self.entry(path.to_owned()) {
                MapEntry::Vacant(v) => {
                    v.insert(CustomModule::File {
                        path: target.to_owned(),
                        is_link,
                    });
                }
                _ => {
                    warn!("Duplicate entry: {}", path);
                }
            },
        }
    }
}

trait DirExt {
    fn open_dir(&mut self, name: &Utf8CStr) -> Self;
}

impl DirExt for Vec<Directory> {
    fn open_dir(&mut self, name: &Utf8CStr) -> Self {
        self.iter()
            .filter_map(|d| d.open_dir(name.as_cstr()).ok())
            .collect()
    }
}

impl DirExt for Vec<CustomModule> {
    fn open_dir(&mut self, name: &Utf8CStr) -> Self {
        let name = name.to_owned();
        self.iter_mut()
            .filter_map(|d| {
                if let CustomModule::Dir { children } = d {
                    children.remove(&name)
                } else {
                    None
                }
            })
            .collect()
    }
}

struct ModuleIterator {
    modules: Vec<Directory>,
    customs: Vec<CustomModule>,
    collected: BTreeSet<Utf8CString>,
}

impl Iterator for ModuleIterator {
    type Item = ModuleEntry;

    fn next(&mut self) -> Option<Self::Item> {
        while let Some(e) = self.modules.last_mut().and_then(|d| d.read().log_ok()) {
            let res: Option<Self::Item> = try {
                let e = e?;
                let name = Utf8CStr::from_cstr(e.name()).log_ok()?;
                if let SetEntry::Vacant(v) = self.collected.entry(name.to_owned()) {
                    let mut path = Utf8CString::default();
                    let attr = e.get_attr().log_ok()?;
                    if attr.is_symlink() {
                        e.read_link(&mut path).log_ok()?;
                    } else {
                        e.path(&mut path).log_ok()?;
                    }
                    let entry = if e.is_dir() {
                        ModuleEntry::Dir {
                            name: name.to_owned(),
                            iter: ModuleIterator {
                                modules: self.modules.open_dir(v.get()),
                                customs: self.customs.open_dir(v.get()),
                                collected: BTreeSet::new(),
                            },
                        }
                    } else {
                        ModuleEntry::File {
                            name: name.to_owned(),
                            path: Some(path).take_if(|_| !attr.is_whiteout()),
                            is_link: attr.is_symlink(),
                        }
                    };
                    v.insert();
                    entry
                } else if !e.is_dir() {
                    warn!("Duplicate entry: {}", name);
                    None?
                } else {
                    None?
                }
            };

            if res.is_some() {
                return res;
            }

            self.modules.pop();
        }

        while let Some(CustomModule::Dir { children }) = self.customs.last_mut() {
            let res: Option<Self::Item> = try {
                let e = children.first_entry()?;

                let name: &Utf8CStr = e.key();

                if let SetEntry::Vacant(v) = self.collected.entry(name.to_owned()) {
                    let entry = match e.get() {
                        CustomModule::Dir { .. } => ModuleEntry::Dir {
                            name: name.to_owned(),
                            iter: ModuleIterator {
                                modules: vec![],
                                customs: self.customs.open_dir(v.get()),
                                collected: BTreeSet::new(),
                            },
                        },
                        CustomModule::File { .. } => {
                            if let (name, CustomModule::File { path, is_link }) = e.remove_entry() {
                                ModuleEntry::File {
                                    name: name.to_owned(),
                                    path: Some(path),
                                    is_link,
                                }
                            } else {
                                unreachable!()
                            }
                        }
                    };
                    v.insert();
                    entry
                } else if matches!(e.get(), CustomModule::File { .. }) {
                    error!("Duplicate entry: {}", name);
                    None?
                } else {
                    None?
                }
            };

            if res.is_some() {
                return res;
            }

            self.customs.pop();
        }

        None
    }
}

struct ModuleList {
    modules: Vec<Directory>,
    customs: Vec<CustomModule>,
}

impl ModuleList {
    fn from_module_infos(module_infos: &[ModuleInfo]) -> Self {
        let mut path = FsPathBuf::default().join(get_magisk_tmp()).join(MODULEMNT);

        let len = path.len();
        let mut modules = Vec::new();
        for info in module_infos {
            path = path.join(&info.name);
            if let Ok(m) = Directory::open(&path) {
                if !m.contains_path(c"skip_mount") && m.contains_path(c"system") {
                    info!("{}: loading mount files", info.name);
                    modules.push(m);
                }
            }
            path = path.resize(len);
        }
        Self {
            modules,
            customs: Vec::new(),
        }
    }

    fn inject_zygisk_bins(&mut self, zygisk_lib: &str) {
        self.customs.push(CustomModule::Dir {
            children: {
                let mut children = BTreeMap::new();
                let mut path = FsPathBuf::default();
                #[cfg(target_pointer_width = "64")]
                {
                    if path!("/system/bin/linker").exists() {
                        path = path.join("/system/lib").join(zygisk_lib);
                        children.insert_entry(
                            &path,
                            &FsPathBuf::default().join(get_magisk_tmp()).join("magisk32"),
                            false,
                        );
                    }
                    if path!("/system/bin/linker64").exists() {
                        path = path.join("/system/lib64").join(zygisk_lib);
                        children.insert_entry(
                            &path,
                            &FsPathBuf::default().join(get_magisk_tmp()).join("magisk"),
                            false,
                        );
                    }
                }
                #[cfg(target_pointer_width = "32")]
                {
                    path = path.join("/system/lib").join(zygisk_lib);
                    if path!("/system/bin/linker").exists() {
                        children.insert_entry(
                            &path,
                            &FsPathBuf::default().join(get_magisk_tmp()).join("magisk"),
                            false,
                        );
                    }
                }
                children
            },
        });
    }

    fn inject_magisk_bins(&mut self, magisk_path: &str) {
        let mut path = Utf8CString::default();
        path.push_str(magisk_path);
        path.push_str("/");
        let len = path.len();
        self.customs.push(CustomModule::Dir {
            children: {
                let mut children = BTreeMap::new();

                path.push_str("magisk");
                children.insert_entry(
                    &path,
                    &FsPathBuf::default().join(get_magisk_tmp()).join("magisk"),
                    false,
                );
                path.resize(len);
                path.push_str("magiskpolicy");
                children.insert_entry(
                    &path,
                    &FsPathBuf::default()
                        .join(get_magisk_tmp())
                        .join("magiskpolicy"),
                    false,
                );
                path.resize(len);
                path.push_str("su");
                children.insert_entry(&path, cstr!("./magisk"), true);
                path.resize(len);
                path.push_str("resetprop");
                children.insert_entry(&path, cstr!("./magisk"), true);
                path.resize(len);
                path.push_str("supolicy");
                children.insert_entry(&path, cstr!("./magiskpolicy"), true);

                children
            },
        })
    }

    fn iter(&mut self, name: &Utf8CStr) -> ModuleIterator {
        ModuleIterator {
            modules: self.modules.open_dir(name),
            customs: self.customs.open_dir(name),
            collected: BTreeSet::new(),
        }
    }
}

pub fn deploy_modules(
    module_list: &[ModuleInfo],
    zygisk_lib: &CxxString,
    magisk_path: &CxxString,
) -> bool {
    let res: LoggedResult<()> = try {
        let mut modules = ModuleList::from_module_infos(module_list);
        if !magisk_path.is_empty() {
            modules.inject_magisk_bins(&magisk_path.to_string_lossy());
        }

        if zygisk_lib != "0" {
            modules.inject_zygisk_bins(&zygisk_lib.to_string_lossy());
        }

        let mut system_node = tree_module(path!("/system"), &mut modules.iter(cstr!("system")))?;
        for dir in [cstr!("product"), cstr!("vendor"), cstr!("system_ext")] {
            system_node.extract(dir).and_then(|mut n| {
                let mut dest = cstr!("/").to_owned();
                dest.push_str(dir);
                n.mount(&dest).log_ok()
            });
        }
        system_node.mount(cstr!("/system"))?;
    };
    res.log().is_ok()
}

fn tree_module(base: &FsPath, dir: &mut ModuleIterator) -> LoggedResult<Node> {
    let mut children = BTreeMap::new();
    let mut target_path = FsPathBuf::default().join(base);
    let mut replace = false;
    let mut exist = target_path.exists();

    debug!("tree on {}", target_path);

    let parent_len = target_path.len();
    for e in dir {
        match e {
            ModuleEntry::Dir { name, mut iter } => {
                target_path = target_path.join(&name);
                exist = exist && target_path.exists();
                children.insert(name, tree_module(&target_path, &mut iter)?);
            }
            ModuleEntry::File {
                name,
                path,
                is_link,
            } => {
                if &name == ".replace" {
                    replace = true;
                    continue;
                }
                target_path = target_path.join(&name);

                if !matches!(
                    (
                        is_link,
                        &path,
                        target_path.get_attr().map(|attr| attr.is_symlink())
                    ),
                    (false, Some(_), Ok(false))
                ) {
                    exist = false;
                }

                children.insert(name, Node::File { src: path, is_link });
            }
        }
        target_path = target_path.resize(parent_len);
    }

    Ok(Node::Dir {
        children,
        exist,
        replace,
    })
}
