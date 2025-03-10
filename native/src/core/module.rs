use crate::ffi::{ModuleInfo, get_magisk_tmp};
use base::{
    Directory, FileAttr, FsPath, FsPathBuf, LoggedResult, Utf8CStr, Utf8CStrBuf, Utf8CStrBufArr,
    Utf8CString, cstr, debug, path, warn,
};
use std::collections::{BTreeMap, btree_map::Entry};

enum DirType {
    Tmpfs,
    Transparent,
}

enum FileType {
    Whiteout,
    Mirror,
    Module { module_path: Utf8CString },
}

enum NodeType {
    Dir {
        dir_type: DirType,
        children: BTreeMap<Utf8CString, Node>,
    },
    File {
        file_type: FileType,
    },
}

struct Node {
    pub(crate) node_type: NodeType,
    pub(crate) target_attr: Option<FileAttr>,
}

impl Node {
    fn mount(&self, dest: &mut Utf8CString) {
        // todo: do mount
        match &self.node_type {
            NodeType::Dir { children, dir_type } => {
                debug!(
                    "{}: {}",
                    match dir_type {
                        DirType::Tmpfs => "tmpfs ",
                        DirType::Transparent => "mirror",
                    },
                    dest
                );
                let len = dest.len();
                for (name, child) in children {
                    dest.push_str("/");
                    dest.push_str(name);
                    child.mount(dest);
                    unsafe { dest.set_len(len) };
                }
            }
            NodeType::File { file_type } => match file_type {
                FileType::Whiteout => debug!("delete: {}", dest),
                FileType::Mirror => debug!("mirror: {}", dest),
                FileType::Module { module_path } => debug!("module: {} <- {}", dest, module_path),
            },
        }
    }
    fn exists(&self) -> bool {
        !matches!(
            self,
            Node {
                node_type: NodeType::Dir {
                    dir_type: DirType::Tmpfs,
                    ..
                },
                ..
            } | Node {
                node_type: NodeType::File {
                    file_type: FileType::Whiteout,
                    ..
                },
                ..
            } | Node {
                node_type: NodeType::File { .. },
                target_attr: None,
                ..
            }
        )
    }
}

pub fn deploy_modules(_zygisk_enabled: bool, module_list: &Vec<ModuleInfo>) -> bool {
    let res: LoggedResult<()> = try {
        let mut path = FsPathBuf::default()
            .join(get_magisk_tmp())
            .join(".magisk")
            .join("modules");

        let len = path.len();

        let mut module_dirs = Vec::new();
        for m in module_list {
            path = path.join(&m.name);
            if let Ok(m) = Directory::open(&path) {
                module_dirs.push(m);
            }
            path = path.set_len(len);
        }

        let system_node = tree_module(path!("/"), &module_dirs, cstr!("system"), Some(false))?;
        system_node.mount(&mut cstr!("/system").to_owned());
        // todo: product, etc
        //  zygisk bin
        //  magisk bin
    };
    res.is_ok()
}

fn tree_module(
    base: &FsPath,
    module_dirs: &[Directory],
    child: &Utf8CStr,
    tmpfs: Option<bool>,
) -> LoggedResult<Node> {
    let mut children: BTreeMap<Utf8CString, Node> = BTreeMap::new();
    let mut module_dirs = module_dirs
        .iter()
        .filter_map(|d| d.open_dir(child.as_cstr()).ok())
        .collect::<Vec<_>>();

    let mut target_path = FsPathBuf::default().join(base).join(child);
    let mut has_replace = false;

    debug!("collecting on {}", target_path.as_str());

    loop {
        match module_dirs.last_mut().map(|d| d.read()) {
            None => {
                let need_tmpfs = match tmpfs {
                    None => has_replace || children.iter().any(|(_, n)| !n.exists()),
                    Some(b) => b,
                };

                if need_tmpfs
                    && !has_replace
                    && let Ok(mut d) = Directory::open(&target_path)
                {
                    while let Some(e) = d.read()? {
                        let name = Utf8CStr::from_cstr(e.name())?.to_owned();
                        if let Entry::Vacant(v) = children.entry(name) {
                            if e.is_dir() {
                                let child = tree_module(&target_path, &[], v.key(), Some(true))?;
                                v.insert(child);
                            } else {
                                v.insert(Node {
                                    node_type: NodeType::File {
                                        file_type: FileType::Mirror,
                                    },
                                    target_attr: Some(e.get_attr()?),
                                });
                            }
                        }
                    }
                }

                break Ok(Node {
                    node_type: NodeType::Dir {
                        dir_type: if need_tmpfs {
                            DirType::Tmpfs
                        } else {
                            DirType::Transparent
                        },
                        children,
                    },
                    target_attr: None,
                });
            }
            Some(e) => {
                if let Some(e) = e? {
                    if e.name() == c".replace" {
                        has_replace = true;
                        continue;
                    }
                    if let Entry::Vacant(v) =
                        children.entry(Utf8CStr::from_cstr(e.name())?.to_owned())
                    {
                        if e.is_dir() {
                            let child = tree_module(&target_path, &module_dirs, v.key(), None)?;
                            v.insert(child);
                        } else {
                            let parent_len = target_path.len();
                            target_path = target_path.join(v.key());

                            let mut module_path = Utf8CStrBufArr::default();
                            e.path(&mut module_path)?;
                            v.insert(if e.is_char_device() && e.get_attr()?.st.st_rdev == 0 {
                                Node {
                                    node_type: NodeType::File {
                                        file_type: FileType::Whiteout,
                                    },
                                    target_attr: None,
                                }
                            } else {
                                Node {
                                    node_type: NodeType::File {
                                        file_type: FileType::Module {
                                            module_path: module_path.to_owned(),
                                        },
                                    },
                                    target_attr: target_path.get_attr().ok(),
                                }
                            });
                            target_path = target_path.set_len(parent_len);
                        }
                    } else if !e.is_dir() {
                        warn!("Duplicate entry: {}", e.name().to_string_lossy());
                    }
                } else {
                    module_dirs.pop();
                    continue;
                }
            }
        };
    }
}
