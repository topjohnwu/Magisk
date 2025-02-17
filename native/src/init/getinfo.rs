use crate::ffi::{backup_init, BootConfig, MagiskInit};
use base::{debug, path, BytesExt, MappedFile};
use std::ffi::CStr;

impl BootConfig {
    pub(crate) fn print(&self) {
        debug!("skip_initramfs=[{}]", self.skip_initramfs);
        debug!("force_normal_boot=[{}]", self.force_normal_boot);
        debug!("rootwait=[{}]", self.rootwait);
        unsafe {
            debug!("slot=[{:?}]", CStr::from_ptr(self.slot.as_ptr()));
            debug!("dt_dir=[{:?}]", CStr::from_ptr(self.dt_dir.as_ptr()));
            debug!(
                "fstab_suffix=[{:?}]",
                CStr::from_ptr(self.fstab_suffix.as_ptr())
            );
            debug!("hardware=[{:?}]", CStr::from_ptr(self.hardware.as_ptr()));
            debug!(
                "hardware.platform=[{:?}]",
                CStr::from_ptr(self.hardware_plat.as_ptr())
            );
        }
        debug!("emulator=[{}]", self.emulator);
        debug!("partition_map=[{:?}]", self.partition_map);
    }
}

impl MagiskInit {
    pub(crate) fn check_two_stage(&self) -> bool {
        path!("/first_stage_ramdisk").exists() ||
            path!("/second_stage_resources").exists() ||
            path!("/system/bin/init").exists() ||
            // Use the apex folder to determine whether 2SI (Android 10+)
            path!("/apex").exists() ||
            // If we still have no indication, parse the original init and see what's up
            MappedFile::open(backup_init())
                .map(|data| data.contains(b"selinux_setup"))
                .unwrap_or(false)
    }
}
