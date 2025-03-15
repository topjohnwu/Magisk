use crate::ffi::{ModuleInfo, get_magisk_tmp};
use base::{
    Directory, FileAttr, FsPath, FsPathBuf, LoggedResult, ResultExt, Utf8CStr, Utf8CStrBuf,
    Utf8CString, cstr, debug, path, warn,
};
use std::collections::{
    BTreeMap, BTreeSet, btree_map::Entry as MapEntry, btree_set::Entry as SetEntry,
};
use base::WalkResult::Continue;

enum NodeType {
    Dir {
        children: BTreeMap<Utf8CString, Node>,
        need_tmpfs: bool,
    },
    File {
        module_path: Option<Utf8CString>,
    },
}

struct Node {
    pub(crate) node_type: NodeType,
    pub(crate) target_attr: Option<FileAttr>,
}

impl Node {
    fn mount(&mut self, dest: &mut Utf8CString) -> LoggedResult<()> {
        if let NodeType::Dir { need_tmpfs, .. } = &mut self.node_type {
            *need_tmpfs = false;
        }
        self.do_mount(dest, false)
    }
    fn do_mount(&self, dest: &mut Utf8CString, parent_tmpfs: bool) -> LoggedResult<()> {
        // todo: do mount
        match &self.node_type {
            NodeType::Dir {
                children,
                need_tmpfs,
            } => {
                debug!(
                    "{}: {}",
                    if *need_tmpfs {
                        if parent_tmpfs {
                            "mkdir "
                        } else {
                            "tmpfs "
                        }
                    } else {
                        "mirror"
                    },
                    dest
                );
                let len = dest.len();
                for (name, child) in children {
                    dest.push_str("/");
                    dest.push_str(name);
                    child.do_mount(dest, *need_tmpfs)?;
                    unsafe {
                        *dest.as_mut_ptr().add(len) = '\0' as _;
                        dest.set_len(len)
                    };
                }
                if !parent_tmpfs && *need_tmpfs {
                    let mut path = Utf8CString::default();
                    Directory::open(dest).log_with_msg(|w| w.write_str(dest))?.pre_order_walk(|f| {
                        f.path(&mut path)?;
                        if f.is_dir() {
                            debug!("tmpfs: {}", &path);
                        } else {
                            debug!("mirror: {}", &path);
                        }
                        Ok(Continue)
                    })?;
                }
            }
            NodeType::File { module_path } => match module_path {
                None => debug!("delete: {}", dest),
                // FileType::Mirror => debug!("mirror: {}", dest),
                Some(module_path) => debug!("module: {} <- {}", dest, module_path),
            },
        }
        Ok(())
    }
}


enum ModuleEntry {
    Dir {
        path: Utf8CString,
        iter: Option<ModuleIterator>,
    },
    // todo: Custom {
    //
    // }
}

impl ModuleEntry {
    fn name(&self) -> &Utf8CStr {
        match self {
            ModuleEntry::Dir { path, .. } => match path.rfind('/') {
                Some(pos) => unsafe {
                    Utf8CStr::from_bytes_unchecked(&path.as_bytes_with_nul()[pos + 1..])
                },
                None => path,
            },
        }
    }

    fn path(&self) -> &Utf8CStr {
        match self {
            ModuleEntry::Dir { path, .. } => path,
        }
    }

    fn get_attr(&self) -> std::io::Result<FileAttr> {
        match self {
            ModuleEntry::Dir { path, .. } => FsPath::from(path).get_attr(),
        }
    }

    fn iter(&mut self, _name: &Utf8CStr) -> Option<&mut ModuleIterator> {
        match self {
            ModuleEntry::Dir { iter, .. } => iter.as_mut(),
        }
    }
}

struct ModuleIterator {
    name: Utf8CString,
    modules: Vec<Directory>,
    collected: BTreeSet<Utf8CString>,
}

impl ModuleIterator {
    fn from_modules(module_infos: &[ModuleInfo], name: &Utf8CStr) -> Self {
        let mut path = FsPathBuf::default()
            .join(get_magisk_tmp())
            .join(".magisk")
            .join("modules");

        let len = path.len();
        let mut modules = Vec::new();
        for info in module_infos {
            path = path.join(&info.name).join(name);
            if let Ok(m) = Directory::open(&path) {
                modules.push(m);
            }
            path = path.set_len(len);
        }
        Self {
            name: name.to_owned(),
            modules,
            collected: BTreeSet::new(),
        }
    }

    fn name(&self) -> &Utf8CStr {
        &self.name
    }

    fn inject_magisk_bins(&mut self) {
        todo!()
    }

    fn inject_zygisk_bins(&mut self) {
        todo!()
    }
}

impl Iterator for ModuleIterator {
    type Item = ModuleEntry;

    fn next(&mut self) -> Option<Self::Item> {
        match self.modules.last_mut().map(|d| d.read()) {
            None => {
                // todo custom node
                None
            }
            Some(e @ Err(_)) => {
                e.log_ok();
                None
            }
            Some(Ok(Some(e))) => {
                let mut path = Utf8CString::default();
                if let (Ok(_), Ok(name)) = (e.path(&mut path), Utf8CStr::from_cstr(e.name()))
                    && let SetEntry::Vacant(v) = self.collected.entry(name.to_owned())
                {
                    let node = ModuleEntry::Dir {
                        path,
                        iter: if e.is_dir() {
                            Some(ModuleIterator {
                                name: name.to_owned(),
                                modules: self
                                    .modules
                                    .iter()
                                    .filter_map(|d| d.open_dir(v.get().as_cstr()).ok())
                                    .collect(),
                                collected: BTreeSet::new(),
                            })
                        } else {
                            None
                        },
                    };
                    v.insert();
                    return Some(node);
                }
                None
            }
            Some(Ok(None)) => {
                self.modules.pop();
                self.next()
            }
        }
    }
}

pub fn deploy_modules(zygisk_enabled: bool, module_list: &[ModuleInfo]) -> bool {
    let res: LoggedResult<()> = try {
        let mut modules = ModuleIterator::from_modules(module_list, cstr!("system"));
        // todo: product, etc
        // modules.inject_magisk_bins();
        //
        // if zygisk_enabled {
        //     modules.inject_zygisk_bins();
        // }

        let mut system_node = tree_module(path!("/"), &mut modules)?;
        system_node.mount(&mut cstr!("/system").to_owned())?;
    };
    res.is_ok()
}

fn tree_module(base: &FsPath, dir: &mut ModuleIterator) -> LoggedResult<Node> {
    let mut children = BTreeMap::new();
    let mut target_path = FsPathBuf::default().join(base).join(dir.name());
    let mut has_replace = false;

    debug!("tree on {}", target_path.as_str());

    for mut e in dir {
        let name = e.name().to_owned();
        if &name == ".replace" {
            has_replace = true;
            continue;
        }
        match (children.entry(name.to_owned()), e.iter(&name)) {
            (MapEntry::Vacant(v), Some(d)) => {
                let child = tree_module(&target_path, d)?;
                v.insert(child);
            }
            (MapEntry::Vacant(v), None) => {
                let parent_len = target_path.len();
                target_path = target_path.join(v.key());
                let attr = e.get_attr()?;
                let child = Node {
                    node_type: NodeType::File {
                        module_path: if attr.is_char_device() && attr.st.st_rdev == 0 {
                            None
                        } else {
                            Some(e.path().to_owned())
                        },
                    },
                    target_attr: target_path.get_attr().ok(),
                };
                target_path = target_path.set_len(parent_len);
                v.insert(child);
            }
            (_, None) => {
                warn!("Duplicate entry: {}", e.name());
            }
            _ => {}
        }
    }

    let target_attr = target_path.get_attr().ok();
    let need_tmpfs = target_attr.is_none()
        || has_replace
        || children.iter().any(|(_, n)| {
            matches!(
                n,
                Node {
                    node_type: NodeType::Dir {
                        need_tmpfs: true,
                        ..
                    },
                    ..
                } | Node {
                    node_type: NodeType::File { module_path: None },
                    ..
                } | Node {
                    target_attr: None,
                    ..
                }
            )
        });

    Ok(Node {
        node_type: NodeType::Dir {
            children,
            need_tmpfs,
        },
        target_attr,
    })
}
