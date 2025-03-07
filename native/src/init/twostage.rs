use crate::ffi::MagiskInit;
use base::{
    clone_attr, cstr, debug, error, info,
    libc::{
        mount, statfs, umount2, MNT_DETACH, MS_BIND, O_CLOEXEC,
        O_CREAT, O_RDONLY, O_WRONLY, TMPFS_MAGIC,
    },
    path, raw_cstr, LibcReturn, MappedFile, MutBytesExt, ResultExt,
};
use std::{ffi::c_long, io::Write, ptr::null};

impl MagiskInit {
    pub(crate) fn first_stage(&self) {
        info!("First Stage Init");
        self.prepare_data();

        if !path!("/sdcard").exists() && !path!("/first_stage_ramdisk/sdcard").exists() {
            if self.config.force_normal_boot {
                path!("/first_stage_ramdisk/storage/self")
                    .mkdirs(0o755)
                    .log_ok();
                path!("/system/system/bin/init")
                    .symlink_to(path!("/first_stage_ramdisk/storage/self/primary"))
                    .log_ok();
                debug!(
                    "Symlink /first_stage_ramdisk/storage/self/primary -> /system/system/bin/init"
                );
                path!("/first_stage_ramdisk/sdcard")
                    .create(O_RDONLY | O_CREAT | O_CLOEXEC, 0)
                    .log_ok();
            } else {
                path!("/storage/self").mkdirs(0o755).log_ok();
                path!("/system/system/bin/init")
                    .symlink_to(path!("/storage/self/primary"))
                    .log_ok();
                debug!("Symlink /storage/self/primary -> /system/system/bin/init");
            }
            path!("/init").rename_to(path!("/sdcard")).log_ok();
            // Try to keep magiskinit in rootfs for samsung RKP
            if unsafe {
                mount(
                    raw_cstr!("/sdcard"),
                    raw_cstr!("/sdcard"),
                    null(),
                    MS_BIND,
                    null(),
                )
            } == 0
            {
                debug!("Bind mount /sdcard -> /sdcard");
            } else {
                // rootfs before 3.12
                unsafe {
                    mount(
                        raw_cstr!("/data/magiskinit"),
                        raw_cstr!("/sdcard"),
                        null(),
                        MS_BIND,
                        null(),
                    )
                };
                debug!("Bind mount /data/magiskinit -> /sdcard");
            }
            self.restore_ramdisk_init();
        } else {
            self.restore_ramdisk_init();

            // fallback to hexpatch if /sdcard exists
            match MappedFile::open_rw(cstr!("/init")) {
                Ok(mut map) => {
                    let from = "/system/bin/init";
                    let to = "/data/magiskinit";

                    // Redirect original init to magiskinit
                    let v = map.patch(from.as_bytes(), to.as_bytes());
                    #[allow(unused_variables)]
                    for off in &v {
                        debug!("Patch @ {:#010X} [{}] -> [{}]", off, from, to);
                    }
                }
                _ => {
                    error!("Failed to open /init for hexpatch");
                }
            }
        }
    }

    pub(crate) fn redirect_second_stage(&self) {
        let src = path!("/init");
        let dest = path!("/data/init");
        // Patch init binary
        match MappedFile::open(src) {
            Ok(mut map) => {
                let from = "/system/bin/init";
                let to = "/data/magiskinit";

                // Redirect original init to magiskinit
                let v = map.patch(from.as_bytes(), to.as_bytes());
                #[allow(unused_variables)]
                for off in &v {
                    debug!("Patch @ {:#010X} [{}] -> [{}]", off, from, to);
                }
                match dest.create(O_CREAT | O_WRONLY, 0) {
                    Ok(mut dest) => {
                        dest.write_all(map.as_ref()).log_ok();
                    }
                    _ => {
                        error!("Failed to create {}", dest);
                    }
                }
            }
            _ => {
                error!("Failed to open {} for hexpatch", src);
            }
        }
        clone_attr(src, dest).log_ok();
        unsafe {
            mount(dest.as_ptr(), src.as_ptr(), null(), MS_BIND, null())
                .as_os_err()
                .ok();
        }
    }

    pub(crate) fn second_stage(&mut self) {
        info!("Second Stage Init");
        unsafe {
            umount2(raw_cstr!("/init"), MNT_DETACH);
            umount2(raw_cstr!("/system/bin/init"), MNT_DETACH); // just in case
            path!("/data/init").remove().ok();

            // Make sure init dmesg logs won't get messed up
            *self.argv = raw_cstr!("/system/bin/init") as *mut _;

            // Some weird devices like meizu, uses 2SI but still have legacy rootfs
            let mut sfs: statfs = std::mem::zeroed();
            statfs(raw_cstr!("/"), &mut sfs);
            if sfs.f_type == 0x858458f6 || sfs.f_type as c_long == TMPFS_MAGIC {
                // We are still on rootfs, so make sure we will execute the init of the 2nd stage
                let init_path = path!("/init");
                init_path.remove().ok();
                path!("/system/bin/init").symlink_to(init_path).log_ok();
                self.patch_rw_root();
            } else {
                self.patch_ro_root();
            }
        }
    }
}
