use crate::ffi::{magisk_proxy_main, BootConfig, MagiskInit};
use crate::logging::setup_klog;
use base::{
    cstr, debug, info,
    libc::{basename, getpid, mount, umask},
    raw_cstr, FsPath, LibcReturn, LoggedResult, ResultExt, Utf8CStr,
};
use std::ffi::{c_char, CStr};
use std::ptr::null as nullptr;

impl MagiskInit {
    fn new(argv: *mut *mut c_char) -> Self {
        Self {
            preinit_dev: String::new(),
            mount_list: Vec::new(),
            argv,
            config: BootConfig {
                skip_initramfs: false,
                force_normal_boot: false,
                rootwait: false,
                emulator: false,
                slot: [0; 3],
                dt_dir: [0; 64],
                fstab_suffix: [0; 32],
                hardware: [0; 32],
                hardware_plat: [0; 32],
                partition_map: Vec::new(),
            },
        }
    }

    pub(crate) fn legacy_system_as_root(&mut self) {
        info!("Legacy SAR Init");
        self.prepare_data();
        if self.mount_system_root() {
            self.redirect_second_stage();
        } else {
            self.patch_ro_root();
        }
    }

    pub(crate) fn rootfs(&mut self) {
        info!("RootFS Init");
        self.prepare_data();
        debug!("Restoring /init\n");
        FsPath::from(cstr!("/.backup/init"))
            .rename_to(FsPath::from(cstr!("/init")))
            .log()
            .ok();
        self.patch_rw_root();
    }

    pub(crate) fn recovery(&self) {
        info!("Ramdisk is recovery, abort");
        self.restore_ramdisk_init();
        FsPath::from(cstr!("/.backup")).remove_all().ok();
    }

    pub(crate) fn restore_ramdisk_init(&self) {
        FsPath::from(cstr!("/init")).remove().ok();

        let orig_init = FsPath::from(unsafe { Utf8CStr::from_ptr_unchecked(self.backup_init()) });

        if orig_init.exists() {
            orig_init.rename_to(FsPath::from(cstr!("/init"))).log().ok();
        } else {
            // If the backup init is missing, this means that the boot ramdisk
            // was created from scratch, and the real init is in a separate CPIO,
            // which is guaranteed to be placed at /system/bin/init.
            FsPath::from(cstr!("/system/bin/init"))
                .symlink_to(FsPath::from(cstr!("/init")))
                .log()
                .ok();
        }
    }

    fn start(&mut self) -> LoggedResult<()> {
        if !FsPath::from(cstr!("/proc/cmdline")).exists() {
            FsPath::from(cstr!("/proc")).mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("proc"),
                    raw_cstr!("/proc"),
                    raw_cstr!("proc"),
                    0,
                    nullptr(),
                )
            }
            .as_os_err()?;
            self.mount_list.push("/proc".to_string());
        }
        if !FsPath::from(cstr!("/sys/block")).exists() {
            FsPath::from(cstr!("/sys")).mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("sysfs"),
                    raw_cstr!("/sys"),
                    raw_cstr!("sysfs"),
                    0,
                    nullptr(),
                )
            }
            .as_os_err()?;
            self.mount_list.push("/sys".to_string());
        }

        setup_klog();

        self.config.init();

        let argv1 = unsafe { *self.argv.offset(1) };
        if !argv1.is_null() && unsafe { CStr::from_ptr(argv1) == c"selinux_setup" } {
            self.second_stage();
        } else if self.config.skip_initramfs {
            self.legacy_system_as_root();
        } else if self.config.force_normal_boot {
            self.first_stage();
        } else if FsPath::from(cstr!("/sbin/recovery")).exists()
            || FsPath::from(cstr!("/system/bin/recovery")).exists()
        {
            self.recovery();
        } else if self.check_two_stage() {
            self.first_stage();
        } else {
            self.rootfs();
        }

        // Finally execute the original init
        self.exec_init();

        Ok(())
    }
}

#[no_mangle]
pub unsafe extern "C" fn main(
    argc: i32,
    argv: *mut *mut c_char,
    _envp: *const *const c_char,
) -> i32 {
    umask(0);

    let name = basename(*argv);

    if CStr::from_ptr(name) == c"magisk" {
        return magisk_proxy_main(argc, argv);
    }

    if getpid() == 1 {
        MagiskInit::new(argv).start().log().ok();
    }

    1
}
