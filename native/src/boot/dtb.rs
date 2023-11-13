use std::{cell::UnsafeCell, process::exit};

use argh::FromArgs;
use fdt::{
    node::{FdtNode, NodeProperty},
    Fdt,
};

use base::{
    libc::c_char, log_err, map_args, EarlyExitExt, LoggedResult, MappedFile, ResultExt, Utf8CStr,
};

use crate::{check_env, patch::patch_verity};

#[derive(FromArgs)]
struct DtbCli {
    #[argh(positional)]
    file: String,
    #[argh(subcommand)]
    action: DtbAction,
}

#[derive(FromArgs)]
#[argh(subcommand)]
enum DtbAction {
    Print(Print),
    Patch(Patch),
    Test(Test),
}

#[derive(FromArgs)]
#[argh(subcommand, name = "print")]
struct Print {
    #[argh(switch, short = 'f')]
    fstab: bool,
}

#[derive(FromArgs)]
#[argh(subcommand, name = "patch")]
struct Patch {}

#[derive(FromArgs)]
#[argh(subcommand, name = "test")]
struct Test {}

fn print_dtb_usage() {
    eprintln!(
        r#"Usage: magiskboot dtb <file> <action> [args...]
Do dtb related actions to <file>.

Supported actions:
  print [-f]
    Print all contents of dtb for debugging
    Specify [-f] to only print fstab nodes
  patch
    Search for fstab and remove verity/avb
    Modifications are done directly to the file in-place
    Configure with env variables: KEEPVERITY
  test
    Test the fstab's status
    Return values:
    0:valid    1:error"#
    );
}

const MAX_PRINT_LEN: usize = 32;

fn print_node(node: &FdtNode) {
    fn pretty_node(depth_set: &[bool]) {
        let mut depth_set = depth_set.iter().peekable();
        while let Some(depth) = depth_set.next() {
            let last = depth_set.peek().is_none();
            if *depth {
                if last {
                    print!("├── ");
                } else {
                    print!("│   ");
                }
            } else if last {
                print!("└── ");
            } else {
                print!("    ");
            }
        }
    }

    fn pretty_prop(depth_set: &[bool]) {
        let mut depth_set = depth_set.iter().peekable();
        while let Some(depth) = depth_set.next() {
            let last = depth_set.peek().is_none();
            if *depth {
                if last {
                    print!("│  ");
                } else {
                    print!("│   ");
                }
            } else if last {
                print!("└─ ");
            } else {
                print!("    ");
            }
        }
    }

    fn do_print_node(node: &FdtNode, depth_set: &mut Vec<bool>) {
        pretty_node(depth_set);
        let depth = depth_set.len();
        depth_set.push(true);
        println!("{}", node.name);
        let mut properties = node.properties().peekable();
        let mut children = node.children().peekable();
        while let Some(NodeProperty { name, value }) = properties.next() {
            let size = value.len();
            let is_str = !(size > 1 && value[0] == 0)
                && matches!(value.last(), Some(0u8) | None)
                && value.iter().all(|c| *c == 0 || (*c >= 32 && *c < 127));

            if depth_set[depth] && properties.peek().is_none() && children.peek().is_none() {
                depth_set[depth] = false;
            }

            pretty_prop(depth_set);
            if is_str {
                println!(
                    "[{}]: [\"{}\"]",
                    name,
                    if value.is_empty() {
                        ""
                    } else {
                        unsafe { Utf8CStr::from_bytes_unchecked(value) }
                    }
                );
            } else if size > MAX_PRINT_LEN {
                println!("[{}]: <bytes>({})", name, size);
            } else {
                println!("[{}]: {:02x?}", name, value);
            }
        }

        while let Some(child) = children.next() {
            if depth_set[depth] && children.peek().is_none() {
                depth_set[depth] = false;
            }
            do_print_node(&child, depth_set);
        }
        depth_set.pop();
    }

    do_print_node(node, &mut vec![]);
}

fn for_each_fdt<F: FnMut(usize, Fdt) -> LoggedResult<()>>(
    file: &Utf8CStr,
    rw: bool,
    mut f: F,
) -> LoggedResult<()> {
    eprintln!("Loading dtbs from [{}]", file);
    let file = if rw {
        MappedFile::open_rw(file)?
    } else {
        MappedFile::open(file)?
    };
    let mut buf = Some(file.as_ref());
    let mut dtb_num = 0usize;
    while let Some(slice) = buf {
        let slice = if let Some(pos) = slice.windows(4).position(|w| w == b"\xd0\x0d\xfe\xed") {
            &slice[pos..]
        } else {
            break;
        };
        if slice.len() < 40 {
            break;
        }
        let fdt = Fdt::new(slice)?;

        let size = fdt.total_size();

        if size > slice.len() {
            eprintln!("dtb.{:04} is truncated", dtb_num);
            break;
        }

        f(dtb_num, fdt)?;

        dtb_num += 1;
        buf = Some(&slice[size..]);
    }
    Ok(())
}

fn find_fstab<'b, 'a: 'b>(fdt: &'b Fdt<'a>) -> Option<FdtNode<'b, 'a>> {
    fdt.all_nodes().find(|node| node.name == "fstab")
}

fn dtb_print(file: &Utf8CStr, fstab: bool) -> LoggedResult<()> {
    for_each_fdt(file, false, |n, fdt| {
        if fstab {
            if let Some(fstab) = find_fstab(&fdt) {
                eprintln!("Found fstab in dtb.{:04}", n);
                print_node(&fstab);
            }
        } else if let Some(mut root) = fdt.find_node("/") {
            eprintln!("Printing dtb.{:04}", n);
            if root.name.is_empty() {
                root.name = "/";
            }
            print_node(&root);
        }
        Ok(())
    })
}

fn dtb_test(file: &Utf8CStr) -> LoggedResult<bool> {
    let mut ret = true;
    for_each_fdt(file, false, |_, fdt| {
        if let Some(fstab) = find_fstab(&fdt) {
            for child in fstab.children() {
                if child.name != "system" {
                    continue;
                }
                if let Some(mount_point) = child.property("mnt_point") {
                    if mount_point.value == b"/system_root\0" {
                        ret = false;
                        break;
                    }
                }
            }
        }
        Ok(())
    })?;
    Ok(ret)
}

fn dtb_patch(file: &Utf8CStr) -> LoggedResult<bool> {
    let keep_verity = check_env("KEEPVERITY");
    let mut patched = false;
    for_each_fdt(file, true, |n, fdt| {
        for node in fdt.all_nodes() {
            if node.name != "chosen" {
                continue;
            }
            if let Some(boot_args) = node.property("bootargs") {
                boot_args.value.windows(14).for_each(|w| {
                    if w == b"skip_initramfs" {
                        let w = unsafe {
                            &mut *std::mem::transmute::<&[u8], &UnsafeCell<[u8]>>(w).get()
                        };
                        w[..=4].copy_from_slice(b"want");
                        eprintln!("Patch [skip_initramfs] -> [want_initramfs] in dtb.{:04}", n);
                        patched = true;
                    }
                });
            }
        }
        if keep_verity {
            return Ok(());
        }
        if let Some(fstab) = find_fstab(&fdt) {
            for child in fstab.children() {
                if let Some(flags) = child.property("fsmgr_flags") {
                    let flags = unsafe {
                        &mut *std::mem::transmute::<&[u8], &UnsafeCell<[u8]>>(flags.value).get()
                    };
                    if patch_verity(flags) != flags.len() {
                        patched = true;
                    }
                }
            }
        }
        Ok(())
    })?;
    Ok(patched)
}

pub fn dtb_commands(argc: i32, argv: *const *const c_char) -> bool {
    fn inner(argc: i32, argv: *const *const c_char) -> LoggedResult<()> {
        if argc < 1 {
            return Err(log_err!("No arguments"));
        }
        let cmds = map_args(argc, argv)?;

        let mut cli =
            DtbCli::from_args(&["magiskboot", "dtb"], &cmds).on_early_exit(print_dtb_usage);

        let file = Utf8CStr::from_string(&mut cli.file);

        match cli.action {
            DtbAction::Print(Print { fstab }) => {
                dtb_print(file, fstab)?;
            }
            DtbAction::Test(_) => {
                if !dtb_test(file)? {
                    exit(1);
                }
            }
            DtbAction::Patch(_) => {
                if !dtb_patch(file)? {
                    exit(1);
                }
            }
        }
        Ok(())
    }
    inner(argc, argv)
        .log_with_msg(|w| w.write_str("Failed to process dtb"))
        .is_ok()
}
