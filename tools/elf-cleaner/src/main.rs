use std::{fs::File, env::args};
use lief::{Binary, elf::{dynamic::{DynamicEntry, Tag, Flags, Entries}, header::{Arch, FileType}}};

const SUPPORTED_DT_FLAGS_1: u64 = 0x00000001u64 | 0x00000002u64 | 0x00000008u64;

fn main() {
    let args = args().skip(1).collect::<Vec<_>>();
    for path in args {
        let mut file = match File::open(&path) {
            Ok(f) => f,
            Err(e) => {
                eprintln!("Error opening file '{}': {}", path, e);
                continue;
            }
        };
        let mut elf = match Binary::from(&mut file) {
            Some(Binary::ELF(elf)) => elf,
            _ => {
                eprintln!("Error: '{}' is not a valid ELF file", path);
                continue;
            }
        };
        let mut tags_to_remove = vec![Tag::RPATH];
        if matches!(elf.header().file_type(), FileType::DYN) {
            tags_to_remove.push(Tag::PREINIT_ARRAY);
        }
        if matches!(elf.header().machine_type(), Arch::AARCH64) {
            tags_to_remove.push(Tag::AARCH64_BTI_PLT);
            tags_to_remove.push(Tag::AARCH64_PAC_PLT);
            tags_to_remove.push(Tag::AARCH64_VARIANT_PCS);
        }
        for tag in tags_to_remove.into_iter() {
            if elf.dynamic_entries().any(|e| e.tag() == tag) {
                elf.remove_dynamic_entries_by_tag(tag);
                println!("Removing the {:?} dynamic section entry from '{}'", tag, path);
            }
        }
        for e in elf.dynamic_entries() {
            if e.tag() == Tag::FLAGS_1 {
                let flags = e.value();
                let new_flags = flags & SUPPORTED_DT_FLAGS_1;
                if flags != new_flags {
                    println!("Replacing unsupported {:?} flags {:#x} with {:#x} in '{}'", Tag::FLAGS_1, flags, new_flags, path);
                    elf.remove_dynamic_entries_by_tag(Tag::FLAGS_1);
                    elf.add_dynamic_entry(&Entries::Flags(Flags::create_dt_flag_1(new_flags)));
                    break;
                }
            }
        }
        elf.write(path.as_ref());
    }
}
