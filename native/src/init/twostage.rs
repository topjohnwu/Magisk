use crate::ffi::MagiskInit;
use crate::mount::is_rootfs;
use base::{
    LoggedResult, MappedFile, MutBytesExt, ResultExt, cstr, debug, error, info,
    libc::{O_CLOEXEC, O_CREAT, O_RDONLY, O_WRONLY},
    path, raw_cstr,
};
use std::io::Write;

fn patch_init_path(init: &mut MappedFile) {
    let from = "/system/bin/init";
    let to = "/data/magiskinit";

    // Redirect original init to magiskinit
    let v = init.patch(from.as_bytes(), to.as_bytes());
    #[allow(unused_variables)]
    for off in &v {
        debug!("Patch @ {:#010X} [{}] -> [{}]", off, from, to);
    }
}

impl MagiskInit {
    fn hijack_init_with_switch_root(&self) {
        // We make use of original init's `SwitchRoot` to help us bind mount
        // magiskinit to /system/bin/init to hijack second stage init.
        //
        // Two important assumption about 2SI:
        // - The second stage init is always /system/bin/init
        // - After `SwitchRoot`, /sdcard is always a symlink to `/storage/self/primary`.
        //
        // `SwitchRoot` will perform the following:
        // - Recursive move all mounts under `/` to `/system`
        // - chroot to `/system`
        //
        // The trick here is that in Magisk's first stage init, we can mount magiskinit to /sdcard,
        // and create a symlink at /storage/self/primary pointing to /system/system/bin/init.
        //
        // During init's `SwitchRoot`, it will mount move /sdcard (which is magiskinit)
        // to /system/sdcard, which is a symlink to /storage/self/primary, which is a
        // symlink to /system/system/bin/init, which will eventually become /system/bin/init after
        // chroot to /system. The effective result is that we coerce the original init into bind
        // mounting magiskinit to /system/bin/init, successfully hijacking the second stage init.
        //
        // An edge case is that some devices (like meizu) use 2SI but does not switch root.
        // In that case, they must already have a /sdcard in ramfs, thus we can check if
        // /sdcard exists and fallback to using hexpatch.

        if self.config.force_normal_boot {
            path!("/first_stage_ramdisk/storage/self")
                .mkdirs(0o755)
                .log_ok();
            path!("/first_stage_ramdisk/storage/self/primary")
                .create_symlink_to(path!("/system/system/bin/init"))
                .log_ok();
            debug!("Symlink /first_stage_ramdisk/storage/self/primary -> /system/system/bin/init");
            path!("/first_stage_ramdisk/sdcard")
                .create(O_RDONLY | O_CREAT | O_CLOEXEC, 0)
                .log_ok();
        } else {
            path!("/storage/self").mkdirs(0o755).log_ok();
            path!("/storage/self/primary")
                .create_symlink_to(path!("/system/system/bin/init"))
                .log_ok();
            debug!("Symlink /storage/self/primary -> /system/system/bin/init");
        }
        path!("/init").rename_to(path!("/sdcard")).log_ok();

        // First try to mount magiskinit from rootfs to workaround Samsung RKP
        if path!("/sdcard").bind_mount_to(path!("/sdcard")).is_ok() {
            debug!("Bind mount /sdcard -> /sdcard");
        } else {
            // Binding mounting from rootfs is not supported before Linux 3.12
            path!("/data/magiskinit")
                .bind_mount_to(path!("/sdcard"))
                .log_ok();
            debug!("Bind mount /data/magiskinit -> /sdcard");
        }
    }

    pub(crate) fn first_stage(&self) {
        info!("First Stage Init");
        self.prepare_data();

        if !path!("/sdcard").exists() && !path!("/first_stage_ramdisk/sdcard").exists() {
            self.hijack_init_with_switch_root();
            self.restore_ramdisk_init();
        } else {
            self.restore_ramdisk_init();

            // fallback to hexpatch if /sdcard exists
            match MappedFile::open_rw(cstr!("/init")) {
                Ok(mut map) => patch_init_path(&mut map),
                _ => {
                    error!("Failed to open /init for hexpatch");
                }
            }
        }
    }

    pub(crate) fn patch_init_for_second_stage(&self) {
        let src = path!("/init");
        let dest = path!("/data/init");
        // Patch init binary
        match MappedFile::open(src) {
            Ok(mut map) => {
                patch_init_path(&mut map);
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
        let _: LoggedResult<()> = try {
            let attr = src.follow_link().get_attr()?;
            dest.set_attr(&attr)?;
            dest.bind_mount_to(src)?;
        };
    }

    pub(crate) fn second_stage(&mut self) {
        info!("Second Stage Init");

        path!("/init").unmount().ok();
        path!("/system/bin/init").unmount().ok(); // just in case
        path!("/data/init").remove().ok();

        unsafe {
            // Make sure init dmesg logs won't get messed up
            *self.argv = raw_cstr!("/system/bin/init") as *mut _;
        }

        // Some weird devices like meizu, uses 2SI but still have legacy rootfs
        if is_rootfs() {
            // We are still on rootfs, so make sure we will execute the init of the 2nd stage
            let init_path = path!("/init");
            init_path.remove().ok();
            init_path
                .create_symlink_to(path!("/system/bin/init"))
                .log_ok();
            self.patch_rw_root();
        } else {
            self.patch_ro_root();
        }
    }
}
