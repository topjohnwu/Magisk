use crate::ffi::{BootConfig, MagiskInit, backup_init};
use base::{BytesExt, MappedFile, cstr};

impl BootConfig {
    #[allow(unused_imports, unused_unsafe)]
    pub(crate) fn print(&self) {
        use base::{Utf8CStr, debug};
        debug!("skip_initramfs=[{}]", self.skip_initramfs);
        debug!("force_normal_boot=[{}]", self.force_normal_boot);
        debug!("rootwait=[{}]", self.rootwait);
        unsafe {
            debug!(
                "slot=[{}]",
                Utf8CStr::from_ptr_unchecked(self.slot.as_ptr())
            );
            debug!(
                "dt_dir=[{}]",
                Utf8CStr::from_ptr_unchecked(self.dt_dir.as_ptr())
            );
            debug!(
                "fstab_suffix=[{}]",
                Utf8CStr::from_ptr_unchecked(self.fstab_suffix.as_ptr())
            );
            debug!(
                "hardware=[{}]",
                Utf8CStr::from_ptr_unchecked(self.hardware.as_ptr())
            );
            debug!(
                "hardware.platform=[{}]",
                Utf8CStr::from_ptr_unchecked(self.hardware_plat.as_ptr())
            );
        }
        debug!("emulator=[{}]", self.emulator);
        debug!("partition_map=[{:?}]", self.partition_map);
    }
}

impl MagiskInit {
    pub(crate) fn check_two_stage(&self) -> bool {
        cstr!("/first_stage_ramdisk").exists() ||
            cstr!("/second_stage_resources").exists() ||
            cstr!("/system/bin/init").exists() ||
            // Use the apex folder to determine whether 2SI (Android 10+)
            cstr!("/apex").exists() ||
            // If we still have no indication, parse the original init and see what's up
            MappedFile::open(backup_init())
                .map(|data| data.contains(b"selinux_setup"))
                .unwrap_or(false)
    }
}
