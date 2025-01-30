use std::ffi::CStr;
use base::debug;
use crate::ffi::BootConfig;

impl BootConfig {
    pub(crate) fn print(&self) {
        debug!("skip_initramfs=[{}]", self.skip_initramfs);
        debug!("force_normal_boot=[{}]", self.force_normal_boot);
        debug!("rootwait=[{}]", self.rootwait);
        unsafe {
            debug!("slot=[{:?}]", CStr::from_ptr(self.slot.as_ptr()));
            debug!("dt_dir=[{:?}]", CStr::from_ptr(self.dt_dir.as_ptr()));
            debug!("fstab_suffix=[{:?}]", CStr::from_ptr(self.fstab_suffix.as_ptr()));
            debug!("hardware=[{:?}]", CStr::from_ptr(self.hardware.as_ptr()));
            debug!("hardware.platform=[{:?}]", CStr::from_ptr(self.hardware_plat.as_ptr()));
        }
        debug!("emulator=[{}]", self.emulator);
        debug!("partition_map=[{:?}]", self.partition_map);
    }
}