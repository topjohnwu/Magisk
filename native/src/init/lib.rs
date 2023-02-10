use std::env;
use std::path::Path;

pub use base;
use base::read_lines;
pub use logging::*;

mod logging;

#[cxx::bridge(namespace = "rust")]
pub mod ffi2 {
    extern "Rust" {
        fn setup_klog();
        fn print_rules_device() -> i32;
    }
}

pub fn print_rules_device() -> i32 {
    const UNKNOWN: i32 = 0;
    const PERSIST: i32 = UNKNOWN + 1;
    const METADATA: i32 = PERSIST + 1;
    const CACHE: i32 = METADATA + 1;
    const UNENCRYPTED: i32 = CACHE + 1;
    const DATA: i32 = UNENCRYPTED + 1;
    const EXISTING: i32 = DATA + 1;

    let encrypted = env::var("ISENCRYPTED").map_or(false, |var| var == "true");

    let mut matched = UNKNOWN;
    let mut rules_dev = String::new();

    if let Ok(lines) = read_lines("/proc/self/mountinfo") {
        for line in lines {
            if let Ok(line) = line {
                let new_matched;
                if line.contains("/.magisk/sepolicy.rules ") {
                    new_matched = EXISTING;
                } else if line.contains(" - ext4 ") && !line.contains("/dm-") {
                    if line.contains(" / /cache ") && matched < CACHE {
                        new_matched = CACHE;
                    } else if line.contains(" / /data ") && matched < DATA {
                        if !encrypted {
                            new_matched = UNENCRYPTED;
                        } else if Path::new("/data/unencrypted").is_dir() {
                            new_matched = DATA;
                        } else {
                            continue;
                        }
                    } else if line.contains(" / /metadata ") && matched < METADATA {
                        new_matched = METADATA;
                    } else if (line.contains(" / /persist ")
                        || line.contains(" / /mnt/vendor/persist "))
                        && matched < PERSIST
                    {
                        new_matched = PERSIST;
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }
                if let Some(device) = line.splitn(4, ' ').nth(2) {
                    rules_dev.clear();
                    rules_dev += device;
                    matched = new_matched;
                }
            }
        }
        if matched > UNKNOWN {
            println!("{rules_dev}");
            return 0;
        } else {
            eprintln!("Failed to find sepolicy rules partition");
        }
    } else {
        eprintln!("Error reading /proc/self/mountinfo");
    }
    return 1;
}
