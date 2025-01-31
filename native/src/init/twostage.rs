use crate::ffi::MagiskInit;
use base::{cstr, info, raw_cstr, FsPath, ResultExt, libc::{statfs, umount2, MNT_DETACH, TMPFS_MAGIC}};
use std::ffi::c_long;

impl MagiskInit {
    pub(crate) fn second_stage(&mut self) {
        info!("Second Stage Init");
        unsafe {
            umount2(raw_cstr!("/init"), MNT_DETACH);
            umount2(raw_cstr!("/system/bin/init"), MNT_DETACH); // just in case
            FsPath::from(cstr!("/data/init")).remove().ok();
            
            // Make sure init dmesg logs won't get messed up
            *self.argv = raw_cstr!("/system/bin/init") as *mut _;

            // Some weird devices like meizu, uses 2SI but still have legacy rootfs
            let mut sfs: statfs = std::mem::zeroed();
            statfs(raw_cstr!("/"), std::ptr::from_mut(&mut sfs));
            if sfs.f_type == 0x858458f6  || sfs.f_type as c_long == TMPFS_MAGIC {
                // We are still on rootfs, so make sure we will execute the init of the 2nd stage
                let init_path = FsPath::from(cstr!("/init"));
                init_path.remove().ok();
                FsPath::from(cstr!("/system/bin/init")).symlink_to(init_path).log().ok();
                self.patch_rw_root();
            } else {
                self.patch_ro_root();
            }
        }
    }
}
